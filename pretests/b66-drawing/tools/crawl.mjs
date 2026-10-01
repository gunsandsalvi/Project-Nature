// B66 (PRE-22): count pixel crawl headless, frame by frame, for each pixel fix in drawing-test.html.
// Usage: node tools/crawl.mjs [--quick] [--motion turn|zoom|pan] [--out results]
// Each motion takes about 5 minutes here; run them one at a time under the shared CPU lock.
// Needs Playwright (node) and its Chromium; WebGL runs on the CPU through SwiftShader, so only pixels count here, not speed.
import { createRequire } from 'module';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const require = createRequire(import.meta.url);
const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '..');
const pwPath = process.env.PLAYWRIGHT_PATH || '/opt/node-tools/node_modules/playwright';
const { chromium } = require(pwPath);
const exe = process.env.CHROMIUM_PATH || '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';

const args = process.argv.slice(2);
const quick = args.includes('--quick');
const only = args.includes('--motion') ? args[args.indexOf('--motion') + 1] : null;
const outDir = path.resolve(root, args.includes('--out') ? args[args.indexOf('--out') + 1] : 'results');
fs.mkdirSync(outDir, { recursive: true });

// The phone's screen in portrait (Pixel Pro XL class: 1344 x 2992 device pixels), 5 device pixels per art pixel as the page picks at 3x.
const SIZE = { Wd: 1344, Hd: 2992, s: 5 };
const FRAMES = quick ? 12 : 60;
// Slow motions at the camp view, one step per 60 Hz frame.
const MOTIONS = [
  { motion: 'turn', rate: 0.002 },   // 0.11 degrees a frame, 6.9 degrees a second
  { motion: 'zoom', rate: 0.000368 }, // art pixels grow about 0.3% a frame
  { motion: 'pan', rate: 0.0123 },   // control: under 0.2 art pixels a frame; snapping should leave no crawl
];
const FIXES = ['base', 'steps', 'fade', 'majority', 'sticky'];

const browser = await chromium.launch({ executablePath: exe, args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'] });
const page = await browser.newPage({ viewport: { width: 400, height: 700 }, deviceScaleFactor: 1 });
const errors = [];
page.on('pageerror', (e) => errors.push(e.message));
page.on('console', (m) => { if (m.type() === 'error' && !/Failed to load resource/.test(m.text())) errors.push(m.text().slice(0, 400)); });
await page.goto('file://' + path.join(root, 'drawing-test.html'));
await page.waitForFunction(() => document.documentElement.dataset.viewReady === '1', null, { timeout: 120000 });
await page.waitForFunction(() => window.__view && window.__view.ready(), null, { timeout: 300000 });

const rows = [];
for (const m of MOTIONS.filter((x) => !only || x.motion === only)) {
  for (const fix of FIXES) {
    const t0 = Date.now();
    const r = await page.evaluate((o) => window.__b66.crawl(o), { ...SIZE, fix, motion: m.motion, rate: m.rate, frames: FRAMES, png: m.motion !== 'pan', crop: [120, 84], cropDy: 24, scale: 2 });
    const secs = ((Date.now() - t0) / 1000).toFixed(1);
    if (r.png) fs.writeFileSync(path.join(outDir, `crawl-${m.motion}-${fix}.png`), Buffer.from(r.png.split(',')[1], 'base64'));
    delete r.png;
    rows.push({ motion: m.motion, fix, ...r });
    console.log(`${m.motion.padEnd(5)} ${fix.padEnd(9)} crawl ${String(r.crawl).padStart(7)} of ${String(r.elig).padStart(8)} small-motion pixel-frames; changed ${String(r.changed).padStart(7)}; frames that changed ${r.changedFrames}/${r.frames}; worst pair ${r.worstK - 1}->${r.worstK} (${r.worstCrawl}) [${secs} s]`);
  }
}
if (errors.length) console.log('page errors:\n' + errors.join('\n'));
await browser.close();

// Summary against base, per motion.
const lines = ['motion,fix,frames,valid_pixel_frames,small_motion_pixel_frames,crawl,crawl_pct_of_small,crawl_vs_base,changed,changed_vs_base,frames_that_changed,worst_pair_end,worst_pair_crawl'];
for (const r of rows) {
  const b = rows.find((x) => x.motion === r.motion && x.fix === 'base');
  const vs = (a, z) => (z ? (a / z).toFixed(3) : (a ? 'inf' : '0'));
  lines.push([r.motion, r.fix, r.frames, r.valid, r.elig, r.crawl, (100 * r.crawl / Math.max(1, r.elig)).toFixed(3), vs(r.crawl, b.crawl), r.changed, vs(r.changed, b.changed), r.changedFrames, r.worstK, r.worstCrawl].join(','));
}
const csv = lines.join('\n') + '\n';
fs.writeFileSync(path.join(outDir, (quick ? 'crawl-quick' : 'crawl') + (only ? '-' + only : '') + '.csv'), csv);
console.log('\n' + csv);
