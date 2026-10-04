// The painter: draws a 3D scene at the game's art-pixel size with the art rules, and hands back PNGs.
import * as THREE from 'three';
import { paletteTexture, MOODS, mat } from './palette.js';
import * as S from './glsl.js';
import { makeAtlas, GRID } from './atlas.js';
import { LOOK, WATERS } from './look.js';

THREE.ColorManagement.enabled = false;

const rt = (w, h, o = {}) => new THREE.WebGLRenderTarget(w, h, {
  type: THREE.FloatType, minFilter: THREE.NearestFilter, magFilter: THREE.NearestFilter, depthBuffer: true, ...o,
});

export class Painter {
  /**
   * w, h: picture size in art pixels; mpp: metres per art pixel; yaw: camera turn in degrees; elev: camera height
   * angle; target: the point at the centre; mood: the hour (palette.js MOODS) or an object; bounds: what casts shadows.
   */
  constructor({ w, h, mpp, yaw = 35, elev = 30, target = [0, 0, 0], mood = 'noon', sunAz, sunEl, sunDir, bounds = { c: [0, 0, 0], r: 60 }, shadowSize = 4096, depth = 1000 }) {
    Object.assign(this, { w, h, mpp, yaw, elev });
    this.mood = typeof mood === 'string' ? { ...MOODS[mood] } : { ...mood };
    if (sunAz !== undefined) this.mood.az = sunAz;
    if (sunEl !== undefined) this.mood.el = sunEl;
    const canvas = document.createElement('canvas');
    canvas.width = w; canvas.height = h;
    this.renderer = new THREE.WebGLRenderer({ canvas, antialias: false });
    this.renderer.setPixelRatio(1);
    this.renderer.autoClear = false;
    this.scene = new THREE.Scene();
    this.waterScene = new THREE.Scene();
    this.airScene = new THREE.Scene();
    this.pass = { value: 0 };
    this.right = { value: new THREE.Vector3() };
    this.up = { value: new THREE.Vector3() };
    this.mirror = { value: 0 };
    this.mirrorY = { value: 0 };
    this.atlas = makeAtlas(mpp);
    this.pal = paletteTexture();
    this.fires = [];
    this.mist = { base: 0, thick: 1, amount: 0, scale: 0.08, col: this.mood.haze.clone() };
    this.exposure = 1;
    this.ao = { rad: 0.9, dist: 1.5, k: 2.4 };
    this.sootU = { value: [0, 1, 2, 3].map(() => new THREE.Vector4()) };
    this.sootN = { value: 0 };

    const hw = (w * mpp) / 2, hh = (h * mpp) / 2;
    this.camera = new THREE.OrthographicCamera(-hw, hw, hh, -hh, 1, depth * 3);
    const yr = THREE.MathUtils.degToRad(yaw), er = THREE.MathUtils.degToRad(elev);
    const back = new THREE.Vector3(Math.sin(yr) * Math.cos(er), Math.sin(er), Math.cos(yr) * Math.cos(er));
    this.target = new THREE.Vector3(...target);
    this.camera.position.copy(this.target).addScaledVector(back, depth);
    if (elev > 89.5) this.camera.up.set(-Math.sin(yr), 0, -Math.cos(yr));   // straight down: the view's forward is up
    this.camera.lookAt(this.target);
    this.camera.updateMatrixWorld();
    this.camDepth = depth;
    this.edgeK = Math.max(0.5, mpp / 0.05);   // outlines judge depth steps in proportion to the art pixel
    this.waterSin = Math.sin(er);
    if (mpp > 0.5) this.ao = { rad: 0.9, dist: 1.5, k: 0 };

    // The sun: az degrees from the view direction toward the left, el above the horizon.
    const f = new THREE.Vector3(-Math.sin(yr), 0, -Math.cos(yr)), l = new THREE.Vector3(-Math.cos(yr), 0, Math.sin(yr));
    const az = THREE.MathUtils.degToRad(this.mood.az), el = THREE.MathUtils.degToRad(this.mood.el);
    this.sun = f.clone().multiplyScalar(Math.cos(az)).addScaledVector(l, Math.sin(az)).multiplyScalar(Math.cos(el));
    this.sun.y = Math.sin(el);
    this.sun.normalize();
    if (sunDir) this.sun.set(...sunDir).normalize();   // a light set in the world's own directions (the globe)
    const B = new THREE.Vector3(...bounds.c), R = bounds.r;
    this.boundsR = R;
    this.shadowCam = new THREE.OrthographicCamera(-R, R, R, -R, 1, 4 * R + 2000);
    this.shadowCam.position.copy(B).addScaledVector(this.sun, 2 * R + 500);
    this.shadowCam.lookAt(B);
    this.shadowCam.updateMatrixWorld();
    this.shadowSize = shadowSize;
    this.shadowTexel = (2 * R) / shadowSize;
    // the sky's view from straight above, to find what lies under cover (overhangs, roofs, crowns)
    this.skyCam = new THREE.OrthographicCamera(-R, R, R, -R, 1, Math.max(2000, 5 * R));
    this.skyCam.position.set(B.x, B.y + Math.max(600, 2 * R), B.z);
    this.skyCam.up.set(0, 0, -1);
    this.skyCam.lookAt(B);
    this.skyCam.updateMatrixWorld();
    this.haze = [this.camDepth + 8, this.camDepth + 140];
  }

