// Ground, rocks and cliffs.
import * as THREE from 'three';
import { ConvexGeometry } from 'three/addons/geometries/ConvexGeometry.js';
import { Solid, PAT, FLAG, Rand, noise2, fbm, clamp, smooth, newObj, M4 } from '../geo.js';

/** A block with its corners cut evenly (chip: the share of the shortest side cut off), moved by matrix m into solid s. */
export function bevelBox(s, m, sx, sy, sz, a, chip = 0.25, taper = 1) {
  const h = [sx / 2, sy / 2, sz / 2], k = Math.min(...h) * 2 * chip;
  const pts = [];
  for (const cx of [-1, 1]) for (const cy of [-1, 1]) for (const cz of [-1, 1]) {
    const t = cy > 0 ? taper : 1;
    const c = [cx * h[0] * t, cy * h[1], cz * h[2] * t];
    pts.push(new THREE.Vector3(c[0] - cx * k, c[1], c[2]), new THREE.Vector3(c[0], c[1] - cy * k, c[2]), new THREE.Vector3(c[0], c[1], c[2] - cz * k));
  }
  const g = new ConvexGeometry(pts);
  const pos = g.attributes.position, v = new THREE.Vector3();
  for (let i = 0; i < pos.count; i += 3) {
    const t = [0, 1, 2].map((j) => v.fromBufferAttribute(pos, i + j).clone().applyMatrix4(m).toArray());
    s.tri(t[0], t[1], t[2], a);
  }
}

/**
 * A chiselled block of stone: a box with its edges and corners knocked off at random, all flat facets, like
 * cut and weathered rock. Faces that look up take moss or grass. size: [x, y, z]; chip: how much is knocked off.
 */
export function stoneBlock({ seed = 1, size = [2, 1, 1.5], chip = 0.22, mat = 'rock', top = 'moss', topPat = PAT.MOSS, topMin = 0.8, obj, flag = 0, bias = 0 }) {
  const r = new Rand(seed);
  const [sx, sy, sz] = size.map((v) => v / 2);
  const pts = [];
  for (const cx of [-1, 1]) for (const cy of [-1, 1]) for (const cz of [-1, 1]) {
    const k = [sx, sy, sz].map((h, i) => Math.min(h * 0.9, Math.min(sx, sy, sz) * 2) * chip * r.range(0.4, 1.4));
    const c = [cx * sx, cy * sy, cz * sz];
    pts.push([c[0] - cx * k[0], c[1], c[2]], [c[0], c[1] - cy * k[1], c[2]], [c[0], c[1], c[2] - cz * k[2]]);
  }
  for (let i = 0; i < 6; i++) { // a few dents and bulges on the faces
    const ax = r.int(0, 2), sgn = r.chance(0.5) ? 1 : -1;
    const p = [r.range(-sx, sx) * 0.7, r.range(-sy, sy) * 0.7, r.range(-sz, sz) * 0.7];
    p[ax] = sgn * [sx, sy, sz][ax] * r.range(0.96, 1.04);
    pts.push(p);
  }
  const jit = Math.min(sx, sy, sz) * 0.08;
  const g = new ConvexGeometry(pts.map((p) => new THREE.Vector3(p[0] + r.range(-jit, jit), p[1] + r.range(-jit, jit) * 0.5, p[2] + r.range(-jit, jit))));
  const pos = g.attributes.position, s = new Solid();
  obj = obj ?? newObj();
  for (let i = 0; i < pos.count; i += 3) {
    const t = [0, 1, 2].map((k) => [pos.getX(i + k), pos.getY(i + k), pos.getZ(i + k)]);
    const e1 = new THREE.Vector3(...t[1]).sub(new THREE.Vector3(...t[0])), e2 = new THREE.Vector3(...t[2]).sub(new THREE.Vector3(...t[0]));
    const ny = e1.cross(e2).normalize().y;
    const up = top && ny > topMin;
    s.tri(t[0], t[1], t[2], up ? { mat: top, pat: topPat, obj, flag, bias } : { mat, pat: PAT.ROCK, obj, flag, bias });
  }
  return s;
}

/**
 * A heightfield: height(x, z) in metres; weights(x, z, y, slope) gives four layer weights (see Painter.addSolid's
 * layers). Returns the Solid; it is smooth-shaded so slopes shade softly in steps.
 */
