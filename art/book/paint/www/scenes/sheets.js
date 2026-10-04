// Element sheets for the art book: the model kit (PRE-46) laid out and named in the game's pixel font.
// opts: people, animals, plants, things, light.
import * as THREE from 'three';
import { Painter, canvasPngs } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, M4 } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { ground, boulder, stoneBlock } from '../kit/land.js';
import { broadleaf, conifer, bush, flatTree, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal, bird } from '../kit/animals.js';
import { tent, dome, windbreak, rack, hearth, pot, canoe, longhouse, pitHouse, mudHouse, furnace, grave, midden, log, smoke } from '../kit/things.js';
import { put } from '../kit/scene.js';
import { RAMPS } from '../palette.js';
import { drawText, measure } from '../font.js';
import { C, rect } from '../ui.js';

/**
 * A sheet: a calm ground under rows of items. rows: [{ h, items }], each item { name, sub, piece, r (footprint
 * radius in metres), scale, yaw, y }; items in a row are spaced evenly and named under their footprint.
 */
async function grid({ w, mpp, light = 'noon', rows, top = 10, ground: gmat = 'meadow', yaw = 22 }) {
  const h = top + rows.reduce((a, r) => a + r.h, 0) + 6;
  K.mpp = mpp;
  const P = new Painter({ w, h, mpp, yaw, elev: 30, target: [0, 0, 0], mood: light, bounds: { c: [0, 0, 0], r: Math.max(w, h) * mpp * 0.8 }, shadowSize: 4096 });
  const span = Math.max(w, h) * mpp;
  P.addSolid(ground({ x0: -span, x1: span, z0: -span, z1: span, step: 1, height: () => 0, weights: () => [1, 0, 0, 0] }), [[gmat, PAT.GROUND]]);
  P.haze = [P.camDepth + 1e4, P.camDepth + 2e4];
  const labels = [];
  let y = top;
  for (const row of rows) {
    const n = row.items.length, colW = w / n;
    row.items.forEach((it, i) => {
      const sx = colW * (i + 0.5), drop = Math.round(((it.r || 0.6) * (it.scale || 1) * 0.5) / mpp);
      const sy = y + row.h - 22 - drop;
      const [x, , z] = P.groundAt(sx, sy, 0);
      if (it.piece.fire && !it.lit) it.piece.fire = null;
      put(P, it.piece, x, z, { y: it.y || 0, yaw: it.yaw ?? 0.5, scale: it.scale || 1 });
      labels.push([it.name, sx, sy + drop + 5, it.sub]);
    });
    y += row.h;
  }
  const cv = await P.render(0);
  const ctx = cv.getContext('2d');
  for (const [name, sx, sy, sub] of labels) {
    drawText(ctx, name, Math.round(sx - measure(name) / 2), sy, { color: '#2a2230', shadow: 'rgba(255,248,230,0.55)' });
    if (sub) drawText(ctx, sub, Math.round(sx - measure(sub) / 2), sy + 10, { color: '#4a3f52', shadow: 'rgba(255,248,230,0.45)' });
  }
  return cv;
}

