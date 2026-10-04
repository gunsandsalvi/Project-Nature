// The one place of the zoom stops (PRE-03): the generated world (worldgen.js), its start region, and a band's camp
// under a cliff by a river. Near the camp the ground is given in the camp's own frame, metres: u along the valley,
// v from the cliff toward the river, as x and z in the rock shelter scene. One height function serves every stop
// from the valley in, so the cliff, the river and the camp stay where they are as the view closes in.
import { worldGrid, findStart, regionGrid, riverLines, smoothLine, elevation, latitude, fbmW, BIOME, FOREST } from '../worldgen.js';
import { fbm } from '../geo.js';

export const SEED = 7;
const smooth = (a, b, x) => { const t = Math.max(0, Math.min(1, (x - a) / (b - a))); return t * t * (3 - 2 * t); };
const wrapK = (v, p) => ((v % p) + p) % p;

function upstreamOf(R, k) {
  let best = -1, acc = 0;
  const x = k % R.w, y = Math.floor(k / R.w);
  for (let dy = -1; dy <= 1; dy++) for (let dx = -1; dx <= 1; dx++) {
    const xx = x + dx, yy = y + dy;
    if (xx < 0 || yy < 0 || xx >= R.w || yy >= R.h) continue;
    const j = yy * R.w + xx;
    if (R.down[j] === k && R.acc[j] > acc) { acc = R.acc[j]; best = j; }
  }
  return best;
}

/** A grid's heights smoothed over (2r+1)² cells: the map look shades broad hills, never single cells (PRE-29). */
export function blurred(Gr, r, wrapping) {
  const { w, h } = Gr, src = Gr.hgt, tmp = new Float32Array(w * h), out = new Float32Array(w * h);
  const at = (a, n) => (wrapping ? wrapK(a, n) : Math.max(0, Math.min(n - 1, a)));
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    let s = 0; for (let k = -r; k <= r; k++) s += Math.max(-300, src[j * w + at(i + k, w)]);
    tmp[j * w + i] = s / (2 * r + 1);
  }
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    let s = 0; for (let k = -r; k <= r; k++) s += tmp[at(j + k, h) * w + i];
    out[j * w + i] = s / (2 * r + 1);
  }
  return { ...Gr, hgt: out };
}

/** Height of a grid at world km, bilinear; wrapping for the whole world. */
export function gridHeight(Gr, xk, yk, wrapping = false) {
  const fx = (xk - Gr.x0) / Gr.cell - 0.5, fy = (yk - Gr.y0) / Gr.cell - 0.5;
  const i = Math.floor(fx), j = Math.floor(fy), tx = fx - i, ty = fy - j;
  const H = (a, b) => {
    if (wrapping) { a = wrapK(a, Gr.w); b = wrapK(b, Gr.h); } else { a = Math.max(0, Math.min(Gr.w - 1, a)); b = Math.max(0, Math.min(Gr.h - 1, b)); }
    return Gr.hgt[b * Gr.w + a];
  };
  return (H(i, j) * (1 - tx) + H(i + 1, j) * tx) * (1 - ty) + (H(i, j + 1) * (1 - tx) + H(i + 1, j + 1) * tx) * ty;
}

const riverWidth = (acc) => Math.max(2.5, 1.5 * acc ** 0.45);   // metres, from the area drained (km²)

