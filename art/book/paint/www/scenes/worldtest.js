// A quick flat view of the generated world, for tuning the generator: cover colours, shaded hills, rivers, the start.
import { canvasPngs } from '../engine.js';
import { RAMPS } from '../palette.js';
import { worldGrid, findStart, COVER, BIOME } from '../worldgen.js';

export default async function ({ opts }) {
  const seed = Number(opts.find((o) => o.startsWith('seed'))?.slice(4) || 1);
  const t0 = performance.now();
  const G = worldGrid({ w: 1000, h: 500, seed });
  const S = findStart(G);
  console.log('world', ((performance.now() - t0) / 1000).toFixed(1), 's; start', S.xk.toFixed(0), S.yk.toFixed(0));
  const c = document.createElement('canvas'); c.width = G.w; c.height = G.h;
  const g = c.getContext('2d'), img = g.createImageData(G.w, G.h);
  for (let j = 0; j < G.h; j++) for (let i = 0; i < G.w; i++) {
    const k = j * G.w + i;
    const e = G.hgt[k], ex = G.hgt[j * G.w + ((i + 1) % G.w)], ey = G.hgt[((j + 1) % G.h) * G.w + i];
    const shade = Math.max(-2, Math.min(1, Math.round(-((ex - e) + (ey - e)) / 160)));
    let col;
    if (G.biome[k] === BIOME.SEA) col = RAMPS.sea[e > -120 ? 5 : e > -1200 ? 4 : 3];
    else {
      const [m, s] = COVER[G.biome[k]];
      col = RAMPS[m][Math.max(0, Math.min(6, 4 + s + shade))];
    }
    if (G.hgt[k] > 0 && G.acc[k] >= 1000 && G.biome[k] !== BIOME.ICE) col = RAMPS.water[4];
    const v = parseInt(col.slice(1), 16);
    img.data.set([v >> 16, (v >> 8) & 255, v & 255, 255], k * 4);
  }
  g.putImageData(img, 0, 0);
  const sx = Math.floor(S.xk / G.cx), sy = Math.floor(S.yk / G.cy);
  g.fillStyle = '#ff2050'; g.fillRect(sx - 3, sy - 3, 7, 7);
  return canvasPngs(c, 1);
}
