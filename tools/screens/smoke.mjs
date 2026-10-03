// The web build's smoke test (A15.4, A15.11): the page loads without errors, an art pixel and a UI pixel are each
// exactly 4 × 4 device pixels in portrait, in landscape and on a screen of scale 2, the core's maths and draws and
// the demo area's ground give the cloud's bits in the browser (A15.9 item 5, the self-check), the probe scene's
// steps, surfaces, looks, outlines, haze levels and relief and the palette rows equal the Rust twins' and the cloud's
// (A11.13 rule 2), a still camera's frames are the same, the hour lights the ground, a one-art-pixel pan moves the
// picture by exactly 4 device pixels inside a block and across a move of the floating origin (A11.2), each gesture
// does what it should and nothing else, two fingers keep the land under them through a pinch and a twist (A12.2),
// and a panic leaves its message in the status line (A3.8). (Chromium's emulated fractional scales, like the
// phone's 2.625, misreport the canvas's device size, so they are left to the phone itself.)
// Usage: node tools/screens/smoke.mjs [--save <dir>]   (after tools/build-web.sh)
// Screenshots go to target/screens/smoke/, which is never committed, or to --save's folder.
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { ROOT, decodePng, launch, open, serve } from './lib.mjs';

const saveAt = process.argv.indexOf('--save');
const outDir = saveAt > 0 ? path.resolve(process.argv[saveAt + 1]) : path.join(ROOT, 'target', 'screens', 'smoke');
mkdirSync(outDir, { recursive: true });