  solidMaterial(layers, cells) {
    const L = layers || [];
    const row = (i) => (L[i] ? mat(L[i][0]) : 0), pat = (i) => (L[i] ? L[i][1] : 0);
    const C = cells || { tex: Painter.noCells(), origin: [0, 0], size: 1, dims: [1, 1] };
    return new THREE.ShaderMaterial({
      glslVersion: THREE.GLSL3, vertexShader: S.SOLID_VERT, fragmentShader: S.SOLID_FRAG, side: THREE.DoubleSide,
      uniforms: {
        uPass: this.pass,
        uLayers: { value: new THREE.Vector4(row(0), row(1), row(2), row(3)) },
        uLayerPat: { value: new THREE.Vector4(pat(0), pat(1), pat(2), pat(3)) },
        uSoot: this.sootU, uSootN: this.sootN,
        uGrain: { value: LOOK.grain }, uBrush: { value: LOOK.brush }, uMirror: this.mirror, uMirrorY: this.mirrorY,
        tCells: { value: C.tex }, uCellOrigin: { value: new THREE.Vector2(...C.origin) }, uCellSize: { value: C.size },
        uCellDims: { value: new THREE.Vector2(...C.dims) }, uJitter: { value: this.jitter ?? 1 },
      },
    });
  }
  /**
   * Adds a Solid (geo.js); layers: up to four [material, pattern] pairs for ground that blends by weight; cells:
   * { tex, origin, size, dims } for ground drawn in world cells (PAT.CELLS), see cellTexture().
   */
  addSolid(solid, layers, cells) {
    const m = new THREE.Mesh(solid.geometry(), this.solidMaterial(layers, cells));
    m.frustumCulled = false;
    this.scene.add(m);
    return m;
  }
  addCards(cards) {
    const m = new THREE.Mesh(cards.geometry(), new THREE.ShaderMaterial({
      glslVersion: THREE.GLSL3, vertexShader: S.CARD_VERT, fragmentShader: S.CARD_FRAG, side: THREE.DoubleSide,
      uniforms: { uPass: this.pass, tAtlas: { value: this.atlas }, uRight: this.right, uUp: this.up, uTiles: { value: GRID }, uMirror: this.mirror, uMirrorY: this.mirrorY },
    }));
    m.frustumCulled = false;
    this.scene.add(m);
    return m;
  }
  /**
   * Water: a geometry with an aFlow attribute (flow direction in x, z, and 1 for running water or 0 for still);
   * sea: 1 for the sea's colours. Its height (the first point's) is the mirror for reflections.
   */
  addWater(geometry, sea = 0) {
    if (this.waterLevel === undefined) this.waterLevel = geometry.getAttribute('position').getY(0);
    const m = new THREE.Mesh(geometry, new THREE.ShaderMaterial({
      glslVersion: THREE.GLSL3, vertexShader: S.WATER_VERT, fragmentShader: S.WATER_FRAG, side: THREE.DoubleSide,
      uniforms: { uKind: { value: sea } },
    }));
    m.frustumCulled = false;
    this.waterScene.add(m);
    return m;
  }
  /** Smoke and mist: Puffs (geo.js) of one material, at an opacity set per puff. */
  addPuffs(puffs, material = 'smoke', shift = 0) {
    const md = this.mood;
    const m = new THREE.Mesh(puffs.geometry(), new THREE.ShaderMaterial({
      glslVersion: THREE.GLSL3, vertexShader: S.PUFF_VERT, fragmentShader: S.PUFF_FRAG,
      uniforms: {
        tG1: { value: null }, tPal: { value: this.pal }, uRow: { value: mat(material) }, uRight: this.right, uUp: this.up,
        uSunTint: { value: md.sun }, uShadeTint: { value: md.shade }, uFireTint: { value: md.fire }, uDesat: { value: md.desat },
        uHazeCol: { value: md.haze }, uHazeK: { value: md.hazeK }, uHaze: { value: new THREE.Vector2() },
        uStepShift: { value: shift + Math.min(0, Math.round(md.shift)) },
      },
    }));
    m.frustumCulled = false;
    this.airScene.add(m);
    return m;
  }
  /** Soot on rock round a point (radius r), or with a negative radius a lighter damp stain. Up to four. */
  addSoot(pos, r) { const i = this.sootN.value++; this.sootU.value[i].set(pos[0], pos[1], pos[2], r); }
  /** A fire's light: power about 0.3 for embers to 1.2 for a big blaze; radius in metres. */
  addFire(pos, power = 0.6, radius = 7) { this.fires.push({ pos, power, radius }); }
  setMist(base, thick, amount, scale = 0.08, col) { this.mist = { base, thick, amount, scale, col: col || this.mood.haze.clone() }; }

