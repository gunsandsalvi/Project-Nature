// Age of first flakes: a band on a gravel bar in a river valley, morning mist on the water. They knap the first
// sharp flakes from river cobbles; children play in the shallows; across the river, red deer come down to drink.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, fbm, noise2, M4, newObj } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder, stoneBlock } from '../kit/land.js';
import { broadleaf, conifer, bush, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { windbreak, smoke } from '../kit/things.js';
import { land, put, riverSheet, lineField } from '../kit/scene.js';

export default async function ({ light, opts }) {
  const wide = opts.includes('wide');
  K.mpp = 0.04;
  const P = new Painter({ w: wide ? 748 : 336, h: wide ? 336 : 748, mpp: K.mpp, yaw: 22, elev: 30, target: [0, 0, 0], mood: light, bounds: { c: [0, 0, 0], r: 52 } });
  P.haze = [P.camDepth + 6, P.camDepth + 48];
  const r = new Rand(11);

  // a meandering river from the far valley toward us
  const centre = (t) => [7 * Math.sin(t * Math.PI * 2 * 1.25 + 0.785) + 3.2, -52 + 104 * t];
  const width = (t) => 5.2 + 1.6 * Math.abs(Math.sin(t * Math.PI * 2 * 1.25 + 0.9));
  const near = lineField(centre);
  const water = -0.62;
  const info = new Map();
  const field = (x, z) => { const k = x.toFixed(2) + ',' + z.toFixed(2); let v = info.get(k); if (!v) { v = near(x, z); info.set(k, v); } return v; };
  const bar = (f) => f.side === f.bend && f.t > 0.445 && f.t < 0.565 ? 1 : 0;
  const height = (x, z) => {
    const f = field(x, z), w = width(f.t) / 2;
    const base = 0.5 * (fbm(x * 0.05 + 4, z * 0.05 + 1, 3) - 0.5) + 0.012 * Math.abs(x) * 2;
    if (f.d < w) return water - 0.35 - 0.6 * (1 - f.d / w);
    const b = bar(f);
    const barLow = water + 0.12 + (f.d - w) * 0.04;
    const bank = Math.min(1, (f.d - w) / 1.6);
    const y = water + (base - water) * bank ** 0.6;
    return b > 0 && f.d < w + 6 ? Math.min(y, barLow + (y - barLow) * Math.max(0, (f.d - w - 3.6) / 2.2)) * 1 : y;
  };
  const L = land(height);
  P.addSolid(ground({
    x0: -34, x1: 34, z0: -56, z1: 50, step: 0.5, height, weights: (x, z, y) => {
      const f = field(x, z), w = width(f.t) / 2;
      const isBar = bar(f) > 0 && f.d < w + 5 && y < water + 0.4;
      const mud = f.d < w + 0.9 && !isBar;
      return [isBar || mud ? 0 : 1, isBar ? 1 : 0, mud ? 1 : 0, 0];
    },
  }), [['meadow', PAT.GROUND], ['sand', PAT.SAND], ['mud', PAT.DIRT], ['grass', PAT.GROUND]]);
  P.addWater(riverSheet({ centre, width: (t) => width(t) + 1.2, level: () => water, n: 260 }));
  P.setMist(water, 1.1, 0.7, 0.07, new THREE.Color('#e6e8f0'));

  // cobbles on the gravel bar, boulders in and by the river
  const stones = new Solid();
  for (let i = 0; i < 2400; i++) {
    const x = r.range(-14, 18), z = r.range(-16, 18), f = field(x, z), w = width(f.t) / 2;
    if (!(bar(f) > 0 && f.d > w - 0.3 && f.d < w + 4.2)) continue;
    const s = r.range(0.06, 0.24);
    stones.add(boulder({ seed: 1000 + i, size: [s * 1.3, s * 0.8, s], detail: 0, mat: r.pick(['rock', 'flint', 'lime', 'rock']), moss: 0 }), M4.T(x, L.at(x, z) - s * 0.15, z));
  }
  for (let i = 0; i < 26; i++) {
    const t = r.range(0.05, 0.95), [cx, cz] = centre(t), side = r.chance(0.5) ? 1 : -1, off = width(t) / 2 + r.range(-1.5, 1.5);
    const s = r.range(0.4, 1.3);
    stones.add(boulder({ seed: 1100 + i, size: [s * 1.4, s, s * 1.2], detail: 1, mat: r.pick(['rock', 'lime']), moss: 0.5 }), M4.T(cx + side * off, Math.max(water - 0.3, L.at(cx + side * off, cz)) - s * 0.25, cz));
  }
  P.addSolid(stones);

  // the woods: spring trees and birches on both banks, thicker far away
  for (let i = 0; i < 70; i++) {
    const x = r.range(-34, 34), z = r.range(-56, 48), f = field(x, z);
    if (f.d < width(f.t) / 2 + (bar(f) > 0 ? 8 : 3)) continue;
    if (z > -12 && Math.abs(x) < 9 && r.chance(0.7)) continue;
    if (Math.hypot(x - 2, z - 2) < 11) continue;
    const far = z < -20;
    const t = r.chance(far ? 0.25 : 0.1) ? conifer({ seed: 2000 + i, h: r.range(9, 14) }) : broadleaf({ seed: 2000 + i, h: r.range(6, 11), season: r.chance(0.3) ? 'spring' : 'summer', leafMat: r.chance(0.3) ? 'leafsp' : undefined });
    put(P, t, x, z, { ground: L.at });
  }
  for (let i = 0; i < 40; i++) {
    const x = r.range(-30, 30), z = r.range(-50, 46), f = field(x, z);
    if (f.d < width(f.t) / 2 + 2) continue;
    put(P, bush({ seed: 2200 + i, size: r.range(0.7, 1.5), flowers: r.chance(0.3) ? 'flowerw' : null }), x, z, { ground: L.at });
  }

  // reeds, sedge and grass
  const low = new Cards();
  const bankOk = (x, z) => { const f = field(x, z), w = width(f.t) / 2; return f.d > w - 0.6 && f.d < w + 1.8 && !(bar(f) > 0); };
  scatter(low, { r, area: [-30, 30, -54, 48], count: 900, tiles: [TILE.REED, TILE.SEDGE, TILE.REED], mat: 'reed', place: L.place, accept: bankOk });
  const meadowOk = (x, z) => { const f = field(x, z); return f.d > width(f.t) / 2 + (bar(f) > 0 ? 6 : 1.8); };
  scatter(low, { r, area: [-34, 34, -56, 50], count: 1700, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: 1, accept: meadowOk });
  scatter(low, { r, area: [-34, 34, -56, 50], count: 260, tiles: TILE.TUFT, mat: 'meadow', place: L.place, bias: -1, accept: meadowOk });
  scatter(low, { r, area: [-34, 34, -56, 50], count: 420, tiles: TILE.FLOWER, mat: (x, z) => (noise2(x * 0.15, z * 0.15) > 0.55 ? 'flowerw' : (noise2(x * 0.1 + 9, z * 0.1) > 0.5 ? 'flowerp' : 'flowery')), place: L.place, accept: meadowOk });
  scatter(low, { r, area: [-34, 34, -56, 50], count: 120, tiles: [TILE.FERN, TILE.HERB], mat: 'leaf', place: L.place, bias: 1, accept: meadowOk });
  P.addCards(low);

  // the band on the gravel bar
  const barPoint = (t, out) => {
    const [cx, cz] = centre(t), [x2, z2] = centre(t + 0.002), dx = x2 - cx, dz = z2 - cz, Lt = Math.hypot(dx, dz);
    const f = near(cx + (-dz / Lt), cz + (dx / Lt));
    const side = f.bend;
    return [cx + side * (-dz / Lt) * (width(t) / 2 + out), cz + side * (dx / Lt) * (width(t) / 2 + out)];
  };
  const folk = (spec, t, out, yaw) => { const [x, z] = barPoint(t, out); put(P, person(spec), x, z, { ground: L.at, yaw }); return [x, z]; };
  const nude = { top: 'none', legs: 'bare', legMat: 'leather', feet: null };
  folk({ kind: 'man', skin: 'skin2', hairStyle: 'long', beard: true, ...nude, necklace: 'shell', pose: 'knap', held: 'stone', seed: 1 }, 0.468, 2.2, 0.5);
  folk({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', ...nude, legMat: 'hide', paint: 'dyedred', pose: 'knap', held: 'stone', seed: 2 }, 0.49, 3.4, -2.3);
  folk({ kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'long', ...nude, headband: 'dyedred', pose: 'sitFloor', seed: 3 }, 0.505, 2.0, 2.4);
  folk({ kind: 'youth', skin: 'skin1', hair: 'hairred', hairStyle: 'short', ...nude, paint: 'ochre', pose: 'stoop', seed: 4 }, 0.45, 0.6, 1.2);
  folk({ kind: 'woman', skin: 'skin3', hairStyle: 'long', ...nude, legMat: 'hide', necklace: 'bone', pose: 'walkA', held: 'basket', seed: 5 }, 0.535, 3.6, -0.4);
  folk({ kind: 'child', skin: 'skin2', hairStyle: 'short', ...nude, pose: 'armsUp', seed: 6 }, 0.515, -0.8, 0.8);
  folk({ kind: 'child', skin: 'skin1', hairStyle: 'long', ...nude, pose: 'stoop', seed: 7 }, 0.525, -1.4, -1.8);
  folk({ kind: 'man', skin: 'skin3', hairStyle: 'short', ...nude, paint: 'charcoal', pose: 'aim', held: { kind: 'spear', tilt: 0.9 }, seed: 8 }, 0.6, -1.4, -2.6);
  folk({ kind: 'man', skin: 'skin1', hairStyle: 'topknot', ...nude, pose: 'walkB', held: 'stone', seed: 9 }, 0.475, 4.6, 2.9);
  // a heap of cobbles brought up to work
  const heap = new Solid();
  for (let i = 0; i < 26; i++) {
    const [x, z] = barPoint(0.478 + r.range(-0.004, 0.004), 2.9 + r.range(-0.5, 0.5));
    const s = r.range(0.1, 0.2);
    heap.add(boulder({ seed: 3500 + i, size: [s * 1.3, s * 0.9, s], detail: 0, mat: r.pick(['flint', 'rock', 'lime']), moss: 0 }), M4.T(x, L.at(x, z) + r.range(0, 0.12), z));
  }
  P.addSolid(heap);
  // flakes and cores scattered where they work
  const flakes = new Solid();
  for (let i = 0; i < 40; i++) {
    const [x, z] = barPoint(0.475 + r.range(-0.012, 0.012), 2.8 + r.range(-0.9, 0.9));
    flakes.add(boulder({ seed: 3000 + i, size: [0.08, 0.025, 0.06], detail: 0, mat: 'flint', moss: 0 }), M4.T(x, L.at(x, z), z));
  }
  P.addSolid(flakes);

  // red deer across the river, a heron in the shallows, ducks
  for (const [t, out, sp, pose, yaw] of [[0.36, 3.5, 'stag', 'alert', 2.2], [0.37, 1.2, 'hind', 'graze', 1.4], [0.345, 2.0, 'hind', 'graze', 2.9], [0.355, -0.5, 'hind', 'stand', 1.9], [0.33, 4.4, 'hind', 'walk', 2.6]]) {
    const [cx, cz] = centre(t), f = near(cx + 1, cz);
    const [x2, z2] = centre(t + 0.002), dx = x2 - cx, dz = z2 - cz, Lt = Math.hypot(dx, dz), side = -f.bend;
    const x = cx + side * (-dz / Lt) * (width(t) / 2 + out), z = cz + side * (dx / Lt) * (width(t) / 2 + out);
    put(P, animal({ species: sp, pose, seed: Math.round(t * 1000) }), x, z, { y: Math.max(L.at(x, z), water - 0.25), yaw });
  }
  const [hx, hz] = centre(0.66);
  put(P, bird({ kind: 'heron' }), hx + 1.8, hz, { y: water - 0.25, yaw: -0.8 });
  for (const [t, dx] of [[0.72, -0.8], [0.725, 0.2], [0.73, -0.2]]) { const [x, z] = centre(t); put(P, bird({ kind: 'duck' }), x + dx, z, { y: water - 0.12, yaw: 2.5 }); }
  for (const [x, y, z, w] of [[-6, 7, -18, 0.2], [-4.5, 7.6, -19, 0.8], [-5.2, 6.8, -16.5, 0.5]]) put(P, bird({ kind: 'crow', fly: true, wing: w }), x, z, { y, yaw: 1.2 });

  return P.render(4);
}
