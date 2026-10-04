// The card atlas: small pictures drawn pixel by pixel at the size they show on screen (one texel, one art pixel).
// Each texel holds a bump (red, green), a step bias (blue: 128 is none, each 40 is one step) and a mask (alpha).
import * as THREE from 'three';
import { Rand } from './geo.js';

export const T = 32, GRID = 8;
export const TILE = {
  LEAF: [0, 1, 2, 3], CONIFER: [4, 5], TUFT: [6, 7, 8], FLOWER: [9, 10], REED: 11, GRAIN: 12, HERB: 13,
  FLAME: [14, 15, 16], FERN: 17, SEDGE: 18, LEAFSMALL: [19, 20], TWIG: 21, DRYTUFT: [22, 23], DOT: 24,
};

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
  const get = (x, y) => (x < 0 || y < 0 || x >= T || y >= T ? 0 : data[((oy + y) * W + ox + x) * 4 + 3]);
  draw(set, get);
}

// A clump of leaves: overlapping round leaves, upper ones in front, a dark line under each (scallops).
function leafClump(seed, R = 12, rmin = 2.2, rmax = 3.8, count = 15) {
  return (set) => {
    const r = new Rand(seed), cx = T / 2, cy = T / 2;
    const leaves = [];
    for (let i = 0; i < count; i++) {
      const a = r.range(0, Math.PI * 2), d = Math.sqrt(r.next()) * (R - rmax);
      leaves.push({ x: cx + Math.cos(a) * d, y: cy + Math.sin(a) * d * 0.85, r: r.range(rmin, rmax) });
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
      const gx = (x - cx) / R, gy = (y - cy) / R;
      let b = 0;
      const below = y > 0 ? owner[(y - 1) * T + x] : -1;
      if (below !== k && below >= 0) b = -1;           // a line where this leaf overlaps the one beneath
      else if (ly > 0.45 && lx < 0.1) b = 0.5;          // a soft lit cap on each leaf
      set(x, y, lx * 0.45 + gx * 0.6, ly * 0.45 + gy * 0.6, b);
    }
  };
}

// A tier of needles: a wide, drooping band with a ragged lower edge.
function coniferTier(seed) {
  return (set) => {
    const r = new Rand(seed), cx = T / 2;
    const top = 26, bot = 6;
    const ragged = Array.from({ length: T }, () => r.int(0, 4));
    for (let y = bot - 4; y <= top; y++) for (let x = 0; x < T; x++) {
      const t = (top - y) / (top - bot);             // 0 at the tip, 1 at the base
      const half = 2 + t * 13;
      if (Math.abs(x - cx) > half) continue;
      if (y < bot + ragged[x] - (Math.abs(x - cx) > half - 3 ? 2 : 0)) continue;
      let b = 0;
      if (y < bot + ragged[x] + 1) b = -1;
      else if ((x + y * 3) % 7 === 0 && t > 0.2) b = -1;
      else if (y > top - 4) b = 0.5;
      set(x, y, (x - cx) / 16, 0.2 + (y - bot) / 30, b);
    }
  };
}

// A grass tuft: a fan of blades from one root.
function tuft(seed, n = 7, hmin = 5, hmax = 12, spread = 0.9) {
  return (set) => {
    const r = new Rand(seed), cx = T / 2;
    for (let i = 0; i < n; i++) {
      const ang = (i / (n - 1) - 0.5) * spread + r.range(-0.12, 0.12), h = r.range(hmin, hmax), bend = r.range(-0.25, 0.25);
      for (let s = 0; s <= h; s++) {
        const t = s / h, a = ang + bend * t * t;
        const x = cx + Math.sin(a) * s, y = Math.cos(a) * s;
        set(x, y, 0, 0, t > 0.78 ? 1 : (t < 0.25 ? -1 : 0));
        if (t < 0.3) set(x + 1, y, 0, 0, -1);
      }
    }
  };
}

function flower(seed, kind) {
  return (set) => {
    const r = new Rand(seed);
    const heads = r.int(2, 4);
    for (let i = 0; i < heads; i++) {
      const x = 16 + r.int(-5, 5), y = r.int(2, 6);
      if (kind === 0) { // small cross-shaped blossom
        set(x, y, 0, 0, 1); set(x - 1, y, 0, 0, 0); set(x + 1, y, 0, 0, 0); set(x, y + 1, 0, 0, 1); set(x, y - 1, 0, 0, -1);
      } else { set(x, y, 0, 0, 1); set(x + 1, y, 0, 0, 0); set(x, y + 1, 0, 0, 1); set(x + 1, y + 1, 0, 0, 0); }
    }
  };
}

function reeds(seed) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 9; i++) {
      const x0 = 16 + r.range(-6, 6), h = r.range(14, 30), lean = r.range(-0.18, 0.18);
      for (let s = 0; s <= h; s++) set(x0 + lean * s, s, 0, 0, s > h - 3 ? 1 : (s < 4 ? -1 : 0));
      if (r.chance(0.4)) for (let s = h - 4; s <= h; s++) { set(x0 + lean * s, s, 0, 0, -2); set(x0 + lean * s + 1, s, 0, 0, -2); }
    }
  };
}

function grain(seed) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 7; i++) {
      const x0 = 16 + r.range(-7, 7), h = r.range(10, 17), lean = r.range(-0.2, 0.25);
      for (let s = 0; s <= h; s++) set(x0 + lean * s, s, 0, 0, s < 3 ? -1 : 0);
      for (let s = h - 4; s <= h + 1; s++) { set(x0 + lean * s, s, 0, 0, 1); if (s % 2) set(x0 + lean * s + 1, s, 0, 0, 0); }
    }
  };
}