  static noCells() {
    if (!Painter._noCells) { Painter._noCells = new THREE.DataTexture(new Uint8Array([0, 128, 0, 255]), 1, 1, THREE.RGBAFormat); Painter._noCells.needsUpdate = true; }
    return Painter._noCells;
  }

  basis(cam) {
    const e = cam.matrixWorld.elements;
    this.right.value.set(e[0], e[1], e[2]).normalize();
    this.up.value.set(e[4], e[5], e[6]).normalize();
  }
  quad(frag, uniforms, outs) {
    const m = new THREE.Mesh(new THREE.PlaneGeometry(2, 2), new THREE.ShaderMaterial({
      glslVersion: THREE.GLSL3, vertexShader: S.QUAD_VERT, fragmentShader: frag, uniforms, depthTest: false, depthWrite: false,
    }));
    const sc = new THREE.Scene(); sc.add(m);
    return { scene: sc, mesh: m };
  }

  render(scale = 4) {
    const { w, h, renderer: R } = this;
    const md = this.mood;
    const qcam = new THREE.OrthographicCamera(-1, 1, 1, -1, 0, 1);
    R.setClearColor(0x000000, 0);

    // 1. shadows, seen from the sun
    const shadow = rt(this.shadowSize, this.shadowSize);
    this.pass.value = 1;
    this.basis(this.shadowCam);
    R.setRenderTarget(shadow); R.clear(); R.render(this.scene, this.shadowCam);
    const skyMap = rt(2048, 2048);
    this.basis(this.skyCam);
    R.setRenderTarget(skyMap); R.clear(); R.render(this.scene, this.skyCam);

    // 2. what each art pixel shows
    const gbuf = rt(w, h, { count: 3 });
    this.pass.value = 0;
    this.basis(this.camera);
    R.setRenderTarget(gbuf); R.clear(); R.render(this.scene, this.camera);
    const wbuf = rt(w, h, { count: 2 });
    R.setRenderTarget(wbuf); R.clear(); R.render(this.waterScene, this.camera);
    // ... and, for water, the same scene mirrored in its surface
    const style = this.waterStyle ?? (WATERS[LOOK.water] || 0);
    const mirrorOn = this.waterScene.children.length > 0 && style > 0 && style !== 5 && this.waterLevel !== undefined;
    let gbufM = null;
    if (mirrorOn) {
      gbufM = rt(w, h, { count: 3 });
      this.mirror.value = 1; this.mirrorY.value = this.waterLevel;
      R.setRenderTarget(gbufM); R.clear(); R.render(this.scene, this.camera);
      this.mirror.value = 0;
    }

    // 3. light into steps
    const lv = this.shadowCam.matrixWorldInverse.clone(), lp = this.shadowCam.projectionMatrix.clone();
    const firePos = [], fireCol = [];
    for (let i = 0; i < 12; i++) {
      const F = this.fires[i];
      firePos.push(F ? new THREE.Vector4(...F.pos, F.radius) : new THREE.Vector4());
      fireCol.push(F ? new THREE.Vector4(1, 1, 1, F.power) : new THREE.Vector4());
    }
    const resolve = (gb, waterOn) => {
      const res = rt(w, h, { count: 2, depthBuffer: false });
      const q = this.quad(S.RESOLVE_FRAG, {
        tG0: { value: gb.textures[0] }, tG1: { value: gb.textures[1] }, tG2: { value: gb.textures[2] },
        tShadow: { value: shadow.texture }, tW0: { value: wbuf.textures[0] }, tW1: { value: wbuf.textures[1] },
        tSky: { value: skyMap.texture }, uSkyView: { value: this.skyCam.matrixWorldInverse.clone() }, uSkyProj: { value: this.skyCam.projectionMatrix.clone() },
        uSkyCover: { value: this.skyCover ?? 0.55 }, uSkyTexel: { value: (2 * this.boundsR) / 2048 },
        uLightView: { value: lv }, uLightProj: { value: lp }, uShadowSize: { value: new THREE.Vector2(this.shadowSize, this.shadowSize) },
        uShadowTexel: { value: this.shadowTexel }, uSun: { value: this.sun }, uSunK: { value: md.sunK }, uSkyK: { value: md.skyK },
        uShift: { value: md.shift }, uExposure: { value: this.exposure }, uSunPow: { value: md.sunPow ?? 1 }, uFires: { value: this.fires.length },
        uFire: { value: firePos }, uFireC: { value: fireCol }, uRes: { value: new THREE.Vector2(w, h) },
        uAORad: { value: this.ao.rad / this.mpp }, uAODist: { value: this.ao.dist }, uAOK: { value: this.ao.k },
        uHaze: { value: new THREE.Vector2(...this.haze) }, uWaterOn: { value: waterOn && this.waterScene.children.length ? 1 : 0 },
        uWaterRow: { value: mat('water') }, uFoamRow: { value: mat('white') }, uGlintRow: { value: mat('glint') }, uSeaOn: { value: 0 },
        uShadowSoft: { value: LOOK.shadowSoft }, uContrast: { value: LOOK.contrast }, uWaterStyle: { value: style },
      });
      R.setRenderTarget(res); R.clear(); R.render(q.scene, qcam);
      return res;
    };
    const res = resolve(gbuf, true);
    const resM = mirrorOn ? resolve(gbufM, false) : null;

    // 4. outlines, lit edges, the hour's colour, water, mist and haze
    const finish = (gb, rs, { mirror = false, refl = null } = {}) => {
      const fin = rt(w, h, { type: THREE.UnsignedByteType, depthBuffer: false });
      const q = this.quad(S.FINAL_FRAG, {
        tR0: { value: rs.textures[0] }, tR1: { value: rs.textures[1] }, tG0: { value: gb.textures[0] }, tG1: { value: gb.textures[1] },
        tG2: { value: gb.textures[2] }, tPal: { value: this.pal }, tW0: { value: wbuf.textures[0] }, tW1: { value: wbuf.textures[1] },
        tRefl: { value: refl }, uReflOn: { value: refl ? 1 : 0 },
        uView: { value: this.camera.matrixWorldInverse }, uSun: { value: this.sun }, uMpp: { value: this.mpp },
        uSunTint: { value: md.sun }, uShadeTint: { value: md.shade }, uFireTint: { value: md.fire }, uHazeCol: { value: md.haze },
        uHazeK: { value: mirror ? 0 : md.hazeK }, uDesat: { value: md.desat }, uBack: { value: md.haze },
        uMist: { value: new THREE.Vector4(this.mist.base, this.mist.thick, mirror ? 0 : this.mist.amount, this.mist.scale) }, uMistCol: { value: this.mist.col },
        uRes: { value: new THREE.Vector2(w, h) }, uWaterTint: { value: new THREE.Color(0.72, 0.86, 0.98) }, uSkyRow: { value: mat('sea') + 0.5 }, uSkyCol: { value: md.sky || md.haze },
        uInk: { value: mirror ? 2 : (this.ink ? 1 : (this.clear ? 2 : 0)) },
        uEdgeK: { value: this.edgeK }, uWaterSin: { value: this.waterSin }, uMapDepth: { value: new THREE.Vector3(...(this.mapDepth || [8, 60, 500])) }, uMapFoam: { value: this.mapFoam || 0 },
        uDither: { value: LOOK.dither }, uWobble: { value: LOOK.wobble || 0 }, uOutline: { value: LOOK.outline }, uOutlineN: { value: LOOK.outlineNature }, uLit: { value: LOOK.lit },
        uMirror: { value: mirror ? 1 : 0 }, uWaterStyle: { value: mirror ? 0 : style }, uWaterY: { value: this.waterLevel ?? 0 },
        uCamRight: { value: this.right.value.clone() },
        uWaterRow: { value: mat('water') }, uSeaRow: { value: mat('sea') }, uLagoonRow: { value: mat('lagoon') }, uFoamRow: { value: mat('white') },
      });
      R.setRenderTarget(fin); R.clear(); R.render(q.scene, qcam);
      return fin;
    };
    const finM = mirrorOn ? finish(gbufM, resM, { mirror: true }) : null;
    const fin = finish(gbuf, res, { refl: finM ? finM.texture : null });

    // 5. smoke and mist over the picture
    let out = fin;
    if (this.airScene.children.length && !this.ink) {
      const air = rt(w, h, { type: THREE.UnsignedByteType });
      for (const m of this.airScene.children) {
        m.material.uniforms.tG1.value = gbuf.textures[1];
        m.material.uniforms.uHaze.value.set(...this.haze);
      }
      R.setRenderTarget(air); R.clear(); R.render(this.airScene, this.camera);
      const comp = rt(w, h, { type: THREE.UnsignedByteType, depthBuffer: false });
      const r3 = this.quad(/* glsl */ `
        precision highp float; layout(location = 0) out vec4 col;
        uniform sampler2D tA; uniform sampler2D tB;
        void main(){ ivec2 p = ivec2(gl_FragCoord.xy); vec4 a = texelFetch(tA, p, 0), b = texelFetch(tB, p, 0);
          col = vec4(mix(a.rgb, b.rgb, b.a), 1.); }`, { tA: { value: fin.texture }, tB: { value: air.texture } });
      R.setRenderTarget(comp); R.clear(); R.render(r3.scene, qcam);
      out = comp;
    }

    const buf = new Uint8Array(w * h * 4);
    R.readRenderTargetPixels(out, 0, 0, w, h, buf);
    R.setRenderTarget(null);
    this.buf = buf;
    return scale === 0 ? toCanvas(buf, w, h) : pngs(buf, w, h, scale);
  }

