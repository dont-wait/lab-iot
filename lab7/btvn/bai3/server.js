'use strict';

/**
 * Diem khoi dong Node-RED cho Lab 7 - Bai 3.
 *
 * Ly do phai co file nay thay vi chay `node-red --settings settings.js`:
 * tu Node-RED 4/5, `httpNodeMiddleware` chi ap dung cho tung node http-in,
 * khong con la middleware toan cuc. De chan /ui va /api/* bang phan quyen,
 * ta nhung Node-RED vao mot Express app va gan rbac.middleware truoc
 * RED.httpNode (noi Dashboard va cac node http-in duoc mount).
 *
 * Chay: npm start   (hoac: node server.js)
 */

const http = require('node:http');
const express = require('express');
const RED = require('node-red');

const settings = require('./settings.js');

const rbac = settings.functionGlobalContext.rbac;
if (!rbac || typeof rbac.middleware !== 'function') {
    throw new Error('settings.js phai tao functionGlobalContext.rbac de khoi dong server.');
}

const app = express();
const server = http.createServer(app);

// Khoi tao Node-RED voi cung server nay.
RED.init(server, settings);

// 1) Lop phan quyen cua bai 3 phai dung TRUOC moi router cua Node-RED.
//    Ly do: httpAdminRoot = '/' nen RED.httpAdmin se nhan moi request; dat
//    rbac.middleware truoc giup /login, /api/*, /ui khong bi editor "nuot"
//    request (truoc day POST /api/rbac/login bi treo vi bi RED.httpAdmin chan).
//    Cac path khac duoc rbac bo qua bang next() de den editor/dashboard.
app.use(rbac.middleware);

// 2) Editor (/red, /settings, ...) - duoc bao ve boi adminAuth trong settings.js.
app.use(settings.httpAdminRoot, RED.httpAdmin);

// 3) Cac route do flow tao ra va Dashboard (/ui).
app.use(settings.httpNodeRoot, RED.httpNode);

server.listen(settings.uiPort, settings.uiHost, () => {
    const host = settings.uiHost || '127.0.0.1';
    console.log(`[bai3] Dashboard: http://${host}:${settings.uiPort}/ui`);
    console.log(`[bai3] Dang nhap:  http://${host}:${settings.uiPort}/login`);
    console.log('[bai3] Tai khoan demo: admin / Admin@123 (quản trị), viewer / Viewer@123 (chỉ xem)');
});

RED.start().catch((error) => {
    console.error('[bai3] Khong khoi dong duoc Node-RED:', error);
    process.exitCode = 1;
});

function shutdown(signal) {
    console.log(`[bai3] Nhan ${signal}, dang dung...`);
    RED.stop().then(() => server.close(() => process.exit(0))).catch(() => process.exit(1));
}

process.on('SIGINT', () => shutdown('SIGINT'));
process.on('SIGTERM', () => shutdown('SIGTERM'));
