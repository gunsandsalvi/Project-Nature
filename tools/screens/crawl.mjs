// The crawl counter (A11.10, A11.12, PRE-22): at the camp and close camp stops of the demo area, 16:30 with time
// held, 60 frames each of a slow turn (0.11° a frame), a slow zoom (the art pixel 0.3% larger a frame) and a slow
// pan (0.37 art pixels a frame along the screen's right, the control), each frame against the last: the share of
// art pixels that crawled, their colour changed while the surface they show moved less than one art pixel, and the
// share that changed at all where they show. A pan moves the picture by the upscale's shift and whole art pixels
// only, so it must count almost no crawl; the turn's and zoom's shares are the alpha's to record in its note, and
// one over the last alpha's recorded share by more than a tenth is listed for the note to explain.
// Usage: node tools/screens/crawl.mjs [--bench <bench json>]   (after tools/build-web.sh; the numbers go in the
// bench file under "crawl")
import { readFileSync, readdirSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { ROOT, launch, open, serve } from './lib.mjs';

const benchAt = process.argv.indexOf('--bench');
const bench = benchAt > 0 ? path.resolve(process.argv[benchAt + 1]) : null;
const STOPS = { camp: 'valley-camp', close_camp: 'valley-close' };
const MOTIONS = { turn: 0.11, zoom: 0.003, pan: 0.37 };
const FRAMES = 60;
// The most a pan may count: a few art pixels a frame where the depth's 16 bits round a surface across a boundary.
const PAN_MOST = 0.0005;

let failed = 0;
const check = (name, ok, detail = '') => {
  if (!ok) failed += 1;
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${name}${detail ? ` (${detail})` : ''}`);
};
const mean = (xs) => xs.reduce((a, b) => a + b, 0) / Math.max(xs.length, 1);
const share = (x) => Number(x.toFixed(5));
const percent = (x) => `${(100 * x).toFixed(2)}%`;

// The newest other bench file's crawl shares, for the tenth's rule.
function lastRecorded() {
  const dir = path.join(ROOT, 'bench', 'cloud');
  const files = readdirSync(dir)
    .filter((f) => f.endsWith('.json') && (!bench || path.join(dir, f) !== bench))
    .sort();
  for (const f of files.reverse()) {
    const d = JSON.parse(readFileSync(path.join(dir, f), 'utf8'));
    if (d.crawl) return { alpha: d.alpha, crawl: d.crawl };
  }
  return null;
}

const server = await serve(path.join(ROOT, 'dist', 'web'));
const browser = await launch();
const out = {};
try {
  const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 270, height: 601, scale: 4 });
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  for (const [stop, golden] of Object.entries(STOPS)) {
    out[stop] = {};
    for (const [motion, rate] of Object.entries(MOTIONS)) {
      // checks: PRE-22
      const { counts, moved } = await page.evaluate(
        ({ g, m, r, n }) => {
          window.kd.hour(4);
          window.kd.golden(g);
          window.kd.frame(1);
          // Where the first frame's target shows before and after, in device pixels.
          const target = window.kd.camera().target;
          const before = window.kd.screenOf(target);
          const counts = window.kd.crawl({ motion: m, rate: r, frames: n });
          const after = window.kd.screenOf(target);
          return { counts, moved: [after[0] - before[0], after[1] - before[1]] };
        },
        { g: golden, m: motion, r: rate, n: FRAMES },
      );
      const crawl = mean(counts.crawl) / counts.pixels;
      const changed = mean(counts.changed) / counts.pixels;
      out[stop][motion] = { crawl: share(crawl), changed: share(changed) };
      console.log(`       ${stop} ${motion}: ${percent(crawl)} crawled, ${percent(changed)} changed a frame (${counts.crawl.length} frames)`);
      check(`${stop} ${motion} counted every frame`, counts.crawl.length === FRAMES && counts.pixels > 0);
      if (motion === 'pan') {
        // A pan that stood still would count none: the picture must have moved left by every frame's step, 4 device
        // pixels an art pixel, to a tick.
        const want = -FRAMES * rate * 4;
        const ok = Math.abs(moved[0] - want) < 0.5 && Math.abs(moved[1]) < 0.5;
        check(`${stop} pan moves the picture ${FRAMES * rate} art pixels`, ok, `${moved.map((v) => v.toFixed(2))} device pixels`);
        check(`${stop} pan counts almost no crawl`, crawl <= PAN_MOST, `${percent(crawl)}, at most ${percent(PAN_MOST)}`);
      } else {
        // A counter that saw nothing move would pass any fix.
        check(`${stop} ${motion} changes the picture`, changed > 0.01, `${percent(changed)} changed`);
      }
    }
  }
  await page.evaluate(() => window.kd.golden(''));
  check('no page errors', errors.length === 0, errors.slice(0, 3).join('; '));
  await close();
} finally {
  await browser.close();
  server.close();
}

const last = lastRecorded();
const grown = [];
for (const [stop, motions] of Object.entries(out)) {
  for (const [motion, s] of Object.entries(motions)) {
    const before = last?.crawl?.[stop]?.[motion]?.crawl;
    if (motion !== 'pan' && before !== undefined && s.crawl > before * 1.1) {
      grown.push(`${stop} ${motion} ${percent(before)} to ${percent(s.crawl)}`);
    }
  }
}
if (grown.length) console.log(`  note crawl grown by over a tenth since ${last.alpha}, for the note: ${grown.join('; ')}`);
else console.log(last ? `       no crawl grown by over a tenth since ${last.alpha}` : '       no earlier crawl recorded');

if (bench && !failed) {
  const d = JSON.parse(readFileSync(bench, 'utf8'));
  d.crawl = {
    ...out,
    grown_since_last: grown.length ? `${grown.join('; ')} (since ${last.alpha})` : 'none',
    how: `mean over ${FRAMES} frames of each art pixel's crawl (A11.10) and change, as shares of the art target's pixels less a border of two; the demo area at 16:30, the phone's 270 x 601 art pixels, headless Chromium with SwiftShader; turn ${MOTIONS.turn}° a frame, zoom ${100 * MOTIONS.zoom}% of the art pixel a frame, pan ${MOTIONS.pan} art pixels a frame (tools/screens/crawl.mjs)`,
  };
  writeFileSync(bench, `${JSON.stringify(d, null, 2)}\n`);
  console.log(`       written to ${path.relative(ROOT, bench)}`);
}
console.log(failed ? `Crawl: FAIL (${failed})` : 'Crawl: PASS');
process.exit(failed ? 1 : 0);
