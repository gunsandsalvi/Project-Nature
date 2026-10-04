// The near zoom stops (PRE-03): the camp, a few hundred metres across, where people are tiny outlined figures
// (PRE-28); the close camp, every figure in full; and one person chipping flint. All three show the same camp of 30
// under the same cliff by the same stream, on the ground the valley stop shows from 10 km.
import * as THREE from 'three';
import { Painter } from '../engine.js';
import { Solid, Cards, Puffs, PAT, FLAG, Rand, M4, newObj, noise2 } from '../geo.js';
import { TILE } from '../atlas.js';
import { K } from '../kit/k.js';
import { blockCliff, boulder, stoneBlock } from '../kit/land.js';
import { broadleaf, conifer, bush, scatter } from '../kit/plants.js';
import { person } from '../kit/people.js';
import { animal } from '../kit/animals.js';
import { hearth, smoke, tent, windbreak, rack, log } from '../kit/things.js';
import { put, decal } from '../kit/scene.js';
import { groundView, farTree, fireGlow, tinyFigure, riverRibbons } from '../kit/far.js';
import { zoomLand } from '../kit/zoomworld.js';
import { RAMPS } from '../palette.js';

const moodFor = (light) => (light === 'dusk' ? 'dusk' : 'noon');

// the band: 30 people, placed in the camp's frame (u along the cliff, dv out from the cliff's foot, or v when abs)
const BAND = [
  // under the overhang
  [{ kind: 'elder', skin: 'skin2', hair: 'hairgrey', hairStyle: 'long', top: 'wrap', topMat: 'leather', pose: 'sitTalk' }, 1.0, 2.6, Math.PI, 'seat'],
  [{ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'hide', necklace: 'shell', pose: 'sitFloor' }, -0.6, 0.5, 1.2],
  [{ kind: 'child', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'sitFloor' }, -0.4, 1.6, 1.6],
  [{ kind: 'man', skin: 'skin2', hairStyle: 'short', beard: true, top: 'tunic', topMat: 'leather', pose: 'knap', held: 'stone' }, 4.2, 3.6, -0.7],   // in the sun at the shelter's mouth
  [{ kind: 'woman', skin: 'skin3', hairStyle: 'bun', top: 'tunic', topMat: 'dyedred', pose: 'scrape' }, -3.3, 0.3, 0.4],
  [{ kind: 'youth', skin: 'skin1', hair: 'hairred', hairStyle: 'topknot', top: 'none', legs: 'leggings', paint: 'ochre', pose: 'reach' }, 5.7, 1.3, -0.4],
  [{ kind: 'woman', skin: 'skin1', hairStyle: 'bun', top: 'dress', topMat: 'leather', pose: 'sitFloor', held: { kind: 'baby', skin: 'skin1' } }, 3.8, 2.4, -2.9],
  [{ kind: 'elder', skin: 'skin1', hair: 'hairgrey', hairStyle: 'bun', top: 'dress', topMat: 'fur', pose: 'sitFloor' }, -2.1, 1.9, 0.9],
  [{ kind: 'youth', skin: 'skin2', hairStyle: 'braid', top: 'tunic', topMat: 'hide', pose: 'carry', held: 'bundle' }, 6.6, 3.6, -2.2],
  [{ kind: 'man', skin: 'skin3', hairStyle: 'long', top: 'wrap', topMat: 'fur', pose: 'kneel', held: { kind: 'spear', tilt: 1.2 } }, -4.7, 1.3, 0.8],
  [{ kind: 'child', skin: 'skin3', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'armsUp' }, 0.2, 4.6, 2.6],
  // in front, down to the stream
  [{ kind: 'woman', skin: 'skin2', hairStyle: 'long', top: 'dress', topMat: 'hide', pose: 'walkA', held: 'basket' }, -6.0, 12, 0.2, 'abs'],
  [{ kind: 'child', skin: 'skin1', hairStyle: 'long', top: 'none', legs: 'bare', feet: null, pose: 'stoop' }, -3.5, 13.4, 0.3, 'abs'],
  [{ kind: 'child', skin: 'skin3', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'point' }, -2.2, 12.6, -0.8, 'abs'],
  [{ kind: 'woman', skin: 'skin3', hairStyle: 'braid', top: 'dress', topMat: 'leather', pose: 'kneel', held: 'pot' }, 1.6, 8.0, 3.0, 'abs'],
  [{ kind: 'man', skin: 'skin2', hairStyle: 'short', top: 'tunic', topMat: 'hide', pose: 'shoulder', held: 'bundle' }, 9.0, 6.5, -2.4, 'abs'],
  // hunters coming home with meat
  [{ kind: 'man', skin: 'skin2', hairStyle: 'long', top: 'tunic', topMat: 'hide', pose: 'walkA', held: { kind: 'spear', tilt: 0.25 } }, 11.6, 26, -2.5, 'abs'],
  [{ kind: 'man', skin: 'skin3', hairStyle: 'short', top: 'wrap', topMat: 'fur', pose: 'carry', held: 'meat' }, 13.3, 27.6, -2.4, 'abs'],
  [{ kind: 'youth', skin: 'skin2', hairStyle: 'braid', top: 'tunic', topMat: 'leather', pose: 'walkB', held: { kind: 'spear', tilt: 0.3 } }, 15.0, 29.2, -2.3, 'abs'],
  // by the tents
  [{ kind: 'woman', skin: 'skin1', hairStyle: 'long', top: 'dress', topMat: 'hide', pose: 'hold' }, -11.2, 7.4, 0.6, 'abs'],
  [{ kind: 'man', skin: 'skin2', hairStyle: 'topknot', top: 'none', legs: 'leggings', pose: 'crouch' }, -13.6, 3.2, 1.0, 'abs'],
  [{ kind: 'elder', skin: 'skin3', hair: 'hairgrey', hairStyle: 'short', top: 'wrap', topMat: 'hide', pose: 'sitFloor' }, -9.0, 1.8, 1.4, 'abs'],
  // on the cliff top, looking out
  [{ kind: 'man', skin: 'skin1', hairStyle: 'long', top: 'tunic', topMat: 'hide', pose: 'point', held: { kind: 'spear', tilt: 0.1 } }, -9.0, -14, 0.3, 'abs'],
  [{ kind: 'youth', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'leggings', pose: 'stand' }, -7.4, -14.6, 0.1, 'abs'],
  // children running in the meadow
  [{ kind: 'child', skin: 'skin2', hairStyle: 'long', top: 'none', legs: 'bare', feet: null, pose: 'walkA' }, 4.0, 18, 1.8, 'abs'],
  [{ kind: 'child', skin: 'skin1', hairStyle: 'short', top: 'tunic', topMat: 'leather', pose: 'walkB' }, 5.6, 19.4, 1.7, 'abs'],
  // at the river, fishing (u along the bank, out from the water's edge), and two on the path to it (how far along)
  [{ kind: 'man', skin: 'skin3', hairStyle: 'long', top: 'none', legs: 'leggings', pose: 'aim', held: { kind: 'spear', tilt: 0.9 } }, 40, 0.8, 0.2, 'bank'],
  [{ kind: 'youth', skin: 'skin2', hairStyle: 'short', top: 'none', legs: 'bare', feet: null, pose: 'stoop' }, 52, -0.6, -0.4, 'bank'],
  [{ kind: 'woman', skin: 'skin2', hairStyle: 'braid', top: 'dress', topMat: 'hide', pose: 'walkA', held: 'basket' }, 0.13, 0.5, 0.3, 'path'],
  [{ kind: 'woman', skin: 'skin3', hairStyle: 'bun', top: 'dress', topMat: 'leather', pose: 'walkB', held: 'pot' }, 0.17, -0.4, 0.2, 'path'],
];

