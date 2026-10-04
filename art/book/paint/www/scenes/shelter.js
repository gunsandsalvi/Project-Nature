// Age of first fire: a band under a rock shelter in a temperate valley. Golden hour (wide) or night (portrait).
// Daily work, the hunters coming home, the evening fire; soot on the rock, hand stencils and a painted deer.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K, card } from '../kit/k.js';
import { ground, boulder, blockCliff, stoneBlock } from '../kit/land.js';
import { broadleaf, conifer, bush, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal } from '../kit/animals.js';
import { hearth, smoke, tent, windbreak, rack, log } from '../kit/things.js';
import { land, put, riverSheet, decal } from '../kit/scene.js';

export async function build({ light, opts = [], view = {} }) {
  const marks = {};
  const wide = opts.includes('wide');
  const night = light === 'night';
  K.mpp = view.mpp ?? (wide ? 0.05 : 0.045);
  const P = new Painter({
    w: wide ? 748 : 336, h: wide ? 336 : 748, yaw: 16, elev: 30,
    target: wide ? [1, 2.6, -2.5] : [1.5, 4, -3.5], mood: light, bounds: { c: [0, 2, 0], r: 46 }, ...view, mpp: K.mpp,
  });
  P.haze = [P.camDepth + 10, P.camDepth + 60];
  const r = new Rand(7);

  // the land: a meadow sloping to a stream, a cliff of rock beds, a wooded plateau behind
  const beds = [1.7, 1.2, 1.9, 1.3, 1.6, 1.4], top = -0.2 + beds.reduce((a, b) => a + b, 0);
  const cliffZ = (x) => -6 + 0.8 * Math.sin(x * 0.13) + 0.02 * x;
  const streamZ = (x) => 10 + 2.2 * Math.sin(x / 7 + 0.5);
  const height = (x, z) => {
    const cz = cliffZ(x);
    if (z < cz - 6) return top - 0.12 + 0.3 * (fbm(x * 0.1, z * 0.1) - 0.5) * Math.min(1, (cz - 6 - z) / 4);
    if (z < cz - 4) return top - 0.4;
    let y = -0.05 * Math.max(0, z - cz - 2) + 0.4 * (fbm(x * 0.07 + 2, z * 0.07 + 5, 3) - 0.5);
    const d = Math.abs(z - streamZ(x));
    y -= 0.8 * Math.max(0, 1 - d / 2.6) ** 1.3;
    return y;
  };
  const L = land(height);
  const campAt = (x, z) => Math.hypot((x - 1) / 7.5, (z - cliffZ(x) - 0.6) / 3.3);
  P.addSolid(ground({
    x0: -38, x1: 38, z0: -36, z1: 46, step: 0.5, height, weights: (x, z) => {
      const d = Math.abs(z - streamZ(x));
      const camp = campAt(x, z) < 1 ? 1 : 0, bank = d < 1.7 ? 1 : 0;
      return [1 - Math.max(camp, bank), camp * (0.8 + noise2(x * 2, z * 2) * 0.4), bank, 0];
    },
  }), [['meadow', PAT.GROUND], ['dirt', PAT.DIRT], ['mud', PAT.DIRT], ['grass', PAT.GROUND]]);

  const path = [];
  for (let x = -40; x <= 40; x += 2) path.push([x, cliffZ(x)]);
  const sAt = (x) => x + 40; // near enough: the path runs along x
  const C = blockCliff({ seed: 21, path, base: -0.25, beds, mat: 'rock', grass: 'meadow', bedMats: ['rock', 'lime', 'rock', 'rock', 'lime', 'rock'],
    overhang: { bed: 3, from: sAt(-5.5), to: sAt(7.5), out: 2.4, recess: 1.6 } });
  P.addSolid(C.solid);
  P.addSoot([1.2, 4.6, cliffZ(1.2) + 0.4], 3.6);
  P.addSoot([-16, 3, cliffZ(-16)], -2.2);
  P.addSoot([19, 3.5, cliffZ(19)], -2.4);

  // paintings on the shelter's back wall: hand stencils and a red deer
  const wallAt = (x, bed) => {
    const row = C.beds[bed]; let best = row[0];
    for (const b of row) if (Math.abs(b.s - sAt(x)) < Math.abs(best.s - sAt(x))) best = b;
    return cliffZ(x) + best.out + 0.06;
  };
  const art = new Solid();
  const hand = ['.#.#.', '#####', '#####', '.###.', '.###.'];
  for (const [x, y] of [[-2.8, 2.1], [-2.4, 2.4], [3.6, 2.0], [4.0, 2.3]]) decal(art, hand, { origin: [x, y, wallAt(x, 1)], right: [1, 0, 0], up: [0, 1, 0], normal: [0, 0, 1], cell: 0.05, mat: 'dyedred' });
  const deer = ['........k.k.', '.........kk.', '.########k#.', '##########..', '.########...', '.#.#...#.#..', '.#.#...#.#..'];
  decal(art, deer, { origin: [-0.4, 3.0, wallAt(0, 2)], right: [1, 0, 0], up: [0, 1, 0], normal: [0, 0, 1], cell: 0.08, mat: 'dyedred' });
  P.addSolid(art);

  // fallen blocks and scree along the foot
  const stones = new Solid();
  for (let i = 0; i < 46; i++) {
    const x = r.range(-36, 36);
    if (x > -7 && x < 10) continue;
    const z = cliffZ(x) + r.range(0.6, 3.2), s = r.range(0.25, 0.9);
    if (r.chance(0.5)) stones.add(stoneBlock({ seed: 500 + i, size: [s * 1.4, s, s * 1.1], chip: 0.45, mat: r.pick(['rock', 'lime']) }), M4.mul(M4.T(x, L.at(x, z) + s * 0.3, z), M4.R(r.range(-0.3, 0.3), r.range(0, 3), r.range(-0.3, 0.3))));
    else stones.add(boulder({ seed: 500 + i, size: [s * 1.5, s * 1.1, s * 1.2], detail: s > 0.5 ? 1 : 0, mat: r.pick(['rock', 'lime']), moss: 0.4 }), M4.T(x, L.at(x, z) - 0.05, z));
  }
  for (const [x, z, s] of [[-15, 4, 1.4], [13, 2.5, 1.2], [-18, 16, 1.9], [20, 16, 1.3], [7, 20, 0.9], [-8, 25, 1.1]]) {
    stones.add(boulder({ seed: 600 + x, size: [s * 1.6, s * 1.2, s * 1.3], detail: 1, mat: 'rock', moss: 0.55 }), M4.T(x, L.at(x, z) - 0.1, z));
  }
  P.addSolid(stones);

  // the stream
  P.addWater(riverSheet({ centre: (t) => { const x = -42 + 84 * t; return [x, streamZ(x)]; }, width: (t) => 3.4 + 0.8 * Math.sin(t * 9), level: (x, z) => height(x, streamZ(x)) + 0.5 }));

  // trees on the plateau and at the sides; bushes on the slope
  const trees = [];
  for (let i = 0; i < 34; i++) {
    const x = r.range(-38, 38), z = cliffZ(x) - r.range(8, 30);
    trees.push([x, z, r.chance(0.3) ? 'conifer' : 'broad', r.range(7, 12)]);
  }
  for (const [x, z] of [[-26, 3], [-31, 10], [27, 5], [33, 13], [-22, 28], [24, 32], [-5, 37], [11, 40]]) trees.push([x, z, 'broad', r.range(7, 10)]);
  trees.forEach(([x, z, kind, h], i) => put(P, kind === 'conifer' ? conifer({ seed: 300 + i, h: h * 1.2 }) : broadleaf({ seed: 300 + i, h }), x, z, { ground: L.at }));
  for (let i = 0; i < 26; i++) {
    const x = r.range(-36, 36), z = r.range(-2, 42);
    if (Math.abs(z - streamZ(x)) < 2.4 || campAt(x, z) < 1.3) continue;
    put(P, bush({ seed: 400 + i, size: r.range(0.8, 1.6), berries: r.chance(0.3) ? 'berry' : null }), x, z, { ground: L.at });
  }
  // bushes on the ledges
  C.beds.forEach((row, bi) => row.forEach((b, k) => {
    if (bi === 0 || r.next() > 0.18) return;
    const A = C.at(b.s), x = A.p[0], z = A.p[1] + b.out - 0.5;
    put(P, bush({ seed: 700 + bi * 50 + k, size: r.range(0.5, 0.9), mat: r.chance(0.5) ? 'leaf' : 'moss' }), x, z, { y: b.top - 0.05 });
  }));

  // low plants: tufts and flowers in the meadow, reeds by the stream, grass hanging over the cliff top
  const low = new Cards();
  const meadowOk = (x, z) => Math.abs(z - streamZ(x)) > 2 && campAt(x, z) > 1.05 && z > cliffZ(x) + 1.2;
  scatter(low, { r, area: [-38, 38, -8, 46], count: 1000, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: 1, accept: meadowOk });
  scatter(low, { r, area: [-38, 38, -8, 46], count: 140, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: -1, accept: meadowOk });
  scatter(low, { r, area: [-38, 38, -8, 46], count: 280, tiles: TILE.FLOWER, mat: (x, z) => (noise2(x * 0.2, z * 0.2) > 0.5 ? 'flowerp' : 'flowery'), place: L.place, accept: meadowOk });
  scatter(low, { r, area: [-38, 38, -8, 46], count: 80, tiles: [TILE.HERB, TILE.FERN], mat: 'leaf', place: L.place, bias: 1, accept: meadowOk });
  scatter(low, { r, area: [-38, 38, 4, 18], count: 300, tiles: [TILE.REED, TILE.SEDGE], mat: 'reed', place: L.place, accept: (x, z) => { const d = Math.abs(z - streamZ(x)); return d > 1.2 && d < 2.6; } });
  scatter(low, { r, area: [-38, 38, -36, -6], count: 700, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: 1, accept: (x, z) => z < cliffZ(x) - 6.5 });
  // grass and moss on the ledges, some of it hanging over the edge
  C.beds.forEach((row, bi) => row.forEach((b) => {
    const n = Math.round(b.L * (bi === beds.length - 1 ? 1.6 : 0.9));
    for (let k = 0; k < n; k++) {
      const A = C.at(b.s + r.range(-b.L / 2, b.L / 2)), hang = r.chance(0.45);
      low.add({ c: [A.p[0], b.top + (hang ? 0.02 : -0.02), A.p[1] + b.out - (hang ? -0.04 : r.range(0.1, 0.5))], w: card() * r.range(0.7, 1), h: card() * r.range(0.5, 0.85),
        tile: r.pick(TILE.TUFT), mat: bi === beds.length - 1 || r.chance(0.5) ? 'meadow' : 'moss', bias: hang ? 0 : 1, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, n: [0, 0.8, 0.6], upright: 1, spin: hang ? Math.PI : 0 });
    }
  }));
  P.addCards(low);

  // the camp under the overhang
  const cz = (x) => cliffZ(x);
  const H = hearth({ seed: 5, level: night ? 4 : 3 });
  if (!night) { H.fire[3] *= 0.45; H.fire[4] *= 0.5; }
  put(P, H, 1, cz(1) + 0.9, { ground: L.at });
  put(P, windbreak({ seed: 3, len: 3.4 }), -5.2, cz(-5.2) + 1.6, { ground: L.at, yaw: 0.6 });
  put(P, rack({ seed: 4, len: 2.4, hang: 'meat' }), 6.4, cz(6.4) + 2.0, { ground: L.at, yaw: -0.3 });
  put(P, tent({ seed: 6, r: 1.7, h: 3.0 }), -10.5, 1.5, { ground: L.at, yaw: 0.5 });
  put(P, log({ len: 2.2, r: 0.17 }), 1.3, cz(1.3) + 2.6, { ground: L.at, yaw: 0.1 });
  const furs = new Solid();
  for (const [x, dz, a] of [[-2.6, -0.6, 0.2], [-0.9, -0.8, -0.1], [4.4, -0.5, 0.3]]) furs.box(M4.mul(M4.T(x, L.at(x, cz(x) + dz) + 0.05, cz(x) + dz), M4.R(0, a, 0)), 1.5, 0.1, 0.9, { mat: 'fur', pat: PAT.FUR, obj: newObj() });
  const hideZ = cz(-3.2) + 0.8;
  furs.box(M4.mul(M4.T(-3.2, L.at(-3.2, hideZ) + 0.03, hideZ), M4.R(0, 0.4, 0)), 1.2, 0.04, 0.9, { mat: 'hide', pat: PAT.HIDE, obj: newObj() });
  P.addSolid(furs);

  const folk = (spec, x, dz, yaw, extra = {}) => put(P, person(spec), x, cz(x) + dz, { ground: L.at, yaw, ...extra });
  folk({ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'leather', pose: 'sitTalk', seed: 1 }, 1.0, 2.6, Math.PI, { y: L.at(1, cz(1) + 2.6) + 0.34 });
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'hide', necklace: 'shell', pose: 'sitFloor', seed: 2 }, -0.6, 0.4, 1.2);
  folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'sitFloor', seed: 3 }, -0.4, 1.6, 1.6);
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'leather', pose: 'knap', held: 'stone', seed: 4 }, 2.8, 0.3, -1.9);
  folk({ kind: 'woman', skin: 'skin3', hairStyle: 'bun', top: 'tunic', topMat: 'dyedred', pose: 'scrape', seed: 5 }, -3.3, 0.2, 0.4);
  folk({ kind: 'youth', skin: 'skin1', hair: 'hairred', hairStyle: 'topknot', top: 'none', legs: 'leggings', paint: 'ochre', pose: 'reach', seed: 6 }, 5.7, 1.3, -0.4);
  // hunters coming home, one carrying meat
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'long', top: 'tunic', topMat: 'hide', pose: 'walkA', held: { kind: 'spear', tilt: 0.25 }, seed: 7 }, 9.6, 7.6, -2.5);
  folk({ kind: 'man', skin: 'skin3', hairStyle: 'short', top: 'wrap', topMat: 'fur', pose: 'carry', held: 'meat', seed: 8 }, 11.3, 8.9, -2.3);
  folk({ kind: 'youth', skin: 'skin2', hairStyle: 'braid', top: 'tunic', topMat: 'leather', pose: 'walkB', held: { kind: 'spear', tilt: 0.3 }, seed: 9 }, 13.0, 10.2, -2.2);
  // a dog asleep by the hearth
  put(P, animal({ species: 'dog', pose: 'lie', seed: 20 }), 2.6, cz(2.6) + 2.9, { ground: L.at, yaw: 2.2 });
  if (!night) {
    // children by the stream
    folk({ kind: 'child', skin: 'skin1', hairStyle: 'long', top: 'none', legs: 'bare', feet: null, pose: 'stoop', seed: 10 }, -3.5, 13.6, 0.3);
    folk({ kind: 'child', skin: 'skin3', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'point', seed: 11 }, -2.2, 12.7, -0.8);
  } else {
    // two walk up from the stream by torchlight, a third still at the fish trap
    const up = (x, z) => ({ y: L.at(x, z) });
    put(P, person({ kind: 'man', skin: 'skin3', hairStyle: 'long', top: 'wrap', topMat: 'leather', pose: 'walkA', held: 'torch', seed: 16 }), 6.2, 17.4, { ...up(6.2, 17.4), yaw: -2.75 });
    put(P, person({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'hide', pose: 'carry', held: { kind: 'basket', mat: 'reed' }, seed: 17 }), 7.4, 18.5, { ...up(7.4, 18.5), yaw: -2.8 });
    put(P, person({ kind: 'youth', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'leggings', pose: 'hold', held: 'torch', seed: 18 }), 3.5, streamZ(3.5) + 2.1, { ...up(3.5, streamZ(3.5) + 2.1), yaw: 0.3 });
  }

  if (night) {
    // the evening: a drummer, sleepers on the furs, a mother with her baby
    folk({ kind: 'man', skin: 'skin3', hairStyle: 'long', top: 'wrap', topMat: 'leather', pose: 'drum', held: 'drum', seed: 12 }, 2.4, 1.9, -2.6);
    folk({ kind: 'woman', skin: 'skin2', hairStyle: 'long', top: 'dress', topMat: 'fur', pose: 'lie', seed: 13 }, -2.6, -0.6, 1.4, { y: L.at(-2.6, cz(-2.6) - 0.6) + 0.1 });
    folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'lie', seed: 14 }, -0.9, -0.8, 1.6, { y: L.at(-0.9, cz(-0.9) - 0.8) + 0.1 });
    folk({ kind: 'woman', skin: 'skin1', hairStyle: 'bun', top: 'dress', topMat: 'leather', pose: 'sitFloor', held: { kind: 'baby', skin: 'skin1' }, seed: 15 }, 3.6, 2.4, -2.9);
    // fireflies over the meadow
    const flies = new Cards();
    for (let i = 0; i < 70; i++) {
      const x = r.range(-14, 16), z = r.range(4, 26);
      if (Math.abs(z - streamZ(x)) < 1.6) continue;
      flies.add({ c: [x, L.at(x, z) + r.range(0.4, 1.6), z], w: K.mpp * 1.0, h: K.mpp * 1.0, tile: TILE.DOT, mat: 'flowery', bias: 5, flag: FLAG.EMISSIVE | FLAG.NOOUTLINE | FLAG.NOSHADOW });
    }
    P.addCards(flies);
  }

  const puffs = new Puffs();
  smoke(puffs, { at: [1, 0.9, cz(1) + 0.9], height: 10, drift: [0.55, 0.3], seed: 3, size: 0.3, tone: night ? 1 : 5, alpha: night ? 0.2 : 0.42 });
  P.addPuffs(puffs);
  if (night) P.addFire([1, 0.5, cz(1) + 0.9], 1.25, 14);
  return { P, marks };
}

export default async function (args) {
  const { P } = await build(args);
  return P.render(4);
}
