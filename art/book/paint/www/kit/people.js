// People (PRE-27): small figures of tiny blocks with a separate head, torso, arms and legs, about five heads
// tall (children about four), posed by joint angles, dressed by climate and age, holding what they carry.
import * as THREE from 'three';
import { Solid, PAT, FLAG, Rand, newObj, M4 } from '../geo.js';
import { boulder } from './land.js';

const { T, R } = M4;
const mul = (...m) => m.reduce((a, b) => a.clone().multiply(b));
const D = Math.PI / 180;

export const BUILDS = {
  man:   { h: 1.68, sh: 1.0, hip: 1.0, head: 1.0 },
  woman: { h: 1.58, sh: 0.9, hip: 1.06, head: 0.98 },
  elder: { h: 1.6, sh: 0.95, hip: 1.0, head: 1.0, stoop: 14 },
  youth: { h: 1.45, sh: 0.86, hip: 0.92, head: 1.02 },
  child: { h: 1.08, sh: 0.8, hip: 0.85, head: 1.12, child: true },
};

// Joint angles in degrees. arm: [forward swing, out to the side, elbow bend]; leg: [forward swing, out, knee bend].
// spine: forward lean; head: nod; drop: how far the hips sink (for crouching), in metres per metre of height.
export const POSES = {
  stand:   { spine: 0, head: 0, armL: [4, 6, 8], armR: [-4, 6, 8], legL: [0, 2, 0], legR: [0, 2, 0] },
  walkA:   { spine: 4, head: 0, armL: [-24, 5, 14], armR: [26, 5, 22], legL: [24, 0, 8], legR: [-18, 0, 22] },
  walkB:   { spine: 4, head: 0, armL: [26, 5, 22], armR: [-24, 5, 14], legL: [-18, 0, 22], legR: [24, 0, 8] },
  carry:   { spine: 2, head: -4, armL: [38, 10, 70], armR: [38, 10, 70], legL: [12, 0, 6], legR: [-10, 0, 16] },
  shoulder:{ spine: 6, head: 0, armL: [-10, 6, 12], armR: [160, 8, 140], legL: [18, 0, 6], legR: [-14, 0, 18] },
  reach:   { spine: -4, head: -18, armL: [165, 12, 4], armR: [150, 8, 10], legL: [0, 3, 0], legR: [0, 3, 0] },
  point:   { spine: 0, head: 0, armL: [4, 6, 8], armR: [86, -6, 0], legL: [6, 2, 0], legR: [-4, 2, 0] },
  armsUp:  { spine: -6, head: -16, armL: [170, 26, 10], armR: [170, 26, 10], legL: [0, 6, 0], legR: [0, 6, 0] },
  danceA:  { spine: 10, head: 8, armL: [120, 40, 50], armR: [-30, 30, 40], legL: [40, 4, 70], legR: [-6, 2, 8], drop: 0.04 },
  danceB:  { spine: -6, head: -10, armL: [-30, 30, 40], armR: [120, 40, 50], legL: [-6, 2, 8], legR: [40, 4, 70], drop: 0.04 },
  throw:   { spine: -8, head: -6, armL: [70, 10, 10], armR: [150, 20, 70], legL: [30, 0, 10], legR: [-30, 0, 14] },
  hold:    { spine: 0, head: 0, armL: [8, 8, 10], armR: [28, 10, 64], legL: [4, 3, 0], legR: [-2, 3, 0] },
  aim:     { spine: 12, head: 4, armL: [62, 4, 30], armR: [100, 10, 110], legL: [36, 0, 30], legR: [-24, 0, 26], drop: 0.06 },
  stoop:   { spine: 62, head: -30, armL: [70, 8, 10], armR: [80, 6, 20], legL: [8, 6, 18], legR: [-4, 6, 22], drop: 0.04 },
  crouch:  { spine: 34, head: -22, armL: [52, 12, 60], armR: [62, 10, 70], legL: [104, 14, 140], legR: [96, 10, 140], drop: 0.38 },
  knap:    { spine: 30, head: -26, armL: [58, 14, 92], armR: [74, 4, 84], legL: [100, 26, 136], legR: [96, 22, 140], drop: 0.38 },
  kneel:   { spine: 18, head: -20, armL: [60, 8, 40], armR: [66, 8, 46], legL: [90, 6, 90], legR: [0, 6, 92], drop: 0.27 },
  scrape:  { spine: 48, head: -30, armL: [76, 12, 30], armR: [84, 10, 34], legL: [92, 8, 120], legR: [30, 8, 150], drop: 0.36 },
  sit:     { spine: -6, head: 0, armL: [26, 10, 44], armR: [30, 10, 50], legL: [88, 8, 84], legR: [84, 8, 92], seat: true },
  sitTalk: { spine: 4, head: -4, armL: [26, 10, 44], armR: [74, -4, 64], legL: [88, 8, 84], legR: [84, 8, 92], seat: true },
  sitFloor:{ spine: 8, head: -8, armL: [34, 14, 56], armR: [36, 14, 60], legL: [74, 32, 120], legR: [70, 32, 124], drop: 0.5 },
  lie:     { spine: 0, head: 0, armL: [6, 6, 10], armR: [-2, 6, 10], legL: [4, 3, 6], legR: [-2, 3, 8], lie: true },
  drum:    { spine: 10, head: -10, armL: [56, 18, 70], armR: [50, 18, 96], legL: [74, 32, 120], legR: [70, 32, 124], drop: 0.5 },
  dig:     { spine: 54, head: -26, armL: [92, 6, 40], armR: [70, 10, 20], legL: [70, 10, 90], legR: [10, 10, 40], drop: 0.12 },
  blow:    { spine: 38, head: -34, armL: [70, 16, 100], armR: [70, 16, 100], legL: [100, 26, 136], legR: [96, 22, 140], drop: 0.38 },
};

