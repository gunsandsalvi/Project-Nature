// The interface plates (PRE-32 to PRE-35, PRE-45): the world at rest with talk bubbles and a live moment, the same
// after a touch, a person's card and a book of ages page as a field journal, the landscape layout, the ring of
// powers (GOD-10, GOD-11), the views the handle opens (PRE-33, PRE-13), the peoples-and-territories map (PRE-07),
// the new-world screen (WLD-10), and a sheet of the parts and fonts.
// opts: rest, touch, card, book, land, powers, views, map, worlds, sheet.
import { build as village } from './village.js';
import { build as shelter } from './shelter.js';
import { regionMap, globeOf } from './zoom.js';
import { Painter, canvasPngs } from '../engine.js';
import { K } from '../kit/k.js';
import { person } from '../kit/people.js';
import { animal } from '../kit/animals.js';
import { pot, hearth } from '../kit/things.js';
import { bush } from '../kit/plants.js';
import { boulder } from '../kit/land.js';
import { put } from '../kit/scene.js';
import { fbm } from '../geo.js';
import { fireGlow, groupMarker } from '../kit/far.js';
import { regionCamps } from '../kit/zoomworld.js';
import { worldGrid, findStart, latitude, WORLD, BIOME } from '../worldgen.js';
import { C, rect, darkPanel, paper, text, block, bubble, button, glyph, handle, ring, renderIcons, measure } from '../ui.js';
import { drawText } from '../font.js';

const ARU = { kind: 'woman', skin: 'skin3', hairStyle: 'long', top: 'tunic', topMat: 'dyedred', legs: 'leggings', legMat: 'leather', seed: 6 };
const ARU_AT = [-0.8, 0, -4.6];   // where she sits in the village scene
// the date as TIM-14 writes it, and the speed the close camp zoom of these plates asks for (TIM-01)
const DATE = 'Year 214, spring, day 12', SPEED = 'an hour a minute', SPEED_K = 0.25;
const clamp = (v, a, b) => Math.max(a, Math.min(b, v));

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

/** A head-and-shoulders render of a person: in ink for the journal's drawings, in colour for cards and faces. */
async function portrait(spec, { w = 64, h = 80, mpp = 0.022, ink = true, target = 1.2 } = {}) {
  K.mpp = mpp;
  const P = new Painter({ w, h, mpp, yaw: 20, elev: 22, target: [0, target, 0], mood: 'noon', sunAz: 75, sunEl: 35, bounds: { c: [0, 1, 0], r: 3 }, shadowSize: 1024 });
  P.ink = ink; P.clear = !ink;
  P.ao = { rad: 0.2, dist: 0.2, k: 0 };
  put(P, person({ ...spec, pose: 'stand' }), 0, 0, { y: 0, yaw: 0.55 });
  return P.render(0);
}

function dim(ctx, w, h, a = 0.25) { ctx.fillStyle = `rgba(12,10,16,${a})`; ctx.fillRect(0, 0, w, h); }

/** The date and the real speed, top centre (PRE-33, TIM-14); "paused" while you choose a power (TIM-15). */
function topLine(ctx, w, { paused = false } = {}) {
  const speed = paused ? 'paused' : SPEED;
  const tw = Math.max(measure(DATE), measure(speed)) + 16;
  darkPanel(ctx, Math.round(w / 2 - tw / 2), 10, tw, 30);
  text(ctx, DATE, Math.round(w / 2 - measure(DATE) / 2), 16, { color: C.text });
  text(ctx, speed, Math.round(w / 2 - measure(speed) / 2), 28, { color: C.accent });
}

/** The time controls (TIM-04, TIM-11): pause; play, which hands the speed back to zoom; the dial; the lock; skip. */
function controls(ctx, cx, y) {
  darkPanel(ctx, cx - 76, y - 16, 152, 32, { corner: 3 });
  button(ctx, cx - 58, y, 11, 'pause');
  button(ctx, cx - 29, y, 11, 'play', { active: true });
  button(ctx, cx, y, 11, 'dial', { k: SPEED_K });
  button(ctx, cx + 29, y, 11, 'lock');
  button(ctx, cx + 58, y, 11, 'skip');
}

function liveMoment(ctx, w, ic) {
  const x = 10, y = 10, bw = w - 20, bh = 36;
  darkPanel(ctx, x, y, bw, bh, { corner: 3 });
  rect(ctx, x + 6, y + 6, 24, 24, 'rgba(244,236,224,0.12)');
  ctx.drawImage(ic.canvas, ...ic.at(0), x + 10, y + 10, 16, 16);
  text(ctx, 'First pot fired whole', x + 38, y + 8, { color: C.accent });
  text(ctx, 'Aru of the Tavu · Year 214', x + 38, y + 21, { color: C.dim });
}

/** A label on a small dark pill, for words over the world. */
function pill(ctx, s, x, y, color = C.text) {
  const tw = measure(s);
  darkPanel(ctx, x - 3, y - 2, tw + 6, 12, { corner: 1 });
  text(ctx, s, x, y, { color });
}

