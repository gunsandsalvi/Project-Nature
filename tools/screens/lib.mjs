// Shared by the screen scripts (A15.11): serves dist/web/ to the session's Chromium through Playwright (WebGL
// through SwiftShader: pixels count, not speed), decodes screenshots, since the session's Node has no PNG library,
// and runs the steadiness counts' motions and zoom strip, and finds the bench file that recorded them (A11.12).
import { createServer } from 'node:http';
import { readFileSync, readdirSync } from 'node:fs';
import { readFile } from 'node:fs/promises';
import { createRequire } from 'node:module';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { inflateSync } from 'node:zlib';

const require = createRequire(import.meta.url);
const { chromium } = require(process.env.PLAYWRIGHT_PATH || 'playwright');

export const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');

const TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript',
  '.mjs': 'text/javascript',
  '.wasm': 'application/wasm',
  '.png': 'image/png',
  '.json': 'application/json',
};

// The skeleton the artifact host wraps a published page in, so a local test sees the page as the owner does.
const skeleton = (body) =>
  '<!doctype html><html lang="en"><head><meta charset="utf-8">' +
  '<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover"></head>' +
  `<body>${body}</body></html>`;

/** A static server for `dir` on a free port: modules and wasm do not load from file:// URLs. */
export async function serve(dir) {
  const root = path.resolve(dir);
  const server = createServer(async (req, res) => {
    let p = decodeURIComponent(new URL(req.url, 'http://local').pathname);
    if (p === '/favicon.ico') {
      res.writeHead(204).end(); // the artifact host gives the page its icon
      return;
    }
    if (p.endsWith('/')) p += 'index.html';
    const file = path.join(root, path.normalize(p));
    if (!file.startsWith(root)) {
      res.writeHead(403).end();
      return;
    }
    try {
      let data = await readFile(file);
      if (path.basename(file) === 'index.html') data = Buffer.from(skeleton(data.toString('utf-8')));
      res.writeHead(200, { 'content-type': TYPES[path.extname(file)] || 'application/octet-stream' }).end(data);
    } catch {
      res.writeHead(404).end();
    }
  });
  await new Promise((resolve) => server.listen(0, '127.0.0.1', resolve));
  return { url: `http://127.0.0.1:${server.address().port}/`, close: () => server.close() };
}

/** The session's Chromium, drawing WebGL2 through SwiftShader. */
export function launch() {
  return chromium.launch({
    executablePath: process.env.CHROMIUM_PATH || undefined,
    args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--ignore-gpu-blocklist'],
  });
}

/** A page at `url` in a touch viewport, collecting page errors and console errors. */
export async function open(browser, url, { width, height, scale = 1 }) {
  const context = await browser.newContext({ viewport: { width, height }, deviceScaleFactor: scale, hasTouch: true });
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(String(e)));
  page.on('console', (m) => {
    if (m.type() === 'error') errors.push(m.text());
  });
  await page.goto(url);
  return { page, errors, close: () => context.close() };
}

