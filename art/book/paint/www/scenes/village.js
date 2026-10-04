// Age of villages, herds and fields: by the river, a band now lives all year in reed-roofed houses, keeps goats and
// dogs, and sows grain on its old rubbish heaps. Morning: mist on the river, smoke from the roofs, work in the plots.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder } from '../kit/land.js';
import { broadleaf, conifer, bush, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { longhouse, fence, smoke, hearth, pot, rack, log } from '../kit/things.js';
import { land, put, riverSheet, lineField } from '../kit/scene.js';

export async function build({ light, opts = [], view = {} }) {
  const marks = {};
  K.mpp = view.mpp ?? 0.05;
  const P = new Painter({ w: 336, h: 748, yaw: 14, elev: 30, target: [-1.5, 1, 3], mood: light, bounds: { c: [0, 0, -4], r: 50 }, ...view, mpp: K.mpp });
  P.haze = [P.camDepth + 4, P.camDepth + 46];
  const r = new Rand(71);
  const water = -1.4;

  // the river behind the village, the terrace it sits on, the plots in front
  const centre = (t) => [-40 + 80 * t, -17 + 3 * Math.sin(t * 6 + 1)];
  const near = lineField(centre);
  const memo = new Map();
  const field = (x, z) => { const k = x.toFixed(2) + ',' + z.toFixed(2); let v = memo.get(k); if (!v) { v = near(x, z); memo.set(k, v); } return v; };
  const plots = [[-6.5, 9, 4.2, 3.2, 0.1], [-1.5, 12.6, 4.0, 2.8, 0.05], [4.5, 10.5, 4.6, 3.0, -0.05], [9.8, 13.4, 3.8, 3.2, 0.1], [-2.5, 17.5, 5.2, 3.0, 0.0], [5.2, 17.0, 4.0, 2.6, 0.08]];
  const inPlot = (x, z) => plots.findIndex(([px, pz, w, d, a]) => {
    const c = Math.cos(a), s = Math.sin(a), lx = (x - px) * c + (z - pz) * s, lz = -(x - px) * s + (z - pz) * c;
    return Math.abs(lx) < w / 2 && Math.abs(lz) < d / 2;
  });
  const height = (x, z) => {
    const f = field(x, z);
    const bank = Math.min(1, Math.max(0, (f.d - 3.2) / 3));
    let y = water - 0.6 + (0.25 * (fbm(x * 0.06, z * 0.06, 3) - 0.5) + 0.6 - water + 0.6) * bank ** 0.7;
    if (f.d < 3.4) y = water - 0.6 - 0.5 * (1 - f.d / 3.4);
    y += 0.02 * Math.max(0, z);
    return y;
  };
  const L = land(height);
  P.addSolid(ground({
    x0: -44, x1: 44, z0: -50, z1: 76, step: 0.5, height, weights: (x, z, y) => {
      const p = inPlot(x, z) >= 0 ? 1 : 0;
      const yard = Math.hypot((x - 0.5) / 10, (z + 4) / 5) < 1 ? 1 : 0;
      const path = Math.abs(x - 1.2 + 0.25 * (z - 4)) < 0.9 && z > 1 && z < 30 ? 1 : 0;
      const mud = field(x, z).d < 4.2 ? 1 : 0;
      return [1 - Math.max(p, yard, path, mud), Math.max(yard, path) * (1 - p), mud, p];
    },
  }), [['meadow', PAT.GROUND], ['dirt', PAT.DIRT], ['mud', PAT.DIRT], ['dirt', PAT.SAND]]);
  P.addWater(riverSheet({ centre, width: () => 6.4, level: () => water, n: 200 }));
  P.setMist(water, 1.6, 0.85, 0.07, new THREE.Color('#e8e6ee'));

  // houses round the yard, facing it
  const houses = [[-8.5, -6.0, 1.35, 6.5], [0.5, -9.5, -0.08, 7], [8.8, -5.5, -1.2, 6], [-4.4, -0.6, 0.12, 6], [6.0, 1.2, 1.5, 5.5]];
  houses.forEach(([x, z, yaw, len], i) => put(P, longhouse({ seed: 10 + i, len, w: 3.8, wallH: 1.25, roofH: 2.3, mat: i % 2 ? 'thatch' : 'reed' }), x, z, { ground: L.at, yaw, sink: 0.05 }));
  const puffs = new Puffs();
  houses.forEach(([x, z], i) => smoke(puffs, { at: [x, L.at(x, z) + 4.3, z], height: 7, drift: [1.0, -0.5], seed: 30 + i, size: 0.2, tone: 5, alpha: 0.35 }));
  // the yard: a fire, pots, a rack, logs
  const H = hearth({ seed: 3, level: 2 });
  H.fire[3] *= 0.4; H.fire[4] *= 0.5;
  put(P, H, 1.2, -3.8, { ground: L.at });
  smoke(puffs, { at: [1.2, 0.9 + L.at(1.2, -3.8), -3.8], height: 8, drift: [1.0, -0.5], seed: 50, size: 0.26, tone: 5, alpha: 0.42 });
  P.addPuffs(puffs);
  const pots = new Solid();
  for (let i = 0; i < 7; i++) { const x = -1.2 + i * 0.5 + r.range(-0.1, 0.1), z = -5.6 + r.range(-0.2, 0.2); pots.add(pot({ h: r.range(0.32, 0.5), r: r.range(0.15, 0.22), mat: r.chance(0.3) ? 'charcoal' : 'clay' }).solid, M4.T(x, L.at(x, z), z)); }
  P.addSolid(pots);
  put(P, rack({ seed: 4, len: 2.4, hang: 'hide' }), 10.5, 0.5, { ground: L.at, yaw: -0.5 });
  put(P, log({ len: 2.2 }), 3.4, -2.6, { ground: L.at, yaw: 0.3 });

  // the goat pen and the plots
  const penPts = [[-11.5, 3], [-7, 3.4], [-6.6, 7.2], [-11.8, 7.0], [-11.5, 3]];
  put(P, fence({ pts: penPts, h: 1.0, ground: L.at }), 0, 0, { y: 0 });
  for (let i = 0; i < 9; i++) put(P, animal({ species: i % 3 ? 'goat' : 'sheep', pose: r.chance(0.5) ? 'graze' : 'stand', seed: 60 + i }), r.range(-11, -7.4), r.range(3.8, 6.6), { ground: L.at, yaw: r.range(0, 6.28) });
  const crop = new Cards();
  plots.forEach(([px, pz, w, d, a], k) => {
    const c = Math.cos(a), s = Math.sin(a);
    const ripe = k % 3 !== 1;
    for (let i = 0; i < w * d * 9; i++) {
      const lx = r.range(-w / 2 + 0.2, w / 2 - 0.2), lz = Math.round(r.range(-d / 2 + 0.2, d / 2 - 0.2) / 0.45) * 0.45;
      const x = px + lx * c - lz * s, z = pz + lx * s + lz * c;
      crop.add({ c: [x, L.at(x, z) - 0.03, z], w: 32 * K.mpp * 0.9, h: 32 * K.mpp * (ripe ? 1.05 : 0.7), tile: ripe ? TILE.GRAIN : r.pick(TILE.TUFT), mat: ripe ? 'thatch' : 'leafsp', bias: ripe ? 0 : 1, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, n: [0, 1, 0], upright: 1 });
    }
  });
  P.addCards(crop);
  const folk = (spec, x, z, yaw, extra = {}) => {
    marks['p' + spec.seed] = [x, L.at(x, z) + (spec.kind === 'child' ? 1.3 : 1.95), z]; // above the head, for talk bubbles
    return put(P, person(spec), x, z, { ground: L.at, yaw, ...extra });
  };
  const cloth = (m) => ({ top: 'tunic', topMat: m, legs: 'leggings', legMat: 'leather' });
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'bun', ...cloth('cloth'), pose: 'dig', held: 'staff', seed: 1 }, -1.0, 12.4, 0.4);
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, ...cloth('dyedred'), pose: 'dig', held: 'staff', seed: 2 }, 5.6, 10.2, -0.3);
  folk({ kind: 'woman', skin: 'skin1', hairStyle: 'braid', ...cloth('ochre'), pose: 'stoop', seed: 3 }, -5.2, 9.4, 1.0);
  folk({ kind: 'youth', skin: 'skin2', hairStyle: 'short', ...cloth('cloth'), pose: 'carry', held: { kind: 'bundle', mat: 'thatch' }, seed: 4 }, 1.6, 15.2, 2.8);
  folk({ kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'long', ...cloth('leather'), pose: 'stand', held: 'staff', seed: 5 }, -9.0, 8.4, 2.6);
  folk({ kind: 'woman', skin: 'skin3', hairStyle: 'long', ...cloth('dyedred'), pose: 'sitFloor', held: 'pot', seed: 6 }, -0.8, -4.6, 2.9);
  folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'cloth', pose: 'walkA', seed: 7 }, 1.8, 4.4, -2.6);
  folk({ kind: 'man', skin: 'skin1', hairStyle: 'long', ...cloth('cloth'), pose: 'walkB', held: { kind: 'pot', mat: 'clay' }, seed: 8 }, 6.4, -0.8, 2.4);
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', ...cloth('ochre'), pose: 'walkA', held: 'basket', seed: 9 }, 2.2, 21.5, 2.9);
  put(P, animal({ species: 'dog', pose: 'walk', seed: 70 }), 2.8, 5.4, { ground: L.at, yaw: -2.4 });
  put(P, animal({ species: 'dog', pose: 'lie', seed: 71 }), -1.5, -3.0, { ground: L.at, yaw: 0.8 });

  // trees, bushes and grass round about; geese over the river
  for (let i = 0; i < 46; i++) {
    const x = r.range(-34, 34), z = r.range(-50, 44), f = field(x, z);
    if (f.d < 5 || (Math.abs(x) < 16 && z > -12 && z < 24)) continue;
    put(P, z < -20 && r.chance(0.3) ? conifer({ seed: 100 + i, h: r.range(9, 13) }) : broadleaf({ seed: 100 + i, h: r.range(7, 11) }), x, z, { ground: L.at });
  }
  for (let i = 0; i < 20; i++) {
    const x = r.range(-30, 30), z = r.range(-8, 40);
    if (inPlot(x, z) >= 0 || Math.hypot((x - 0.5) / 13, (z + 3.5) / 7) < 1) continue;
    put(P, bush({ seed: 200 + i, size: r.range(0.7, 1.3), berries: r.chance(0.3) ? 'berry' : null }), x, z, { ground: L.at });
  }
  const low = new Cards();
  const open = (x, z) => inPlot(x, z) < 0 && Math.hypot((x - 0.5) / 12, (z + 3.5) / 6) > 1.05 && field(x, z).d > 4.4 && !(Math.abs(x - 1.2 + 0.25 * (z - 4)) < 1 && z > 1);
  scatter(low, { r, area: [-44, 44, -50, 76], count: 2600, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: 1, accept: open });
  scatter(low, { r, area: [-44, 44, -50, 76], count: 700, tiles: TILE.FLOWER, mat: (x, z) => (noise2(x * 0.2, z * 0.2) > 0.5 ? 'flowerw' : 'flowerp'), place: L.place, accept: open });
  scatter(low, { r, area: [-34, 34, -30, -4], count: 500, tiles: [TILE.REED, TILE.SEDGE], mat: 'reed', place: L.place, accept: (x, z) => { const d = field(x, z).d; return d > 2.6 && d < 4.6; } });
  P.addCards(low);
  for (const [x, y, z, w] of [[-6, 8, -18, 0.2], [-4.6, 8.4, -19, 0.7], [-3.2, 8.2, -18.2, 0.4], [-1.8, 8.6, -19.4, 0.9], [-0.4, 8.3, -18.6, 0.5]]) put(P, bird({ kind: 'goose', fly: true, wing: w, scale: 0.7 }), x, z, { y, yaw: 1.4 });
  return { P, marks };
}

export default async function (args) {
  const { P } = await build(args);
  return P.render(4);
}