/** A text button: a dark pill with its word in the middle. */
function textButton(ctx, s, cx, cy, { active = false } = {}) {
  const tw = measure(s) + 20;
  darkPanel(ctx, Math.round(cx - tw / 2), cy - 9, tw, 18, { corner: 3 });
  text(ctx, s, Math.round(cx - measure(s) / 2), cy - 3, { color: active ? C.accent : C.text });
}

export default async function ({ light, opts }) {
  const kind = opts[0] || 'rest';
  const ic = await icons();

  if (kind === 'sheet') return sheet(ic);
  if (kind === 'book') return book();
  if (kind === 'map') return peoplesMap();
  if (kind === 'worlds') return worlds();

  const land = kind === 'land';
  const w = land ? 748 : 336, h = land ? 336 : 748;
  const { P, marks } = await village({ light: light || 'morning', view: land ? { w, h, mpp: 0.06, target: [3.5, 1, 2.2] } : { w, h, mpp: 0.06, target: [-1.5, 1, 6] } });
  const world = await P.render(0);
  const ctx = world.getContext('2d');
  const at = (k) => P.toScreen(marks[k]);
  const aru = P.toScreen(ARU_AT);

  if (kind === 'rest' || kind === 'touch') {
    // talk bubbles over the speakers: the potter about her pots, the diggers about goats and grain, a child about the dog
    const talk = [['p6', 0], ['p2', 1], ['p1', 3], ['p7', 5]];
    for (const [k, i] of talk) { const [x, y] = at(k); bubble(ctx, x, y, ic.canvas, ic.at(i)); }
    if (kind === 'rest') liveMoment(ctx, w, ic);
    if (kind === 'touch') {
      topLine(ctx, w);
      ring(ctx, aru[0], aru[1] + 2, 9, 4);
      controls(ctx, w / 2, h - 52);
      handle(ctx, w / 2, h - 18);
    }
    return canvasPngs(world, 4);
  }

  if (kind === 'card' || kind === 'land') {
    if (kind === 'card') {
      dim(ctx, w, h, 0.2);
      ring(ctx, aru[0], aru[1] + 2, 9, 4);
      await card(ctx, 0, 368, w, h - 368, ic);
      handle(ctx, w / 2, h - 10);
    } else {
      const pw = 270;
      await card(ctx, w - pw, 0, pw, h, ic, true);
      ring(ctx, aru[0], aru[1] + 2, 9, 4);
      darkPanel(ctx, 10, 10, 150, 30);
      text(ctx, DATE, 18, 16, { color: C.text });
      text(ctx, SPEED, 18, 28, { color: C.accent });
      controls(ctx, 96, h - 30);
    }
    return canvasPngs(world, 4);
  }

  if (kind === 'powers') { await powers(ctx, P, w, h, aru); return canvasPngs(world, 4); }
  if (kind === 'views') { views(ctx, w, h); return canvasPngs(world, 4); }
}

// --- the person's card ----------------------------------------------------------------------------------------------

// a body seen from the front for the card's health (PRE-35): h head, t torso, r/R her right arm and hand, l/L her
// left, g/f her right and left leg; her left is on your right
const BODY = [
  '....hhhh....', '...hhhhhh...', '...hhhhhh...', '...hhhhhh...', '....hhhh....', '.....tt.....',
  '..rrttttll..', '.rr.tttt.ll.', '.rr.tttt.ll.', '.rr.tttt.ll.', '.rr.tttt.ll.', '.RR.tttt.LL.', '.RR.tttt.LL.',
  '....tttt....', '....ggff....', '....ggff....', '....ggff....', '....ggff....', '....ggff....', '....ggff....',
  '....ggff....', '...ggg.fff..',
];
const HEALTH = { well: '#7f9c58', hurt: '#d39a3c', bad: '#b8452f' };

function bodyDiagram(ctx, x, y, state, k = 2) {
  BODY.forEach((row, j) => [...row].forEach((ch, i) => {
    if (ch === '.') return;
    // an ink edge where a part meets the paper or another part
    const at = (di, dj) => { const r = BODY[j + dj]; return r ? r[i + di] : undefined; };
    const edge = [[-1, 0], [1, 0], [0, -1], [0, 1]].some(([di, dj]) => { const c = at(di, dj); return c === undefined || c === '.'; });
    rect(ctx, x + i * k, y + j * k, k, k, edge ? C.inkMid : HEALTH[state[ch] || 'well']);
  }));
}

