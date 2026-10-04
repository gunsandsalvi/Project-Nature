// Made things and camp life: hearths and fire, smoke, tents, huts and houses, racks, logs, pots, boats, graves.
// Each is built from the materials it is made of (PRE-42), so a hut of hides and one of reeds look different.
import * as THREE from 'three';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, newObj, M4, noise2 } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from './k.js';
import { boulder } from './land.js';

const { T, R } = M4;
const card = () => 32 * K.mpp;
const v = (m) => m.elements.slice(12, 15);

/** A ring polygon on the ground (for ash, floors, pits). */
function disc(s, c, r, y, mat, obj, pat = 0, n = 12, seed = 1) {
  const rr = new Rand(seed);
  const pts = Array.from({ length: n }, (_, i) => { const a = (i / n) * Math.PI * 2; const k = r * rr.range(0.85, 1.1); return [c[0] + Math.cos(a) * k, y, c[2] + Math.sin(a) * k]; });
  for (let i = 0; i < n; i++) s.tri([c[0], y, c[2]], pts[(i + 1) % n], pts[i], { mat, obj, pat });
}

/**
 * A hearth: a ring of stones, an ash bed, logs, and flames at level 0 (cold) to 4 (a blaze).
 * Returns { solid, cards, fire: [x, y, z, power, radius] relative to the hearth's centre }.
 */
export function hearth({ seed = 1, level = 2, ring = true, logs = 4 }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  disc(s, [0, 0, 0], 0.48, 0.03, 'ash', obj, PAT.DIRT, 12, seed);
  disc(s, [0, 0, 0], 0.26, 0.045, 'charcoal', obj, 0, 9, seed + 1);
  if (ring) for (let i = 0; i < 9; i++) {
    const a = (i / 9) * Math.PI * 2 + r.range(-0.15, 0.15);
    s.add(boulder({ seed: seed * 13 + i, size: [r.range(0.16, 0.24), r.range(0.11, 0.16), r.range(0.14, 0.2)], detail: 0, moss: 0, obj }), T(Math.cos(a) * 0.52, 0, Math.sin(a) * 0.52));
  }
  for (let i = 0; i < logs; i++) {
    const a = (i / logs) * Math.PI * 2 + r.range(0, 0.5), L = r.range(0.55, 0.8);
    s.beam([Math.cos(a) * L, 0.06, Math.sin(a) * L], [Math.cos(a) * 0.06, 0.16, Math.sin(a) * 0.06], 0.05, 0.045, 5, { mat: 'wood', pat: PAT.BARK, obj },
      { capMat: { mat: level > 0 ? 'ember' : 'charcoal', obj, flag: level > 0 ? FLAG.EMISSIVE : 0, bias: level > 0 ? 4 : 0 } });
  }
  const flames = [0, 2, 3, 5, 7][level];
  for (let i = 0; i < flames; i++) {
    const hgt = card() * r.range(0.55, 0.85) * (0.6 + level * 0.18);
    cards.add({ c: [r.range(-0.12, 0.12), 0.05, r.range(-0.12, 0.12)], w: hgt * 0.9, h: hgt, tile: r.pick(TILE.FLAME), mat: 'fire', bias: 3,
      obj, flag: FLAG.EMISSIVE | FLAG.NOOUTLINE | FLAG.NOSHADOW, upright: 1 });
  }
  for (let i = 0; i < level * 3; i++) {
    const sz = K.mpp * 1.2;
    cards.add({ c: [r.range(-0.3, 0.3), 0.6 + r.range(0, 1.2) * level * 0.4, r.range(-0.3, 0.3)], w: sz, h: sz, tile: TILE.FLAME[0], mat: 'ember', bias: 3,
      obj: 0, flag: FLAG.EMISSIVE | FLAG.NOOUTLINE | FLAG.NOSHADOW });
  }
  return { solid: s, cards, fire: level > 0 ? [0, 0.45, 0, [0, 0.35, 0.55, 0.75, 1.0][level], [0, 4.5, 7, 9, 12][level]] : null };
}

