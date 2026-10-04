// Geometry builders for the painter: solids with a material per face, cards (leaves, tufts, flowers, flames),
// water sheets and smoke puffs. Everything is merged per builder, so a whole tree or person is one mesh.
import * as THREE from 'three';
import { mat } from './palette.js';

export const FLAG = { EMISSIVE: 1, FOLIAGE: 2, NOOUTLINE: 4, SOFT: 8, NOSHADOW: 16, CREATURE: 32 };
export const PAT = {
  NONE: 0, GROUND: 1, ROCK: 2, BARK: 3, WOOD: 4, HIDE: 5, THATCH: 6, FACE: 7, FUR: 8, SNOW: 9, SAND: 10,
  DIRT: 11, CLOTH: 13, MOSS: 14, PLANK: 15, WATTLE: 16, TERRAIN: 100, CELLS: 101,
};

let nextObj = 1;
export const newObj = () => nextObj++;

// A small seeded random source, so every picture is the same on every run.
export class Rand {
  constructor(seed = 1) { this.s = (seed * 2654435761) >>> 0 || 1; }
  next() { let t = (this.s += 0x6d2b79f5); t = Math.imul(t ^ (t >>> 15), t | 1); t ^= t + Math.imul(t ^ (t >>> 7), t | 61); return ((t ^ (t >>> 14)) >>> 0) / 4294967296; }
  range(a, b) { return a + (b - a) * this.next(); }
  int(a, b) { return Math.floor(this.range(a, b + 1)); }
  pick(list) { return list[Math.floor(this.next() * list.length)]; }
  chance(p) { return this.next() < p; }
}

// Value noise in JS, matching the feel of the shader noise.
function hash2(x, y) { let h = Math.imul(x | 0, 374761393) + Math.imul(y | 0, 668265263); h = Math.imul(h ^ (h >>> 13), 1274126177); return ((h ^ (h >>> 16)) >>> 0) / 4294967296; }
export function noise2(x, y) {
  const xi = Math.floor(x), yi = Math.floor(y), xf = x - xi, yf = y - yi;
  const u = xf * xf * (3 - 2 * xf), v = yf * yf * (3 - 2 * yf);
  const a = hash2(xi, yi), b = hash2(xi + 1, yi), c = hash2(xi, yi + 1), d = hash2(xi + 1, yi + 1);
  return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
}
export function fbm(x, y, oct = 4) { let a = 0.5, s = 0, n = 0; for (let i = 0; i < oct; i++) { s += a * noise2(x, y); n += a; x = x * 2.03 + 7.1; y = y * 2.03 + 3.7; a *= 0.5; } return s / n; }
export const clamp = (x, a, b) => Math.max(a, Math.min(b, x));
export const lerp = (a, b, t) => a + (b - a) * t;
export const smooth = (a, b, x) => { const t = clamp((x - a) / (b - a), 0, 1); return t * t * (3 - 2 * t); };

const _v = new THREE.Vector3(), _n = new THREE.Vector3(), _m3 = new THREE.Matrix3();

