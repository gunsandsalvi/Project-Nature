// The look: switches for how clean or painterly a picture is, and how water is drawn, so options can be painted
// side by side from the same scene. A job picks them with options such as "look-clean" and "water-mirror".
//   grain: small dabs and noise inside patterns (1 on, 0 off); brush: large soft patches instead (painted look);
//   dither: mix two steps where they meet; leaf: leaf clumps 'busy' (many small leaves), 'clean' (fewer, larger) or
//   'big' (larger clumps still); tufts: how many grass tufts and flowers; shadowSoft: shadow edge softness (1 today);
//   outline: steps an outline darkens objects by; lit: steps a lit edge brightens by; contrast: spread of the steps
//   round the middle (1 today); wobble: how far step edges waver like brush strokes (0 none);
//   water: 'today', 'clear', 'bands', 'mirror' or 'lagoon'.
export const LOOKS = {
  today:   { name: 'Today', grain: 1, brush: 0, dither: 1, leaf: 'busy', tufts: 1, shadowSoft: 1, outline: 2, outlineNature: 1, lit: 1, contrast: 1 },
  clean:   { name: 'A · Clean', grain: 0, brush: 0, dither: 0, leaf: 'clean', tufts: 0.45, shadowSoft: 1, outline: 2, outlineNature: 1, lit: 1, contrast: 1 },
  sharp:   { name: 'B · Sharp', grain: 0, brush: 0, dither: 0, leaf: 'clean', tufts: 0.3, shadowSoft: 0.2, outline: 2, outlineNature: 2, lit: 1, contrast: 1.25 },
  painted: { name: 'C · Painted, clean', grain: 0, brush: 1, dither: 0, wobble: 0.7, leaf: 'big', tufts: 0.6, shadowSoft: 1.6, outline: 2, outlineNature: 1, lit: 1, contrast: 0.92 },
};
export const WATERS = { today: 0, clear: 1, bands: 2, mirror: 3, lagoon: 4 };

// The book is painted in the sharp look with clear water unless a job asks for another.
export const LOOK = { ...LOOKS.sharp, water: 'clear' };

/** Sets the look and the water from a job's options ("look-sharp", "water-clear"). */
export function lookFromOpts(opts) {
  for (const o of opts) {
    if (o.startsWith('look-') && LOOKS[o.slice(5)]) Object.assign(LOOK, LOOKS[o.slice(5)]);
    if (o.startsWith('water-') && o.slice(6) in WATERS) LOOK.water = o.slice(6);
  }
}
