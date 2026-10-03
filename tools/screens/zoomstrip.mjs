// The zoom strip (A11.12, PRE-22): the camp stop's place and heading on the demo area, 16:30 with time held, zoomed
// from 0.10 to 0.34 in steps of 0.01, each step against the last: the share of art pixels it changes where they
// show, and of those that crawled (A11.10), recorded for the note; every step must draw.
// Usage: node tools/screens/zoomstrip.mjs [--bench <bench json>]   (after tools/build-web.sh; the numbers go in the
// bench file under "zoom_strip")
import { readFileSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { ROOT, launch, open, serve } from './lib.mjs';

const benchAt = process.argv.indexOf('--bench');
const bench = benchAt > 0 ? path.resolve(process.argv[benchAt + 1]) : null;
const [FROM, TO, STEP] = [0.1, 0.34, 0.01];
const STEPS = Math.round((TO - FROM) / STEP);

let failed = 0;
const check = (name, ok, detail = '') => {
  if (!ok) failed += 1;
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${name}${detail ? ` (${detail})` : ''}`);
};
const share = (x) => Number(x.toFixed(4));

const server = await serve(path.join(ROOT, 'dist', 'web'));
const browser = await launch();
let counts;
try {
  const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 270, height: 601, scale: 4 });
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  // checks: PRE-22
  counts = await page.evaluate(
    ({ from, to, step }) => {
      window.kd.hour(4);
      window.kd.golden('valley-camp');
      window.kd.frame(1);
      return window.kd.zoomstrip({ from, to, step });
    },
    { from: FROM, to: TO, step: STEP },
  );
  await page.evaluate(() => window.kd.golden(''));
  check(`every one of the ${STEPS} steps counted`, counts.changed.length === STEPS && counts.pixels > 0);
  check('no page errors', errors.length === 0, errors.slice(0, 3).join('; '));
  await close();
} finally {
  await browser.close();
  server.close();
}

const changed = counts.changed.map((c) => share(c / counts.pixels));
const crawl = counts.crawl.map((c) => share(c / counts.pixels));
for (let k = 0; k < changed.length; k++) {
  const [a, b] = [FROM + STEP * k, FROM + STEP * (k + 1)];
  console.log(`       ${a.toFixed(2)} to ${b.toFixed(2)}: ${(100 * changed[k]).toFixed(1)}% changed, ${(100 * crawl[k]).toFixed(2)}% crawled`);
}
if (bench && !failed) {
  const d = JSON.parse(readFileSync(bench, 'utf8'));
  d.zoom_strip = {
    from: FROM,
    to: TO,
    step: STEP,
    changed,
    crawl,
    how: "each zoom step against the last at the camp stop's place and heading: the shares of the art target's pixels, less a border of two, that changed where they show and that crawled (A11.10); the demo area at 16:30, the phone's 270 x 601 art pixels, headless Chromium with SwiftShader (tools/screens/zoomstrip.mjs)",
  };
  writeFileSync(bench, `${JSON.stringify(d, null, 2)}\n`);
  console.log(`       written to ${path.relative(ROOT, bench)}`);
}
console.log(failed ? `Zoom strip: FAIL (${failed})` : 'Zoom strip: PASS');
process.exit(failed ? 1 : 0);
