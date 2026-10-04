// Things for the far zoom stops (PRE-03, PRE-28, PRE-29): trees as round crowns once a leaf is smaller than a pixel,
// ground laid along the view so far ground costs no more points than the picture has pixels, the globe, and the
// markers drawn over the picture for people, groups, herds and camps.
import * as THREE from 'three';
import { Solid, FLAG, Rand, newObj } from '../geo.js';
import { RAMPS } from '../palette.js';

function ball(s, c, rx, ry, rz, a, detail = 1) {
  const g = new THREE.IcosahedronGeometry(1, detail), p = g.attributes.position;
  const P = [], N = [], I = [];
  for (let i = 0; i < p.count; i++) {
    const x = p.getX(i), y = p.getY(i), z = p.getZ(i);
    P.push(c[0] + x * rx, c[1] + y * ry, c[2] + z * rz);
    const n = new THREE.Vector3(x / rx, y / ry, z / rz).normalize();
    N.push(n.x, n.y, n.z); I.push(i);
  }
  s.smooth(P, N, I, a);
}

function cone(s, base, r, h, a, sides = 8) {
  for (let i = 0; i < sides; i++) {
    const a0 = (i / sides) * Math.PI * 2, a1 = ((i + 1) / sides) * Math.PI * 2;
    const p0 = [base[0] + Math.cos(a0) * r, base[1], base[2] + Math.sin(a0) * r], p1 = [base[0] + Math.cos(a1) * r, base[1], base[2] + Math.sin(a1) * r];
    const top = [base[0], base[1] + h, base[2]], k = r / h;
    const n0 = new THREE.Vector3(Math.cos(a0), k, Math.sin(a0)).normalize(), n1 = new THREE.Vector3(Math.cos(a1), k, Math.sin(a1)).normalize();
    const nt = n0.clone().add(n1).normalize();
    s.smooth([...p0, ...top, ...p1], [n0.x, n0.y, n0.z, nt.x, nt.y, nt.z, n1.x, n1.y, n1.z], [0, 1, 2], a);
  }
}

/** A tree seen from far off: a trunk and round crowns lit as balls, or a pine's cone of tiers. */
export function farTree({ seed = 1, h = 9, kind = 'broad', mat }) {
  const r = new Rand(seed), s = new Solid(), obj = newObj();
  const leaf = { mat: mat || (kind === 'pine' ? 'pine' : 'leaf'), obj, flag: FLAG.FOLIAGE };
  s.beam([0, -0.3, 0], [0, h * 0.55, 0], 0.16 + h * 0.014, 0.1, 5, { mat: 'bark', obj });
  if (kind === 'pine') {
    cone(s, [0, h * 0.18, 0], h * 0.26, h * 0.5, leaf);
    cone(s, [0, h * 0.48, 0], h * 0.19, h * 0.52, leaf);
  } else {
    const n = r.int(2, 3);
    for (let i = 0; i < n; i++) {
      const R = h * r.range(0.2, 0.27);
      ball(s, [r.range(-0.16, 0.16) * h, h * r.range(0.6, 0.72), r.range(-0.16, 0.16) * h], R, R * 0.86, R, leaf);
    }
  }
  return { solid: s, obj };
}

/**
 * Ground laid along the view: a grid in the picture's own directions over the ground the camera sees, from heights
 * ymin to ymax, plus a margin. height(x, z); weights(x, z, y, slope) gives four layer weights. Normals come from the
 * grid itself, so each point's height is worked out once; smoothN cells either side light the ground by its broad
 * slopes only, so small bumps never speckle in a low sun, while the layers still see the steep places.
 */