export default async function ({ light, opts }) {
  const kind = opts[0] || 'people';
  const fur = { top: 'parka', topMat: 'fur', legMat: 'furdark', feet: 'furdark', hairStyle: 'hood' };
  if (kind === 'people') {
    const P = (spec) => person({ seed: 3, ...spec });
    const items = [
      { name: 'man', piece: P({ kind: 'man', skin: 'skin2', beard: true, top: 'tunic', topMat: 'hide' }) },
      { name: 'woman', piece: P({ kind: 'woman', skin: 'skin1', hairStyle: 'braid', top: 'dress', topMat: 'leather', necklace: 'shell' }) },
      { name: 'elder', piece: P({ kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'fur' }) },
      { name: 'youth', piece: P({ kind: 'youth', skin: 'skin2', hair: 'hairred', hairStyle: 'topknot', top: 'none', legs: 'leggings', paint: 'ochre' }) },
      { name: 'child', piece: P({ kind: 'child', skin: 'skin3', top: 'none', legs: 'bare', feet: null }) },
      { name: 'warm lands', sub: 'bare, paint', piece: P({ kind: 'man', skin: 'skin3', top: 'none', legs: 'bare', feet: null, paint: 'dyedred', necklace: 'bone' }) },
      { name: 'temperate', sub: 'hide tunic', piece: P({ kind: 'woman', skin: 'skin2', hairStyle: 'bun', top: 'tunic', topMat: 'hide', legMat: 'leather' }) },
      { name: 'cold', sub: 'fur parka', piece: P({ kind: 'man', skin: 'skin1', ...fur }) },
      { name: 'villages', sub: 'woven cloth', piece: P({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'cloth', belt: 'dyedred' }) },
      { name: 'a chief', sub: 'copper, fur', piece: P({ kind: 'man', skin: 'skin2', hairStyle: 'long', beard: true, top: 'tunic', topMat: 'dyedred', legMat: 'cloth', necklace: 'copper', headband: 'copper', belt: 'copper', cloak: 'fur', held: 'staff' }) },
      { name: 'walk', piece: P({ kind: 'man', skin: 'skin2', top: 'tunic', topMat: 'hide', pose: 'walkA', held: { kind: 'spear', tilt: 0.25 } }) },
      { name: 'carry', piece: P({ kind: 'woman', skin: 'skin1', top: 'tunic', topMat: 'leather', pose: 'carry', held: 'bundle' }) },
      { name: 'knap', piece: P({ kind: 'man', skin: 'skin3', top: 'tunic', topMat: 'hide', pose: 'knap', held: 'stone' }) },
      { name: 'scrape', piece: P({ kind: 'woman', skin: 'skin2', top: 'dress', topMat: 'hide', pose: 'scrape' }) },
      { name: 'throw', piece: P({ kind: 'youth', skin: 'skin2', top: 'none', legs: 'leggings', pose: 'throw', held: 'spear' }) },
      { name: 'dig', piece: P({ kind: 'woman', skin: 'skin3', top: 'tunic', topMat: 'cloth', pose: 'dig', held: 'staff' }) },
      { name: 'drum', piece: P({ kind: 'man', skin: 'skin3', top: 'wrap', topMat: 'leather', pose: 'drum', held: 'drum' }) },
      { name: 'blow', piece: P({ kind: 'man', skin: 'skin2', top: 'none', legs: 'leggings', legMat: 'cloth', pose: 'blow' }) },
      { name: 'grieve', piece: P({ kind: 'woman', skin: 'skin2', hairStyle: 'long', top: 'dress', topMat: 'hide', pose: 'armsUp' }) },
      { name: 'rest', piece: P({ kind: 'elder', skin: 'skin1', hair: 'hairgrey', top: 'wrap', topMat: 'fur', pose: 'sitFloor' }) },
    ];
    items.forEach((it) => { it.r = 0.4; });
    return canvasPngs(await grid({ w: 748, mpp: 0.025, rows: [{ h: 132, items: items.slice(0, 10) }, { h: 128, items: items.slice(10) }], top: 14 }), 3);
  }
  if (kind === 'animals') {
    const A = (species, pose, extra = {}) => animal({ species, pose, seed: 5, ...extra });
    const items = [
      { name: 'red deer', sub: 'stag', piece: A('stag', 'alert'), yaw: 1.2 }, { name: 'hind', piece: A('hind', 'graze'), yaw: 1.2 },
      { name: 'reindeer', piece: A('reindeer', 'walk'), yaw: 1.2 }, { name: 'wild horse', piece: A('horse', 'stand'), yaw: 1.2 },
      { name: 'bison', piece: A('bison', 'stand'), yaw: 1.2, scale: 0.85 }, { name: 'aurochs', piece: A('aurochs', 'walk'), yaw: 1.2, scale: 0.8 },
      { name: 'boar', piece: A('boar', 'stand'), yaw: 1.2 }, { name: 'wolf', piece: A('wolf', 'run'), yaw: 1.2 },
      { name: 'goat', piece: A('goat', 'graze'), yaw: 1.2 }, { name: 'sheep', piece: A('sheep', 'stand'), yaw: 1.2 },
      { name: 'dog', piece: A('dog', 'stand'), yaw: 1.2 }, { name: 'hare', piece: A('hare', 'stand'), yaw: 1.2 },
      { name: 'heron', piece: bird({ kind: 'heron' }), yaw: 1.0 }, { name: 'duck', piece: bird({ kind: 'duck' }), yaw: 1.0 },
      { name: 'crow', piece: bird({ kind: 'crow', fly: true }), y: 1.0, yaw: 1.0 }, { name: 'gull', piece: bird({ kind: 'gull', fly: true }), y: 1.0, yaw: 1.0 },
    ];
    items.forEach((it) => { it.r = it.r || 0.7; });
    return canvasPngs(await grid({ w: 748, mpp: 0.032, rows: [{ h: 112, items: items.slice(0, 8) }, { h: 84, items: items.slice(8) }], top: 8 }), 3);
  }
  if (kind === 'plants') {
    const low = (tiles, mat, n, bias = 0, size = 1) => {
      const c = new Cards(), r = new Rand(n);
      for (let i = 0; i < n; i++) c.add({ c: [r.range(-0.7, 0.7), 0, r.range(-0.7, 0.7)], w: 32 * K.mpp * size, h: 32 * K.mpp * size, tile: r.pick(tiles), mat, bias, flag: FLAG.NOOUTLINE | FLAG.NOSHADOW, upright: 1 });
      return { cards: c };
    };
    K.mpp = 0.05;
    const items = [
      { name: 'oak', sub: 'summer', piece: broadleaf({ seed: 3, h: 7 }) },
      { name: 'oak', sub: 'spring', piece: broadleaf({ seed: 4, h: 7, season: 'spring', leafMat: 'leafsp' }) },
      { name: 'oak', sub: 'autumn', piece: broadleaf({ seed: 5, h: 7, season: 'autumn' }) },
      { name: 'oak', sub: 'winter', piece: broadleaf({ seed: 6, h: 7, season: 'bare' }) },
      { name: 'pine', piece: conifer({ seed: 7, h: 8 }) },
      { name: 'pine', sub: 'snow', piece: conifer({ seed: 8, h: 8, snow: true }) },
      { name: 'acacia', sub: 'dry lands', piece: flatTree({ seed: 9, h: 5.5 }) },
      { name: 'olive', sub: 'dry lands', piece: flatTree({ seed: 10, h: 5, olive: true }) },
      { name: 'bush', sub: 'berries', piece: bush({ seed: 11, size: 1.3, berries: 'berry' }) },
      { name: 'grass', piece: low(TILE.TUFT, 'meadow', 14, 1) },
      { name: 'flowers', piece: low(TILE.FLOWER, 'flowerp', 10) },
      { name: 'reeds', piece: low([TILE.REED, TILE.SEDGE], 'reed', 12, 0, 1.1) },
      { name: 'wild grain', piece: low([TILE.GRAIN], 'thatch', 12) },
      { name: 'ferns', piece: low([TILE.FERN, TILE.HERB], 'leaf', 8, 1) },
      { name: 'dry grass', piece: low(TILE.DRYTUFT, 'drygrass', 12) },
    ];
    items.forEach((it, i) => { it.r = i < 9 ? 2.6 : 0.7; });
    return canvasPngs(await grid({ w: 748, mpp: 0.05, rows: [{ h: 178, items: items.slice(0, 5) }, { h: 168, items: items.slice(5, 9) }, { h: 64, items: items.slice(9) }], top: 6 }), 3);
  }
  if (kind === 'things') {
    K.mpp = 0.045;
    const items = [
      { name: 'hide tent', piece: tent({ seed: 1, r: 1.6, h: 2.8 }) },
      { name: 'hide dome', sub: 'mammoth bone', piece: dome({ seed: 2, r: 1.8, h: 1.7, bones: true }) },
      { name: 'thatch dome', piece: dome({ seed: 3, r: 1.8, h: 1.7, mat: 'thatch', pat: PAT.THATCH }) },
      { name: 'windbreak', piece: windbreak({ seed: 4, len: 2.8 }) },
      { name: 'meat rack', piece: rack({ seed: 5, len: 2.0 }) },
      { name: 'fish rack', piece: rack({ seed: 6, len: 2.0, hang: 'fish' }) },
      { name: 'pit house', piece: pitHouse({ seed: 7, r: 2.0, h: 2.6 }) },
      { name: 'longhouse', sub: 'reed roof', piece: longhouse({ seed: 8, len: 5, w: 3.2, wallH: 1.1, roofH: 1.9 }), scale: 0.8 },
      { name: 'mud brick', piece: mudHouse({ seed: 9, w: 3.6, d: 3, h: 1.9 }) },
      { name: 'hearth', sub: 'cold', piece: hearth({ seed: 10, level: 0 }) },
      { name: 'hearth', sub: 'embers', piece: hearth({ seed: 11, level: 1 }) },
      { name: 'hearth', sub: 'cooking', piece: hearth({ seed: 12, level: 2 }) },
      { name: 'hearth', sub: 'blaze', piece: hearth({ seed: 13, level: 4 }) },
      { name: 'copper furnace', piece: furnace({ seed: 14 }) },
      { name: 'pots', piece: { solid: (() => { const s = new Solid(); [[-0.35, 0.36, 0.17], [0.1, 0.5, 0.21], [0.5, 0.3, 0.15]].forEach(([x, h, r], i) => s.add(pot({ h, r, mat: i === 1 ? 'clay' : (i ? 'charcoal' : 'ochre') }).solid, M4.T(x, 0, 0))); return s; })() }, scale: 1.3 },
      { name: 'dugout', piece: canoe({ len: 3.6 }) },
      { name: 'grave', piece: grave({ seed: 15 }) },
      { name: 'shell midden', piece: midden({ seed: 16, r: 1.3, h: 0.5 }) },
    ];
    const by = (names) => names.map((n) => items.find((it) => (it.name + (it.sub || '')) === n));
    const R = { 'hide tent': 1.7, 'hide domemammoth bone': 2.1, 'thatch dome': 1.9, 'pit house': 2.6, windbreak: 1.4, 'longhousereed roof': 3.4, 'mud brick': 2.6, 'meat rack': 1.0, 'fish rack': 1.0, dugout: 1.0, grave: 1.6, 'shell midden': 1.4 };
    items.forEach((it) => { it.r = R[it.name + (it.sub || '')] ?? 0.7; });
    return canvasPngs(await grid({ w: 748, mpp: 0.045, rows: [
      { h: 150, items: by(['hide tent', 'hide domemammoth bone', 'thatch dome', 'pit house', 'windbreak']) },
      { h: 132, items: by(['longhousereed roof', 'mud brick', 'meat rack', 'fish rack', 'dugout']) },
      { h: 86, items: by(['hearthcold', 'hearthembers', 'hearthcooking', 'hearthblaze', 'copper furnace', 'pots', 'grave', 'shell midden']) },
    ], top: 8 }), 3);
  }
  if (kind === 'light') return lightSheet();
}

