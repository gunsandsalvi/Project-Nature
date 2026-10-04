// Helpers for building scenes: placing things on the ground, rivers, scattering low plants, soot and decals.
import * as THREE from 'three';
import { Solid, Cards, PAT, FLAG, Rand, M4 } from '../geo.js';
import { TILE } from '../atlas.js';

/** Ground helpers from a height function: y at a point, the normal, and a placer for kit pieces. */
export function land(height) {
  const at = (x, z) => height(x, z);
  const normal = (x, z) => {
    const e = 0.2;
    return new THREE.Vector3(-(height(x + e, z) - height(x - e, z)) / (2 * e), 1, -(height(x, z + e) - height(x, z - e)) / (2 * e)).normalize().toArray();
  };
  const place = (x, z) => [height(x, z), normal(x, z)];
  return { at, normal, place };
}

/** Moves a built piece ({ solid, cards }) to x, z on the ground (or at y), turned by yaw radians; adds it to the painter. */
export function put(P, piece, x, z, { y, yaw = 0, sink = 0, ground, scale = 1 } = {}) {
  const yy = y ?? (ground ? ground(x, z) : 0);
  const m = M4.mul(M4.T(x, yy - sink, z), M4.R(0, yaw, 0), M4.S(scale, scale, scale));
  if (piece.solid) { const s = new Solid(); s.add(piece.solid, m); P.addSolid(s); }
  if (piece.cards && piece.cards.n) P.addCards(moveCards(piece.cards, m));
  if (piece.fire) {
    const f = new THREE.Vector3(piece.fire[0], piece.fire[1], piece.fire[2]).applyMatrix4(m);
    P.addFire(f.toArray(), piece.fire[3], piece.fire[4]);
  }
  return m;
}

export function moveCards(cards, m) {
  const c = cards.d.aCenter, v = new THREE.Vector3();
  const s = new THREE.Vector3(); m.decompose(new THREE.Vector3(), new THREE.Quaternion(), s);
  for (let i = 0; i < c.length; i += 3) {
    v.set(c[i], c[i + 1], c[i + 2]).applyMatrix4(m);
    c[i] = v.x; c[i + 1] = v.y; c[i + 2] = v.z;
  }
  const n = cards.d.aNrm, q = new THREE.Quaternion(); m.decompose(new THREE.Vector3(), q, new THREE.Vector3());
  for (let i = 0; i < n.length; i += 3) { v.set(n[i], n[i + 1], n[i + 2]).applyQuaternion(q); n[i] = v.x; n[i + 1] = v.y; n[i + 2] = v.z; }
  if (s.x !== 1) for (let i = 0; i < cards.d.aSize.length; i++) cards.d.aSize[i] *= s.x;
  return cards;
}

/**
 * A river: a water ribbon along a centre line. centre(t) gives [x, z] for t in 0..1; width(t) its width;
 * level(x, z) the water height. Returns the geometry for Painter.addWater.
 */
export function riverSheet({ centre, width, level, n = 160, sea = 0 }) {
  const P = [], F = [], idx = [];
  for (let i = 0; i <= n; i++) {
    const t = i / n, [x, z] = centre(t), [x2, z2] = centre(Math.min(1, t + 0.001)), [x0, z0] = centre(Math.max(0, t - 0.001));
    const dx = x2 - x0, dz = z2 - z0, L = Math.hypot(dx, dz) || 1, nx = -dz / L, nz = dx / L, w = width(t) / 2;
    const y = level(x, z);
    P.push(x + nx * w, y, z + nz * w, x - nx * w, y, z - nz * w);
    F.push(dx / L, dz / L, 1, dx / L, dz / L, 1);
    if (i < n) { const a = i * 2; idx.push(a, a + 1, a + 2, a + 1, a + 3, a + 2); }
  }
  const g = new THREE.BufferGeometry();
  g.setAttribute('position', new THREE.Float32BufferAttribute(P, 3));
  g.setAttribute('aFlow', new THREE.Float32BufferAttribute(F, 3));
  g.setIndex(idx);
  return g;
}