export function groundView(P, { height, weights, step, stepD = step, ymin = 0, ymax = 0, margin = 0.12, obj = 0, pat = 100, extra = 0, smoothN = 1 }) {
  const yr = (P.yaw * Math.PI) / 180, er = (P.elev * Math.PI) / 180;
  const right = [Math.cos(yr), -Math.sin(yr)], fwd = [-Math.sin(yr), -Math.cos(yr)];
  const t = P.target, W = P.w * P.mpp * (1 + margin) + 2 * extra, Hs = P.h * P.mpp;
  const sin = Math.max(Math.sin(er), 0.05), cos = Math.cos(er);
  const d0 = (-Hs / 2 - (ymax - t.y) * cos) / sin - Hs * margin - extra, d1 = (Hs / 2 - (ymin - t.y) * cos) / sin + Hs * margin + extra;
  const nx = Math.ceil(W / step) + 1, nd = Math.ceil((d1 - d0) / stepD) + 1;
  const pos = new Float32Array(nx * nd * 3);
  for (let j = 0; j < nd; j++) for (let i = 0; i < nx; i++) {
    const sx = -W / 2 + i * step, d = d0 + j * stepD;
    const x = t.x + right[0] * sx + fwd[0] * d, z = t.z + right[1] * sx + fwd[1] * d, k = (j * nx + i) * 3;
    pos[k] = x; pos[k + 1] = height(x, z); pos[k + 2] = z;
  }
  const nrm = new Float32Array(nx * nd * 3), wts = new Float32Array(nx * nd * 4), v = new THREE.Vector3(), a = new THREE.Vector3(), b = new THREE.Vector3();
  const P3 = (i, j) => { i = Math.max(0, Math.min(nx - 1, i)); j = Math.max(0, Math.min(nd - 1, j)); const k = (j * nx + i) * 3; return [pos[k], pos[k + 1], pos[k + 2]]; };
  const normal = (i, j, c) => {
    const L = P3(i - c, j), R = P3(i + c, j), N = P3(i, j - c), S = P3(i, j + c);
    a.set(R[0] - L[0], R[1] - L[1], R[2] - L[2]); b.set(S[0] - N[0], S[1] - N[1], S[2] - N[2]);
    v.crossVectors(b, a).normalize(); if (v.y < 0) v.negate();
    return [v.x, v.y, v.z];
  };
  for (let j = 0; j < nd; j++) for (let i = 0; i < nx; i++) {
    const k = j * nx + i, n1 = normal(i, j, 1);
    nrm.set(smoothN > 1 ? normal(i, j, smoothN) : n1, k * 3);
    wts.set(weights ? weights(pos[k * 3], pos[k * 3 + 2], pos[k * 3 + 1], 1 - n1[1]) : [1, 0, 0, 0], k * 4);
  }
  const n = nx * nd, idx = new Uint32Array((nx - 1) * (nd - 1) * 6);
  let c = 0;
  for (let j = 0; j < nd - 1; j++) for (let i = 0; i < nx - 1; i++) {
    const p = j * nx + i, q = p + 1, r = p + nx, s = r + 1;
    idx.set([p, q, r, q, s, r], c); c += 6;
  }
  // an indexed mesh straight to the painter (a Solid would copy every corner of every triangle)
  const g = new THREE.BufferGeometry(), one = (v) => new THREE.BufferAttribute(new Float32Array(n).fill(v), 1);
  g.setAttribute('position', new THREE.BufferAttribute(pos, 3));
  g.setAttribute('normal', new THREE.BufferAttribute(nrm, 3));
  g.setAttribute('aMat', one(0)); g.setAttribute('aBias', one(0)); g.setAttribute('aObj', one(obj));
  g.setAttribute('aPat', one(pat)); g.setAttribute('aFlag', one(0));
  g.setAttribute('aLoc', new THREE.BufferAttribute(pos.slice(), 3));
  g.setAttribute('aW', new THREE.BufferAttribute(wts, 4));
  g.setIndex(new THREE.BufferAttribute(idx, 1));
  return { geometry: () => g, pos, nx, nd };
}

/**
 * The globe (WLD-02): a ball of radius R metres whose every small face takes the colour of the world cell under it;
 * at(lat, lon) gives { e: height in metres, mat, bias }. Heights are raised `exag` times so the hills shade.
 */
export function globe({ R, detail = 90, exag = 4, at }) {
  const g = new THREE.IcosahedronGeometry(1, detail), p = g.attributes.position, s = new Solid(), obj = newObj();
  const cache = new Map();
  const vert = (i) => {
    const x = p.getX(i), y = p.getY(i), z = p.getZ(i), key = `${x.toFixed(5)},${y.toFixed(5)},${z.toFixed(5)}`;
    let c = cache.get(key);
    if (!c) {
      const lat = Math.asin(Math.max(-1, Math.min(1, y))) * 180 / Math.PI, lon = Math.atan2(z, x) * 180 / Math.PI;
      const q = at(lat, lon), r = R + Math.max(q.e, -200) * exag;
      c = [x * r, y * r, z * r];
      cache.set(key, c);
    }
    return c;
  };
  for (let i = 0; i < p.count; i += 3) {
    const A = vert(i), B = vert(i + 1), C = vert(i + 2);
    const cx = (p.getX(i) + p.getX(i + 1) + p.getX(i + 2)) / 3, cy = (p.getY(i) + p.getY(i + 1) + p.getY(i + 2)) / 3, cz = (p.getZ(i) + p.getZ(i + 1) + p.getZ(i + 2)) / 3;
    const L = Math.hypot(cx, cy, cz);
    const q = at(Math.asin(cy / L) * 180 / Math.PI, Math.atan2(cz, cx) * 180 / Math.PI);
    // lit as a smooth ball: each corner's normal points out from the centre
    const nA = A.map((v) => v / Math.hypot(...A)), nB = B.map((v) => v / Math.hypot(...B)), nC = C.map((v) => v / Math.hypot(...C));
    s.smooth([...A, ...B, ...C], [...nA, ...nB, ...nC], [0, 1, 2], { mat: q.mat, bias: q.bias || 0, obj });
  }
  return s;
}

