// The interface plates (PRE-32 to PRE-35, PRE-45): the world at rest with talk bubbles and a live moment, the same
// after a touch, a person's card and a book of ages page as a field journal, the landscape layout, and a sheet of
// bubbles, controls and fonts. opts: rest, touch, card, book, land, sheet.
import { build as village } from './village.js';
import { build as shelter } from './shelter.js';
import { Painter, canvasPngs } from '../engine.js';
import { K } from '../kit/k.js';
import { person } from '../kit/people.js';
import { animal } from '../kit/animals.js';
import { pot, hearth } from '../kit/things.js';
import { bush } from '../kit/plants.js';
import { boulder } from '../kit/land.js';
import { put } from '../kit/scene.js';
import { C, rect, darkPanel, paper, text, block, bubble, button, handle, ring, renderIcons, measure } from '../ui.js';
import { drawText } from '../font.js';

const ARU = { kind: 'woman', skin: 'skin3', hairStyle: 'long', top: 'tunic', topMat: 'dyedred', legs: 'leggings', legMat: 'leather', seed: 6 };

async function icons() {
  return renderIcons([
    { piece: pot({ h: 0.42, r: 0.2, mat: 'clay' }), scale: 3.4 },
    { piece: animal({ species: 'goat', pose: 'stand', seed: 3 }), scale: 1.9, yaw: 1.3 },
    { piece: animal({ species: 'stag', pose: 'alert', seed: 4 }), scale: 0.95, yaw: 1.3 },
    { piece: hearth({ seed: 2, level: 3, ring: true, logs: 4 }), scale: 1.25 },
    { piece: bush({ seed: 5, size: 0.9, berries: 'berry' }), scale: 1.5 },
    { piece: animal({ species: 'dog', pose: 'stand', seed: 6 }), scale: 2.1, yaw: 1.3 },
    { piece: { solid: boulder({ seed: 7, size: [0.3, 0.2, 0.25], detail: 0, mat: 'flint', moss: 0 }) }, scale: 4.4 },
    { piece: animal({ species: 'reindeer', pose: 'alert', seed: 8 }), scale: 0.95, yaw: 1.3 },
  ], { cell: 16, mpp: 0.1, cols: 8 });
}

/** A head-and-shoulders render of a person, for faces in bubbles and the card's sketch. */
async function portrait(spec, { w = 64, h = 80, mpp = 0.022, ink = true, target = 1.2 } = {}) {
  K.mpp = mpp;
  const P = new Painter({ w, h, mpp, yaw: 20, elev: 22, target: [0, target, 0], mood: 'noon', sunAz: 75, sunEl: 35, bounds: { c: [0, 1, 0], r: 3 }, shadowSize: 1024 });
  P.ink = ink; P.clear = !ink;
  P.ao = { rad: 0.2, dist: 0.2, k: 0 };
  put(P, person({ ...spec, pose: 'stand' }), 0, 0, { y: 0, yaw: 0.55 });
  return P.render(0);
}

function dim(ctx, w, h, a = 0.25) { ctx.fillStyle = `rgba(12,10,16,${a})`; ctx.fillRect(0, 0, w, h); }

function topLine(ctx, w) {
  const date = 'Year 214 · spring, day 12', speed = 'one day a minute';
  const tw = Math.max(measure(date), measure(speed)) + 16;
  darkPanel(ctx, Math.round(w / 2 - tw / 2), 10, tw, 30);
  text(ctx, date, Math.round(w / 2 - measure(date) / 2), 16, { color: C.text });
  text(ctx, speed, Math.round(w / 2 - measure(speed) / 2), 28, { color: C.accent });
}

function controls(ctx, cx, y) {
  darkPanel(ctx, cx - 64, y - 16, 128, 32, { corner: 3 });
  button(ctx, cx - 45, y, 11, 'slow');
  button(ctx, cx - 15, y, 11, 'pause');
  button(ctx, cx + 15, y, 11, 'play', { active: true });
  button(ctx, cx + 45, y, 11, 'fast');
}

