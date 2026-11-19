// Kiểm thử offline: thực thi JS trong JSON và mô phỏng các action ThingsBoard.
// Không thay thế kiểm thử import, Rule Engine và SMTP trên server thật.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const chain = JSON.parse(fs.readFileSync(path.join(__dirname, 'rule_chain_bai1_export.json'), 'utf8'));
const {nodes, connections, firstNodeIndex} = chain.metadata;

for (const edge of connections) {
  assert.ok(nodes[edge.fromIndex] && nodes[edge.toIndex], 'Chỉ số node phải hợp lệ');
}

function execute(payload, state = {}, messageType = 'POST_TELEMETRY_REQUEST') {
  const result = {saved: 0, alarms: [], emails: [], relations: []};
  function walk(index, msg, metadata, msgType, visited = new Set()) {
    assert.ok(!visited.has(index), 'Rule chain không được có vòng lặp');
    const nextVisited = new Set(visited).add(index);
    const node = nodes[index];
    const config = node.configuration;
    let relation;
    if (node.type.endsWith('TbMsgTypeSwitchNode')) {
      relation = msgType === 'POST_TELEMETRY_REQUEST' ? 'Post telemetry' : 'Other';
    } else if (node.type.endsWith('TbMsgTimeseriesNode')) {
      result.saved++;
      relation = 'Success';
    } else if (node.type.endsWith('TbJsFilterNode')) {
      const filter = new Function('msg', 'metadata', 'msgType', config.jsScript);
      relation = filter(msg, metadata, msgType) ? 'True' : 'False';
    } else if (node.type.endsWith('TbTransformMsgNode')) {
      const transform = new Function('msg', 'metadata', 'msgType', config.jsScript);
      const output = transform(msg, metadata, msgType);
      ({msg, metadata, msgType} = output);
      relation = 'Success';
    } else if (node.type.endsWith('TbCreateAlarmNode')) {
      relation = state.alarm ? 'Updated' : 'Created';
      const details = new Function('msg', 'metadata', 'msgType', config.alarmDetailsBuildJs)(msg, metadata, msgType);
      assert.equal(config.alarmType, 'HighTemperatureAlert');
      assert.equal(config.useMessageAlarmData, false);
      state.alarm = {type: config.alarmType, severity: config.severity, details};
      result.alarms.push(structuredClone(state.alarm));
      // ThingsBoard thay telemetry bằng Alarm object ở đầu ra Create Alarm.
      msg = structuredClone(state.alarm);
      msgType = 'ALARM';
    } else if (node.type.endsWith('TbMsgToEmailNode')) {
      function render(template) {
        return template.replace(/\$\{([^}]+)\}/g, (_, key) => {
          assert.ok(Object.hasOwn(metadata, key), `Thiếu metadata email: ${key}`);
          return metadata[key];
        });
      }
      msg = {subject: render(config.subjectTemplate), body: render(config.bodyTemplate)};
      msgType = 'SEND_EMAIL';
      relation = 'Success';
    } else if (node.type.endsWith('TbSendEmailNode')) {
      assert.equal(msgType, 'SEND_EMAIL');
      result.emails.push(msg);
      relation = 'Success';
    } else {
      assert.fail(`Chưa hỗ trợ node: ${node.type}`);
    }
    result.relations.push([index, relation]);
    for (const edge of connections.filter(e => e.fromIndex === index && e.type === relation)) {
      walk(edge.toIndex, structuredClone(msg), structuredClone(metadata), msgType, nextVisited);
    }
  }
  walk(firstNodeIndex, payload, {deviceName: 'Sensor_Lab9'}, messageType);
  return result;
}

let count = 0;
for (const [value, severity] of [[-5, null], [29.9, null], [30, 'WARNING'], [32, 'WARNING'], [35, 'WARNING'], [35.0001, 'CRITICAL'], [40, 'CRITICAL']]) {
  const output = execute({temperature: value});
  assert.equal(output.saved, 1);
  assert.equal(output.alarms[0]?.severity ?? null, severity);
  assert.equal(output.emails.length, severity === 'CRITICAL' ? 1 : 0);
  if (severity) assert.equal(output.alarms[0].details.temperature, value);
  if (severity === 'CRITICAL') {
    assert.ok(output.emails[0].body.includes(`${value} C`));
    assert.ok(output.emails[0].subject.includes('Sensor_Lab9'));
  }
  count++;
}
for (const payload of [{}, {humidity: 60}, {temperature: '40'}, {temperature: null}, {temperature: true}, {temperature: []}, {temperature: {}}, {temperature: NaN}, {temperature: Infinity}]) {
  const output = execute(payload);
  assert.equal(output.alarms.length, 0);
  assert.equal(output.emails.length, 0);
  count++;
}
const state = {};
execute({temperature: 32}, state);
const escalated = execute({temperature: 40}, state);
assert.equal(state.alarm.severity, 'CRITICAL');
assert.ok(escalated.relations.some(([, relation]) => relation === 'Updated'));
assert.equal(escalated.emails.length, 1, 'Warning -> Critical phải gửi email qua Updated');
assert.equal(execute({temperature: 41}, state).emails.length, 1);
const cooled = execute({temperature: 25}, state);
assert.equal(cooled.alarms.length, 0);
assert.equal(cooled.emails.length, 0);
assert.equal(state.alarm.severity, 'CRITICAL', 'Bài 1 chưa tự động clear Alarm');
const attributes = execute({temperature: 40}, {}, 'POST_ATTRIBUTES_REQUEST');
assert.equal(attributes.saved, 0);
assert.equal(attributes.alarms.length, 0);
assert.equal(attributes.emails.length, 0);
console.log(`PASS: ${count} trường hợp nhiệt độ + nâng mức, Critical lặp lại, Normal sau Alarm, loại bản tin và template email.`);
