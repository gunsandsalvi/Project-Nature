// Writes crates/kd-render/tests/fixtures/palette-ref.json from the approved mockup (A11.3, T01a.4): the mockup's own
// PAL module, evaluated in Node, gives the 12 index tables, the four palette versions and nearest() on 1,000 colours,
// which kd-render's port must match exactly (palette::tests::matches_mockup). Usage: node tools/screens/palette-ref.mjs
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const html = fs.readFileSync(path.join(root, 'mockups/visual-style.html'), 'utf8');
const start = html.indexOf('const PAL = (() => {');
const end = html.indexOf('})();', start) + '})();'.length;
const line = (name) => html.split('\n').find((l) => l.startsWith(`const ${name} = `));
if (start < 0 || end < start || !line('clamp') || !line('lerp')) throw new Error('mockup has no PAL module, clamp or lerp');
const src = [line('clamp'), line('lerp'), html.slice(start, end), 'return PAL;'].join('\n');
const PAL = new Function(src)();

const tx = PAL.textures();
const rows = Object.entries(PAL.LUT_ROWS).sort((a, b) => a[1] - b[1]).map(([n]) => n);
const tables = {};
rows.forEach((n, r) => { tables[n] = Array.from({ length: 256 }, (_, k) => tx.luts[(r * 256 + k) * 4]); });
const versions = {};
PAL.VARIANT_ORDER.forEach((v, r) => {
  const out = [];
  for (let k = 0; k < PAL.count; k++) for (let c = 0; c < 3; c++) out.push(tx.pal[(r * 256 + k) * 4 + c]);
  versions[v] = out;
});
// nearest() on 1,000 colours spread over the cube, no colour excluded: r, g, b, then the index.
const nearest = [];
for (let i = 0; i < 1000; i++) {
  const c = [(i * 97) % 256, (i * 57 + 31) % 256, (i * 13 + 101) % 256];
  nearest.push(...c, PAL.nearest(c));
}
const arr = (a) => '[' + a.join(', ') + ']';
const obj = (o) => '{\n' + Object.entries(o).map(([k, v]) => `    "${k}": ${arr(v)}`).join(',\n') + '\n  }';
const json = `{\n  "colours": ${PAL.count},\n  "tables": ${obj(tables)},\n  "versions": ${obj(versions)},\n  "nearest": ${arr(nearest)}\n}\n`;
const out = path.join(root, 'crates/kd-render/tests/fixtures/palette-ref.json');
fs.mkdirSync(path.dirname(out), { recursive: true });
fs.writeFileSync(out, json);
console.log(`Palette reference: ${rows.length} tables, ${PAL.VARIANT_ORDER.length} versions, 1000 nearest -> ${path.relative(root, out)}`);
