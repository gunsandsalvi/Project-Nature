// Shared by every screen script (A15.11): headless Chromium with WebGL on the CPU, a static server, and pixel reads.
// From pretests/b66-drawing/tools (B66's launch settings).
import { createRequire } from 'module';
import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';

const require = createRequire(import.meta.url);

/** Chromium with SwiftShader WebGL, a 412 × 860 touch viewport at scale 1; page and console errors collected. */
export async function launch() {
  const { chromium } = require(process.env.PLAYWRIGHT_PATH || '/opt/node-tools/node_modules/playwright');
  const exe = process.env.CHROMIUM_PATH || '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';
  const browser = await chromium.launch({
    executablePath: exe,
    args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'],
  });
  const ctx = await browser.newContext({ viewport: { width: 412, height: 860 }, deviceScaleFactor: 1, hasTouch: true, isMobile: true });
  const page = await ctx.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(e.message));
  page.on('console', (m) => { if (m.type() === 'error') errors.push(m.text().slice(0, 300)); });
  return { browser, ctx, page, errors };
}

const TYPES = { '.html': 'text/html', '.js': 'text/javascript', '.mjs': 'text/javascript', '.wasm': 'application/wasm', '.png': 'image/png' };

/** A static server for `dir` on a free port (modules and wasm do not load from file://). Returns { url, close }. */
export async function serve(dir) {
  const root = path.resolve(dir);
  const server = http.createServer((req, res) => {
    const u = decodeURIComponent(new URL(req.url, 'http://x').pathname);
    let f = path.join(root, u);
    if (!f.startsWith(root)) { res.writeHead(403); res.end(); return; }
    if (u === '/favicon.ico') { res.writeHead(204); res.end(); return; } // browsers ask for it unprompted
    if (fs.existsSync(f) && fs.statSync(f).isDirectory()) f = path.join(f, 'index.html');
    if (!fs.existsSync(f)) { res.writeHead(404); res.end('not found'); return; }
    res.writeHead(200, { 'content-type': TYPES[path.extname(f)] || 'application/octet-stream' });
    fs.createReadStream(f).pipe(res);
  });
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  const { port } = server.address();
  return { url: `http://127.0.0.1:${port}`, close: () => new Promise((r) => server.close(r)) };
}

/** Decodes a PNG screenshot inside the page (the session's Node has no PNG library): { width, height, data }. */
export async function pixels(page, png) {
  const b64 = await page.evaluate(async (src) => {
    const img = new Image();
    img.src = src;
    await img.decode();
    const c = document.createElement('canvas');
    c.width = img.width; c.height = img.height;
    const g = c.getContext('2d');
    g.drawImage(img, 0, 0);
    const d = g.getImageData(0, 0, c.width, c.height).data;
    let s = '';
    for (let i = 0; i < d.length; i += 0x8000) s += String.fromCharCode(...d.subarray(i, i + 0x8000));
    return JSON.stringify([c.width, c.height, btoa(s)]);
  }, 'data:image/png;base64,' + png.toString('base64'));
  const [width, height, data] = JSON.parse(b64);
  return { width, height, data: Buffer.from(data, 'base64') };
}
