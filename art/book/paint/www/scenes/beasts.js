// A line of animals, to judge the body patterns. opts: "close" draws them twice as big.
import { Painter } from '../engine.js';
import { Solid, PAT, M4 } from '../geo.js';
import { K } from '../kit/k.js';
import { ground } from '../kit/land.js';
import { animal, bird } from '../kit/animals.js';

export default async function ({ light, opts }) {
  const close = opts.includes('close');
  K.mpp = close ? 0.03 : 0.06;
  const P = new Painter({ w: close ? 520 : 336, h: close ? 300 : 200, mpp: K.mpp, yaw: 25, elev: 30, target: [0, 0.6, 0.5], mood: light, bounds: { c: [0, 0, 0], r: 14 } });
  P.addSolid(ground({ x0: -16, x1: 16, z0: -16, z1: 16, step: 1, height: () => 0, weights: () => [1, 0, 0, 0] }), [['meadow', PAT.GROUND]]);
  const list = [['stag', 'alert'], ['hind', 'graze'], ['reindeer', 'walk'], ['horse', 'stand'], ['bison', 'stand'], ['aurochs', 'walk'], ['goat', 'graze'], ['boar', 'stand'], ['wolf', 'run'], ['dog', 'stand'], ['hare', 'stand'], ['hind', 'lie']];
  const all = new Solid();
  list.forEach(([sp, pose], i) => {
    const a = animal({ species: sp, pose, seed: i + 1 });
    const x = ((i % 6) - 2.5) * 2.4, z = Math.floor(i / 6) * 3 - 1.5;
    all.add(a.solid, M4.mul(M4.T(x, 0, z), M4.R(0, 1.1 + (i % 2) * 0.6, 0)));
  });
  for (const [k, x, fly] of [['crow', -5, false], ['heron', -3.5, false], ['duck', -2, false], ['gull', 0, true], ['vulture', 2.5, true], ['goose', 5, false]]) {
    const b = bird({ kind: k, fly });
    all.add(b.solid, M4.mul(M4.T(x, fly ? 1.5 : 0, 4.2), M4.R(0, 0.8, 0)));
  }
  P.addSolid(all);
  return P.render(close ? 3 : 4);
}
