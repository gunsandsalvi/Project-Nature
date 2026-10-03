// The zoom strip (A11.12, PRE-22): from the camp and close camp stops' places and headings on the demo area, 16:30
// with time held, zoomed from 0.10 to 0.34 in steps of 0.01, each step against the last: the share of art pixels it
// changes where they show, and of those that crawled (A11.10), recorded for the note, which the smoke test holds
// later builds to; every step must draw.
// Usage: node tools/screens/zoomstrip.mjs [--bench <bench json>]   (after tools/build-web.sh; the numbers go in the
// bench file under "zoom_strip")
import { readFileSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { ROOT, STEADY_STOPS, STRIP, launch, open, serve, zoomStrip } from './lib.mjs';

const benchAt = process.argv.indexOf('--bench');
const bench = benchAt > 0 ? path.resolve(process.argv[benchAt + 1]) : null;
const STEPS = Math.round((STRIP.to - STRIP.from) / STRIP.step);

let failed = 0;
const check = (name, ok, detail = '') => {
  if (!ok) failed += 1;
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${name}${detail ? ` (${detail})` : ''}`);
};

const server = await serve(path.join(ROOT, 'dist', 'web'));
const browser = await launch();
const out = {};
try {
  const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 270, height: 601, scale: 4 });
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  for (const [stop, golden] of Object.entries(STEADY_STOPS)) {
    // checks: PRE-22
    const strip = await zoomStrip(page, golden);
    check(`${stop}: every one of the ${STEPS} steps counted`, strip.changed.length === STEPS && strip.pixels > 0);
    out[stop] = { changed: strip.changed, crawl: strip.crawl };
    for (let k = 0; k < strip.changed.length; k++) {
      const [a, b] = [STRIP.from + STRIP.step * k, STRIP.from + STRIP.step * (k + 1)];
      console.log(`       ${stop} ${a.toFixed(2)} to ${b.toFixed(2)}: ${(100 * strip.changed[k]).toFixed(1)}% changed, ${(100 * strip.crawl[k]).toFixed(2)}% crawled`);
    }
  }
  await page.evaluate(() => window.kd.golden(''));
  check('no page errors', errors.length === 0, errors.slice(0, 3).join('; '));
  await close();
} finally {
  await browser.close();
  server.close();
}

if (bench && !failed) {
  const d = JSON.parse(readFileSync(bench, 'utf8'));
  d.zoom_strip = {
    ...STRIP,
    ...out,
    how: "each zoom step against the last from the camp and close camp stops' places and headings: the shares of the art target's pixels, less a border of two, that changed where they show and that crawled (A11.10); the demo area at 16:30, the phone's 270 x 601 art pixels, headless Chromium with SwiftShader (tools/screens/zoomstrip.mjs); the smoke test fails a step that changes more",
  };
  writeFileSync(bench, `${JSON.stringify(d, null, 2)}\n`);
  console.log(`       written to ${path.relative(ROOT, bench)}`);
}
console.log(failed ? `Zoom strip: FAIL (${failed})` : 'Zoom strip: PASS');
process.exit(failed ? 1 : 0);