/** Smoke rising from a point and drifting with the wind; tone: the step of the smoke ramp. */
export function smoke(puffs, { at, height = 6, drift = [1, 0.2], seed = 1, size = 0.35, tone = 4, alpha = 0.5, spread = 1 }) {
  const r = new Rand(seed);
  let p = [...at], s = size;
  const steps = Math.round(height / 0.22);
  for (let i = 0; i < steps; i++) {
    const t = i / steps;
    if (r.next() < 1 - t * 0.5) for (let k = 0; k < 2; k++) {
      puffs.add([p[0] + r.range(-s, s) * 0.5 * spread, p[1] + r.range(-0.1, 0.1), p[2] + r.range(-s, s) * 0.5 * spread], s * r.range(1.4, 2.0), tone + (t > 0.6 ? -0.4 : 0), alpha * (1 - t * 0.55));
    }
    p = [p[0] + drift[0] * 0.22 * (0.2 + t), p[1] + 0.22, p[2] + drift[1] * 0.22 * (0.2 + t)];
    s = Math.min(s * 1.05 + 0.012, 1.4);
  }
}

/** A conical tent of hides on poles, its door toward +z. */
export function tent({ seed = 1, r = 1.8, h = 3.1, mat = 'hide', door = true, poles = 7, stones = true }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid();
  const n = 10, top = [0, h, 0];
  for (let i = 0; i < n; i++) {
    const a0 = (i / n) * Math.PI * 2 + Math.PI / 2, a1 = ((i + 1) / n) * Math.PI * 2 + Math.PI / 2;
    const k0 = r * rr.range(0.97, 1.03), k1 = r * rr.range(0.97, 1.03);
    const p0 = [Math.cos(a0) * k0, -0.05, Math.sin(a0) * k0], p1 = [Math.cos(a1) * k1, -0.05, Math.sin(a1) * k1];
    const isDoor = door && i === 0;
    s.tri(p0, [top[0], top[1] * 0.92, top[2]], p1, { mat, pat: PAT.HIDE, obj }, [[a0 * r, 0, 0], [(a0 + a1) / 2 * r, h, 0], [a1 * r, 0, 0]]);
    if (isDoor) {
      const mid = [(p0[0] + p1[0]) / 2, 0, (p0[2] + p1[2]) / 2];
      const up = [mid[0] * 0.55, h * 0.42, mid[2] * 0.55];
      const off = 0.03;
      s.tri([p0[0] * 0.5 + mid[0] * 0.5 + 0, 0.02, (p0[2] * 0.5 + mid[2] * 0.5) + off], [up[0], up[1], up[2] + off], [p1[0] * 0.5 + mid[0] * 0.5, 0.02, (p1[2] * 0.5 + mid[2] * 0.5) + off], { mat: 'charcoal', obj });
    }
  }
  for (let i = 0; i < poles; i++) {
    const a = (i / poles) * Math.PI * 2 + 0.2;
    s.beam([Math.cos(a) * r * 0.97, -0.05, Math.sin(a) * r * 0.97], [-Math.cos(a) * 0.32, h + 0.55 + rr.range(0, 0.25), -Math.sin(a) * 0.32], 0.04, 0.03, 4, { mat: 'wood', pat: PAT.WOOD, obj });
  }
  if (stones) for (let i = 0; i < 14; i++) {
    const a = (i / 14) * Math.PI * 2 + rr.range(-0.1, 0.1);
    if (door && Math.abs(Math.sin(a) - 1) < 0.08) continue;
    s.add(boulder({ seed: seed * 7 + i, size: [0.2, 0.13, 0.17], detail: 0, moss: 0, obj }), T(Math.cos(a) * (r + 0.06), 0, Math.sin(a) * (r + 0.06)));
  }
  return { solid: s, obj };
}