/**
 * The nearest point on a centre line, for shaping ground round a river or a path: near(x, z) gives
 * { d: distance, t: 0..1 along the line, side: +1 or -1, bend: curvature sign there (+ turns left) }.
 */
export function lineField(centre, n = 240) {
  const P = Array.from({ length: n + 1 }, (_, i) => centre(i / n));
  const bendAt = (i) => {
    const a = P[Math.max(0, i - 3)], b = P[i], c = P[Math.min(n, i + 3)];
    return Math.sign((b[0] - a[0]) * (c[1] - b[1]) - (b[1] - a[1]) * (c[0] - b[0]));
  };
  const bends = P.map((_, i) => bendAt(i));
  return (x, z) => {
    let best = 1e9, bt = 0, bs = 1, bi = 0;
    for (let i = 0; i < n; i++) {
      const [ax, az] = P[i], [bx, bz] = P[i + 1];
      const vx = bx - ax, vz = bz - az, L2 = vx * vx + vz * vz || 1;
      const u = Math.max(0, Math.min(1, ((x - ax) * vx + (z - az) * vz) / L2));
      const px = ax + vx * u, pz = az + vz * u, d = Math.hypot(x - px, z - pz);
      if (d < best) { best = d; bt = (i + u) / n; bs = Math.sign(vx * (z - az) - vz * (x - ax)) || 1; bi = i; }
    }
    return { d: best, t: bt, side: bs, bend: bends[bi] };
  };
}

/** A flat sheet of water (a lake or the sea) over a rectangle, still unless it runs along flow. */
export function waterPlane({ x0, x1, z0, z1, y, flow = [1, 0], still = true }) {
  const g = new THREE.BufferGeometry();
  g.setAttribute('position', new THREE.Float32BufferAttribute([x0, y, z0, x1, y, z0, x1, y, z1, x0, y, z1], 3));
  g.setAttribute('aFlow', new THREE.Float32BufferAttribute([0, 1, 2, 3].flatMap(() => [flow[0], flow[1], still ? 0 : 1]), 3));
  g.setIndex([0, 2, 1, 0, 3, 2]);
  return g;
}

/** Darkens a solid's vertices near a point by up to `steps` (soot above a hearth, damp stains). */
export function stain(solid, centre, radius, steps = -2, squash = [1, 0.6, 1]) {
  for (let i = 0; i < solid.P.length / 3; i++) {
    const dx = (solid.P[i * 3] - centre[0]) / squash[0], dy = (solid.P[i * 3 + 1] - centre[1]) / squash[1], dz = (solid.P[i * 3 + 2] - centre[2]) / squash[2];
    const d = Math.hypot(dx, dy, dz) / radius;
    if (d < 1) solid.A[i * 5 + 1] += steps * (1 - d);
  }
}

/**
 * A picture painted on rock: rows of characters ('#' paint, '.' none), one cell per `cell` metres, on the plane
 * through origin spanned by right and up, nudged off the wall along normal.
 */
export function decal(solid, rows, { origin, right, up, normal, cell = 0.06, mat = 'dyedred', obj = 0 }) {
  const o = new THREE.Vector3(...origin).addScaledVector(new THREE.Vector3(...normal), 0.02);
  const R = new THREE.Vector3(...right).multiplyScalar(cell), U = new THREE.Vector3(...up).multiplyScalar(cell);
  rows.forEach((row, j) => [...row].forEach((ch, i) => {
    if (ch === '.' || ch === ' ') return;
    const p = o.clone().addScaledVector(R, i).addScaledVector(U, -j);
    const a = p.toArray(), b = p.clone().add(R).toArray(), c = p.clone().add(R).sub(U).toArray(), d = p.clone().sub(U).toArray();
    solid.quad(a, b, c, d, { mat: ch === 'k' ? 'charcoal' : (ch === 'o' ? 'ochre' : mat), obj, flag: FLAG.NOOUTLINE });
  }));
}
