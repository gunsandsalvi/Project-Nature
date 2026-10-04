// Paints the art book's pictures. Each job is "scene:light", for example "camp:golden".
// The scenes are small 3D models drawn at the game's pixel size with the art rules (PRE-01, PRE-02, PRE-20,
// PRE-21, PRE-30); the page www/index.html does the drawing in a headless browser and hands back PNGs.
// Usage: node paint.js OUTDIR camp:golden river:morning ...
const { chromium } = require('playwright');
const fs = require('fs');
const path = require('path');

const ROOT = __dirname;
const MIME = { '.html': 'text/html', '.js': 'text/javascript', '.json': 'application/json', '.png': 'image/png' };

function serve(route) {
  const p = decodeURIComponent(new URL(route.request().url()).pathname);
  const file = p.startsWith('/three/') ? path.join(ROOT, 'node_modules/three', p.slice(7)) : path.join(ROOT, 'www', p);
  if (!fs.existsSync(file)) return route.fulfill({ status: 404, body: 'not found ' + p });
  route.fulfill({ status: 200, contentType: MIME[path.extname(file)] || 'application/octet-stream', body: fs.readFileSync(file) });
}

async function main() {
  const [out, ...jobs] = process.argv.slice(2);
  fs.mkdirSync(out, { recursive: true });
  const browser = await chromium.launch({
    args: ['--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'],
  });
  for (const job of jobs) {
    const [scene, light = 'noon', ...rest] = job.split(':');
    const page = await browser.newPage();
    page.on('console', (m) => console.log(`[${scene}]`, m.text()));
    page.on('pageerror', (e) => console.log(`[${scene}] error:`, e.message));
    await page.route('http://paint.local/**', serve);
    const t0 = Date.now();
    await page.goto(`http://paint.local/index.html?scene=${scene}&light=${light}&opts=${rest.join(',')}`);
    await page.waitForFunction(() => window.result !== undefined, null, { timeout: 900000, polling: 250 });
    const res = await page.evaluate(() => window.result);
    if (res.error) console.log(`FAILED ${job}: ${res.error}`);
    for (const [name, b64] of Object.entries(res.files || {})) {
      const f = path.join(out, name);
      fs.writeFileSync(f, Buffer.from(b64, 'base64'));
      console.log(`wrote ${f} (${((Date.now() - t0) / 1000).toFixed(1)} s)`);
    }
    for (const [name, url] of Object.entries(res.images || {})) {
      const f = path.join(out, `${scene}-${light}${rest.length ? '-' + rest.join('-') : ''}${name ? '-' + name : ''}.png`);
      fs.writeFileSync(f, Buffer.from(url.split(',')[1], 'base64'));
      console.log(`wrote ${f} (${((Date.now() - t0) / 1000).toFixed(1)} s)`);
    }
    await page.close();
  }
  await browser.close();
}

main();