function liveMoment(ctx, w, ic) {
  const x = 10, y = 10, bw = w - 20, bh = 36;
  darkPanel(ctx, x, y, bw, bh, { corner: 3 });
  rect(ctx, x + 6, y + 6, 24, 24, 'rgba(244,236,224,0.12)');
  ctx.drawImage(ic.canvas, ...ic.at(0), x + 10, y + 10, 16, 16);
  text(ctx, 'First pot fired whole', x + 38, y + 8, { color: C.accent });
  text(ctx, 'Aru of the Tavu · Year 214', x + 38, y + 21, { color: C.dim });
}

export default async function ({ light, opts }) {
  const kind = opts[0] || 'rest';
  const ic = await icons();

  if (kind === 'sheet') return sheet(ic);
  if (kind === 'book') return book();

  const land = kind === 'land';
  const w = land ? 748 : 336, h = land ? 336 : 748;
  const { P, marks } = await village({ light: light || 'morning', view: land ? { w, h, mpp: 0.06, target: [-4, 1, 4] } : { w, h, mpp: 0.06, target: [-1.5, 1, 6] } });
  const world = await P.render(0);
  const ctx = world.getContext('2d');
  const at = (k) => P.toScreen(marks[k]);

  if (kind === 'rest' || kind === 'touch') {
    // talk bubbles over the speakers: the potter about her pots, the diggers about goats and grain, a child about the dog
    const talk = [['p6', 0], ['p2', 1], ['p1', 3], ['p7', 5]];
    for (const [k, i] of talk) { const [x, y] = at(k); bubble(ctx, x, y, ic.canvas, ic.at(i)); }
    if (kind === 'rest') liveMoment(ctx, w, ic);
    if (kind === 'touch') {
      topLine(ctx, w);
      const [sx, sy] = P.toScreen([-0.8, 0, -4.6]);
      ring(ctx, sx, sy + 2, 9, 4);
      controls(ctx, w / 2, h - 52);
      handle(ctx, w / 2, h - 18);
    }
    return canvasPngs(world, 4);
  }

  if (kind === 'card' || kind === 'land') {
    if (kind === 'card') {
      dim(ctx, w, h, 0.2);
      const [sx, sy] = P.toScreen([-0.8, 0, -4.6]);
      ring(ctx, sx, sy + 2, 9, 4);
      await card(ctx, 0, 368, w, h - 368, ic);
      handle(ctx, w / 2, h - 10);
    } else {
      const pw = 270;
      await card(ctx, w - pw, 0, pw, h, ic, true);
      const [sx, sy] = P.toScreen([-0.8, 0, -4.6]);
      ring(ctx, sx, sy + 2, 9, 4);
      darkPanel(ctx, 10, 10, 150, 30);
      text(ctx, 'Year 214 · spring, day 12', 18, 16, { color: C.text });
      text(ctx, 'one day a minute', 18, 28, { color: C.accent });
      controls(ctx, 90, h - 30);
    }
    return canvasPngs(world, 4);
  }
}

