// The zoom stops (PRE-03): one place seen from one person chipping flint out to the globe, each stop at noon or
// dusk (PRE-31). The far stops come from the generated world (worldgen.js): the valley's coarse ground under its
// cover, the region's and the world's cells in flat colours with shaded hills (PRE-29), and the globe (WLD-02);
// people, groups, herds and camps show at every distance (PRE-28).
// opts: the stop (person, closecamp, camp, valley, region, map, globe), and "land" for a landscape picture.
import * as THREE from 'three';
import { Painter, cellTexture, canvasPngs } from '../engine.js';
import { PAT, fbm } from '../geo.js';
import { RAMPS, MOODS } from '../palette.js';
import { riverLines, smoothLine, latitude, WORLD, BIOME, COVER } from '../worldgen.js';
import { groundView, globe, fireGlow, groupMarker, herdMarker, riverRibbons } from '../kit/far.js';
import { waterPlane } from '../kit/scene.js';
import { zoomWorld, zoomLand, blurred, gridHeight, regionCamps, toWorld, fromWorld } from '../kit/zoomworld.js';
import { campStop, nearStop } from './zoomnear.js';

const wrapK = (v, p) => ((v % p) + p) % p;
// from the valley out a day passes in under a second, so the light holds steady from high up and only its tint
// follows the hour (PRE-30), and the map look is always lit so (PRE-29)
const steadyMood = (light) => (light === 'dusk' ? { ...MOODS.dusk, el: 45, sunK: 0.3 } : { ...MOODS.noon, el: 45 });

/** The valley (about 10 km): coarse ground under its cover, forest as raised canopy, rivers at least a pixel wide. */
async function valleyStop({ light, w, h }) {
  const Lnd = zoomLand(), W = Lnd.W, H0 = Lnd.H0;
  const P = new Painter({ w, h, mpp: 30, yaw: 16, elev: 46, target: [0, H0, 0], mood: steadyMood(light), depth: 90000,
    bounds: { c: [0, H0, -2000], r: 24000 }, shadowSize: 4096 });
  P.haze = [P.camDepth + 1500, P.camDepth + 26000];
  P.jitter = 30;
  P.waterStyle = 5; P.mapDepth = [0.8, 2.5, 30];
  // the relief three times its height, so a valley 10 km across still reads; forest as a raised canopy
  // woods stand back from the water, so every river shows (PRE-26); their canopy is raised three times too, so a
  // wood stands up from the meadow with an edge
  const ex = 3, woods = (u, v) => { const r = Lnd.nearRiver(u, v); return Lnd.cover(u, v).forest > 0.5 && !(r && r.d < Math.max(r.hw, 18) + 30); };
  const canopy = (u, v) => (woods(u, v) ? 28 : 0);
  const ground = groundView(P, {
    height: (u, v) => H0 + (Lnd.height(u, v, false, 18, true) - H0) * ex + canopy(u, v),
    weights: (u, v, y, slope) => {
      // open sun-facing slopes dry to gold
      const c = Lnd.cover(u, v), w = woods(u, v) ? 1 : 0, rock = Math.max(c.rock, slope > 0.5 ? 1 : 0), dry = Math.max(c.dry, c.sunny * 0.95);
      return [w, (1 - w) * (1 - dry), (1 - w) * dry, rock * 1.5];
    },
    step: 30, stepD: 34, ymin: H0 - 800, ymax: H0 + 1600, smoothN: 3,
  });
  P.addSolid(ground, [['pine', PAT.NONE], ['meadow', PAT.NONE], ['drygrass', PAT.NONE], ['rock', PAT.NONE]]);
  // every river at least a pixel wide, the big ones two (PRE-26)
  const wide = (pts) => pts.map((p) => [...p.slice(0, 4), Math.max(p[4], p[2] >= 500 ? 62 : 34)]);
  P.addWater(riverRibbons(Lnd.lines.map(wide), Lnd.lines.map((pts) => Lnd.waterLevel(pts).map((L) => H0 + (L - H0) * ex)), 34));
  P.addWater(waterPlane({ x0: -60000, x1: 60000, z0: -60000, z1: 60000, y: 0 }), 1);
  const cv = await P.render(0), ctx = cv.getContext('2d');
  const mark = (u, v) => P.toScreen([u, H0 + (Lnd.height(u, v) - H0) * ex, v]);
  for (const c of regionCamps(W)) {
    const [u, v] = fromWorld(W, c.xk, c.yk), [sx, sy] = mark(u, v);
    if (sx < -5 || sy < -5 || sx > w + 5 || sy > h + 5) continue;
    fireGlow(ctx, sx, sy, { big: light === 'dusk' });
    groupMarker(ctx, sx, sy - 3, { colour: c.colour });
  }
  for (const [u, v, kind, coat] of [[2100, -1300, 'deer', RAMPS.deer[4]], [-2800, 520, 'horse', RAMPS.dun[4]], [3600, 1400, 'bison', RAMPS.bison[3]]]) {
    const [sx, sy] = mark(u, v); herdMarker(ctx, sx, sy, { kind, coat });
  }
  return cv;
}

