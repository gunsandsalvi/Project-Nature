// The alpha page's smoke test (A15.4, A15.11): loads dist/web with ?test=1, checks the cube draws and turns.
// From pretests/b66-drawing/tools/smoke.mjs, cut to the alpha page. Usage: node tools/screens/smoke.mjs [--save]
// The screenshot goes to target/screens/smoke/ (not committed), or with --save to results/screens/<versionName>/,
// so a plain run never changes a committed file (α00's review).
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { launch, serve, pixels } from './lib.mjs';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const results = [];
const check = (name, ok, info) => { results.push(ok); console.log((ok ? 'ok   ' : 'FAIL ') + name + (info ? '  ' + info : '')); };

const srv = await serve(path.join(root, 'dist/web'));
const { browser, page, errors } = await launch();
try {
  await page.goto(srv.url + '/?test=1');
  // checks: PRC-11
  let loaded = true;
  try {
    await page.waitForFunction(() => document.getElementById('status').textContent.includes('running'), null, { timeout: 60000 });
  } catch (e) { loaded = false; }
  const status = await page.$eval('#status', (e) => e.textContent);
  check('page loads', loaded, status);
  // checks: RES-05
  check('core OK', status.includes('core OK'), 'the wasm leg of the cross-target check');

  // checks: PRC-11
  await page.waitForTimeout(1500);
  const png = await page.screenshot();
  const version = /^versionName=(.*)$/m.exec(fs.readFileSync(path.join(root, 'android/version.properties'), 'utf8'))[1].trim();
  const out = process.argv.includes('--save') ? path.join(root, 'results/screens', version) : path.join(root, 'target/screens/smoke');
  fs.mkdirSync(out, { recursive: true });
  fs.writeFileSync(path.join(out, 'cube.png'), png);
  const px = await pixels(page, png);
  let differ = 0, n = 0;
  for (let y = Math.floor(px.height / 3); y < Math.floor(2 * px.height / 3); y++) {
    for (let x = Math.floor(px.width / 3); x < Math.floor(2 * px.width / 3); x++) {
      const i = (y * px.width + x) * 4;
      n++;
      if (Math.abs(px.data[i] - 0x0d) > 2 || Math.abs(px.data[i + 1] - 0x0b) > 2 || Math.abs(px.data[i + 2] - 0x14) > 2) differ++;
    }
  }
  check('cube drawn', differ / n >= 0.02, `${(100 * differ / n).toFixed(1)}% of the middle third differs from #0d0b14`);

  // checks: PRC-11
  // Touch-type pointer events on the canvas: a 200-pixel drag to the left.
  const yaw0 = await page.evaluate(() => window.kd.yaw());
  await page.evaluate(async () => {
    const c = document.getElementById('cv');
    const fire = (k, x) => c.dispatchEvent(new PointerEvent(k, { pointerId: 11, pointerType: 'touch', isPrimary: true, clientX: x, clientY: 430, buttons: k === 'pointerup' ? 0 : 1, bubbles: true, cancelable: true }));
    fire('pointerdown', 306);
    for (let i = 1; i <= 10; i++) { fire('pointermove', 306 - 20 * i); await new Promise((r) => setTimeout(r, 16)); }
    fire('pointerup', 106);
  });
  const yaw1 = await page.evaluate(() => window.kd.yaw());
  check('drag turns the cube', Math.abs(yaw1 - yaw0) > 1, `yaw ${yaw0.toFixed(3)} -> ${yaw1.toFixed(3)}`);

  // checks: PRC-11
  check('no page errors', errors.length === 0, errors.join(' | '));
} finally {
  await browser.close();
  await srv.close();
}
const pass = results.every(Boolean);
console.log(pass ? 'Smoke: PASS' : 'Smoke: FAIL');
process.exit(pass ? 0 : 1);
