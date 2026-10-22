'use strict';

/**
 * Tao flows.json cho Node-RED tu cac file HTML/CSS trong ui/.
 *
 * Ly do: template dashboard la HTML/JS dai, viet truc tiep trong flows.json se
 * rat kho doc va kho sua. Chay `node tools/build_flows.js` moi khi sua ui/*.
 */

const fs = require('node:fs');
const path = require('node:path');

const ROOT = path.join(__dirname, '..');
const read = (file) => fs.readFileSync(path.join(ROOT, 'ui', file), 'utf8');
const css = read('dashboard.css');
const sensorPanel = read('sensor_panel.html');
const controlPanel = read('control_panel.html');

const styleTemplate = `<style>\n${css}\n</style>`;

const flows = [
    {
        id: 'bai3-tab',
        type: 'tab',
        label: 'Bài 3 · Phân quyền Dashboard',
        disabled: false,
        info: [
            '# Bài 3 - Phân quyền truy cập và chia sẻ Dashboard',
            '',
            'Dashboard giám sát IoT có hai vai trò:',
            '- admin: xem + điều khiển + quản lý người dùng.',
            '- viewer: chỉ xem thông số, nút điều khiển bị vô hiệu hóa.',
            '',
            'Xác thực và phân quyền nằm ở rbac.js (settings.js).',
            'Template chỉ hiển thị theo quyền do /api/rbac/me trả về;',
            'mọi lệnh điều khiển đều bị kiểm tra lại ở máy chủ.',
        ].join('\n'),
    },
    {
        id: 'ui-base',
        type: 'ui_base',
        name: 'RBAC Dashboard',
        theme: {
            name: 'theme-source',
            lightTheme: {
                default: '#334155',
                baseColor: '#ffffff',
                primaryColor: '#0ea5e9',
                pageBackgroundColor: '#f1f5f9',
                headerColor: '#0f172a',
                groupTextColor: '#0f172a',
                groupBackgroundColor: '#ffffff',
                groupBorderColor: '#dbe3ee',
                widgetTextColor: '#0f172a',
                widgetBackgroundColor: '#ffffff',
                widgetAccentColor: '#0ea5e9',
                widgetBorderColor: '#dbe3ee',
            },
            darkTheme: {
                default: '#cbd5e1',
                baseColor: '#0b1220',
                primaryColor: '#38bdf8',
                pageBackgroundColor: '#0b1220',
                headerColor: '#020617',
                groupTextColor: '#e2e8f0',
                groupBackgroundColor: '#111c2e',
                groupBorderColor: '#1e293b',
                widgetTextColor: '#e2e8f0',
                widgetBackgroundColor: '#111c2e',
                widgetAccentColor: '#38bdf8',
                widgetBorderColor: '#1e293b',
            },
        },
        site: {
            name: 'Lab 7 · Phân quyền Dashboard',
            hideToolbar: 'false',
            hideStatus: 'false',
            hidePoweredBy: 'true',
            hideTabbar: 'false',
            pagesAsTabs: 'false',
            showPageTitle: true,
        },
        sizes: { sx: 32, sy: 32, gx: 6, gy: 6, cx: 6, cy: 6, px: 0, py: 0 },
        sidebar: {
            name: 'menu',
            icon: 'menu',
            order: -1,
            sizing: '100%',
            width: '250px',
            dismissable: true,
            show: false,
        },
        allowSwipe: true,
        lockMenu: false,
        allowTempTheme: true,
        dateFormat: 'DD/MM/YYYY',
    },
    {
        id: 'ui-tab-monitor',
        type: 'ui_tab',
        name: 'Giám sát phòng khách',
        icon: 'dashboard',
        order: 1,
        disabled: false,
        hidden: false,
    },
    {
        id: 'ui-group-sensor',
        type: 'ui_group',
        name: '🌡️ Thông số cảm biến',
        tab: 'ui-tab-monitor',
        order: 1,
        disp: true,
        width: '12',
        collapse: false,
        className: 'sensor-area',
    },
    {
        id: 'ui-group-control',
        type: 'ui_group',
        name: '🎛️ Điều khiển & phân quyền',
        tab: 'ui-tab-monitor',
        order: 2,
        disp: true,
        width: '12',
        collapse: false,
        className: 'control-area',
    },
    {
        id: 'template-style',
        type: 'ui_template',
        z: 'bai3-tab',
        group: 'ui-group-sensor',
        name: 'CSS giao diện',
        order: 0,
        width: 12,
        height: 0,
        format: styleTemplate,
        storeOutMessages: true,
        fwdInMessages: true,
        resendOnRefresh: false,
        templateScope: 'local',
        className: '',
        x: 180,
        y: 80,
        wires: [[]],
    },
    {
        id: 'template-sensor',
        type: 'ui_template',
        z: 'bai3-tab',
        group: 'ui-group-sensor',
        name: 'Bảng thông số (mọi vai trò)',
        order: 1,
        width: 12,
        height: 0,
        format: sensorPanel,
        storeOutMessages: true,
        fwdInMessages: true,
        resendOnRefresh: false,
        templateScope: 'local',
        className: '',
        x: 180,
        y: 160,
        wires: [[]],
    },
    {
        id: 'template-control',
        type: 'ui_template',
        z: 'bai3-tab',
        group: 'ui-group-control',
        name: 'Điều khiển + quản lý người dùng',
        order: 1,
        width: 12,
        height: 0,
        format: controlPanel,
        storeOutMessages: true,
        fwdInMessages: true,
        resendOnRefresh: false,
        templateScope: 'local',
        className: '',
        x: 180,
        y: 240,
        wires: [[]],
    },
    // ---------------------------------------------------------------
    // Cau noi that giua RBAC server va MQTT: moi giay lay cac lenh
    // admin vua tao trong commandQueue (rbac.drainCommands) va publish
    // len broker. Nho vay viewer bi 403 tu truoc khi lenh vao hang doi.
    // ---------------------------------------------------------------
    {
        id: 'broker-hivemq',
        type: 'mqtt-broker',
        name: 'HiveMQ public',
        broker: process.env.MQTT_BROKER || 'broker.hivemq.com',
        port: process.env.MQTT_PORT || '1883',
        clientid: '',
        autoConnect: true,
        usetls: false,
        protocolVersion: '4',
        keepalive: '60',
        cleansession: true,
        birthTopic: '',
        birthQos: '0',
        birthPayload: '',
        closeTopic: '',
        closeQos: '0',
        closePayload: '',
        willTopic: '',
        willQos: '0',
        willPayload: '',
    },
    {
        id: 'queue-inject',
        type: 'inject',
        z: 'bai3-tab',
        name: 'Mỗi giây',
        props: [{ p: 'payload' }],
        repeat: '1',
        crontab: '',
        once: true,
        onceDelay: '0.5',
        topic: '',
        payload: '',
        payloadType: 'date',
        x: 140,
        y: 360,
        wires: [['queue-function']],
    },
    {
        id: 'queue-function',
        type: 'function',
        z: 'bai3-tab',
        name: 'Hàng đợi lệnh MQTT',
        func: [
            "'use strict';",
            "var rbac = global.get('rbac');",
            "if (!rbac || typeof rbac.drainCommands !== 'function') { return null; }",
            'var commands = rbac.drainCommands(10);',
            'if (!commands.length) { return null; }',
            "node.log('[rbac] publish ' + commands.length + ' lenh len MQTT');",
            'return commands.map(function (command) {',
            '    return { topic: command.topic, payload: command.payload, qos: 0, retain: false };',
            '});',
        ].join('\n'),
        outputs: 1,
        timeout: 0,
        noerr: 0,
        initialize: '',
        finalize: '',
        libs: [],
        x: 380,
        y: 360,
        wires: [['mqtt-out-control']],
    },
    {
        id: 'mqtt-out-control',
        type: 'mqtt out',
        z: 'bai3-tab',
        name: 'Gửi lệnh điều khiển',
        topic: '',
        qos: '0',
        retain: 'false',
        broker: 'broker-hivemq',
        x: 640,
        y: 360,
        wires: [],
    },
];

function validate(entries) {
    const ids = new Set();
    for (const entry of entries) {
        if (!entry.id) { throw new Error('Node thieu id'); }
        if (ids.has(entry.id)) { throw new Error('Node id trung: ' + entry.id); }
        ids.add(entry.id);
    }
}

validate(flows);

const target = path.join(ROOT, 'flows.json');
fs.writeFileSync(target, JSON.stringify(flows, null, 2) + '\n');
console.log(`Da tao ${path.relative(process.cwd(), target)} voi ${flows.length} node.`);
