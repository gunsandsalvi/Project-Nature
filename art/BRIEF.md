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
3. **A budget for each batch:** batch 1 is at most 30 pictures, retries included; ask the builder in the report before going over.
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
- **A ground or rock surface** is a seamless tile of 256 × 256 texture pixels at band 0 (4 m), with every level below it: 128, 64, 32, 16, 8, 4, 2 and 1, each texture pixel of a level covering exactly 2 × 2 of the level above.
- **Designed, never averaged:** bands 1 to 3 are drawn for their size, by the redraw route (GPT redraws the level above, then re-gridding and colour matching) for big surfaces, or by the code reduction for small ones; the levels below band 3 come from the code reduction.
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
   The engine's Lab page later shows the real light.
9. **The record** below, and the commit.

## Files

- `art/requests/<name>-<nn>.txt`: each request.
- `art/sources/<name>/<name>-<nn>.webp`: each original kept, as WebP of quality 85; the full original's digest and whether it carried its C2PA record go into the record.
- `art/textures/<name>/b0.png` to `b8.png`: the levels, lossless PNG; and `record.toml`.
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

## A record

Integers and strings only, never a float, since the catalogue's loader refuses floats (A3.6); only these keys.

```toml
about = "short wild meadow grass and bare earth: the background of the meadow cover"
route = "picture"                      # picture, code or world
tile_texels = 256                      # band 0's side: 4 m at 64 texture pixels a metre
texels_a_metre = 64                    # at band 0
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
calibration = "lightness +1.3%, hue -6.2 degrees, colourfulness 103%, contrast 90%"
```

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

## The batch report

When a batch is done, commit it and report to the builder:
- what is in it, material by material, with each sheet's path;
- every check's result, and anything that failed and why;
- the GPT runs used against the budget, and anything waiting at a limit;
- every truth flag, and what was done about it;
- questions for the owner, and proposed changes for the builder.
