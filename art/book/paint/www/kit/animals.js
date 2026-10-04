// Animals (PRE-27, PRE-46): small figures of bevelled blocks, a few body patterns, each species its proportions and
// colours, with antlers, horns or tusks. Facing +z; poses: stand, graze, walk, run, alert, lie.
import * as THREE from 'three';
import { Solid, FLAG, Rand, newObj, M4 } from '../geo.js';
import { bevelBox } from './land.js';

const { T, R } = M4;
const mul = (...m) => m.reduce((a, b) => a.clone().multiply(b));
const D = Math.PI / 180;

// body: [length, height, width] of the barrel; legH: leg length; neck: [length, angle up from level]; head: [length, height, width]
export const SPECIES = {
  stag:     { coat: 'deer', belly: 'fur', rump: 'bone', body: [1.15, 0.52, 0.38], legH: 0.86, neck: [0.62, 55], head: [0.44, 0.2, 0.17], ears: 0.15, tail: 0.1, antlers: 'stag' },
  hind:     { coat: 'deer', belly: 'fur', rump: 'bone', body: [1.0, 0.46, 0.34], legH: 0.8, neck: [0.55, 52], head: [0.4, 0.18, 0.15], ears: 0.17, tail: 0.1 },
  reindeer: { coat: 'reindeer', belly: 'white', mane: 'white', body: [1.05, 0.52, 0.4], legH: 0.68, neck: [0.48, 40], head: [0.44, 0.2, 0.17], ears: 0.1, tail: 0.08, antlers: 'reindeer' },
  horse:    { coat: 'dun', belly: 'fur', legs: 'furdark', mane: 'hair', body: [1.25, 0.6, 0.42], legH: 0.82, neck: [0.72, 50], head: [0.56, 0.24, 0.2], ears: 0.12, tail: 0.55, tailMat: 'hair' },
  bison:    { coat: 'bison', belly: 'bison', hump: 'furdark', body: [1.8, 0.95, 0.75], legH: 0.6, neck: [0.3, 8], head: [0.5, 0.42, 0.36], ears: 0.08, tail: 0.35, horns: 'short', beard: true },
  aurochs:  { coat: 'charcoal', belly: 'charcoal', body: [2.0, 0.85, 0.7], legH: 0.85, neck: [0.42, 18], head: [0.56, 0.32, 0.3], ears: 0.1, tail: 0.6, horns: 'lyre', stripe: 'flowerw' },
  goat:     { coat: 'goat', belly: 'goat', body: [0.72, 0.36, 0.28], legH: 0.42, neck: [0.32, 55], head: [0.3, 0.15, 0.13], ears: 0.1, tail: 0.08, horns: 'back', beard: true },
  sheep:    { coat: 'white', belly: 'white', legs: 'charcoal', body: [0.75, 0.42, 0.36], legH: 0.38, neck: [0.24, 45], head: [0.26, 0.15, 0.13], headMat: 'charcoal', ears: 0.08, tail: 0.1 },
  boar:     { coat: 'boar', belly: 'boar', body: [1.0, 0.55, 0.42], legH: 0.36, neck: [0.2, 5], head: [0.48, 0.3, 0.26], ears: 0.1, tail: 0.15, tusks: true, bristle: true },
  wolf:     { coat: 'wolf', belly: 'white', body: [0.85, 0.34, 0.25], legH: 0.44, neck: [0.28, 40], head: [0.32, 0.17, 0.16], ears: 0.1, tail: 0.45, snout: true, bushy: true, pointy: true },
  dog:      { coat: 'dun', belly: 'fur', body: [0.56, 0.26, 0.2], legH: 0.32, neck: [0.2, 45], head: [0.24, 0.14, 0.13], ears: 0.08, tail: 0.28, snout: true, pointy: true, curl: true },
  hare:     { coat: 'dun', belly: 'white', body: [0.38, 0.2, 0.16], legH: 0.14, neck: [0.06, 60], head: [0.14, 0.11, 0.1], ears: 0.14, tail: 0.05, long: true },
};

