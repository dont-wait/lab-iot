'use strict';

/**
 * Kiem thu tu dong lop phan quyen (Lab 7 - Bai 3).
 *
 * Chay: node tests/verify_rbac.js
 * Ket qua duoc ghi vao evidence/verify_rbac.log va in ra stdout.
 *
 * Bo test khoi dong mot HTTP server that voi middleware that trong rbac.js,
 * sau do gia lap hai tai khoan admin / viewer de kiem tra:
 *   1. Chua dang nhap -> khong xem duoc /ui va API.
 *   2. Dang nhap sai -> 401, sai nhieu lan -> 429.
 *   3. viewer dang nhap -> xem duoc thong so nhung dieu khien -> 403.
 *   4. admin dang nhap -> dieu khien duoc, lenh duoc phat.
 *   5. Cookie bi sua -> 401.
 *   6. Dashboard template co an/disable nut theo quyen.
 */

const fs = require('node:fs');
const http = require('node:http');
const path = require('node:path');
const { createRbac } = require('../rbac.js');

const ROOT = path.join(__dirname, '..');
const EVIDENCE_DIR = path.join(ROOT, 'evidence');
fs.mkdirSync(EVIDENCE_DIR, { recursive: true });

const lines = [];
let failures = 0;
let passes = 0;

function log(line = '') {
    lines.push(line);
    console.log(line);
}

function pass(name, detail = '') {
    passes += 1;
    log(`PASS  ${name}${detail ? ' — ' + detail : ''}`);
}

function fail(name, detail = '') {
    failures += 1;
    log(`FAIL  ${name}${detail ? ' — ' + detail : ''}`);
}

function check(name, condition, detail = '') {
    if (condition) { pass(name, detail); } else { fail(name, detail); }
}

function startServer() {
    const rbac = createRbac({
        usersFile: path.join(ROOT, 'users.json'),
        sessionSecret: 'test-secret-0123456789-abcdefghijklmnopqrstuvwxyz',
        siteName: 'Lab 7 Test Dashboard',
        onCommand: (command) => {
            commands.push(command);
        },
    });
    const commands = [];

    const server = http.createServer((req, res) => {
        rbac.middleware(req, res, () => {
            if ((req.url || '').split('?')[0].startsWith('/ui')) {
                res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
                res.end('<html><body><div id="dashboard">dashboard</div></body></html>');
                return;
            }
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('not found');
        });
    });

    return new Promise((resolve) => {
        server.listen(0, '127.0.0.1', () => {
            resolve({ server, rbac, commands, port: server.address().port });
        });
    });
}

async function request(port, method, urlPath, options = {}) {
    const headers = { ...(options.headers || {}) };
    const init = { method, headers, redirect: 'manual' };
    if (options.body !== undefined) {
        init.body = typeof options.body === 'string' ? options.body : JSON.stringify(options.body);
        if (!headers['Content-Type'] && !headers['content-type']) {
            headers['Content-Type'] = 'application/json';
        }
    }
    if (options.cookie) {
        headers.Cookie = options.cookie;
    }
    const response = await fetch(`http://127.0.0.1:${port}${urlPath}`, init);
    const text = await response.text();
    let json = null;
    try { json = JSON.parse(text); } catch { /* khong phai JSON */ }
    return { status: response.status, headers: response.headers, text, json };
}

function extractCookie(response) {
    const raw = response.headers.getSetCookie ? response.headers.getSetCookie() : [];
    const cookieLine = raw.find((value) => value.startsWith('iot_session='));
    if (!cookieLine) { return null; }
    return cookieLine.split(';')[0];
}

async function login(port, username, password, cookie) {
    return request(port, 'POST', '/api/rbac/login', {
        body: { username, password },
        cookie,
        headers: { Origin: `http://127.0.0.1:${port}`, 'X-Requested-With': 'fetch' },
    });
}