/** Water ribbons along river lines in the camp's frame: points [u, v, area, level, width], at least minW wide. */
export function riverRibbons(lines, levels, minW, keep = () => true) {
  const P = [], F = [], idx = [];
  lines.forEach((pts, li) => {
    if (!keep(pts)) return;
    const lv = levels[li];
    for (let i = 0; i < pts.length; i++) {
      const a = pts[Math.max(0, i - 1)], b = pts[Math.min(pts.length - 1, i + 1)];
      const dx = b[0] - a[0], dz = b[1] - a[1], L = Math.hypot(dx, dz) || 1, nx = -dz / L, nz = dx / L, w = Math.max(pts[i][4], minW) / 2;
      const base = P.length / 3;
      P.push(pts[i][0] + nx * w, lv[i], pts[i][1] + nz * w, pts[i][0] - nx * w, lv[i], pts[i][1] - nz * w);
      F.push(dx / L, dz / L, 1, dx / L, dz / L, 1);
      if (i < pts.length - 1) idx.push(base, base + 1, base + 2, base + 1, base + 3, base + 2);
    }
  });
  const g = new THREE.BufferGeometry();
  g.setAttribute('position', new THREE.Float32BufferAttribute(P, 3));
  g.setAttribute('aFlow', new THREE.Float32BufferAttribute(F, 3));
  g.setIndex(idx);
  return g;
}

// --- markers drawn over the picture --------------------------------------------------------------------------------
const px = (ctx, x, y, c) => { ctx.fillStyle = c; ctx.fillRect(Math.round(x), Math.round(y), 1, 1); };
const OUT = '#18131f';

/** A camp's fire as a glowing point (PRE-28); big: brighter and wider at dusk and night. */
export function fireGlow(ctx, x, y, { big = false } = {}) {
  if (big) {
    for (const [dx, dy] of [[0, -2], [-1, -1], [0, -1], [1, -1], [-2, 0], [-1, 0], [1, 0], [2, 0], [-1, 1], [0, 1], [1, 1], [0, 2]]) px(ctx, x + dx, y + dy, 'rgba(240,120,40,0.55)');
  }
  for (const [dx, dy] of [[0, -1], [-1, 0], [1, 0], [0, 1]]) px(ctx, x + dx, y + dy, '#ffad3a');
  px(ctx, x, y, '#fff4c0');
}

/**
 * A person as a tiny outlined figure in their strongest colour (PRE-28): a column of legs, body and head, one pixel
 * wide, outlined in the darkest shade of what they wear, as everything is outlined in its own colour (PRE-21).
 * legs: their colour (skin when bare); children are a pixel shorter.
 */
export function tinyFigure(ctx, x, y, { skin = '#c78e65', body = '#9c3a2a', legs = '#5a3a2a', edge = OUT, child = false } = {}) {
  const col = child ? [legs, body, skin] : [legs, body, body, skin];   // from the feet up; feet at (x, y)
  const h = col.length;
  for (let k = 0; k < h; k++) { px(ctx, x - 1, y - k, edge); px(ctx, x + 1, y - k, edge); px(ctx, x, y - k, col[k]); }
  px(ctx, x, y + 1, edge); px(ctx, x, y - h, edge);
}

/** A group of people as one marker (PRE-28): a small banner with a figure, pinned to the ground at (x, y). */
export function groupMarker(ctx, x, y, { colour = '#b44f39', skin = '#dfac81' } = {}) {
  const shape = [
    '.ooooooo.',
    'ocbbbbbco',
    'ocbbsbbco',
    'ocbsssbco',
    'ocbbsbbco',
    'ocbsssbco',
    'ocbsbsbco',
    'ocbbbbbco',
    '.ooooooo.',
    '....o....',
    '....o....',
  ];
  const col = { o: OUT, c: colour, b: colour, s: skin };
  shape.forEach((row, j) => [...row].forEach((ch, i) => { if (ch !== '.') px(ctx, x - 4 + i, y - 10 + j, col[ch]); }));
}

/** A herd as one marker (PRE-28): an animal's outline in its coat colour. kind: deer, horse, bison. */
export function herdMarker(ctx, x, y, { kind = 'deer', coat = '#9d5f3f' } = {}) {
  const S = {
    deer: ['.....o.', '....o..', 'ooooo..', 'ccccco.', 'ccccc..', 'o.o.o..'],
    horse: ['.....oo', 'oooooc.', 'cccccc.', 'ccccc..', 'o.o.o..'],
    bison: ['.ooo...', 'occco..', 'cccccoo', 'ccccccc', 'o.o..o.'],
  }[kind];
  // outline the shape: every empty pixel next to a filled one becomes dark
  const H = S.length, W = S[0].length, filled = (i, j) => j >= 0 && j < H && i >= 0 && i < W && S[j][i] !== '.';
  for (let j = -1; j <= H; j++) for (let i = -1; i <= W; i++) {
    const at = [x - Math.floor(W / 2) + i, y - H + j];
    if (filled(i, j)) px(ctx, ...at, S[j][i] === 'o' ? coat : coat);
    else if (filled(i - 1, j) || filled(i + 1, j) || filled(i, j - 1) || filled(i, j + 1)) px(ctx, ...at, OUT);
  }
}

export const MARK = { OUT, ramp: (m, k) => RAMPS[m][k] };