const POSES = {
  stand: { neck: 0, legs: [0, 0, 0, 0] },
  graze: { neck: -82, legs: [6, -6, 0, 0], head: 50 },
  walk:  { neck: -8, legs: [22, -18, -18, 22] },
  run:   { neck: -20, legs: [55, 30, -50, -35], body: -4 },
  alert: { neck: 16, legs: [0, 0, 0, 0], head: -10 },
  lie:   { neck: 4, legs: [0, 0, 0, 0], lie: true },
};

/** An animal of a species in a pose; scale for big and small ones. */
export function animal({ species = 'hind', pose = 'stand', seed = 1, scale = 1 }) {
  const S = SPECIES[species], P = POSES[pose] || POSES.stand, r = new Rand(seed);
  const obj = newObj(), legsObj = [newObj(), newObj(), newObj(), newObj()], headObj = newObj();
  const s = new Solid();
  const A = (mat, o = obj, bias = 0) => ({ mat, obj: o, flag: FLAG.CREATURE, bias });
  const bb = (m, sx, sy, sz, mat, o = obj, chip = 0.3, taper = 1, bias = 0) => bevelBox(s, mul(m, M4.S(scale)), sx, sy, sz, A(mat, o, bias), chip, taper);
  const [bl, bh, bw] = S.body;
  const legH = S.lie ? S.legH : S.legH;
  const bodyY = (P.lie ? bh * 0.55 : legH + bh / 2);
  const body = mul(T(0, bodyY * scale, 0), R((P.body || 0) * D, 0, 0));
  // chest, barrel and haunch: the chest deepest, the waist narrower
  bb(mul(body, T(0, bh * 0.02 * scale, bl * 0.3 * scale)), bw * 1.04, bh * 1.08, bl * 0.46, S.coat, obj, 0.32);
  bb(mul(body, T(0, bh * 0.04 * scale, -bl * 0.02 * scale)), bw * 0.92, bh * 0.9, bl * 0.42, S.coat, obj, 0.3);
  bb(mul(body, T(0, bh * 0.06 * scale, -bl * 0.3 * scale)), bw * 1.0, bh * 0.98, bl * 0.4, S.coat, obj, 0.36);
  bb(mul(body, T(0, -bh * 0.38 * scale, bl * 0.02 * scale)), bw * 0.78, bh * 0.22, bl * 0.62, S.belly || S.coat, obj, 0.3, 1, 1);
  if (S.rump) bb(mul(body, T(0, bh * 0.08 * scale, -bl * 0.5 * scale)), bw * 0.7, bh * 0.56, 0.06, S.rump, obj, 0.2);
  if (S.hump) bb(mul(body, T(0, bh * 0.52 * scale, bl * 0.2 * scale)), bw * 0.86, bh * 0.6, bl * 0.52, S.hump, obj, 0.4, 0.8);
  if (S.bristle) bb(mul(body, T(0, bh * 0.5 * scale, 0)), bw * 0.3, bh * 0.2, bl * 0.86, 'furdark', obj, 0.3);
  if (S.stripe) bb(mul(body, T(0, bh * 0.49 * scale, 0)), bw * 0.14, 0.03, bl * 0.86, S.stripe, obj, 0.1);
  // neck and head
  const neckA = (S.neck[1] + P.neck) * D;
  const neck = mul(body, T(0, bh * 0.2 * scale, bl * 0.46 * scale), R(Math.PI / 2 - neckA, 0, 0));
  const nl = S.neck[0];
  bb(mul(neck, T(0, nl * 0.45 * scale, 0)), bw * 0.5, nl * 1.05, bh * 0.56, S.coat, obj, 0.3, 0.78);
  if (S.mane) bb(mul(neck, T(0, nl * 0.48 * scale, -bh * 0.2 * scale)), bw * 0.18, nl * 0.88, bh * 0.16, S.mane, obj, 0.2);
  const headM = mul(neck, T(0, nl * 0.95 * scale, 0), R(neckA - Math.PI / 2 + ((P.head || 0) + 18) * D, 0, 0));
  const [hl, hh, hw] = S.head, hm = S.headMat || S.coat;
  bb(mul(headM, T(0, 0, hl * 0.25 * scale)), hw, hh, hl * 0.62, hm, headObj, 0.3);
  bb(mul(headM, T(0, -hh * 0.14 * scale, hl * 0.66 * scale)), hw * 0.72, hh * 0.68, hl * 0.42, S.snout ? hm : (species === 'sheep' ? hm : 'furdark'), headObj, 0.3, 1, S.snout ? 0 : 0);
  for (const sx of [-1, 1]) bb(mul(headM, T(sx * hw * 0.5 * scale, hh * 0.14 * scale, hl * 0.3 * scale)), 0.02, 0.035, 0.035, 'charcoal', headObj, 0.1);
  for (const sx of [-1, 1]) {
    const ear = mul(headM, T(sx * hw * 0.4 * scale, hh * 0.45 * scale, 0), R(S.long ? -0.25 : 0.25, 0, sx * (S.pointy || S.long ? -0.15 : -0.8)));
    bb(mul(ear, T(0, S.ears * 0.5 * scale, 0)), 0.06, S.ears, 0.035, hm, headObj, 0.25, 0.6);
  }
  if (S.beard) bb(mul(headM, T(0, -hh * 0.62 * scale, hl * 0.3 * scale)), hw * 0.42, hh * 0.55, hl * 0.3, 'furdark', headObj, 0.3, 0.7);
  if (S.tusks) for (const sx of [-1, 1]) bb(mul(headM, T(sx * hw * 0.36 * scale, -hh * 0.18 * scale, hl * 0.82 * scale), R(-0.7, 0, 0)), 0.035, 0.11, 0.035, 'bone', headObj, 0.2);
  if (S.antlers) antlers(s, headM, S.antlers, scale, headObj);
  if (S.horns) horns(s, headM, S.horns, hw, hh, scale, headObj);
  // legs: front left, front right, back left, back right; a lying animal folds them under
  const legsMat = S.legs || S.coat;
  [[1, 1], [-1, 1], [1, -1], [-1, -1]].forEach(([sx, sz], i) => {
    if (P.lie) {
      bb(mul(body, T(sx * bw * 0.36 * scale, -bh * 0.42 * scale, sz * bl * 0.3 * scale)), bw * 0.24, bh * 0.22, legH * 0.55, legsMat, legsObj[i], 0.3);
      return;
    }
    const hip = mul(body, T(sx * bw * 0.3 * scale, -bh * 0.3 * scale, sz * bl * 0.36 * scale), R(-P.legs[i] * D, 0, 0));
    const up = legH * 0.52, lo = legH * 0.56;
    const k = sz > 0 ? -0.1 : 0.24;
    bb(mul(hip, T(0, -up * 0.45 * scale, 0)), bw * 0.26, up * 1.05, bw * 0.3, S.coat, legsObj[i], 0.3, 0.75);
    const knee = mul(hip, T(0, -up * scale, 0), R(k, 0, 0));
    bb(mul(knee, T(0, -lo * 0.5 * scale, 0)), bw * 0.14, lo, bw * 0.15, legsMat, legsObj[i], 0.2);
    bb(mul(knee, T(0, -lo * scale, 0.01 * scale)), bw * 0.17, 0.06, bw * 0.2, 'charcoal', legsObj[i], 0.2);
  });
  // tail
  const tail = mul(body, T(0, bh * 0.32 * scale, -bl * 0.5 * scale), R(S.curl ? -2.3 : (S.bushy ? -0.55 : -0.25), 0, 0));
  bb(mul(tail, T(0, -S.tail * 0.5 * scale, 0)), S.bushy ? 0.12 : 0.07, S.tail, S.bushy ? 0.12 : 0.06, S.tailMat || S.coat, obj, 0.3, S.bushy ? 0.7 : 1);

  let minY = Infinity;
  for (let i = 1; i < s.P.length; i += 3) minY = Math.min(minY, s.P[i]);
  const out = new Solid(); out.add(s, T(0, -minY, 0));
  return { solid: out, obj };
}

