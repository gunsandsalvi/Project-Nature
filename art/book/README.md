# The art book

The art book shows the look the game is built to reach: one scene for each age of the arc, sheets of the model kit,
and the interface. It lives on a design canvas; this folder keeps the pictures at their true size (one art pixel
to one pixel) and the code that paints them.

- `plates/scenes`, `plates/sheets`, `plates/ui`: the pictures. Enlarge them by whole numbers with hard edges.
- `paint/`: the painter. Each scene is a small 3D model drawn at the game's pixel size with the art rules:
  colour in steps (`PRE-20`), own-colour outlines and lit edges (`PRE-21`), real shadows, sky-coloured shade,
  haze and mist (`PRE-30`), people and animals of blocks (`PRE-27`), the model kit (`PRE-46`).

To paint again (needs Node and the browser this machine already has):

- `cd art/book/paint && npm install` fetches the 3D library the painter uses.
- `NODE_PATH=$(npm root -g) node paint.js OUT river:morning` paints one scene into the folder OUT, at true size and
  enlarged four times. Scenes: `river`, `shelter` (add `:wide` for the cover), `winter`, `coast`, `lake`,
  `village`, `copper`; `sheets` with `:people`, `:animals`, `:plants`, `:things` or `:light`; `ui` with `:rest`,
  `:touch`, `:card`, `:book`, `:land` or `:sheet`. The second word is the hour: `morning`, `noon`, `golden`,
  `dusk`, `night` or `winter`.