async function main() {
    const { server, rbac, commands, port } = await startServer();
    const origin = `http://127.0.0.1:${port}`;

    log('='.repeat(72));
    log('KIEM THU PHAN QUYEN DASHBOARD - LAB 7 BAI 3');
    log(`Thoi diem: ${new Date().toISOString()}`);
    log('='.repeat(72));

    try {
        // ---------------------------------------------------------------- 1
        log('\n[1] Truy cap khi chua dang nhap');
        let response = await request(port, 'GET', '/ui');
        check('/ui -> 302 ve trang dang nhap', response.status === 302 &&
            String(response.headers.get('location')).startsWith('/login?next='), `status=${response.status} location=${response.headers.get('location')}`);

        response = await request(port, 'GET', '/api/state');
        check('GET /api/state -> 401', response.status === 401, `status=${response.status}`);

        response = await request(port, 'POST', '/api/device/control', { body: { action: 'ON' } });
        check('POST /api/device/control -> 401', response.status === 401, `status=${response.status}`);
        check('Khong co lenh nao duoc phat khi chua dang nhap', commands.length === 0, `commands=${commands.length}`);

        // ---------------------------------------------------------------- 2
        log('\n[2] Dang nhap sai va chong do mat khau');
        response = await login(port, 'viewer', 'sai-mat-khau');
        check('Sai mat khau -> 401', response.status === 401, `status=${response.status}`);

        response = await request(port, 'POST', '/api/rbac/login', {
            body: { username: 'viewer', password: 'Viewer@123' },
            headers: { 'Content-Type': 'text/plain' },
        });
        check('Sai Content-Type -> 415', response.status === 415, `status=${response.status}`);

        for (let attempt = 0; attempt < 5; attempt += 1) {
            response = await login(port, 'khong-ton-tai', 'sai-mat-khau');
        }
        check('Sai 5 lan lien tiep -> 429 (rate limit)', response.status === 429, `status=${response.status}`);
        check('Co header Retry-After', Boolean(response.headers.get('retry-after')), `retry-after=${response.headers.get('retry-after')}`);

        // ---------------------------------------------------------------- 3
        log('\n[3] Tai khoan viewer (read-only)');
        response = await login(port, 'viewer', 'Viewer@123');
        const viewerCookie = extractCookie(response);
        check('viewer dang nhap -> 200', response.status === 200, `status=${response.status}`);
        check('cookie co HttpOnly', String(response.headers.get('set-cookie')).includes('HttpOnly'));
        check('cookie co SameSite=Strict', String(response.headers.get('set-cookie')).includes('SameSite=Strict'));
        check('role tra ve la viewer', response.json && response.json.user && response.json.user.role === 'viewer', JSON.stringify(response.json && response.json.user));
        check('permissions viewer khong co device:control',
            response.json && Array.isArray(response.json.permissions) &&
            !response.json.permissions.includes('device:control') &&
            response.json.permissions.includes('device:view'),
            JSON.stringify(response.json && response.json.permissions));
        check('cookie viewer duoc tao', Boolean(viewerCookie));

        response = await request(port, 'GET', '/api/state', { cookie: viewerCookie });
        check('viewer xem duoc thong so -> 200', response.status === 200, `status=${response.status}`);
        check('API tra ve nhiet do/do am/den',
            Boolean(response.json && response.json.state) &&
            'temperature' in response.json.state &&
            'humidity' in response.json.state &&
            'lamp' in response.json.state,
            JSON.stringify(response.json && response.json.state));

        response = await request(port, 'GET', '/api/rbac/me', { cookie: viewerCookie });
        check('GET /api/rbac/me -> 200 voi viewer', response.status === 200 &&
            response.json && response.json.user.role === 'viewer', `status=${response.status}`);

        response = await request(port, 'GET', '/ui', { cookie: viewerCookie });
        check('viewer mo duoc /ui -> 200', response.status === 200, `status=${response.status}`);

        const commandsBeforeViewer = commands.length;
        response = await request(port, 'POST', '/api/device/control', {
            body: { action: 'ON' },
            cookie: viewerCookie,
            headers: { Origin: origin, 'X-Requested-With': 'fetch' },
        });
        check('viewer bam nut dieu khien -> 403', response.status === 403, `status=${response.status}`);
        check('403 co requiredPermission=device:control',
            Boolean(response.json && response.json.requiredPermission === 'device:control'),
            JSON.stringify(response.json));
        check('Khong co lenh nao duoc phat boi viewer', commands.length === commandsBeforeViewer,
            `commands=${commands.length}`);
        check('Trang thai den khong doi', (await request(port, 'GET', '/api/state', { cookie: viewerCookie })).json.state.lamp === 'OFF');

        response = await request(port, 'GET', '/api/rbac/users', { cookie: viewerCookie });
        check('viewer khong doc duoc danh sach tai khoan -> 403', response.status === 403, `status=${response.status}`);

        response = await request(port, 'POST', '/api/sensor', {
            body: { temperature: 30, humidity: 60 },
            cookie: viewerCookie,
        });
        check('viewer khong duoc ghi du lieu cam bien -> 403', response.status === 403, `status=${response.status}`);

        // ---------------------------------------------------------------- 4
        log('\n[4] Tai khoan admin');
        response = await login(port, 'admin', 'Admin@123');
        const adminCookie = extractCookie(response);
        check('admin dang nhap -> 200', response.status === 200, `status=${response.status}`);
        check('admin co quyen device:control',
            Boolean(response.json && response.json.permissions.includes('device:control')));
        check('admin co quyen users:read',
            Boolean(response.json && response.json.permissions.includes('users:read')));

        response = await request(port, 'GET', '/api/rbac/users', { cookie: adminCookie });
        check('admin doc duoc danh sach tai khoan -> 200', response.status === 200, `status=${response.status}`);
        check('Danh sach co admin + viewer, khong lo password_hash',
            Boolean(response.json && response.json.users.length >= 2) &&
            !JSON.stringify(response.json.users).includes('password_hash') &&
            !JSON.stringify(response.json.users).includes('scrypt$'),
            `users=${response.json && response.json.users.map((user) => user.role).join(',')}`);

        const commandsBeforeAdmin = commands.length;
        response = await request(port, 'POST', '/api/device/control', {
            body: { action: 'ON' },
            cookie: adminCookie,
            headers: { Origin: origin, 'X-Requested-With': 'fetch' },
        });
        check('admin bat den -> 200', response.status === 200, `status=${response.status}`);
        check('Trang thai den = ON', Boolean(response.json && response.json.lamp === 'ON'), JSON.stringify(response.json));
        check('Lenh MQTT duoc phat dung topic/payload',
            commands.length === commandsBeforeAdmin + 1 &&
            commands[commands.length - 1].topic === 'home/living_room/control' &&
            commands[commands.length - 1].payload.includes('ON'),
            JSON.stringify(commands[commands.length - 1]));
        check('Lenh ghi nhan nguoi gui la admin',
            commands[commands.length - 1].user.username === 'admin' &&
            commands[commands.length - 1].role === 'admin');

        response = await request(port, 'POST', '/api/device/control', {
            body: { action: 'TOGGLE' },
            cookie: adminCookie,
            headers: { Origin: origin, 'X-Requested-With': 'fetch' },
        });
        check('admin dao trang thai -> OFF', response.status === 200 && response.json.lamp === 'OFF',
            JSON.stringify(response.json));

        response = await request(port, 'POST', '/api/device/control', {
            body: { action: 'NOOP' },
            cookie: adminCookie,
        });
        check('action khong hop le -> 400', response.status === 400, `status=${response.status}`);

        response = await request(port, 'GET', '/api/audit', { cookie: adminCookie });
        check('admin doc duoc audit log -> 200', response.status === 200 && Array.isArray(response.json.entries));
        const auditText = JSON.stringify(response.json.entries);
        check('Audit co ghi lan dang nhap cua viewer', auditText.includes('"username":"viewer"'));
        check('Audit co ghi lan bi tu choi 403 cua viewer',
            response.json.entries.some((entry) => entry.action === 'access_denied' && entry.username === 'viewer'));
        check('Audit co ghi lenh dieu khien cua admin',
            response.json.entries.some((entry) => entry.action === 'device_control' && entry.username === 'admin' && entry.lamp === 'ON'));

        // ---------------------------------------------------------------- 5
        log('\n[5] Gia mao cookie / phien');
        const tampered = viewerCookie.replace(/.$/, (char) => (char === 'A' ? 'B' : 'A'));
        response = await request(port, 'GET', '/api/state', { cookie: tampered });
        check('Cookie bi sua chu ky -> 401', response.status === 401, `status=${response.status}`);

        response = await request(port, 'GET', '/api/state', { cookie: 'iot_session=gia-mao.sai-chu-ky' });
        check('Cookie gia mao hoan toan -> 401', response.status === 401, `status=${response.status}`);

        response = await request(port, 'POST', '/api/rbac/logout', { cookie: viewerCookie });
        check('Logout -> 204 va xoa cookie', response.status === 204 &&
            String(response.headers.get('set-cookie')).includes('Max-Age=0'), `status=${response.status}`);
        response = await request(port, 'GET', '/api/state', { cookie: viewerCookie });
        check('Sau logout, cookie cu khong con hieu luc -> 401', response.status === 401, `status=${response.status}`);

        // ---------------------------------------------------------------- 6
        log('\n[6] Kiem tra Dashboard template the hien dung phan quyen');
        const controlTemplate = fs.readFileSync(path.join(ROOT, 'ui', 'control_panel.html'), 'utf8');
        check('Template an nut khi thieu device:control',
            controlTemplate.includes("has('device:control')") &&
            controlTemplate.includes('setControlsEnabled(false)'));
        check('Nut bi disable o phia trinh duyet',
            controlTemplate.includes('buttons[i].disabled = !enabled'));
        check('Template hien khoa read-only', controlTemplate.includes('Chế độ chỉ xem'));
        check('Moi lan bam nut deu goi API co kiem tra quyen',
            controlTemplate.includes("fetch('/api/device/control'"));
        check('Template an bang quan ly nguoi dung voi viewer',
            controlTemplate.includes("has('users:read')") && controlTemplate.includes("$('iot-admin').hidden = true"));

        const flows = JSON.parse(fs.readFileSync(path.join(ROOT, 'flows.json'), 'utf8'));
        check('flows.json co dashboard template va MQTT command queue',
            flows.some((node) => node.type === 'ui_template' && node.id === 'template-control') &&
            flows.some((node) => node.type === 'function' && node.id === 'queue-function') &&
            flows.some((node) => node.type === 'mqtt out' && node.id === 'mqtt-out-control'));
        const serverSource = fs.readFileSync(path.join(ROOT, 'server.js'), 'utf8');
        check('server.js gan rbac.middleware truoc router cua Node-RED',
            serverSource.includes('app.use(rbac.middleware)') &&
            serverSource.indexOf('app.use(rbac.middleware)') <
            serverSource.indexOf('app.use(settings.httpAdminRoot, RED.httpAdmin)') &&
            serverSource.indexOf('app.use(rbac.middleware)') <
            serverSource.indexOf('app.use(settings.httpNodeRoot, RED.httpNode)'));
        check('settings.js cung cap rbac trong functionGlobalContext',
            fs.readFileSync(path.join(ROOT, 'settings.js'), 'utf8').includes('rbac,'));
        check('users.json khong chua mat khau ro',
            !fs.readFileSync(path.join(ROOT, 'users.json'), 'utf8').includes('Admin@123') &&
            !fs.readFileSync(path.join(ROOT, 'users.json'), 'utf8').includes('Viewer@123'));

        // ---------------------------------------------------------------- tong ket
        log('\n' + '='.repeat(72));
        log(`KET QUA: ${passes} PASS, ${failures} FAIL`);
        log('='.repeat(72));
        if (failures === 0) {
            log('Tat ca kiem tra deu dat: viewer dang nhap duoc nhung moi thao tac');
            log('dieu khien deu bi tu choi (403) o ca giao dien lan may chu.');
        }
        log('');
        log(`Audit log trong bo nho (${rbac.auditEntries.length} su kien):`);
        for (const entry of rbac.auditEntries) {
            log(`  ${entry.ts} | ${entry.action.padEnd(15)} | ${String(entry.username || '-').padEnd(8)} | ${entry.role || '-'} | allowed=${entry.allowed}${entry.reason ? ' | ' + entry.reason : ''}`);
        }
    } finally {
        server.close();
    }

    const output = lines.join('\n') + '\n';
    fs.writeFileSync(path.join(EVIDENCE_DIR, 'verify_rbac.log'), output);
    if (failures > 0) {
        process.exitCode = 1;
    }
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
