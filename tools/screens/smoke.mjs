// The web build's smoke test (A15.4, A15.11): the page loads without errors, an art pixel is exactly 4 × 4
// device pixels in portrait, in landscape and on a screen of scale 2, the core's maths and draws give the cloud's
// bits in the browser (A15.9 item 5), the probe scene's steps equal the Rust twins' (A11.13 rule 2), and a panic
// leaves its message in the status line (A3.8). (Chromium's emulated fractional scales, like the phone's 2.625,
// misreport the canvas's device size, so they are left to the phone itself.)
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

// The cloud's hashes of the core's probes.
const storedCore = readFileSync(path.join(ROOT, 'crates', 'kd-core', 'tests', 'fixtures', 'hashes.txt'), 'utf8');

// checks: PRE-22 PLT-02 PRE-01
// Every art pixel is exactly 4 × 4 device pixels of one colour: each 4 × 4 block of the grid is one colour, down to
// the last whole row. The grid starts at the screen's top-left corner, for the art and the UI alike (A12.1); the
// picture must also hold more than a few colours, so a blank page cannot pass.
function blocksOk(img, bottom) {
  const same = (a, b) => a[0] === b[0] && a[1] === b[1] && a[2] === b[2];
  const y0 = 0;
  const colours = new Set();
  let blocks = 0;
  for (let by = y0; by + 4 <= bottom; by += 4) {
    for (let bx = 0; bx + 4 <= img.width; bx += 4) {
      const c = img.at(bx, by);
      colours.add(c.slice(0, 3).join(','));
      blocks += 1;
      for (let dy = 0; dy < 4; dy++) {
        for (let dx = 0; dx < 4; dx++) {
          if (!same(img.at(bx + dx, by + dy), c)) return [false, `art pixel at device pixel ${bx}, ${by} is not one colour`];
        }
      }
    }
  }
  if (colours.size < 10) return [false, `only ${colours.size} colours`];
  return [true, `${blocks} art pixels, ${colours.size} colours`];
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
      check(`probe equals the twins, ${label}`, probe && probe.gpu.length === 256 && wrong === 0,
        probe ? `${wrong} of ${probe.gpu.length} differ` : 'no probe');
      await page.evaluate(() => window.kd.frame(1));
      const shot = await page.screenshot();
      writeFileSync(path.join(outDir, `${label.replace(' ', '-')}.png`), shot);
      const bottom = await page.evaluate(() => {
        const status = document.getElementById('status');
        const h = Math.round(innerHeight * devicePixelRatio);
        const top = status.textContent ? Math.floor(status.getBoundingClientRect().top * devicePixelRatio) : h;
        return Math.min(top, h - (h % 4));
      });
      const [ok, why] = blocksOk(decodePng(shot), bottom);
      check(`art pixel 4x4, ${label}`, ok, why);
    }
    check(`no page errors, ${label}`, errors.length === 0, errors.slice(0, 3).join('; '));
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
