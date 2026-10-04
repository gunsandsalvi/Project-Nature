// The card atlas: small pictures drawn pixel by pixel at the size they show on screen (one texel, one art pixel).
// They are drawn in metres: a picture keeps its size in the world at every zoom (f texels for each texel of the
// design, f = MPP_REF / mpp), so a tuft, a reed or a flame stands the same against a person in every scene.
// Each texel holds a bump (red, green), a step bias (blue: 128 is none, each 40 is one step) and a mask (alpha).
import * as THREE from 'three';
import { Rand } from './geo.js';
import { LOOK } from './look.js';

export const T = 64, GRID = 8, MPP_REF = 0.05;
// FLAME holds small, middle and large flames (about 0.35, 0.55 and 0.8 m), FLAMEB other shapes of the same sizes
export const TILE = {
  LEAF: [0, 1, 2, 3], CONIFER: [4, 5], TUFT: [6, 7, 8], FLOWER: [9, 10], REED: 11, GRAIN: 12, HERB: 13,
  FLAME: [14, 15, 16], FERN: 17, SEDGE: 18, LEAFSMALL: [19, 20], TWIG: 21, DRYTUFT: [22, 23], DOT: 24, FLAMEB: [25, 26, 27],
};
const C = T / 2;

function makeTile(data, id, draw) {
  const ox = (id % GRID) * T, oy = Math.floor(id / GRID) * T, W = T * GRID;
  const set = (x, y, nx = 0, ny = 0, b = 0) => {
    x = Math.round(x); y = Math.round(y);
    if (x < 0 || y < 0 || x >= T || y >= T) return;
    const i = ((oy + y) * W + ox + x) * 4;
    data[i] = 128 + Math.max(-127, Math.min(127, nx * 127));
    data[i + 1] = 128 + Math.max(-127, Math.min(127, ny * 127));
    data[i + 2] = Math.max(0, Math.min(255, 128 + b * 40));
    data[i + 3] = 255;
  };
  draw(set);
}

// A clump of leaves: overlapping round leaves, upper ones in front, a dark line under each (scallops). The look
// sets how many and how big: 'busy' many small leaves each with a lit cap; 'clean' and 'big' fewer, larger ones,
// lit as one clump so its light and shade read at a glance.
function leafClump(seed, f, small) {
  const mode = LOOK.leaf;
  const P = {
    busy: small ? [7, 1.6, 2.6, 9, 0.45, 0.6] : [12, 2.2, 3.8, 15, 0.45, 0.6],
    clean: small ? [7, 2.4, 3.6, 5, 0.18, 0.75] : [12, 3.4, 5.4, 8, 0.18, 0.75],
    big: small ? [9, 3, 4.5, 5, 0.15, 0.8] : [15, 4.2, 6.4, 9, 0.15, 0.8],
  }[mode] || [12, 2.2, 3.8, 15, 0.45, 0.6];
  const [R0, rmin0, rmax0, count, leafK, clumpK] = P;
  const R = R0 * f, rmin = rmin0 * f, rmax = rmax0 * f;
  return (set) => {
    const r = new Rand(seed);
    const leaves = [];
    for (let i = 0; i < count; i++) {
      const a = r.range(0, Math.PI * 2), d = Math.sqrt(r.next()) * (R - rmax);
      leaves.push({ x: C + Math.cos(a) * d, y: C + Math.sin(a) * d * 0.85, r: r.range(rmin, rmax) });
    }
    leaves.sort((a, b) => a.y - b.y); // lower first, upper drawn over them
    const owner = new Int32Array(T * T).fill(-1);
    leaves.forEach((L, k) => {
      for (let y = Math.floor(L.y - L.r); y <= L.y + L.r; y++) for (let x = Math.floor(L.x - L.r); x <= L.x + L.r; x++) {
        if ((x - L.x) ** 2 + ((y - L.y) * 1.1) ** 2 <= L.r * L.r && x >= 0 && y >= 0 && x < T && y < T) owner[y * T + x] = k;
      }
    });
    for (let y = 0; y < T; y++) for (let x = 0; x < T; x++) {
      const k = owner[y * T + x]; if (k < 0) continue;
      const L = leaves[k];
      const lx = (x - L.x) / L.r, ly = (y - L.y) / L.r;
      const gx = (x - C) / R, gy = (y - C) / R;
      let b = 0;
      const below = y > 0 ? owner[(y - 1) * T + x] : -1;
      if (below !== k && below >= 0) b = -1;           // a line where this leaf overlaps the one beneath
      else if (mode === 'busy' ? (ly > 0.45 && lx < 0.1) : (ly > 0.5 && lx < 0 && gy > 0.2 && gx < 0.15)) b = 0.5; // a lit cap
      set(x, y, lx * leafK + gx * clumpK, ly * leafK + gy * clumpK, b);
    }
  };
}

