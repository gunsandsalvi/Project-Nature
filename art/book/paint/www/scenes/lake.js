// Age of pottery and dogs: a camp on a reedy lakeshore at golden hour. A potter coils a pot while others dry in a
// row; a bonfire kiln smokes; dogs doze and run with the children; a canoe works the fish weir.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder } from '../kit/land.js';
import { broadleaf, bush, scatter, crown } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { hearth, smoke, dome, rack, pot, canoe, log } from '../kit/things.js';
import { land, put, waterPlane } from '../kit/scene.js';

export default async function ({ light, opts }) {
  K.mpp = 0.036;
  const P = new Painter({ w: 336, h: 748, mpp: K.mpp, yaw: 20, elev: 30, target: [2.5, 0.5, 3.5], mood: light, bounds: { c: [0, 0, -2], r: 46 } });
  P.haze = [P.camDepth + 4, P.camDepth + 46];
  const r = new Rand(57);
  const lake = -0.5;

  // the shore runs across the picture: land in front, the lake behind with reed beds and an island
  const shoreZ = (x) => -4 + 2.2 * Math.sin(x * 0.16 + 0.4) + 0.06 * x;
  const height = (x, z) => {
    const d = z - shoreZ(x);
    let y = d > 0 ? Math.min(1.1, 0.16 * d) - 0.2 : Math.max(-2.2, 0.35 * d - 0.2);
    y += 0.35 * (fbm(x * 0.07 + 1, z * 0.07 + 3, 3) - 0.5) * Math.min(1, Math.max(0, d) / 4);
    if (Math.abs(x - 3.2) < 1.4 && Math.abs(z - 13.8) < 0.9) y -= 0.36;   // the clay pit
    const isl = Math.hypot((x + 6) / 6, (z + 24) / 3.5);
    if (isl < 1) y = Math.max(y, lake + 0.9 * (1 - isl) + 0.1);
    return y;
  };
  const L = land(height);
  // a trodden path from the camp past the clay pit to the meadow
  const pathX = (z) => 2.2 + 1.2 * Math.sin(z * 0.25) - Math.max(0, z - 14) * 0.25;
  P.addSolid(ground({
    x0: -34, x1: 34, z0: -50, z1: 40, step: 0.5, height, weights: (x, z, y) => {
      const camp = Math.hypot((x - 1) / 9, (z - 6) / 5) < 1 || (z > 8 && Math.abs(x - pathX(z)) < 0.5 + 0.15 * Math.sin(z * 1.7)) ? 1 : 0;
      const mud = y < lake + 0.35 ? 1 : 0;
      return [1 - Math.max(camp, mud), camp * (1 - mud), mud, 0];
    },
  }), [['meadow', PAT.GROUND], ['dirt', PAT.DIRT], ['mud', PAT.DIRT], ['grass', PAT.GROUND]]);
  P.addWater(waterPlane({ x0: -60, x1: 60, z0: -70, z1: 40, y: lake, flow: [1, 0.15] }));

  // reed beds along the shore and round the island; willows
  const low = new Cards();
  const reedOk = (x, z) => { const y = L.at(x, z); return y < lake + 0.25 && y > lake - 0.7 && noise2(x * 0.25, z * 0.25) > 0.42; };
  scatter(low, { r, area: [-34, 34, -40, 6], count: 1100, tiles: [TILE.REED, TILE.SEDGE, TILE.SEDGE], mat: 'reed', place: (x, z) => [Math.max(L.at(x, z), lake), [0, 1, 0]], accept: reedOk });
  scatter(low, { r, area: [-34, 34, -6, 40], count: 1300, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: 1, accept: (x, z) => L.at(x, z) > lake + 0.3 && Math.hypot((x - 1) / 9, (z - 6) / 5) > 1.05 });
  scatter(low, { r, area: [-34, 34, -6, 40], count: 380, tiles: TILE.FLOWER, mat: (x, z) => (noise2(x * 0.2, z * 0.2) > 0.5 ? 'flowery' : 'flowerw'), place: L.place, accept: (x, z) => L.at(x, z) > lake + 0.3 });
  P.addCards(low);
  for (const [x, z, h] of [[-12, 4, 9], [-16, 9, 8], [14, 2, 10], [18, 10, 8], [-7, -24, 7], [-4, -25, 6], [24, 22, 9], [-22, 24, 9]]) {
    put(P, broadleaf({ seed: 100 + x * 3, h, spread: 1.2, leafMat: 'leafsp' }), x, z, { ground: L.at });
  }
  for (let i = 0; i < 18; i++) {
    const x = r.range(-30, 30), z = r.range(4, 38);
    if (Math.hypot((x - 1) / 10, (z - 6) / 6) < 1.1) continue;
    put(P, bush({ seed: 200 + i, size: r.range(0.7, 1.4), berries: r.chance(0.4) ? 'berry' : null }), x, z, { ground: L.at });
  }

  // the camp: reed-thatched domes, the potter's place, the kiln, racks of fish
  for (const [x, z, s, yaw] of [[-5, 3.5, 2.4, 0.5], [6.5, 4.0, 2.2, -0.4], [1.5, 9.5, 2.6, 0.1]]) {
    put(P, dome({ seed: x * 7 + 3, r: s, h: s * 0.95, mat: 'thatch', pat: PAT.THATCH }), x, z, { ground: L.at, yaw });
  }
  put(P, rack({ seed: 3, len: 2.6, hang: 'fish' }), -6.5, 8.6, { ground: L.at, yaw: 0.6 });
  put(P, rack({ seed: 4, len: 2.2, hang: 'fish' }), 9.5, 8.0, { ground: L.at, yaw: -0.5 });
  // the bonfire kiln: pots stacked in the fire under smoke
  const K1 = hearth({ seed: 5, level: 4, logs: 6 });
  K1.fire[3] *= 0.5; K1.fire[4] *= 0.5;
  put(P, K1, 4.5, 0.6, { ground: L.at });
  const kiln = new Solid();
  for (const [dx, dz, s] of [[-0.25, 0.1, 1], [0.22, -0.1, 0.9], [0, 0.28, 0.8]]) kiln.add(pot({ h: 0.34 * s, r: 0.16 * s }).solid, M4.T(4.5 + dx, L.at(4.5, 0.6) + 0.12, 0.6 + dz));
  P.addSolid(kiln);
  // pots drying in a row by the potter
  const row = new Solid();
  for (let i = 0; i < 6; i++) row.add(pot({ h: 0.3 + (i % 3) * 0.06, r: 0.14 + (i % 2) * 0.03, mat: 'clay' }).solid, M4.T(-1.6 + i * 0.42, L.at(-1.6 + i * 0.42, 1.2), 1.2));
  P.addSolid(row);
  const folk = (spec, x, z, yaw, extra = {}) => put(P, person(spec), x, z, { ground: L.at, yaw, ...extra });
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'bun', top: 'dress', topMat: 'leather', necklace: 'shell', pose: 'knap', held: 'pot', seed: 1 }, -0.8, 2.2, 0.3);
  folk({ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'dress', topMat: 'hide', pose: 'sitFloor', seed: 2 }, -2.4, 2.6, 0.9);
  folk({ kind: 'man', skin: 'skin3', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'hide', pose: 'crouch', seed: 3 }, 5.7, 1.4, -0.9);
  folk({ kind: 'youth', skin: 'skin1', hairStyle: 'topknot', top: 'tunic', topMat: 'dyedred', pose: 'carry', held: 'bundle', seed: 4 }, 3.0, 4.4, -1.4);
  folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'walkA', seed: 5 }, 0.5, 6.2, 1.9);
  folk({ kind: 'child', skin: 'skin3', hairStyle: 'long', top: 'tunic', topMat: 'leather', pose: 'walkB', seed: 6 }, -0.6, 6.9, 1.7);
  folk({ kind: 'woman', skin: 'skin1', hairStyle: 'braid', top: 'dress', topMat: 'hide', pose: 'walkA', held: 'basket', seed: 7 }, 8.0, 11.0, 2.6);
  // dogs: one runs with the children, two doze by the domes
  put(P, animal({ species: 'dog', pose: 'run', seed: 8 }), 1.6, 5.6, { ground: L.at, yaw: 1.9 });
  put(P, animal({ species: 'dog', pose: 'lie', seed: 9 }), -3.2, 5.2, { ground: L.at, yaw: 0.3 });
  put(P, animal({ species: 'wolf', pose: 'lie', seed: 10, scale: 0.85 }), 6.4, 6.4, { ground: L.at, yaw: -2.0 });
  put(P, animal({ species: 'dog', pose: 'alert', seed: 11 }), -7.4, 6.6, { ground: L.at, yaw: 1.2 });

  // on the water: a canoe at the fish weir, ducks, a heron
  const weir = new Solid(), wo = newObj();
  for (let i = 0; i < 22; i++) { const x = 4 + i * 0.5, z = -11 - i * 0.25 + Math.sin(i) * 0.1; weir.beam([x, lake - 1.4, z], [x + r.range(-0.05, 0.05), lake + 0.55, z], 0.04, 0.03, 4, { mat: 'wood', obj: wo }); }
  P.addSolid(weir);
  put(P, canoe({ len: 4.4 }), 9.5, -9.4, { y: lake - 0.1, yaw: 0.4 });
  put(P, person({ kind: 'man', skin: 'skin2', hairStyle: 'long', top: 'none', legs: 'leggings', pose: 'aim', held: { kind: 'spear', tilt: 1.0 }, seed: 12 }), 10.0, -9.2, { y: lake + 0.15, yaw: 2.2 });
  put(P, person({ kind: 'youth', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'leather', pose: 'sit', seed: 13 }), 8.6, -9.8, { y: lake + 0.25, yaw: 2.0 });
  for (const [x, z] of [[-3, -12], [-2.4, -12.6], [-3.6, -13.1], [-1.8, -11.6]]) put(P, bird({ kind: 'duck' }), x, z, { y: lake - 0.12, yaw: 2.6 });
  put(P, bird({ kind: 'heron' }), -10, -3.6, { y: lake - 0.3, yaw: 1.0 });
  for (const [x, y, z, w] of [[2, 6, -20, 0.2], [3.6, 6.4, -21, 0.7], [5, 6.1, -19.6, 0.4], [6.4, 6.6, -20.4, 0.9]]) put(P, bird({ kind: 'goose', fly: true, wing: w }), x, z, { y, yaw: 1.6 });

  // in front: the clay pit where they dig potting clay, a basket of it, a dog after a hare
  const pit = new Solid(), po = newObj();
  const px = 3.2, pz = 13.8, py = L.at(px, pz);
  pit.quad([px - 1.4, py - 0.35, pz + 0.9], [px + 1.4, py - 0.35, pz + 0.9], [px + 1.4, py - 0.35, pz - 0.9], [px - 1.4, py - 0.35, pz - 0.9], { mat: 'clay', obj: po, pat: PAT.DIRT });
  for (const [a, b] of [[[px - 1.4, pz + 0.9], [px + 1.4, pz + 0.9]], [[px + 1.4, pz - 0.9], [px - 1.4, pz - 0.9]], [[px + 1.4, pz + 0.9], [px + 1.4, pz - 0.9]], [[px - 1.4, pz - 0.9], [px - 1.4, pz + 0.9]]])
    pit.quad([a[0], py + 0.02, a[1]], [b[0], py + 0.02, b[1]], [b[0], py - 0.35, b[1]], [a[0], py - 0.35, a[1]], { mat: 'clay', obj: po, bias: -1 });
  pit.add(boulder({ seed: 77, size: [1.6, 0.5, 1.0], detail: 1, mat: 'clay', moss: 0, sink: 0.4 }), M4.T(px + 2.4, py - 0.1, pz - 0.6));
  P.addSolid(pit);
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'leggings', legMat: 'leather', pose: 'dig', held: 'staff', seed: 14 }, px - 0.4, pz + 0.1, 0.4, { y: py - 0.35 });
  folk({ kind: 'woman', skin: 'skin3', hairStyle: 'braid', top: 'dress', topMat: 'leather', pose: 'carry', held: { kind: 'basket', mat: 'clay' }, seed: 15 }, px + 1.9, pz + 1.6, 2.5);
  folk({ kind: 'child', skin: 'skin1', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'point', seed: 16 }, 7.5, 17.5, -0.6);
  put(P, animal({ species: 'dog', pose: 'run', seed: 17 }), 9.5, 16.0, { ground: L.at, yaw: 2.4 });
  put(P, animal({ species: 'hare', pose: 'run', seed: 18 }), 11.4, 14.4, { ground: L.at, yaw: 2.4 });
  // the near meadow: a fallen log where two sit and talk, a hide stretched to dry and one being scraped, firewood
  put(P, log({ len: 2.6, r: 0.2 }), -3.2, 19.2, { ground: L.at, yaw: 0.25 });
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'leather', pose: 'sitTalk', seed: 19 }, -4.0, 19.0, 1.2, { y: L.at(-4.0, 19.0) + 0.38 });
  folk({ kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'bun', top: 'dress', topMat: 'hide', pose: 'sit', seed: 20 }, -2.3, 19.4, -1.9, { y: L.at(-2.3, 19.4) + 0.38 });
  put(P, rack({ seed: 21, len: 2.0, hang: 'hide' }), 7.2, 21.0, { ground: L.at, yaw: -0.3 });
  const hide = new Solid();
  hide.box(M4.mul(M4.T(5.6, L.at(5.6, 22.4) + 0.03, 22.4), M4.R(0, 0.5, 0)), 1.3, 0.04, 0.9, { mat: 'hide', pat: PAT.HIDE, obj: newObj() });
  P.addSolid(hide);
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'leather', pose: 'scrape', seed: 22 }, 5.3, 23.3, 3.4);
  const wood = new Solid(), woodObj = newObj();
  for (let i = 0; i < 9; i++) {
    const k = i < 4 ? i : (i < 7 ? i - 4 + 0.5 : i - 7 + 1), lay = i < 4 ? 0 : (i < 7 ? 1 : 2);
    const x = -8 + k * 0.17, z = 16.5, y = L.at(-7.7, 16.5) + 0.08 + lay * 0.14;
    wood.beam([x, y, z - 0.6], [x + r.range(-0.03, 0.03), y, z + 0.6], 0.075, 0.07, 6, { mat: 'bark', pat: PAT.BARK, obj: woodObj }, { capA: true, capMat: { mat: 'wood', obj: woodObj, bias: 1 } });
  }
  P.addSolid(wood);
  put(P, person({ kind: 'youth', skin: 'skin3', hairStyle: 'short', top: 'tunic', topMat: 'hide', pose: 'carry', held: 'bundle', seed: 23 }), -6.4, 18.0, { ground: L.at, yaw: -2.4 });

  const puffs = new Puffs();
  smoke(puffs, { at: [4.5, 1.0, 0.6], height: 9, drift: [1.0, -0.4], seed: 9, size: 0.34, tone: 4, alpha: 0.5 });
  for (const [x, z] of [[-5, 3.5], [1.5, 9.5]]) smoke(puffs, { at: [x, L.at(x, z) + 2.4, z], height: 5, drift: [1.0, -0.4], seed: x + 20, size: 0.18, tone: 5, alpha: 0.32 });
  P.addPuffs(puffs);
  P.setMist(lake, 0.5, 0.3, 0.08, new THREE.Color('#f2d6b8'));
  return P.render(4);
}