// Antlers as a main beam with tines, from the head's crown.
function antlers(s, head, kind, scale, obj) {
  const a = { mat: 'bone', obj, flag: FLAG.CREATURE };
  const shape = kind === 'reindeer'
    ? { beam: [[0.06, 0.1, -0.02], [0.16, 0.3, -0.16], [0.2, 0.52, -0.12], [0.16, 0.7, 0.04]], tines: [[1, [0.06, 0.12, 0.2]], [2, [0.12, 0.1, 0.1]], [3, [0.04, 0.1, 0.1]]], brow: true }
    : { beam: [[0.06, 0.1, -0.02], [0.14, 0.28, -0.1], [0.2, 0.5, -0.08], [0.16, 0.68, 0.0]], tines: [[1, [0.02, 0.06, 0.18]], [2, [0.02, 0.08, 0.16]], [3, [0.06, 0.12, 0.04]], [3, [-0.04, 0.1, 0.06]]] };
  for (const sx of [-1, 1]) {
    const P = shape.beam.map(([x, y, z]) => new THREE.Vector3(sx * x * scale, y * scale + 0.06, z * scale).applyMatrix4(head));
    for (let i = 0; i < P.length - 1; i++) s.beam(P[i].toArray(), P[i + 1].toArray(), (0.026 - i * 0.004) * scale, (0.022 - i * 0.004) * scale, 4, a);
    for (const [at, [x, y, z]] of shape.tines) {
      const q = P[at].clone().add(new THREE.Vector3(sx * x * scale, y * scale, z * scale).transformDirection(head).multiplyScalar(Math.hypot(x, y, z) * scale));
      s.beam(P[at].toArray(), q.toArray(), 0.016 * scale, 0.01 * scale, 4, a);
    }
    if (shape.brow) {
      const q = P[0].clone().add(new THREE.Vector3(0, 0.02, 0.2 * scale).transformDirection(head).multiplyScalar(0.2 * scale));
      s.beam(P[0].toArray(), q.toArray(), 0.016 * scale, 0.02 * scale, 4, a);
    }
  }
}