/** The big river's near bank, going out from the cliff at u: the water's edge in v. */
function bankV(Lnd, u) {
  for (let v = 40; v < 900; v += 0.25) { const r = Lnd.bigRiver(u, v); if (r && r.d <= r.hw) return v; }
  return 280;
}

// the path the band has worn from the camp to the river, ending at the water's edge
let PATH = null;
function pathPts(Lnd) {
  if (PATH) return PATH;
  const end = [40, bankV(Lnd, 40) + 0.4];
  PATH = [[2, 6], [6, 40], [14, 90], [24, 140], [31, 190]].filter((p) => p[1] < end[1] - 25).concat([end]);
  return PATH;
}
function pathDist(u, v) {
  let d = 1e9;
  for (let i = 0; i < PATH.length - 1; i++) {
    const [ax, az] = PATH[i], [bx, bz] = PATH[i + 1], vx = bx - ax, vz = bz - az, L2 = vx * vx + vz * vz;
    const t = Math.max(0, Math.min(1, ((u - ax) * vx + (v - az) * vz) / L2));
    d = Math.min(d, Math.hypot(u - ax - vx * t, v - az - vz * t));
  }
  return d;
}
/** The point a share f of the way along the path, moved off it sideways by side metres. */
function alongPath(f, side) {
  const L = PATH.slice(1).map((p, i) => Math.hypot(p[0] - PATH[i][0], p[1] - PATH[i][1]));
  let s = f * L.reduce((a, b) => a + b, 0), i = 0;
  while (i < L.length - 1 && s > L[i]) s -= L[i++];
  const [ax, az] = PATH[i], [bx, bz] = PATH[i + 1], t = s / L[i], dx = (bx - ax) / L[i], dz = (bz - az) / L[i];
  return [ax + (bx - ax) * t + dz * side, az + (bz - az) * t - dx * side];
}