/** The region's map (about 100 km): world cells in flat colours, shaded hills, lakes, rivers as lines; no markers. */
export async function regionMap({ light, w, h }) {
  const W = zoomWorld(), R = W.R, ex = 5, Rs = blurred(R, 1, false);
  const P = new Painter({ w, h, mpp: 300, yaw: W.yawW, elev: 66, target: [0, 0, 0], mood: steadyMood(light), depth: 400000,
    bounds: { c: [0, 0, 0], r: 180000 }, shadowSize: 4096 });
  P.haze = [P.camDepth + 1e6, P.camDepth + 2e6];
  P.waterStyle = 5; P.mapDepth = [30, 600, 4000]; P.mapFoam = 6;
  const lx = (xk) => (xk - W.camp.xk) * 1000, lz = (yk) => (yk - W.camp.yk) * 1000;
  const height = (x, z) => gridHeight(Rs, W.camp.xk + x / 1000, W.camp.yk + z / 1000) * ex;
  const ground = groundView(P, { height, step: 300, stepD: 330, ymin: -6000, ymax: 12000, pat: PAT.CELLS });
  // world cells of 1 km: two by two region cells
  const cells = cellTexture({ dims: [R.w / 2, R.h / 2], origin: [lx(R.x0), lz(R.y0)], size: 1000, at: (i, j) => {
    const k = 2 * j * R.w + 2 * i, b = R.biome[k];
    if (R.hgt[k] <= 0) return ['sand', 0];
    return R.fill[k] - R.hgt[k] > 6 ? ['mud', 0] : COVER[b];
  } });
  P.addSolid(ground, null, cells);
  P.addWater(waterPlane({ x0: -400000, x1: 400000, z0: -400000, z1: 400000, y: 0 }), 1);
  addLakes(P, R, Rs, lx, lz, ex);
  const cv = await P.render(0), ctx = cv.getContext('2d');
  // rivers draining about 1,000 km² or more, as lines a pixel wide, the biggest two (PRE-26)
  const lines = riverLines(R, 1000).map((l) => smoothLine(l.pts, 2).map(([x, y, a]) => [lx(x), height(lx(x), lz(y)) + 20, lz(y), a]));
  drawRivers(ctx, P, lines, light);
  return { cv, ctx, P, W, lx, lz, height };
}

/** The region stop: the map with the camps' fires and markers, and the herds (PRE-28). */
async function regionStop({ light, w, h }) {
  const { cv, ctx, P, W, lx, lz, height } = await regionMap({ light, w, h });
  for (const c of regionCamps(W)) {
    const [sx, sy] = P.toScreen([lx(c.xk), height(lx(c.xk), lz(c.yk)), lz(c.yk)]);
    fireGlow(ctx, sx, sy, { big: light === 'dusk' });
    groupMarker(ctx, sx, sy - 3, { colour: c.colour });
  }
  const herd = (dx, dy, kind, coat) => { const x = lx(W.camp.xk + dx), z = lz(W.camp.yk + dy); const [sx, sy] = P.toScreen([x, height(x, z), z]); herdMarker(ctx, sx, sy, { kind, coat }); };
  herd(9, -14, 'deer', RAMPS.deer[4]); herd(-22, -9, 'horse', RAMPS.dun[4]); herd(31, 11, 'bison', RAMPS.bison[3]);
  return cv;
}

