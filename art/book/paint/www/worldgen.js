// The world for the zoom stops (PRE-03): a small planet 2,000 km around and 1,000 km from pole to pole that wraps
// both ways (WLD-01, WLD-03), held in world cells (WLD-12): continents and mountains, a climate by latitude, plant
// cover, rivers that drain to the sea and the polar ice along the seam. The same ground function, with finer
// octaves, gives the region's and the valley's detail, so every stop shows one consistent place.
// Units: x and y in km on the world (x east, y south; y = 0 and y = 1000 are the polar seam), heights in metres.

export const WORLD = { W: 2000, H: 1000 };

// --- noise that wraps round the world ---------------------------------------------------------------------------
function ihash(x, y, s) {
  let h = (Math.imul(x | 0, 374761393) + Math.imul(y | 0, 668265263) + Math.imul(s | 0, 1442695041)) | 0;
  h = Math.imul(h ^ (h >>> 13), 1274126177);
  h ^= h >>> 16;
  return h >>> 0;
}
const GR = Array.from({ length: 16 }, (_, i) => [Math.cos((i * Math.PI) / 8), Math.sin((i * Math.PI) / 8)]);
const wrap = (i, p) => ((i % p) + p) % p;

/** Gradient noise on a lattice that repeats every px by py cells; about -0.7..0.7. */
export function pnoise(x, y, px, py, s) {
  const x0 = Math.floor(x), y0 = Math.floor(y), fx = x - x0, fy = y - y0;
  const g = (i, j) => GR[ihash(wrap(i, px), wrap(j, py), s) & 15];
  const ga = g(x0, y0), gb = g(x0 + 1, y0), gc = g(x0, y0 + 1), gd = g(x0 + 1, y0 + 1);
  const a = ga[0] * fx + ga[1] * fy, b = gb[0] * (fx - 1) + gb[1] * fy;
  const c = gc[0] * fx + gc[1] * (fy - 1), d = gd[0] * (fx - 1) + gd[1] * (fy - 1);
  const u = fx * fx * fx * (fx * (fx * 6 - 15) + 10), v = fy * fy * fy * (fy * (fy * 6 - 15) + 10);
  return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
}

/** Layered noise over the world: base is the first layer's lattice in km (it must divide 1,000). About -0.6..0.6. */
export function fbmW(xk, yk, base, oct, s, gain = 0.5) {
  let px = Math.round(WORLD.W / base), py = Math.round(WORLD.H / base), f = 1 / base, a = 1, sum = 0, n = 0;
  for (let k = 0; k < oct; k++) {
    sum += a * pnoise(xk * f, yk * f, px, py, s + k * 31);
    n += a; a *= gain; f *= 2; px *= 2; py *= 2;
  }
  return sum / n;
}

const smooth = (a, b, x) => { const t = Math.max(0, Math.min(1, (x - a) / (b - a))); return t * t * (3 - 2 * t); };

// --- the ground ---------------------------------------------------------------------------------------------------
let seaLine = null;
const cont = (xk, yk, seed) => fbmW(xk, yk, 250, 6, seed + 11) * 0.8 + fbmW(xk, yk, 100, 4, seed + 23) * 0.35;

/** The height in metres at a point of the world; level adds finer octaves: 0 world cells, 1 region, 2 valley, 3 camp. */
export function elevation(xk, yk, level = 0, seed = 1) {
  if (seaLine === null || seaLine.seed !== seed) {
    // the sea level that leaves about 37% of the world dry land
    const v = [];
    for (let j = 0; j < 100; j++) for (let i = 0; i < 200; i++) v.push(cont(i * 10 + 5, j * 10 + 5, seed));
    v.sort((a, b) => a - b);
    seaLine = { seed, t: v[Math.floor(v.length * 0.63)] };
  }
  const d = cont(xk, yk, seed) - seaLine.t;
  let e;
  if (d < 0) e = d > -0.03 ? d * 4000 : -120 + (d + 0.03) * 14000;          // a shelf, then the deep sea
  else {
    const n = fbmW(xk, yk, 125, 4, seed + 41);
    const ridge = (1 - Math.min(1, Math.abs(n) * 1.7)) ** 2.5;
    const prov = smooth(0.0, 0.16, fbmW(xk, yk, 500, 2, seed + 53));          // where the mountain chains run
    e = 30 + d * 2200 + ridge * prov * 2600 * smooth(0.02, 0.1, d) + fbmW(xk, yk, 40, 4, seed + 67) * 320 * smooth(0, 0.05, d);
  }
  if (level >= 1) e += fbmW(xk, yk, 10, 4, seed + 79) * 90 * (e > 0 ? 1 : 0.3);
  if (level >= 2) e += (fbmW(xk, yk, 2, 4, seed + 83) * 34 + fbmW(xk, yk, 0.2, 3, seed + 89) * 5) * (e > 0 ? 1 : 0.3);
  if (level >= 3) e += fbmW(xk, yk, 0.04, 3, seed + 97) * 1.2;
  return e;
}