/** Where each of the band stands: [u, y, v, yaw, spec]. */
function bandPlaces(Lnd) {
  pathPts(Lnd);
  return BAND.map(([spec, a, b, yaw, mode], i) => {
    let u = a, v;
    if (mode === 'abs') v = b;
    else if (mode === 'bank') v = bankV(Lnd, a) - b;
    else if (mode === 'path') [u, v] = alongPath(a, b);
    else v = Lnd.cliffV(u) + b;
    let y = Lnd.height(u, v, true) + (mode === 'seat' ? 0.34 : 0);
    if (mode === 'bank') { const r = Lnd.nearRiver(u, v); if (r && r.d < r.hw) y = Math.max(y, r.level - 0.35); }   // wading in the shallows
    return { u, v, y, yaw, spec: { ...spec, seed: 10 + i }, seat: mode === 'seat' };
  });
}

/** Ground layers: meadow, the worn camp floor and path, the river's mud, the woods' floor. */
function weights(Lnd) {
  pathPts(Lnd);
  return (u, v, y) => {
    const camp = Math.hypot((u - 1) / 9, (v - Lnd.cliffV(u) - 1.2) / 3.6) < 1 ? 1 : 0;
    const path = pathDist(u, v) < 0.9 + 0.3 * noise2(u * 0.3, v * 0.3) ? 1 : 0;
    const r = Lnd.nearRiver(u, v), wet = r && r.d < r.hw + (r.area >= 500 ? 6 : 0.5) ? 1 : 0;
    const woods = Lnd.cover(u, v).forest > 0.5 ? 1 : 0;
    const dirt = Math.max(camp, path) * (1 - wet);
    return [(1 - dirt) * (1 - wet) * (1 - woods), dirt, wet, woods * (1 - wet) * (1 - dirt)];
  };
}
const LAYERS = [['meadow', PAT.GROUND], ['dirt', PAT.DIRT], ['mud', PAT.DIRT], ['grass', PAT.GROUND]];