/** A dome hut: bent poles under hides, turf or thatch, a low door toward +z; bones: mammoth bones round the foot. */
export function dome({ seed = 1, r = 2, h = 1.8, mat = 'hide', pat = PAT.HIDE, bones = false, door = true }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid();
  const rings = 5, segs = 12;
  const pt = (i, j) => {
    const lat = (i / rings) * Math.PI / 2, a = (j / segs) * Math.PI * 2;
    const k = 1 + (noise2(i * 1.7 + seed, j * 1.3) - 0.5) * 0.08;
    return [Math.cos(a) * Math.cos(lat) * r * k, Math.sin(lat) * h * k - 0.05, Math.sin(a) * Math.cos(lat) * r * k];
  };
  for (let i = 0; i < rings; i++) for (let j = 0; j < segs; j++) {
    const a = pt(i, j), b = pt(i, j + 1), c = pt(i + 1, j + 1), d = pt(i + 1, j);
    const loc = (p, ii, jj) => [(jj / segs) * Math.PI * 2 * r, (ii / rings) * h * 1.6, 0];
    s.quad(a, d, c, b, { mat, pat, obj }, [loc(a, i, j), loc(d, i + 1, j), loc(c, i + 1, j + 1), loc(b, i, j + 1)]);
  }
  if (door) {
    const m = M4.mul(T(0, 0.42, r * 0.95), R(0.1, 0, 0));
    s.box(m, 0.8, 0.9, 0.7, { mat, pat, obj }, { front: { mat: 'charcoal' } });
  }
  if (bones) for (let j = 0; j < 16; j++) {
    const a = (j / 16) * Math.PI * 2 + rr.range(-0.05, 0.05);
    if (door && Math.abs(Math.sin(a) - 1) < 0.1) continue;
    const p0 = [Math.cos(a) * (r + 0.12), -0.05, Math.sin(a) * (r + 0.12)];
    s.beam(p0, [p0[0] * 0.92, rr.range(0.5, 0.75), p0[2] * 0.92], 0.09, 0.07, 5, { mat: 'bone', obj });
  }
  s.beam([0, h - 0.1, 0], [0.05, h + 0.25, 0.02], 0.05, 0.04, 4, { mat: 'wood', obj });
  return { solid: s, obj };
}

/** A windbreak: a row of leaning poles with brush or hides. */
export function windbreak({ seed = 1, len = 4, h = 1.6, hides = false }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  const n = Math.round(len / 0.4);
  for (let i = 0; i < n; i++) {
    const x = -len / 2 + (i + 0.5) * (len / n);
    s.beam([x, -0.05, 0], [x + r.range(-0.1, 0.1), h * r.range(0.9, 1.08), -h * 0.45], 0.035, 0.025, 4, { mat: 'wood', obj });
  }
  s.beam([-len / 2, h * 0.8, -h * 0.36], [len / 2, h * 0.8, -h * 0.36], 0.03, 0.03, 4, { mat: 'wood', obj });
  if (hides) {
    s.quad([-len / 2, 0.1, -0.04], [len / 2, 0.1, -0.04], [len / 2, h * 0.85, -h * 0.4], [-len / 2, h * 0.85, -h * 0.4], { mat: 'hide', pat: PAT.HIDE, obj });
  } else for (let i = 0; i < n * 3; i++) {
    const x = r.range(-len / 2, len / 2), t = r.range(0.15, 0.95);
    cards.add({ c: [x, h * t, -h * 0.45 * t - 0.05], w: card() * 0.6, h: card() * 0.6, tile: r.pick([TILE.TWIG, ...TILE.LEAFSMALL]), mat: r.chance(0.7) ? 'drygrass' : 'leaf', bias: -1, obj, flag: FLAG.FOLIAGE, n: [0, 0.6, 0.8] });
  }
  return { solid: s, cards, obj };
}