  /** The point at height y that shows at picture pixel (sx, sy): for laying things out on a sheet. */
  groundAt(sx, sy, y = 0) {
    const nx = (sx / this.w) * 2 - 1, ny = 1 - (sy / this.h) * 2;
    const a = new THREE.Vector3(nx, ny, -1).unproject(this.camera), b = new THREE.Vector3(nx, ny, 1).unproject(this.camera);
    const t = (y - a.y) / (b.y - a.y);
    return [a.x + (b.x - a.x) * t, y, a.z + (b.z - a.z) * t];
  }

  /** Where a world point lands in the picture, in art pixels from the top left. */
  toScreen(p) {
    const v = new THREE.Vector3(...p).project(this.camera);
    return [Math.round((v.x * 0.5 + 0.5) * this.w), Math.round((0.5 - v.y * 0.5) * this.h)];
  }

  /** The point at height y seen through a pixel of the picture. */
  fromScreen(sx, sy, y = 0) {
    const nx = ((sx + 0.5) / this.w) * 2 - 1, ny = 1 - ((sy + 0.5) / this.h) * 2;
    const a = new THREE.Vector3(nx, ny, -1).unproject(this.camera), b = new THREE.Vector3(nx, ny, 1).unproject(this.camera);
    const t = (y - a.y) / (b.y - a.y);
    return [a.x + (b.x - a.x) * t, y, a.z + (b.z - a.z) * t];
  }
}

