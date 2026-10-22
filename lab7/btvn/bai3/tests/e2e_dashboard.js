'use strict';

/**
 * E2E that - Lab 7 Bai 3.
 *
 * Khoi dong HTTP server that voi rbac.middleware, sau do:
 *   1. Dang nhap bang tai khoan viewer qua /api/rbac/login.
 *   2. Render that template control_panel.html trong mot DOM (jsdom)
 *      voi cookie cua viewer.
 *   3. Xac thuc nut dieu khien bi disabled, khoa read-only hien ra,
 *      bang quan ly nguoi dung bi an.
 *   4. Bam thu nut dieu khien -> khong co request nao duoc gui.
 *   5. Goi thang API /api/device/control -> 403.
 *   6. Lam lai voi admin: nut bat, bang quan ly hien, dieu khien -> 200.
 *
 * Chay: node tests/e2e_dashboard.js
 * Ket qua: evidence/e2e.log
 */

const fs = require('node:fs');
const http = require('node:http');
const path = require('node:path');
const { JSDOM } = require('jsdom');
const { createRbac } = require('../rbac.js');

const ROOT = path.join(__dirname, '..');
const EVIDENCE_DIR = path.join(ROOT, 'evidence');
fs.mkdirSync(EVIDENCE_DIR, { recursive: true });

const lines = [];
let passes = 0;
let failures = 0;

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
        sessionSecret: 'e2e-secret-0123456789-abcdefghijklmnopqrstuvwxyz',
        siteName: 'Lab 7 E2E Dashboard',
    });
    const controlPanel = fs.readFileSync(path.join(ROOT, 'ui', 'control_panel.html'), 'utf8');
    const sensorPanel = fs.readFileSync(path.join(ROOT, 'ui', 'sensor_panel.html'), 'utf8');

    const server = http.createServer((req, res) => {
        rbac.middleware(req, res, () => {
            const pathname = (req.url || '').split('?')[0];
            if (pathname === '/ui' || pathname.startsWith('/ui/')) {
                res.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
                res.end(`<!doctype html><html><head><meta charset="utf-8"></head><body>${sensorPanel}${controlPanel}</body></html>`);
                return;
            }
            res.writeHead(404, { 'Content-Type': 'text/plain' });
            res.end('not found');
        });
    });

    return new Promise((resolve) => {
        server.listen(0, '127.0.0.1', () => {
            resolve({ server, rbac, port: server.address().port });
        });
    });
}

function extractCookie(response) {
    const raw = response.headers.getSetCookie ? response.headers.getSetCookie() : [];
    const line = raw.find((value) => value.startsWith('iot_session='));
    return line ? line.split(';')[0] : null;
}

async function postJson(port, urlPath, body, cookie) {
    const headers = {
        'Content-Type': 'application/json',
        Origin: `http://127.0.0.1:${port}`,
        'X-Requested-With': 'fetch',
    };
    if (cookie) { headers.Cookie = cookie; }
    const response = await fetch(`http://127.0.0.1:${port}${urlPath}`, {
        method: 'POST',
        headers,
        body: JSON.stringify(body),
        redirect: 'manual',
    });
    let json = null;
    try { json = await response.json(); } catch { /* ignore */ }
    return { response, json };
}

async function login(port, username, password) {
    const { response, json } = await postJson(port, '/api/rbac/login', { username, password });
    return { response, json, cookie: extractCookie(response) };
}

async function renderUi(port, cookie) {
    const response = await fetch(`http://127.0.0.1:${port}/ui`, {
        headers: { Cookie: cookie },
        redirect: 'manual',
    });
    const html = await response.text();
    const requests = [];
    const dom = new JSDOM(html, {
        url: `http://127.0.0.1:${port}/ui`,
        runScripts: 'dangerously',
        pretendToBeVisual: true,
        beforeParse(window) {
            window.fetch = (input, init = {}) => {
                const url = new URL(String(input), `http://127.0.0.1:${port}`).toString();
                const headers = { ...(init.headers || {}), Cookie: cookie };
                requests.push({ url, method: (init.method || 'GET').toUpperCase() });
                return fetch(url, { ...init, headers, redirect: 'manual' });
            };
            // Canvas cua sensor panel khong can cho bai test nay.
            window.HTMLCanvasElement.prototype.getContext = () => null;
        },
    });
    return { response, dom, requests };
}

