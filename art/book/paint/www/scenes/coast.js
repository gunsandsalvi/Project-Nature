// Age of rites: a burial on a headland above the sea at dusk. The band stands round the open grave; the dead lies
// in red ochre with shells; a woman lifts her arms, the drummer keeps time. Below, the beach camp by its shell midden.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder, blockCliff, stoneBlock } from '../kit/land.js';
import { bush, scatter, conifer } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { bird } from '../kit/animals.js';
import { hearth, smoke, windbreak, rack, canoe, midden, grave } from '../kit/things.js';
import { land, put, waterPlane } from '../kit/scene.js';

export default async function ({ light, opts }) {
  K.mpp = 0.045;
  const P = new Painter({ w: 336, h: 748, mpp: K.mpp, yaw: 12, elev: 30, target: [-1, 1.5, 0], mood: light, bounds: { c: [0, 0, -4], r: 50 } });
  P.haze = [P.camDepth + 2, P.camDepth + 50];
  const r = new Rand(31);
  const sea = -0.9;

  // land: a beach in front, a bay, and across it a grassy headland whose sea cliff faces us; open sea to the east
  const top = 5.2;
  const nearShore = (x) => 6 + 0.9 * Math.sin(x * 0.15) + 0.08 * x;
  const cliffZ = (x) => -9 + 1.4 * Math.sin(x * 0.2 + 1) - 0.05 * x;
  const headEnd = 9;
  const height = (x, z) => {
    if (z > nearShore(x)) {
      const d = z - nearShore(x);
      return Math.min(1.5, 0.22 * d) - 0.35 + 0.5 * (fbm(x * 0.08 + 2, z * 0.08, 3) - 0.5) * Math.min(1, d / 5) - Math.max(0, x - 14) * 0.3;
    }
    const cz = cliffZ(x);
    if (z < cz - 3.2 && x < headEnd + (cz - z) * 0.25) {
      const edgeE = Math.max(0, x - headEnd + 4) * 0.5;
      return top - edgeE + 0.4 * (fbm(x * 0.15, z * 0.15) - 0.5) - Math.max(0, -z - 30) * 0.05;
    }
    return -2.6;
  };
  const L = land(height);
  const inHead = (x, z) => (z < cliffZ(x) - 3.4 && x < headEnd ? 0.5 : 2);
  P.addSolid(ground({
    x0: -36, x1: 30, z0: -54, z1: 44, step: 0.5, height, weights: (x, z, y) => {
      const sand = z > nearShore(x) && y < 0.6 ? 1 : 0;
      const wet = y < sea + 0.3 ? 1 : 0;
      return [1 - sand - wet * (1 - sand), sand * (1 - wet), wet, 0];
    },
  }), [['grass', PAT.GROUND], ['sand', PAT.SAND], ['mud', PAT.SAND], ['grass', PAT.GROUND]]);
  P.addWater(waterPlane({ x0: -60, x1: 60, z0: -70, z1: 50, y: sea, flow: [0.6, 0.8] }), 1);

  // the headland's sea cliff, facing the bay
  const path = [];
  for (let x = -40; x <= headEnd + 2; x += 2) path.push([x, cliffZ(x)]);
  const C = blockCliff({ seed: 41, path, base: -2.4, beds: [1.6, 1.3, 1.5, 1.2, 1.1, 1.0], mat: 'rock', grass: 'grass', bedMats: ['rock', 'rock', 'lime', 'rock', 'lime', 'rock'], jut: 0.35, depth: 6 });
  P.addSolid(C.solid);
  const stones = new Solid();
  for (let i = 0; i < 34; i++) {
    const x = r.range(-34, headEnd + 3), z = cliffZ(x) + r.range(0.3, 3.5), s = r.range(0.4, 1.5);
    stones.add(stoneBlock({ seed: 600 + i, size: [s * 1.4, s, s * 1.2], chip: 0.4, mat: 'rock', top: 'moss' }), M4.mul(M4.T(x, sea - 0.4 + s * 0.3, z), M4.R(r.range(-0.3, 0.3), r.range(0, 3), r.range(-0.3, 0.3))));
  }
  for (let i = 0; i < 16; i++) {
    const x = r.range(-30, 24), z = nearShore(x) + r.range(-0.5, 8), s = r.range(0.3, 0.9);
    stones.add(boulder({ seed: 700 + i, size: [s * 1.4, s, s * 1.2], detail: 1, mat: 'rock', moss: 0.2 }), M4.T(x, L.at(x, z) - 0.1, z));
  }
  P.addSolid(stones);

  // the grave on the headland, the dead laid in ochre, the band round it
  const gx = -1.5, gz = cliffZ(-1.5) - 7.6, gy = L.at(gx, gz);
  put(P, grave({ seed: 3 }), gx, gz, { y: gy, yaw: 0.2 });
  put(P, person({ kind: 'man', skin: 'skin2', hairStyle: 'long', top: 'none', legs: 'leggings', legMat: 'dyedred', paint: 'dyedred', necklace: 'shell', headband: 'shell', pose: 'lie', seed: 1 }), gx, gz, { y: gy - 0.46, yaw: 0.2 - Math.PI / 2 });
  const ochre = new Cards();
  for (let i = 0; i < 40; i++) ochre.add({ c: [gx + r.range(-1.2, 1.2), gy - 0.47, gz + r.range(-0.5, 0.5)], w: K.mpp * 2, h: K.mpp * 2, tile: TILE.DOT, mat: 'dyedred', bias: 0, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, n: [0, 1, 0] });
  P.addCards(ochre);
  const mourners = [
    [{ kind: 'woman', skin: 'skin2', hairStyle: 'long', top: 'dress', topMat: 'hide', necklace: 'shell', pose: 'armsUp' }, -1.9, 0.6],
    [{ kind: 'man', skin: 'skin3', hairStyle: 'long', top: 'wrap', topMat: 'fur', headband: 'dyedred', pose: 'drum', held: 'drum' }, 2.4, 1.6],
    [{ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'leather', paint: 'ochre', pose: 'point', held: 'staff' }, 2.2, -0.9],
    [{ kind: 'woman', skin: 'skin1', hairStyle: 'braid', top: 'dress', topMat: 'leather', pose: 'kneel' }, -1.2, 1.5],
    [{ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'hide', pose: 'stand' }, -2.6, 2.3],
    [{ kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'hide', pose: 'stand', held: 'spear' }, 0.4, 2.6],
    [{ kind: 'youth', skin: 'skin3', hairStyle: 'topknot', top: 'tunic', topMat: 'dyedred', pose: 'stand' }, -0.6, -1.6],
    [{ kind: 'woman', skin: 'skin3', hairStyle: 'bun', top: 'dress', topMat: 'hide', pose: 'stand', held: { kind: 'baby', skin: 'skin3' } }, 1.3, 2.5],
  ];
  mourners.forEach(([spec, dx, dz], i) => {
    const x = gx + dx, z = gz + dz, yaw = Math.atan2(gx - x, gz - z);
    put(P, person({ ...spec, seed: 10 + i }), x, z, { ground: L.at, yaw });
  });
  // small stones set round the grave
  const ringStones = new Solid();
  for (let i = 0; i < 12; i++) {
    const a = (i / 12) * Math.PI * 2, x = gx + Math.cos(a) * 3.6, z = gz + Math.sin(a) * 3.0;
    ringStones.add(stoneBlock({ seed: 800 + i, size: [0.5, 0.7, 0.4], chip: 0.35, mat: 'lime', top: 'moss' }), M4.mul(M4.T(x, L.at(x, z) + 0.25, z), M4.R(0, a, r.range(-0.1, 0.1))));
  }
  P.addSolid(ringStones);

  // the beach camp below: a midden, a windbreak, a fire, a canoe drawn up, fish drying
  put(P, midden({ seed: 4, r: 2.4, h: 0.7 }), 5.5, 13.5, { ground: L.at });
  const H = hearth({ seed: 6, level: 3 });
  put(P, H, 1.5, 14.6, { ground: L.at });
  put(P, rack({ seed: 7, len: 2.4, hang: 'fish' }), -3.5, 13.0, { ground: L.at, yaw: 0.5 });
  put(P, canoe({ len: 4.4 }), 3.5, 8.6, { ground: L.at, yaw: 0.3 });
  put(P, canoe({ len: 3.8 }), 6.8, 9.4, { ground: L.at, yaw: 0.1 });
  put(P, person({ kind: 'elder', skin: 'skin1', hair: 'hairgrey', hairStyle: 'bun', top: 'dress', topMat: 'hide', pose: 'sitFloor', seed: 30 }), 0.4, 15.3, { ground: L.at, yaw: 2.4 });
  put(P, person({ kind: 'child', skin: 'skin1', hairStyle: 'long', top: 'tunic', topMat: 'leather', pose: 'walkA', held: 'basket', seed: 31 }), 3.0, 11.6, { ground: L.at, yaw: -0.8 });
  put(P, person({ kind: 'man', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'leather', pose: 'carry', held: 'fish', seed: 32 }), 5.6, 10.0, { ground: L.at, yaw: -2.6 });
  // a few walk the beach toward the headland path, carrying shells and ochre for the grave
  const walkers = [['woman', 'skin2', 'walkA', 'basket'], ['man', 'skin3', 'walkB', 'staff'], ['child', 'skin2', 'walkA', null], ['woman', 'skin1', 'walkB', 'pot']];
  walkers.forEach(([kind, skin, pose, held], i) => put(P, person({ kind, skin, hairStyle: i % 2 ? 'long' : 'braid', top: kind === 'child' ? 'tunic' : 'dress', topMat: i % 2 ? 'leather' : 'hide', necklace: 'shell', pose, held, seed: 40 + i }), 3.5 - i * 1.5, 21.5 + i * 0.7, { ground: L.at, yaw: -1.9 }));
  const puffs = new Puffs();
  smoke(puffs, { at: [1.5, 0.9, 14.6], height: 9, drift: [-0.9, -0.5], seed: 7, size: 0.28, tone: 4, alpha: 0.4 });
  P.addPuffs(puffs);

  // dune grass, sea pinks on the headland, gulls
  const low = new Cards();
  scatter(low, { r, area: [-34, 28, 4, 44], count: 1300, tiles: [TILE.SEDGE, TILE.DRYTUFT[0], TILE.TUFT[2]], mat: (x, z) => (noise2(x * 0.2, z * 0.2) > 0.5 ? 'drygrass' : 'grass'), place: L.place, bias: 0, accept: (x, z) => L.at(x, z) > 0.3 });
  scatter(low, { r, area: [-30, 10, -40, -9], count: 1200, tiles: TILE.TUFT, mat: 'grass', place: L.place, bias: 1, accept: (x, z) => inHead(x, z) < 1 && Math.hypot(x - gx, z - gz) > 2 });
  scatter(low, { r, area: [-30, 10, -40, -9], count: 420, tiles: TILE.FLOWER, mat: 'flowerp', place: L.place, accept: (x, z) => inHead(x, z) < 1 });
  P.addCards(low);
  for (let i = 0; i < 26; i++) {
    const x = r.range(-20, 22), z = r.range(-30, 30);
    if (L.at(x, z) < 1.5 || Math.hypot(x - gx, z - gz) < 5) continue;
    put(P, bush({ seed: 900 + i, size: r.range(0.6, 1.2), mat: r.chance(0.5) ? 'moss' : 'leaf' }), x, z, { ground: L.at });
  }
  for (const [x, y, z, w] of [[4, 8, -4, 0.3], [6, 9, -2, 0.8], [-6, 10, -14, 0.5], [9, 7, 2, 0.2]]) put(P, bird({ kind: 'gull', fly: true, wing: w }), x, z, { y, yaw: 2.2 });
  return P.render(4);
}