/** An 8-bit, non-interlaced RGB or RGBA PNG as RGBA pixels, with `at(x, y)` giving [r, g, b]. */
export function decodePng(buf) {
  if (buf.readUInt32BE(0) !== 0x89504e47) throw new Error('not a PNG');
  let pos = 8;
  let w = 0, h = 0, depth = 0, type = 0, interlace = 0;
  const idat = [];
  while (pos < buf.length) {
    const len = buf.readUInt32BE(pos);
    const kind = buf.toString('ascii', pos + 4, pos + 8);
    const data = buf.subarray(pos + 8, pos + 8 + len);
    if (kind === 'IHDR') {
      w = data.readUInt32BE(0);
      h = data.readUInt32BE(4);
      depth = data[8];
      type = data[9];
      interlace = data[12];
    } else if (kind === 'IDAT') {
      idat.push(data);
    } else if (kind === 'IEND') {
      break;
    }
    pos += 12 + len;
  }
  if (depth !== 8 || interlace !== 0 || (type !== 2 && type !== 6)) {
    throw new Error(`unsupported PNG: depth ${depth}, colour type ${type}, interlace ${interlace}`);
  }
  const bpp = type === 6 ? 4 : 3;
  const stride = w * bpp;
  const raw = inflateSync(Buffer.concat(idat));
  const out = Buffer.alloc(w * h * 4);
  let prev = Buffer.alloc(stride);
  let cur = Buffer.alloc(stride);
  for (let y = 0; y < h; y++) {
    const filter = raw[y * (stride + 1)];
    const line = raw.subarray(y * (stride + 1) + 1, (y + 1) * (stride + 1));
    for (let i = 0; i < stride; i++) {
      const a = i >= bpp ? cur[i - bpp] : 0;
      const b = prev[i];
      const c = i >= bpp ? prev[i - bpp] : 0;
      let v = line[i];
      if (filter === 1) v += a;
      else if (filter === 2) v += b;
      else if (filter === 3) v += (a + b) >> 1;
      else if (filter === 4) {
        const p = a + b - c, pa = Math.abs(p - a), pb = Math.abs(p - b), pc = Math.abs(p - c);
        v += pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
      }
      cur[i] = v & 255;
    }
    for (let x = 0; x < w; x++) {
      const o = (y * w + x) * 4;
      out[o] = cur[x * bpp];
      out[o + 1] = cur[x * bpp + 1];
      out[o + 2] = cur[x * bpp + 2];
      out[o + 3] = bpp === 4 ? cur[x * bpp + 3] : 255;
    }
    [prev, cur] = [cur, prev];
  }
  return {
    width: w,
    height: h,
    data: out,
    at(x, y) {
      const o = (y * w + x) * 4;
      return [out[o], out[o + 1], out[o + 2]];
    },
  };
}

// The steadiness counts' stops (A11.12): the golden scenes each starts from, at 16:30 with time held.
export const STEADY_STOPS = { camp: 'valley-camp', close_camp: 'valley-close' };
// The zoom strip: 1% of the zoom's range a step, through the close camp and camp stops (A11.12).
export const STRIP = { from: 0.1, to: 0.34, step: 0.01 };

/** A slow camera motion from a golden scene's pose, each frame against the last (A11.10): the counts, and how far
 * the first frame's target moved on the screen, in device pixels. */
export async function slowMotion(page, golden, motion, rate, frames) {
  return page.evaluate(
    ({ g, m, r, n }) => {
      window.kd.hour(4);
      window.kd.golden(g);
      window.kd.frame(1);
      const target = window.kd.camera().target;
      const before = window.kd.screenOf(target);
      const counts = window.kd.crawl({ motion: m, rate: r, frames: n });
      const after = window.kd.screenOf(target);
      return { counts, moved: [after[0] - before[0], after[1] - before[1]] };
    },
    { g: golden, m: motion, r: rate, n: frames },
  );
}

/** The zoom strip from a golden scene's place and heading (A11.12): each step's shares of the art target's pixels
 * that changed where they show and that crawled, to four places. */
export async function zoomStrip(page, golden) {
  const counts = await page.evaluate(
    ({ g, s }) => {
      window.kd.hour(4);
      window.kd.golden(g);
      window.kd.frame(1);
      return window.kd.zoomstrip(s);
    },
    { g: golden, s: STRIP },
  );
  const share = (c) => Number((c / counts.pixels).toFixed(4));
  return { changed: counts.changed.map(share), crawl: counts.crawl.map(share), pixels: counts.pixels };
}

/** The newest bench file in bench/cloud/ holding `key`, other than the file `except`: its alpha and that value. */
export function newestBench(key, except = null) {
  const dir = path.join(ROOT, 'bench', 'cloud');
  const files = readdirSync(dir)
    .filter((f) => f.endsWith('.json') && path.join(dir, f) !== except)
    .sort()
    .reverse();
  for (const f of files) {
    const d = JSON.parse(readFileSync(path.join(dir, f), 'utf8'));
    if (d[key] !== undefined) return { alpha: d.alpha, value: d[key] };
  }
  return null;
}