function addLakes(P, R, Rs, lx, lz, ex) {
  const pos = [], fl = [], idx = [];
  for (let k = 0; k < R.n; k++) {
    if (R.hgt[k] <= 0 || R.fill[k] - R.hgt[k] < 6) continue;
    const x = R.x0 + (k % R.w) * R.cell, y = R.y0 + Math.floor(k / R.w) * R.cell, yy = Math.max(R.fill[k], Rs.hgt[k] + 4) * ex, b = pos.length / 3;
    for (const [a, c] of [[0, 0], [1, 0], [1, 1], [0, 1]]) { pos.push(lx(x + a * R.cell), yy, lz(y + c * R.cell)); fl.push(1, 0, 0); }
    idx.push(b, b + 2, b + 1, b, b + 3, b + 2);
  }
  if (!idx.length) return;
  const g = new THREE.BufferGeometry();
  g.setAttribute('position', new THREE.Float32BufferAttribute(pos, 3));
  g.setAttribute('aFlow', new THREE.Float32BufferAttribute(fl, 3));
  g.setIndex(idx);
  P.addWater(g);
}

/** River lines over the picture: a pixel wide, two for rivers draining more than 3,000 km². */
function drawRivers(ctx, P, lines, light) {
  ctx.fillStyle = light === 'dusk' ? RAMPS.water[4] : RAMPS.water[5];
  for (const pts of lines) {
    let prev = null;
    for (const p of pts) {
      const s = P.toScreen(p.slice(0, 3)), wide = (p[3] || 0) > 3000;
      if (prev) {
        const n = Math.max(Math.abs(s[0] - prev[0]), Math.abs(s[1] - prev[1]));
        for (let i = 0; i <= n; i++) {
          const x = Math.round(prev[0] + ((s[0] - prev[0]) * i) / (n || 1)), y = Math.round(prev[1] + ((s[1] - prev[1]) * i) / (n || 1));
          ctx.fillRect(x, y, 1, 1); if (wide) ctx.fillRect(x, y + 1, 1, 1);
        }
      }
      prev = s;
    }
  }
}

/** The world map: straight down over the whole world from pole to pole, cells in flat colours. */
async function mapStop({ light, w, h }) {
  const W = zoomWorld(), G = W.G, ex = 14, Gs = blurred(G, 1, true);
  const P = new Painter({ w, h, mpp: 1500, yaw: 0, elev: 90, target: [0, 0, 0], mood: steadyMood(light), depth: 600000,
    bounds: { c: [0, 0, 0], r: 900000 }, shadowSize: 4096 });
  P.haze = [P.camDepth + 1e7, P.camDepth + 2e7];
  P.waterStyle = 5; P.mapDepth = [60, 1200, 8000]; P.mapFoam = 90;
  // centred on the camp east-west and on the equator north-south, so both edges of the polar ice show
  const cx = W.camp.xk, cy = 500, lx = (xk) => (xk - cx) * 1000, lz = (yk) => (yk - cy) * 1000;
  const height = (x, z) => gridHeight(Gs, cx + x / 1000, cy + z / 1000, true) * ex;
  const ground = groundView(P, { height, step: 1500, stepD: 1500, ymin: -60000, ymax: 70000, margin: 0.04, pat: PAT.CELLS });
  const cells = cellTexture({ dims: [G.w, G.h], origin: [lx(0), lz(0)], size: G.cell * 1000, at: (i, j) => {
    const k = j * G.w + i, b = G.biome[k];
    if (b === BIOME.ICE) return ['ice', 1];
    if (G.hgt[k] <= 0) return ['sand', 0];
    return COVER[b];
  } });
  P.addSolid(ground, null, cells);
  P.addWater(waterPlane({ x0: -2e6, x1: 2e6, z0: -2e6, z1: 2e6, y: 0 }), 1);
  const cv = await P.render(0), ctx = cv.getContext('2d');
  const segs = [];
  for (let k = 0; k < G.n; k++) {
    if (G.acc[k] < 1000 || G.hgt[k] <= 0 || G.down[k] < 0 || G.biome[k] === BIOME.ICE) continue;
    const a = [((k % G.w) + 0.5) * G.cell, (Math.floor(k / G.w) + 0.5) * G.cell], d = G.down[k];
    const b = [((d % G.w) + 0.5) * G.cell, (Math.floor(d / G.w) + 0.5) * G.cell];
    if (Math.abs(a[0] - b[0]) > 100 || Math.abs(a[1] - b[1]) > 100) continue;   // across the wrap
    for (const off of [-WORLD.H, 0, WORLD.H]) segs.push([[lx(a[0]), 0, lz(a[1] + off), G.acc[k]], [lx(b[0]), 0, lz(b[1] + off), G.acc[k]]]);
  }
  drawRivers(ctx, P, segs, light);
  for (const c of regionCamps(W)) {
    const [sx, sy] = P.toScreen([lx(c.xk), 0, lz(c.yk)]);
    fireGlow(ctx, sx, sy, { big: light === 'dusk' });
    if (c.ours) groupMarker(ctx, sx, sy - 3, { colour: c.colour });
  }
  return cv;
}

