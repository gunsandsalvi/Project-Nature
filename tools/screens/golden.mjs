// Golden scenes (A11.12, T01a.8): opens dist/web with ?test=1, shows each golden scene through window.kd.golden(name),
// takes the canvas right after a frame with kd.shot(), and compares its pixels exactly with
// tests/golden/<name>-chromium-<build>.png for this Chromium build. A missing reference is written once, for the
// builder to look at and commit; any difference fails, with the count of differing pixels.
// Usage: node tools/screens/golden.mjs
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { launch, serve, pixels } from './lib.mjs';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const SCENES = ['cube'];
const exe = process.env.CHROMIUM_PATH || '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';
const build = (/chromium-(\d+)/.exec(exe) || [, 'unknown'])[1];
let pass = true;

const srv = await serve(path.join(root, 'dist/web'));
const { browser, page, errors } = await launch();
try {
  await page.goto(srv.url + '/?test=1');
  await page.waitForFunction(() => window.kd && window.kd.ready(), null, { timeout: 60000 });
  for (const name of SCENES) {
    // checks: PRE-01 PRE-20 PRE-21 PRE-22
    const known = await page.evaluate((n) => window.kd.golden(n), name);
    if (!known) { console.log(`FAIL ${name}: no such golden scene`); pass = false; continue; }
    await page.waitForTimeout(300);
    const url = await page.evaluate(() => window.kd.shot());
    const png = Buffer.from(url.slice(url.indexOf(',') + 1), 'base64');
    const ref = path.join(root, 'tests/golden', `${name}-chromium-${build}.png`);
    if (!fs.existsSync(ref)) {
      fs.mkdirSync(path.dirname(ref), { recursive: true });
      fs.writeFileSync(ref, png);
      console.log(`new  ${name}: wrote ${path.relative(root, ref)}; look at it, then commit it`);
      continue;
    }
    const [a, b] = [await pixels(page, png), await pixels(page, fs.readFileSync(ref))];
    let differ = a.width === b.width && a.height === b.height ? 0 : -1;
    if (differ === 0) {
      for (let i = 0; i < a.data.length; i += 4) {
        if (a.data[i] !== b.data[i] || a.data[i + 1] !== b.data[i + 1] || a.data[i + 2] !== b.data[i + 2]) differ++;
      }
    }
    if (differ === 0) console.log(`ok   ${name} (${a.width} x ${a.height}, exact)`);
    else {
      pass = false;
      const out = path.join(root, 'target/screens/golden');
      fs.mkdirSync(out, { recursive: true });
      fs.writeFileSync(path.join(out, `${name}.png`), png);
      console.log(`FAIL ${name}: ${differ < 0 ? `size ${a.width} x ${a.height}, not ${b.width} x ${b.height}` : `${differ} pixels differ`}; this run's picture is target/screens/golden/${name}.png`);
    }
  }
  if (errors.length) { console.log('FAIL page errors: ' + errors.join(' | ')); pass = false; }
} finally {
  await browser.close();
  await srv.close();
}
console.log(pass ? 'Golden: PASS' : 'Golden: FAIL');
process.exit(pass ? 0 : 1);
