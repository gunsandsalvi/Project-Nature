// The interface drawn in art pixels over a painted world: quiet dark panels for play, warm paper and ink for the
// field journal (cards and the book of ages), talk bubbles with a picture of the topic, time controls and the handle.
import { Painter } from './engine.js';
import { K } from './kit/k.js';
import { Solid } from './geo.js';
import { put } from './kit/scene.js';
import { drawText, measure, wrap, LINE } from './font.js';

export const C = {
  panel: 'rgba(22,19,28,0.88)', edge: '#3d3548', rim: '#0c0a10', text: '#efe6d6', dim: '#b9ad9c', accent: '#e0a24a',
  paper: '#efe2c4', paper2: '#e6d6b2', ink: '#3b2a2c', inkMid: '#8a6a50', red: '#9c3a2a', bubble: '#f4ece0', line: '#d8c6a0',
};

export function rect(ctx, x, y, w, h, c) { ctx.fillStyle = c; ctx.fillRect(Math.round(x), Math.round(y), Math.round(w), Math.round(h)); }

/** A quiet dark panel: see-through dark purple, a 1-pixel inner rim, corners cut. */
export function darkPanel(ctx, x, y, w, h, { corner = 2 } = {}) {
  ctx.save();
  ctx.beginPath();
  ctx.moveTo(x + corner, y); ctx.lineTo(x + w - corner, y); ctx.lineTo(x + w, y + corner); ctx.lineTo(x + w, y + h - corner);
  ctx.lineTo(x + w - corner, y + h); ctx.lineTo(x + corner, y + h); ctx.lineTo(x, y + h - corner); ctx.lineTo(x, y + corner); ctx.closePath();
  ctx.clip();
  rect(ctx, x, y, w, h, C.panel);
  ctx.restore();
  rect(ctx, x + corner, y, w - corner * 2, 1, C.edge); rect(ctx, x + corner, y + h - 1, w - corner * 2, 1, C.rim);
  rect(ctx, x, y + corner, 1, h - corner * 2, C.edge); rect(ctx, x + w - 1, y + corner, 1, h - corner * 2, C.rim);
  for (let i = 0; i < corner; i++) { rect(ctx, x + corner - i - 1, y + i + 1, 1, 1, C.edge); rect(ctx, x + w - corner + i, y + i + 1, 1, 1, C.edge); }
}

/** Field-journal paper: warm, with soft blotches in a second tone and a darker edge. */
export function paper(ctx, x, y, w, h, seed = 1) {
  rect(ctx, x, y, w, h, C.paper);
  let s = seed * 7919;
  const rnd = () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
  for (let i = 0; i < (w * h) / 180; i++) {
    const bx = x + rnd() * w, by = y + rnd() * h, br = 2 + rnd() * 6;
    for (let yy = -br; yy <= br; yy++) for (let xx = -br; xx <= br; xx++) {
      if (xx * xx + yy * yy * 1.6 > br * br) continue;
      const px = Math.round(bx + xx), py = Math.round(by + yy);
      if (px < x || py < y || px >= x + w || py >= y + h) continue;
      if ((px + py) % 2 === 0 || xx * xx + yy * yy < br * br * 0.4) rect(ctx, px, py, 1, 1, C.paper2);
    }
  }
  rect(ctx, x, y, w, 1, C.inkMid); rect(ctx, x, y + h - 1, w, 1, C.inkMid); rect(ctx, x, y, 1, h, C.inkMid); rect(ctx, x + w - 1, y, 1, h, C.inkMid);
}

/** Text helpers: a line, or a block wrapped to a width; returns the y below it. */
export function text(ctx, s, x, y, o = {}) { return drawText(ctx, s, x, y, o); }
export function block(ctx, s, x, y, maxW, o = {}) {
  const lh = o.lineH || LINE + (o.hand ? 1 : 0);
  for (const line of wrap(s, maxW, o)) { drawText(ctx, line, x, y, o); y += lh; }
  return y;
}
export { measure };

/** A talk bubble over a point: a pale rounded box with a tail, holding an icon (a canvas region). */
export function bubble(ctx, cx, by, icon, [sx, sy, sw, sh]) {
  const w = sw + 6, h = sh + 6, x = Math.round(cx - w / 2), y = Math.round(by - h - 4);
  rect(ctx, x + 1, y, w - 2, h, C.bubble); rect(ctx, x, y + 1, w, h - 2, C.bubble);
  rect(ctx, x + 1, y - 1, w - 2, 1, C.rim); rect(ctx, x + 1, y + h, w - 2, 1, C.rim);
  rect(ctx, x - 1, y + 1, 1, h - 2, C.rim); rect(ctx, x + w, y + 1, 1, h - 2, C.rim);
  rect(ctx, x, y, 1, 1, C.rim); rect(ctx, x + w - 1, y, 1, 1, C.rim); rect(ctx, x, y + h - 1, 1, 1, C.rim); rect(ctx, x + w - 1, y + h - 1, 1, 1, C.rim);
  // the tail
  rect(ctx, cx - 2, y + h, 4, 1, C.bubble); rect(ctx, cx - 1, y + h + 1, 2, 1, C.bubble); rect(ctx, cx, y + h + 2, 1, 1, C.bubble);
  rect(ctx, cx - 3, y + h, 1, 1, C.rim); rect(ctx, cx + 2, y + h, 1, 1, C.rim); rect(ctx, cx - 2, y + h + 1, 1, 1, C.rim);
  rect(ctx, cx + 1, y + h + 1, 1, 1, C.rim); rect(ctx, cx - 1, y + h + 2, 1, 1, C.rim); rect(ctx, cx + 1, y + h + 2, 1, 1, C.rim); rect(ctx, cx, y + h + 3, 1, 1, C.rim);
  ctx.drawImage(icon, sx, sy, sw, sh, x + 3, y + 3, sw, sh);
}