/** Builds a solid mesh from triangles; each vertex carries its material, step bias, object, pattern and flags. */
export class Solid {
  constructor() { this.P = []; this.N = []; this.A = []; this.L = []; this.W = []; }
  /** a = { mat: name or row, bias, obj, pat, flag } */
  static attrs(a) {
    return [typeof a.mat === 'string' ? mat(a.mat) : a.mat, a.bias || 0, a.obj || 0, a.pat || 0, a.flag || 0];
  }
  vert(p, n, at, loc = [0, 0, 0], w = [0, 0, 0, 0]) {
    this.P.push(p[0], p[1], p[2]); this.N.push(n[0], n[1], n[2]); this.A.push(...at); this.L.push(...loc); this.W.push(...w);
  }
  /** A flat triangle; locs are the pattern coordinates of its corners. */
  tri(p0, p1, p2, a, locs) {
    const at = Solid.attrs(a);
    const e1 = [p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]], e2 = [p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]];
    const n = [e1[1] * e2[2] - e1[2] * e2[1], e1[2] * e2[0] - e1[0] * e2[2], e1[0] * e2[1] - e1[1] * e2[0]];
    const l = Math.hypot(...n) || 1; n[0] /= l; n[1] /= l; n[2] /= l;
    const L = locs || [p0, p1, p2];
    this.vert(p0, n, at, L[0]); this.vert(p1, n, at, L[1]); this.vert(p2, n, at, L[2]);
  }
  quad(p0, p1, p2, p3, a, locs) {
    this.tri(p0, p1, p2, a, locs && [locs[0], locs[1], locs[2]]);
    this.tri(p0, p2, p3, a, locs && [locs[0], locs[2], locs[3]]);
  }
  /**
   * A box of size sx, sy, sz centred on the origin, moved by matrix m. Pattern coordinates are the box's own,
   * in metres (or normalised to the box when a.norm). faces: optional per-face overrides {top, bottom, front, back, left, right}.
   */
  box(m, sx, sy, sz, a, faces = {}) {
    const hx = sx / 2, hy = sy / 2, hz = sz / 2;
    const c = [[-hx, -hy, -hz], [hx, -hy, -hz], [hx, hy, -hz], [-hx, hy, -hz], [-hx, -hy, hz], [hx, -hy, hz], [hx, hy, hz], [-hx, hy, hz]];
    const w = c.map((p) => _v.set(...p).applyMatrix4(m).toArray());
    const loc = c.map((p) => (a.norm ? [p[0] / sx, p[1] / sy, p[2] / sz] : p));
    const F = [
      ['front', [4, 5, 6, 7]], ['back', [1, 0, 3, 2]], ['top', [7, 6, 2, 3]],
      ['bottom', [0, 1, 5, 4]], ['right', [5, 1, 2, 6]], ['left', [0, 4, 7, 3]],
    ];
    for (const [name, q] of F) {
      const fa = faces[name] ? { ...a, ...faces[name] } : a;
      if (fa.skip) continue;
      this.quad(w[q[0]], w[q[1]], w[q[2]], w[q[3]], fa, [loc[q[0]], loc[q[1]], loc[q[2]], loc[q[3]]]);
    }
  }
  /** A prism between two points: a beam of n sides (poles, logs, limbs, trunks), radius r0 at a and r1 at b. */
  beam(a3, b3, r0, r1, n, at, opts = {}) {
    const A = new THREE.Vector3(...a3), B = new THREE.Vector3(...b3);
    const ax = B.clone().sub(A); const len = ax.length(); ax.normalize();
    const t = Math.abs(ax.y) < 0.9 ? new THREE.Vector3(0, 1, 0) : new THREE.Vector3(1, 0, 0);
    const u = t.clone().cross(ax).normalize(), v = ax.clone().cross(u).normalize();
    const rot = opts.rot || 0;
    const ring = (C, r) => Array.from({ length: n }, (_, i) => {
      const ang = rot + (i / n) * Math.PI * 2;
      return C.clone().addScaledVector(u, Math.cos(ang) * r).addScaledVector(v, Math.sin(ang) * r).toArray();
    });
    const ra = ring(A, r0), rb = ring(B, r1);
    for (let i = 0; i < n; i++) {
      const j = (i + 1) % n, la = (i / n) * Math.PI * 2 * (r0 + r1) / 2, lb = ((i + 1) / n) * Math.PI * 2 * (r0 + r1) / 2;
      this.quad(ra[i], ra[j], rb[j], rb[i], at, [[la, 0, 0], [lb, 0, 0], [lb, len, 0], [la, len, 0]]);
    }
    if (opts.capB !== false) for (let i = 1; i < n - 1; i++) this.tri(rb[0], rb[i], rb[i + 1], opts.capMat || at);
    if (opts.capA) for (let i = 1; i < n - 1; i++) this.tri(ra[0], ra[i + 1], ra[i], opts.capMat || at);
  }
  /** Adds a smooth-shaded indexed geometry (positions, normals, index) with one set of attributes. */
  smooth(positions, normals, index, a, locs, weights) {
    const at = Solid.attrs(a);
    for (let k = 0; k < index.length; k++) {
      const i = index[k];
      this.vert([positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]], [normals[i * 3], normals[i * 3 + 1], normals[i * 3 + 2]], at,
        locs ? [locs[i * 3], locs[i * 3 + 1], locs[i * 3 + 2]] : [positions[i * 3], positions[i * 3 + 1], positions[i * 3 + 2]],
        weights ? [weights[i * 4], weights[i * 4 + 1], weights[i * 4 + 2], weights[i * 4 + 3]] : [0, 0, 0, 0]);
    }
  }
  /** Appends another solid moved by matrix m. */
  add(other, m) {
    _m3.getNormalMatrix(m);
    for (let i = 0; i < other.P.length; i += 3) {
      _v.set(other.P[i], other.P[i + 1], other.P[i + 2]).applyMatrix4(m);
      _n.set(other.N[i], other.N[i + 1], other.N[i + 2]).applyMatrix3(_m3).normalize();
      this.P.push(_v.x, _v.y, _v.z); this.N.push(_n.x, _n.y, _n.z);
    }
    this.A.push(...other.A); this.L.push(...other.L); this.W.push(...other.W);
  }
  geometry() {
    const g = new THREE.BufferGeometry();
    const n = this.P.length / 3;
    g.setAttribute('position', new THREE.Float32BufferAttribute(this.P, 3));
    g.setAttribute('normal', new THREE.Float32BufferAttribute(this.N, 3));
    const cols = [[], [], [], [], []];
    for (let i = 0; i < n; i++) for (let k = 0; k < 5; k++) cols[k].push(this.A[i * 5 + k]);
    ['aMat', 'aBias', 'aObj', 'aPat', 'aFlag'].forEach((name, k) => g.setAttribute(name, new THREE.Float32BufferAttribute(cols[k], 1)));
    g.setAttribute('aLoc', new THREE.Float32BufferAttribute(this.L, 3));
    g.setAttribute('aW', new THREE.Float32BufferAttribute(this.W, 4));
    return g;
  }
}