/** A drying rack: two leaning frames and a crossbar hung with meat strips, fish or a hide. */
export function rack({ seed = 1, len = 2.2, h = 1.7, hang = 'meat' }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid();
  for (const x of [-len / 2, len / 2]) {
    s.beam([x, -0.05, -0.35], [x, h, 0], 0.035, 0.03, 4, { mat: 'wood', obj });
    s.beam([x, -0.05, 0.35], [x, h, 0], 0.035, 0.03, 4, { mat: 'wood', obj });
  }
  s.beam([-len / 2 - 0.15, h - 0.04, 0], [len / 2 + 0.15, h - 0.04, 0], 0.03, 0.03, 4, { mat: 'wood', obj });
  if (hang === 'hide') s.box(T(0, h - 0.6, 0), len * 0.7, 1.1, 0.03, { mat: 'hide', pat: PAT.HIDE, obj });
  else {
    const n = Math.round(len / 0.16);
    for (let i = 0; i < n; i++) {
      const x = -len / 2 + 0.15 + i * ((len - 0.3) / (n - 1)), L = r.range(0.3, 0.6);
      s.box(T(x, h - 0.04 - L / 2, 0), hang === 'fish' ? 0.07 : 0.06, L, hang === 'fish' ? 0.025 : 0.03, { mat: hang === 'fish' ? 'flint' : (r.chance(0.5) ? 'berry' : 'dyedred'), obj });
    }
  }
  return { solid: s, obj };
}

export function log({ len = 2, r = 0.18, seed = 1 }) {
  const obj = newObj(), s = new Solid();
  s.beam([-len / 2, r, 0], [len / 2, r * 0.95, 0], r, r * 0.95, 7, { mat: 'bark', pat: PAT.BARK, obj }, { capA: true, capMat: { mat: 'wood', obj, bias: 1 } });
  return { solid: s, obj };
}

/** A pot, turned: radius profile from foot to lip. */
export function pot({ h = 0.35, r = 0.16, mat = 'clay', seed = 1, sides = 9 }) {
  const obj = newObj(), s = new Solid();
  const prof = [[0.55, 0], [0.9, 0.18], [1, 0.45], [0.85, 0.78], [0.7, 0.9], [0.76, 1]];
  for (let k = 0; k < prof.length - 1; k++) for (let i = 0; i < sides; i++) {
    const a0 = (i / sides) * Math.PI * 2, a1 = ((i + 1) / sides) * Math.PI * 2;
    const [r0, y0] = prof[k], [r1, y1] = prof[k + 1];
    s.quad([Math.cos(a0) * r0 * r, y0 * h, Math.sin(a0) * r0 * r], [Math.cos(a1) * r0 * r, y0 * h, Math.sin(a1) * r0 * r],
      [Math.cos(a1) * r1 * r, y1 * h, Math.sin(a1) * r1 * r], [Math.cos(a0) * r1 * r, y1 * h, Math.sin(a0) * r1 * r], { mat, obj, bias: k === 2 ? 0 : 0, pat: k === 3 ? PAT.CLOTH : 0 });
  }
  disc(s, [0, 0, 0], r * 0.62, h * 0.97, 'charcoal', obj, 0, sides);
  return { solid: s, obj };
}

/** A pit house: a ring of low earth wall and a cone of thatch or reed with deep eaves; door toward +z. */
export function pitHouse({ seed = 1, r = 2.6, h = 3.2, mat = 'thatch', wall = 'dirt' }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid();
  const n = 12, eave = 0.55;
  for (let i = 0; i < n; i++) {
    const a0 = (i / n) * Math.PI * 2, a1 = ((i + 1) / n) * Math.PI * 2;
    const w0 = [Math.cos(a0) * r, 0, Math.sin(a0) * r], w1 = [Math.cos(a1) * r, 0, Math.sin(a1) * r];
    s.quad([w0[0], -0.05, w0[2]], [w1[0], -0.05, w1[2]], [w1[0], eave, w1[2]], [w0[0], eave, w0[2]], { mat: wall, pat: PAT.DIRT, obj });
    const e0 = [Math.cos(a0) * (r + 0.35), eave - 0.05, Math.sin(a0) * (r + 0.35)], e1 = [Math.cos(a1) * (r + 0.35), eave - 0.05, Math.sin(a1) * (r + 0.35)];
    s.tri(e0, [0, h, 0], e1, { mat, pat: PAT.THATCH, obj }, [[a0 * r, 0, 0], [(a0 + a1) / 2 * r, h * 1.4, 0], [a1 * r, 0, 0]]);
    s.quad(e0, e1, [w1[0], eave + 0.02, w1[2]], [w0[0], eave + 0.02, w0[2]], { mat, pat: PAT.THATCH, obj, bias: -2 });
  }
  const dm = M4.mul(T(0, 0.6, r + 0.1));
  s.box(dm, 0.9, 1.25, 0.8, { mat, pat: PAT.THATCH, obj }, { front: { mat: 'charcoal' } });
  s.beam([0, h - 0.15, 0], [0.1, h + 0.35, 0], 0.05, 0.04, 4, { mat: 'wood', obj });
  return { solid: s, obj };
}

