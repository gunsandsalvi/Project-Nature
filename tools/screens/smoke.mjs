// The alpha page's smoke test (A15.4, A15.11): loads dist/web with ?test=1, checks the valley draws, that a pan of one
// art pixel moves the picture exactly 4 screen pixels, B66's gestures (drag, twist, pinch, double-tap drag, tap), and
// that the picture is pixel art: 4 x 4 blocks of one colour, every colour in the current palette row (PRE-01, PRE-22,
// PRE-33). From pretests/b66-drawing/tools/smoke.mjs, cut to the alpha page. Usage: node tools/screens/smoke.mjs [--save]
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
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  await page.waitForTimeout(500);
  const png = await page.screenshot();
  const version = /^versionName=(.*)$/m.exec(fs.readFileSync(path.join(root, 'android/version.properties'), 'utf8'))[1].trim();
  const out = process.argv.includes('--save') ? path.join(root, 'results/screens', version) : path.join(root, 'target/screens/smoke');
  fs.mkdirSync(out, { recursive: true });
  fs.writeFileSync(path.join(out, 'valley.png'), png);
  const px = await pixels(page, png);
  let differ = 0, n = 0;
  for (let y = Math.floor(px.height / 3); y < Math.floor(2 * px.height / 3); y++) {
    for (let x = Math.floor(px.width / 3); x < Math.floor(2 * px.width / 3); x++) {
      const i = (y * px.width + x) * 4;
      n++;
      if (Math.abs(px.data[i] - 0x0d) > 2 || Math.abs(px.data[i + 1] - 0x0b) > 2 || Math.abs(px.data[i + 2] - 0x14) > 2) differ++;
    }
  }
  check('ground drawn', differ / n >= 0.9, `${(100 * differ / n).toFixed(1)}% of the middle third differs from #0d0b14`);

  const st0 = await page.$eval('#status', (e) => { const r = e.getBoundingClientRect(); return [r.left, r.top, r.right, r.bottom]; });
  const nearStatus = (x, y) => x >= st0[0] - 6 && x <= st0[2] + 6 && y >= st0[1] - 6 && y <= st0[3] + 6;

  // checks: PRE-22
  // A pan of one art pixel to the right moves the whole picture exactly 4 screen pixels left (A11.2's test): the art
  // grid stays on the ground, both inside an area and where the pan moves the floating origin to the next area's
  // corner (at x 128 m). A drag keeps the target's height, so the pan here does too.
  const pan = async (x0) => {
    await page.evaluate((x) => window.kd.camera({ x, y: 150, yaw: 0, zoom: 0.2 }), x0);
    let c = await page.evaluate(() => window.kd.camera());
    // the target mid-pixel first, so rounding the move to whole ticks of 1/256 m cannot cross a pixel's edge
    c = await page.evaluate((c) => window.kd.camera({ x: c.x + (0.5 - c.fx) * c.texel, y: c.y, z: c.z, yaw: c.yaw, zoom: c.zoom }), c);
    await page.evaluate(() => window.kd.frame(2));
    const A = await pixels(page, await page.screenshot());
    await page.evaluate((c) => window.kd.camera({ x: c.x + c.texel, y: c.y, z: c.z, yaw: c.yaw, zoom: c.zoom }), c);
    await page.evaluate(() => window.kd.frame(2));
    const B = await pixels(page, await page.screenshot());
    let bad = 0, n = 0;
    for (let y = 0; y < A.height; y++) {
      for (let x = 0; x + 4 < A.width; x++) {
        if (nearStatus(x, y) || nearStatus(x + 4, y)) continue;
        const a = (y * A.width + x + 4) * 4, b = (y * B.width + x) * 4;
        n++;
        if (A.data[a] !== B.data[b] || A.data[a + 1] !== B.data[b + 1] || A.data[a + 2] !== B.data[b + 2]) bad++;
      }
    }
    return { bad, n, x: c.x, texel: c.texel };
  };
  const inside = await pan(100);
  const across = await pan(128 - inside.texel / 2);
  check('pan stays crisp', inside.n > 0 && inside.bad === 0 && across.bad === 0,
    `${inside.bad} and ${across.bad} of ${inside.n} pixels not where a 4-pixel shift puts them, from x ${inside.x.toFixed(2)} and ${across.x.toFixed(2)} m (texel ${inside.texel.toFixed(3)} m)`);

  // checks: PRE-33
  // B66's gesture checks, with touch-type pointer events on the canvas; each starts from the same view.
  const W = 412, H = 860, cx = W / 2, cy = H * 0.45;
  const cam = () => page.evaluate(() => window.kd.camera());
  const reset = (zoom = 0.2) => page.evaluate((z) => window.kd.camera({ x: 128, y: 118, yaw: 3.74, zoom: z }), zoom);
  // one call dispatches a list of steps, each a pointer event kind and its points [id, x, y], 16 ms apart
  const gesture = (steps) => page.evaluate(async (list) => {
    const c = document.getElementById('cv');
    for (const [kind, pts] of list) {
      for (const [id, x, y] of pts) {
        c.dispatchEvent(new PointerEvent(kind, { pointerId: id, pointerType: 'touch', isPrimary: id === 1, clientX: x, clientY: y, buttons: kind === 'pointerup' ? 0 : 1, bubbles: true, cancelable: true }));
      }
      await new Promise((r) => setTimeout(r, 16));
    }
  }, steps);
  const line = (id, from, to, k = 10) => {
    const s = [['pointerdown', [[id, ...from]]]];
    for (let i = 1; i <= k; i++) s.push(['pointermove', [[id, from[0] + (to[0] - from[0]) * i / k, from[1] + (to[1] - from[1]) * i / k]]]);
    s.push(['pointerup', [[id, ...to]]]);
    return s;
  };
  const pair = (ang, r) => [[1, cx + r * Math.cos(ang), cy + r * Math.sin(ang)], [2, cx - r * Math.cos(ang), cy - r * Math.sin(ang)]];
  const two = (from, to, k = 10) => {
    const s = [['pointerdown', from(0)]];
    for (let i = 1; i <= k; i++) s.push(['pointermove', from(i / k)]);
    s.push(['pointerup', to]);
    return s;
  };

  await reset();
  let a = await cam();
  await gesture(line(1, [cx, cy], [cx + 80, cy + 60]));
  let b = await cam();
  check('drag moves the camera', Math.hypot(b.x - a.x, b.y - a.y) > 0.5 && b.yaw === a.yaw, `(${a.x.toFixed(2)}, ${a.y.toFixed(2)}) -> (${b.x.toFixed(2)}, ${b.y.toFixed(2)})`);

  await reset();
  a = await cam();
  await gesture(two((t) => pair((40 * Math.PI / 180) * t, 90), pair(40 * Math.PI / 180, 90)));
  b = await cam();
  check('twist turns', b.yaw - a.yaw > 0.3, `yaw ${a.yaw.toFixed(3)} -> ${b.yaw.toFixed(3)} (a 40-degree twist)`);

  await reset();
  a = await cam();
  await gesture(two((t) => pair(Math.PI / 2, 60 + 60 * t), pair(Math.PI / 2, 120)));
  b = await cam();
  check('pinch zooms in', a.zoom - b.zoom > 0.08, `zoom ${a.zoom.toFixed(3)} -> ${b.zoom.toFixed(3)}`);

  // checks: PRE-34
  // one thumb zooms: double-tap, then drag (portrait, one-handed)
  await reset(0.35);
  a = await cam();
  await gesture([['pointerdown', [[1, cx, cy]]], ['pointerup', [[1, cx, cy]]], ...line(1, [cx, cy], [cx, cy + 200])]);
  b = await cam();
  check('double-tap and drag zooms', a.zoom - b.zoom > 0.1, `zoom ${a.zoom.toFixed(3)} -> ${b.zoom.toFixed(3)}`);

  await reset();
  a = await cam();
  await gesture([['pointerdown', [[1, cx, cy]]], ['pointerup', [[1, cx + 2, cy + 1]]]]);
  b = await cam();
  check('a tap is no drag', b.x === a.x && b.y === a.y && b.zoom === a.zoom && b.yaw === a.yaw, `(${b.x.toFixed(2)}, ${b.y.toFixed(2)})`);

  // After the gestures the version strip shows too, so the UI pass is checked with the world.
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