/** A person's card as a page of the field journal (PRE-35): likeness, name and meaning, the lines that matter. */
async function card(ctx, x, y, w, h, ic, side = false) {
  paper(ctx, x, y, w, h, 3);
  if (!side) { rect(ctx, x, y - 2, w, 2, 'rgba(12,10,16,0.35)'); }
  // her likeness in colour, as she looks at person zoom (PRE-27), pasted in like a picture
  const face = await portrait(ARU, { w: 66, h: 84, mpp: 0.0125, target: 1.24, ink: false });
  const px = x + 12, py = y + 14;
  rect(ctx, px - 2, py - 2, 70, 88, C.inkMid);
  rect(ctx, px, py, 66, 84, '#d9c7a0');
  ctx.drawImage(face, px, py);
  const tx = px + 78;
  drawText(ctx, 'Aru', tx, py + 2, { color: C.ink, scale: 2, hand: true, seed: 4 });
  text(ctx, '"river stone"', tx, py + 22, { color: C.inkMid });
  text(ctx, 'Woman, 31 · the Tavu', tx, py + 36, { color: C.ink });
  text(ctx, 'Potter · mother of Sefi', tx, py + 48, { color: C.ink });
  text(ctx, 'Mood: proud, a little tired', tx, py + 60, { color: C.red });
  let yy = py + 96;
  const col = w - 24;
  const head = (s) => { text(ctx, s.toUpperCase(), px, yy, { color: C.inkMid }); rect(ctx, px, yy + 9, col, 1, C.line); yy += 14; };
  head('Doing, and why');
  yy = block(ctx, 'Firing pots for the winter stores, because last year the grain went damp in the baskets.', px, yy, col, { color: C.ink }) + 4;
  // her ambition and how close she is (MND-32)
  head('Ambition');
  yy = block(ctx, 'To teach Sefi the whole craft before winter.', px, yy, col, { color: C.ink }) + 2;
  const bw = 96;
  rect(ctx, px, yy + 1, bw, 7, C.inkMid); rect(ctx, px + 1, yy + 2, bw - 2, 5, C.paper2); rect(ctx, px + 1, yy + 2, Math.round((bw - 2) * 0.6), 5, C.red);
  text(ctx, 'close: Sefi coils, not yet fires', px + bw + 6, yy, { color: C.inkMid });
  yy += 14;
  // her body: body words, each part's health, wounds and illness (BIO-08, BIO-09, BIO-13)
  head('Body');
  bodyDiagram(ctx, px + 2, yy, { t: 'hurt', L: 'hurt' });
  const lines = [['Strong, well fed.', null], ['Left hand: old burn, healed.', 'hurt'], ['Back: sore from lifting.', 'hurt'], ['No illness.', null]];
  lines.forEach(([s, st], i) => {
    if (st) rect(ctx, px + 34, yy + 4 + i * 11, 5, 5, HEALTH[st]);
    text(ctx, s, px + (st ? 43 : 34), yy + 2 + i * 11, { color: C.ink });
  });
  yy += BODY.length * 2 + 6;
  head('Recent talk');
  const tk = [[0, 'pots'], [1, 'goats'], [3, 'fire']];
  tk.forEach(([i, label], k) => {
    const bx = px + k * 62;
    rect(ctx, bx, yy, 18, 18, C.paper2);
    ctx.drawImage(ic.canvas, ...ic.at(i), bx + 1, yy + 1, 16, 16);
    text(ctx, label, bx + 22, yy + 5, { color: C.ink });
  });
  yy += 26;
  // the deeper views: her mind (PRE-14), her family (PRE-10), her crafts, and following her life (PRE-06)
  const tabs = ['Mind', 'Family', 'Crafts', 'Follow'];
  let tx2 = px;
  for (const t of tabs) { const tw = measure(t) + 12; rect(ctx, tx2, yy, tw, 15, C.paper2); rect(ctx, tx2, yy + 14, tw, 1, C.inkMid); text(ctx, t, tx2 + 6, yy + 4, { color: C.ink }); tx2 += tw + 6; }
}

// --- the book of ages -----------------------------------------------------------------------------------------------

const MARK = '#4f6b8c';   // your own acts, beside the story and never in it (GOD-07)

/**
 * The book of ages: a page of the journal with an ink drawing and the age's entries, upright and plain to read; the
 * handwriting is kept for the age's name, drawn at twice the size.
 */