/** A long post house: wattle walls, posts, a pitched roof of reed or thatch. Long axis along x, door on +z. */
export function longhouse({ seed = 1, len = 9, w = 5, wallH = 1.5, roofH = 3, mat = 'reed', wall = 'wattle' }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid();
  const hx = len / 2, hz = w / 2;
  const wallMat = wall === 'wattle' ? 'wood' : 'clay', wallPat = wall === 'wattle' ? PAT.WATTLE : PAT.DIRT;
  const walls = [
    [[-hx, 0, hz], [hx, 0, hz]], [[hx, 0, -hz], [-hx, 0, -hz]], [[hx, 0, hz], [hx, 0, -hz]], [[-hx, 0, -hz], [-hx, 0, hz]],
  ];
  for (const [a, b] of walls) {
    const L = Math.hypot(b[0] - a[0], b[2] - a[2]);
    s.quad([a[0], -0.05, a[2]], [b[0], -0.05, b[2]], [b[0], wallH, b[2]], [a[0], wallH, a[2]], { mat: wallMat, pat: wallPat, obj },
      [[0, 0, 0], [L, 0, 0], [L, wallH, 0], [0, wallH, 0]]);
  }
  // gables
  const ov = 0.6, ex = 0.5;
  for (const sx of [-1, 1]) s.tri([sx * hx, wallH, -hz], [sx * hx, wallH + roofH, 0], [sx * hx, wallH, hz], { mat: wallMat, pat: wallPat, obj });
  // roof
  const ridge = wallH + roofH;
  const A = [-hx - ex, ridge, 0], B = [hx + ex, ridge, 0];
  const Cf = [hx + ex, wallH - ov * 0.6, hz + ov], Df = [-hx - ex, wallH - ov * 0.6, hz + ov];
  const Cb = [hx + ex, wallH - ov * 0.6, -hz - ov], Db = [-hx - ex, wallH - ov * 0.6, -hz - ov];
  const slope = Math.hypot(hz + ov, roofH + ov * 0.6);
  s.quad(Df, Cf, B, A, { mat, pat: PAT.THATCH, obj }, [[0, 0, 0], [len + 2 * ex, 0, 0], [len + 2 * ex, slope, 0], [0, slope, 0]]);
  s.quad(Cb, Db, A, B, { mat, pat: PAT.THATCH, obj }, [[0, 0, 0], [len + 2 * ex, 0, 0], [len + 2 * ex, slope, 0], [0, slope, 0]]);
  s.beam([A[0], ridge + 0.05, 0], [B[0], ridge + 0.05, 0], 0.09, 0.09, 5, { mat: 'thatch', obj, bias: -1 });
  // door and posts
  s.box(T(rr.range(-1, 1), 0.75, hz + 0.02), 0.85, 1.5, 0.06, { mat: 'charcoal', obj });
  for (const x of [-hx, -hx / 2, 0, hx / 2, hx]) for (const z of [-hz, hz]) s.beam([x, -0.05, z], [x, wallH + 0.15, z], 0.08, 0.07, 5, { mat: 'wood', pat: PAT.BARK, obj });
  return { solid: s, obj };
}

/** A wattle fence along points [x, z]; ground(x, z) gives the height. */
export function fence({ pts, h = 1, ground = () => 0, seed = 1 }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid();
  for (let i = 0; i < pts.length - 1; i++) {
    const [ax, az] = pts[i], [bx, bz] = pts[i + 1];
    const L = Math.hypot(bx - ax, bz - az), n = Math.max(1, Math.round(L / 0.9));
    for (let k = 0; k <= n; k++) {
      const t = k / n, x = ax + (bx - ax) * t, z = az + (bz - az) * t, y = ground(x, z);
      s.beam([x, y - 0.1, z], [x, y + h + 0.1, z], 0.04, 0.035, 4, { mat: 'wood', obj });
    }
    const y0 = ground(ax, az), y1 = ground(bx, bz);
    s.quad([ax, y0 + 0.08, az], [bx, y1 + 0.08, bz], [bx, y1 + h, bz], [ax, y0 + h, az], { mat: 'wood', pat: PAT.WATTLE, obj }, [[0, 0, 0], [L, 0, 0], [L, h, 0], [0, h, 0]]);
  }
  return { solid: s, obj };
}