/** One small place in every hour, and the colour ramps the light picks from (PRE-20, PRE-30). */
async function lightSheet() {
  const hours = [['morning', 'morning'], ['noon', 'noon'], ['golden', 'golden hour'], ['dusk', 'dusk'], ['night', 'night'], ['winter', 'winter']];
  const cw = 236, ch = 150, gap = 10, W = cw * 3 + gap * 4, H = ch * 2 + gap * 3 + 30 * 2 + 150;
  const out = document.createElement('canvas'); out.width = W; out.height = H;
  const ctx = out.getContext('2d');
  rect(ctx, 0, 0, W, H, '#1c1824');
  for (let i = 0; i < hours.length; i++) {
    const [mood, label] = hours[i];
    K.mpp = 0.05;
    const P = new Painter({ w: cw, h: ch, mpp: 0.05, yaw: 22, elev: 30, target: [0, 1, 0], mood, bounds: { c: [0, 0, 0], r: 14 }, shadowSize: 2048 });
    const snow = mood === 'winter';
    P.addSolid(ground({ x0: -16, x1: 16, z0: -16, z1: 16, step: 0.5, height: (x, z) => 0.3 * Math.sin(x * 0.3) * Math.cos(z * 0.25), weights: () => [1, 0, 0, 0] }), [[snow ? 'snow' : 'meadow', snow ? PAT.SNOW : PAT.GROUND]]);
    put(P, tent({ seed: 1, r: 1.4, h: 2.6 }), -2.6, -0.8, { y: 0, yaw: 0.5 });
    put(P, snow ? conifer({ seed: 2, h: 6, snow: true }) : broadleaf({ seed: 2, h: 6 }), 3.2, -2.5, { y: 0 });
    put(P, { solid: boulder({ seed: 3, size: [1.6, 1.1, 1.3], detail: 1, mood: 'rock', moss: snow ? 0.6 : 0.5, mossMat: snow ? 'snow' : 'moss' }) }, 1.8, 1.4, { y: -0.1 });
    const H = hearth({ seed: 4, level: mood === 'night' || mood === 'dusk' ? 3 : 2 });
    if (mood !== 'night') { H.fire[3] *= 0.45; H.fire[4] *= 0.5; }
    put(P, H, -0.4, 1.2, { y: 0 });
    put(P, person({ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: snow ? 'parka' : 'tunic', topMat: snow ? 'fur' : 'dyedred', legMat: 'leather', pose: 'sitFloor', seed: 5 }), -1.3, 1.9, { y: 0, yaw: 2.2 });
    put(P, person({ kind: 'man', skin: 'skin3', hairStyle: 'short', top: snow ? 'parka' : 'tunic', topMat: snow ? 'reindeer' : 'hide', legMat: 'leather', pose: 'stand', held: 'spear', seed: 6 }), 0.6, 2.4, { y: 0, yaw: -2.4 });
    const pf = new Puffs(); smoke(pf, { at: [-0.4, 0.9, 1.2], height: 5, drift: [0.6, -0.3], seed: 7, size: 0.22, tone: mood === 'night' ? 1 : 5, alpha: mood === 'night' ? 0.2 : 0.4 }); P.addPuffs(pf);
    if (mood === 'night') P.addFire([-0.4, 0.5, 1.2], 1.1, 9);
    const cv = await P.render(0);
    const x = gap + (i % 3) * (cw + gap), y = gap + Math.floor(i / 3) * (ch + gap + 30);
    ctx.drawImage(cv, x, y);
    drawText(ctx, label.toUpperCase(), x, y + ch + 8, { color: C.accent });
  }
  // the ramps
  const names = ['grass', 'meadow', 'leaf', 'pine', 'rock', 'lime', 'redrock', 'sand', 'snow', 'water', 'dirt', 'hide', 'fur', 'wood', 'thatch', 'clay', 'skin1', 'skin2', 'skin3', 'deer', 'dyedred', 'ochre', 'copper', 'fire'];
  const y0 = gap + 2 * (ch + gap + 30) + 4;
  drawText(ctx, 'THE PALETTE: 7 STEPS A MATERIAL, THE LIGHT PICKS THE STEP', gap, y0, { color: C.accent });
  names.forEach((n, i) => {
    const cx = gap + (i % 8) * 90, cy = y0 + 16 + Math.floor(i / 8) * 42;
    RAMPS[n].forEach((hex, k) => rect(ctx, cx + k * 10, cy, 10, 18, hex));
    drawText(ctx, n, cx, cy + 22, { color: C.dim });
  });
  return canvasPngs(out, 3);
}
