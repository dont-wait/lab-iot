'use strict';

/**
 * Them / doi mat khau tai khoan trong users.json.
 *
 * Vi du:
 *   node tools/add_user.js --username viewer2 --name "Người xem 2" \
 *       --role viewer --password 'Viewer2@123'
 *
 * Role hop le: admin, viewer, gateway.
 */

const fs = require('node:fs');
const path = require('node:path');
const { hashPassword, VALID_ROLES } = require('../rbac.js');

function parseArgs(argv) {
    const args = {};
    for (let index = 0; index < argv.length; index += 1) {
        const token = argv[index];
        if (!token.startsWith('--')) { continue; }
        const key = token.slice(2);
        const value = argv[index + 1];
        if (value === undefined || value.startsWith('--')) {
            args[key] = true;
        } else {
            args[key] = value;
            index += 1;
        }
    }
    return args;
}

const args = parseArgs(process.argv.slice(2));
const required = ['username', 'role', 'password'];
for (const key of required) {
    if (!args[key] || args[key] === true) {
        console.error(`Thieu --${key}. Vi du: node tools/add_user.js --username viewer2 --name "Nguoi xem" --role viewer --password 'Viewer2@123'`);
        process.exit(1);
    }
}
if (!VALID_ROLES.includes(args.role)) {
    console.error(`Role khong hop le: ${args.role}. Chon mot trong: ${VALID_ROLES.join(', ')}`);
    process.exit(1);
}
if (String(args.password).length < 8) {
    console.error('Mat khau phai dai it nhat 8 ky tu.');
    process.exit(1);
}

const usersFile = path.join(__dirname, '..', 'users.json');
let document;
try {
    document = JSON.parse(fs.readFileSync(usersFile, 'utf8'));
} catch (error) {
    console.error(`Khong doc duoc ${usersFile}: ${error.message}`);
    process.exit(1);
}
const users = Array.isArray(document) ? document : document.users;
const username = String(args.username).trim();
const existing = users.find((user) => user.username.toLowerCase() === username.toLowerCase());

if (existing) {
    existing.name = args.name || existing.name;
    existing.role = args.role;
    existing.password_hash = hashPassword(args.password);
    existing.updated_at = new Date().toISOString();
    console.log(`Da cap nhat tai khoan ${username} (${args.role}).`);
} else {
    users.push({
        username,
        name: args.name || username,
        role: args.role,
        created_at: new Date().toISOString(),
        password_hash: hashPassword(args.password),
    });
    console.log(`Da tao tai khoan ${username} (${args.role}).`);
}

fs.writeFileSync(usersFile, JSON.stringify(document, null, 2) + '\n');