// A tier of needles: a wide, drooping band with a ragged lower edge.
function coniferTier(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    const top = C + 10 * f, bot = C - 10 * f, span = top - bot;
    const ragged = Array.from({ length: T }, () => r.int(0, Math.round(4 * f)));
    for (let y = Math.floor(bot - 4 * f); y <= top; y++) for (let x = 0; x < T; x++) {
      const t = (top - y) / span;                       // 0 at the tip, 1 at the base
      const half = (2 + t * 13) * f;
      if (Math.abs(x - C) > half) continue;
      if (y < bot + ragged[x] - (Math.abs(x - C) > half - 3 * f ? 2 * f : 0)) continue;
      let b = 0;
      if (y < bot + ragged[x] + 1) b = -1;
      else if (LOOK.leaf === 'busy' && (x + y * 3) % 7 === 0 && t > 0.2) b = -1;
      else if (y > top - 4 * f) b = 0.5;
      set(x, y, (x - C) / (16 * f), 0.2 + (y - bot) / (30 * f), b);
    }
  };
}

// A grass tuft: a fan of blades from one root.
function tuft(seed, f, n = 7, hmin = 5, hmax = 12, spread = 0.9) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < n; i++) {
      const ang = (i / (n - 1) - 0.5) * spread + r.range(-0.12, 0.12), h = r.range(hmin, hmax) * f, bend = r.range(-0.25, 0.25);
      for (let s = 0; s <= h; s++) {
        const t = s / h, a = ang + bend * t * t;
        const x = C + Math.sin(a) * s, y = Math.cos(a) * s;
        set(x, y, 0, 0, t > 0.78 ? 1 : (t < 0.25 ? -1 : 0));
        if (t < 0.3) set(x + 1, y, 0, 0, -1);
      }
    }
  };
}

// Small blossoms held above the grass.
function flower(seed, f, kind) {
  return (set) => {
    const r = new Rand(seed);
    const heads = r.int(2, 4);
    for (let i = 0; i < heads; i++) {
      const x = C + r.int(-5, 5) * f, y = r.int(2, 6) * f;
      if (kind === 0) { // small cross-shaped blossom
        set(x, y, 0, 0, 1); set(x - 1, y, 0, 0, 0); set(x + 1, y, 0, 0, 0); set(x, y + 1, 0, 0, 1); set(x, y - 1, 0, 0, -1);
      } else { set(x, y, 0, 0, 1); set(x + 1, y, 0, 0, 0); set(x, y + 1, 0, 0, 1); set(x + 1, y + 1, 0, 0, 0); }
    }
  };
}

// Reeds about 0.9 to 1.8 m tall, some with a dark seed head.
function reeds(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 9; i++) {
      const x0 = C + r.range(-6, 6) * f, h = r.range(18, 36) * f, lean = r.range(-0.18, 0.18);
      for (let s = 0; s <= h; s++) set(x0 + lean * s, s, 0, 0, s > h - 3 ? 1 : (s < 4 * f ? -1 : 0));
      if (r.chance(0.4)) for (let s = h - 4 * f; s <= h; s++) { set(x0 + lean * s, s, 0, 0, -2); set(x0 + lean * s + 1, s, 0, 0, -2); }
    }
  };
}

// Wild grain or a crop, about 0.7 to 1.1 m, with ears.
function grain(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 7; i++) {
      const x0 = C + r.range(-7, 7) * f, h = r.range(14, 22) * f, lean = r.range(-0.2, 0.25);
      for (let s = 0; s <= h; s++) set(x0 + lean * s, s, 0, 0, s < 3 * f ? -1 : 0);
      for (let s = h - 4 * f; s <= h + 1; s++) { set(x0 + lean * s, s, 0, 0, 1); if (Math.round(s) % 2) set(x0 + lean * s + 1, s, 0, 0, 0); }
    }
  };
}

function herb(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 8; i++) {
      const a = (i / 8) * Math.PI - Math.PI / 2 + r.range(-0.2, 0.2), L = r.range(5, 9) * f;
      for (let s = 0; s <= L; s++) {
        const x = C + Math.sin(a) * s * 1.2, y = 1 + Math.cos(a) * s * 0.6 + s * 0.15;
        set(x, y, Math.sin(a) * 0.5, 0.3, s > L - 2 ? 1 : 0); set(x, y + 1, Math.sin(a) * 0.5, 0.5, 0);
      }
    }
  };
}

