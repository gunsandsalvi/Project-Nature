// Loads one scene by name, paints it in the hour asked for, and leaves the PNGs in window.result.
const q = new URLSearchParams(location.search);
const name = q.get('scene'), light = q.get('light') || 'noon';
const opts = (q.get('opts') || '').split(',').filter(Boolean);
try {
  const mod = await import(`./scenes/${name}.js`);
  const images = await mod.default({ light, opts });
  window.result = { images };
} catch (e) {
  window.result = { error: (e && e.stack) || String(e) };
}