/** The cliff in pieces along its line, each piece standing on the ground at its foot, the shelter in the middle. */
function addCliff(P, Lnd, u0, u1, r) {
  const BEDS = [1.7, 1.2, 1.9, 1.3, 1.6, 1.4], MATS = ['rock', 'lime', 'rock', 'rock', 'lime', 'rock'];
  const pieces = [];
  for (let a = Math.floor(u0 / 40) * 40 - 20; a < u1; a += 40) pieces.push([a, a + 40]);
  let mid = null;
  for (const [a, b] of pieces) {
    const um = (a + b) / 2;
    if (Lnd.cliffH(um) < 1) continue;
    const base = Math.min(Lnd.height(a, Lnd.cliffV(a) + 0.6), Lnd.height(b, Lnd.cliffV(b) + 0.6), Lnd.height(um, Lnd.cliffV(um) + 0.6)) - 0.3;
    // the beds reach the plateau's ground, so the turf on top runs on into it
    const k = (Lnd.height(um, Lnd.cliffV(um) - 9, true) - base) / 9.1;
    const path = []; for (let u = a - 1; u <= b + 1.01; u += 2) path.push([u, Lnd.cliffV(u)]);
    const hasShelter = a <= 1 && b >= 1;
    const C = blockCliff({ seed: 21 + a, path, base, beds: BEDS.map((t) => t * k), mat: 'rock', grass: 'meadow', bedMats: MATS,
      topAt: (x) => Lnd.height(x, Lnd.cliffV(x) - Lnd.rimBack(x) - 1, true) - 0.08,
      overhang: hasShelter ? { bed: 3, from: -5.5 - a + 1, to: 7.5 - a + 1, out: 2.4, recess: 1.6 } : null });
    P.addSolid(C.solid);
    if (hasShelter) mid = { C, a, base };
  }
  return mid;
}

/** Trees over an area: woods where the land's cover says so, a few in the meadows; far ones as round crowns. */
function addTrees(P, Lnd, area, { near = null, nearR = 0, cell = 7, seed = 5, hole = null }) {
  const r = new Rand(seed), far = new Solid();
  let nFar = 0;
  for (let u = area[0]; u < area[1]; u += cell) for (let v = area[2]; v < area[3]; v += cell) {
    const x = u + r.range(0, cell), z = v + r.range(0, cell), c = Lnd.cover(x, z);
    if (hole && x >= hole[0] && x < hole[1] && z >= hole[2] && z < hole[3]) continue;
    const keep = c.forest > 0.5 ? r.chance(0.8) : r.chance(0.02);
    if (!keep) continue;
    if (Math.abs(z - Lnd.cliffV(x)) < 5 || (Math.abs(x) < 26 && z > Lnd.cliffV(x) - 3 && z < 36)) continue;   // the cliff face and the camp
    const rv = Lnd.nearRiver(x, z); if (rv && rv.d < rv.hw + 3) continue;
    if (pathDist(x, z) < 3) continue;
    const pine = r.chance(c.pine), h = r.range(7, 13), y = Lnd.height(x, z);
    if (near && Math.hypot(x - near[0], z - near[1]) < nearR) {
      put(P, pine ? conifer({ seed: 300 + nFar, h: h * 1.1 }) : broadleaf({ seed: 300 + nFar, h }), x, z, { y });
    } else far.add(farTree({ seed: 300 + nFar, h, kind: pine ? 'pine' : 'broad' }).solid, M4.T(x, y, z));
    nFar++;
  }
  P.addSolid(far);
}