/** A dugout canoe along x. */
export function canoe({ len = 4.2, w = 0.6, seed = 1 }) {
  const obj = newObj(), s = new Solid();
  const n = 8, hx = len / 2;
  const prof = (t) => Math.sin(Math.PI * t) ** 0.6;
  for (let i = 0; i < n; i++) {
    const t0 = i / n, t1 = (i + 1) / n, x0 = -hx + t0 * len, x1 = -hx + t1 * len;
    const w0 = (w / 2) * prof(t0), w1 = (w / 2) * prof(t1);
    s.quad([x0, 0.3, w0], [x1, 0.3, w1], [x1, 0.02, w1 * 0.6], [x0, 0.02, w0 * 0.6], { mat: 'wood', pat: PAT.PLANK, obj });
    s.quad([x1, 0.3, -w1], [x0, 0.3, -w0], [x0, 0.02, -w0 * 0.6], [x1, 0.02, -w1 * 0.6], { mat: 'wood', pat: PAT.PLANK, obj });
    s.quad([x0, 0.26, -w0 * 0.8], [x1, 0.26, -w1 * 0.8], [x1, 0.26, w1 * 0.8], [x0, 0.26, w0 * 0.8], { mat: 'leather', obj });
  }
  return { solid: s, obj };
}

/** A shell midden: a low heap of pale shells. */
export function midden({ seed = 1, r = 1.6, h = 0.5 }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid();
  s.add(boulder({ seed, size: [r * 2, h * 2, r * 1.6], detail: 2, mat: 'shell', moss: 0.2, mossMat: 'grass', sink: 0.5, obj }), T(0, -h * 0.4, 0));
  for (let i = 0; i < 26; i++) {
    const a = rr.range(0, Math.PI * 2), d = rr.range(0.3, 1.1) * r;
    s.add(boulder({ seed: seed * 11 + i, size: [0.1, 0.05, 0.08], detail: 0, mat: rr.chance(0.5) ? 'shell' : 'white', moss: 0, obj }), T(Math.cos(a) * d, 0, Math.sin(a) * d * 0.8));
  }
  return { solid: s, obj };
}

/** A grave: an open pit with a body laid in it by the caller, a heap of earth, and ochre on the rim. */
export function grave({ len = 1.9, w = 0.9, depth = 0.5, seed = 1 }) {
  const obj = newObj(), s = new Solid();
  const hx = len / 2, hz = w / 2, y = 0.02;
  s.quad([-hx, -depth, hz], [hx, -depth, hz], [hx, -depth, -hz], [-hx, -depth, -hz], { mat: 'dirt', obj, bias: -2 });
  s.quad([-hx, y, hz], [hx, y, hz], [hx, -depth, hz], [-hx, -depth, hz], { mat: 'dirt', obj, bias: -1 });
  s.quad([hx, y, -hz], [-hx, y, -hz], [-hx, -depth, -hz], [hx, -depth, -hz], { mat: 'dirt', obj, bias: -1, pat: PAT.DIRT });
  s.quad([hx, y, hz], [hx, y, -hz], [hx, -depth, -hz], [hx, -depth, hz], { mat: 'dirt', obj, bias: -1 });
  s.quad([-hx, y, -hz], [-hx, y, hz], [-hx, -depth, hz], [-hx, -depth, -hz], { mat: 'dirt', obj, bias: -1 });
  s.add(boulder({ seed, size: [len * 0.9, 0.5, 0.8], detail: 1, mat: 'dirt', moss: 0, sink: 0.4 }), T(0, -0.05, -hz - 0.6));
  return { solid: s, obj, floor: -depth };
}