/** A person's card as a page of the field journal (PRE-35): sketch, name and meaning, the lines that matter. */
async function card(ctx, x, y, w, h, ic, side = false) {
  paper(ctx, x, y, w, h, 3);
  if (!side) { rect(ctx, x, y - 2, w, 2, 'rgba(12,10,16,0.35)'); }
  const sketch = await portrait(ARU, { w: 66, h: 84, mpp: 0.0125, target: 1.12 });
  const px = x + 12, py = y + 14;
  rect(ctx, px - 2, py - 2, 70, 88, C.inkMid);
  ctx.drawImage(sketch, px, py);
  const tx = px + 78;
  drawText(ctx, 'Aru', tx, py + 2, { color: C.ink, scale: 2, hand: true, seed: 4 });
  text(ctx, '"river stone"', tx, py + 22, { color: C.inkMid, hand: true, seed: 9 });
  text(ctx, 'Woman, 31 · the Tavu', tx, py + 36, { color: C.ink });
  text(ctx, 'Potter · mother of Sefi', tx, py + 48, { color: C.ink });
  text(ctx, 'Mood: proud, a little tired', tx, py + 60, { color: C.red });
  let yy = py + 96;
  const col = side ? w - 24 : w - 24;
  const head = (s) => { text(ctx, s.toUpperCase(), px, yy, { color: C.inkMid }); rect(ctx, px, yy + 9, col, 1, C.line); yy += 14; };
  head('Doing, and why');
  yy = block(ctx, 'Firing pots for the winter stores, because last year the grain went damp in the baskets.', px, yy, col, { color: C.ink }) + 4;
  head('Ambition');
  yy = block(ctx, 'To teach the whole craft to Sefi before the next winter. Close: Sefi can coil, not yet fire.', px, yy, col, { color: C.ink }) + 4;
  head('Body');
  yy = block(ctx, 'Strong, well fed. An old burn on the left hand. Sore back.', px, yy, col, { color: C.ink }) + 4;
  head('In her words');
  yy = block(ctx, '"Pots crack when they are fired before they are dry. Wait for the cold wind."', px, yy, col, { color: C.ink, hand: true, seed: 12 }) + 6;
  head('Recent talk');
  const tk = [[0, 'pots'], [1, 'goats'], [3, 'fire']];
  tk.forEach(([i, label], k) => {
    const bx = px + k * 62;
    rect(ctx, bx, yy, 18, 18, C.paper2);
    ctx.drawImage(ic.canvas, ...ic.at(i), bx + 1, yy + 1, 16, 16);
    text(ctx, label, bx + 22, yy + 5, { color: C.ink });
  });
  yy += 26;
  // links to deeper views
  const tabs = ['Family', 'Crafts', 'Life story'];
  let tx2 = px;
  for (const t of tabs) { const tw = measure(t) + 12; rect(ctx, tx2, yy, tw, 15, C.paper2); rect(ctx, tx2, yy + 14, tw, 1, C.inkMid); text(ctx, t, tx2 + 6, yy + 4, { color: C.ink }); tx2 += tw + 6; }
}

/** The book of ages: a page of the journal with an ink drawing and the age's events in handwriting. */
async function book() {
  const w = 336, h = 748;
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  paper(ctx, 0, 0, w, h, 11);
  text(ctx, 'THE BOOK OF AGES', 16, 16, { color: C.inkMid });
  text(ctx, 'iii', w - 30, 16, { color: C.inkMid });
  rect(ctx, 16, 27, w - 32, 1, C.line);
  drawText(ctx, 'The age of hesoru', 16, 38, { color: C.ink, scale: 2, hand: true, seed: 3 });
  text(ctx, 'fire from wood · from Year 31', 16, 60, { color: C.red, hand: true, seed: 5 });
  // the drawing: the shelter, in ink
  const { P } = await shelter({ light: 'golden', opts: ['wide'], view: { w: 304, h: 150, mpp: 0.06, target: [1, 2.4, -3.5] } });
  P.ink = true;
  const sk = await P.render(0);
  rect(ctx, 14, 76, 308, 154, C.inkMid);
  ctx.drawImage(sk, 16, 78);
  text(ctx, 'The shelter by the stream, as it was kept.', 16, 236, { color: C.inkMid, hand: true, seed: 8 });
  let y = 256;
  const entries = [
    ['Year 31, autumn', 'Ume of the Tavu carried a burning branch from the lightning fire to the shelter and kept it alive through three nights of rain. The band called it hesoru.'],
    ['Year 33', 'The first hearth ringed with stones, so the fire stays where it is put.'],
    ['Year 38, winter', 'Meat dried over smoke lasted until the thaw for the first time.'],
    ['Year 44', 'Ume died, aged 58. Her fire was carried to two new bands upstream.'],
    ['Year 51', 'The Tavu painted a red deer on the shelter wall.'],
  ];
  for (const [when, what] of entries) {
    text(ctx, when, 16, y, { color: C.red, hand: true, seed: y });
    y += 13;
    y = block(ctx, what, 16, y, w - 32, { color: C.ink, hand: true, seed: y + 3 }) + 9;
  }
  // a portrait of Ume, who kept the first fire
  const ume = await portrait({ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'fur', necklace: 'shell', seed: 31 }, { w: 60, h: 76, mpp: 0.013, target: 1.1 });
  y += 4;
  rect(ctx, 14, y - 2, 64, 80, C.inkMid);
  ctx.drawImage(ume, 16, y);
  text(ctx, 'Ume, who kept the fire', 90, y + 18, { color: C.ink, hand: true, seed: 41 });
  text(ctx, '"Feed it before you eat."', 90, y + 34, { color: C.inkMid, hand: true, seed: 42 });
  text(ctx, 'Year 0 - 44', 90, y + 50, { color: C.red, hand: true, seed: 43 });
  // a red hand stamp in the margin, as on the shelter wall
  const hand = ['.#.#.', '#####', '#####', '.###.', '.###.'];
  hand.forEach((row, j) => [...row].forEach((ch, i) => { if (ch === '#') rect(ctx, w - 44 + i * 3, h - 50 + j * 3, 3, 3, 'rgba(156,58,42,0.75)'); }));
  return canvasPngs(c, 4);
}