/**
 * World cells as a small picture for PAT.CELLS ground: dims [w, h] cells of `size` metres from origin [x, z];
 * at(i, j) gives [material name, step offset] for the cell.
 */
export function cellTexture({ dims, origin, size, at }) {
  const [w, h] = dims, data = new Uint8Array(w * h * 4);
  for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) {
    const [m, b] = at(i, j), k = (j * w + i) * 4;
    data[k] = mat(m); data[k + 1] = 128 + Math.round(b || 0); data[k + 2] = 0; data[k + 3] = 255;
  }
  const tex = new THREE.DataTexture(data, w, h, THREE.RGBAFormat);
  tex.magFilter = tex.minFilter = THREE.NearestFilter;
  tex.needsUpdate = true;
  return { tex, origin, size, dims };
}

/** The picture as a canvas at one art pixel per pixel (for drawing the interface over it). */
export function toCanvas(buf, w, h) {
  const c = document.createElement('canvas'); c.width = w; c.height = h;
  const g = c.getContext('2d'); const img = g.createImageData(w, h);
  for (let y = 0; y < h; y++) img.data.set(buf.subarray((h - 1 - y) * w * 4, (h - y) * w * 4), y * w * 4);
  g.putImageData(img, 0, 0);
  return c;
}

/** PNGs of a canvas: as it is, and enlarged with hard edges. */
export function canvasPngs(c1, scale = 4) {
  const out = { '': c1.toDataURL('image/png') };
  const c2 = document.createElement('canvas'); c2.width = c1.width * scale; c2.height = c1.height * scale;
  const g2 = c2.getContext('2d'); g2.imageSmoothingEnabled = false; g2.drawImage(c1, 0, 0, c2.width, c2.height);
  out['x' + scale] = c2.toDataURL('image/png');
  return out;
}

/** The picture at one art pixel per pixel, and enlarged with hard edges. */
export function pngs(buf, w, h, scale = 4) {
  const c1 = document.createElement('canvas'); c1.width = w; c1.height = h;
  const g1 = c1.getContext('2d'); const img = g1.createImageData(w, h);
  for (let y = 0; y < h; y++) img.data.set(buf.subarray((h - 1 - y) * w * 4, (h - y) * w * 4), y * w * 4);
  g1.putImageData(img, 0, 0);
  const out = { '': c1.toDataURL('image/png') };
  if (scale > 1) {
    const c2 = document.createElement('canvas'); c2.width = w * scale; c2.height = h * scale;
    const g2 = c2.getContext('2d'); g2.imageSmoothingEnabled = false; g2.drawImage(c1, 0, 0, w * scale, h * scale);
    out['x' + scale] = c2.toDataURL('image/png');
  }
  return out;
}