async function book() {
  const w = 336, h = 748;
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  paper(ctx, 0, 0, w, h, 11);
  text(ctx, 'THE BOOK OF AGES', 16, 14, { color: C.inkMid });
  text(ctx, 'iii', w - 30, 14, { color: C.inkMid });
  // its pages: the ages, each people's timeline (CUL-23), those you follow (PRE-06), moments waiting (PRE-08), species (PRE-16)
  let x = 16;
  for (const [t, on] of [['Ages', true], ['Peoples', false], ['Followed', false], ['Waiting 2', false], ['Species', false]]) {
    const tw = measure(t) + 10;
    if (on) { rect(ctx, x, 26, tw, 13, C.paper2); rect(ctx, x, 38, tw, 1, C.ink); }
    text(ctx, t, x + 5, 29, { color: on ? C.ink : C.inkMid });
    x += tw + 4;
  }
  rect(ctx, 16, 40, w - 32, 1, C.line);
  drawText(ctx, 'The age of hesoru', 16, 50, { color: C.ink, scale: 2, hand: true, seed: 3 });
  text(ctx, 'fire from wood · from Year 31', 16, 72, { color: C.red });
  // the drawing: the shelter, in ink
  const { P } = await shelter({ light: 'golden', opts: ['wide'], view: { w: 304, h: 150, mpp: 0.06, target: [1, 2.4, -3.5] } });
  P.ink = true;
  const sk = await P.render(0);
  rect(ctx, 14, 86, 308, 154, C.inkMid);
  ctx.drawImage(sk, 16, 88);
  text(ctx, 'The shelter by the stream, as it was kept.', 16, 245, { color: C.inkMid });
  let y = 263;
  const entries = [
    ['Year 31, autumn, day 4', 'Ume of the Tavu carried a burning branch from the lightning fire to the shelter and kept it alive through three nights of rain. The band called it hesoru.'],
    ['Year 33, spring, day 18', 'The first hearth ringed with stones, so the fire stays where it is put.'],
    ['Year 38, winter, day 9', 'Meat dried over smoke lasted until the thaw for the first time.'],
    ['Year 44, winter, day 2', 'Ume died, aged 58. Her fire was carried to two new bands upstream.'],
    ['Year 51, summer, day 20', 'The Tavu painted a red deer on the shelter wall.'],
  ];
  entries.forEach(([when, what], i) => {
    text(ctx, when, 16, y, { color: C.red });
    y += 13;
    y = block(ctx, what, 16, y, w - 32, { color: C.ink, lineH: 13 }) + 6;
    if (i === 0) {
      // your act beside the entry, marked as yours, opening what came of it (GOD-07, GOD-09)
      rect(ctx, 16, y - 1, 2, 22, MARK);
      glyph(ctx, 28, y + 4, 'lightning', MARK);
      text(ctx, 'Yours: lightning on the oak by the stream,', 38, y, { color: MARK });
      text(ctx, 'Year 31, autumn, day 3.', 38, y + 11, { color: MARK });
      const lx = 38 + measure('Year 31, autumn, day 3.') + 6;
      text(ctx, 'What came of it', lx, y + 11, { color: MARK });
      rect(ctx, lx, y + 19, measure('What came of it'), 1, MARK);
      y += 26;
    }
    y += 3;
  });
  // a portrait of Ume, who kept the first fire, with what the records hold of her
  const ume = await portrait({ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'fur', necklace: 'shell', seed: 31 }, { w: 60, h: 76, mpp: 0.013, target: 1.1 });
  y += 2;
  rect(ctx, 14, y - 2, 64, 80, C.inkMid);
  ctx.drawImage(ume, 16, y);
  text(ctx, 'Ume, who kept the fire', 90, y + 14, { color: C.ink });
  text(ctx, 'of the Tavu, died Year 44, aged 58', 90, y + 30, { color: C.inkMid });
  text(ctx, 'taught the fire to Asi and Koro', 90, y + 46, { color: C.red });
  // a red hand stamp in the margin, as on the shelter wall
  const hand = ['.#.#.', '#####', '#####', '.###.', '.###.'];
  hand.forEach((row, j) => [...row].forEach((ch, i) => { if (ch === '#') rect(ctx, w - 44 + i * 3, h - 50 + j * 3, 3, 3, 'rgba(156,58,42,0.75)'); }));
  return canvasPngs(c, 4);
}

// --- the ring of powers ---------------------------------------------------------------------------------------------

/**
 * Long-press on Aru (GOD-10): time pauses, a ring shows the powers possible there, and those that are not say why
 * (GOD-11); a faint mark shows an act of yours still at work near by.
 */
async function powers(ctx, P, w, h, aru) {
  dim(ctx, w, h, 0.32);
  topLine(ctx, w, { paused: true });
  // your rain over the barley, still falling: its stretch and how long it has left
  const [fx, fy] = [266, 452];
  for (let a = 0; a < Math.PI * 2; a += 0.04) {
    const x = Math.round(fx + Math.cos(a) * 56), y = Math.round(fy + Math.sin(a) * 22);
    if ((x + y) % 4 === 0) rect(ctx, x, y, 1, 1, '#9fc2e8');
  }
  pill(ctx, 'your rain · till dusk', fx - 50, fy - 4, '#cfe0f4');
  glyph(ctx, fx + 48, fy + 1, 'rain', '#cfe0f4');
  ring(ctx, aru[0], aru[1] + 2, 9, 4);
  const cx = clamp(aru[0], 64, w - 64), cy = clamp(aru[1] - 18, 110, h - 230);
  const ringP = [['dream', 'Dream'], ['fortune', 'Fortune'], ['reveal', 'Reveal'], ['rain', 'Rain'], ['storm', 'Storm'], ['cold', 'Cold snap']];
  ringP.forEach(([g, name], i) => {
    const a = -Math.PI / 2 + (i * 2 * Math.PI) / ringP.length;
    const bx = cx + Math.round(Math.cos(a) * 46), by = cy + Math.round(Math.sin(a) * 46);
    button(ctx, bx, by, 11, g);
    pill(ctx, name, Math.round(bx - measure(name) / 2), by + 15);
  });
  // what the ring leaves out, and why
  const py = h - 158;
  darkPanel(ctx, 10, py, w - 20, 116, { corner: 3 });
  text(ctx, 'POWERS FOR ARU, HERE AND NOW', 20, py + 8, { color: C.accent });
  text(ctx, 'Time waits while you choose.', 20, py + 20, { color: C.dim });
  text(ctx, 'Not possible here:', 20, py + 38, { color: C.text });
  const out = [['lightning', 'Lightning: no storm overhead'], ['drought', 'Drought: no dry spells in spring'], ['flood', 'Flood: no river valley here'], ['quake', 'Quake: no fault near']];
  out.forEach(([g, s], i) => { glyph(ctx, 26, py + 55 + i * 13, g, C.dim); text(ctx, s, 36, py + 51 + i * 13, { color: C.dim }); });
  textButton(ctx, 'Cancel', w - 52, py + 98);
}