export function ground({ x0, x1, z0, z1, step = 0.5, height, weights, obj = 0 }) {
  const nx = Math.round((x1 - x0) / step) + 1, nz = Math.round((z1 - z0) / step) + 1;
  const P = new Float32Array(nx * nz * 3), N = new Float32Array(nx * nz * 3), W = new Float32Array(nx * nz * 4);
  const H = (x, z) => height(x, z);
  for (let j = 0; j < nz; j++) for (let i = 0; i < nx; i++) {
    const x = x0 + i * step, z = z0 + j * step, y = H(x, z), k = j * nx + i;
    P[k * 3] = x; P[k * 3 + 1] = y; P[k * 3 + 2] = z;
    const e = 0.25;
    const dx = (H(x + e, z) - H(x - e, z)) / (2 * e), dz = (H(x, z + e) - H(x, z - e)) / (2 * e);
    const n = new THREE.Vector3(-dx, 1, -dz).normalize();
    N[k * 3] = n.x; N[k * 3 + 1] = n.y; N[k * 3 + 2] = n.z;
    const w = weights ? weights(x, z, y, 1 - n.y) : [1, 0, 0, 0];
    W.set(w, k * 4);
  }
  const idx = [];
  for (let j = 0; j < nz - 1; j++) for (let i = 0; i < nx - 1; i++) {
    const a = j * nx + i, b = a + 1, c = a + nx, d = c + 1;
    idx.push(a, c, b, b, c, d);
  }
  const s = new Solid();
  s.smooth(P, N, idx, { mat: 0, pat: PAT.TERRAIN, obj }, null, W);
  return s;
}

const _p = new THREE.Vector3();

/**
 * A boulder: a chiselled lump of facets, with moss on the faces that look up. size: [x, y, z] in metres.
 * detail 0 for pebbles, 1 for stones and boulders, 2 for big rocks.
 */
export function boulder({ seed = 1, size = [1, 0.7, 0.9], detail = 1, mat = 'rock', moss = 0.45, mossMat = 'moss', sink = 0.18, obj }) {
  const r = new Rand(seed);
  obj = obj ?? newObj();
  const g = new THREE.IcosahedronGeometry(1, detail);
  const pos = g.attributes.position;
  const off = [r.range(0, 99), r.range(0, 99)];
  const shape = (v) => {
    const n = fbm(v.x * 1.3 + off[0], v.z * 1.3 + v.y * 0.7 + off[1], 3);
    const k = 0.78 + n * 0.45;
    v.multiplyScalar(k);
    if (v.y < -sink * 2) v.y = -sink * 2 + (v.y + sink * 2) * 0.25; // a flatter underside
    v.set(v.x * size[0] / 2, (v.y + sink) * size[1] / 2, v.z * size[2] / 2);
    return v;
  };
  const s = new Solid();
  const rot = r.range(0, Math.PI * 2), cs = Math.cos(rot), sn = Math.sin(rot);
  for (let i = 0; i < pos.count; i += 3) {
    const t = [0, 1, 2].map((k) => {
      _p.fromBufferAttribute(pos, i + k);
      const v = shape(_p.clone());
      return [v.x * cs - v.z * sn, v.y, v.x * sn + v.z * cs];
    });
    const e1 = new THREE.Vector3(...t[1]).sub(new THREE.Vector3(...t[0])), e2 = new THREE.Vector3(...t[2]).sub(new THREE.Vector3(...t[0]));
    const ny = e1.clone().cross(e2).normalize().y;
    const cy = (t[0][1] + t[1][1] + t[2][1]) / 3;
    const mossy = moss > 0 && ny > 0.62 - moss * 0.35 + (noise2(t[0][0] * 3 + off[0], t[0][2] * 3) - 0.5) * 0.3 && cy > size[1] * 0.2;
    s.tri(t[0], t[1], t[2], { mat: mossy ? mossMat : mat, pat: mossy ? PAT.MOSS : PAT.ROCK, obj });
  }
  return s;
}

/**
 * A cliff of chiselled blocks (PRE-23): beds of different thickness along a line, each a row of stone blocks that
 * jut or sit back, overlap a little and lean at random, so the face is rough and natural; ledges take moss and the
 * top takes grass. path: points [x, z] along the foot, open side on the left (a path toward +x faces +z).
 * overhang: { bed, from, to, out, recess } in metres along the path makes a rock shelter.
 */
