// A quick flat view of the generated land, for tuning the generator (worldgen.js): cover colours under light from the
// upper left, lakes and rivers in blue, one pixel a cell, and the start. opts: seedN, and region for the start
// region in place of the whole world.
import { canvasPngs } from '../engine.js';
import { RAMPS } from '../palette.js';
import { worldGrid, findStart, regionGrid, COVER, BIOME } from '../worldgen.js';

export default async function ({ opts }) {
  const seed = Number(opts.find((o) => o.startsWith('seed'))?.slice(4) || 7);
  const t0 = performance.now();
  const W = worldGrid({ w: 1000, h: 500, seed }), S = findStart(W);
  const G = opts.includes('region') ? regionGrid({ xk: S.xk, yk: S.yk, size: 256, cell: 0.5, seed }) : W;
  console.log('grid', G.w, 'x', G.h, ((performance.now() - t0) / 1000).toFixed(1), 's; start', S.xk.toFixed(0), S.yk.toFixed(0));
  const c = document.createElement('canvas'); c.width = G.w; c.height = G.h;
  const g = c.getContext('2d'), img = g.createImageData(G.w, G.h);
  // the world wraps east to west; a region stops at its edge
  const I = (i) => (G === W ? (i + G.w) % G.w : Math.max(0, Math.min(G.w - 1, i)));
  const H = (i, j) => G.hgt[Math.max(0, Math.min(G.h - 1, j)) * G.w + I(i)];
  const cellM = (G.cell || G.cx) * 1000;
  for (let j = 0; j < G.h; j++) for (let i = 0; i < G.w; i++) {
    const k = j * G.w + i, e = G.hgt[k];
    // the slope toward the light, in metres per metre
    const sx = (H(i + 1, j) - H(i - 1, j)) / (2 * cellM), sy = (H(i, j + 1) - H(i, j - 1)) / (2 * cellM);
    const shade = Math.max(-3, Math.min(2, Math.round(-(sx + sy) * 6)));
    let col;
    if (e <= 0) col = RAMPS.sea[e > -120 ? 5 : e > -1200 ? 4 : 3];
    else if (G.fill[k] - e > 3) col = RAMPS.water[4];
    else {
      const [m, s] = COVER[G.biome[k]];
      col = RAMPS[m][Math.max(0, Math.min(6, 4 + s + shade))];
    }
    if (e > 0 && G.acc[k] >= (G === W ? 1000 : 40) && G.biome[k] !== BIOME.ICE) col = RAMPS.water[4];
    const v = parseInt(col.slice(1), 16);
    img.data.set([v >> 16, (v >> 8) & 255, v & 255, 255], k * 4);
  }
  g.putImageData(img, 0, 0);
  if (G === W) {
    const sx = Math.floor(S.xk / W.cx), sy = Math.floor(S.yk / W.cy);
    g.fillStyle = '#ff2050'; g.fillRect(sx - 3, sy - 3, 7, 7);
  }
  return canvasPngs(c, 1);
}
