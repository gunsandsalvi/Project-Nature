// The master palette (PRE-20): every material is a ladder of 7 shades, deepest first. The shade end leans to
// purple and blue, the lit end to warm yellow, with the colour strongest in the middle. The light picks the step.
import * as THREE from 'three';

// Colours here are the picture's own (sRGB) values: no conversion anywhere.
THREE.ColorManagement.enabled = false;

export const RAMPS = {
  // land
  grass:   ['#1d2a38', '#26404a', '#335c50', '#4a7c52', '#6f9f58', '#9bc062', '#c9dc84'],
  meadow:  ['#1f2c36', '#2c4448', '#3e624c', '#5a844e', '#80a655', '#a9c561', '#d2df86'],
  moss:    ['#212a30', '#2f4238', '#455e40', '#637c45', '#86994c', '#aab65c', '#cfd383'],
  dirt:    ['#271d2b', '#3e2c37', '#5a4140', '#795a4c', '#987759', '#b8976e', '#d8bc93'],
  sand:    ['#382c3e', '#584853', '#7c6c6a', '#a3907e', '#c5b293', '#dfcfac', '#f2e7cb'],
  snow:    ['#2b3252', '#434e78', '#65729e', '#8b9abe', '#b3c1db', '#d9e3f0', '#f8fbff'],
  ice:     ['#25335a', '#34497a', '#4a6596', '#6886b0', '#8eaac8', '#b8d0e2', '#e4f0f8'],
  drygrass:['#2c2428', '#4a3c34', '#6c5a40', '#8f7a4c', '#b19a5c', '#ccb872', '#e4d394'],
  mud:     ['#1d1920', '#2e292c', '#423b38', '#595046', '#726856', '#8b8268', '#a59c80'],
  water:   ['#172040', '#20365c', '#2a4f78', '#386a90', '#4f88a6', '#76aabb', '#b2d8d6'],
  sea:     ['#16203a', '#1f3554', '#2a4c6c', '#3a6884', '#528698', '#78a8b2', '#b2d6d2'],
  lagoon:  ['#10283a', '#143f50', '#175c68', '#1d7c82', '#319e9b', '#5ebfb1', '#a2e0cd'],
  // rock
  rock:    ['#231e33', '#383050', '#52486a', '#706785', '#91879f', '#b4aab2', '#dad0c6'],
  lime:    ['#28243a', '#413b54', '#5f586f', '#827a8b', '#a59da7', '#c7bfbf', '#e7dfd5'],
  redrock: ['#291626', '#47222f', '#6e3537', '#964d3f', '#b96a4b', '#d48d61', '#ebb787'],
  flint:   ['#141220', '#221f38', '#333154', '#494970', '#65678c', '#898da8', '#b5bbcb'],
  // plants
  leaf:    ['#1c2536', '#243c44', '#2c5748', '#447644', '#6a963f', '#99ba48', '#cbdd6c'],
  leafsp:  ['#1e2738', '#284048', '#335f4a', '#4f8248', '#7aa64a', '#a9c95a', '#d8e888'],
  autumn:  ['#2a1a2a', '#4a2632', '#723832', '#9c5230', '#c27432', '#dc9c42', '#f0c86a'],
  pine:    ['#1a2132', '#20333e', '#274844', '#335e48', '#4a774b', '#6b9156', '#97b268'],
  bark:    ['#1e1726', '#32222d', '#493035', '#63423d', '#7e5949', '#9b755e', '#bb9578'],
  birch:   ['#2c2a38', '#4a4652', '#6e6a72', '#948f92', '#b8b2ae', '#d8d2c8', '#f2ede2'],
  reed:    ['#2b2124', '#46382f', '#67563c', '#8a7447', '#a89254', '#c4ad69', '#dec888'],
  // made things
  wood:    ['#291d25', '#442f2e', '#634737', '#856143', '#a77e54', '#c89f6c', '#e3c38e'],
  hide:    ['#2a1c25', '#493033', '#6b4941', '#8e654f', '#af845e', '#cba572', '#e5c793'],
  leather: ['#1e1317', '#321f1e', '#4a2e27', '#644032', '#7f543e', '#9b6b4e', '#b88965'],
  fur:     ['#29212f', '#453941', '#65564e', '#877562', '#a89577', '#c6b491', '#e3d5b3'],
  furdark: ['#1c1720', '#2c242c', '#3e3236', '#524340', '#69574e', '#816e60', '#9c8876'],
  bone:    ['#393340', '#595157', '#7c7371', '#9f9587', '#c1b7a2', '#dcd3bc', '#f1ebd9'],
  ash:     ['#29252f', '#413c45', '#5b555f', '#767079', '#928c93', '#afa9af', '#ccc7cb'],
  thatch:  ['#2a2023', '#45372f', '#64523a', '#846e45', '#a08853', '#bba366', '#d5c084'],
  clay:    ['#2b191f', '#492929', '#6d3d33', '#92553f', '#b16f4b', '#cb8d61', '#e1af87'],
  copper:  ['#291317', '#491f1f', '#713329', '#994d33', '#bf6d3f', '#db9351', '#f1bf6f'],
  verdi:   ['#1a2a30', '#22403e', '#2c5a4c', '#3c7a5c', '#58996c', '#80b882', '#b2d8a4'],
  dyedred: ['#291121', '#491b25', '#6d292b', '#923931', '#b44f39', '#cf6f47', '#e79967'],
  ochre:   ['#2d1f1f', '#4d3325', '#744f2b', '#9b6f33', '#c1913d', '#dab456', '#edd485'],
  charcoal:['#0e0c15', '#191621', '#25212d', '#332f3a', '#45404a', '#5a555f', '#726c75'],
  cloth:   ['#2a2630', '#46404a', '#686068', '#8c8286', '#ada3a2', '#cbc2bc', '#e6ded6'],
  blue:    ['#141a34', '#1c2850', '#283c6e', '#36548c', '#4c70a6', '#6e92bf', '#9cbcd8'],
  // bodies
  skin1:   ['#392130', '#5b3337', '#844f3e', '#a76e4e', '#c78e65', '#dfac81', '#f1cba1'],
  skin2:   ['#291723', '#43242b', '#623632', '#814f3e', '#9f694b', '#ba865d', '#d5a577'],
  skin3:   ['#1b111f', '#2d1a25', '#44272b', '#5d3732', '#78493c', '#925e49', '#ad795b'],
  hair:    ['#140f1a', '#21182a', '#33232f', '#47322f', '#5e4334', '#7a573e', '#98704c'],
  hairred: ['#1a0f17', '#2e1820', '#4a2426', '#6a342c', '#8a4a34', '#a8643e', '#c4844e'],
  hairgrey:['#29272f', '#43414b', '#615f69', '#848289', '#a5a3a5', '#c5c1bd', '#e1dcd5'],
  // animals
  deer:    ['#231521', '#3d2127', '#5d322d', '#7e4735', '#9d5f3f', '#b97b51', '#d39d6f'],
  dun:     ['#292025', '#453531', '#69533f', '#8b714f', '#ac8f61', '#c7ab79', '#dfc797'],
  reindeer:['#211f29', '#393337', '#564c4b', '#756961', '#948777', '#b2a68f', '#cfc5ad'],
  wolf:    ['#1d1d27', '#33313b', '#4d4951', '#6a656b', '#898385', '#a9a2a1', '#c9c2be'],
  bison:   ['#191117', '#291b1f', '#3d2927', '#543931', '#6d4b3b', '#89614b', '#a57d61'],
  boar:    ['#1a141a', '#2a2026', '#3d2f32', '#52403f', '#6a544e', '#836a60', '#9e8474'],
  goat:    ['#2a2428', '#463c3c', '#665850', '#887664', '#a8957a', '#c6b496', '#e0d2b6'],
  feather: ['#14141e', '#22222e', '#33323e', '#47454f', '#5e5b63', '#79757b', '#969197'],
  white:   ['#38384a', '#5c5c70', '#848498', '#acacbe', '#cfcfdb', '#ececf2', '#ffffff'],
  // small colour
  flowerp: ['#291737', '#452359', '#65357d', '#894d9f', '#aa6bbd', '#c891d5', '#e3bbe9'],
  flowery: ['#392a1e', '#69491e', '#996d1e', '#c7951e', '#e5b730', '#f3d55a', '#fcef9a'],
  flowerw: ['#39394a', '#5d5d71', '#858599', '#adadbf', '#cfcfdb', '#ebebf1', '#ffffff'],
  berry:   ['#290d1d', '#4d1325', '#791d2d', '#a32b33', '#c7433d', '#df6951', '#f19977'],
  shell:   ['#393340', '#5d535d', '#84787f', '#aa9e9e', '#ccc1bb', '#e7ddd5', '#faf5ed'],
  // light and air
  fire:    ['#4a1418', '#8a2a1a', '#c84a1a', '#ee7a20', '#ffad3a', '#ffd86a', '#fff4c0'],
  ember:   ['#2a1018', '#4a1418', '#7a1e18', '#a83218', '#d4561e', '#f08a2e', '#ffc25a'],
  smoke:   ['#2d2937', '#443f4d', '#5d5765', '#79737f', '#958f99', '#b2acb3', '#cfc9cb'],
  glint:   ['#ffffff', '#ffffff', '#ffffff', '#ffffff', '#ffffff', '#ffffff', '#ffffff'],
};