/**
 * A person. kind: a BUILDS key. skin: skin1-3. hair: material; hairStyle: short, long, braid, bun, bald, hood.
 * top: none, wrap, tunic, dress, parka. topMat, legMat; legs: bare, leggings. cloak: material or null.
 * extras: beard, necklace (material), paint (material), belt (material), headband (material).
 */
export function person({ kind = 'man', skin = 'skin1', hair = 'hair', hairStyle = 'short', top = 'tunic', topMat = 'hide',
  legs = 'leggings', legMat = 'leather', feet = 'leather', cloak = null, beard = false, necklace = null, paint = null,
  belt = 'leather', headband = null, pose = 'stand', held = null, heldL = null, seed = 1, sleeve = 'half' }) {
  const B = BUILDS[kind], s = B.h / 1.65, P = typeof pose === 'string' ? POSES[pose] : pose;
  const r = new Rand(seed);
  const id = newObj(); const arms = [newObj(), newObj()], legsId = [newObj(), newObj()], heldId = newObj(), headId = newObj();
  newObj();
  const out = new Solid();
  const box = (m, sx, sy, sz, mat, o = id, pat = 0, extra = {}) => out.box(m, sx, sy, sz, { mat, obj: o, pat, flag: FLAG.CREATURE, ...extra });

  // sizes (metres)
  const headH = (B.child ? 0.27 : 0.3) * s * B.head * (B.child ? 1.05 : 1), headW = 0.235 * s * B.head, headD = 0.255 * s * B.head;
  const neckH = 0.04 * s, chestH = 0.3 * s, hipsH = 0.17 * s;
  const chestW = 0.38 * s * B.sh, hipsW = 0.32 * s * B.hip, chestD = 0.21 * s, hipsD = 0.2 * s;
  const upA = 0.29 * s, foreA = 0.26 * s, armW = 0.095 * s, handL = 0.08 * s;
  const thigh = 0.38 * s, shin = 0.36 * s, legW = 0.13 * s, footH = 0.055 * s, footL = 0.21 * s;
  const hipY = thigh + shin + footH;
  const drop = (P.drop || 0) * B.h;
  const stoop = (B.stoop || 0);

  const pelvis = mul(T(0, hipY - drop, 0), R((P.pelvis || 0) * D, 0, 0));
  const spine = mul(pelvis, T(0, hipsH * 0.5, 0), R((P.spine + stoop) * D, (P.twist || 0) * D, 0));
  const neck = mul(spine, T(0, chestH, 0));
  const head = mul(neck, T(0, neckH, 0), R((P.head + (stoop ? -stoop * 0.6 : 0)) * D, (P.turn || 0) * D, 0));
  const topMatC = top === 'none' ? skin : topMat;

  // hips and chest
  box(mul(pelvis, T(0, 0, 0)), hipsW, hipsH, hipsD, top === 'none' && legs === 'bare' ? 'leather' : (top === 'none' ? legMat : topMatC), id, PAT.HIDE);
  box(mul(spine, T(0, chestH / 2, 0)), chestW, chestH, chestD, topMatC, id, top === 'none' ? 0 : (top === 'parka' ? PAT.FUR : PAT.HIDE));
  if (top === 'wrap') box(mul(spine, T(-chestW * 0.18, chestH * 0.55, chestD * 0.08), R(0, 0, 0.5)), chestW * 0.5, chestH * 1.15, chestD * 1.12, topMat, id, PAT.HIDE);
  if (top === 'tunic' || top === 'dress' || top === 'parka') {
    const len = top === 'dress' ? 0.52 * s : (top === 'parka' ? 0.36 * s : 0.3 * s);
    box(mul(pelvis, T(0, -len / 2 + hipsH * 0.3, 0)), hipsW * 1.12, len, hipsD * 1.14, topMat, id, top === 'parka' ? PAT.FUR : PAT.HIDE);
    if (top === 'parka') box(mul(pelvis, T(0, -len + hipsH * 0.3, 0)), hipsW * 1.16, 0.05 * s, hipsD * 1.18, 'furdark', id, PAT.FUR);
  }
  if (top === 'none' && legs === 'bare') box(mul(pelvis, T(0, -0.08 * s, hipsD * 0.25)), hipsW * 0.7, 0.16 * s, hipsD * 0.6, legMat, id, PAT.HIDE);
  if (belt && top !== 'parka') box(mul(pelvis, T(0, hipsH * 0.42, 0)), hipsW * 1.16, 0.035 * s, hipsD * 1.16, belt, id);
  if (paint) {
    box(mul(spine, T(0, chestH * 0.62, chestD / 2 + 0.004)), chestW * 0.86, 0.035 * s, 0.01, paint, id);
    box(mul(spine, T(0, chestH * 0.32, chestD / 2 + 0.004)), chestW * 0.7, 0.035 * s, 0.01, paint, id);
  }
  if (necklace) box(mul(spine, T(0, chestH * 0.86, chestD / 2 + 0.006)), chestW * 0.55, 0.04 * s, 0.02, necklace, id);
  if (cloak) box(mul(spine, T(0, chestH * 0.35, -chestD / 2 - 0.025), R(-0.08, 0, 0)), chestW * 1.12, chestH * 1.9, 0.05, cloak, id, PAT.FUR);

  // neck and head
  box(mul(neck, T(0, neckH / 2, 0)), headW * 0.5, neckH * 1.6, headD * 0.5, top === 'parka' ? topMat : skin, id);
  box(mul(head, T(0, headH / 2, 0)), headW, headH, headD, skin, headId, PAT.FACE, { norm: true });
  const hc = mul(head, T(0, headH / 2, 0));
  const hairBox = (dx, dy, dz, sx, sy, sz, m = hair) => box(mul(hc, T(dx * headW, dy * headH, dz * headD)), sx * headW, sy * headH, sz * headD, m, headId);
  if (hairStyle === 'hood') {
    hairBox(0, 0.12, -0.06, 1.24, 1.12, 1.12, topMat);
    hairBox(0, -0.42, 0, 1.3, 0.3, 1.2, 'furdark');
  } else if (hairStyle !== 'bald') {
    hairBox(0, 0.42, -0.02, 1.08, 0.24, 1.08);          // the crown
    hairBox(0, 0.05, -0.42, 1.08, 0.78, 0.22);          // the back
    hairBox(0.5, 0.18, -0.1, 0.12, 0.5, 0.8); hairBox(-0.5, 0.18, -0.1, 0.12, 0.5, 0.8); // the sides
    if (hairStyle === 'long') hairBox(0, -0.55, -0.42, 1.0, 1.1, 0.22);
    if (hairStyle === 'braid') { hairBox(0.32, -0.6, -0.4, 0.22, 1.2, 0.2); hairBox(-0.32, -0.6, -0.4, 0.22, 1.2, 0.2); }
    if (hairStyle === 'bun') hairBox(0, 0.42, -0.58, 0.5, 0.42, 0.42);
    if (hairStyle === 'topknot') hairBox(0, 0.66, -0.1, 0.4, 0.34, 0.4);
  }
  if (headband) hairBox(0, 0.28, 0, 1.12, 0.1, 1.12, headband);
  if (beard) hairBox(0, -0.36, 0.42, 0.86, 0.36, 0.24);

  // arms (each its own object, so an arm in front of the body is outlined)
  const armsOut = [];
  [[1, P.armL], [-1, P.armR]].forEach(([side, a], k) => {
    const sh = mul(spine, T(side * (chestW / 2 + armW * 0.42), chestH - armW * 0.55, 0), R(-a[0] * D, 0, side * a[1] * D));
    const sleeveM = top === 'none' || sleeve === 'none' ? skin : topMatC;
    box(mul(sh, T(0, -upA / 2, 0)), armW * 1.08, upA, armW * 1.08, sleeveM, arms[k], top === 'parka' ? PAT.FUR : 0);
    const el = mul(sh, T(0, -upA, 0), R(-a[2] * D, 0, 0));
    const foreM = top === 'parka' || sleeve === 'full' ? topMatC : skin;
    box(mul(el, T(0, -foreA / 2, 0)), armW, foreA, armW, foreM, arms[k], top === 'parka' ? PAT.FUR : 0);
    const hand = mul(el, T(0, -foreA - handL / 2, 0));
    box(hand, armW * 0.9, handL, armW * 0.75, top === 'parka' ? 'furdark' : skin, arms[k]);
    armsOut.push(mul(el, T(0, -foreA - handL * 0.6, 0)));
  });

  // legs
  [[1, P.legL], [-1, P.legR]].forEach(([side, l], k) => {
    const hp = mul(pelvis, T(side * hipsW * 0.27, -hipsH * 0.2, 0), R(-l[0] * D, 0, side * l[1] * D));
    const legM = legs === 'bare' ? skin : legMat;
    box(mul(hp, T(0, -thigh / 2, 0)), legW, thigh, legW, legM, legsId[k], top === 'parka' ? PAT.FUR : 0);
    const kn = mul(hp, T(0, -thigh, 0), R(l[2] * D, 0, 0));
    box(mul(kn, T(0, -shin / 2, 0)), legW * 0.92, shin, legW * 0.92, legM, legsId[k], top === 'parka' ? PAT.FUR : 0);
    const an = mul(kn, T(0, -shin, 0), R(-l[2] * D * 0.5 + l[0] * D * 0.4, 0, 0));
    box(mul(an, T(0, -footH / 2, footL * 0.28)), legW * 0.86, footH, footL, feet || skin, legsId[k]);
  });

  // what the hands hold
  const hands = { R: armsOut[1], L: armsOut[0] };
  for (const [which, item] of [['R', held], ['L', heldL]]) {
    if (!item) continue;
    const m = hands[which];
    const it = typeof item === 'string' ? { kind: item } : item;
    holdItem(out, m, it, heldId, s, r);
  }

  // stand the figure on the ground, sit it on its seat (y = 0 is the seat), or lay it down
  let body = out;
  if (P.lie) { body = new Solid(); body.add(out, R(-Math.PI / 2, 0, 0)); }
  let minY = Infinity;
  for (let i = 1; i < body.P.length; i += 3) minY = Math.min(minY, body.P[i]);
  const res = new Solid();
  res.add(body, T(0, P.seat ? -(hipY - drop - hipsH * 0.5) : -minY, 0));
  return { solid: res, obj: id, height: B.h };
}