const GLOBE_R = (WORLD.W / (2 * Math.PI)) * 1000;
const lonOf = (xk) => (xk / WORLD.W) * 360 - 180;
const unit = (lat, lon) => { const a = (lat * Math.PI) / 180, b = (lon * Math.PI) / 180; return [Math.cos(a) * Math.cos(b), Math.sin(a), Math.cos(a) * Math.sin(b)]; };
const spaceMood = (light) => ({ el: 30, az: 140, sun: new THREE.Color(light === 'dusk' ? '#ffcfb0' : '#fff6e2'), shade: new THREE.Color('#9aa6e8'), sunK: 0.42, skyK: 0.32, sunPow: 0.8, shift: 0,
  sky: new THREE.Color('#0b0e1f'), haze: new THREE.Color('#0b0e1f'), hazeK: 0, desat: 0, fire: new THREE.Color('#ffc070') });

/** A world's globe: every small face in its cell's flat colour (PRE-29); the polar ice hides the seam. */
function globeSolid(G, detail) {
  const cellOf = (lat, lon) => {
    const xk = wrapK(((lon + 180) / 360) * WORLD.W, WORLD.W), yk = wrapK((90 - lat) / 0.18, WORLD.H);
    return Math.floor(yk / G.cell) * G.w + Math.floor(xk / G.cell);
  };
  return globe({ R: GLOBE_R, detail, exag: 0, at: (lat, lon) => {
    const k = cellOf(lat, lon), e = G.hgt[k], b = G.biome[k];
    if (b === BIOME.ICE) return { e: Math.max(e, 0), mat: 'ice', bias: 1 };
    if (e <= 0) return { e, mat: 'sea', bias: e > -120 ? 1 : e > -1500 ? 0 : -1 };
    const [m, s] = COVER[b];
    return { e, mat: m, bias: s };
  } });
}

/** A small globe on a clear ground, lit from the upper left, seen from (lat, lon): the new-world screen's (WLD-10). */
export async function globeOf(G, size, { lat = 25, lon = 0 } = {}) {
  const view = unit(lat, lon), elev = (Math.asin(view[1]) * 180) / Math.PI, yaw = (Math.atan2(view[0], view[2]) * 180) / Math.PI;
  const east = [-Math.sin((lon * Math.PI) / 180), 0, Math.cos((lon * Math.PI) / 180)];
  const sunDir = view.map((v, i) => v * 0.75 + east[i] * 0.55 + (i === 1 ? 0.45 : 0));
  const P = new Painter({ w: size, h: size, mpp: (2.12 * GLOBE_R) / size, yaw, elev, target: [0, 0, 0], mood: spaceMood('noon'), sunDir, depth: GLOBE_R * 4,
    bounds: { c: [0, 0, 0], r: GLOBE_R * 1.2 }, shadowSize: 1024 });
  P.haze = [P.camDepth * 10, P.camDepth * 20];
  P.clear = true;
  P.addSolid(globeSolid(G, 40));
  return P.render(0);
}