/** The camp's things: hearth, windbreak, racks, log, furs, tents, stones at the cliff's foot, rock art. */
function addCamp(P, Lnd, light, shelter, detail) {
  const cz = (u) => Lnd.cliffV(u), at = (u, dv) => [u, cz(u) + dv];
  const H = hearth({ seed: 5, level: light === 'dusk' ? 4 : 3 });
  if (light !== 'dusk') { H.fire[3] *= 0.45; H.fire[4] *= 0.5; }
  const hp = at(1, 0.9);
  put(P, H, hp[0], hp[1], { y: Lnd.height(...hp, true) });
  const ground = (u, v) => Lnd.height(u, v, true);
  put(P, windbreak({ seed: 3, len: 3.4 }), ...at(-5.2, 1.6), { ground, yaw: 0.6 });
  put(P, rack({ seed: 4, len: 2.4, hang: 'meat' }), ...at(6.4, 2.0), { ground, yaw: -0.3 });
  put(P, log({ len: 2.2, r: 0.17 }), ...at(1.3, 2.6), { ground, yaw: 0.1 });
  put(P, tent({ seed: 6, r: 1.7, h: 3.0 }), -11.0, 4.6, { ground, yaw: 0.5 });
  put(P, tent({ seed: 7, r: 1.5, h: 2.7 }), -14.6, 7.2, { ground, yaw: 0.2 });
  put(P, rack({ seed: 8, len: 2.0, hang: 'hide' }), -10.4, 8.6, { ground, yaw: 0.3 });
  const furs = new Solid();
  for (const [x, dz, a] of [[-2.6, -0.4, 0.2], [-0.9, -0.6, -0.1], [4.4, -0.3, 0.3]]) {
    const [u, v] = at(x, dz);
    furs.box(M4.mul(M4.T(u, ground(u, v) + 0.05, v), M4.R(0, a, 0)), 1.5, 0.1, 0.9, { mat: 'fur', pat: PAT.FUR, obj: newObj() });
  }
  P.addSolid(furs);
  if (detail) {
    // berry bushes in the meadow, and a hide pegged out to dry
    for (const [u, v, s, i] of [[-9, 42, 1.3, 0], [-7.2, 44.6, 1.0, 1], [-10.6, 45.4, 1.2, 2], [-8.4, 47.2, 0.8, 3], [9.5, 50, 1.2, 4], [11, 52.5, 0.9, 5]]) {
      put(P, bush({ seed: 700 + i, size: s, berries: i < 4 ? 'berry' : null }), u, v, { ground });
    }
    const hide = new Solid(), hv = 31;
    hide.box(M4.mul(M4.T(7, ground(7, hv) + 0.03, hv), M4.R(0, 0.35, 0)), 1.7, 0.04, 1.2, { mat: 'hide', pat: PAT.HIDE, obj: newObj() });
    for (const [a, b] of [[-0.9, -0.65], [0.9, -0.65], [-0.9, 0.65], [0.9, 0.65], [0, -0.7], [0, 0.7]]) {
      const c = Math.cos(0.35), s = Math.sin(0.35), u = 7 + a * c + b * s, v = hv - a * s + b * c;
      hide.box(M4.T(u, ground(u, v) + 0.08, v), 0.04, 0.16, 0.04, { mat: 'wood', obj: newObj() });
    }
    P.addSolid(hide);
    P.addSoot([1.2, Lnd.H0 + 4.6, cz(1.2) + 0.4], 3.6);
    const art = new Solid(), wallAt = (x) => cz(x) - 1.6 + 0.06;
    const hand = ['.#.#.', '#####', '#####', '.###.', '.###.'];
    for (const [x, y] of [[-2.8, 2.1], [-2.4, 2.4], [3.6, 2.0], [4.0, 2.3]]) decal(art, hand, { origin: [x, shelter.base + y, wallAt(x)], right: [1, 0, 0], up: [0, 1, 0], normal: [0, 0, 1], cell: 0.05, mat: 'dyedred' });
    P.addSolid(art);
    const r = new Rand(9), stones = new Solid();
    for (let i = 0; i < 40; i++) {
      const u = r.range(-40, 40); if (u > -8 && u < 10) continue;
      const v = cz(u) + r.range(0.6, 3.0), s = r.range(0.25, 0.8);
      stones.add(boulder({ seed: 500 + i, size: [s * 1.5, s * 1.1, s * 1.2], detail: s > 0.5 ? 1 : 0, mat: r.pick(['rock', 'lime']), moss: 0.4 }), M4.T(u, ground(u, v) - 0.05, v));
    }
    P.addSolid(stones);
  }
  return hp;
}

