// Kindling's web shell (A2.6): loads the WebAssembly, sizes the canvas in exact device pixels, forwards pointer
// events, runs the frame loop, pauses while the page is hidden, and carries out the app's requests: the code
// dialog gzips a report with CompressionStream and shows it with Copy (A15.4). Every failure is written in the
// status line, so the page is never blank (A17.3).
import init, { WebApp, build_line } from './pkg/kd_web.js';

const canvas = document.getElementById('cv');
const status = document.getElementById('status');
const say = (text) => { status.textContent = text; };
const testing = new URLSearchParams(location.search).has('test');

async function codeOf(prefix, json) {
  const gz = new Blob([json]).stream().pipeThrough(new CompressionStream('gzip'));
  const bytes = new Uint8Array(await new Response(gz).arrayBuffer());
  let bin = '';
  for (const b of bytes) bin += String.fromCharCode(b);
  return prefix + btoa(bin);
}

async function showCode(title, prefix, json) {
  const box = document.getElementById('code');
  const text = document.getElementById('code-text');
  document.getElementById('code-title').textContent = title;
  text.value = await codeOf(prefix, json);
  box.hidden = false;
  document.getElementById('code-copy').onclick = () => {
    navigator.clipboard.writeText(text.value).catch(() => { text.focus(); text.select(); });
  };
  document.getElementById('code-close').onclick = () => { box.hidden = true; };
}

function carryOut(json) {
  for (const r of JSON.parse(json)) {
    if (r.ShowCode) showCode(r.ShowCode.title, r.ShowCode.prefix, r.ShowCode.json);
  }
}

async function main() {
  try {
    await init();
  } catch (e) {
    say(`WebAssembly could not start here: ${e}`);
    return;
  }
  let app;
  try {
    app = new WebApp(canvas, new Map());
  } catch (e) {
    say(`WebGL2 could not start here: ${e}`);
    return;
  }
  const drain = () => {
    const r = app.take_requests();
    if (r) carryOut(r);
  };
  // The canvas's exact size in device pixels, so an art pixel is exactly 4 of them (A2.6): the browser's own
  // count when it agrees with the CSS size times the screen's scale (emulated screens report CSS pixels there),
  // else that product, rounded.
  const fit = (entry) => {
    const d = entry.devicePixelContentBoxSize?.[0];
    const cw = entry.contentRect.width * devicePixelRatio;
    const ch = entry.contentRect.height * devicePixelRatio;
    const exact = d && Math.abs(d.inlineSize - cw) <= 1 && Math.abs(d.blockSize - ch) <= 1;
    app.resize(exact ? d.inlineSize : Math.round(cw), exact ? d.blockSize : Math.round(ch));
    if (testing) {
      app.frame(performance.now());
      drain();
    }
  };
  const observer = new ResizeObserver((entries) => fit(entries[0]));
  try {
    observer.observe(canvas, { box: 'device-pixel-content-box' });
  } catch {
    observer.observe(canvas);
  }
  const kinds = { pointerdown: 0, pointermove: 1, pointerup: 2, pointercancel: 3 };
  for (const [type, kind] of Object.entries(kinds)) {
    canvas.addEventListener(type, (e) => {
      if (type === 'pointerdown') canvas.setPointerCapture(e.pointerId);
      app.pointer(kind, e.pointerId, e.offsetX * devicePixelRatio, e.offsetY * devicePixelRatio, e.timeStamp);
    });
  }
  document.addEventListener('visibilitychange', () => (document.hidden ? app.pause() : app.resume()));
  // The game shows its own version line in the strip (PRE-32); the page's line stays for failures.
  say('');
  console.log(`Kindling ${build_line()}`);
  if (testing) {
    // A12.4's test hooks: frames are drawn only when a test asks, so every screenshot is exact.
    window.kd = {
      ready: () => app.ready(),
      frame: (n = 1) => {
        for (let i = 0; i < n; i++) app.frame(performance.now());
        drain();
        return app.frames();
      },
      artSize: () => Array.from(app.art_size()),
      core: () => app.core_hashes(),
      probe: () => JSON.parse(app.probe()),
      hour: (n) => app.set_hour(n),
      palette: () => app.palette(),
      golden: (name) => app.golden(name),
      // The next frame as a PNG data URL, or with `art` one pixel an art pixel (the grid starts at the top-left).
      shot: ({ art = false } = {}) => {
        app.frame(performance.now());
        drain();
        const w = canvas.width;
        const h = canvas.height;
        const full = document.createElement('canvas');
        full.width = w;
        full.height = h;
        const fctx = full.getContext('2d');
        fctx.drawImage(canvas, 0, 0);
        if (!art) return full.toDataURL('image/png');
        const [aw, ah] = [Math.floor(w / 4), Math.floor(h / 4)];
        const src = fctx.getImageData(0, 0, w, h).data;
        const out = document.createElement('canvas');
        out.width = aw;
        out.height = ah;
        const octx = out.getContext('2d');
        const img = octx.createImageData(aw, ah);
        for (let y = 0; y < ah; y++) {
          for (let x = 0; x < aw; x++) {
            const from = ((y * 4) * w + x * 4) * 4;
            img.data.set(src.subarray(from, from + 4), (y * aw + x) * 4);
          }
        }
        octx.putImageData(img, 0, 0);
        return out.toDataURL('image/png');
      },
      crash: () => app.crash(),
    };
    return;
  }
  // A panic's hook has written its line by the time its trap reaches here (A3.8); any other error is written now.
  const loop = (t) => {
    try {
      app.frame(t);
      drain();
    } catch (e) {
      if (!status.textContent.startsWith('Kindling stopped')) say(`Kindling stopped: ${e}`);
      return;
    }
    requestAnimationFrame(loop);
  };
  requestAnimationFrame(loop);
}

main();
