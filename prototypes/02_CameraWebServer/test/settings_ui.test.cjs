const {test} = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const script = fs.readFileSync(path.join(__dirname, '../web/camera_settings.js'), 'utf8');
const reply = (ok, body) => ({ok, json: async () => body});

async function page(fetch) {
  const elements = new Map();
  const events = {};
  const element = id => {
    if (!elements.has(id)) elements.set(id, {
      id, disabled: id.endsWith('settings'), style: {}, textContent: '', events: {},
      addEventListener(name, callback) { this.events[name] = callback; },
    });
    return elements.get(id);
  };
  element('menu').prepend = () => {};
  element('menu').querySelectorAll = () => [element('save-settings'), element('restore-settings'), element('quality')];
  element('framesize').options = [{value:'6'}, {value:'10'}];
  const location = {href:'http://camera/', origin:'http://camera', reload() { location.reloaded = true; }};
  const context = {
    URL, window: {fetch}, location,
    document: {getElementById: element, createElement: () => ({style:{}}), addEventListener: (name, fn) => { events[name] = fn; }},
    sessionStorage: {getItem: () => null, setItem() {}, removeItem() {}},
  };
  vm.runInNewContext(script, context);
  await events.DOMContentLoaded();
  return {context, element, click: id => element(id).events.click()};
}

test('Save waits for an outstanding slider request and disables controls while saving', async () => {
  let finishControl;
  const calls = [];
  const p = await page(async url => {
    calls.push(url);
    if (url === '/settings') return reply(true, {saved:false, max_framesize:6});
    if (url.includes('/control')) return new Promise(resolve => { finishControl = resolve; });
    return reply(true, {message:'Saved'});
  });
  p.context.window.fetch('/control?var=quality&val=14');
  const saving = p.click('save-settings');
  assert.equal(p.element('quality').disabled, true);
  assert.equal(calls.includes('/settings/save'), false);
  finishControl(reply(true, {}));
  await saving;
  assert.equal(calls.filter(url => url === '/settings/save').length, 1);
  assert.equal(p.element('settings-message').textContent, 'Saved');
  assert.equal(p.element('quality').disabled, false);
  assert.equal(p.element('framesize').options[1].disabled, true);
});

test('A rejected setting prevents a misleading Save until that setting succeeds', async () => {
  let accepted = false;
  let saves = 0;
  const p = await page(async url => {
    if (url === '/settings') return reply(true, {saved:false, max_framesize:6});
    if (url.includes('/control')) return reply(accepted, {});
    saves++;
    return reply(true, {message:'Saved'});
  });
  await p.context.window.fetch('/control?var=quality&val=99');
  await p.click('save-settings');
  assert.equal(saves, 0);
  assert.match(p.element('settings-message').textContent, /not applied/);
  accepted = true;
  await p.context.window.fetch('/control?var=quality&val=12');
  await p.click('save-settings');
  assert.equal(saves, 1);
});

test('Storage failure is visible and leaves the buttons usable for retry', async () => {
  const p = await page(async url => url === '/settings'
    ? reply(true, {saved:false, max_framesize:6})
    : reply(false, {message:'Storage failed'}));
  await p.click('save-settings');
  assert.equal(p.element('settings-message').textContent, 'Storage failed');
  assert.equal(p.element('save-settings').disabled, false);
  assert.equal(p.element('restore-settings').disabled, false);
});