/** The camp's stream and the river, the river's level the mirror's (it is the bigger water). */
function addWaters(P, Lnd, minW) {
  const main = Lnd.lines.filter((pts) => pts[0][2] >= 500);
  const lv = Lnd.nearRiver(55, 395);
  P.waterLevel = lv ? lv.level : Lnd.H0 - 3;
  P.addWater(riverRibbons(Lnd.lines, Lnd.lines.map((pts) => Lnd.waterLevel(pts)), minW));
  void main;
}

/** The camp (a few hundred metres): the ground, the cliff, woods as round crowns, people as tiny figures. */
export async function campStop({ light, w, h }) {
  const Lnd = zoomLand();
  K.mpp = 0.9;
  const P = new Painter({ w, h, mpp: 0.9, yaw: 16, elev: 34, target: [6, Lnd.H0 - 3, 110], mood: moodFor(light), depth: 3000,
    bounds: { c: [0, Lnd.H0, 0], r: 900 }, shadowSize: 4096 });
  P.haze = [P.camDepth + 150, P.camDepth + 1400];
  const u0 = -230, u1 = 240;
  P.addSolid(groundView(P, { height: (u, v) => Lnd.height(u, v, true), weights: weights(Lnd), step: 1.8, stepD: 2.0, ymin: Lnd.H0 - 40, ymax: Lnd.H0 + 60 }), LAYERS);
  const shelter = addCliff(P, Lnd, u0, u1);
  addTrees(P, Lnd, [u0, u1, -700, 900], { cell: 8 });
  const hp = addCamp(P, Lnd, light, shelter, false);
  addWaters(P, Lnd, 1);
  const puffs = new Puffs();
  smoke(puffs, { at: [hp[0], Lnd.H0 + 1, hp[1]], height: 22, drift: [0.55, 0.3], seed: 3, size: 1.1, tone: 5, alpha: 0.42 });
  P.addPuffs(puffs);
  const cv = await P.render(0), ctx = cv.getContext('2d');
  // the band as tiny outlined figures in their strongest colours (PRE-28)
  // far ones first, so near ones stand in front
  const figs = bandPlaces(Lnd).map((b) => ({ b, s: P.toScreen([b.u, b.y, b.v]) })).sort((p, q) => p.s[1] - q.s[1]);
  for (const { b, s } of figs) {
    const top = b.spec.top === 'none' ? (b.spec.paint || b.spec.skin) : b.spec.topMat || 'hide';
    const legs = b.spec.legs === 'bare' ? RAMPS[b.spec.skin][3] : RAMPS[b.spec.legMat || 'leather'][2];
    tinyFigure(ctx, s[0], s[1], { skin: RAMPS[b.spec.skin][5], body: RAMPS[top][5], legs, edge: RAMPS[top][1], child: b.spec.kind === 'child' });
  }
  // deer grazing across the river
  for (const [u, v] of [[-60, 520], [-52, 528], [-44, 515], [-70, 534], [-38, 540]]) {
    const [sx, sy] = P.toScreen([u, Lnd.height(u, v), v]);
    deerTiny(ctx, sx, sy);
  }
  fireGlow(ctx, ...P.toScreen([hp[0], Lnd.height(...hp, true) + 0.3, hp[1]]), { big: light === 'dusk' });
  return cv;
}

function deerTiny(ctx, x, y) {
  const c = RAMPS.deer[4], o = '#18131f';
  const S = ['.oooo.o', 'occcco.', '.o..o..'];
  S.forEach((row, j) => [...row].forEach((ch, i) => { if (ch !== '.') { ctx.fillStyle = ch === 'o' ? o : c; ctx.fillRect(x - 3 + i, y - 2 + j, 1, 1); } }));
  ctx.fillStyle = c; ctx.fillRect(x - 2, y - 1, 3, 1); ctx.fillRect(x + 1, y - 2, 1, 1);
}

