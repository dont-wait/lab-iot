'use strict';

/**
 * Lop phan quyen (RBAC) cho Node-RED Dashboard - Lab 7 Bai 3.
 *
 * Node-RED Dashboard khong co ACL theo nguoi dung: adminAuth chi bao ve
 * trinh soan flow, con /ui thi ai cung xem duoc. Module nay bo sung:
 *   - Tai khoan + mat khau hash scrypt (users.json, khong luu mat khau ro).
 *   - Phien dang nhap bang cookie HttpOnly + chu ky HMAC-SHA256.
 *   - Phan quyen theo vai tro: admin / viewer / gateway.
 *   - API doc du lieu cho moi vai tro, API dieu khien chi danh cho admin.
 *   - Ghi audit log cho hanh dong dang nhap va dieu khien thiet bi.
 *
 * Module khong phu thuoc package ngoai, chi dung thu vien chuan cua Node.
 */

const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

const DEFAULT_COOKIE_NAME = 'iot_session';
const DEFAULT_SESSION_TTL_MS = 2 * 60 * 60 * 1000;
const LOGIN_WINDOW_MS = 60 * 1000;
const LOGIN_MAX_FAILURES = 5;
const LOGIN_BLOCK_MS = 60 * 1000;
const MAX_BODY_BYTES = 16 * 1024;
const MAX_AUDIT_ENTRIES = 200;

const ROLE_PERMISSIONS = Object.freeze({
    admin: Object.freeze([
        'device:view',
        'device:control',
        'dashboard:view',
        'sensor:ingest',
        'users:read',
        'audit:read',
    ]),
    viewer: Object.freeze(['device:view', 'dashboard:view']),
    gateway: Object.freeze(['sensor:ingest']),
});

const VALID_ROLES = Object.freeze(Object.keys(ROLE_PERMISSIONS));
const CONTROL_ACTIONS = Object.freeze(['ON', 'OFF', 'TOGGLE']);

function hashPassword(password) {
    const salt = crypto.randomBytes(16);
    const params = { N: 16384, r: 8, p: 1 };
    const hash = crypto.scryptSync(String(password), salt, 64, params);
    return ['scrypt', params.N, params.r, params.p, salt.toString('base64url'), hash.toString('base64url')].join('$');
}

function verifyPassword(password, stored) {
    const parts = String(stored || '').split('$');
    if (parts.length !== 6 || parts[0] !== 'scrypt') {
        return false;
    }
    const [, rawN, rawR, rawP, rawSalt, rawHash] = parts;
    const N = Number(rawN);
    const r = Number(rawR);
    const p = Number(rawP);
    if (!Number.isInteger(N) || !Number.isInteger(r) || !Number.isInteger(p)) {
        return false;
    }
    let salt;
    let expected;
    try {
        salt = Buffer.from(rawSalt, 'base64url');
        expected = Buffer.from(rawHash, 'base64url');
    } catch {
        return false;
    }
    if (salt.length === 0 || expected.length === 0) {
        return false;
    }
    let actual;
    try {
        actual = crypto.scryptSync(String(password), salt, expected.length, {
            N,
            r,
            p,
            maxmem: 64 * 1024 * 1024,
        });
    } catch {
        return false;
    }
    return actual.length === expected.length && crypto.timingSafeEqual(actual, expected);
}

function signToken(token, secret) {
    return crypto.createHmac('sha256', secret).update(token).digest('base64url');
}

function parseCookies(header) {
    const cookies = {};
    if (!header) {
        return cookies;
    }
    for (const part of String(header).split(';')) {
        const index = part.indexOf('=');
        if (index < 0) {
            continue;
        }
        const name = part.slice(0, index).trim();
        const value = part.slice(index + 1).trim();
        if (name) {
            cookies[name] = decodeURIComponent(value);
        }
    }
    return cookies;
}

function loadOrCreateSecret(file) {
    const target = path.resolve(file);
    try {
        const existing = fs.readFileSync(target, 'utf8').trim();
        if (existing.length >= 32) {
            return existing;
        }
    } catch {
        /* chua co file, tao moi ben duoi */
    }
    const secret = crypto.randomBytes(48).toString('base64url');
    fs.mkdirSync(path.dirname(target), { recursive: true });
    fs.writeFileSync(target, secret + '\n', { mode: 0o600 });
    return secret;
}