/** A copper furnace: a clay shaft with blowpipes, a heap of charcoal and one of green ore. */
export function furnace({ seed = 1, h = 0.9, r = 0.32, lit = true }) {
  const rr = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  s.beam([0, -0.05, 0], [0, h, 0], r, r * 0.75, 9, { mat: 'clay', pat: PAT.DIRT, obj }, { capMat: { mat: lit ? 'ember' : 'charcoal', obj, flag: lit ? FLAG.EMISSIVE : 0, bias: 5 } });
  for (let i = 0; i < 3; i++) {
    const a = (i / 3) * Math.PI * 2 + 0.6;
    s.beam([Math.cos(a) * r * 0.9, 0.18, Math.sin(a) * r * 0.9], [Math.cos(a) * 1.0, 0.45, Math.sin(a) * 1.0], 0.025, 0.02, 4, { mat: 'reed', obj });
  }
  s.add(boulder({ seed: seed + 3, size: [0.9, 0.35, 0.7], detail: 1, mat: 'charcoal', moss: 0, sink: 0.3 }), T(1.1, 0, -0.6));
  s.add(boulder({ seed: seed + 5, size: [0.7, 0.3, 0.6], detail: 1, mat: 'verdi', moss: 0, sink: 0.3 }), T(-1.0, 0, -0.7));
  if (lit) for (let i = 0; i < 3; i++) cards.add({ c: [rr.range(-0.08, 0.08), h - 0.05, rr.range(-0.08, 0.08)], w: card() * 0.5, h: card() * 0.6, tile: rr.pick(TILE.FLAME), mat: 'fire', bias: 3, obj, flag: FLAG.EMISSIVE | FLAG.NOOUTLINE | FLAG.NOSHADOW, upright: 1 });
  return { solid: s, cards, obj, fire: lit ? [0, h + 0.2, 0, 0.6, 6] : null };
}

/** A house of mud brick with a flat roof of beams and packed earth; door toward +z. court: a walled yard in front. */
export function mudHouse({ seed = 1, w = 5, d = 4, h = 2.2, court = 0, mat = 'clay' }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid();
  const wall = { mat, pat: PAT.DIRT, obj };
  s.box(T(0, h / 2 - 0.05, 0), w, h, d, wall, { top: { mat: 'dirt', pat: PAT.DIRT, bias: 1 } });
  s.box(T(0, h + 0.08, 0), w + 0.2, 0.18, d + 0.2, { mat: 'dirt', pat: PAT.DIRT, obj, bias: 0.5 });
  for (let i = 0; i < Math.round(w / 0.6); i++) s.beam([-w / 2 + 0.3 + i * 0.6, h - 0.05, d / 2 - 0.1], [-w / 2 + 0.3 + i * 0.6, h - 0.05, d / 2 + 0.28], 0.05, 0.05, 4, { mat: 'wood', obj });
  s.box(T(r.range(-w / 4, w / 4), 0.8, d / 2 + 0.02), 0.8, 1.6, 0.06, { mat: 'charcoal', obj });
  if (court) {
    const cw = w + 1.2, cd = court;
    for (const [x0, z0, x1, z1] of [[-cw / 2, d / 2, -cw / 2, d / 2 + cd], [cw / 2, d / 2, cw / 2, d / 2 + cd], [-cw / 2, d / 2 + cd, -0.7, d / 2 + cd], [0.7, d / 2 + cd, cw / 2, d / 2 + cd]]) {
      const L = Math.hypot(x1 - x0, z1 - z0), cx = (x0 + x1) / 2, cz = (z0 + z1) / 2, a = Math.atan2(z1 - z0, x1 - x0);
      s.box(M4.mul(T(cx, 0.55, cz), R(0, -a, 0)), L + 0.3, 1.1, 0.3, wall, { top: { mat: 'dirt', bias: 1 } });
    }
  }
  return { solid: s, obj };
}

/** A standing stone. */
export function standingStone({ seed = 1, h = 2.2, w = 0.8 }) {
  return { solid: boulder({ seed, size: [w, h * 1.1, w * 0.6], detail: 1, mat: 'rock', moss: 0.35, sink: 0.6 }) };
}