// --- climate and plant cover ----------------------------------------------------------------------------------------
export const BIOME = { SEA: 0, ICE: 1, TUNDRA: 2, BOREAL: 3, MIXED: 4, BROADLEAF: 5, GRASS: 6, STEPPE: 7, SCRUB: 8,
  DESERT: 9, SAVANNA: 10, RAIN: 11, ROCK: 12, WET: 13 };
// the flat colour of each cover on the map look (PRE-29): a material and a step offset
export const COVER = {
  [BIOME.ICE]: ['snow', 1], [BIOME.TUNDRA]: ['moss', 0], [BIOME.BOREAL]: ['pine', 0], [BIOME.MIXED]: ['pine', 1],
  [BIOME.BROADLEAF]: ['leaf', 0], [BIOME.GRASS]: ['meadow', 0], [BIOME.STEPPE]: ['drygrass', 0], [BIOME.SCRUB]: ['moss', -1],
  [BIOME.DESERT]: ['sand', 0], [BIOME.SAVANNA]: ['drygrass', 1], [BIOME.RAIN]: ['grass', -1], [BIOME.ROCK]: ['rock', 0],
  [BIOME.WET]: ['reed', -1], [BIOME.SEA]: ['sea', 0],
};
export const FOREST = new Set([BIOME.BOREAL, BIOME.MIXED, BIOME.BROADLEAF, BIOME.RAIN]);

export const latitude = (yk) => 90 - yk * 0.18;

/** Mean temperature (deg C) and wetness (0..1) at a point; coast: closeness to the sea, 0..1. */
export function climate(xk, yk, e, coast = 0, seed = 1) {
  const al = Math.abs(latitude(yk));
  const T = 28 - 0.55 * al - 0.0065 * Math.max(0, e) + fbmW(xk, yk, 250, 2, seed + 61) * 5;
  const M = 0.52 + 0.55 * fbmW(xk, yk, 200, 4, seed + 71) - 0.28 * Math.exp(-(((al - 25) / 9) ** 2))
    + 0.08 * Math.exp(-(((al - 50) / 12) ** 2)) + 0.16 * coast;
  return { T, M, al };
}

export function biomeOf(T, M, e, al) {
  if (al > 72) return BIOME.ICE;
  if (e <= 0) return BIOME.SEA;
  if (T < -9) return BIOME.ICE;
  if (e > 2100 || (e > 1600 && T < 1)) return BIOME.ROCK;
  if (T < -1) return BIOME.TUNDRA;
  if (T < 5) return M > 0.35 ? BIOME.BOREAL : BIOME.TUNDRA;
  if (T < 11) return M > 0.5 ? BIOME.MIXED : M > 0.36 ? BIOME.GRASS : M > 0.22 ? BIOME.STEPPE : BIOME.SCRUB;
  if (T < 18) return M > 0.5 ? BIOME.BROADLEAF : M > 0.35 ? BIOME.GRASS : M > 0.2 ? BIOME.STEPPE : M > 0.12 ? BIOME.SCRUB : BIOME.DESERT;
  return M > 0.6 ? BIOME.RAIN : M > 0.3 ? BIOME.SAVANNA : M > 0.15 ? BIOME.SCRUB : BIOME.DESERT;
}

// --- water: filling hollows, flow and rivers --------------------------------------------------------------------------
class Heap {
  constructor() { this.k = []; this.v = []; }
  push(key, val) {
    const k = this.k, v = this.v; let i = k.length; k.push(key); v.push(val);
    while (i > 0) { const p = (i - 1) >> 1; if (k[p] <= key) break; k[i] = k[p]; v[i] = v[p]; i = p; }
    k[i] = key; v[i] = val;
  }
  pop() {
    const k = this.k, v = this.v, top = v[0], lk = k.pop(), lv = v.pop(), n = k.length;
    if (n) {
      let i = 0;
      for (;;) { let c = 2 * i + 1; if (c >= n) break; if (c + 1 < n && k[c + 1] < k[c]) c++; if (k[c] >= lk) break; k[i] = k[c]; v[i] = v[c]; i = c; }
      k[i] = lk; v[i] = lv;
    }
    return top;
  }
  get size() { return this.k.length; }
}

const D8 = [[1, 0], [-1, 0], [0, 1], [0, -1], [1, 1], [-1, 1], [1, -1], [-1, -1]];

