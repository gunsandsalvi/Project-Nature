// A style frame: a meadow slope with stones, a tree, a bush and a few figures in boxes, to tune the look.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, PAT, FLAG, Rand, fbm, newObj, M4 } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder } from '../kit/land.js';
import { broadleaf, bush, scatter } from '../kit/plants.js';

export default async function ({ light }) {
  K.mpp = 0.06;
  const P = new Painter({ w: 320, h: 240, mpp: K.mpp, yaw: 35, elev: 30, target: [0, 0.5, 0], mood: light, bounds: { c: [0, 0, 0], r: 30 } });
  const height = (x, z) => 0.6 * fbm(x * 0.06 + 3, z * 0.06 + 1, 3) * 3 - 0.9 + (-z) * 0.06 + 0.25 * Math.sin(x * 0.2);
  const place = (x, z) => {
    const e = 0.2, y = height(x, z);
    const n = new THREE.Vector3(-(height(x + e, z) - height(x - e, z)) / (2 * e), 1, -(height(x, z + e) - height(x, z - e)) / (2 * e)).normalize();
    return [y, n.toArray()];
  };
  P.addSolid(ground({ x0: -30, x1: 30, z0: -30, z1: 30, step: 0.5, height, weights: (x, z) => {
    const path = Math.abs(x * 0.8 + z * 0.4 - 2 + Math.sin(z * 0.4) * 0.8) < 0.7 ? 1 : 0;
    return [1 - path, path, 0, 0];
  } }), [['meadow', PAT.GROUND], ['dirt', PAT.DIRT]]);

  const r = new Rand(5);
  const put = (solid, x, z, rot = 0, sink = 0) => {
    const [y] = place(x, z);
    const m = M4.mul(M4.T(x, y - sink, z), M4.R(0, rot, 0));
    const s = new Solid(); s.add(solid, m); return s;
  };
  // stones
  const rocks = new Solid();
  rocks.add(boulder({ seed: 3, size: [2.4, 1.6, 1.9] }), M4.T(-3, place(-3, -1)[0] - 0.1, -1));
  rocks.add(boulder({ seed: 7, size: [1.2, 0.8, 1.0] }), M4.T(-1.4, place(-1.4, 0.4)[0] - 0.05, 0.4));
  rocks.add(boulder({ seed: 9, size: [0.5, 0.35, 0.45], detail: 0 }), M4.T(-0.6, place(-0.6, 1.2)[0], 1.2));
  rocks.add(boulder({ seed: 12, size: [3.2, 2.4, 2.6], detail: 2 }), M4.T(5, place(5, -5)[0] - 0.2, -5));
  P.addSolid(rocks);

  // a tree and bushes
  const t = broadleaf({ seed: 4, h: 8 });
  const ty = place(2.5, -3)[0];
  const ts = new Solid(); ts.add(t.solid, M4.T(2.5, ty, -3)); P.addSolid(ts);
  const tc = new Cards();
  for (let i = 0; i < t.cards.n * 4; i++) {}
  P.addCards(shift(t.cards, [2.5, ty, -3]));
  const b = bush({ seed: 2, size: 1.3, berries: 'berry' });
  const by = place(-4.5, 2)[0];
  const bs = new Solid(); bs.add(b.solid, M4.T(-4.5, by, 2)); P.addSolid(bs);
  P.addCards(shift(b.cards, [-4.5, by, 2]));

  // tufts and flowers
  const low = new Cards();
  scatter(low, { r, area: [-12, 12, -12, 12], count: 110, tiles: TILE.TUFT, mat: 'meadow', place, bias: 1 });
  scatter(low, { r, area: [-12, 12, -12, 12], count: 25, tiles: TILE.TUFT, mat: 'meadow', place, bias: -1 });
  scatter(low, { r, area: [-12, 12, -12, 12], count: 70, tiles: TILE.FLOWER, mat: 'flowerp', place, bias: 0 });
  scatter(low, { r, area: [-12, 12, -12, 12], count: 30, tiles: TILE.FLOWER, mat: 'flowery', place, bias: 0 });
  P.addCards(low);

  // stand-in figures and a tent
  const figs = new Solid();
  for (const [x, z, c] of [[0.5, 1.5, 'hide'], [1.4, 2.2, 'dyedred'], [-0.2, 3.0, 'leather']]) {
    const y = place(x, z)[0], o = newObj();
    figs.box(M4.T(x, y + 0.45, z), 0.16, 0.9, 0.16, { mat: c, obj: o });
    figs.box(M4.T(x, y + 1.15, z), 0.42, 0.55, 0.24, { mat: c, obj: o });
    figs.box(M4.T(x, y + 1.6, z), 0.26, 0.3, 0.26, { mat: 'skin1', obj: o, pat: PAT.FACE, norm: true });
  }
  P.addSolid(figs);

  const tent = new Solid(), to = newObj(), [tyy] = place(-2.5, 4.5);
  const n = 9, R = 1.8, H = 3.2;
  for (let i = 0; i < n; i++) {
    const a0 = (i / n) * Math.PI * 2, a1 = ((i + 1) / n) * Math.PI * 2;
    const p0 = [-2.5 + Math.cos(a0) * R, tyy - 0.1, 4.5 + Math.sin(a0) * R], p1 = [-2.5 + Math.cos(a1) * R, tyy - 0.1, 4.5 + Math.sin(a1) * R];
    const top = [-2.5, tyy + H, 4.5];
    tent.tri(p0, top, p1, { mat: 'hide', pat: PAT.HIDE, obj: to }, [[a0 * R, 0, 0], [(a0 + a1) / 2 * R, H, 0], [a1 * R, 0, 0]]);
  }
  for (let i = 0; i < 5; i++) {
    const a = (i / 5) * Math.PI * 2 + 0.3;
    tent.beam([-2.5 + Math.cos(a) * R * 0.95, tyy, 4.5 + Math.sin(a) * R * 0.95], [-2.5 - Math.cos(a) * 0.35, tyy + H + 0.6, 4.5 - Math.sin(a) * 0.35], 0.045, 0.035, 4, { mat: 'wood', pat: PAT.WOOD, obj: to });
  }
  P.addSolid(tent);

  return P.render(4);
}

function shift(cards, d) {
  const c = cards.d.aCenter;
  for (let i = 0; i < c.length; i += 3) { c[i] += d[0]; c[i + 1] += d[1]; c[i + 2] += d[2]; }
  return cards;
}