function herb(seed) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 8; i++) {
      const a = (i / 8) * Math.PI - Math.PI / 2 + r.range(-0.2, 0.2), L = r.range(5, 9);
      for (let s = 0; s <= L; s++) {
        const x = 16 + Math.sin(a) * s * 1.2, y = 1 + Math.cos(a) * s * 0.6 + s * 0.15;
        set(x, y, Math.sin(a) * 0.5, 0.3, s > L - 2 ? 1 : 0); set(x, y + 1, Math.sin(a) * 0.5, 0.5, 0);
      }
    }
  };
}

function fern(seed) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 6; i++) {
      const a = (i / 5 - 0.5) * 2.2, L = r.range(8, 13);
      for (let s = 0; s <= L; s++) {
        const t = s / L, x = 16 + Math.sin(a) * s, y = Math.cos(a) * s * 0.8 - t * t * 3 + 2;
        set(x, y, Math.sin(a) * 0.4, 0.4, t > 0.8 ? 1 : 0);
        if (s % 2 === 0 && t < 0.85) { set(x + Math.cos(a) * 1.5, y - Math.sin(a) * 1.5, 0, 0.3, -1); set(x - Math.cos(a) * 1.5, y + Math.sin(a) * 1.5, 0, 0.3, 0); }
      }
    }
  };
}

// A flame: bands from a white-yellow core to a red edge; the blue channel gives the step offset from 3.
function flameTile(seed) {
  return (set) => {
    const r = new Rand(seed);
    const lick = Array.from({ length: 4 }, () => [r.range(-4, 4), r.range(10, 22)]);
    for (let y = 0; y < T; y++) for (let x = 0; x < T; x++) {
      const dx = x - 16, base = Math.max(0, 1 - y / 26);
      let w = 7 * Math.sqrt(base) * (1 - Math.max(0, (y - 18) / 10));
      let inside = Math.abs(dx) < w;
      for (const [lx, lh] of lick) if (Math.abs(dx - lx * (y / lh)) < 2.2 * (1 - y / lh) && y < lh) inside = true;
      if (!inside) continue;
      const core = Math.abs(dx) / Math.max(w, 1) + y / 30;
      const b = core < 0.35 ? 3 : core < 0.6 ? 2 : core < 0.85 ? 1 : 0;
      set(x, y, 0, 0, b);
    }
  };
}

function sedge(seed) {
  return (set) => {
    const r = new Rand(seed);
    for (let i = 0; i < 11; i++) {
      const ang = r.range(-0.6, 0.6), h = r.range(8, 16), bend = r.range(-0.5, 0.5);
      for (let s = 0; s <= h; s++) { const t = s / h, a = ang + bend * t * t; set(16 + Math.sin(a) * s, Math.cos(a) * s, 0, 0, t > 0.8 ? 1 : (t < 0.2 ? -1 : 0)); }
    }
  };
}

function twig(seed) {
  return (set) => {
    const r = new Rand(seed);
    const grow = (x, y, a, len, depth) => {
      for (let s = 0; s < len; s++) { x += Math.sin(a); y += Math.cos(a); set(x, y, 0, 0, depth > 1 ? 0 : -1); }
      if (depth < 3) { grow(x, y, a - r.range(0.3, 0.7), len * 0.65, depth + 1); grow(x, y, a + r.range(0.3, 0.7), len * 0.65, depth + 1); }
    };
    grow(16, 0, r.range(-0.1, 0.1), 9, 0);
  };
}

export function makeAtlas() {
  const W = T * GRID, data = new Uint8Array(W * W * 4);
  TILE.LEAF.forEach((id, i) => makeTile(data, id, leafClump(11 + i * 7)));
  TILE.CONIFER.forEach((id, i) => makeTile(data, id, coniferTier(41 + i * 5)));
  TILE.TUFT.forEach((id, i) => makeTile(data, id, tuft(61 + i * 3, 6 + i, 4 + i, 9 + i * 2)));
  TILE.FLOWER.forEach((id, i) => makeTile(data, id, flower(81 + i, i)));
  makeTile(data, TILE.REED, reeds(91));
  makeTile(data, TILE.GRAIN, grain(93));
  makeTile(data, TILE.HERB, herb(95));
  TILE.FLAME.forEach((id, i) => makeTile(data, id, flameTile(101 + i * 3)));
  makeTile(data, TILE.FERN, fern(111));
  makeTile(data, TILE.SEDGE, sedge(113));
  TILE.LEAFSMALL.forEach((id, i) => makeTile(data, id, leafClump(121 + i * 5, 7, 1.6, 2.6, 9)));
  makeTile(data, TILE.TWIG, twig(131));
  TILE.DRYTUFT.forEach((id, i) => makeTile(data, id, tuft(141 + i * 3, 9, 3, 8 + i * 3, 1.3)));
  makeTile(data, TILE.DOT, (set) => { for (let y = 0; y < T; y++) for (let x = 0; x < T; x++) set(x, y, 0, 0, 0); });
  const t = new THREE.DataTexture(data, W, W, THREE.RGBAFormat);
  t.magFilter = t.minFilter = THREE.NearestFilter;
  t.needsUpdate = true;
  return t;
}