export function blockCliff({ seed = 1, path, base = 0, beds = [1.6, 1.1, 1.8, 1.2, 1.5, 1.3], mat = 'rock', moss = 'moss',
  grass = 'grass', overhang = null, jut = 0.3, setback = 0.28, len = [0.9, 3.6], depth = 5.5, bedMats = null, obj }) {
  const r = new Rand(seed);
  obj = obj ?? newObj();
  const s = new Solid();
  const segs = []; let total = 0;
  for (let i = 0; i < path.length - 1; i++) {
    const a = path[i], b = path[i + 1], L = Math.hypot(b[0] - a[0], b[1] - a[1]);
    segs.push({ a, b, L, s0: total }); total += L;
  }
  const at = (d) => {
    d = clamp(d, 0, total - 1e-6);
    const q = segs.find((g) => d <= g.s0 + g.L) || segs[segs.length - 1], t = (d - q.s0) / q.L;
    const dir = [(q.b[0] - q.a[0]) / q.L, (q.b[1] - q.a[1]) / q.L];
    return { p: [q.a[0] + (q.b[0] - q.a[0]) * t, q.a[1] + (q.b[1] - q.a[1]) * t], dir, n: [-dir[1], dir[0]] };
  };
  const ease = (x, a, b, w) => smooth(a - w, a + w, x) * (1 - smooth(b - w, b + w, x));
  let y = base, i = 0;
  const offs = [];
  beds.forEach((th, bi) => {
    let d = -1;
    const row = [];
    while (d < total + 1) {
      const L = len[0] + (len[1] - len[0]) * r.next() ** 1.6, mid = d + L / 2, A = at(mid);
      // the face bulges and bays over several beds at once, as weathered rock does, so the beds never line up
      let out = r.range(-jut, jut) - bi * setback + 1.1 * (noise2(mid * 0.11 + seed, bi * 0.3) - 0.5);
      if (overhang) {
        const e = ease(mid, overhang.from, overhang.to, 0.9);
        if (bi === overhang.bed) out += overhang.out * e;
        if (bi < overhang.bed) out -= overhang.recess * e * (0.8 + 0.2 * bi / Math.max(1, overhang.bed - 1));
      }
      const h = th * r.range(0.82, 1.25), dep = depth + Math.max(0, out), dy = r.range(-0.18, 0.12);
      const cx = A.p[0] + A.n[0] * (out - dep / 2), cz = A.p[1] + A.n[1] * (out - dep / 2);
      const yaw = Math.atan2(A.dir[1], A.dir[0]);
      const m = M4.mul(M4.T(cx, y + h / 2 - 0.04 + dy, cz), M4.R(r.range(-0.06, 0.06), -yaw + r.range(-0.12, 0.12), r.range(-0.1, 0.1)));
      const isTop = bi === beds.length - 1;
      s.add(stoneBlock({ seed: seed * 1000 + i++, size: [L * 1.12, h + 0.08, dep], chip: r.range(0.2, 0.48), mat: bedMats ? bedMats[bi] : mat,
        top: isTop ? grass : moss, topPat: isTop ? PAT.GROUND : PAT.MOSS, topMin: isTop ? 0.75 : 0.85, obj, bias: r.chance(0.3) ? (r.chance(0.5) ? -0.5 : 0.5) : 0 }), m);
      row.push({ s: mid, out, top: y + h + dy - 0.04, L });
      d += L * r.range(0.84, 0.96);
    }
    offs.push(row);
    y += th;
  });
  return { solid: s, top: y, at, total, beds: offs };
}

/**
 * A cliff of rock beds (PRE-23): beds of different thickness stacked along a line, each cut into blocks that jut
 * or sit back a little, so the face is chiselled but continuous; narrow ledges carry grass. path: points [x, z]
 * along the cliff foot, walked so the open side is on the left (for a path toward +x the face looks to +z).
 * beds: thicknesses from the bottom. overhang: { bed, from, to, out, recess } in metres along the path makes a
 * rock shelter: that bed juts out, the beds below sit back. Returns { solid, top, front(s, bed) }.
 */
