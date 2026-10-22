'use strict';

/**
 * Cau hinh Node-RED cho Lab 7 - Bai 3.
 *
 * Chay: node-red --settings settings.js --userDir ./.node-red
 * (xem scripts/run_node_red.sh de cai dat va chay tu dong)
 */

const path = require('node:path');

const { createRbac, loadOrCreateSecret } = require(path.join(__dirname, 'rbac.js'));

const USERS_FILE = path.join(__dirname, 'users.json');
const SECRET_FILE = path.join(__dirname, '.rbac-secret');
const AUDIT_FILE = path.join(__dirname, 'evidence', 'audit.jsonl');

// Secret ky cookie phien: uu tien bien moi truong, neu khong co thi tao file
// rieng (mode 600, da nam trong .gitignore).
const sessionSecret = process.env.RBAC_SESSION_SECRET || loadOrCreateSecret(SECRET_FILE);

// Mat khau cua tai khoan admin tren Blynk/Node-RED editor: Admin@123
// Hash bcrypt duoc tao boi: node-red-admin hash-pw
// Tai khoan "viewer" KHONG co trong danh sach nay nen khong the mo editor.
const ADMIN_EDITOR_HASH = '$2a$10$XYWYajLp88VP/uekPdmWiOrJ09PxejHrSnLyld0oyO6yhinmvyfSy';

const rbac = createRbac({
    usersFile: USERS_FILE,
    sessionSecret,
    siteName: 'Lab 7 · Dashboard giám sát IoT',
    auditFile: process.env.RBAC_AUDIT_FILE || AUDIT_FILE,
    secureCookie: process.env.RBAC_SECURE_COOKIE === '1',
    onCommand: (command) => {
        // Lenh da duoc ghi vao commandQueue; flow "Day lenh xuong MQTT" se lay
        // va publish. Log o day de de theo doi khi chay thu.
        console.log(
            `[rbac] ${command.user.username} (${command.role}) -> ${command.action} => lamp ${command.lamp}`
        );
    },
});

module.exports = {
    uiPort: Number(process.env.PORT) || 1880,
    uiHost: process.env.HOST || '127.0.0.1',
    userDir: process.env.NODE_RED_USER_DIR || path.join(__dirname, '.node-red'),
    flowFile: path.join(__dirname, 'flows.json'),
    credentialSecret: process.env.NODE_RED_CREDENTIAL_SECRET || 'lab7-bai3-rbac-credential-secret',

    // Chi admin duoc mo trinh soan flow. Viewer chi vao /ui theo phan quyen.
    adminAuth: {
        type: 'credentials',
        users: [
            {
                username: 'admin',
                password: ADMIN_EDITOR_HASH,
                permissions: '*',
            },
        ],
    },

    // Dashboard 3.x ho tro middleware rieng cho /ui. Lop chan chinh nam o
    // server.js (rbac.middleware truoc RED.httpNode); middleware nay bao ve
    // them cac tai nguyen tinh cua Dashboard neu truy cap truc tiep.
    ui: {
        path: 'ui',
        middleware: rbac.middleware,
    },

    // Cho flow truy cap doi tuong rbac de day lenh MQTT va doc trang thai.
    functionGlobalContext: {
        rbac,
    },

    logging: {
        console: {
            level: 'info',
            metrics: false,
            audit: false,
        },
    },

    editorTheme: {
        page: {
            title: 'Lab 7 · RBAC Dashboard',
        },
        header: {
            title: 'Lab 7 · Dashboard giám sát IoT',
        },
        projects: {
            enabled: false,
        },
    },

    // Cac thiet lap mac dinh con lai cua Node-RED.
    httpAdminRoot: '/',
    httpNodeRoot: '/',
    functionExternalModules: false,
    exportGlobalContextKeys: false,
    debugMaxLength: 1000,
    apiMaxLength: '5mb',
};