let world = null;
/** The world, its start region, the camp on its river, and the camp's frame. */
export function zoomWorld() {
  if (world) return world;
  const G = worldGrid({ w: 1000, h: 500, seed: SEED });
  const S = findStart(G);
  const R = regionGrid({ xk: S.xk, yk: S.yk, size: 256, cell: 0.5, seed: SEED });
  // the camp stands by the river nearest the start that drains 1,000-5,000 km², so it shows from the region out
  let best = -1, bd = 1e9;
  for (let k = 0; k < R.n; k++) {
    if (R.acc[k] < 1000 || R.acc[k] > 5000 || R.hgt[k] <= 5) continue;
    const x = R.x0 + ((k % R.w) + 0.5) * R.cell, y = R.y0 + (Math.floor(k / R.w) + 0.5) * R.cell, d = Math.hypot(x - S.xk, y - S.yk);
    if (d < bd) { bd = d; best = k; }
  }
  const at = (k) => [R.x0 + ((k % R.w) + 0.5) * R.cell, R.y0 + (Math.floor(k / R.w) + 0.5) * R.cell];
  let up = best, down = best;
  for (let i = 0; i < 6; i++) if (R.down[down] >= 0) down = R.down[down];
  for (let i = 0; i < 6; i++) { const u = upstreamOf(R, up); if (u >= 0) up = u; }
  const [ux, uy] = at(up), [dx, dy] = at(down), L = Math.hypot(dx - ux, dy - uy) || 1;
  const dir = [(dx - ux) / L, (dy - uy) / L], river = at(best), nA = [-dir[1], dir[0]];
  // the camp is on the higher bank, under a cliff that runs along the valley
  const hA = elevation(river[0] + nA[0] * 1.2, river[1] + nA[1] * 1.2, 1, SEED), hB = elevation(river[0] - nA[0] * 1.2, river[1] - nA[1] * 1.2, 1, SEED);
  const side = hA > hB ? 1 : -1;
  const vdir = [-side * nA[0], -side * nA[1]], udir = [vdir[1], -vdir[0]];
  const W = { G, S, R, udir, vdir, riverAcc: R.acc[best], camp: { xk: river[0] + side * nA[0] * 0.3, yk: river[1] + side * nA[1] * 0.3 } };
  // place the camp 280 m from the main river as its smoothed line runs, measured in the camp's frame
  W.lines = frameRivers(W);
  let vr = null;
  for (const pts of W.lines) for (let i = 0; i < pts.length - 1; i++) {
    const a = pts[i], b = pts[i + 1];
    if (a[2] < 800 || (a[0] - 0) * (b[0] - 0) > 0) continue;
    const t = a[0] / (a[0] - b[0]), v = a[1] + (b[1] - a[1]) * t;
    if (v > -2000 && v < 2000 && (vr === null || Math.abs(v) < Math.abs(vr))) vr = v;
  }
  if (vr !== null) {
    const s = (vr - 280) / 1000;
    W.camp = { xk: W.camp.xk + W.vdir[0] * s, yk: W.camp.yk + W.vdir[1] * s };
    W.lines = frameRivers(W);
  }
  const yr = (16 * Math.PI) / 180, back = [Math.sin(yr) * udir[0] + Math.cos(yr) * vdir[0], Math.sin(yr) * udir[1] + Math.cos(yr) * vdir[1]];
  W.yawW = (Math.atan2(back[0], back[1]) * 180) / Math.PI;
  console.log('camp at', W.camp.xk.toFixed(2), W.camp.yk.toFixed(2), 'river', Math.round(W.riverAcc), 'km², at v', vr && vr.toFixed(0), '; lat', latitude(W.camp.yk).toFixed(1));
  world = W;
  return W;
}

/** The camp frame's point (u, v metres) in world km, and back. */
export const toWorld = (W, u, v) => [W.camp.xk + (u * W.udir[0] + v * W.vdir[0]) / 1000, W.camp.yk + (u * W.udir[1] + v * W.vdir[1]) / 1000];
export const fromWorld = (W, xk, yk) => {
  const dx = (xk - W.camp.xk) * 1000, dy = (yk - W.camp.yk) * 1000;
  return [dx * W.udir[0] + dy * W.udir[1], dx * W.vdir[0] + dy * W.vdir[1]];
};

/** Rivers draining 3 km² or more near the camp, smoothed, in the camp's frame: points [u, v, area, level, width]. */
function frameRivers(W) {
  const Rs = blurred(W.R, 1, false);
  return riverLines(W.R, 20)
    .map((l) => smoothLine(l.pts.map(([x, y, a]) => { const [u, v] = fromWorld(W, x, y); return [u, v, a]; }), 3))
    .filter((pts) => pts.some((p) => Math.abs(p[0]) < 45000 && Math.abs(p[1]) < 45000))
    .map((pts) => {
      let lv = 1e9;
      for (const p of pts) {
        const [x, y] = toWorld(W, p[0], p[1]);
        lv = Math.min(lv, gridHeight(Rs, x, y) - 1.5);   // the water never runs uphill
        p[3] = lv; p[4] = riverWidth(p[2]);
      }
      return pts;
    });
}

