// B66: headless smoke test of drawing-test.html. Runs a shortened benchmark (VIS-14) and drives every
// PRE-33 gesture with synthetic touches. Speed here means nothing (WebGL on the CPU); this only checks the wiring.
// Usage: node tools/smoke.mjs
import { createRequire } from 'module';
import path from 'path';
import { fileURLToPath } from 'url';

const require = createRequire(import.meta.url);
const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const { chromium } = require(process.env.PLAYWRIGHT_PATH || '/opt/node-tools/node_modules/playwright');
const exe = process.env.CHROMIUM_PATH || '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';

const browser = await chromium.launch({ executablePath: exe, args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'] });
const ctx = await browser.newContext({ viewport: { width: 412, height: 860 }, deviceScaleFactor: 1, hasTouch: true, isMobile: true });
const page = await ctx.newPage();
const errors = [];
page.on('pageerror', (e) => errors.push(e.message));
page.on('console', (m) => { if (m.type() === 'error' && !/Failed to load resource/.test(m.text())) errors.push(m.text().slice(0, 300)); });
await page.goto('file://' + path.join(root, 'drawing-test.html'));
await page.waitForFunction(() => document.documentElement.dataset.viewReady === '1', null, { timeout: 120000 });
await page.waitForFunction(() => !document.getElementById('bench-run').disabled, null, { timeout: 300000 });

const results = [];
const check = (name, ok, info) => { results.push({ name, ok }); console.log((ok ? 'ok   ' : 'FAIL ') + name + (info ? '  ' + info : '')); };

// ---- benchmark, shortened to about 6 seconds
await page.evaluate(() => { window.__b66.bench().scale = 0.1; });
await page.click('#bench-run');
await page.waitForFunction(() => document.getElementById('bench-code').value.length > 0, null, { timeout: 180000 });
const code = await page.$eval('#bench-code', (e) => e.value);
const rowsN = await page.$$eval('#bench-table tbody tr', (r) => r.length);
check('benchmark produces a code', /^B66\.1 (PASS|FAIL) hz=/.test(code), '\n     ' + code);
check('benchmark fills the table', rowsN === 8, rowsN + ' rows');
check('benchmark leaves the full screen', !(await page.$eval('#view', (v) => v.classList.contains('play'))));

// ---- gestures
await page.click('#play');
await page.waitForTimeout(400);
check('gesture test fills the screen', await page.$eval('#view', (v) => v.classList.contains('play')));
const cdp = await ctx.newCDPSession(page);
const touch = (type, pts) => cdp.send('Input.dispatchTouchEvent', { type, touchPoints: pts.map(([x, y], id) => ({ x, y, id, radiusX: 4, radiusY: 4, force: 1 })) });
const S = () => page.evaluate(() => ({ yaw: window.__view.S.yaw, zoom: window.__view.S.zoom, pan: window.__view.S.pan.slice() }));
const shown = (id) => page.$eval('#' + id, (e) => !e.hidden);
const sleep = (ms) => page.waitForTimeout(ms);
const W = 412, H = 860, cx = W / 2, cy = H * 0.45;
async function drag(from, to, steps = 8, holdMs = 0) {
  await touch('touchStart', [from]);
  if (holdMs) await sleep(holdMs);
  for (let i = 1; i <= steps; i++) { await touch('touchMove', [[from[0] + (to[0] - from[0]) * i / steps, from[1] + (to[1] - from[1]) * i / steps]]); await sleep(16); }
  await touch('touchEnd', []);
}

// a brief touch shows the time control; a tap selects
await sleep(700);
await touch('touchStart', [[cx, cy]]); await sleep(60); await touch('touchEnd', []);
await sleep(150);
check('brief touch shows date, speed and time control', await shown('hud'), await page.$eval('#hud .txt', (e) => e.textContent.trim()));
check('tap selects (stub highlight)', await shown('mark'), await page.$eval('#toast', (e) => e.textContent));

// drag moves the camera
await sleep(400);
let a = await S();
await drag([cx, cy], [cx + 80, cy + 60]);
let b = await S();
check('drag moves the camera', Math.hypot(b.pan[0] - a.pan[0], b.pan[1] - a.pan[1]) > 0.5 && Math.abs(b.yaw - a.yaw) < 1e-6, `pan ${a.pan.map((v) => v.toFixed(2))} -> ${b.pan.map((v) => v.toFixed(2))}`);
await sleep(800);

// twist with two fingers turns: rotate the pair 40 degrees clockwise on screen
a = await S();
const r0 = 90;
const pair = (ang, r = r0) => [[cx + r * Math.cos(ang), cy + r * Math.sin(ang)], [cx - r * Math.cos(ang), cy - r * Math.sin(ang)]];
await touch('touchStart', pair(0));
for (let i = 1; i <= 10; i++) { await touch('touchMove', pair((40 * Math.PI / 180) * i / 10)); await sleep(16); }
await touch('touchEnd', []);
await sleep(600);
b = await S();
check('twist turns (world follows the fingers)', b.yaw - a.yaw > 0.3, `yaw ${a.yaw.toFixed(3)} -> ${b.yaw.toFixed(3)} (40 deg twist, 7 deg dead zone, then eases)`);

// pinch zooms: spread the fingers to twice the distance -> zoom in (smaller zoom value)
a = await S();
await touch('touchStart', pair(Math.PI / 2, 60));
for (let i = 1; i <= 10; i++) { await touch('touchMove', pair(Math.PI / 2, 60 + 60 * i / 10)); await sleep(16); }
await touch('touchEnd', []);
await sleep(300);
b = await S();
check('pinch zooms in', a.zoom - b.zoom > 0.08, `zoom ${a.zoom.toFixed(3)} -> ${b.zoom.toFixed(3)}`);

// double-tap and drag down with one thumb zooms in (start from the valley so there is room to zoom in)
await sleep(500);
await page.evaluate(() => { window.__view.S.zoom = 0.4; });
a = await S();
await touch('touchStart', [[cx, cy]]); await sleep(50); await touch('touchEnd', []);
await sleep(120);
await drag([cx, cy], [cx, cy + 200], 10);
await sleep(200);
b = await S();
check('double-tap and drag zooms', a.zoom - b.zoom > 0.1, `zoom ${a.zoom.toFixed(3)} -> ${b.zoom.toFixed(3)}`);

// long-press opens the powers menu, with "Draw an area"
await sleep(500);
await touch('touchStart', [[cx, cy + 40]]); await sleep(750); await touch('touchEnd', []);
await sleep(100);
const menuOpen = await shown('menu');
check('long-press opens the powers menu', menuOpen, await page.$eval('#menu', (e) => [...e.querySelectorAll('button')].map((x) => x.textContent).join(' / ')));
if (menuOpen) {
  await page.click('[data-power="draw"]');
  a = await S();
  const ring = []; for (let i = 0; i <= 16; i++) ring.push([cx + 70 * Math.cos(i / 16 * 2 * Math.PI), cy + 70 * Math.sin(i / 16 * 2 * Math.PI)]);
  await touch('touchStart', [ring[0]]);
  for (const p of ring.slice(1)) { await touch('touchMove', [p]); await sleep(12); }
  await touch('touchEnd', []);
  b = await S();
  await sleep(100);
  check('"Draw an area" draws instead of moving', /Area drawn/.test(await page.$eval('#toast', (e) => e.textContent)) && Math.abs(b.pan[0] - a.pan[0]) < 1e-9, await page.$eval('#toast', (e) => e.textContent));
}

// swipe up from the bottom edge opens the views sheet
await sleep(2500);
await drag([cx, H - 30], [cx + 5, H - 160], 8);
await sleep(150);
check('swipe up from the bottom edge opens views', await shown('sheet'));
await page.click('#play-exit');
await sleep(300);
check('leaving the test restores the page', !(await page.$eval('#view', (v) => v.classList.contains('play'))));

// each pixel fix draws without errors
for (const f of ['steps', 'fade', 'majority', 'sticky', 'base']) {
  await page.click(`[data-fix="${f}"]`);
  await page.evaluate(() => { window.__view.S.yaw += 0.05; window.__view.render(); });
  await sleep(150);
}
check('no page errors', errors.length === 0, errors.join(' | '));
console.log(results.every((r) => r.ok) ? '\nALL OK' : '\nSOME FAILED');
await browser.close();