// --- the views the handle opens --------------------------------------------------------------------------------------

/** Swiping up the handle (PRE-33): the book of ages, those you follow, your acts, the six map overlays (PRE-07). */
function views(ctx, w, h) {
  dim(ctx, w, h, 0.3);
  const top = h - 352;
  darkPanel(ctx, 0, top, w, 352 + 4, { corner: 4 });
  handle(ctx, w / 2, top + 6);
  text(ctx, 'VIEWS', 16, top + 18, { color: C.accent });
  let y = top + 34;
  const row = (g, name, sub) => {
    button(ctx, 30, y + 13, 11, g);
    text(ctx, name, 50, y + 4, { color: C.text });
    text(ctx, sub, 50, y + 16, { color: C.dim });
    rect(ctx, 16, y + 29, w - 32, 1, C.edge);
    y += 32;
  };
  row('book', 'Book of ages', 'The age of hesoru · 7 ages · 2 moments waiting');
  row('person', 'Followed', 'Aru, Sefi and Tor');
  row('hand', 'Your acts', 'two at work: rain, a dream');
  y += 6;
  text(ctx, 'MAP OVERLAYS', 16, y, { color: C.accent });
  y += 14;
  const overlays = [['peoples', 'Peoples and lands'], ['craft', 'Who knows a craft'], ['belief', 'A belief'], ['reveal', 'What they know'], ['tree', 'Plants, water, herds'], ['rain', 'Weather and seasons']];
  overlays.forEach(([g, name], i) => {
    const x = 16 + (i % 2) * 156, yy = y + Math.floor(i / 2) * 30;
    button(ctx, x + 12, yy + 11, 10, g);
    text(ctx, name, x + 28, yy + 8, { color: C.text });
  });
  y += 96;
  rect(ctx, 16, y, w - 32, 1, C.edge);
  textButton(ctx, 'Worlds', w / 2 - 50, y + 22);
  textButton(ctx, 'Settings', w / 2 + 50, y + 22);
}

// --- peoples and territories on the map -------------------------------------------------------------------------------

const PEOPLES = [
  { name: 'Tavu', n: 31, mood: 'content' }, { name: 'Ketu', n: 24, mood: 'uneasy' }, { name: 'Sora', n: 18, mood: 'content', sick: 2 },
  { name: 'Amri', n: 40, mood: 'hungry' }, { name: 'Neshi', n: 12, mood: 'grieving' },
];
// a camp's mood as a small face (MND-29): '#' its outline and features, 'o' skin
const FACES = {
  content: ['.###.', '#ooo#', '##o##', '##o##', '#o#o#', '.###.'],
  uneasy: ['.###.', '#ooo#', '##o##', '#ooo#', '#####', '.###.'],
  hungry: ['.###.', '#ooo#', '##o##', '#o#o#', '#o#o#', '.###.'],
  grieving: ['.###.', '#ooo#', '##o##', '#o#o#', '##o##', '.###.'],
};
function face(ctx, x, y, mood) {
  FACES[mood].forEach((row, j) => [...row].forEach((ch, i) => { if (ch !== '.') rect(ctx, x + i, y + j, 1, 1, ch === 'o' ? '#e8c28a' : '#18131f'); }));
}
function cross(ctx, x, y) {
  for (const [i, j] of [[1, 0], [0, 1], [1, 1], [2, 1], [1, 2]]) rect(ctx, x + i, y + j, 1, 1, '#d84a3a');
  for (const [i, j] of [[1, -1], [0, 0], [2, 0], [-1, 1], [3, 1], [0, 2], [2, 2], [1, 3]]) rect(ctx, x + i, y + j, 1, 1, '#f4ece0');
}
function cairn(ctx, x, y) {
  const S = ['..o..', '.ooo.', 'ooooo'];
  S.forEach((r, j) => [...r].forEach((ch, i) => { if (ch === 'o') rect(ctx, x + i, y + j, 1, 1, (i + j) % 2 ? '#8d8796' : '#c9c2cf'); }));
  rect(ctx, x - 1, y + 3, 7, 1, '#18131f');
}
function oldCamp(ctx, x, y) {
  for (const [i, j] of [[1, 0], [2, 0], [3, 0], [0, 1], [4, 1], [1, 2], [2, 2], [3, 2]]) rect(ctx, x + i, y + j, 1, 1, '#5a3a2a');
  rect(ctx, x + 2, y + 1, 1, 1, '#2a2024');
}