/** Cards: small pictures from the atlas standing in 3D (leaf clumps, tufts, flowers, reeds, flames). */
export class Cards {
  constructor() { this.d = { aCenter: [], aCorner: [], aSize: [], aTile: [], aMat: [], aBias: [], aObj: [], aFlag: [], aNrm: [], aUpright: [], aSpin: [] }; this.n = 0; }
  /** c: centre; w, h: size in metres; n: the normal it is lit with; upright: stands on the ground, facing the camera's way */
  add({ c, w, h, tile, mat: m, bias = 0, obj = 0, flag = 0, n = [0, 1, 0], upright = 0, spin = 0 }) {
    const row = typeof m === 'string' ? mat(m) : m;
    const corners = [[-0.5, -0.5], [0.5, -0.5], [0.5, 0.5], [-0.5, 0.5]];
    const cy = upright ? c[1] + h / 2 : c[1];
    for (const k of corners) {
      this.d.aCenter.push(c[0], cy, c[2]); this.d.aCorner.push(...k); this.d.aSize.push(w, h); this.d.aTile.push(tile);
      this.d.aMat.push(row); this.d.aBias.push(bias); this.d.aObj.push(obj); this.d.aFlag.push(flag);
      this.d.aNrm.push(...n); this.d.aUpright.push(upright); this.d.aSpin.push(spin);
    }
    this.n++;
  }
  geometry() {
    const g = new THREE.BufferGeometry();
    const sizes = { aCenter: 3, aCorner: 2, aSize: 2, aTile: 1, aMat: 1, aBias: 1, aObj: 1, aFlag: 1, aNrm: 3, aUpright: 1, aSpin: 1 };
    for (const [k, s] of Object.entries(sizes)) g.setAttribute(k, new THREE.Float32BufferAttribute(this.d[k], s));
    g.setAttribute('position', new THREE.Float32BufferAttribute(new Array(this.n * 12).fill(0), 3));
    const idx = [];
    for (let i = 0; i < this.n; i++) idx.push(i * 4, i * 4 + 1, i * 4 + 2, i * 4, i * 4 + 2, i * 4 + 3);
    g.setIndex(idx);
    g.boundingSphere = new THREE.Sphere(new THREE.Vector3(), 1e5);
    return g;
  }
}

/** Smoke and mist puffs, drawn over the picture. */
export class Puffs {
  constructor() { this.d = { aCenter: [], aCorner: [], aSize: [], aTone: [], aAlpha: [] }; this.n = 0; }
  add(c, size, tone = 4, alpha = 0.5) {
    for (const k of [[-0.5, -0.5], [0.5, -0.5], [0.5, 0.5], [-0.5, 0.5]]) {
      this.d.aCenter.push(...c); this.d.aCorner.push(...k); this.d.aSize.push(size); this.d.aTone.push(tone); this.d.aAlpha.push(alpha);
    }
    this.n++;
  }
  geometry() {
    const g = new THREE.BufferGeometry();
    const sizes = { aCenter: 3, aCorner: 2, aSize: 1, aTone: 1, aAlpha: 1 };
    for (const [k, s] of Object.entries(sizes)) g.setAttribute(k, new THREE.Float32BufferAttribute(this.d[k], s));
    g.setAttribute('position', new THREE.Float32BufferAttribute(new Array(this.n * 12).fill(0), 3));
    const idx = [];
    for (let i = 0; i < this.n; i++) idx.push(i * 4, i * 4 + 1, i * 4 + 2, i * 4, i * 4 + 2, i * 4 + 3);
    g.setIndex(idx);
    g.boundingSphere = new THREE.Sphere(new THREE.Vector3(), 1e5);
    return g;
  }
}

/** Matrix helpers. */
export const M4 = {
  T: (x, y, z) => new THREE.Matrix4().makeTranslation(x, y, z),
  R: (x, y, z, order = 'YXZ') => new THREE.Matrix4().makeRotationFromEuler(new THREE.Euler(x, y, z, order)),
  S: (x, y, z) => new THREE.Matrix4().makeScale(x, y ?? x, z ?? x),
  mul: (...ms) => ms.reduce((a, b) => a.clone().multiply(b)),
};