async function waitFor(checkFn, timeoutMs = 3000) {
    const start = Date.now();
    while (Date.now() - start < timeoutMs) {
        if (checkFn()) { return true; }
        await new Promise((resolve) => setTimeout(resolve, 25));
    }
    return false;
}

function buttonStates(document) {
    return [...document.querySelectorAll('#iot-actions .iot-btn')].map((button) => button.disabled);
}

async function main() {
    const { server, rbac, port } = await startServer();
    const doms = [];

    log('='.repeat(72));
    log('E2E DASHBOARD PHAN QUYEN - LAB 7 BAI 3');
    log(`Thoi diem: ${new Date().toISOString()}`);
    log('='.repeat(72));

    try {
        // ---------------------------------------------------------- viewer
        log('\n[1] Dang nhap tai khoan phu "viewer" va render Dashboard');
        const viewerLogin = await login(port, 'viewer', 'Viewer@123');
        check('viewer dang nhap thanh cong -> 200', viewerLogin.response.status === 200,
            `status=${viewerLogin.response.status}`);
        check('cookie phien viewer duoc tao', Boolean(viewerLogin.cookie));

        const viewer = await renderUi(port, viewerLogin.cookie);
        doms.push(viewer.dom);
        check('viewer mo duoc /ui -> 200', viewer.response.status === 200,
            `status=${viewer.response.status}`);

        const viewerDoc = viewer.dom.window.document;
        const rendered = await waitFor(() => viewerDoc.getElementById('iot-user-role').textContent !== '--');
        check('Dashboard doc duoc phien viewer qua /api/rbac/me', rendered);

        const roleText = viewerDoc.getElementById('iot-user-role').textContent.trim();
        check('Vai tro hien thi la "Chi xem"', roleText === 'Chỉ xem', `role=${roleText}`);

        const locks = buttonStates(viewerDoc);
        check('Ca 3 nut dieu khien deu bi disabled',
            locks.length === 3 && locks.every(Boolean), `disabled=${JSON.stringify(locks)}`);

        const lockBox = viewerDoc.getElementById('iot-lock');
        check('Khoa "Che do chi xem" duoc hien ra', lockBox && lockBox.hidden === false);
        check('Tieu de khoa la read-only',
            viewerDoc.getElementById('iot-lock-title').textContent.includes('Chế độ chỉ xem'),
            viewerDoc.getElementById('iot-lock-title').textContent.trim());

        const adminSection = viewerDoc.getElementById('iot-admin');
        check('Bang quan ly nguoi dung bi an voi viewer', adminSection && adminSection.hidden === true);

        const requestsBefore = viewer.requests.filter((item) => item.url.includes('/api/device/control')).length;
        const firstButton = viewerDoc.querySelector('#iot-actions .iot-btn');
        firstButton.dispatchEvent(new viewer.dom.window.MouseEvent('click', { bubbles: true }));
        await new Promise((resolve) => setTimeout(resolve, 150));
        const requestsAfter = viewer.requests.filter((item) => item.url.includes('/api/device/control')).length;
        check('Bam nut khi bi disabled khong gui lenh nao', requestsAfter === requestsBefore,
            `requests=${requestsAfter}`);

        const viewerControl = await postJson(port, '/api/device/control', { action: 'ON' }, viewerLogin.cookie);
        check('Goi thang API dieu khien bang viewer -> 403', viewerControl.response.status === 403,
            `status=${viewerControl.response.status}`);
        check('403 neu ro requiredPermission=device:control',
            viewerControl.json && viewerControl.json.requiredPermission === 'device:control',
            JSON.stringify(viewerControl.json));

        const viewerState = await fetch(`http://127.0.0.1:${port}/api/state`, {
            headers: { Cookie: viewerLogin.cookie },
        }).then((response) => response.json());
        check('Viewer van xem duoc thong so cam bien', Boolean(viewerState.state) &&
            'temperature' in viewerState.state && 'humidity' in viewerState.state,
            JSON.stringify(viewerState.state));
        check('Trang thai den khong bi viewer thay doi', viewerState.state.lamp === 'OFF',
            `lamp=${viewerState.state.lamp}`);

        // ----------------------------------------------------------- admin
        log('\n[2] Dang nhap "admin" va doi chieu');
        const adminLogin = await login(port, 'admin', 'Admin@123');
        check('admin dang nhap thanh cong -> 200', adminLogin.response.status === 200,
            `status=${adminLogin.response.status}`);

        const admin = await renderUi(port, adminLogin.cookie);
        doms.push(admin.dom);
        const adminDoc = admin.dom.window.document;
        const adminRendered = await waitFor(() => adminDoc.getElementById('iot-user-role').textContent !== '--');
        check('Dashboard render duoc phien admin', adminRendered);

        const adminRole = adminDoc.getElementById('iot-user-role').textContent.trim();
        check('Vai tro hien thi la "Quan tri vien"', adminRole === 'Quản trị viên', `role=${adminRole}`);

        const adminButtons = buttonStates(adminDoc);
        check('Nut dieu khien bat voi admin',
            adminButtons.length === 3 && adminButtons.every((value) => value === false),
            `disabled=${JSON.stringify(adminButtons)}`);
        check('Khoa read-only an voi admin', adminDoc.getElementById('iot-lock').hidden === true);

        const usersLoaded = await waitFor(() => adminDoc.querySelectorAll('#iot-users tr').length >= 2);
        check('Bang quan ly nguoi dung hien va co du lieu', usersLoaded,
            `rows=${adminDoc.querySelectorAll('#iot-users tr').length}`);
        check('Danh sach khong lo password hash',
            !adminDoc.getElementById('iot-admin').textContent.includes('scrypt$'));

        const adminControl = await postJson(port, '/api/device/control', { action: 'ON' }, adminLogin.cookie);
        check('admin dieu khien duoc -> 200', adminControl.response.status === 200,
            `status=${adminControl.response.status}`);
        check('Trang thai den = ON', adminControl.json && adminControl.json.lamp === 'ON',
            JSON.stringify(adminControl.json));

        const adminState = await fetch(`http://127.0.0.1:${port}/api/state`, {
            headers: { Cookie: adminLogin.cookie },
        }).then((response) => response.json());
        check('Admin doc lai thay den ON', adminState.state.lamp === 'ON', `lamp=${adminState.state.lamp}`);

        const audit = await fetch(`http://127.0.0.1:${port}/api/audit`, {
            headers: { Cookie: adminLogin.cookie },
        }).then((response) => response.json());
        check('Audit ghi nhan viewer bi tu choi 403',
            audit.entries.some((entry) => entry.action === 'access_denied' && entry.username === 'viewer'));
        check('Audit ghi nhan admin dieu khien thanh cong',
            audit.entries.some((entry) => entry.action === 'device_control' && entry.username === 'admin'));

        log('\n' + '='.repeat(72));
        log(`KET QUA E2E: ${passes} PASS, ${failures} FAIL`);
        log('='.repeat(72));
        log('Viewer dang nhap va xem duoc Dashboard nhung khong the dieu khien:');
        log('- Nut bi disabled ngay tren trinh duyet.');
        log('- Goi thang API van bi may chu tu choi 403.');
        log('- Admin van dieu khien binh thuong.');
        log(`Audit entries: ${rbac.auditEntries.length}`);
    } finally {
        for (const dom of doms) {
            try { dom.window.close(); } catch { /* ignore */ }
        }
        server.close();
    }

    fs.writeFileSync(path.join(EVIDENCE_DIR, 'e2e.log'), lines.join('\n') + '\n');
    // jsdom co the con timer/keep-alive socket; thoat tuong minh sau khi ghi log.
    process.exit(failures > 0 ? 1 : 0);
}

main().catch((error) => {
    console.error(error);
    process.exitCode = 1;
});