export const NAMES = Object.keys(RAMPS);
const ROW = Object.fromEntries(NAMES.map((n, i) => [n, i]));

/** The palette row of a material, by name. */
export function mat(name) {
  if (!(name in ROW)) throw new Error('no material ' + name);
  return ROW[name];
}

/** One texture holding every ramp: x is the step, y the material. */
export function paletteTexture() {
  const w = 8, h = NAMES.length, data = new Uint8Array(w * h * 4);
  NAMES.forEach((n, y) => RAMPS[n].forEach((hex, x) => {
    const c = parseInt(hex.slice(1), 16), i = (y * w + x) * 4;
    data[i] = c >> 16; data[i + 1] = (c >> 8) & 255; data[i + 2] = c & 255; data[i + 3] = 255;
  }));
  const t = new THREE.DataTexture(data, w, h, THREE.RGBAFormat);
  t.magFilter = t.minFilter = THREE.NearestFilter;
  t.needsUpdate = true;
  return t;
}

const hex = (s) => new THREE.Color(s);

// The light of each hour (PRE-30). The sun sits behind the camera's left shoulder: az is measured from the
// camera's view direction, so the light falls from the upper left of the picture.
//   sun, shade: tints for sunlit and shaded colour (brightness kept: the step sets that); sunK, skyK: how
//   much each lifts the step, set so flat sunlit ground and flat shade land on chosen steps; sunPow below 1
//   evens out a low sun; shift: steps added everywhere; haze: the colour of the air; desat: how grey colour goes.
export const MOODS = {
  noon:    { el: 52, az: 140, sun: hex('#fff6e2'), shade: hex('#c8d2ff'), sunK: 0.38, skyK: 0.433, sunPow: 1.0, shift: 0,
             sky: hex('#a9cfe0'), haze: hex('#c4dbe6'), hazeK: 0.30, desat: 0.0, fire: hex('#ffd59a') },
  morning: { el: 16, az: 128, sun: hex('#ffe2c0'), shade: hex('#b8c4f6'), sunK: 0.58, skyK: 0.433, sunPow: 0.6, shift: 0,
             sky: hex('#c4d2e8'), haze: hex('#d6dde6'), hazeK: 0.45, desat: 0.08, fire: hex('#ffd59a') },
  golden:  { el: 11, az: 132, sun: hex('#ffd8a6'), shade: hex('#aaa6ec'), sunK: 0.55, skyK: 0.42, sunPow: 0.6, shift: 0,
             sky: hex('#b9b2dc'), haze: hex('#f0c49c'), hazeK: 0.38, desat: 0.0, fire: hex('#ffc070') },
  dusk:    { el: 5, az: 150, sun: hex('#ffbca6'), shade: hex('#a3a0dc'), sunK: 0.6, skyK: 0.36, sunPow: 0.7, shift: 0,
             sky: hex('#8e86c4'), haze: hex('#9584b8'), hazeK: 0.4, desat: 0.14, fire: hex('#ffb060') },
  night:   { el: 38, az: 120, sun: hex('#9aaeff'), shade: hex('#6a78d0'), sunK: 0.16, skyK: 0.283, sunPow: 1.0, shift: 0,
             sky: hex('#1c2652'), haze: hex('#232a58'), hazeK: 0.40, desat: 0.42, fire: hex('#ffb35a') },
  winter:  { el: 14, az: 135, sun: hex('#fff1e2'), shade: hex('#c6cdf2'), sunK: 0.62, skyK: 0.467, sunPow: 0.6, shift: 0,
             sky: hex('#c6d4ee'), haze: hex('#d4def0'), hazeK: 0.42, desat: 0.06, fire: hex('#ffcc88') },
  cave:    { el: 30, az: 140, sun: hex('#ffffff'), shade: hex('#7a6cb0'), sunK: 0.0, skyK: 0.12, sunPow: 1.0, shift: 0,
             sky: hex('#0c0a14'), haze: hex('#120e1e'), hazeK: 0.0, desat: 0.2, fire: hex('#ffb35a') },
};
