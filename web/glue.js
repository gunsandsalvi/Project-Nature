// Kindling's web shell (A2.6): loads the wasm, forwards pointer events, runs the frame loop, pauses when hidden,
// and shows the self-check's KDS1: code (A15.4). Never a blank page: every failure is written in #status (A17.3).
import init, { WebApp, core_check, build_line } from './pkg/kd_web.js';

const status = document.getElementById('status');
const canvas = document.getElementById('cv');
const say = (text) => { status.textContent = text; };

async function kds1(json) {
  // gzip by CompressionStream, then base64, as the phone does (A15.4)
  const gz = new Blob([json]).stream().pipeThrough(new CompressionStream('gzip'));
  const bytes = new Uint8Array(await new Response(gz).arrayBuffer());
  let bin = '';
  for (const b of bytes) bin += String.fromCharCode(b);
  return 'KDS1:' + btoa(bin);
}

async function showSelfCheck(json) {
  const code = await kds1(json);
  status.textContent = 'Kindling self-check: something failed. Copy the code and send it: ';
  const btn = document.createElement('button');
  btn.textContent = 'Copy';
  btn.onclick = () => navigator.clipboard.writeText(code).then(() => { btn.textContent = 'Copied'; }, () => {});
  const pre = document.createElement('div');
  pre.textContent = code;
  status.append(btn, pre);
}

async function main() {
  try {
    await init();
  } catch (e) {
    say('WebAssembly blocked: ' + e);
    return;
  }
  let app;
  try {
    app = new WebApp(canvas, new Map(), window.devicePixelRatio || 1);
  } catch (e) {
    say('WebAssembly works · WebGL2 blocked: ' + e);
    return;
  }
  say(`WebAssembly works · WebGL2 works · core ${core_check() ? 'OK' : 'MISMATCH'} · ${build_line()} · running`);

  new ResizeObserver(() => {
    app.resize(canvas.clientWidth, canvas.clientHeight, window.devicePixelRatio || 1);
  }).observe(canvas);

  const kinds = { pointerdown: 0, pointermove: 1, pointerup: 2, pointercancel: 3 };
  for (const [type, kind] of Object.entries(kinds)) {
    canvas.addEventListener(type, (e) => {
      if (kind === 0) { try { canvas.setPointerCapture(e.pointerId); } catch (err) { /* not everywhere */ } }
      const dpr = window.devicePixelRatio || 1;
      app.pointer(kind, e.pointerId, e.clientX * dpr, e.clientY * dpr, e.timeStamp);
      e.preventDefault();
    });
  }
  document.addEventListener('visibilitychange', () => { if (document.hidden) app.pause(); else app.resume(); });

  const loop = (t) => {
    app.frame(t);
    const req = app.take_requests();
    if (req) {
      for (const r of JSON.parse(req)) {
        if (r.SelfCheck) showSelfCheck(r.SelfCheck.json);
      }
    }
    requestAnimationFrame(loop);
  };
  requestAnimationFrame(loop);

  if (new URLSearchParams(location.search).get('test') === '1') {
    window.kd = { ready: () => true, yaw: () => app.yaw() }; // A12.4's test hook, grown later
  }
}

main();