/** A sheet of the interface's parts: bubbles, controls, panels and the two fonts. */
async function sheet(ic) {
  const w = 748, h = 420;
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  rect(ctx, 0, 0, w, h, '#1c1824');
  const label = (s, x, y) => text(ctx, s.toUpperCase(), x, y, { color: C.accent });
  label('Talk bubbles: a picture of the topic', 20, 18);
  const names = ['pots', 'goats', 'deer', 'fire', 'berries', 'dogs', 'flint', 'reindeer'];
  names.forEach((n, i) => { const x = 40 + i * 52; bubble(ctx, x, 70, ic.canvas, ic.at(i)); text(ctx, n, x - Math.round(measure(n) / 2), 80, { color: C.dim }); });
  const face = await portrait(ARU, { w: 16, h: 16, mpp: 0.026, ink: false, target: 1.42 });
  bubble(ctx, 40 + 8 * 52, 70, face, [0, 0, 16, 16]); text(ctx, 'a person', 40 + 8 * 52 - 18, 80, { color: C.dim });
  label('Time controls and the handle', 20, 110);
  controls(ctx, 100, 146);
  handle(ctx, 100, 176);
  button(ctx, 220, 146, 11, 'book'); button(ctx, 250, 146, 11, 'globe');
  text(ctx, 'views', 222, 162, { color: C.dim });
  label('A live moment', 320, 110);
  const lm = document.createElement('canvas'); lm.width = 300; lm.height = 60;
  liveMoment(lm.getContext('2d'), 300, ic);
  ctx.drawImage(lm, 310, 116);
  label('Quiet panel', 20, 200);
  darkPanel(ctx, 20, 214, 200, 60);
  text(ctx, 'Year 214 · spring, day 12', 30, 222, { color: C.text });
  text(ctx, 'one day a minute', 30, 234, { color: C.accent });
  text(ctx, 'Tavu · 47 people · 3 houses', 30, 250, { color: C.dim });
  label('Journal paper', 250, 200);
  paper(ctx, 250, 214, 200, 60, 5);
  drawText(ctx, 'Aru', 260, 222, { color: C.ink, scale: 2, hand: true, seed: 4 });
  text(ctx, '"river stone"', 300, 228, { color: C.inkMid, hand: true, seed: 9 });
  text(ctx, 'Woman, 31 · the Tavu', 260, 252, { color: C.ink });
  label('Plain pixel font', 20, 296);
  text(ctx, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 20, 310, { color: C.text });
  text(ctx, 'abcdefghijklmnopqrstuvwxyz 0123456789', 20, 322, { color: C.text });
  text(ctx, 'The Tavu keep goats and sow grain by the river.', 20, 336, { color: C.dim });
  drawText(ctx, 'Titles at 2x', 20, 350, { color: C.text, scale: 2 });
  label('Pixel handwriting', 400, 296);
  text(ctx, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 400, 310, { color: C.bubble, hand: true, seed: 2 });
  text(ctx, 'abcdefghijklmnopqrstuvwxyz 0123456789', 400, 322, { color: C.bubble, hand: true, seed: 3 });
  text(ctx, '"Wait for the cold wind," said Aru.', 400, 336, { color: C.dim, hand: true, seed: 5 });
  drawText(ctx, 'The age of hesoru', 400, 350, { color: C.bubble, scale: 2, hand: true, seed: 6 });
  return canvasPngs(c, 3);
}