function fern(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 6; i++) {
      const a = (i / 5 - 0.5) * 2.2, L = r.range(8, 13) * f;
      for (let s = 0; s <= L; s++) {
        const t = s / L, x = C + Math.sin(a) * s, y = Math.cos(a) * s * 0.8 - t * t * 3 * f + 2;
        set(x, y, Math.sin(a) * 0.4, 0.4, t > 0.8 ? 1 : 0);
        if (Math.round(s) % 2 === 0 && t < 0.85) { set(x + Math.cos(a) * 1.5, y - Math.sin(a) * 1.5, 0, 0.3, -1); set(x - Math.cos(a) * 1.5, y + Math.sin(a) * 1.5, 0, 0.3, 0); }
      }
    }
  };
}

// A flame H design texels tall: bands from a white-yellow core to a red edge; blue gives the step offset from 3.
function flameTile(seed, f, H) {
  return (set) => {
    const r = new Rand(seed);
    const h = H * f, w0 = h * 0.3;
    const lick = Array.from({ length: 4 }, () => [r.range(-0.15, 0.15) * h, r.range(0.45, 0.95) * h]);
    for (let y = 0; y < T; y++) for (let x = 0; x < T; x++) {
      const dx = x - C, base = Math.max(0, 1 - y / h);
      const w = w0 * Math.sqrt(base) * (1 - Math.max(0, (y - h * 0.65) / (h * 0.4)));
      let inside = Math.abs(dx) < w;
      for (const [lx, lh] of lick) if (y < lh && Math.abs(dx - lx * (y / lh)) < Math.max(0.8, w0 * 0.3 * (1 - y / lh))) inside = true;
      if (!inside) continue;
      const core = Math.abs(dx) / Math.max(w, 1) + y / (h * 1.15);
      const b = core < 0.35 ? 3 : core < 0.6 ? 2 : core < 0.85 ? 1 : 0;
      set(x, y, 0, 0, b);
    }
  };
}

function sedge(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 11; i++) {
      const ang = r.range(-0.6, 0.6), h = r.range(8, 16) * f, bend = r.range(-0.5, 0.5);
      for (let s = 0; s <= h; s++) { const t = s / h, a = ang + bend * t * t; set(C + Math.sin(a) * s, Math.cos(a) * s, 0, 0, t > 0.8 ? 1 : (t < 0.2 ? -1 : 0)); }
    }
  };
}

function twig(seed, f) {
  return (set) => {
    const r = new Rand(seed);
    const grow = (x, y, a, len, depth) => {
      for (let s = 0; s < len; s++) { x += Math.sin(a); y += Math.cos(a); set(x, y, 0, 0, depth > 1 ? 0 : -1); }
      if (depth < 3) { grow(x, y, a - r.range(0.3, 0.7), len * 0.65, depth + 1); grow(x, y, a + r.range(0.3, 0.7), len * 0.65, depth + 1); }
    };
    grow(C, 0, r.range(-0.1, 0.1), 9 * f, 0);
  };
}

/** The atlas for a picture drawn at mpp metres per art pixel. */
export function makeAtlas(mpp = MPP_REF) {
  const f = MPP_REF / mpp;
  const W = T * GRID, data = new Uint8Array(W * W * 4);
  TILE.LEAF.forEach((id, i) => makeTile(data, id, leafClump(11 + i * 7, f, false)));
  TILE.CONIFER.forEach((id, i) => makeTile(data, id, coniferTier(41 + i * 5, f)));
  TILE.TUFT.forEach((id, i) => makeTile(data, id, tuft(61 + i * 3, f, 6 + i, 4 + i, 9 + i * 2)));
  TILE.FLOWER.forEach((id, i) => makeTile(data, id, flower(81 + i, f, i)));
  makeTile(data, TILE.REED, reeds(91, f));
  makeTile(data, TILE.GRAIN, grain(93, f));
  makeTile(data, TILE.HERB, herb(95, f));
  TILE.FLAME.forEach((id, i) => makeTile(data, id, flameTile(101 + i * 3, f, [7, 11, 16][i])));
  TILE.FLAMEB.forEach((id, i) => makeTile(data, id, flameTile(151 + i * 3, f, [6, 10, 15][i])));
  makeTile(data, TILE.FERN, fern(111, f));
  makeTile(data, TILE.SEDGE, sedge(113, f));
  TILE.LEAFSMALL.forEach((id, i) => makeTile(data, id, leafClump(121 + i * 5, f, true)));
  makeTile(data, TILE.TWIG, twig(131, f));
  TILE.DRYTUFT.forEach((id, i) => makeTile(data, id, tuft(141 + i * 3, f, 9, 3, 8 + i * 3, 1.3)));
  makeTile(data, TILE.DOT, (set) => { for (let y = 0; y < T; y++) for (let x = 0; x < T; x++) set(x, y, 0, 0, 0); });
  const t = new THREE.DataTexture(data, W, W, THREE.RGBAFormat);
  t.magFilter = t.minFilter = THREE.NearestFilter;
  t.needsUpdate = true;
  return t;
}
