# The art lane's brief

The art lane prepares the textures, targets and guide pictures the graphics engine needs (`MIL-09`), so the builder can build the engine meanwhile.
It was the owner's idea, OK'd on 6 October 2026 with M2's plan ("Yes, that works. Let's start").
This brief is its contract: what it makes, in what form, and by which rules, so that its work drops straight into the engine (`ARCHITECTURE.md` A5.4).

## Read first

- `CLAUDE.md`, then `PROJECT.md`'s `PRE-01`, `PRE-20`, `PRE-22`, `PRE-23`, `PRE-42`, `PRE-46`, `PRN-10` and `SCP-20`.
- `ARCHITECTURE.md` A5, above all A5.3 (the texel ladder), A5.4 (where textures come from), A5.5 and A5.6 (truth in pictures).
- `research/19-graphics.md`, and its notes `research/19/5-content.md`, `research/19/7-pipeline.md` and `research/19/1-look.md` section 6.4.
- `art/targets/README.md` and its pictures, and the owner's 36 answers in `art/reviews/2026-10-06-graphics/answers.md`.

## Who does what

- **The art lane:** requests, GPT runs and their log, truth checks, the cloud tools that prepare pictures (re-gridding, levels, colour matching, sheets, checks), and the prepared textures with their records.
- **The builder:** the engine; the `texture` catalogue kind that reads the records; the Lab page on the phone; the shared colour measures in C++ (`kindling look`); wiring the art checks into `tools/check.sh`; reviewing and merging each batch.
- **The owner:** yes or no on each material's sheet, and on every target before anyone aims at it.

## Rules

1. **Its own lane.** Work only in `art/`, `tools/art/` and `tools/tests/test_art_*.py`, on the art lane's own branch.
   Never change engine code, `PROJECT.md`, `ARCHITECTURE.md`, `IMPLEMENTATION.md` or `CLAUDE.md`: propose changes to the builder in the batch's report.
   Commit on your branch; never push or merge: the builder merges after review.
2. **GPT only through `tools/art/gpt-run.sh`,** outside every build and test.
   Never print, copy or move a secret: the Codex sign-in file, a key or a passphrase.
   Never buy credits or change a plan; at a limit (a run that fails for usage), note what waits and carry on with work that needs no GPT.
   While `/tmp/kindling-gpt-paused` exists, runs are held: the builder pauses the lane that way.
3. **Pictures as needed:** no fixed number a batch since the owner's word of 6 October 2026 ("I have used almost nothing of my gpt limit, you can allow more pictures if the agents evaluates it can improve the result"): ask GPT again wherever you judge a picture will improve a material, log every run, and give the count in the report; at a usage limit, wait as rule 2 says.
4. **No AI model's name in any committed file.** The run log says "Codex 0.160.1, its image tool"; Codex's own log stays out of git.
5. **Truth before use:** every picture is checked against the list below before it becomes a source, a guide or a target, and the verdict goes into its record.
6. **Tools are tested code.** Each tool in `tools/art/` has unit tests in `tools/tests/test_art_<tool>.py` (`unittest`, with `# checks: <ID>` above each test), passes `ruff format` and `ruff check`, and names what it implements in its docstring ("Implements PRE-22, see A5.4.").
   Run your own tests and ruff; the builder runs the full check when merging.
7. **One measure, one place** (`CLAUDE.md`, rule 4): OKLab, accents and texture pixel contrast come from `kindling look`, the same C++ the engine's checks use.
   Call it through the `KINDLING` path (default `build/sim/kindling`), handing it a picture as raw RGBA bytes on standard input:
   - `kindling look stats <width> <height>` prints its measures as `name value` pairs on one line (lightness, colourfulness, hue, contrast, accents and others the builder adds), so read them by name;
   - `kindling look adjust <width> <height> <lightness> <hue> <colourfulness> <contrast>` writes the picture back with the four-number change made in OKLab (lightness added, hue turned in degrees, colourfulness and contrast in percent).

   Until the builder has built it, explore with scratch scripts kept out of the repository.