/**
 * Water on a grid of heights h (w by h cells, each `area` km²): hollows filled to their spill height (lakes),
 * each cell's flow to its lowest neighbour, and the area draining through each cell. wrapX/wrapY: the grid wraps.
 * Sea cells (height 0 or below) and, unless wrapped, the edges drain away. Returns { fill, down, acc }.
 */
export function water(hgt, w, h, area, { wrapX = false, wrapY = false } = {}) {
  const n = w * h, fill = new Float32Array(n), done = new Uint8Array(n), down = new Int32Array(n).fill(-1);
  const heap = new Heap();
  const nb = (i, k) => {
    let x = (i % w) + D8[k][0], y = ((i / w) | 0) + D8[k][1];
    if (wrapX) x = wrap(x, w); else if (x < 0 || x >= w) return -1;
    if (wrapY) y = wrap(y, h); else if (y < 0 || y >= h) return -1;
    return y * w + x;
  };
  for (let i = 0; i < n; i++) {
    const x = i % w, y = (i / w) | 0;
    const edge = (!wrapX && (x === 0 || x === w - 1)) || (!wrapY && (y === 0 || y === h - 1));
    if (hgt[i] <= 0 || edge) { fill[i] = hgt[i]; done[i] = 1; heap.push(hgt[i], i); }
  }
  // priority flood: from the sea and the edges inward, each cell no lower than the one it drains to (a little higher)
  while (heap.size) {
    const i = heap.pop();
    for (let k = 0; k < 8; k++) {
      const j = nb(i, k);
      if (j < 0 || done[j]) continue;
      done[j] = 1;
      fill[j] = Math.max(hgt[j], fill[i] + 1e-3);
      down[j] = i;
      heap.push(fill[j], j);
    }
  }
  // area draining through each cell, highest first
  const order = Array.from({ length: n }, (_, i) => i).sort((a, b) => fill[b] - fill[a]);
  const acc = new Float32Array(n).fill(area);
  for (const i of order) if (down[i] >= 0 && hgt[i] > 0) acc[down[i]] += acc[i];
  return { fill, down, acc };
}

/** River lines: from every land cell draining at least minArea km², a segment to the cell it flows into. */
export function riverSegments(grid, minArea) {
  const out = [];
  for (let i = 0; i < grid.n; i++) {
    if (grid.acc[i] < minArea || grid.hgt[i] <= 0 || grid.down[i] < 0) continue;
    out.push([i, grid.down[i], grid.acc[i]]);
  }
  return out;
}

// --- the world in cells ------------------------------------------------------------------------------------------------
/** The whole world on a grid of w by h cells (default 2 km), with heights, cover, water and rivers. */
export function worldGrid({ w = 1000, h = 500, seed = 1 } = {}) {
  const n = w * h, cx = WORLD.W / w, cy = WORLD.H / h;
  const hg = new Float32Array(n), biome = new Uint8Array(n), coast = new Float32Array(n), T = new Float32Array(n);
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) hg[j * w + i] = elevation((i + 0.5) * cx, (j + 0.5) * cy, 0, seed);
  // closeness to the sea: a breadth-first walk from the sea, about 150 km deep
  const dist = new Int32Array(n).fill(1 << 30), q = [];
  for (let i = 0; i < n; i++) if (hg[i] <= 0) { dist[i] = 0; q.push(i); }
  for (let qi = 0; qi < q.length; qi++) {
    const i = q[qi], x = i % w, y = (i / w) | 0;
    for (const [dx, dy] of D8.slice(0, 4)) {
      const j = wrap(y + dy, h) * w + wrap(x + dx, w);
      if (dist[j] > dist[i] + 1) { dist[j] = dist[i] + 1; q.push(j); }
    }
  }
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    const k = j * w + i, xk = (i + 0.5) * cx, yk = (j + 0.5) * cy;
    coast[k] = Math.exp(-(dist[k] * cx) / 120);
    const c = climate(xk, yk, hg[k], coast[k], seed);
    T[k] = c.T;
    biome[k] = biomeOf(c.T, c.M, hg[k], c.al);
  }
  const wat = water(hg, w, h, cx * cy, { wrapX: true, wrapY: true });
  return { w, h, n, cx, cy, x0: 0, y0: 0, cell: cx, hgt: hg, biome, coast, T, ...wat, seed, dist };
}

