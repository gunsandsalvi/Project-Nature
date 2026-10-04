// Plants (PRE-46: about 8 forms): broad-leaved tree, needle tree, bush, grass, herb or flower, reed, root plant,
// fungus. Trunks are solids; leaves are clumps on cards, lit as a round crown (soft, painted light).
import * as THREE from 'three';
import { Solid, Cards, PAT, FLAG, Rand, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from './k.js';

const card = () => 32 * K.mpp; // a card shows its tile at one texel per art pixel

/** Points spread over a ball, more on the outside. */
function ballPoints(r, n, rx, ry, rz, shell = 0.55) {
  const pts = [];
  for (let i = 0; i < n; i++) {
    const u = r.range(-1, 1), a = r.range(0, Math.PI * 2), s = Math.sqrt(1 - u * u);
    const d = shell + (1 - shell) * Math.cbrt(r.next());
    pts.push([Math.cos(a) * s * rx * d, u * ry * d, Math.sin(a) * s * rz * d]);
  }
  return pts;
}

/** Leaf clumps round a set of crown balls; each clump is lit as part of its ball (soft round light). */
export function crown(cards, r, balls, { mat = 'leaf', obj, density = 1, tiles = TILE.LEAF, size = 1, bias = 0, flag = FLAG.FOLIAGE }) {
  for (const b of balls) {
    const n = Math.round(density * 26 * b.r * b.r / (card() * card()) * 0.9) + 6;
    for (const p of ballPoints(r, n, b.r, b.r * (b.sy || 0.85), b.r)) {
      const c = [b.c[0] + p[0], b.c[1] + p[1], b.c[2] + p[2]];
      const len = Math.hypot(...p) || 1;
      const nb = [p[0] / len, p[1] / len * 1.1 + 0.15, p[2] / len];
      const s = card() * size * r.range(0.85, 1.15);
      cards.add({ c, w: s, h: s, tile: r.pick(tiles), mat, obj, flag, n: nb, spin: r.range(-0.4, 0.4), bias });
    }
  }
}

/** A broad-leaved tree: trunk, a few limbs, and a crown of several balls. season: summer, spring, autumn, bare. */
export function broadleaf({ seed = 1, h = 9, spread = 1, season = 'summer', snow = false, leafMat }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  const trunkH = h * r.range(0.38, 0.48), lean = [r.range(-0.3, 0.3), r.range(-0.3, 0.3)];
  const top = [lean[0], trunkH, lean[1]];
  const rb = 0.16 + h * 0.022;
  s.beam([0, -0.3, 0], [lean[0] * 0.5, trunkH * 0.5, lean[1] * 0.5], rb * 1.25, rb, 7, { mat: 'bark', pat: PAT.BARK, obj }, { capB: false });
  s.beam([lean[0] * 0.5, trunkH * 0.5, lean[1] * 0.5], top, rb, rb * 0.8, 7, { mat: 'bark', pat: PAT.BARK, obj });
  // roots flare
  for (let i = 0; i < 4; i++) {
    const a = (i / 4) * Math.PI * 2 + r.range(0, 1);
    s.beam([0, 0.35, 0], [Math.cos(a) * rb * 3.2, -0.1, Math.sin(a) * rb * 3.2], rb * 0.6, rb * 0.25, 5, { mat: 'bark', pat: PAT.BARK, obj });
  }
  const crownC = [top[0], trunkH + (h - trunkH) * 0.45, top[1]];
  const R = (h - trunkH) * 0.55 * spread;
  const balls = [{ c: crownC, r: R * 0.75 }];
  const nb = r.int(4, 6);
  for (let i = 0; i < nb; i++) {
    const a = (i / nb) * Math.PI * 2 + r.range(-0.4, 0.4);
    const out = R * r.range(0.45, 0.75), up = R * r.range(-0.35, 0.45);
    const c = [crownC[0] + Math.cos(a) * out, crownC[1] + up, crownC[2] + Math.sin(a) * out];
    balls.push({ c, r: R * r.range(0.42, 0.6) });
    s.beam(top, [c[0] * 0.8 + top[0] * 0.2, c[1] - R * 0.15, c[2] * 0.8 + top[2] * 0.2], rb * 0.55, rb * 0.25, 5, { mat: 'bark', pat: PAT.BARK, obj });
  }
  if (season === 'bare') {
    for (const b of balls) for (let k = 0; k < 3; k++) {
      const t = [b.c[0] + r.range(-b.r, b.r), b.c[1] + r.range(0, b.r), b.c[2] + r.range(-b.r, b.r)];
      s.beam([b.c[0] * 0.7 + top[0] * 0.3, b.c[1] - b.r * 0.4, b.c[2] * 0.7 + top[2] * 0.3], t, rb * 0.22, 0.03, 4, { mat: 'bark', pat: PAT.BARK, obj });
    }
    if (snow) for (const b of balls) crown(cards, r, [{ c: [b.c[0], b.c[1] + b.r * 0.35, b.c[2]], r: b.r * 0.55, sy: 0.35 }], { mat: 'snow', obj, density: 0.25, tiles: TILE.LEAFSMALL });
  } else {
    const mat = leafMat || { summer: 'leaf', spring: 'leafsp', autumn: 'autumn' }[season] || 'leaf';
    crown(cards, r, balls, { mat, obj });
    if (season === 'spring') crown(cards, r, balls.slice(1), { mat: 'flowerw', obj, density: 0.08, tiles: TILE.FLOWER, bias: 1 });
    if (snow) crown(cards, r, balls.map((b) => ({ c: [b.c[0], b.c[1] + b.r * 0.4, b.c[2]], r: b.r * 0.6, sy: 0.4 })), { mat: 'snow', obj, density: 0.4 });
  }
  return { solid: s, cards, obj, height: h };
}

/** A dry-land tree with a wide, flat crown on a leaning, forked trunk (acacia-like); olive: a rounder grey crown. */
export function flatTree({ seed = 1, h = 6, olive = false }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  const rb = 0.14 + h * 0.015;
  const fork = [r.range(-0.4, 0.4), h * 0.4, r.range(-0.4, 0.4)];
  s.beam([0, -0.3, 0], fork, rb * 1.2, rb, 6, { mat: 'bark', pat: PAT.BARK, obj });
  const balls = [];
  const n = olive ? 3 : 4;
  for (let i = 0; i < n; i++) {
    const a = (i / n) * Math.PI * 2 + r.range(-0.4, 0.4), out = h * r.range(0.22, 0.38);
    const c = [fork[0] + Math.cos(a) * out, h * r.range(0.78, 0.92), fork[2] + Math.sin(a) * out];
    s.beam(fork, [c[0] * 0.9, c[1] - 0.3, c[2] * 0.9], rb * 0.6, rb * 0.3, 5, { mat: 'bark', pat: PAT.BARK, obj });
    balls.push({ c, r: h * (olive ? 0.3 : 0.26), sy: olive ? 0.75 : 0.32 });
  }
  if (!olive) balls.push({ c: [fork[0], h * 0.95, fork[2]], r: h * 0.3, sy: 0.3 });
  crown(cards, r, balls, { mat: olive ? 'pine' : 'leaf', obj, tiles: TILE.LEAFSMALL.concat(TILE.LEAF), density: 1.2, size: 0.8 });
  return { solid: s, cards, obj, height: h };
}

/** A needle tree: a straight trunk and tiers of drooping needles, narrowing to a tip. */
export function conifer({ seed = 1, h = 12, width = 0.32, snow = false, mat = 'pine' }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  const rb = 0.12 + h * 0.012;
  s.beam([0, -0.3, 0], [0, h * 0.95, 0], rb, 0.04, 6, { mat: 'bark', pat: PAT.BARK, obj });
  const tiers = Math.round(h / 0.9);
  for (let i = 0; i < tiers; i++) {
    const t = i / (tiers - 1);
    const y = h * (0.18 + 0.8 * t);
    const R = h * width * (1 - t) * r.range(0.85, 1.1) + 0.25;
    const n = Math.max(3, Math.round(R * 7));
    for (let k = 0; k < n; k++) {
      const a = (k / n) * Math.PI * 2 + r.range(-0.3, 0.3) + i;
      const d = R * r.range(0.45, 0.85);
      const c = [Math.cos(a) * d, y + r.range(-0.2, 0.2), Math.sin(a) * d];
      const sz = card() * r.range(0.8, 1.05) * Math.min(1.1, 0.55 + R * 0.35);
      cards.add({ c, w: sz, h: sz, tile: r.pick(TILE.CONIFER), mat, obj, flag: FLAG.FOLIAGE, n: [Math.cos(a) * 0.8, 0.55, Math.sin(a) * 0.8] });
      if (snow && r.chance(0.6)) cards.add({ c: [c[0], c[1] + sz * 0.18, c[2]], w: sz * 0.8, h: sz * 0.5, tile: r.pick(TILE.LEAFSMALL), mat: 'snow', obj, flag: FLAG.FOLIAGE, n: [0, 1, 0] });
    }
  }
  return { solid: s, cards, obj, height: h };
}

/** A bush: a low dome of leaf clumps on a few stems; berries optional. */
export function bush({ seed = 1, size = 1.2, mat = 'leaf', berries = null, flowers = null }) {
  const r = new Rand(seed), obj = newObj(), s = new Solid(), cards = new Cards();
  const balls = [];
  const n = r.int(2, 4);
  for (let i = 0; i < n; i++) balls.push({ c: [r.range(-0.35, 0.35) * size, size * r.range(0.4, 0.6), r.range(-0.35, 0.35) * size], r: size * r.range(0.4, 0.55), sy: 0.8 });
  for (let i = 0; i < 4; i++) s.beam([0, 0, 0], [r.range(-0.4, 0.4) * size, size * 0.6, r.range(-0.4, 0.4) * size], 0.04, 0.02, 4, { mat: 'bark', obj });
  crown(cards, r, balls, { mat, obj, tiles: TILE.LEAFSMALL.concat(TILE.LEAF), density: 1.3, size: 0.85 });
  if (berries) crown(cards, r, balls, { mat: berries, obj, density: 0.1, tiles: TILE.FLOWER, bias: 0 });
  if (flowers) crown(cards, r, balls, { mat: flowers, obj, density: 0.12, tiles: TILE.FLOWER, bias: 1 });
  return { solid: s, cards, obj };
}

/** Low plants scattered on the ground: tufts, flowers, herbs, ferns, reeds, grain. place(x, z) gives [y, normal]. */
export function scatter(cards, { r, area, count, tiles, mat, place, size = 1, bias = 0, flag = FLAG.NOOUTLINE | FLAG.NOSHADOW, accept, obj = 0 }) {
  let made = 0, tries = 0;
  while (made < count && tries < count * 20) {
    tries++;
    const x = r.range(area[0], area[1]), z = r.range(area[2], area[3]);
    if (accept && !accept(x, z)) continue;
    const [y, n] = place(x, z);
    const sz = card() * size * r.range(0.85, 1.15);
    cards.add({ c: [x, y - 0.03, z], w: sz, h: sz, tile: r.pick(tiles), mat: typeof mat === 'function' ? mat(x, z) : mat, bias, obj, flag, n, upright: 1 });
    made++;
  }
}