8. **Commits:** `T2.3a.2: …` for tools and `T2.3a.3: …` for materials, naming their items, such as `(PRE-22, A5.4)`, and ending with the session's attribution lines.
9. **Write plainly:** the owner reads everything on a phone.

## The sizes (A5.3)

- **Band 0,** the closest zoom (about 8 m across the screen): 64 texture pixels a metre, each shown as about 2 × 2 screen pixels.
- **Each band farther out** has half the density: band 1 32 a metre, band 2 16 (the close camp), band 3 8, band 4 4, band 5 2 (the camp zoom), band 6 1.
- **A texture** is a seamless tile of 256 × 256 texture pixels at its first band, with every level below it: 128, 64, 32, 16, 8, 4, 2 and 1, each texture pixel of a level covering exactly 2 × 2 of the level above.
- **A big surface has three tiles,** each drawn from its own picture of how the material looks from that distance, never the nearer tile shrunk (A5.3; the owner's word of 6 October 2026: "I don't think you should use the same textures for up close and zoom away"):
  - **near,** 4 m at band 0 (64 a metre), for bands 0 and 1: blades, crumbs and pebbles;
  - **middle,** 16 m at band 2 (16 a metre), for bands 2 and 3, the close camp: clumps, tufts and stones in drifts;
  - **far,** 64 m at band 4 (4 a metre), for bands 4 to 6, the camp zoom: swathes of taller and shorter growth, bare and damp patches.
  The big surfaces are the ground covers and rock: meadow, bare earth, trodden floor, bank gravel, river bed and rock A (the owner's pick, 6 October 2026).
- **Versions:** each tile of a big surface in two to four versions that share their edges, so any version joins any other without a seam, and the ground picks one for each cell by its place (A5.3; study 5's "repeats broken by mixing two to four maps by seed", which the owner asked for on 6 October 2026); four for the near tile, two or more for the middle and far ones.
  They go in `art/textures/<name>/` (version 1, as now) and `art/textures/<name>/v2/` to `v4/`, each with its levels and record; the middle and far tiles' in `middle/v2/` and so on.
  Hide, bark, poles, brush, hearth stones and ash cover small things and keep one tile.
- **Designed, never averaged:** within each tile, the band after its first is drawn for its size by the redraw route (GPT redraws the level above, then re-gridding and colour matching) for big surfaces, or by the code reduction for small ones; the levels below come from the code reduction, which draws bolder marks and fewer of them.
  Godot's averaged mipmaps are never used: averaging was the speckle of answer 31.
- **Every level is a file:** the phone loads all of them and makes none.
- Small things (plant cards, faces, held tools) have their own sizes for each band; they come in a later batch, when the builder asks.

## The path of a texture (A5.4)

1. **Request:** `art/requests/<name>-<nn>.txt` in the format below, with an approved picture or the level above as its input wherever one exists.
2. **Run:** `tools/art/gpt-run.sh art/requests/<name>-<nn>.txt <scratch folder>`; the picture and Codex's log stay in the scratch folder, outside git, and the run is logged in `art/log/gpt-runs.md`.
3. **Truth:** the checks below, the verdict into the record.
4. **Re-grid:** the block size (fractional allowed) and its phase found window by window, each block's median colour a texture pixel; seams blended where needed; painted light removed and checked (a slope of at most 0.02); the loss at most 10%.
5. **Band 0's tile:** 256 × 256 and seamless, built by code from one source or more, with no strong repeat inside it (at most 0.2).
6. **Levels:** bands 1 to 3 designed as above, each recording the digest of the level it came from; the rest by the code reduction.
7. **Colour matching:** each designed level fitted to the level above in four numbers (lightness, hue, colourfulness, contrast) that keep its accents: at least 90% of band 0's.
8. **The sheet:** `art/sheets/<name>.webp`, lossless and 1080 pixels wide, the owner's phone's width: every band at true size (each texture pixel 2 × 2 pixels), the same enlarged, flat and under a stand-in light (true midday, late afternoon, shade), beside a crop of its source.
   For a big surface, each band also as a strip the phone's full width, the ground the screen shows at that zoom, so any repeat shows as it would in the game.
   The engine's Lab page later shows the real light.
9. **The record** below, and the commit.

## Files

- `art/requests/<name>-<nn>.txt`: each request.
- `art/sources/<name>/<name>-<nn>.webp`: each original kept, as WebP of quality 85; the full original's digest and whether it carried its C2PA record go into the record.
- `art/textures/<name>/b0.png` to `b8.png`: the levels, lossless PNG; and `record.toml`.
  A big surface's middle and far tiles are folders of their own inside it, `art/textures/<name>/middle/` and `art/textures/<name>/far/`, each with its levels from `b0.png` (its first band) and its own record.
- `art/textures/<name>/source.png`: the source re-gridded on band 0's grid before the tile is cut (for a tile cut from parts of a picture, that picture re-gridded), so the contrast check always has it.
- `art/sheets/<name>.webp`: the sheet.
- `art/log/gpt-runs.md`: every run.

## A request

```
Purpose: why this picture, and what decision it serves.
Orientation: square, 1024 x 1024
Input picture: an absolute path to the input picture, or none
Transparent: no
Prompt:
The full prompt, as GPT will read it: what to draw, the area in metres and the blocks in picture pixels (8 to 12),
the background material only, the camera straight down or straight on and orthographic, even overcast light with no
light from one side and no darker corners, seamless in both directions, a Stone Age truth line, and what to avoid.
```

A redraw of a coarse band may ask for bigger blocks, the picture's side over the level's (16 for band 2, 32 for band 3), saying so in its Purpose.

## A record

Integers and strings only, never a float, since the catalogue's loader refuses floats (A3.6); only these keys.

```toml
about = "short wild meadow grass and bare earth: the background of the meadow cover"
route = "picture"                      # picture, code or world
tile_texels = 256                      # its first level's side: 4 m at 64 texture pixels a metre
texels_a_metre = 64                    # at its first level
first_band = 0                         # the band its first level is drawn for: 0 near, 2 middle, 4 far
sources = ["art/sources/meadow/meadow-01.webp"]
original_sha256 = ["…"]                # each full original as GPT made it, in the same order
c2pa = ["present"]                     # whether each full original carried its C2PA record
requests = ["art/requests/meadow-01.txt"]
made = "how band 0's tile was made from its sources, in a sentence"
regrid_loss = "3.9%"
truth = "art lane, 2026-10-06: wild grasses and bare earth only; nothing countable"
approved = "waiting"                   # the builder writes the owner's words and date when they approve

[[band]]
level = 0
file = "art/textures/meadow/b0.png"
sha256 = "…"
way = "re-gridded from its source, seams blended by code"

[[band]]
level = 1
file = "art/textures/meadow/b1.png"
sha256 = "…"
made_from = "…"                        # the digest of the level above; a level whose source changed is stale
way = "GPT redraw of band 0, re-gridded, colours matched by code"
regrid_loss = "4.4%"                   # a redrawn band's own re-grid loss; only on redrawn bands
calibration = "lightness +1.3%, hue -6.2 degrees, colourfulness 103%, contrast 90%"
```

## The checks (T2.3a.1)

- **Re-grid loss:** at most 10%, for band 0 and for each redrawn band.
- **Seams:** band 0's wrap at most 1.2 times its median step; each redrawn band's wrap no larger than its own ordinary steps between neighbouring rows and columns (in a smaller level the 1.2 line is noise: 10 to 33% of a level's ordinary boundaries already pass it).
- **Painted light:** band 0's slope at most 0.02; a redrawn band's is measured when it is prepared and written in its `way`.
- **Repeat:** band 0's at most 0.2, or at most its source's own where the material's grain repeats (a bark's fissures), so only tiling fails it.
- **Texture pixel contrast:** band 0's within a quarter of `source.png`'s.
- **Accents:** every band a surface is seen at, from the tile that serves it (near bands 0 and 1, middle 2 and 3, far 4 to 6), at least 90% of the near tile's band 0, without speckle: a level drawn for its band keeps bolder marks and fewer of them (A5.3), never single texture pixels.
  A material with one tile is judged on bands 1 to 3; its bands 4 to 6 are reported only, since there its texture pixel is 25 cm to a metre, larger than its marks, on things that small.
- **Drift:** every band's lightness within 0.02 and hue within 5° of the near tile's band 0, across the tiles, so the ground keeps its colour as you zoom.

## Truth (A5.6)

Every picture is looked at enlarged, every made thing, animal and garment, before it is used; anything doubtful is dated against a first-hand source and noted.
Known slips GPT makes, never to pass into a source or a target:
- metal: blades, axe heads, arrowheads, pots or sickles of metal before copper;
- sawn wood: flat cuts, planks, squared blocks; timber was split and adzed;
- later things: chickens, hooped buckets and barrels, wells with winches, lattice windows, glass-bead colours, rucksacks, slatted sleds, maize, a pot hung over a fire, boats with seats or ribs (dugouts only);
- animals out of time or season: spotted horses or long falling manes (wild horses are bay or dun with a short erect mane), striped piglets outside spring;
- borrowed peoples' signs: tipi-like cones with smoke flaps, Lascaux-like paintings, any real culture's motifs (`SCP-20`);
- clichés: fur bikinis and skirts, a grass rain cape;
- dated case by case: a potter's wheel and white woolly sheep, only for a first-copper village;
- in a ground or surface texture, anything countable: paths, flowers, stones, sticks, tracks, people (the world places those).

## Batch 1: first light's materials (T2.3a.3)

For the camp under the cliff by the river at the closest zoom (α2.3), in this order, each with band 0, its levels, its record and its sheet:
1. meadow grass and earth, from the liked camp's meadow (answer 15's code route and answer 36's redrawn band are the owner's picks);
2. bare earth and a trodden floor, the camp's working ground;
3. bank gravel, and the river bed of pebbles and sand seen through clear water;
4. two or three limestone rock surfaces, for each world's layers laid by code: answer 34 found neither surface tried so far right and 34b found the layers right, so show each candidate under stand-in layers on its sheet;
5. hide: pale, dark and smoked;
6. birch bark, and peeled and unpeeled poles;
7. brush (twigs and branches) and bark sheets;
8. the stones of a hearth ring and a tent's ring;
9. ash and charcoal round a hearth.

### Its second round, after the builder's review (6 October 2026)

With as many pictures as improve the result (rule 3):
1. **Middle and far tiles** for the meadow, bare earth, trodden floor, bank gravel, river bed and rock A, each from GPT's own picture of the material at that distance, so no zoom repeats a 4 m tile; and each tile's **versions**, as "The sizes" says.
2. **The near tile's band 1** redrawn by GPT for bare earth, trodden floor and bank gravel, the route answer 36 chose for big surfaces, and the improved reduction for any that still comes off the grid.
3. **The code reduction draws bolder marks and fewer of them** (A5.3): the larger marks kept two texture pixels wide at the coarser level and the smaller ones dropped, so designed bands keep 90% of their first level's accents without speckle.
4. **The checks above:** the loss of each redrawn band, the seams of redrawn bands, the repeat against the source's own, accents on the designed bands, `source.png` for limestone A, and the middle and far tiles' folders.
5. **The sheets** with each band as a strip the phone's full width, the versions mixed as the ground would mix them.
6. **The owner's answers** (6 October 2026): rock A, so limestone B and C leave `art/textures/` and `art/sheets/` (their sources stay); the meadow's band 0 kept as it is; bare earth, trodden floor, stone and ash redrawn without GPT's stepped-diamond pattern.

## The batch report

When a batch is done, commit it and report to the builder:
- what is in it, material by material, with each sheet's path;
- every check's result, and anything that failed and why;
- the GPT runs used against the budget, and anything waiting at a limit;
- every truth flag, and what was done about it;
- questions for the owner, and proposed changes for the builder.
