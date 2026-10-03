// The crawl counter (A11.10, PRE-22): B66's tools/crawl.mjs on top of lib.mjs. Opens dist/web with ?test=1 and runs
// window.kd.crawl for slow motions at the camp stop on the phone's art target in portrait, one step a 60 Hz frame:
// a turn (0.002 radians a frame, 6.9 degrees a second) and a zoom (0.000368 a frame, art pixels growing about 0.3% a
// frame), counting art pixels that change colour while the ground under them moves less than one art pixel; and a
// pan, B66's control, which the snapped view leaves without crawl. Only the fix `base` exists until the owner's review
// at the stage close (α07e measures the other four with this script).
// Usage: node tools/screens/crawl.mjs [--quick] [--motion turn|zoom|pan] [--zoom z] [--no-write]
// Writes crawl_turn, crawl_zoom and crawl_pan into bench/cloud/<versionName>.json, keeping the file's other figures;
// --zoom measures elsewhere (B66's own view was zoom 0.16) and writes nothing.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { launch, serve } from './lib.mjs';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const args = process.argv.slice(2);
const quick = args.includes('--quick');
const only = args.includes('--motion') ? args[args.indexOf('--motion') + 1] : null;
const zoom = args.includes('--zoom') ? Number(args[args.indexOf('--zoom') + 1]) : undefined;
const FRAMES = quick ? 12 : 60;
const MOTIONS = [
  { motion: 'turn', rate: 0.002 },
  { motion: 'zoom', rate: 0.000368 },
  { motion: 'pan', rate: 0.0123 },
];

const srv = await serve(path.join(root, 'dist/web'));
const { browser, page, errors } = await launch();
const out = {};
let pass = true;
try {
  await page.goto(srv.url + '/?test=1');
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  for (const m of MOTIONS.filter((x) => !only || x.motion === only)) {
    // checks: PRE-22
    const t0 = Date.now();
    const r = await page.evaluate((o) => window.kd.crawl(o), { motion: m.motion, rate: m.rate, frames: FRAMES, fix: 'base', zoom });
    if (!r) { console.log(`FAIL ${m.motion}: no crawl count`); pass = false; continue; }
    const pct = Number((100 * r.crawl / Math.max(1, r.elig)).toFixed(3));
    out[`crawl_${m.motion}`] = { fix: 'base', frames: r.frames, pct, crawl: r.crawl, elig: r.elig, changed: r.changed };
    console.log(`${m.motion.padEnd(5)} base  crawl ${String(r.crawl).padStart(7)} of ${String(r.elig).padStart(8)} small-motion pixel-frames (${pct}%); changed ${r.changed}; frames that changed ${r.changedFrames}/${r.frames}; worst pair ${r.worstK - 1}->${r.worstK} (${r.worstCrawl}) [${((Date.now() - t0) / 1000).toFixed(1)} s]`);
  }
  if (errors.length) { console.log('FAIL page errors: ' + errors.join(' | ')); pass = false; }
} finally {
  await browser.close();
  await srv.close();
}
if (pass && !args.includes('--no-write') && !quick && zoom === undefined) {
  const version = /^versionName=(.*)$/m.exec(fs.readFileSync(path.join(root, 'android/version.properties'), 'utf8'))[1].trim();
  const file = path.join(root, 'bench/cloud', `${version}.json`);
  const bench = fs.existsSync(file) ? JSON.parse(fs.readFileSync(file, 'utf8')) : { alpha: version };
  fs.writeFileSync(file, JSON.stringify({ ...bench, ...out }) + '\n');
  console.log(`wrote ${Object.keys(out).join(', ')} into ${path.relative(root, file)}`);
}
console.log(pass ? 'Crawl: PASS' : 'Crawl: FAIL');
process.exit(pass ? 0 : 1);
