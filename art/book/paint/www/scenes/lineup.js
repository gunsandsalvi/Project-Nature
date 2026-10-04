// A line of people and poses on plain ground, to judge the figures. opts: "close" draws them twice as big.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, PAT, M4 } from '../geo.js';
import { K } from '../kit/k.js';
import { ground } from '../kit/land.js';
import { person } from '../kit/people.js';

export default async function ({ light, opts }) {
  K.mpp = opts.includes('close') ? 0.03 : 0.06;
  const W = opts.includes('close') ? 400 : 336, H = opts.includes('close') ? 300 : 200;
  const P = new Painter({ w: W, h: H, mpp: K.mpp, yaw: 25, elev: 30, target: [0, 0.6, 0], mood: light, bounds: { c: [0, 0, 0], r: 12 } });
  P.addSolid(ground({ x0: -14, x1: 14, z0: -14, z1: 14, step: 1, height: () => 0, weights: () => [1, 0, 0, 0] }), [['meadow', PAT.GROUND]]);
  const folk = [
    { kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'hide', pose: 'hold', held: { kind: 'spear' } },
    { kind: 'woman', skin: 'skin1', hairStyle: 'braid', top: 'dress', topMat: 'leather', necklace: 'shell', pose: 'walkA', held: 'basket' },
    { kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'fur', pose: 'stand', held: 'staff' },
    { kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', legMat: 'leather', feet: null, pose: 'walkB' },
    { kind: 'youth', skin: 'skin1', hair: 'hairred', hairStyle: 'topknot', top: 'none', legs: 'leggings', paint: 'ochre', pose: 'throw', held: 'spear' },
    { kind: 'man', skin: 'skin3', hairStyle: 'hood', top: 'parka', topMat: 'fur', legMat: 'furdark', feet: 'furdark', pose: 'carry', held: 'meat' },
    { kind: 'woman', skin: 'skin2', hairStyle: 'bun', top: 'tunic', topMat: 'dyedred', legMat: 'leather', pose: 'knap', held: 'stone' },
    { kind: 'woman', skin: 'skin3', hairStyle: 'long', top: 'dress', topMat: 'hide', headband: 'ochre', pose: 'kneel' },
  ];
  const all = new Solid();
  folk.forEach((f, i) => {
    const p = person({ ...f, seed: i + 1 });
    const x = (i - (folk.length - 1) / 2) * 1.1;
    all.add(p.solid, M4.mul(M4.T(x, 0, (i % 2) * 0.8), M4.R(0, 0.35 - (i % 3) * 0.3, 0)));
  });
  P.addSolid(all);
  return P.render(opts.includes('close') ? 3 : 4);
}