// --- the ground in the camp's frame --------------------------------------------------------------------------------
let land = null;
/**
 * The camp's land: height(u, v, fine) in metres (fine adds the camp zoom's last octave and the shelter's own shape),
 * cover(u, v) as { forest, pine, open, dry, rock }, the rivers with their water levels, and the camp's stream.
 */
export function zoomLand() {
  if (land) return land;
  const W = zoomWorld();
  const nat = (u, v, level) => { const [x, y] = toWorld(W, u, v); return elevation(x, y, level, SEED); };
  const H0 = nat(0, 0, 2);
  // the camp's stream: from a spring under the cliff along the meadow, then down to the main river
  const vr = 280;
  const stream = smoothLine([[-900, 30], [-600, 14], [-300, 11], [0, 10], [250, 13], [480, 26], [640, 80], [720, 170], [760, vr]], 3)
    .map(([u, v]) => [u, v, 6, 0, 3.4]);
  const lines = [...W.lines, stream], STREAM = lines.length - 1;
  // bucket grids of river segments, each segment in every cell its reach touches: all rivers in 250 m cells for their
  // channels and banks, the big ones again in 1 km cells reaching 4 km for the valleys they make
  const key = (i, j) => i * 100003 + j;
  const fill = (buckets, cell, s, reach) => {
    const { a, b } = s;
    const i0 = Math.floor((Math.min(a[0], b[0]) - reach) / cell), i1 = Math.floor((Math.max(a[0], b[0]) + reach) / cell);
    const j0 = Math.floor((Math.min(a[1], b[1]) - reach) / cell), j1 = Math.floor((Math.max(a[1], b[1]) + reach) / cell);
    for (let ii = i0; ii <= i1; ii++) for (let jj = j0; jj <= j1; jj++) { const k = key(ii, jj); if (!buckets.has(k)) buckets.set(k, []); buckets.get(k).push(s); }
  };
  const CELL = 250, buckets = new Map(), BCELL = 1000, BREACH = 4000, bigBuckets = new Map();
  lines.forEach((pts, line) => {
    for (let i = 0; i < pts.length - 1; i++) {
      const a = pts[i], b = pts[i + 1], reach = a[2] >= 500 ? 900 : a[2] >= 40 ? 260 : 60;
      if (Math.abs(a[0]) > 40000 && Math.abs(b[0]) > 40000) continue;
      const s = { a, b, reach, line };
      fill(buckets, CELL, s, reach);
      if (a[2] >= 500) fill(bigBuckets, BCELL, s, BREACH);
    }
  });
  // the stream's level follows the meadow it runs in
  for (const p of stream) p[3] = 1e9;
  const segNear = (s, u, v) => {
    const ax = s.a[0], ay = s.a[1], vx = s.b[0] - ax, vy = s.b[1] - ay, L2 = vx * vx + vy * vy || 1;
    const t = Math.max(0, Math.min(1, ((u - ax) * vx + (v - ay) * vy) / L2)), d = Math.hypot(u - ax - vx * t, v - ay - vy * t);
    // hw: the true half width; a caller may ask for a wider channel
    return { d, t, s, hw: (s.a[4] + (s.b[4] - s.a[4]) * t) / 2, reach: s.reach, area: s.a[2], level: s.a[3] + (s.b[3] - s.a[3]) * t, line: s.line };
  };
  /** Every river within its reach of a point, each by its nearest segment: [{ d, level, hw, reach, area, line }]. */
  const riversNear = (u, v) => {
    const list = buckets.get(key(Math.floor(u / CELL), Math.floor(v / CELL)));
    if (!list) return [];
    const best = new Map();
    for (const s of list) {
      const r = segNear(s, u, v);
      if (r.d > s.reach) continue;
      const cur = best.get(s.line);
      if (!cur || r.d - r.hw < cur.d - cur.hw) best.set(s.line, r);
    }
    return [...best.values()];
  };
  /** The nearest river at a point: { d, level, hw, reach, area, line } or null. */
  const nearRiver = (u, v) => riversNear(u, v).reduce((m, r) => (!m || r.d - r.hw < m.d - m.hw ? r : m), null);
  /** The nearest big river (500 km² or more) within 4 km, for the valley it makes. */
  const bigRiver = (u, v) => {
    const list = bigBuckets.get(key(Math.floor(u / BCELL), Math.floor(v / BCELL)));
    let best = null;
    for (const s of list || []) { const r = segNear(s, u, v); if (r.d <= BREACH && (!best || r.d - r.hw < best.d - best.hw)) best = r; }
    return best;
  };
  // the cliff along the valley's side: the shelter's line near the camp, wandering farther off; it fades out 3 km away
  const cliffV = (u) => -6 + 0.8 * Math.sin(u * 0.13) + 0.02 * u * smooth(-60, 60, -Math.abs(u) + 60) + 18 * Math.sin(u / 380) * smooth(60, 300, Math.abs(u)) + 0.04 * u * smooth(60, 600, Math.abs(u));
  const cliffH = (u) => (9.1 + 7 * smooth(80, 700, Math.abs(u)) + 5 * Math.sin(u / 230) * smooth(80, 700, Math.abs(u))) * (1 - smooth(2600, 3400, Math.abs(u)));
  const streamLevel = (u) => H0 - 0.7 - 0.0012 * Math.max(0, u + 900);

  // the shelter's stretch of the cliff, where its lower beds sit back under the overhang
  const shelterAt = (u) => smooth(-7, -5, u) * (1 - smooth(7, 9, u));
  // seen near, how far behind the cliff's foot the plateau's ground rises: inside the rock the cliff's blocks build
  const rimBack = (u) => 3 + 2.5 * shelterAt(u);

  const height = (u, v, fine = false, minHw = 0) => {
    let h = nat(u, v, fine ? 3 : 2);
    // the big river's valley: a floor about 650 m wide that rises gently from the water to the land's own slopes
    const B = bigRiver(u, v);
    if (B) {
      const top = B.level + 0.5 + Math.max(0, B.d - B.hw) * 0.006;
      if (h > top) h = top + (h - top) * smooth(B.hw, B.hw + 650, B.d) ** 1.3;
    }
    // the valley floor near the camp leans gently to the river and levels off just above it, so the meadow reads as
    // the shelter's
    const near = (1 - smooth(500, 1500, Math.abs(u))) * smooth(-200, -60, v) * (1 - smooth(300, 700, v));
    h = h * (1 - near * 0.85) + (H0 - Math.min(0.045 * Math.max(0, v - 4), 2.6) + 0.4 * (fbm(u * 0.07 + 2, v * 0.07 + 5, 3) - 0.5)) * near * 0.85;
    // the plateau above the cliff; seen near (fine), its rise sits a few metres behind the face, inside the rock the
    // cliff's blocks build there, so coarse ground never shows in front of them
    const cv = cliffV(u), ch = cliffH(u);
    if (ch > 0.1) {
      const back = fine ? rimBack(u) : 0;
      h += ch * smooth(0.6, -0.6, v - cv + back) + 0.3 * (fbm(u * 0.1, v * 0.1) - 0.5) * smooth(4, 8, cv - back - v);
    }
    // every river within reach cuts its channel; its banks come down toward the water and never lie below it
    for (const r of riversNear(u, v)) {
      const hw = Math.max(r.hw, minHw), L = r.line === STREAM ? streamLevel(u) : r.level;
      const depth = 0.5 + hw * 0.06, floor = r.area >= 500 ? 60 : r.area >= 40 ? 160 : 6;
      if (r.d < hw) { h = Math.min(h, L - depth * (1 - (r.d / hw) ** 2)); continue; }
      const top = L + 0.5 + (r.d - hw) * 0.006;
      if (h > top) h = top + (h - top) * smooth(hw, hw + floor, r.d) ** 1.3;
      else h += Math.max(0, L + 0.35 - h) * (1 - smooth(hw + floor * 0.3, r.reach, r.d));
    }
    return h;
  };
  const waterLevel = (pts) => pts.map((p) => (pts === stream ? streamLevel(p[0]) : p[3]));

  // what grows where: woods by the region's cover, clearings, the open floodplain, the meadow round the camp
  const coverCache = new Map();
  const cover = (u, v) => {
    const [x, y] = toWorld(W, u, v), R = W.R;
    const i = Math.floor((x - R.x0) / R.cell), j = Math.floor((y - R.y0) / R.cell), k = Math.max(0, Math.min(R.n - 1, j * R.w + i));
    const b = R.biome[k];
    // woods and grassland in a patchwork: the region's cover sets the share, noise the patches
    let f = FOREST.has(b) ? 0.62 : b === BIOME.GRASS ? 0.42 : b === BIOME.STEPPE || b === BIOME.SCRUB ? 0.16 : 0;
    const n = fbmW(x, y, 2, 3, SEED + 101), n2 = fbmW(x, y, 0.4, 2, SEED + 103);
    f = Math.max(0, Math.min(1, f + n * 1.5 + n2 * 0.6));
    const B = bigRiver(u, v);
    if (B) f *= B.d < B.hw + 45 ? 1.2 : smooth(B.hw + 120, B.hw + 650, B.d);   // willows on the banks, meadow on the floor
    // the camp's meadow, the open ground at the cliff top, and woods on the plateau behind it, so the cliff reads as
    // the edge between woods above and meadow below; their far edges wander, as a wood's edge does
    const wig = (fbm(u / 150 + 7, v / 150 + 3) - 0.5) * 400, au = Math.abs(u) + wig;
    const meadow = (1 - smooth(300, 600, au)) * smooth(-14, -4, v) * (1 - smooth(200, 260, v + wig * 0.15));
    const ledge = smooth(-30, -22, v) * (1 - smooth(-12, -6, v)) * (1 - smooth(500, 900, au));
    const top = smooth(-26, -40, v) * smooth(-900, -600, v - wig) * (1 - smooth(500, 900, au));
    f = Math.max(f, 0.8 * top) * (1 - meadow * 0.9) * (1 - ledge * 0.7);
    const dry = b === BIOME.STEPPE || b === BIOME.SCRUB ? 1 : 0, rock = b === BIOME.ROCK ? 1 : 0;
    return { forest: Math.max(0, Math.min(1, f)), pine: b === BIOME.BOREAL ? 0.9 : 0.25, open: 1 - f, dry, rock };
  };
  land = { W, H0, height, cover, nearRiver, bigRiver, lines, stream, waterLevel, cliffV, cliffH, streamLevel, shelterAt, rimBack };
  return land;
}

// the camps of the region: ours, and a few other bands on other rivers (PRE-28: camps show at every distance)
export function regionCamps(W) {
  const out = [{ xk: W.camp.xk, yk: W.camp.yk, colour: '#b44f39', ours: true }];
  const R = W.R, picks = [[24, -31, '#4c70a6'], [-38, 18, '#c1913d'], [46, 27, '#80b882'], [-12, 52, '#aa6bbd']];
  for (const [dx, dy, colour] of picks) {
    let best = -1, bd = 1e9;
    for (let k = 0; k < R.n; k++) {
      if (R.acc[k] < 40 || R.hgt[k] <= 5) continue;
      const x = R.x0 + ((k % R.w) + 0.5) * R.cell, y = R.y0 + (Math.floor(k / R.w) + 0.5) * R.cell;
      const d = Math.hypot(x - (W.camp.xk + dx), y - (W.camp.yk + dy));
      if (d < bd) { bd = d; best = k; }
    }
    out.push({ xk: R.x0 + ((best % R.w) + 0.5) * R.cell + 0.2, yk: R.y0 + (Math.floor(best / R.w) + 0.5) * R.cell, colour });
  }
  return out;
}