function loadUsers(file) {
    const document = JSON.parse(fs.readFileSync(path.resolve(file), 'utf8'));
    const list = Array.isArray(document) ? document : document.users;
    if (!Array.isArray(list)) {
        throw new Error('users.json phai chua mang "users"');
    }
    const users = new Map();
    for (const entry of list) {
        if (!entry || !entry.username) {
            continue;
        }
        if (!VALID_ROLES.includes(entry.role)) {
            throw new Error(`Vai tro khong hop le cho ${entry.username}: ${entry.role}`);
        }
        if (!entry.password_hash) {
            throw new Error(`Thieu password_hash cho ${entry.username}`);
        }
        users.set(String(entry.username).toLowerCase(), {
            username: String(entry.username),
            name: entry.name || String(entry.username),
            role: entry.role,
            createdAt: entry.created_at || null,
            password_hash: entry.password_hash,
        });
    }
    return users;
}

function publicUser(user) {
    return {
        username: user.username,
        name: user.name,
        role: user.role,
        createdAt: user.createdAt || null,
    };
}

function escapeHtml(value) {
    return String(value)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

function clientIp(req) {
    const forwarded = req.headers['x-forwarded-for'];
    if (typeof forwarded === 'string' && forwarded.length > 0) {
        return forwarded.split(',')[0].trim();
    }
    return req.socket && req.socket.remoteAddress ? req.socket.remoteAddress : 'unknown';
}

function sameOrigin(req) {
    const origin = req.headers.origin;
    if (!origin) {
        return true; // curl / thiet bi khong gui Origin
    }
    try {
        return new URL(origin).host === req.headers.host;
    } catch {
        return false;
    }
}

function isJsonRequest(req) {
    const type = String(req.headers['content-type'] || '').toLowerCase();
    return type.startsWith('application/json');
}

function sendJson(res, status, payload, extraHeaders = {}) {
    const body = JSON.stringify(payload);
    res.writeHead(status, {
        'Content-Type': 'application/json; charset=utf-8',
        'Cache-Control': 'no-store',
        'X-Content-Type-Options': 'nosniff',
        ...extraHeaders,
    });
    res.end(body);
}

function sendHtml(res, status, html, extraHeaders = {}) {
    res.writeHead(status, {
        'Content-Type': 'text/html; charset=utf-8',
        'Cache-Control': 'no-store',
        'X-Content-Type-Options': 'nosniff',
        'Content-Security-Policy':
            "default-src 'self'; style-src 'unsafe-inline'; script-src 'unsafe-inline'; img-src 'self' data:",
        ...extraHeaders,
    });
    res.end(html);
}

function readJsonBody(req, res, limit = MAX_BODY_BYTES) {
    return new Promise((resolve) => {
        let size = 0;
        let tooLarge = false;
        const chunks = [];
        req.on('data', (chunk) => {
            size += chunk.length;
            if (size > limit) {
                tooLarge = true;
                return;
            }
            chunks.push(chunk);
        });
        req.on('end', () => {
            if (tooLarge) {
                sendJson(res, 413, {
                    error: 'payload_too_large',
                    message: `Du lieu gui len vuot qua ${limit} byte.`,
                });
                resolve(null);
                return;
            }
            if (chunks.length === 0) {
                resolve({});
                return;
            }
            try {
                const body = JSON.parse(Buffer.concat(chunks).toString('utf8'));
                if (!body || typeof body !== 'object' || Array.isArray(body)) {
                    throw new Error('JSON root phai la object');
                }
                resolve(body);
            } catch (error) {
                sendJson(res, 400, {
                    error: 'invalid_json',
                    message: `Khong doc duoc JSON: ${error.message}`,
                });
                resolve(null);
            }
        });
        req.on('error', () => resolve(null));
    });
}

function renderLoginPage(next, options) {
    const siteName = escapeHtml(options.siteName || 'IoT Dashboard');
    return `<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Đăng nhập · ${siteName}</title>
<style>
  :root { color-scheme: light dark; --bg:#0b1220; --card:#111c2e; --text:#e2e8f0; --muted:#94a3b8; --accent:#38bdf8; --danger:#f87171; }
  * { box-sizing: border-box; }
  body { margin:0; min-height:100vh; display:grid; place-items:center; padding:24px;
         font-family: ui-sans-serif, system-ui, "Segoe UI", sans-serif;
         background: radial-gradient(1200px 600px at 20% 0%, #12233d 0%, var(--bg) 55%); color:var(--text); }
  main { width:100%; max-width:420px; background:var(--card); border:1px solid #1e293b;
         border-radius:18px; padding:28px; box-shadow:0 24px 60px rgba(2,6,23,.55); }
  .brand { display:flex; gap:12px; align-items:center; margin-bottom:20px; }
  .logo { width:44px; height:44px; border-radius:12px; display:grid; place-items:center;
          background:linear-gradient(135deg, #0ea5e9, #22c55e); font-size:22px; }
  h1 { font-size:19px; margin:0; }
  p.sub { margin:4px 0 0; color:var(--muted); font-size:13px; }
  label { display:block; font-size:13px; color:var(--muted); margin:16px 0 6px; }
  input { width:100%; padding:11px 12px; border-radius:10px; border:1px solid #24344d;
          background:#0b1524; color:var(--text); font-size:14px; }
  input:focus { outline:2px solid var(--accent); outline-offset:1px; }
  button { width:100%; margin-top:22px; padding:12px; border:0; border-radius:10px; cursor:pointer;
           background:linear-gradient(135deg,#0284c7,#0ea5e9); color:#f8fafc; font-weight:600; font-size:15px; }
  button[disabled] { opacity:.6; cursor:progress; }
  .error { margin-top:14px; padding:10px 12px; border-radius:10px; font-size:13px;
           background:rgba(248,113,113,.12); border:1px solid rgba(248,113,113,.35); color:#fecaca; display:none; }
  .error.visible { display:block; }
  footer { margin-top:18px; font-size:12px; color:var(--muted); text-align:center; line-height:1.6; }
  code { background:#0b1524; border:1px solid #1e293b; border-radius:6px; padding:1px 5px; }
</style>
</head>
<body>
<main>
  <div class="brand">
    <div class="logo">🔐</div>
    <div>
      <h1>${siteName}</h1>
      <p class="sub">Đăng nhập để xem dashboard giám sát</p>
    </div>
  </div>
  <form id="login-form" autocomplete="on">
    <label for="username">Tên đăng nhập</label>
    <input id="username" name="username" autocomplete="username" required autofocus>
    <label for="password">Mật khẩu</label>
    <input id="password" name="password" type="password" autocomplete="current-password" required>
    <button id="submit" type="submit">Đăng nhập</button>
    <div class="error" id="error" role="alert"></div>
  </form>
  <footer>
    Tài khoản demo: <code>admin</code> (toàn quyền) · <code>viewer</code> (chỉ xem).<br>
    Mật khẩu xem trong <code>README.md</code>. Phiên đăng nhập hết hạn sau 2 giờ.
  </footer>
</main>
<script>
(function () {
  const form = document.getElementById('login-form');
  const errorBox = document.getElementById('error');
  const submit = document.getElementById('submit');
  const next = ${JSON.stringify(next)};
  form.addEventListener('submit', async (event) => {
    event.preventDefault();
    errorBox.classList.remove('visible');
    submit.disabled = true;
    try {
      const response = await fetch('/api/rbac/login', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', 'X-Requested-With': 'fetch' },
        credentials: 'same-origin',
        body: JSON.stringify({
          username: form.username.value.trim(),
          password: form.password.value
        })
      });
      const data = await response.json().catch(() => ({}));
      if (!response.ok) {
        const retry = data.retryAfter ? ' Thử lại sau ' + data.retryAfter + ' giây.' : '';
        throw new Error((data.message || 'Đăng nhập thất bại.') + retry);
      }
      window.location.replace(next || '/ui');
    } catch (error) {
      errorBox.textContent = error.message;
      errorBox.classList.add('visible');
    } finally {
      submit.disabled = false;
    }
  });
})();
</script>
</body>
</html>`;
}

/**
 * @param {object} options
 * @param {string} options.usersFile           Duong dan users.json
 * @param {string} options.sessionSecret       Secret ky cookie (uu tien env)
 * @param {string} [options.cookieName]
 * @param {number} [options.sessionTtlMs]
 * @param {boolean} [options.secureCookie]
 * @param {string} [options.auditFile]         Ghi audit JSONL (tuy chon)
 * @param {string} [options.siteName]
 * @param {(command: object) => void|Promise<void>} [options.onCommand]
 * @param {(line: string) => void} [options.log]
 */
function createRbac(options) {
    if (!options || !options.usersFile) {
        throw new Error('createRbac can usersFile');
    }
    const users = loadUsers(options.usersFile);
    const cookieName = options.cookieName || DEFAULT_COOKIE_NAME;
    const sessionTtlMs = options.sessionTtlMs || DEFAULT_SESSION_TTL_MS;
    const secret = options.sessionSecret;
    if (!secret || String(secret).length < 32) {
        throw new Error('sessionSecret phai dai it nhat 32 ky tu');
    }
    const log = options.log || ((line) => console.log(line));
    const siteName = options.siteName || 'IoT Dashboard';
    const auditFile = options.auditFile ? path.resolve(options.auditFile) : null;
    const onCommand = typeof options.onCommand === 'function' ? options.onCommand : null;

    const sessions = new Map();
    const loginFailures = new Map();
    const auditEntries = [];
    const commandQueue = [];

    const deviceState = {
        deviceId: 'ESP32_LIVING_ROOM',
        temperature: null,
        humidity: null,
        lamp: 'OFF',
        updatedAt: null,
    };
    const history = { temperature: [], humidity: [] };
    const HISTORY_LIMIT = 180;

    function addAudit(entry) {
        const record = {
            ts: new Date().toISOString(),
            ...entry,
        };
        auditEntries.push(record);
        if (auditEntries.length > MAX_AUDIT_ENTRIES) {
            auditEntries.shift();
        }
        if (auditFile) {
            try {
                fs.appendFileSync(auditFile, JSON.stringify(record) + '\n');
            } catch (error) {
                log(`[rbac] khong ghi duoc audit file: ${error.message}`);
            }
        }
        return record;
    }

    function getSession(req) {
        const raw = parseCookies(req.headers.cookie)[cookieName];
        if (!raw) {
            return null;
        }
        const separator = raw.lastIndexOf('.');
        if (separator < 1) {
            return null;
        }
        const token = raw.slice(0, separator);
        const signature = raw.slice(separator + 1);
        const expected = signToken(token, secret);
        if (signature.length !== expected.length) {
            return null;
        }
        if (!crypto.timingSafeEqual(Buffer.from(signature), Buffer.from(expected))) {
            return null;
        }
        const session = sessions.get(token);
        if (!session) {
            return null;
        }
        if (session.expiresAt <= Date.now()) {
            sessions.delete(token);
            return null;
        }
        return { token, ...session };
    }

    function hasPermission(user, permission) {
        return Boolean(user && ROLE_PERMISSIONS[user.role] && ROLE_PERMISSIONS[user.role].includes(permission));
    }

    function requirePermission(req, res, permission) {
        const session = getSession(req);
        if (!session) {
            sendJson(res, 401, {
                error: 'authentication_required',
                message: 'Can dang nhap de thuc hien thao tac nay.',
            });
            return null;
        }
        if (!hasPermission(session, permission)) {
            addAudit({
                action: 'access_denied',
                username: session.username,
                role: session.role,
                permission,
                allowed: false,
                ip: clientIp(req),
            });
            sendJson(res, 403, {
                error: 'forbidden',
                message: 'Tai khoan cua ban chi duoc xem, khong duoc dieu khien thiet bi.',
                requiredPermission: permission,
                role: session.role,
            });
            return null;
        }
        return session;
    }

    function rateLimitKey(ip, username) {
        return `${ip}|${String(username).toLowerCase()}`;
    }

    function checkRateLimit(ip, username) {
        const key = rateLimitKey(ip, username);
        const record = loginFailures.get(key);
        if (!record) {
            return null;
        }
        const now = Date.now();
        if (record.blockedUntil && record.blockedUntil > now) {
            return { retryAfter: Math.ceil((record.blockedUntil - now) / 1000) };
        }
        if (record.blockedUntil && record.blockedUntil <= now) {
            loginFailures.delete(key);
        }
        return null;
    }

    function registerFailure(ip, username) {
        const key = rateLimitKey(ip, username);
        const now = Date.now();
        const record = loginFailures.get(key) || { count: 0, firstAt: now, blockedUntil: 0 };
        if (now - record.firstAt > LOGIN_WINDOW_MS) {
            record.count = 0;
            record.firstAt = now;
            record.blockedUntil = 0;
        }
        record.count += 1;
        let retryAfter = null;
        if (record.count >= LOGIN_MAX_FAILURES) {
            record.blockedUntil = now + LOGIN_BLOCK_MS;
            record.count = 0;
            record.firstAt = now;
            retryAfter = Math.ceil(LOGIN_BLOCK_MS / 1000);
        }
        loginFailures.set(key, record);
        return retryAfter;
    }

    function clearFailures(ip, username) {
        loginFailures.delete(rateLimitKey(ip, username));
    }

    function permissionsFor(session) {
        return ROLE_PERMISSIONS[session.role] || [];
    }

    async function handleLogin(req, res) {
        if (!isJsonRequest(req) || !sameOrigin(req)) {
            sendJson(res, 415, {
                error: 'unsupported_media_type',
                message: 'Yeu cau dang nhap phai la JSON va cung origin.',
            });
            return;
        }
        const body = await readJsonBody(req, res);
        if (body === null) {
            return;
        }
        const username = String(body.username || '').trim();
        const password = String(body.password || '');
        if (!username || !password) {
            sendJson(res, 400, {
                error: 'missing_credentials',
                message: 'Vui long nhap ten dang nhap va mat khau.',
            });
            return;
        }
        const ip = clientIp(req);
        const limited = checkRateLimit(ip, username);
        if (limited) {
            addAudit({
                action: 'login',
                username,
                allowed: false,
                reason: 'rate_limited',
                ip,
            });
            sendJson(
                res,
                429,
                {
                    error: 'too_many_attempts',
                    message: 'Qua nhieu lan dang nhap sai. Tam khoa tai khoan.',
                    retryAfter: limited.retryAfter,
                },
                { 'Retry-After': String(limited.retryAfter) }
            );
            return;
        }
        const user = users.get(username.toLowerCase());
        const valid = user ? verifyPassword(password, user.password_hash) : false;
        if (!valid) {
            const retryAfter = registerFailure(ip, username);
            addAudit({ action: 'login', username, allowed: false, reason: 'invalid_credentials', ip });
            if (retryAfter) {
                sendJson(
                    res,
                    429,
                    {
                        error: 'too_many_attempts',
                        message: 'Qua nhieu lan dang nhap sai. Tam khoa tai khoan.',
                        retryAfter,
                    },
                    { 'Retry-After': String(retryAfter) }
                );
                return;
            }
            sendJson(res, 401, {
                error: 'invalid_credentials',
                message: 'Sai ten dang nhap hoac mat khau.',
            });
            return;
        }
        clearFailures(ip, username);
        const token = crypto.randomBytes(32).toString('base64url');
        const expiresAt = Date.now() + sessionTtlMs;
        sessions.set(token, {
            username: user.username,
            role: user.role,
            expiresAt,
            ip,
        });
        addAudit({ action: 'login', username: user.username, role: user.role, allowed: true, ip });
        const cookie = [
            `${cookieName}=${token}.${signToken(token, secret)}`,
            'Path=/',
            'HttpOnly',
            'SameSite=Strict',
            `Max-Age=${Math.floor(sessionTtlMs / 1000)}`,
            options.secureCookie ? 'Secure' : null,
        ]
            .filter(Boolean)
            .join('; ');
        sendJson(
            res,
            200,
            {
                user: publicUser(user),
                permissions: ROLE_PERMISSIONS[user.role],
                expiresAt: new Date(expiresAt).toISOString(),
            },
            { 'Set-Cookie': cookie }
        );
    }

    function handleLogout(req, res) {
        if (!sameOrigin(req)) {
            sendJson(res, 403, { error: 'forbidden', message: 'Yeu cau khong cung origin.' });
            return;
        }
        const session = getSession(req);
        if (session) {
            sessions.delete(session.token);
            addAudit({ action: 'logout', username: session.username, role: session.role, allowed: true, ip: clientIp(req) });
        }
        res.writeHead(204, {
            'Set-Cookie': `${cookieName}=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0`,
            'Cache-Control': 'no-store',
        });
        res.end();
    }

    async function handleControl(req, res) {
        const session = requirePermission(req, res, 'device:control');
        if (!session) {
            return;
        }
        if (!isJsonRequest(req) || !sameOrigin(req)) {
            sendJson(res, 415, {
                error: 'unsupported_media_type',
                message: 'Lenh dieu khien phai la JSON va cung origin.',
            });
            return;
        }
        const body = await readJsonBody(req, res);
        if (body === null) {
            return;
        }
        const action = String(body.action || '').toUpperCase();
        if (!CONTROL_ACTIONS.includes(action)) {
            sendJson(res, 400, {
                error: 'invalid_action',
                message: 'action phai la ON, OFF hoac TOGGLE.',
            });
            return;
        }
        const before = deviceState.lamp;
        const lamp = action === 'TOGGLE' ? (before === 'ON' ? 'OFF' : 'ON') : action;
        deviceState.lamp = lamp;
        deviceState.updatedAt = new Date().toISOString();
        const command = {
            id: crypto.randomUUID(),
            ts: new Date().toISOString(),
            action,
            lamp,
            previousLamp: before,
            topic: 'home/living_room/control',
            payload: JSON.stringify({ command: action === 'TOGGLE' ? 'TOGGLE' : action }),
            user: publicUser(session),
            role: session.role,
            at: deviceState.updatedAt,
        };
        commandQueue.push(command);
        if (commandQueue.length > 50) {
            commandQueue.shift();
        }
        try {
            if (onCommand) {
                await onCommand(command);
            }
        } catch (error) {
            addAudit({
                action: 'device_control',
                username: session.username,
                role: session.role,
                allowed: true,
                action_value: action,
                lamp,
                result: 'publish_failed',
                error: error.message,
                ip: clientIp(req),
            });
            sendJson(res, 502, {
                error: 'command_dispatch_failed',
                message: 'Khong gui duoc lenh den thiet bi.',
            });
            return;
        }
        addAudit({
            action: 'device_control',
            username: session.username,
            role: session.role,
            allowed: true,
            command: action,
            lamp,
            ip: clientIp(req),
        });
        sendJson(res, 200, {
            commandId: command.id,
            lamp,
            previousLamp: before,
            updatedAt: deviceState.updatedAt,
        });
    }

    function handleState(req, res) {
        const session = requirePermission(req, res, 'device:view');
        if (!session) {
            return;
        }
        sendJson(res, 200, {
            user: publicUser(session),
            permissions: permissionsFor(session),
            state: deviceState,
            history,
            serverTime: new Date().toISOString(),
        });
    }

    async function handleSensorIngest(req, res) {
        const session = requirePermission(req, res, 'sensor:ingest');
        if (!session) {
            return;
        }
        if (!isJsonRequest(req) || !sameOrigin(req)) {
            sendJson(res, 415, {
                error: 'unsupported_media_type',
                message: 'Du lieu cam bien phai la JSON va cung origin.',
            });
            return;
        }
        const body = await readJsonBody(req, res);
        if (body === null) {
            return;
        }
        const temperature = Number(body.temperature);
        const humidity = Number(body.humidity);
        if (!Number.isFinite(temperature) || !Number.isFinite(humidity)) {
            sendJson(res, 400, {
                error: 'invalid_sensor_payload',
                message: 'temperature va humidity phai la so.',
            });
            return;
        }
        if (temperature < -40 || temperature > 125 || humidity < 0 || humidity > 100) {
            sendJson(res, 400, {
                error: 'sensor_value_out_of_range',
                message: 'temperature phai trong -40..125, humidity trong 0..100.',
            });
            return;
        }
        const now = new Date().toISOString();
        deviceState.temperature = Number(temperature.toFixed(2));
        deviceState.humidity = Number(humidity.toFixed(2));
        deviceState.updatedAt = now;
        if (body.deviceId) {
            deviceState.deviceId = String(body.deviceId).slice(0, 64);
        }
        history.temperature.push({ t: now, v: deviceState.temperature });
        history.humidity.push({ t: now, v: deviceState.humidity });
        if (history.temperature.length > HISTORY_LIMIT) {
            history.temperature.shift();
        }
        if (history.humidity.length > HISTORY_LIMIT) {
            history.humidity.shift();
        }
        addAudit({
            action: 'sensor_ingest',
            username: session.username,
            role: session.role,
            allowed: true,
            ip: clientIp(req),
        });
        sendJson(res, 202, { accepted: true, at: now });
    }

    function handleUsers(req, res) {
        const session = requirePermission(req, res, 'users:read');
        if (!session) {
            return;
        }
        sendJson(res, 200, {
            users: [...users.values()].map(publicUser),
            roles: VALID_ROLES.map((role) => ({ role, permissions: ROLE_PERMISSIONS[role] })),
        });
    }

    function handleAudit(req, res) {
        const session = requirePermission(req, res, 'audit:read');
        if (!session) {
            return;
        }
        sendJson(res, 200, { entries: auditEntries.slice(-100).reverse() });
    }

    function redirectToLogin(req, res) {
        const original = req.originalUrl || req.url || '/ui';
        const next = original.startsWith('/') ? original : '/ui';
        res.writeHead(302, {
            Location: `/login?next=${encodeURIComponent(next)}`,
            'Cache-Control': 'no-store',
        });
        res.end();
    }

    function middleware(req, res, next) {
        let url;
        try {
            url = new URL(req.url, 'http://localhost');
        } catch {
            sendJson(res, 400, { error: 'bad_request', message: 'URL khong hop le.' });
            return;
        }
        const pathname = url.pathname.replace(/\/+$/, '') || '/';
        const method = (req.method || 'GET').toUpperCase();

        if (pathname === '/login' && method === 'GET') {
            const requested = url.searchParams.get('next') || '/ui';
            const safeNext = requested.startsWith('/') && !requested.startsWith('//') ? requested : '/ui';
            if (getSession(req)) {
                res.writeHead(302, { Location: safeNext, 'Cache-Control': 'no-store' });
                res.end();
                return;
            }
            sendHtml(res, 200, renderLoginPage(safeNext, { siteName }));
            return;
        }
        if (pathname === '/api/rbac/login' && method === 'POST') {
            handleLogin(req, res).catch((error) => {
                log(`[rbac] loi dang nhap: ${error.stack || error.message}`);
                sendJson(res, 500, { error: 'internal_error', message: 'Loi may chu.' });
            });
            return;
        }
        if (pathname === '/api/rbac/logout' && method === 'POST') {
            handleLogout(req, res);
            return;
        }
        if (pathname === '/api/rbac/me' && method === 'GET') {
            const session = getSession(req);
            if (!session) {
                sendJson(res, 401, { error: 'authentication_required', message: 'Chua dang nhap.' });
                return;
            }
            sendJson(res, 200, {
                user: publicUser(session),
                permissions: permissionsFor(session),
                expiresAt: new Date(session.expiresAt).toISOString(),
            });
            return;
        }
        if (pathname === '/api/state' && method === 'GET') {
            handleState(req, res);
            return;
        }
        if (pathname === '/api/sensor' && method === 'POST') {
            handleSensorIngest(req, res).catch((error) => {
                log(`[rbac] loi sensor ingest: ${error.stack || error.message}`);
                sendJson(res, 500, { error: 'internal_error', message: 'Loi may chu.' });
            });
            return;
        }
        if (pathname === '/api/device/control' && method === 'POST') {
            handleControl(req, res).catch((error) => {
                log(`[rbac] loi dieu khien: ${error.stack || error.message}`);
                sendJson(res, 500, { error: 'internal_error', message: 'Loi may chu.' });
            });
            return;
        }
        if (pathname === '/api/rbac/users' && method === 'GET') {
            handleUsers(req, res);
            return;
        }
        if (pathname === '/api/audit' && method === 'GET') {
            handleAudit(req, res);
            return;
        }
        if (pathname === '/healthz') {
            sendJson(res, 200, { status: 'ok' });
            return;
        }
        if (pathname === '/ui' || pathname.startsWith('/ui/')) {
            if (!getSession(req)) {
                redirectToLogin(req, res);
                return;
            }
        }
        next();
    }

    // Don phien het han moi 10 phut, khong giu process chay vi interval.
    const cleanup = setInterval(() => {
        const now = Date.now();
        for (const [token, session] of sessions) {
            if (session.expiresAt <= now) {
                sessions.delete(token);
            }
        }
    }, 10 * 60 * 1000);
    if (typeof cleanup.unref === 'function') {
        cleanup.unref();
    }

    return {
        middleware,
        getSession,
        hasPermission,
        state: deviceState,
        history,
        auditEntries,
        users,
        /** Node-RED flow goi ham nay de lay cac lenh cho MQTT out. */
        drainCommands(limit = 10) {
            return commandQueue.splice(0, Math.max(0, limit));
        },
        close() {
            clearInterval(cleanup);
        },
        // Hook cho test va cho script quan tri.
        addAudit,
        clearRateLimit(ip, username) {
            if (ip && username) {
                clearFailures(ip, username);
                return;
            }
            loginFailures.clear();
        },
    };
}

module.exports = {
    ROLE_PERMISSIONS,
    VALID_ROLES,
    hashPassword,
    verifyPassword,
    loadOrCreateSecret,
    loadUsers,
    createRbac,
};
