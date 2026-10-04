// Age of copper and chiefs: in dry red hills, a village smelts copper. Men blow through pipes into clay furnaces
// beside heaps of charcoal and green ore; the chief, in copper, watches from his walled yard; goats on the slopes.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder, blockCliff, stoneBlock } from '../kit/land.js';
import { bush, scatter, flatTree } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { furnace, smoke, mudHouse, pot, rack, hearth } from '../kit/things.js';
import { land, put } from '../kit/scene.js';

export default async function ({ light, opts }) {
  K.mpp = 0.045;
  const P = new Painter({ w: 336, h: 748, mpp: K.mpp, yaw: 18, elev: 30, target: [0, 1.5, 0], mood: light, bounds: { c: [0, 2, -4], r: 50 } });
  P.haze = [P.camDepth + 4, P.camDepth + 50];
  const r = new Rand(83);

  // a dry valley under a red cliff; a stony wash runs across the front
  const beds = [1.5, 1.2, 1.7, 1.1, 1.4, 1.2, 1.3], top = -0.3 + beds.reduce((a, b) => a + b, 0);
  const cliffZ = (x) => -15 + 1.6 * Math.sin(x * 0.12 + 0.6) - 0.03 * x;
  const washZ = (x) => 16 + 2 * Math.sin(x * 0.15);
  const height = (x, z) => {
    const cz = cliffZ(x);
    if (z < cz - 4) return top - 0.1 + 0.5 * (fbm(x * 0.08, z * 0.08) - 0.5);
    let y = -0.03 * Math.max(0, z - cz) + 0.7 * (fbm(x * 0.06 + 5, z * 0.06 + 2, 4) - 0.5) + Math.max(0, 6 - (z - cz)) * 0.12;
    const d = Math.abs(z - washZ(x));
    y -= 0.5 * Math.max(0, 1 - d / 2.4);
    return y;
  };
  const L = land(height);
  const yard = (x, z) => Math.hypot((x - 1) / 11, (z + 2) / 7.5) < 1;
  P.addSolid(ground({
    x0: -36, x1: 36, z0: -52, z1: 44, step: 0.5, height, weights: (x, z, y, slope) => {
      const wash = Math.abs(z - washZ(x)) < 1.9 ? 1 : 0;
      const bare = yard(x, z) ? 1 : 0;
      const red = slope > 0.15 || noise2(x * 0.15, z * 0.15) > 0.62 ? 1 : 0;
      return [1 - Math.max(wash, bare, red), bare, wash, red * (1 - bare) * (1 - wash)];
    },
  }), [['drygrass', PAT.GROUND], ['dirt', PAT.DIRT], ['sand', PAT.SAND], ['redrock', PAT.DIRT]]);
  const path = [];
  for (let x = -40; x <= 40; x += 2) path.push([x, cliffZ(x)]);
  P.addSolid(blockCliff({ seed: 91, path, base: -0.4, beds, mat: 'redrock', moss: 'drygrass', grass: 'drygrass', jut: 0.4, setback: 0.35 }).solid);
  const stones = new Solid();
  for (let i = 0; i < 60; i++) {
    const x = r.range(-34, 34), z = r.range(-14, 40);
    if (yard(x, z)) continue;
    const s = r.range(0.2, 1.1);
    stones.add(stoneBlock({ seed: 300 + i, size: [s * 1.4, s, s * 1.2], chip: 0.42, mat: 'redrock', top: 'drygrass' }), M4.mul(M4.T(x, L.at(x, z) + s * 0.25, z), M4.R(r.range(-0.2, 0.2), r.range(0, 3), r.range(-0.2, 0.2))));
  }
  for (let i = 0; i < 90; i++) {
    const x = r.range(-34, 34), z = washZ(x) + r.range(-1.6, 1.6), s = r.range(0.1, 0.35);
    stones.add(boulder({ seed: 400 + i, size: [s * 1.3, s * 0.8, s], detail: 0, mat: r.pick(['redrock', 'lime', 'rock']), moss: 0 }), M4.T(x, L.at(x, z) - s * 0.2, z));
  }
  P.addSolid(stones);

  // the village: mud-brick houses, the chief's with its walled yard
  const houses = [[-8.5, -7.5, 0.15, 4.6, 3.8, 0], [1.5, -9, 0, 7, 5, 4], [10, -6.5, -0.2, 4.4, 3.6, 0], [-11, 1.5, 0.9, 4, 3.4, 0], [12.5, 3, -1.1, 4.2, 3.4, 0]];
  houses.forEach(([x, z, yaw, w, d, court], i) => put(P, mudHouse({ seed: 20 + i, w, d, h: i === 1 ? 2.8 : 2.2, court }), x, z, { ground: L.at, yaw, sink: 0.1 }));

  // the smelting ground: three furnaces, blowers round each, charcoal and ore
  const puffs = new Puffs();
  [[-3.5, 3.5], [1.5, 5.5], [6.5, 3.8]].forEach(([x, z], i) => {
    const F = furnace({ seed: 30 + i, lit: true });
    put(P, F, x, z, { ground: L.at, yaw: i * 0.7 });
    smoke(puffs, { at: [x, L.at(x, z) + 1.1, z], height: 10, drift: [0.6, -0.6], seed: 40 + i, size: 0.3, tone: 3, alpha: 0.5 });
    for (let k = 0; k < 3; k++) {
      const a = (k / 3) * Math.PI * 2 + 0.6 + i * 0.7;
      const px = x + Math.cos(a) * 1.25, pz = z + Math.sin(a) * 1.25;
      put(P, person({ kind: k === 2 ? 'youth' : 'man', skin: r.pick(['skin2', 'skin3']), hairStyle: r.pick(['short', 'long', 'topknot']), top: 'none', legs: 'leggings', legMat: 'cloth', pose: 'blow', seed: 50 + i * 3 + k }), px, pz, { ground: L.at, yaw: Math.atan2(x - px, z - pz) });
    }
  });
  P.addPuffs(puffs);
  // slag and finished copper
  const slag = new Solid();
  slag.add(boulder({ seed: 61, size: [2.4, 0.6, 1.6], detail: 1, mat: 'charcoal', moss: 0, sink: 0.4 }), M4.T(9.5, L.at(9.5, 7) - 0.1, 7));
  for (let i = 0; i < 6; i++) slag.box(M4.mul(M4.T(-0.5 + i * 0.3, L.at(-0.5, 9) + 0.04, 9 + (i % 2) * 0.2), M4.R(0, r.range(-0.4, 0.4), 0)), 0.22, 0.06, 0.12, { mat: 'copper', obj: newObj() });
  P.addSolid(slag);

  // the chief in copper, in his yard; a trader brings ore; potters, goats, a dog
  const folk = (spec, x, z, yaw) => put(P, person(spec), x, z, { ground: L.at, yaw });
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'long', beard: true, top: 'tunic', topMat: 'dyedred', legMat: 'cloth', necklace: 'copper', headband: 'copper', belt: 'copper', cloak: 'fur', pose: 'point', held: 'staff', seed: 70 }, 1.6, -3.0, 0.2);
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'cloth', necklace: 'copper', pose: 'stand', seed: 71 }, 0.2, -3.6, 0.6);
  folk({ kind: 'man', skin: 'skin3', hairStyle: 'short', top: 'tunic', topMat: 'cloth', pose: 'walkA', held: { kind: 'basket', mat: 'verdi' }, seed: 72 }, -6.5, 8.5, 2.3);
  folk({ kind: 'youth', skin: 'skin3', hairStyle: 'short', top: 'tunic', topMat: 'ochre', pose: 'walkB', held: { kind: 'bundle', mat: 'charcoal' }, seed: 73 }, -8.0, 10.0, 2.4);
  folk({ kind: 'woman', skin: 'skin1', hairStyle: 'bun', top: 'dress', topMat: 'ochre', pose: 'sitFloor', held: 'pot', seed: 74 }, -9.8, -3.2, 1.0);
  folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'cloth', pose: 'point', seed: 75 }, 4.5, 8.8, -0.5);
  const jars = new Solid();
  for (let i = 0; i < 5; i++) jars.add(pot({ h: 0.5, r: 0.2, mat: i % 2 ? 'clay' : 'ochre' }).solid, M4.T(-10.8 + i * 0.5, L.at(-10.8, -4), -4.2));
  P.addSolid(jars);
  for (let i = 0; i < 10; i++) {
    const x = r.range(-26, -10), z = r.range(12, 30);
    put(P, animal({ species: 'goat', pose: r.chance(0.6) ? 'graze' : 'walk', seed: 90 + i }), x, z, { ground: L.at, yaw: r.range(0, 6.28) });
  }
  folk({ kind: 'youth', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'cloth', pose: 'stand', held: 'staff', seed: 76 }, -15, 20, 1.2);
  put(P, animal({ species: 'dog', pose: 'stand', seed: 99 }), -13.8, 21, { ground: L.at, yaw: 1.8 });

  // dry trees, scrub and grass; vultures wheeling
  for (const [x, z, h, o] of [[-18, -3, 6, 0], [16, 10, 5.5, 1], [-22, 9, 7, 0], [20, -2, 6, 0], [-4, 26, 6.5, 1], [9, 30, 5, 0], [24, 22, 6, 1]]) put(P, flatTree({ seed: 500 + x * 3, h, olive: !!o }), x, z, { ground: L.at });
  for (let i = 0; i < 40; i++) {
    const x = r.range(-34, 34), z = r.range(-12, 42);
    if (yard(x, z)) continue;
    put(P, bush({ seed: 600 + i, size: r.range(0.5, 1.1), mat: r.chance(0.6) ? 'pine' : 'drygrass' }), x, z, { ground: L.at });
  }
  const low = new Cards();
  scatter(low, { r, area: [-36, 36, -14, 44], count: 1500, tiles: TILE.DRYTUFT.concat([TILE.SEDGE]), mat: 'drygrass', place: L.place, bias: 0, accept: (x, z) => !yard(x, z) && Math.abs(z - washZ(x)) > 1.8 });
  scatter(low, { r, area: [-36, 36, -52, -16], count: 600, tiles: TILE.DRYTUFT, mat: 'drygrass', place: L.place, accept: (x, z) => z < cliffZ(x) - 4 });
  P.addCards(low);
  for (const [x, y, z, w] of [[-4, 13, -10, 0.3], [2, 14, -14, 0.8], [6, 12.5, -11, 0.5]]) put(P, bird({ kind: 'vulture', fly: true, wing: w, scale: 0.8 }), x, z, { y, yaw: 0.8 });
  return P.render(4);
}
