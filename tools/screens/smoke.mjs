// The alpha page's smoke test (A15.4, A15.11): loads dist/web with ?test=1, checks the cube draws and turns, and that
// the picture is pixel art: 4 x 4 blocks of one colour, every colour in the current palette row (PRE-01, PRE-22).
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

  // After the drag the version strip shows too, so the UI pass is checked with the world.
  await page.waitForTimeout(200);
  const shot2 = await pixels(page, await page.screenshot());
  const st = await page.$eval('#status', (e) => { const r = e.getBoundingClientRect(); return [r.left, r.top, r.right, r.bottom]; });
  const inStatus = (x, y) => x >= st[0] - 1 && x <= st[2] + 1 && y >= st[1] - 1 && y <= st[3] + 1;
  const at = (x, y) => (y * shot2.width + x) * 4;

  // checks: PRE-22 PRE-01
  // Every whole 4 x 4 block of screen pixels is one colour, for one alignment of the grid (the upscale may shift it).
  let best = null;
  for (let py = 0; py < 4 && !best; py++) {
    for (let px0 = 0; px0 < 4 && !best; px0++) {
      let bad = 0, blocks = 0;
      for (let by = py; by + 4 <= shot2.height; by += 4) {
        for (let bx = px0; bx + 4 <= shot2.width; bx += 4) {
          if (inStatus(bx, by) || inStatus(bx + 3, by + 3)) continue;
          blocks++;
          const i0 = at(bx, by);
          let same = true;
          for (let y = by; y < by + 4 && same; y++) {
            for (let x = bx; x < bx + 4; x++) {
              const i = at(x, y);
              if (shot2.data[i] !== shot2.data[i0] || shot2.data[i + 1] !== shot2.data[i0 + 1] || shot2.data[i + 2] !== shot2.data[i0 + 2]) { same = false; break; }
            }
          }
          if (!same) bad++;
        }
      }
      if (bad === 0) best = `${blocks} blocks, grid at (${px0}, ${py})`;
    }
  }
  check('pixel grid', best !== null, best || 'no alignment makes every 4 x 4 block one colour');

  // checks: PRE-01 PRE-30
  const pal = await page.evaluate(() => window.kd.palette());
  const allowed = new Set();
  for (let k = 0; k + 2 < pal.length; k += 3) allowed.add((pal[k] << 16) | (pal[k + 1] << 8) | pal[k + 2]);
  let off = 0, first = '';
  for (let y = 0; y < shot2.height; y++) {
    for (let x = 0; x < shot2.width; x++) {
      if (inStatus(x, y)) continue;
      const i = at(x, y), c = (shot2.data[i] << 16) | (shot2.data[i + 1] << 8) | shot2.data[i + 2];
      if (!allowed.has(c)) { if (!off) first = `#${c.toString(16).padStart(6, '0')} at ${x},${y}`; off++; }
    }
  }
  check('palette only', allowed.size > 0 && off === 0, off ? `${off} pixels off the palette, first ${first}` : `${allowed.size} colours in the row`);

  // checks: PRC-11
  check('no page errors', errors.length === 0, errors.join(' | '));
} finally {
  await browser.close();
  await srv.close();
}
const pass = results.every(Boolean);
console.log(pass ? 'Smoke: PASS' : 'Smoke: FAIL');
process.exit(pass ? 0 : 1);
