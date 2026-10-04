# The art book

The art book shows the look the game is built to reach: one scene for each age of the arc, sheets of the model kit,
and the interface. It lives on a design canvas; this folder keeps the pictures at their true size (one art pixel
to one pixel) and the code that paints them.

The owner accepted it on 4 October 2026 as the starting point for the look: the game is built toward these pictures
and judged against them, and they are tweaked as the real game takes shape on the phone.

- `plates/scenes`, `plates/sheets`, `plates/ui`: the pictures. Enlarge them by whole numbers with hard edges.
- `plates/zoom`: one place at each zoom stop of `PRE-03`, from one person to the globe, at noon and dusk, and the
  close camp held sideways. The place is the start of the world made from seed 7: a band's camp under a cliff by a
  river.
- `plates/options`: the same crops painted in each look and each water style, for the owner to choose from.
- `paint/`: the painter. Each scene is a small 3D model drawn at the game's pixel size with the art rules:
  colour in steps (`PRE-20`), own-colour outlines and lit edges (`PRE-21`), real shadows and reflections,
  sky-coloured shade, haze and mist (`PRE-30`), people and animals of blocks (`PRE-27`), the model kit (`PRE-46`).
  Grass, reeds, flowers and flames are drawn in metres, so everything keeps its true size at every zoom.

To paint again (needs Node and the browser this machine already has):

- `cd art/book/paint && npm install` fetches the 3D library the painter uses.
- `NODE_PATH=$(npm root -g) node paint.js OUT river:morning` paints one scene into the folder OUT, at true size and
  enlarged four times. Scenes: `river`, `shelter` (add `:wide` for the cover), `winter`, `coast`, `lake`,
  `village`, `copper`; `sheets` with `:people`, `:animals`, `:plants`, `:things`, `:light` or `:scale`; `ui` with
  `:rest`, `:touch`, `:card`, `:book`, `:land`, `:powers`, `:views`, `:map`, `:worlds` or `:sheet`. The second
  word is the hour: `morning`, `noon`, `golden`, `dusk`, `night` or `winter`.
- `zoom:noon:camp` paints a zoom stop: `person`, `closecamp`, `camp`, `valley`, `region`, `map` or `globe`; add
  `:land` for a landscape picture. `worldtest:noon:seed7` paints a flat map of a whole generated world, one pixel
  a cell, for tuning the generator (`paint/www/worldgen.js`); add `:region` for the start region instead.
- `zoom:noon:closecamp:export` writes the close camp as data instead of a picture, 50 m wider than the picture, for
  the Godot prototypes (`prototypes/app/look/make-scene.sh`); the picture's own scene stays as it is.
- Add a look and a water style as more words, for example `river:morning:look-clean:water-mirror`. Looks:
  `look-today` (fine grain, mixed pixels where light changes), `look-clean` (A), `look-sharp` (B, the book's
  look) and `look-painted` (C). Water: `water-today`, `water-clear` (W1, the book's water), `water-bands` (W2),
  `water-mirror` (W3) and `water-lagoon` (W4). `paint/www/look.js` holds the settings behind each.