/** A round button with a glyph: pause, play, fast, slow. */
export function button(ctx, cx, cy, r, kind, { active = false } = {}) {
  for (let y = -r; y <= r; y++) for (let x = -r; x <= r; x++) {
    const d = x * x + y * y;
    if (d > r * r + r) continue;
    const ring = d > (r - 1) * (r - 1) + (r - 1);
    rect(ctx, cx + x, cy + y, 1, 1, ring ? (y < 0 ? C.edge : C.rim) : (active ? 'rgba(70,52,30,0.92)' : C.panel));
  }
  const col = active ? C.accent : C.text;
  if (kind === 'pause') { rect(ctx, cx - 3, cy - 4, 2, 9, col); rect(ctx, cx + 2, cy - 4, 2, 9, col); }
  if (kind === 'play' || kind === 'fast') {
    const tri = (ox) => { for (let i = 0; i < 5; i++) rect(ctx, cx + ox + i, cy - 4 + i, 1, 9 - i * 2, col); };
    if (kind === 'play') tri(-2); else { tri(-5); tri(1); }
  }
  if (kind === 'slow') { for (let i = 0; i < 5; i++) rect(ctx, cx + 2 - i, cy - 4 + i, 1, 9 - i * 2, col); rect(ctx, cx - 4, cy - 4, 2, 9, col); }
  if (kind === 'book') { rect(ctx, cx - 5, cy - 4, 5, 8, col); rect(ctx, cx + 1, cy - 4, 5, 8, col); rect(ctx, cx, cy - 4, 1, 9, C.rim); }
  if (kind === 'globe') { for (let y = -5; y <= 5; y++) for (let x = -5; x <= 5; x++) { const d = x * x + y * y; if (d <= 25 && (d > 16 || x === 0 || y === 0)) rect(ctx, cx + x, cy + y, 1, 1, col); } }
}

/** The handle above the bottom edge that opens the views. */
export function handle(ctx, cx, y) { rect(ctx, cx - 14, y, 28, 3, C.rim); rect(ctx, cx - 13, y, 26, 2, C.dim); }

/** A dotted ring on the ground round a selected person. */
export function ring(ctx, cx, cy, rx, ry, color = C.accent) {
  for (let a = 0; a < Math.PI * 2; a += 0.06) {
    const x = Math.round(cx + Math.cos(a) * rx), y = Math.round(cy + Math.sin(a) * ry);
    if ((x + y) % 3 !== 0) rect(ctx, x, y, 1, 1, color);
  }
}

/**
 * Renders models as icons with a clear background: items is a list of { piece, scale, y, yaw }. Each sits in a cell
 * of `cell` art pixels. Returns { canvas, at(i) => [sx, sy, sw, sh] }.
 */
export async function renderIcons(items, { cell = 16, mpp = 0.1, cols = 8, light = 'noon', ink = false } = {}) {
  const rows = Math.ceil(items.length / cols), w = cols * cell, h = rows * cell;
  K.mpp = mpp;
  const step = cell * mpp;
  const P = new Painter({ w, h, mpp, yaw: 25, elev: 30, target: [0, 0, 0], mood: light, bounds: { c: [0, 0, 0], r: Math.max(w, h) * mpp }, shadowSize: 1024 });
  P.clear = !ink; P.ink = ink;
  P.ao = { rad: 0.3, dist: 0.3, k: 0 };
  // lay the cells out in the camera's screen plane, so each model sits in its own cell
  const right = [Math.cos(25 * Math.PI / 180), 0, -Math.sin(25 * Math.PI / 180)];
  const fwd = [-Math.sin(25 * Math.PI / 180), 0, -Math.cos(25 * Math.PI / 180)];
  items.forEach((it, i) => {
    const c = i % cols, r = Math.floor(i / cols);
    const sx = (c + 0.5 - cols / 2) * step, sy = (rows / 2 - r - 0.5) * step;
    // a point on the ground plane that shows at screen offset (sx, sy): move along screen up = world forward / sin(30)
    const up = sy / Math.sin(30 * Math.PI / 180);
    const x = right[0] * sx + fwd[0] * up, z = right[2] * sx + fwd[2] * up;
    put(P, it.piece, x, z, { y: (it.y || 0) - step * 0.3, yaw: it.yaw || 0, scale: it.scale || 1 });
  });
  const canvas = await P.render(0);
  return { canvas, at: (i) => [(i % cols) * cell, Math.floor(i / cols) * cell, cell, cell] };
}