/** The peoples-and-territories overlay on the region's map (PRE-07): each band's land, the sick, each camp's mood, graves and old camps. */
async function peoplesMap() {
  const w = 336, h = 748;
  const { cv, ctx, P, W } = await regionMap({ light: 'noon', w, h });
  const camps = regionCamps(W).map((c, i) => ({ ...c, ...PEOPLES[i] }));
  // each land pixel within 16 km of a camp is that band's, the nearest's where lands meet
  const own = new Int8Array(w * h).fill(-1), Rg = W.R;
  const campH = camps.map((c) => Rg.hgt[Math.floor((c.yk - Rg.y0) / Rg.cell) * Rg.w + Math.floor((c.xk - Rg.x0) / Rg.cell)]);
  for (let sy = 0; sy < h; sy++) for (let sx = 0; sx < w; sx++) {
    const [x, , z] = P.fromScreen(sx, sy, 0), xk = W.camp.xk + x / 1000, yk = W.camp.yk + z / 1000;
    const i = Math.floor((xk - Rg.x0) / Rg.cell), j = Math.floor((yk - Rg.y0) / Rg.cell);
    if (i < 0 || j < 0 || i >= Rg.w || j >= Rg.h || Rg.hgt[j * Rg.w + i] <= 0) continue;
    // a band ranges farther over easy ground than up into hills above its camp: its reach wanders with the land
    let best = -1, bd = 1e9;
    camps.forEach((c, k) => { const d = Math.hypot(c.xk - xk, c.yk - yk); if (d < bd) { bd = d; best = k; } });
    const reach = 16 * (0.7 + 0.6 * fbm(xk / 9 + 3, yk / 9 + 7, 2)) - Math.max(0, Rg.hgt[j * Rg.w + i] - campH[best] - 250) / 50;
    if (bd < reach) own[sy * w + sx] = best;
  }
  const img = ctx.getImageData(0, 0, w, h), D = img.data;
  const rgb = (hex) => [1, 3, 5].map((k) => parseInt(hex.slice(k, k + 2), 16));
  for (let sy = 0; sy < h; sy++) for (let sx = 0; sx < w; sx++) {
    const k = sy * w + sx, o = own[k];
    if (o < 0) continue;
    const [r, g, b] = rgb(camps[o].colour), q = k * 4;
    const edge = [[1, 0], [-1, 0], [0, 1], [0, -1]].some(([dx, dy]) => { const x = sx + dx, y = sy + dy; return x >= 0 && y >= 0 && x < w && y < h && own[y * w + x] !== o; });
    if (edge) { D[q] = r * 0.7; D[q + 1] = g * 0.7; D[q + 2] = b * 0.7; } else { D[q] = D[q] * 0.7 + r * 0.3; D[q + 1] = D[q + 1] * 0.7 + g * 0.3; D[q + 2] = D[q + 2] * 0.7 + b * 0.3; }
  }
  ctx.putImageData(img, 0, 0);
  const scr = (xk, yk) => P.toScreen([(xk - W.camp.xk) * 1000, 0, (yk - W.camp.yk) * 1000]);
  // graves and old camps, from the records (PRE-09)
  for (const [dx, dy] of [[3, -2], [-4.5, 5], [21, -29], [-36, 15]]) { const [x, y] = scr(W.camp.xk + dx, W.camp.yk + dy); cairn(ctx, x - 2, y - 2); }
  for (const [dx, dy] of [[7, 6], [-9, -4], [44, 22], [-14, 49]]) { const [x, y] = scr(W.camp.xk + dx, W.camp.yk + dy); oldCamp(ctx, x - 2, y - 1); }
  // each camp: its fire, its band's marker, its mood, and any sick
  for (const c of camps) {
    const [sx, sy] = scr(c.xk, c.yk);
    fireGlow(ctx, sx, sy);
    groupMarker(ctx, sx, sy - 3, { colour: c.colour });
    face(ctx, sx + 6, sy - 12, c.mood);
    if (c.sick) cross(ctx, sx + 6, sy - 4);
  }
  // the overlays, this one chosen
  darkPanel(ctx, 6, 8, w - 12, 22, { corner: 3 });
  let x = 14;
  for (const [t, on] of [['Peoples', true], ['Crafts', false], ['Beliefs', false], ['Known', false], ['Land', false], ['Weather', false]]) {
    text(ctx, t, x, 15, { color: on ? C.accent : C.dim });
    if (on) rect(ctx, x, 24, measure(t), 1, C.accent);
    x += measure(t) + 14;
  }
  // the key
  const py = h - 132;
  darkPanel(ctx, 6, py, w - 12, 124, { corner: 3 });
  text(ctx, 'PEOPLES AND THEIR LANDS · YEAR 214', 14, py + 8, { color: C.accent });
  camps.forEach((c, i) => {
    const yy = py + 24 + i * 13;
    rect(ctx, 14, yy, 7, 7, c.colour); rect(ctx, 14, yy + 7, 7, 1, '#18131f');
    face(ctx, 27, yy, c.mood);
    text(ctx, `${c.name} · ${c.n} people · ${c.mood}${c.sick ? ` · ${c.sick} sick` : ''}`, 38, yy, { color: C.text });
  });
  const ky = py + 94;
  cross(ctx, 15, ky + 2); text(ctx, 'sick', 22, ky, { color: C.dim });
  cairn(ctx, 52, ky + 2); text(ctx, 'grave', 60, ky, { color: C.dim });
  oldCamp(ctx, 96, ky + 3); text(ctx, 'old camp', 104, ky, { color: C.dim });
  fireGlow(ctx, 158, ky + 4); text(ctx, 'camp with fire', 164, ky, { color: C.dim });
  return canvasPngs(cv, 4);
}

