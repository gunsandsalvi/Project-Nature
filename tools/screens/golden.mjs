// The golden scenes (A11.12, A15.11): the light card at 06:30, 12:00, 16:30, 18:30 and 23:00, the block alone at
// 16:30, the demo area's ground at the camp stop at the same five hours, and at the cliff's foot close up and near
// the person stop at 16:30, drawn with time frozen at the phone's own size (270 x 601 art pixels at 4 device pixels
// each), one pixel an art pixel, and compared exactly with tests/golden/<name>-<hhmm>.png, stored for the session's
// Chromium. A missing golden is written and reported, so a new one is looked at before it is committed.
// Usage: node tools/screens/golden.mjs [--save <dir>]   (after tools/build-web.sh)
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { ROOT, decodePng, launch, open, serve } from './lib.mjs';

const GOLDEN = path.join(ROOT, 'tests', 'golden');
const saveAt = process.argv.indexOf('--save');
const saveDir = saveAt > 0 ? path.resolve(process.argv[saveAt + 1]) : null;
// Each hour's place in kd-app's HOURS.
const HOUR = { '0630': 0, '1200': 2, '1630': 4, '1830': 6, '2300': 7 };
const CASES = [
  ['light-card', ['0630', '1200', '1630', '1830', '2300']],
  ['block', ['1630']],
  ['valley-camp', ['0630', '1200', '1630', '1830', '2300']],
  ['valley-close', ['1630']],
  ['valley-near', ['1630']],
];

let failed = 0;
let written = 0;
const check = (name, ok, detail = '') => {
  if (!ok) failed += 1;
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${name}${detail ? ` (${detail})` : ''}`);
};

// checks: PRE-20 PRE-30
// The same pixels as the stored golden, or the first that differs.
function compare(a, b) {
  if (a.width !== b.width || a.height !== b.height) return `size ${a.width} x ${a.height}, stored ${b.width} x ${b.height}`;
  let differ = 0;
  let first = '';
  for (let y = 0; y < a.height; y++) {
    for (let x = 0; x < a.width; x++) {
      const [p, q] = [a.at(x, y), b.at(x, y)];
      if (p[0] !== q[0] || p[1] !== q[1] || p[2] !== q[2]) {
        if (!differ) first = ` first at ${x}, ${y}: ${p.slice(0, 3)} against ${q.slice(0, 3)}`;
        differ += 1;
      }
    }
  }
  return differ ? `${differ} pixels differ;${first}` : '';
}

mkdirSync(GOLDEN, { recursive: true });
if (saveDir) mkdirSync(saveDir, { recursive: true });
const server = await serve(path.join(ROOT, 'dist', 'web'));
const browser = await launch();
try {
  const { page, errors, close } = await open(browser, `${server.url}?test=1`, { width: 270, height: 601, scale: 4 });
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  for (const [name, hours] of CASES) {
    for (const hhmm of hours) {
      const url = await page.evaluate(({ n, g }) => {
        window.kd.hour(n);
        window.kd.golden(g);
        window.kd.frame(1);
        return window.kd.shot({ art: true });
      }, { n: HOUR[hhmm], g: name });
      const png = Buffer.from(url.split(',')[1], 'base64');
      const file = path.join(GOLDEN, `${name}-${hhmm}.png`);
      if (saveDir) writeFileSync(path.join(saveDir, `${name}-${hhmm}.png`), png);
      if (!existsSync(file)) {
        writeFileSync(file, png);
        written += 1;
        console.log(`  new  ${name} ${hhmm}: written to ${path.relative(ROOT, file)}; look at it before committing`);
        continue;
      }
      const why = compare(decodePng(png), decodePng(readFileSync(file)));
      check(`${name} ${hhmm} equals its golden`, why === '', why);
    }
  }
  await page.evaluate(() => window.kd.golden(''));
  check('no page errors', errors.length === 0, errors.slice(0, 3).join('; '));
  await close();
} finally {
  await browser.close();
  server.close();
}
console.log(failed ? `Goldens: FAIL (${failed})` : `Goldens: PASS${written ? ` (${written} written)` : ''}`);
process.exit(failed ? 1 : 0);