function horns(s, head, kind, hw, hh, scale, obj) {
  const a = { mat: kind === 'lyre' ? 'bone' : 'charcoal', obj, flag: FLAG.CREATURE };
  for (const sx of [-1, 1]) {
    const p0 = new THREE.Vector3(sx * hw * 0.45 * scale, hh * 0.45 * scale, 0.05 * scale).applyMatrix4(head);
    const pts = kind === 'lyre' ? [[sx * 0.25, 0.08, 0.05], [sx * 0.38, 0.28, 0.12], [sx * 0.3, 0.45, 0.16]]
      : kind === 'back' ? [[sx * 0.05, 0.12, -0.08], [sx * 0.09, 0.15, -0.22], [sx * 0.1, 0.06, -0.3]]
      : [[sx * 0.14, 0.06, 0.02], [sx * 0.18, 0.16, 0.04]];
    let p = p0;
    pts.forEach((q, i) => {
      const n = new THREE.Vector3(...q).multiplyScalar(scale).applyMatrix4(head);
      s.beam(p.toArray(), n.toArray(), (0.035 - i * 0.008) * scale, (0.027 - i * 0.008) * scale, 4, a);
      p = n;
    });
  }
}

/** A bird: perched, or flying with wings in a shallow V (wing 0..1 is the beat). kind: crow, duck, heron, gull, vulture, goose. */
export function bird({ kind = 'crow', fly = false, wing = 0.5, seed = 1, scale = 1 }) {
  const C = {
    crow: { body: 'feather', size: 0.4, neck: 0.05, legs: 0.08, beak: 'charcoal', span: 0.9 },
    duck: { body: 'reed', head: 'verdi', size: 0.45, neck: 0.06, legs: 0.03, beak: 'ochre', span: 0.8 },
    heron: { body: 'cloth', head: 'white', size: 0.55, neck: 0.32, legs: 0.45, beak: 'ochre', span: 1.6 },
    gull: { body: 'white', size: 0.45, neck: 0.05, legs: 0.08, beak: 'ochre', wingTip: 'charcoal', span: 1.2 },
    vulture: { body: 'furdark', head: 'skin1', size: 0.85, neck: 0.18, legs: 0.15, beak: 'bone', span: 2.4 },
    goose: { body: 'cloth', head: 'charcoal', size: 0.65, neck: 0.25, legs: 0.08, beak: 'charcoal', span: 1.5 },
  }[kind];
  const s = new Solid(), obj = newObj();
  const z = C.size * scale;
  const b = (m, sx, sy, sz, mat, chip = 0.25, taper = 1) => bevelBox(s, m, sx, sy, sz, { mat, obj, flag: FLAG.CREATURE }, chip, taper);
  const legY = fly ? 0 : C.legs * scale;
  b(T(0, legY + z * 0.2, 0), z * 0.36, z * 0.34, z * 0.78, C.body, 0.35);
  b(T(0, legY + z * 0.3 + C.neck * scale * 0.55, z * 0.36), z * 0.13, C.neck * scale + 0.05, z * 0.13, C.body, 0.2);
  b(T(0, legY + z * 0.36 + C.neck * scale, z * 0.44), z * 0.22, z * 0.2, z * 0.26, C.head || C.body, 0.3);
  b(T(0, legY + z * 0.34 + C.neck * scale, z * 0.64), z * 0.07, z * 0.06, z * 0.24, C.beak, 0.1);
  b(T(0, legY + z * 0.22, -z * 0.45), z * 0.24, z * 0.06, z * 0.26, C.body, 0.2);
  if (!fly) for (const sx of [-1, 1]) b(T(sx * 0.04 * scale, legY / 2, 0), 0.025, legY, 0.025, 'charcoal', 0.1);
  for (const sx of [-1, 1]) {
    if (fly) {
      // a wing in two parts: the arm lifted in a shallow V, the hand swept back to a point (bent down for gulls)
      const span = C.span * scale * 0.5, up = 0.28 + (wing - 0.5) * 0.9, bend = C.wingTip ? -0.6 : 0.5, c0 = z * 0.5;
      const a = { mat: C.body, obj, flag: FLAG.CREATURE };
      const S = [sx * z * 0.12, legY + z * 0.28, 0.02];
      const W = [S[0] + sx * span * 0.5 * Math.cos(up), S[1] + span * 0.5 * Math.sin(up), -c0 * 0.1];
      const Tp = [W[0] + sx * span * 0.5 * Math.cos(up * bend), W[1] + span * 0.5 * Math.sin(up * bend), -c0 * 0.6];
      s.quad([S[0], S[1], S[2] + c0 * 0.5], [W[0], W[1], W[2] + c0 * 0.36], [W[0], W[1], W[2] - c0 * 0.42], [S[0], S[1], S[2] - c0 * 0.5], a);
      s.tri([W[0], W[1], W[2] + c0 * 0.36], Tp, [W[0], W[1], W[2] - c0 * 0.42], C.wingTip ? { ...a, mat: C.wingTip } : a);
    } else b(T(sx * z * 0.18, legY + z * 0.22, -z * 0.04), 0.035, z * 0.26, z * 0.62, C.body, 0.2);
  }
  return { solid: s, obj };
}