// --- the new-world screen ---------------------------------------------------------------------------------------------

const COVER_NAME = {
  [BIOME.TUNDRA]: 'tundra', [BIOME.BOREAL]: 'pine forest', [BIOME.MIXED]: 'mixed forest', [BIOME.BROADLEAF]: 'broadleaf forest',
  [BIOME.GRASS]: 'grassland', [BIOME.STEPPE]: 'steppe', [BIOME.SCRUB]: 'scrub', [BIOME.DESERT]: 'desert', [BIOME.SAVANNA]: 'savanna',
  [BIOME.RAIN]: 'rainforest', [BIOME.ROCK]: 'bare mountains', [BIOME.WET]: 'wetland',
};
const NUM = ['no', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight', 'nine', 'ten'];

/** A world's one-line summary, from its cells alone (WLD-10): land, great lands, its widest covers, where the first band lives. */
function summary(G) {
  let land = 0;
  const cover = new Map(), isLand = new Uint8Array(G.n);
  for (let k = 0; k < G.n; k++) {
    const b = G.biome[k];
    if (b === BIOME.SEA || b === BIOME.ICE || G.hgt[k] <= 0) continue;
    land++; isLand[k] = 1; cover.set(b, (cover.get(b) || 0) + 1);
  }
  // great lands: land joined cell to cell (round the world east to west), each at least a twentieth of all land
  const seen = new Uint8Array(G.n), stack = [];
  let great = 0;
  for (let k0 = 0; k0 < G.n; k0++) {
    if (!isLand[k0] || seen[k0]) continue;
    let size = 0; stack.push(k0); seen[k0] = 1;
    while (stack.length) {
      const k = stack.pop(), x = k % G.w, y = Math.floor(k / G.w); size++;
      for (const [dx, dy] of [[1, 0], [-1, 0], [0, 1], [0, -1]]) {
        const yy = y + dy; if (yy < 0 || yy >= G.h) continue;
        const j = yy * G.w + ((x + dx + G.w) % G.w);
        if (isLand[j] && !seen[j]) { seen[j] = 1; stack.push(j); }
      }
    }
    if (size >= land / 20) great++;
  }
  const top = [...cover].sort((a, b) => b[1] - a[1]).slice(0, 2).map(([b]) => COVER_NAME[b]);
  const S = findStart(G), lat = latitude(S.yk), sb = G.biome[Math.floor(S.yk / G.cell) * G.w + Math.floor(S.xk / G.cell)];
  return `${Math.round((100 * land) / G.n)}% land in ${NUM[Math.min(great, 10)]} great lands, mostly ${top[0]} and ${top[1]}; the first band lives by a river in ${COVER_NAME[sb] || 'the hills'}, ${Math.abs(lat).toFixed(0)}° ${lat < 0 ? 'south' : 'north'}.`;
}

/** "New world" (WLD-10, PRE-40): the best three candidates as small globes, each with its summary. */
async function worlds() {
  const w = 336, h = 748;
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  rect(ctx, 0, 0, w, h, '#0b0e1f');
  let s = 7;
  const rnd = () => { s = (s * 16807) % 2147483647; return s / 2147483647; };
  for (let i = 0; i < 160; i++) rect(ctx, Math.floor(rnd() * w), Math.floor(rnd() * h), 1, 1, rnd() > 0.8 ? '#f4ece0' : '#7d7a99');
  text(ctx, 'NEW WORLD', 16, 18, { color: C.accent });
  block(ctx, 'The best three of the twelve worlds made. Pick one, let the game pick, or enter a seed.', 16, 32, w - 32, { color: C.dim });
  const seeds = [7, 12, 29];
  for (let i = 0; i < seeds.length; i++) {
    const G = worldGrid({ w: 1000, h: 500, seed: seeds[i] }), S = findStart(G);
    const y = 64 + i * 196;
    darkPanel(ctx, 8, y, w - 16, 186, { corner: 3 });
    const g = await globeOf(G, 112, { lat: latitude(S.yk) * 0.6, lon: (S.xk / WORLD.W) * 360 - 180 });
    ctx.drawImage(g, 14, y + 8);
    drawText(ctx, `World ${i + 1}`, 134, y + 12, { color: C.text, scale: 2 });
    text(ctx, `seed ${seeds[i]}`, 134, y + 32, { color: C.dim });
    block(ctx, summary(G), 134, y + 48, w - 150, { color: C.text });
    textButton(ctx, 'Choose', w - 52, y + 168);
  }
  textButton(ctx, 'Let the game pick', w / 2 - 66, h - 40, { active: true });
  textButton(ctx, 'Enter a seed', w / 2 + 74, h - 40);
  return canvasPngs(c, 4);
}

// --- the sheet of parts ---------------------------------------------------------------------------------------------

/** A sheet of the interface's parts: bubbles, time controls and the dial opened, powers, panels and the two fonts. */
async function sheet(ic) {
  const w = 748, h = 520;
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const ctx = c.getContext('2d');
  rect(ctx, 0, 0, w, h, '#1c1824');
  const label = (s, x, y) => text(ctx, s.toUpperCase(), x, y, { color: C.accent });
  label('Talk bubbles: a picture of the topic', 20, 18);
  const names = ['pots', 'goats', 'deer', 'fire', 'berries', 'dogs', 'flint', 'reindeer'];
  names.forEach((n, i) => { const x = 40 + i * 52; bubble(ctx, x, 70, ic.canvas, ic.at(i)); text(ctx, n, x - Math.round(measure(n) / 2), 80, { color: C.dim }); });
  const face1 = await portrait(ARU, { w: 16, h: 16, mpp: 0.026, ink: false, target: 1.42 });
  bubble(ctx, 40 + 8 * 52, 70, face1, [0, 0, 16, 16]); text(ctx, 'a person', 40 + 8 * 52 - 18, 80, { color: C.dim });
  // time: pause, play (speed back to zoom), the dial, the lock, skip to the next moment (TIM-04, TIM-11)
  label('Time controls', 20, 110);
  controls(ctx, 96, 140);
  [['pause', -58], ['play', -29], ['dial', 0], ['lock', 29], ['skip', 58]].forEach(([n, dx]) => text(ctx, n, 96 + dx - Math.round(measure(n) / 2), 160, { color: C.dim }));
  handle(ctx, 96, 180); text(ctx, 'the handle: views', 96 - Math.round(measure('the handle: views') / 2), 188, { color: C.dim });
  label('A live moment', 320, 110);
  const lm = document.createElement('canvas'); lm.width = 300; lm.height = 60;
  liveMoment(lm.getContext('2d'), 300, ic);
  ctx.drawImage(lm, 310, 116);
  // the dial opened: from real speed to top speed, each zoom stop's speed marked (TIM-01)
  label('The dial opened', 20, 214);
  darkPanel(ctx, 20, 228, 708, 50, { corner: 3 });
  const stops = [['real', 'person'], ['an hour a minute', 'close camp'], ['a day in a few minutes', 'camp'], ['a season a minute', 'valley'], ['a few years a minute', 'region'], ['top speed', 'map, globe']];
  rect(ctx, 50, 246, 648, 2, C.edge);
  stops.forEach(([sp, z], i) => {
    const x = 50 + Math.round((i * 648) / (stops.length - 1));
    rect(ctx, x - 1, 242, 3, 10, i === 1 ? C.accent : C.dim);
    const tx = clamp(x - Math.round(measure(sp) / 2), 28, 720 - measure(sp));
    text(ctx, sp, tx, 232, { color: i === 1 ? C.accent : C.text });
    const zx = clamp(x - Math.round(measure(z) / 2), 28, 720 - measure(z));
    text(ctx, z, zx, 258, { color: C.dim });
  });
  rect(ctx, 50 + Math.round(648 * 0.2) - 2, 244, 5, 6, C.accent);
  // the powers' marks (GOD-02 to GOD-13), as the ring shows them; dim: not possible here
  label('Powers in the ring', 20, 296);
  const pw = [['lightning', 'lightning'], ['rain', 'rain'], ['storm', 'storm'], ['drought', 'drought'], ['cold', 'cold snap'], ['flood', 'flood'], ['quake', 'quake'], ['dream', 'dream'], ['fortune', 'fortune'], ['reveal', 'revelation']];
  pw.forEach(([g, n], i) => { const x = 40 + i * 62; button(ctx, x, 322, 11, g, { dim: i === 6 }); text(ctx, n, x - Math.round(measure(n) / 2), 338, { color: C.dim }); });
  label('Quiet panel', 20, 362);
  darkPanel(ctx, 20, 376, 200, 50);
  text(ctx, DATE, 30, 384, { color: C.text });
  text(ctx, SPEED, 30, 396, { color: C.accent });
  text(ctx, 'Tavu · 47 people · 3 houses', 30, 410, { color: C.dim });
  label('Journal paper', 250, 362);
  paper(ctx, 250, 376, 200, 50, 5);
  drawText(ctx, 'Aru', 260, 382, { color: C.ink, scale: 2, hand: true, seed: 4 });
  text(ctx, '"river stone"', 300, 388, { color: C.inkMid });
  text(ctx, 'Woman, 31 · the Tavu', 260, 410, { color: C.ink });
  label('Plain pixel font', 20, 444);
  text(ctx, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 20, 458, { color: C.text });
  text(ctx, 'abcdefghijklmnopqrstuvwxyz 0123456789', 20, 470, { color: C.text });
  text(ctx, 'The Tavu keep goats and sow grain by the river.', 20, 484, { color: C.dim });
  drawText(ctx, 'Titles at 2x', 20, 496, { color: C.text, scale: 2 });
  label('Pixel handwriting: big titles only', 400, 444);
  drawText(ctx, 'The age of hesoru', 400, 460, { color: C.bubble, scale: 2, hand: true, seed: 6 });
  drawText(ctx, 'Aru  Ume  Sefi', 400, 486, { color: C.bubble, scale: 2, hand: true, seed: 7 });
  return canvasPngs(c, 3);
}