let failed = 0;
const check = (name, ok, detail = '') => {
  if (!ok) failed += 1;
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${name}${detail ? ` (${detail})` : ''}`);
};

// The cloud's hashes of the core's probes, and its palette row at each of the app's hours.
const storedCore = readFileSync(path.join(ROOT, 'crates', 'kd-core', 'tests', 'fixtures', 'hashes.txt'), 'utf8');
const storedRows = readFileSync(path.join(ROOT, 'tests', 'golden', 'palette.txt'), 'utf8').trim().split('\n')
  .map((line) => line.split(' ').slice(1).join(' '));

// checks: PRE-22 PLT-02 PRE-01
// Every art pixel is exactly 4 × 4 device pixels of one colour: each 4 × 4 block of the grid is one colour, in the
// whole rows from `top` to `bottom`. The camera shifts the art's grid by whole device pixels as it pans (A11.2), so
// its phase is found, not assumed; the UI's grid is `fixed` to the screen's top-left corner (A12.1). The rows must
// also hold at least `least` colours, so a blank page cannot pass.
function blocksOk(img, top, bottom, { fixed = false, least = 10 } = {}) {
  const same = (a, b) => a[0] === b[0] && a[1] === b[1] && a[2] === b[2];
  let first = '';
  for (let phase = 0; phase < (fixed ? 1 : 16); phase++) {
    const [x0, y0] = [phase % 4, top + Math.floor(phase / 4)];
    const colours = new Set();
    let blocks = 0;
    let broken = '';
    for (let by = y0; by + 4 <= bottom && !broken; by += 4) {
      for (let bx = x0; bx + 4 <= img.width && !broken; bx += 4) {
        const c = img.at(bx, by);
        colours.add(c.slice(0, 3).join(','));
        blocks += 1;
        for (let d = 0; d < 16 && !broken; d++) {
          if (!same(img.at(bx + (d % 4), by + Math.floor(d / 4)), c)) broken = `device pixel ${bx}, ${by}`;
        }
      }
    }
    if (!broken) {
      if (colours.size < least) return [false, `only ${colours.size} colours`];
      return [true, `${blocks} blocks from device pixel ${x0}, ${y0}, ${colours.size} colours`];
    }
    first ||= broken;
  }
  return [false, `no grid of 4 × 4 blocks fits; the first broken block at ${first}`];
}

// A screenshot of the page, decoded.
async function screen(page) {
  const url = await page.evaluate(() => window.kd.shot());
  return decodePng(Buffer.from(url.split(',')[1], 'base64'));
}

// checks: PRE-22
// The picture after a pan equals the one before moved `dx` device pixels to the left: every pixel of the overlap,
// in its first `rows` rows.
function shifted(before, after, dx, rows = after.height) {
  let differ = 0;
  let first = '';
  for (let y = 0; y < rows; y++) {
    for (let x = 0; x + dx < after.width; x++) {
      const [p, q] = [after.at(x, y), before.at(x + dx, y)];
      if (p[0] !== q[0] || p[1] !== q[1] || p[2] !== q[2]) {
        if (!differ) first = ` first at ${x}, ${y}`;
        differ += 1;
      }
    }
  }
  return differ ? `${differ} pixels differ;${first}` : '';
}

// The camera moved east so the target's place on the screen's grid is `a` art pixels from the world's corner (the
// view looks north, so east is the screen's right).
async function aimAt(page, start, a) {
  const cam = await page.evaluate(({ t, a }) => {
    const c = window.kd.camera();
    const now = c.view.corner[0] + c.view.off[0];
    const x = c.target[0] + Math.round((a - now) * c.view.texel * 256);
    window.kd.aim([x, t[1], t[2]], 0, c.zoom);
    window.kd.frame(1);
    return window.kd.camera();
  }, { t: start, a });
  return cam;
}

const server = await serve(path.join(ROOT, 'dist', 'web'));
const browser = await launch();
try {
  for (const [label, width, height, scale] of [
    ['portrait', 412, 915, 1],
    ['landscape', 915, 412, 1],
    ['scale 2', 412, 915, 2],
  ]) {
    const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width, height, scale });
    // checks: PRC-11
    let loaded = true;
    try {
      await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
    } catch {
      loaded = false;
    }
    check(`page loads, ${label}`, loaded, loaded ? `art target ${(await page.evaluate(() => window.kd.artSize())).join(' × ')}` : 'kd.ready() never came');
    if (loaded) {
      const report = await page.evaluate(() => (document.getElementById('code').hidden ? '' : document.getElementById('code-title').textContent));
      check(`self-check passes, ${label}`, report === '', report);
      // checks: RES-05 TIM-16
      const core = await page.evaluate(() => window.kd.core());
      const differ = core.split('\n').filter((line) => line && !storedCore.split('\n').includes(line));
      check(`core hashes equal, ${label}`, core === storedCore, differ.length ? `differ: ${differ.join('; ')}` : '');
      // checks: PRE-20 PRE-01
      const probe = await page.evaluate(() => window.kd.probe());
      const wrong = probe ? probe.gpu.filter((g, i) => g !== probe.twins[i]).length : -1;
      check(`probe equals the twins, ${label}`, probe && probe.gpu.length === 1024 && wrong === 0,
        probe ? `${wrong} of ${probe.gpu.length} differ` : 'no probe');
      // checks: PRE-30 RES-05
      const rows = await page.evaluate((n) => {
        const out = [];
        for (let h = 0; h < n; h++) {
          window.kd.hour(h);
          window.kd.frame(1);
          out.push(window.kd.palette());
        }
        window.kd.hour(4);
        return out;
      }, storedRows.length);
      const hours = rows.map((r, h) => (r === storedRows[h] ? -1 : h)).filter((h) => h >= 0);
      check(`palette row equals the cloud's, ${label}`, rows.length === 8 && hours.length === 0,
        hours.length ? `hours ${hours.join(', ')} differ` : '8 hours');
      // A tap on the land shows the strip for its 3 seconds (PRE-32), so the screenshot holds it whole.
      await page.evaluate(() => {
        const t = performance.now();
        window.kd.touch(0, 9, 8, 8, t);
        window.kd.touch(2, 9, 8, 8, t + 10);
        window.kd.frame(1);
      });
      const shot = await page.screenshot();
      writeFileSync(path.join(outDir, `${label.replace(' ', '-')}.png`), shot);
      // The art above the strip, with a row to spare; and the strip, the screen's last 24 whole UI pixels, which
      // keeps the UI's own grid from the screen's top-left corner (A12.1).
      const band = await page.evaluate(() => {
        const status = document.getElementById('status');
        const h = Math.round(innerHeight * devicePixelRatio);
        const top = status.textContent ? Math.floor(status.getBoundingClientRect().top * devicePixelRatio) : h;
        const foot = h - (h % 4);
        return { bottom: Math.min(top, foot - 4 * 28), strip: [foot - 4 * 24, Math.min(top, foot)] };
      });
      const img = decodePng(shot);
      const [ok, why] = blocksOk(img, 0, band.bottom);
      check(`art pixel 4x4, ${label}`, ok, why);
      const [stripOk, stripWhy] = blocksOk(img, band.strip[0], band.strip[1], { fixed: true, least: 3 });
      check(`strip pixel 4x4 from the top-left, ${label}`, stripOk, stripWhy);
      // checks: PRE-22
      // With the camera still, the picture holds still: frames half a second apart are the same above the strip.
      // (This takes the place of α01a's turning block: the ground, unlike the light card, moves only when asked.)
      const still = await screen(page);
      await page.waitForTimeout(500);
      const stirred = shifted(still, await screen(page), 0, band.bottom);
      check(`a still camera holds still, ${label}`, stirred === '', stirred);
      // checks: PRE-30 PRE-20
      // The ground is lit by the hour: noon's picture differs from late afternoon's above the strip, whose own line
      // names the hour.
      const late = await screen(page);
      await page.evaluate(() => window.kd.hour(2));
      const noon = await screen(page);
      await page.evaluate(() => window.kd.hour(4));
      const lit = shifted(late, noon, 0, band.bottom);
      check(`the hour lights the ground, ${label}`, lit !== '', lit ? '' : 'noon looks like 16:30 above the strip');
    }
    check(`no page errors, ${label}`, errors.length === 0, errors.slice(0, 3).join('; '));
    await close();
  }
  // checks: PRE-22 PRE-02
  // A one-art-pixel pan at the close camp stop moves the picture by exactly 4 device pixels: inside a block, where
  // the projection stays put and only the viewport moves, and across a block's edge where the floating origin
  // moves too (A11.2). Each pan starts with the art grid's phase well clear of a device pixel's middle.
  {
    const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 412, height: 915, scale: 1 });
    await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
    const start = await page.evaluate(() => {
      window.kd.golden('valley-camp');
      window.kd.frame(1);
      const c = window.kd.camera();
      window.kd.aim([c.target[0] - 60 * 256, c.target[1], c.target[2]], 0, 0.14);
      window.kd.frame(1);
      return window.kd.camera();
    });
    const pan = async (a) => {
      const from = await aimAt(page, start.target, a);
      const before = await screen(page);
      const to = await aimAt(page, start.target, a + 1);
      const after = await screen(page);
      return { from, to, why: shifted(before, after, 4) };
    };
    const block = start.view.block[0];
    const inside = await pan(block + 100.3);
    check('pan stays crisp inside a block', inside.why === '' && inside.from.view.block[0] === inside.to.view.block[0],
      inside.why || `${(inside.to.view.corner[0] - inside.from.view.corner[0])} art pixels of corner`);
    // Block edges every 512 art pixels to the east, until one where the origin moves.
    let across = null;
    for (let k = 1; k <= 8 && !across; k++) {
      const edge = block + 512 * k;
      const a = await aimAt(page, start.target, edge + 0.3);
      const b = await aimAt(page, start.target, edge + 1.3);
      if (a.view.origin.join() !== b.view.origin.join()) across = await pan(edge + 0.3);
    }
    check('pan stays crisp across a move of the floating origin',
      across !== null && across.why === '' && across.from.view.block[0] !== across.to.view.block[0],
      across ? across.why || `origin ${across.from.view.origin} to ${across.to.view.origin}` : 'no edge moved the origin');
    check('no page errors, pans', errors.length === 0, errors.slice(0, 3).join('; '));
    await close();
  }

  // checks: PRE-33 PRE-34
  // Synthetic touches: each gesture does what it should and nothing else (A12.2).
  {
    const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 412, height: 915, scale: 1 });
    await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
    let t = 10000;
    // Touches, each [kind, pointer, x, y] a 10 ms tick apart, with pauses written as numbers of milliseconds.
    const touch = async (steps) => {
      const list = [];
      for (const s of steps) {
        if (typeof s === 'number') t += s;
        else {
          t += 10;
          list.push([...s, t]);
        }
      }
      t += 1000;
      return page.evaluate((events) => {
        for (const [k, id, x, y, at] of events) window.kd.touch(k, id, x, y, at);
        window.kd.frame(1);
        return { cam: window.kd.camera(), hour: window.kd.palette() };
      }, list);
    };
    const state = () => page.evaluate(() => { window.kd.frame(1); return { cam: window.kd.camera(), hour: window.kd.palette() }; });
    const same = (a, b) => Math.abs(a - b) < 1e-6;
    const moved = (a, b) => a.cam.target[0] !== b.cam.target[0] || a.cam.target[1] !== b.cam.target[1];
    await page.evaluate(() => window.kd.golden(''));
    let before = await state();
    let after = await touch([[0, 1, 200, 400], [2, 1, 201, 401]]);
    check('a tap on the land moves nothing', !moved(before, after) && same(before.cam.zoom, after.cam.zoom)
      && before.hour === after.hour);
    before = after;
    after = await touch([[0, 1, 200, 900], [2, 1, 200, 900]]);
    check('a tap on the strip steps the hour, and moves nothing', !moved(before, after) && before.hour !== after.hour);
    before = after;
    after = await touch([[0, 1, 200, 400], [1, 1, 230, 420], [1, 1, 260, 440], [1, 1, 290, 460], 300, [2, 1, 290, 460]]);
    check('a drag moves the land, and only that', moved(before, after) && same(before.cam.zoom, after.cam.zoom)
      && same(before.cam.yaw, after.cam.yaw));
    before = after;
    const spread = [[0, 1, 150, 450], [0, 2, 250, 450]];
    for (let k = 1; k <= 10; k++) spread.push([1, 1, 150 - 6 * k, 450], [1, 2, 250 + 6 * k, 450]);
    spread.push(300, [2, 1, 90, 450], [2, 2, 310, 450]);
    after = await touch(spread);
    check('a pinch zooms, and turns nothing', after.cam.zoom < before.cam.zoom - 0.01 && same(before.cam.yaw, after.cam.yaw),
      `zoom ${before.cam.zoom.toFixed(3)} to ${after.cam.zoom.toFixed(3)}`);
    before = after;
    const twist = [[0, 1, 120, 450], [0, 2, 280, 450]];
    for (let k = 1; k <= 10; k++) {
      const r = (3 * k * Math.PI) / 180;
      twist.push([1, 1, 200 - 80 * Math.cos(r), 450 - 80 * Math.sin(r)], [1, 2, 200 + 80 * Math.cos(r), 450 + 80 * Math.sin(r)]);
    }
    twist.push(300, [2, 1, 120, 450], [2, 2, 280, 450]);
    after = await touch(twist);
    check('a twist turns, and zooms nothing', !same(before.cam.yaw, after.cam.yaw) && same(before.cam.zoom, after.cam.zoom),
      `heading ${before.cam.yaw.toFixed(3)} to ${after.cam.yaw.toFixed(3)}`);
    before = after;
    after = await touch([[0, 1, 200, 400], [2, 1, 200, 400], 100, [0, 1, 202, 402], [1, 1, 202, 430], [1, 1, 202, 560], 300, [2, 1, 202, 560]]);
    const want = before.cam.zoom - (0.8 * (560 - 430)) / 915;
    check('a double-tap drag down zooms in, and turns nothing', Math.abs(after.cam.zoom - Math.max(0, want)) < 1e-3
      && same(before.cam.yaw, after.cam.yaw), `zoom ${after.cam.zoom.toFixed(4)}, asked ${want.toFixed(4)}`);

    // checks: PRE-33 PRE-22
    // Two fingers through a pinch and a twist together: once both are taken up (each past its threshold, from where
    // the fingers are then), the ground under each finger stays under it within half an art pixel, 2 device pixels.
    await page.evaluate(() => { window.kd.golden('valley-camp'); window.kd.golden(''); window.kd.frame(1); });
    const fingers = [[140, 380], [280, 560]];
    const placeAt = (k) => {
      const r = (2.5 * k * Math.PI) / 180;
      const s = 1 + 0.03 * k;
      const mid = [210 + 2 * k, 470 - k];
      return fingers.map(([x, y]) => {
        const [dx, dy] = [(x - 210) * s, (y - 470) * s];
        return [mid[0] + dx * Math.cos(r) - dy * Math.sin(r), mid[1] + dx * Math.sin(r) + dy * Math.cos(r)];
      });
    };
    const moves = (k0, k1) => {
      const out = [];
      for (let k = k0; k <= k1; k++) {
        const p = placeAt(k);
        out.push([1, 1, ...p[0]], [1, 2, ...p[1]]);
      }
      return out;
    };
    const ground = await page.evaluate(({ start, first, f }) => {
      let at = 50000;
      for (const [k, id, x, y] of [...start, ...first]) window.kd.touch(k, id, x, y, (at += 10));
      window.kd.frame(1);
      return f.map((p) => window.kd.groundAt(p[0], p[1]));
    }, { start: [[0, 1, ...fingers[0]], [0, 2, ...fingers[1]]], first: moves(1, 4), f: placeAt(4) });
    const last = placeAt(12);
    const shown = await page.evaluate(({ events, g }) => {
      let at = 60000;
      for (const [k, id, x, y] of events) window.kd.touch(k, id, x, y, (at += 10));
      window.kd.frame(1);
      return g.map((p) => window.kd.screenOf(p));
    }, { events: moves(5, 12), g: ground });
    const off = shown.map((p, i) => Math.hypot(p[0] - last[i][0], p[1] - last[i][1]));
    check('land under the fingers', off.every((d) => d <= 2), `${off.map((d) => d.toFixed(2)).join(', ')} device pixels`);
    check('no page errors, gestures', errors.length === 0, errors.slice(0, 3).join('; '));
    await close();
  }

  // checks: PRC-11
  // A panic stops the instance, since WebAssembly cannot unwind here; its message stays in the status line.
  const { page, close } = await open(browser, `${server.url}?test=1`, { width: 412, height: 915, scale: 1 });
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  const thrown = await page.evaluate(() => {
    try {
      window.kd.crash();
      return '';
    } catch (e) {
      return String(e);
    }
  });
  const line = await page.evaluate(() => document.getElementById('status').textContent);
  check('a panic shows in the status line', thrown !== '' && line.startsWith('Kindling stopped:') && line.includes('a test panic'), line);
  await close();
} finally {
  await browser.close();
  server.close();
}
console.log(failed ? `Smoke: FAIL (${failed})` : 'Smoke: PASS');
process.exit(failed ? 1 : 0);
