// The web build's smoke test (A15.4, A15.11): the page loads without errors, an art pixel is exactly 4 × 4
// device pixels in portrait, in landscape and on a screen of scale 2, the test card's bar moves one art pixel
// a frame, and the core's maths and draws give the cloud's bits in the browser (A15.9 item 5), its block green. (Chromium's emulated fractional scales, like the phone's 2.625, misreport the canvas's device size, so
// they are left to the phone itself.) Usage: node tools/screens/smoke.mjs [--save <dir>]   (after tools/build-web.sh)
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

const isWhite = ([r, g, b]) => r >= 250 && g >= 250 && b >= 250;
const isBlack = ([r, g, b]) => r <= 5 && g <= 5 && b <= 5;
const isBar = ([r, g, b]) => r >= 220 && g >= 110 && g <= 170 && b <= 60;
const isGreen = ([r, g, b]) => g >= 150 && r <= 100 && b <= 120;
// The cloud's hashes of the core's probes, and the self-check block's middle in device pixels (card::CORE_X 64,
// CORE_Y 24, 32 × 32 art pixels).
const storedCore = readFileSync(path.join(ROOT, 'crates', 'kd-core', 'tests', 'fixtures', 'hashes.txt'), 'utf8');
const coreMiddle = [4 * (64 + 16) + 2, 4 * (24 + 16) + 2];

// checks: PRE-22 PLT-02
// The checker of single art pixels: 32 × 32 art pixels, each exactly 4 × 4 device pixels of one colour,
// alternating, white at its top-left.
function checkerOk(img) {
  let x0 = Infinity, y0 = Infinity, x1 = -1, y1 = -1;
  // Its 32 art pixels from art pixel 24 (card::MARGIN) lie within 4 × 60 device pixels of the corner (the top row
  // of art pixels may show in part); the grey steps from art row 64 hold black and white too, so they are left out.
  for (let y = 0; y < Math.min(img.height, 4 * 60); y++) {
    for (let x = 0; x < Math.min(img.width, 4 * 60); x++) {
      const c = img.at(x, y);
      if (isWhite(c) || isBlack(c)) {
        x0 = Math.min(x0, x); y0 = Math.min(y0, y); x1 = Math.max(x1, x); y1 = Math.max(y1, y);
      }
    }
  }
  const w = x1 - x0 + 1, h = y1 - y0 + 1;
  if (w !== 128 || h !== 128) return [false, `checker is ${w} × ${h} device pixels, not 128 × 128`];
  for (let j = 0; j < 32; j++) {
    for (let i = 0; i < 32; i++) {
      const white = (i + j) % 2 === 0;
      for (let dy = 0; dy < 4; dy++) {
        for (let dx = 0; dx < 4; dx++) {
          const c = img.at(x0 + 4 * i + dx, y0 + 4 * j + dy);
          if (white ? !isWhite(c) : !isBlack(c)) return [false, `art pixel (${i}, ${j}) is not one colour over 4 × 4`];
        }
      }
    }
  }
  return [true, `at device pixel ${x0}, ${y0}`];
}

// The bar's columns in device pixels.
function barColumns(img) {
  let x0 = Infinity, x1 = -1;
  for (let y = 0; y < img.height; y++) {
    for (let x = 0; x < img.width; x++) {
      if (isBar(img.at(x, y))) { x0 = Math.min(x0, x); x1 = Math.max(x1, x); }
    }
  }
  return [x0, x1];
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
      await page.evaluate(() => window.kd.frame(1));
      const first = await page.screenshot();
      writeFileSync(path.join(outDir, `${label.replace(' ', '-')}.png`), first);
      const a = decodePng(first);
      const [ok, why] = checkerOk(a);
      check(`art pixel 4x4, ${label}`, ok, why);
      const block = a.at(...coreMiddle);
      check(`core block green, ${label}`, isGreen(block), `colour ${block.slice(0, 3).join(', ')}`);
      await page.evaluate(() => window.kd.frame(1));
      const b = decodePng(await page.screenshot());
      const [ax0, ax1] = barColumns(a);
      const [bx0, bx1] = barColumns(b);
      check(`bar moves a pixel a frame, ${label}`, ax1 - ax0 === 3 && bx1 - bx0 === 3 && bx0 - ax0 === 4,
        `columns ${ax0}–${ax1}, then ${bx0}–${bx1}`);
    }
    check(`no page errors, ${label}`, errors.length === 0, errors.slice(0, 3).join('; '));
    await close();
  }
} finally {
  await browser.close();
  server.close();
}
console.log(failed ? `Smoke: FAIL (${failed})` : 'Smoke: PASS');
process.exit(failed ? 1 : 0);