export function cliff({ seed = 1, path, base = 0, beds = [1.4, 0.9, 1.6, 1.0, 1.4], depth = 8, mat = 'rock', grass = 'grass',
  blockLen = [1.6, 4.2], jut = 0.22, setback = 0.3, overhang = null, ledgeGrass = 0.65, obj }) {
  const r = new Rand(seed);
  obj = obj ?? newObj();
  const s = new Solid();
  // the path resampled every ds metres
  const ds = 0.25, pts = [];
  for (let i = 0; i < path.length - 1; i++) {
    const a = path[i], b = path[i + 1], L = Math.hypot(b[0] - a[0], b[1] - a[1]), n = Math.max(1, Math.round(L / ds));
    for (let k = 0; k < n; k++) pts.push([a[0] + (b[0] - a[0]) * (k / n), a[1] + (b[1] - a[1]) * (k / n)]);
  }
  pts.push(path[path.length - 1]);
  const N = pts.length;
  const nrm = pts.map((p, i) => {
    const a = pts[Math.max(0, i - 1)], b = pts[Math.min(N - 1, i + 1)];
    const dx = b[0] - a[0], dz = b[1] - a[1], L = Math.hypot(dx, dz) || 1;
    return [-dz / L, dx / L];
  });
  const S = pts.map((_, i) => i * ds);
  const ease = (x, a, b, w) => smooth(a - w, a + w, x) * (1 - smooth(b - w, b + w, x));
  // per bed: thickness along the path, and the face's offset (block by block)
  const B = beds.map((th, bi) => {
    const off = new Float32Array(N), thick = new Float32Array(N), cut = new Uint8Array(N);
    let i = 0;
    while (i < N) {
      const len = Math.max(2, Math.round(r.range(...blockLen) / ds));
      const o = r.range(-jut, jut) - bi * setback;
      for (let k = i; k < Math.min(N, i + len); k++) off[k] = o + 0.05 * (noise2(k * 0.4, bi * 7 + seed) - 0.5);
      cut[i] = 1;
      i += len;
    }
    for (let k = 0; k < N; k++) {
      thick[k] = th * (0.88 + 0.24 * noise2(k * ds * 0.15 + bi * 3, seed));
      if (overhang) {
        const e = ease(S[k], overhang.from, overhang.to, 1.2);
        if (bi === overhang.bed) off[k] += overhang.out * e;
        if (bi < overhang.bed) off[k] -= overhang.recess * e * (0.75 + 0.25 * bi / Math.max(1, overhang.bed));
      }
    }
    return { off, thick, cut };
  });
  // heights of each bed's bottom and top along the path
  const y0 = beds.map(() => new Float32Array(N)), y1 = beds.map(() => new Float32Array(N));
  for (let k = 0; k < N; k++) {
    let y = base;
    beds.forEach((_, bi) => { y0[bi][k] = y; y += B[bi].thick[k]; y1[bi][k] = y; });
  }
  const P3 = (k, o, y) => [pts[k][0] + nrm[k][0] * o, y, pts[k][1] + nrm[k][1] * o];
  const rockA = { mat, pat: PAT.ROCK, obj };
  const u = 0.09, c = 0.15, lean = 0.07; // undercut, chamfer, lean of the face
  beds.forEach((_, bi) => {
    const { off, cut } = B[bi];
    const next = bi + 1 < beds.length ? B[bi + 1].off : null;
    // the profile of the bed's front at path point k: [offset, height] from bottom to top
    const prof = (k, o) => {
      const a = y0[bi][k], b = y1[bi][k], h = b - a;
      const top = o - (h - c) * lean;
      return [[o - u, a], [o, a + u], [top, b - c], [top - c, b]];
    };
    for (let k = 0; k < N - 1; k++) {
      const pa = prof(k, off[k]), pb = prof(k + 1, off[k]);
      for (let j = 0; j < 3; j++) {
        const bias = j === 0 ? -1 : (j === 2 ? 0.6 : 0);
        s.quad(P3(k, pa[j][0], pa[j][1]), P3(k + 1, pb[j][0], pb[j][1]), P3(k + 1, pb[j + 1][0], pb[j + 1][1]), P3(k, pa[j + 1][0], pa[j + 1][1]), { ...rockA, bias });
      }
      // a step where the next block juts differently
      if (cut[k + 1] && Math.abs(off[k + 1] - off[k]) > 0.01) {
        const qa = prof(k + 1, off[k]), qb = prof(k + 1, off[k + 1]);
        for (let j = 0; j < 3; j++) s.quad(P3(k + 1, qa[j][0], qa[j][1]), P3(k + 1, qb[j][0], qb[j][1]), P3(k + 1, qb[j + 1][0], qb[j + 1][1]), P3(k + 1, qa[j + 1][0], qa[j + 1][1]), rockA);
      }
      // the ledge on top: up to the next bed's face, or back into the hill for the top bed
      const back = next ? next[k] - u : -depth;
      const edge = pa[3][0];
      if (back < edge - 0.02) {
        const wide = edge - back;
        const g = bi + 1 === beds.length || (wide > 0.3 && noise2(S[k] * 0.35 + bi * 5, seed + 3) < ledgeGrass);
        s.quad(P3(k, edge, y1[bi][k]), P3(k, back, y1[bi][k]), P3(k + 1, back, y1[bi][k + 1]), P3(k + 1, pb[3][0], y1[bi][k + 1]),
          g ? { mat: grass, pat: PAT.GROUND, obj } : { ...rockA, bias: 0.5 });
      } else if (next && back > edge + 0.02) {
        // the underside of an overhanging bed
        s.quad(P3(k, edge, y1[bi][k]), P3(k + 1, pb[3][0], y1[bi][k + 1]), P3(k + 1, back, y1[bi][k + 1]), P3(k, back, y1[bi][k]), { ...rockA, bias: -1 });
      }
    }
  });
  const top = base + beds.reduce((a, b) => a + b, 0);
  const front = (k, bi) => B[bi].off[clamp(k, 0, N - 1)];
  return { solid: s, top, pts, nrm, front, ds };
}
