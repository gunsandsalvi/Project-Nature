// Age of clothing and huts: a reindeer hunt on a winter steppe. Hunters in fur crouch behind a snowbank with
// spears; the herd turns across the plain; beyond, the band's huts of hide on bent poles ringed with mammoth bones.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder, stoneBlock } from '../kit/land.js';
import { broadleaf, conifer, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { dome, rack, hearth, smoke, tent } from '../kit/things.js';
import { land, put, lineField } from '../kit/scene.js';

export default async function ({ light, opts }) {
  K.mpp = 0.045;
  const P = new Painter({ w: 336, h: 748, mpp: K.mpp, yaw: 12, elev: 30, target: [1.5, 0, -2], mood: light, bounds: { c: [0, 0, -6], r: 56 } });
  P.haze = [P.camDepth + 4, P.camDepth + 46];
  const r = new Rand(23);

  // rolling snowfield, a frozen stream, a low ridge behind the camp
  const stream = (t) => [-26 + 52 * t, -14 + 5 * Math.sin(t * 7)];
  const near = lineField(stream);
  const memo = new Map();
  const field = (x, z) => { const k = x.toFixed(2) + ',' + z.toFixed(2); let v = memo.get(k); if (!v) { v = near(x, z); memo.set(k, v); } return v; };
  const height = (x, z) => {
    let y = 0.9 * (fbm(x * 0.045 + 3, z * 0.045 + 8, 4) - 0.5) + Math.max(0, -z - 24) * 0.12;
    y += 0.55 * Math.max(0, 1 - Math.hypot((x - 3) / 6, (z - 12) / 1.6)); // the snowbank the hunters hide behind
    const f = field(x, z);
    y -= 0.45 * Math.max(0, 1 - f.d / 2.2);
    return y;
  };
  const L = land(height);
  P.addSolid(ground({
    x0: -34, x1: 34, z0: -56, z1: 44, step: 0.5, height, weights: (x, z, y, slope) => {
      const f = field(x, z);
      const ice = f.d < 1.3 ? 1 : 0;
      const bare = slope > 0.12 && noise2(x * 0.5, z * 0.5) > 0.55 ? 1 : 0;
      return [1 - Math.max(ice, bare), 0, ice, bare * 0.8];
    },
  }), [['snow', PAT.SNOW], ['snow', PAT.SNOW], ['ice', 0], ['drygrass', PAT.GROUND]]);

  // trees: snowy conifers on the ridge and bare birches in hollows
  for (let i = 0; i < 46; i++) {
    const x = r.range(-34, 34), z = r.range(-56, -20);
    if (Math.abs(x + 2) < 10 && z > -36) continue;
    put(P, conifer({ seed: 100 + i, h: r.range(8, 13), snow: true }), x, z, { ground: L.at });
  }
  for (const [x, z] of [[14, 6], [17, 9], [-16, 22], [-19, 26], [18, 30], [12, -6], [-14, -2]]) put(P, broadleaf({ seed: 200 + x, h: r.range(6, 8), season: 'bare' }), x, z, { ground: L.at });
  const rocks = new Solid();
  for (const [x, z, s] of [[-6, 15, 1.1], [3.5, 14.5, 0.8], [8, 21, 1.4], [-9, 4, 0.9], [10, -9, 1.2]]) rocks.add(boulder({ seed: 300 + x, size: [s * 1.5, s, s * 1.2], detail: 1, mat: 'rock', moss: 0.6, mossMat: 'snow' }), M4.T(x, L.at(x, z) - 0.15, z));
  P.addSolid(rocks);

  // dry grass through the snow
  const low = new Cards();
  scatter(low, { r, area: [-34, 34, -40, 44], count: 1600, tiles: TILE.DRYTUFT, mat: 'drygrass', place: L.place, bias: -1, size: 0.8, accept: (x, z) => field(x, z).d > 1.6 && fbm(x * 0.12, z * 0.12) > 0.56 });
  scatter(low, { r, area: [-34, 34, -40, 44], count: 300, tiles: TILE.TWIG, mat: 'bark', place: L.place, accept: (x, z) => field(x, z).d > 1.6 && noise2(x * 0.18 + 5, z * 0.18) > 0.6 });
  P.addCards(low);

  // the camp beyond the stream: domes of hide ringed with bones, a rack of meat, a fire
  const camp = [[-3, -24], [3.5, -27.5], [8.5, -22.5], [2, -19.5]];
  camp.forEach(([x, z], i) => put(P, dome({ seed: 400 + i, r: 2.2 + (i % 2) * 0.4, h: 1.9, mat: i % 2 ? 'fur' : 'hide', pat: i % 2 ? PAT.FUR : PAT.HIDE, bones: true }), x, z, { ground: L.at, yaw: r.range(-0.4, 0.4) + 0.3 }));
  put(P, rack({ seed: 5, len: 2.6, hang: 'meat' }), 7, -18.5, { ground: L.at, yaw: 0.4 });
  const H = hearth({ seed: 6, level: 3 });
  H.fire[3] *= 0.4; H.fire[4] *= 0.5;
  put(P, H, 4.5, -21.5, { ground: L.at });
  const puffs = new Puffs();
  smoke(puffs, { at: [4.5, 0.9, -21.5], height: 9, drift: [1.0, -0.2], seed: 4, size: 0.28, tone: 5, alpha: 0.45 });
  for (const [x, z] of camp) smoke(puffs, { at: [x, L.at(x, z) + 2.0, z], height: 5, drift: [0.9, -0.2], seed: x * 3 + 9, size: 0.18, tone: 5, alpha: 0.35 });
  P.addPuffs(puffs);
  const fur = { top: 'parka', topMat: 'fur', legs: 'leggings', legMat: 'furdark', feet: 'furdark', hairStyle: 'hood' };
  put(P, person({ kind: 'woman', skin: 'skin1', ...fur, pose: 'scrape', seed: 1 }), -0.5, -20.5, { ground: L.at, yaw: 0.6 });
  put(P, person({ kind: 'child', skin: 'skin1', ...fur, topMat: 'furdark', pose: 'point', seed: 2 }), 0.6, -17.6, { ground: L.at, yaw: 0.2 });
  put(P, person({ kind: 'elder', skin: 'skin2', ...fur, topMat: 'reindeer', pose: 'stand', held: 'staff', seed: 3 }), 3.5, -16.8, { ground: L.at, yaw: 0.3 });

  // the herd turning across the plain
  for (let i = 0; i < 22; i++) {
    const a = r.range(0, Math.PI * 2), d = Math.sqrt(r.next()) * 6.5;
    const x = 1.5 + Math.cos(a) * d * 1.1, z = -2 + Math.sin(a) * d * 0.9;
    put(P, animal({ species: 'reindeer', pose: r.chance(0.7) ? 'run' : 'alert', seed: 500 + i, scale: r.range(0.9, 1.08) }), x, z, { ground: L.at, yaw: -2.2 + r.range(-0.35, 0.35) });
  }
  // the hunters: two crouched behind the snowbank, one throwing, one running to cut the herd off
  put(P, person({ kind: 'man', skin: 'skin2', ...fur, pose: 'aim', held: { kind: 'spear', tilt: 1.1 }, seed: 11 }), 1.4, 13.2, { ground: L.at, yaw: 2.9 });
  put(P, person({ kind: 'man', skin: 'skin3', ...fur, topMat: 'reindeer', pose: 'crouch', held: { kind: 'spear', tilt: 1.3 }, seed: 12 }), 3.6, 13.6, { ground: L.at, yaw: 3.0 });
  put(P, person({ kind: 'youth', skin: 'skin2', ...fur, pose: 'throw', held: { kind: 'spear', tilt: 0.4 }, seed: 13 }), 5.6, 11.2, { ground: L.at, yaw: 2.7 });
  put(P, person({ kind: 'woman', skin: 'skin1', ...fur, topMat: 'fur', pose: 'walkA', held: { kind: 'spear', tilt: 0.3 }, seed: 14 }), 8.5, 3.5, { ground: L.at, yaw: -2.6 });
  // in front, an earlier kill: a reindeer down, two hunters at work on it, a third bringing a sled-load
  put(P, animal({ species: 'reindeer', pose: 'lie', seed: 77 }), 2.2, 20.5, { ground: L.at, yaw: 1.3 });
  put(P, person({ kind: 'man', skin: 'skin3', ...fur, topMat: 'reindeer', pose: 'kneel', seed: 15 }), 3.4, 21.3, { ground: L.at, yaw: -2.2 });
  put(P, person({ kind: 'woman', skin: 'skin2', ...fur, pose: 'scrape', seed: 16 }), 1.0, 21.6, { ground: L.at, yaw: 2.4 });
  put(P, person({ kind: 'youth', skin: 'skin1', ...fur, topMat: 'furdark', pose: 'carry', held: 'meat', seed: 17 }), 5.4, 23.8, { ground: L.at, yaw: 0.9 });
  const blood = new Cards();
  for (let i = 0; i < 9; i++) blood.add({ c: [2.2 + r.range(-0.6, 0.8), L.at(2.2, 20.5) + 0.01, 20.5 + r.range(-0.2, 0.9)], w: K.mpp * r.range(2, 4), h: K.mpp * r.range(1.5, 3), tile: TILE.DOT, mat: 'berry', bias: -1, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, n: [0, 1, 0] });
  P.addCards(blood);
  const fg = new Solid();
  fg.add(boulder({ seed: 991, size: [2.2, 1.3, 1.8], detail: 1, mat: 'rock', moss: 0.65, mossMat: 'snow' }), M4.T(-4.5, L.at(-4.5, 25) - 0.2, 25));
  P.addSolid(fg);
  put(P, broadleaf({ seed: 992, h: 7, season: 'bare' }), 9, 27, { ground: L.at });
  // a spear in flight
  const sp = new Solid();
  sp.beam([5.0, L.at(5.0, 6.5) + 2.4, 6.5], [6.0, L.at(6.0, 4.6) + 2.6, 4.6], 0.02, 0.018, 4, { mat: 'wood', obj: newObj() });
  P.addSolid(sp);
  // tracks in the snow from the hunters
  const tracks = new Cards();
  for (let i = 0; i < 40; i++) {
    const t = i / 40, x = 8.5 + (1.4 - 8.5) * t + Math.sin(i) * 0.15, z = 3.5 + (13.2 - 3.5) * t + 1.5 * Math.sin(t * 3);
    tracks.add({ c: [x, L.at(x, z) + 0.01, z], w: K.mpp * 3, h: K.mpp * 2, tile: TILE.DOT, mat: 'snow', bias: -2, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, n: [0, 1, 0], upright: 0 });
  }
  P.addCards(tracks);
  for (const [x, y, z, w] of [[-8, 9, -6, 0.3], [-7, 9.5, -7.5, 0.8]]) put(P, bird({ kind: 'crow', fly: true, wing: w }), x, z, { y, yaw: 0.6 });
  return P.render(4);
}