function holdItem(out, hand, it, obj, s, r) {
  const at = (m, ...rest) => out.box(m, ...rest);
  const k = it.kind;
  if (k === 'spear') {
    const L = (it.length || 2.1) * s;
    const m = M4.mul(hand, M4.R(it.tilt ?? 0, 0, 0), M4.T(0, L * 0.18, 0));
    out.beam(M4.mul(m, M4.T(0, -L * 0.6, 0)).elements.slice(12, 15), M4.mul(m, M4.T(0, L * 0.4, 0)).elements.slice(12, 15), 0.018, 0.016, 4, { mat: 'wood', obj });
    const tip = M4.mul(m, M4.T(0, L * 0.4 + 0.05, 0));
    at(tip, 0.045, 0.12, 0.02, { mat: 'flint', obj });
  } else if (k === 'staff') {
    const L = (it.length || 1.7) * s;
    const m = M4.mul(hand, M4.T(0, -0.05, 0));
    out.beam(M4.mul(m, M4.T(0, -L * 0.62, 0)).elements.slice(12, 15), M4.mul(m, M4.T(0, L * 0.38, 0)).elements.slice(12, 15), 0.022, 0.02, 4, { mat: 'wood', obj });
  } else if (k === 'bundle' || k === 'meat') {
    at(M4.mul(hand, M4.T(0, 0.02, 0.12)), 0.3 * s, 0.2 * s, 0.25 * s, { mat: it.mat || (k === 'meat' ? 'berry' : 'hide'), obj, pat: PAT.HIDE });
  } else if (k === 'basket') {
    at(M4.mul(hand, M4.T(0, -0.1, 0.06)), 0.3 * s, 0.22 * s, 0.3 * s, { mat: it.mat || 'reed', obj, pat: PAT.THATCH });
  } else if (k === 'stone') {
    out.add(boulder({ seed: 3, size: [0.11, 0.09, 0.1], detail: 0, mat: 'flint', moss: 0, obj }), M4.mul(hand, M4.T(0, -0.02, 0.04)));
  } else if (k === 'torch') {
    out.beam(M4.mul(hand, M4.T(0, -0.25, 0)).elements.slice(12, 15), M4.mul(hand, M4.T(0, 0.35, 0.05)).elements.slice(12, 15), 0.025, 0.03, 4, { mat: 'wood', obj });
  } else if (k === 'pot') {
    at(M4.mul(hand, M4.T(0, 0.0, 0.14)), 0.26 * s, 0.26 * s, 0.26 * s, { mat: 'clay', obj });
  } else if (k === 'fish') {
    at(M4.mul(hand, M4.T(0, -0.16, 0.02)), 0.05, 0.34, 0.09, { mat: 'flint', obj });
  } else if (k === 'baby') {
    at(M4.mul(hand, M4.T(-0.08, 0.04, 0.12)), 0.32 * s, 0.18 * s, 0.18 * s, { mat: it.mat || 'fur', obj, pat: PAT.FUR });
    at(M4.mul(hand, M4.T(0.1, 0.1, 0.14)), 0.11, 0.11, 0.11, { mat: it.skin || 'skin1', obj });
  } else if (k === 'hide') {
    at(M4.mul(hand, M4.T(0, -0.2, 0.15)), 0.6 * s, 0.5 * s, 0.03, { mat: 'hide', obj, pat: PAT.HIDE });
  } else if (k === 'drum') {
    out.beam(M4.mul(hand, M4.T(0, -0.05, 0.18)).elements.slice(12, 15), M4.mul(hand, M4.T(0, 0.03, 0.18)).elements.slice(12, 15), 0.2, 0.2, 9, { mat: 'hide', obj });
  }
}