/** A good start: temperate, wooded or open land by a middling river, some way inland (WLD-24, picture only). */
export function findStart(G) {
  let best = null, bestScore = -1e9;
  for (let k = 0; k < G.n; k++) {
    const b = G.biome[k];
    if (![BIOME.MIXED, BIOME.BROADLEAF, BIOME.GRASS].includes(b)) continue;
    const acc = G.acc[k], d = G.dist[k] * G.cx, e = G.hgt[k];
    if (acc < 300 || acc > 6000 || d < 25 || d > 110 || e < 40 || e > 500) continue;
    const y = Math.floor(k / G.w), al = Math.abs(latitude((y + 0.5) * G.cy));
    if (al < 25 || al > 48) continue;
    const score = -Math.abs(Math.log(acc / 1500)) - Math.abs(G.T[k] - 10) * 0.2 - Math.abs(d - 50) / 60 + (b === BIOME.MIXED ? 0.3 : 0);
    if (score > bestScore) { bestScore = score; best = k; }
  }
  const x = best % G.w, y = Math.floor(best / G.w);
  return { k: best, xk: (x + 0.5) * G.cx, yk: (y + 0.5) * G.cy };
}

/** A region of the world round a point, on a finer grid (default 0.5 km), with its own water and rivers. */
export function regionGrid({ xk, yk, size = 256, cell = 0.5, seed = 1 }) {
  const w = Math.round(size / cell), h = w, n = w * h, x0 = xk - size / 2, y0 = yk - size / 2;
  const hg = new Float32Array(n), biome = new Uint8Array(n);
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) hg[j * w + i] = elevation(x0 + (i + 0.5) * cell, y0 + (j + 0.5) * cell, 1, seed);
  const dist = new Int32Array(n).fill(1 << 30), q = [];
  for (let i = 0; i < n; i++) if (hg[i] <= 0) { dist[i] = 0; q.push(i); }
  for (let qi = 0; qi < q.length; qi++) {
    const i = q[qi], x = i % w, y = (i / w) | 0;
    for (const [dx, dy] of D8.slice(0, 4)) {
      const xx = x + dx, yy = y + dy;
      if (xx < 0 || yy < 0 || xx >= w || yy >= h) continue;
      const j = yy * w + xx;
      if (dist[j] > dist[i] + 1) { dist[j] = dist[i] + 1; q.push(j); }
    }
  }
  const coastAt = (k) => (dist[k] >= 1 << 29 ? 0 : Math.exp(-(dist[k] * cell) / 120));
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    const k = j * w + i, c = climate(x0 + (i + 0.5) * cell, y0 + (j + 0.5) * cell, hg[k], coastAt(k), seed);
    biome[k] = biomeOf(c.T, c.M, hg[k], c.al);
  }
  const wat = water(hg, w, h, cell * cell);
  return { w, h, n, cell, x0, y0, hgt: hg, biome, ...wat, seed };
}

/** The cell of a grid (x0, y0, cell, w, h) at a world point, or -1. */
export function cellAt(G, xk, yk) {
  const i = Math.floor((xk - G.x0) / G.cell), j = Math.floor((yk - G.y0) / G.cell);
  return i < 0 || j < 0 || i >= G.w || j >= G.h ? -1 : j * G.w + i;
}

/**
 * Rivers of a region as smooth lines in km: each line follows cells draining at least minArea km² down to the sea,
 * a lake or the region's edge; returns [{ pts: [[xk, yk, acc]], ... }].
 */
export function riverLines(G, minArea) {
  const isRiver = (k) => G.acc[k] >= minArea && G.hgt[k] > 0;
  const ups = new Int32Array(G.n);
  for (let k = 0; k < G.n; k++) if (isRiver(k) && G.down[k] >= 0) ups[G.down[k]]++;
  const lines = [], seen = new Uint8Array(G.n);
  const xy = (k) => [G.x0 + ((k % G.w) + 0.5) * G.cell, G.y0 + (Math.floor(k / G.w) + 0.5) * G.cell, G.acc[k]];
  for (let k = 0; k < G.n; k++) {
    if (!isRiver(k) || ups[k] === 1) continue;  // start at springs and below joins
    let c = k; const pts = [xy(c)];
    while (G.down[c] >= 0) {
      const d = G.down[c];
      pts.push(xy(d));
      if (seen[d] || !isRiver(d) || ups[d] !== 1) break;
      seen[d] = 1; c = d;
    }
    lines.push({ pts });
  }
  return lines;
}

/** Corner cutting: smooths a line of points (keeps the ends). */
export function smoothLine(pts, times = 3) {
  for (let t = 0; t < times; t++) {
    const out = [pts[0]];
    for (let i = 0; i < pts.length - 1; i++) {
      const a = pts[i], b = pts[i + 1];
      out.push(a.map((v, k) => v * 0.75 + b[k] * 0.25), a.map((v, k) => v * 0.25 + b[k] * 0.75));
    }
    out.push(pts[pts.length - 1]);
    pts = out;
  }
  return pts;
}