/** The globe (WLD-02): the camps' fires show where the bands live; a thin bright rim of air round it. */
async function globeStop({ light, w, h }) {
  const W = zoomWorld(), G = W.G, R = GLOBE_R;
  const lat0 = latitude(W.camp.yk), lon0 = lonOf(W.camp.xk);
  // look at the camp from a little nearer the equator, so the ice at its pole shows
  const view = unit(lat0 * 0.55, lon0);
  const elev = (Math.asin(view[1]) * 180) / Math.PI, yaw = (Math.atan2(view[0], view[2]) * 180) / Math.PI;
  const east = [-Math.sin((lon0 * Math.PI) / 180), 0, Math.cos((lon0 * Math.PI) / 180)];
  // at top speed the light holds steady (PRE-30), from the upper left, so the shade falls in a crescent on the
  // lower right; dusk only tints it
  const sunDir = view.map((v, i) => v * 0.45 + east[i] * 0.65 + (i === 1 ? 0.6 : 0));
  const P = new Painter({ w, h, mpp: (2.3 * R) / w, yaw, elev, target: [0, 0, 0], mood: spaceMood(light), sunDir, depth: R * 4, bounds: { c: [0, 0, 0], r: R * 1.2 }, shadowSize: 2048 });
  P.haze = [P.camDepth * 10, P.camDepth * 20];
  P.addSolid(globeSolid(G, 70));
  const cv = await P.render(0), ctx = cv.getContext('2d');
  const img = ctx.getImageData(0, 0, w, h);
  let seed = 7;
  const rnd = () => { seed = (seed * 16807) % 2147483647; return seed / 2147483647; };
  for (let i = 0; i < 220; i++) {   // stars in the black
    const x = Math.floor(rnd() * w), y = Math.floor(rnd() * h), k = (y * w + x) * 4;
    if (img.data[k] + img.data[k + 1] + img.data[k + 2] < 60) { const v = rnd() > 0.85 ? 255 : 160; img.data.set([v, v, v * 0.95 + 10, 255], k); }
  }
  // the air: a two-pixel rim just outside the edge, pale blue, brightest toward the sun (upper left)
  const [cx, cy] = P.toScreen([0, 0, 0]), rs = R / P.mpp;
  for (let y = 0; y < h; y++) for (let x = 0; x < w; x++) {
    const dx = x + 0.5 - cx, dy = y + 0.5 - cy, d = Math.hypot(dx, dy);
    if (d < rs || d > rs + 2) continue;
    const lit = (-dx - dy) / (d * Math.SQRT2) > -0.2, k = (y * w + x) * 4;
    img.data.set(d < rs + 1 ? (lit ? [150, 196, 226, 255] : [74, 104, 150, 255]) : (lit ? [70, 106, 150, 255] : [34, 46, 82, 255]), k);
  }
  ctx.putImageData(img, 0, 0);
  for (const c of regionCamps(W)) {
    const p = unit(latitude(c.yk), lonOf(c.xk)).map((v) => v * (R + 3000));
    const [sx, sy] = P.toScreen(p);
    fireGlow(ctx, sx, sy, { big: light === 'dusk' });
  }
  return cv;
}

export default async function ({ light, opts }) {
  const stop = opts.find((o) => ['person', 'closecamp', 'camp', 'valley', 'region', 'map', 'globe'].includes(o)) || 'region';
  const land = opts.includes('land');
  const w = land ? 748 : 336, h = land ? 336 : 748;
  const fn = { person: nearStop, closecamp: nearStop, camp: campStop, valley: valleyStop, region: regionStop, map: mapStop, globe: globeStop }[stop];
  const cv = await fn({ light, w, h, stop, land });
  return canvasPngs(cv, 4);
}