/** The close camp and one person: every figure and thing in full, the woods near by in full, far ones as crowns. */
export async function nearStop({ light, w, h, stop, extra = 0 }) {
  const Lnd = zoomLand(), person1 = stop === 'person';
  const mpp = person1 ? 0.024 : 0.09;
  K.mpp = mpp;
  const knapper = bandPlaces(Lnd)[3];
  const target = person1 ? [knapper.u, knapper.y + 0.7, knapper.v - 0.6] : [0, Lnd.H0 + 2.4, 6];
  const P = new Painter({ w, h, mpp, yaw: 16, elev: 30, target, mood: moodFor(light), depth: 1000,
    bounds: { c: [0, Lnd.H0, 0], r: person1 ? 30 : 70 }, shadowSize: 4096 });
  P.haze = [P.camDepth + 20, P.camDepth + 160];
  const span = w * mpp, deep = (h * mpp) / Math.sin(Math.PI / 6);
  P.addSolid(groundView(P, { height: (u, v) => Lnd.height(u, v, true), weights: weights(Lnd), step: person1 ? 0.12 : 0.4, stepD: person1 ? 0.14 : 0.45,
    ymin: Lnd.H0 - 6, ymax: Lnd.H0 + 12, extra }), LAYERS);
  const shelter = addCliff(P, Lnd, target[0] - span - extra, target[0] + span + extra);
  const woods = [target[0] - span - 30, target[0] + span + 30, target[2] - deep - 60, target[2] + deep * 0.5];
  addTrees(P, Lnd, woods, { near: [target[0], target[2] - 10], nearR: 60, cell: 7 });
  if (extra) {
    addTrees(P, Lnd, [woods[0] - extra, woods[1] + extra, woods[2] - extra, woods[3] + extra], { near: [target[0], target[2] - 10], nearR: 60, cell: 7, seed: 6, hole: woods });
  }
  const hp = addCamp(P, Lnd, light, shelter, true);
  addWaters(P, Lnd, 0);
  // the band in full
  const band = bandPlaces(Lnd);
  for (const b of band) put(P, person(b.spec), b.u, b.v, { y: b.y, yaw: b.yaw });
  // the knapper's hide, the flakes he has struck (chips of flint a few centimetres long) and his core
  const kn = band[3], kit = new Solid(), fr = new Rand(31), fx = Math.sin(kn.yaw), fz = Math.cos(kn.yaw);
  kit.box(M4.mul(M4.T(kn.u, Lnd.height(kn.u, kn.v, true) + 0.02, kn.v), M4.R(0, kn.yaw, 0)), 1.1, 0.04, 0.9, { mat: 'hide', pat: PAT.HIDE, obj: newObj() });
  for (let i = 0; i < 16; i++) {
    const d = fr.range(0.35, 0.9), a = fr.range(-0.9, 0.9), u = kn.u + (fx * Math.cos(a) - fz * Math.sin(a)) * d, v = kn.v + (fz * Math.cos(a) + fx * Math.sin(a)) * d, s = fr.range(0.03, 0.07);
    kit.box(M4.mul(M4.T(u, Lnd.height(u, v, true) + 0.05, v), M4.R(fr.range(-0.3, 0.3), fr.range(0, 3), 0)), s * 1.4, 0.015, s, { mat: 'flint', bias: fr.chance(0.4) ? 1 : 0, obj: newObj() });
  }
  kit.box(M4.mul(M4.T(kn.u + fx * 0.3, Lnd.height(kn.u, kn.v, true) + 0.1, kn.v + fz * 0.3), M4.R(0.2, 0.5, 0.1)), 0.14, 0.1, 0.11, { mat: 'flint', obj: newObj() });
  P.addSolid(kit);
  put(P, animal({ species: 'dog', pose: 'lie', seed: 20 }), 2.6, Lnd.cliffV(2.6) + 2.9, { y: Lnd.height(2.6, Lnd.cliffV(2.6) + 2.9, true), yaw: 2.2 });
  // grass, flowers and reeds
  const low = new Cards(), r = new Rand(4);
  const place = (u, v) => [Lnd.height(u, v, true), [0, 1, 0]];
  const open = (u, v) => Lnd.cover(u, v).forest < 0.5 && v > Lnd.cliffV(u) + 1.2 && Math.hypot((u - 1) / 10, (v - Lnd.cliffV(u) - 1.2) / 4.2) > 1 && pathDist(u, v) > 1.4;
  const area = [target[0] - span, target[0] + span, Lnd.cliffV(target[0]) + 1, target[2] + deep * 0.5];
  scatter(low, { r, area, count: person1 ? 260 : 1400, tiles: TILE.TUFT, mat: 'meadow', place, bias: 1, accept: (u, v) => open(u, v) && Math.abs(v - 10) > 2.4 });
  scatter(low, { r, area, count: person1 ? 60 : 300, tiles: TILE.FLOWER, mat: (u, v) => (noise2(u * 0.2, v * 0.2) > 0.5 ? 'flowerp' : 'flowery'), place, accept: open });
  // reeds in clumps along the stream's banks, sedge between
  const streamAt = (u) => Lnd.stream.reduce((m, p) => (Math.abs(p[0] - u) < Math.abs(m[0] - u) ? p : m))[1];
  scatter(low, { r, area, count: person1 ? 50 : 320, tiles: [TILE.SEDGE, TILE.REED, TILE.SEDGE], mat: 'reed', place,
    accept: (u, v) => { const d = Math.abs(v - streamAt(u)); return d > 1.7 && d < 2.6 + 1.6 * noise2(u * 0.4, v * 0.4) && noise2(u * 0.25 + 3, 7) > 0.4; } });
  if (extra) {
    // the same again in the margin, by its own chance, so the picture's own tufts stay where they were
    const r2 = new Rand(5), ring = [area[0] - extra, area[1] + extra, area[2], area[3] + extra];
    const out = (u, v) => !(u >= area[0] && u < area[1] && v >= area[2] && v < area[3]);
    const k = ((ring[1] - ring[0]) * (ring[3] - ring[2])) / ((area[1] - area[0]) * (area[3] - area[2])) - 1;
    scatter(low, { r: r2, area: ring, count: Math.round(1400 * k), tiles: TILE.TUFT, mat: 'meadow', place, bias: 1, accept: (u, v) => out(u, v) && open(u, v) && Math.abs(v - 10) > 2.4 });
    scatter(low, { r: r2, area: ring, count: Math.round(300 * k), tiles: TILE.FLOWER, mat: (u, v) => (noise2(u * 0.2, v * 0.2) > 0.5 ? 'flowerp' : 'flowery'), place, accept: (u, v) => out(u, v) && open(u, v) });
    scatter(low, { r: r2, area: ring, count: Math.round(320 * k), tiles: [TILE.SEDGE, TILE.REED, TILE.SEDGE], mat: 'reed', place,
      accept: (u, v) => { const d = Math.abs(v - streamAt(u)); return out(u, v) && d > 1.7 && d < 2.6 + 1.6 * noise2(u * 0.4, v * 0.4) && noise2(u * 0.25 + 3, 7) > 0.4; } });
  }
  P.addCards(low);
  const puffs = new Puffs();
  smoke(puffs, { at: [hp[0], Lnd.H0 + 0.9, hp[1]], height: 10, drift: [0.55, 0.3], seed: 3, size: 0.3, tone: light === 'dusk' ? 3 : 5, alpha: light === 'dusk' ? 0.4 : 0.26 });
  P.addPuffs(puffs);
  return P.render(0);
}
