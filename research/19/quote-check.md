# Research 19: the check of the studies' quotes

Every quotation in the seven studies' notes was checked against its page on 6 October 2026 by four checkers, each on its own notes, as `research/README.md` requires: saved copies first, the live page where none held it, and the repository for quotes of its own files.
Their reports follow, as written.

## Quote check: note 1, the look

Checked on 6 October 2026, as `CHECKER.md` asks. `R` is `/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/m2-research`.
My files are in `R/work/checker/1/`: the note as it was (`1-look.orig.md`), the pages I fetched (`pages/`), the scripts that matched each quote to its page (`find.py`, `check.py`) and their outputs (`check-*.txt`), and the re-runs of the study's measures (`rerun-*.txt`).

### Totals

| | Checked | Held as written | Changed |
|---|---|---|---|
| Quotations of a source | 147 | 145 | 2 corrected (one not exact, one credited too widely); none removed, none left unverified |
| Titles in quotation marks | 21 | 20 (case or dash differences only) | 1 shortened title restored |
| New quotations added for the "to check" and second-hand items | 15 (and 6 titles) | 15 | added, each checked against its page |

Not counted as quotations: 5 defined terms (the headings of table 3.4), 20 proposed wordings (section 6.5) and 2 references to the note itself ("drawn to read", "to check").

I also checked the numbers given with a source: every cell of tables 3.4, 3.6 and 3.7, the measured numbers in the text, the phone's numbers, the arithmetic of texel sizes, and each source's author and date. Four were wrong or misleading and are corrected (changes 4 to 7).

### How each quotation was checked

| Kind of check | Quotations | Which |
|---|---|---|
| The repository at commit `4320212` (the note's own reference; `PROJECT.md` and `ARCHITECTURE.md` are unchanged since), and git `7874f41` | 35 | `PROJECT.md` 22, `ARCHITECTURE.md` 10, the art book's commit 3; each also confirmed under the item or rule the note names |
| The owner's answers, `R/owner/ask/answers.md` | 17 | all the owner's words |
| The study's own files | 3 | round 1's note ("rich pixel art"); the GPT requests' "avoid" lines (twice) |
| Saved copies of the pages | 34 | Hytale 16 (`R/work2/5/pages/`), t3ssel8r's feed 8 (`R/work/1/`), the Team Fortress 2 paper 6 (`R/work/1/`, `R/work2/1/`), A Short Hike 2 (`R/work2/3/web/`), Slynyrd's Pixelblog 15 2 (`R/work2/5/pages/`) |
| Pages fetched (with curl, so the words could be matched exactly) | 44 | Xbox Wire 4, PCGamesN 2, 80 Level on Shadowglass 2 and on Kingdom 2, Slynyrd's Pixelblogs 62 and 20 4 each, Riot 4, Itti 1, Tegel 7, GWCT 4, Wikipedia on Pesse 3, Center of the West 2, O'Sullivan 3, Denoyer 2 |
| NCBI's own copy of the PMC article (PMC shows curl a browser check, so I used NCBI's E-utilities service) | 12 | Wolfe and Horowitz 4, Peters 4, Ludwig 4 |
| WebFetch (Britannica refuses curl); a search engine's copy agrees | 2 | Britannica on Przewalski's horse |

Differences of italics, bold, link tags, typographic quotes and dashes were allowed, as the brief says. Three quotations differed only that way: Xbox Wire's *Minecraft*-y, Riot's bold "noise should be kept minimal", and Wolfe and Horowitz's italics.

### Every change made

#### Quotations

1. **Tegel and others, PLOS ONE** (sources list).
   - Old: "All of the chest-like well linings were constructed using notched timbers that were either cogged or interlocked at their corner joints."
   - The page has "(Figure S17)" before the full stop, so the quotation silently dropped words.
   - Done: quoted exactly, with "(Figure S17)".
2. **The owner's "As I liked"** (section 3.2, the "Kept" row).
   - Old: the row was headed **Kept, "As I liked"** and covered answers 6, 7, 8 and 17.
   - `answers.md`: the owner typed "As I liked" for answers 6 and 7 only; answer 8 was "both lose it" and answer 17 "all four matter".
   - Done: the row is headed **Kept**, and "As I liked" stands beside answers 6 and 7.

#### Titles and source details

3. **Xbox Wire's title.** Old: "Minecraft: Vibrant Visuals". The page's title is "Minecraft: Vibrant Visuals Transforms the Game Into What You've Always Imagined in Your Head". Done: the full title.
4. **The owner's answers.** Old: "the owner's 32 answers". `answers.md` has answers 1 to 36 (and 34b), and section 3.2 already said 36. Done: 36.
5. **The tipi article's date.** Old: "Points West (first published 1979)". The page says Gene Ball "penned this article in 1979" and that it was "Originally published in Points West magazine Summer 2016", put online on 13 March 2020. Done: the title, "Points West, Summer 2016 (written in 1979; online 13 March 2020)".

#### Numbers

6. **Two superlatives that contradicted the note's own table** (section 3.4, "What each moment adds").
   - "Mist: the biggest calm masses (0.43)": the cave's masses are 0.51 in the same table. Done: "(0.43, after the cave's darkness, 0.51)".
   - "The village: the most light (35%)": winter is 58%. Done: "the most light after winter (35%)".
7. **Where the measures' outputs are** (the note's header).
   - Old: the scripts are listed "each with a `.txt` of its output".
   - Four saved none: `blocksize.py`, `localgrid.py`, `halfres.py` and `farsmooth.py` only printed their results (the `blocksize*.json` files are their inputs).
   - Done: the header says so. I re-ran the first three (they write nothing; output in `R/work/checker/1/rerun-*.txt`). They reproduce every number the note gives: grain 4 to 5 pixels for every picture and material, and 2, 3 and 4 on exact grids; a local grid in 19.0% and 19.8% of the liked pictures' windows, 0 to 2.1% of the round-2 pictures' and 100% of the snapped ones'; half resolution 0.210 to 0.126 and edges 0.322 to 0.189. `farsmooth.py` also writes pictures into the study's folder, so I did not run it; its numbers (0.012 to 0.014) match `look-notes.md`.

#### The items the study left "to check" or second-hand (6.4)

8. **Hooped buckets and barrels:** a first-hand source found.
   - Old: "hoops and barrels belong to much later coopers (to check)".
   - Quibell, the excavator of Hesy's tomb (1913, page 26, the Internet Archive's copy): "It will be noted that real cooper's work is intended, — barrels with bevelled staves." The tomb is of King Neterkhet's Third Dynasty; its date, about 2600 BC, is Wikipedia's ("Cooper (profession)", secondary).
   - Done: the row says coopered barrels are known from about 2600 BC, and whether any are older is still to check. That is still about two thousand years after the first copper, so the flag stands, now with a source.
9. **The potter's wheel:** a first-hand source found, and it weakens the flag.
   - Old: listed among "Metal-age tools and goods ... later than the scenes".
   - Roux and Harush (2022), abstract on Bar-Ilan University's portal: "The sign value of the first potter's wheels used in the southern Levant (second half of the 5th mill. BC) is explored through the production modalities of V-shaped bowls, the main category of vessels shaped on the wheel at that time." Its title places these vessels in the Late Chalcolithic, that is, the Copper Age.
   - Done: the wheel has its own row, "not clearly later". The jar GPT drew on a wheel (seen in the study's crops, `R/work2/1/v/08-crops.png`) is still to date.
10. **White woolly sheep:** first-hand sources found, and they weaken the flag.
    - Old: the sources list said they were "marked 'to check' in 6.4", but 6.4 never mentioned them. The study's working notes flagged them in picture 08.
    - Becker and others (2016, eTopoi) call the start of wool production "still largely unclear", placing it in "later Neolithic and Chalcolithic societies", with the first written sources at the "end of the 4th to beginning of the 3rd millennium BCE".
    - Anaya and others (2024, Animals) give the cautious view: "the probable absence of woolly sheep breeds in Western Europe before the 3rd millennium BCE".
    - Done: a row "not clearly later". When sheep became white is still to check.
11. **Lattice windows:** no first-hand source found. What I found says only that prehistoric houses had small windows or none. Left "to check".
12. **The pot hung over the fire, the metal sickle, glass-bead colours, the rucksack, the sled and the maize-like cobs:** not researched further. Left marked, as before.
13. **Przewalski's horse (Britannica, secondary):** the quotation holds. Added a first-hand source: the Smithsonian's National Zoo, which breeds the horses, says "They are dun-colored with a dark zebra-like erect mane and no forelock." The table now quotes the zoo and keeps Britannica's "short".
14. **The Pesse canoe (Wikipedia, secondary):** the quotations hold. The Drents Museum, which holds the canoe, calls it "the oldest boat in the world". Its page shows the wood and the date only through JavaScript, which the tools cannot read. Left marked secondary.
15. The "Not verified" line in the sources now lists what is still open. The six new sources are listed with their quotations.

### Checked and left as they were

- Every `PROJECT.md` and `ARCHITECTURE.md` quotation sits under the item or rule the note names. For example, `PRE-30` does name Minecraft's Vibrant Visuals, and A5.2's rules 2, 5, 6, 7 and 10 say what the note says.
- The phone's numbers: 1,080 pixels across at 390 dpi (`ARCHITECTURE.md` A4.3); 32 to 36 cm (Bababekova and others, 2011, abstract from NCBI: 32.2 and 36.2 cm); one minute of arc (Webvision). The arithmetic holds: 0.065 mm and 0.7 minutes a pixel; at 8 m across, 8.4 pixels a texel at 16 a metre and 2.1 at 64; a 1.7 m adult about 230 pixels tall.
- t3ssel8r's rain: the video description lists eight effects, and streaks, splashes, ripples and mist are four of them, as the note says.
- The GPT requests: each "avoid" line the note says was ignored is in the request (thwarts and flat stumps, long flowing manes, striped piglets twice). The deer was asked for "on a pole", and `d2-pixel-paint` asked for about 3 screen pixels. `gpt-ledger.txt` gives study 1 "16 of 32".
- One description not from its source, left alone since it is not quoted: the 4.1 table calls Valheim "Low-detail 3D with low-resolution textures". The PCGamesN article never mentions textures; the quotation itself holds.
- Section 3.6 gives question 31's texels as about 1.5 and 7 screen pixels. The study's working notes say about 1.4 and 6, and no output file holds them. The two agree within "about", so I left them.

### Changes that alter the note's meaning or recommendation

- **Two of 6.4's truth verdicts change.** A potter's wheel and white woolly sheep are no longer certain anachronisms in a first-copper village. First-hand sources put the first potter's wheels (southern Levant, second half of the 5th millennium BC, mainly for V-shaped bowls) and the uncertain start of wool production within the Copper Age. A village target should date them case by case, not strike them out. The jar on a wheel and the white fleece are still to check.
- **Hooped barrels stay flagged,** now with a first-hand source (about 2600 BC). Whether older ones exist is still open.
- **Nothing changes the note's recommendation:** direction B, the 2-pixel texel at 64 a metre, zoom bands drawn as pixel art, light only for readability, and a truth check before any target picture.

## Quote check: notes 2 (drawing) and 3 (engine)

Checked on 6 October 2026 by the quote checker, as `CHECKER.md` asks. `R` is the research folder. The lists, the checking script and its outputs are in `R/work/checker/2-3/`: `check.py`, `q2.lst`, `q3.lst`, the `.out` files, the original notes and `webfetch-readings.txt`.

The check is case-sensitive. The only differences it allows are typographic quotes, dashes, spacing and line breaks. A quote counts once for each place it appears, in the body and again in the sources list. Words in quotation marks that are the note's own (proposed wording, defined terms such as "Flicker", the author's "game-like") are not counted as quotes.

### Totals

| Note | Quotes checked | Held | Changes |
|---|---|---|---|
| 2, drawing | 196 | 195 | 1 quote reworded; 1 number's wording fixed; the Bevy issue's status updated |
| 3, engine | 198 | 198 | 2 claims corrected (alpha to coverage); 1 pair of numbers marked unverified; the Bevy issue's status updated |

**How note 2's quotes were checked:**
- 54 in Godot 4.7.2's source and class reference, using the checkout at the tag (`R/work/4/godot`, commit ed1daf0).
- 13 in the saved Godot 4.7 documentation pages.
- 91 in other saved pages: Imagination, t3ssel8r's feed, CptPotato's readme, Hytale, Minecraft, Sassone, RetroArch, Holland and Google's display-settings page.
- 38 in the repository, git history (`git show 29beda4^:prototypes/app/zoom/zoom.gd`), the owner's `answers.md`, round 1's notes, study 6's note and study 1's GPT requests.
- None needed a fetch. Its unquoted mentions of Bevy issue 25788, Godot issue 115171 and pull request 123614 were checked through WebFetch.

**How note 3's quotes were checked:**
- 56 in Godot's source and class reference.
- 4 in the saved Godot 4.7 documentation pages.
- 64 in other saved pages. Saber's three slide quotes hold in the second text extraction of the same saved PDF (`saber2025_raw.txt`); the layout extraction splits the slide's columns.
- 70 in the repository, the owner's answers and the other studies' notes.
- 4 through WebFetch: Bevy issue 25788's title, error line and driver build, and the error line again in section 4.7.

### Changes to note 2

1. **"foreshortening at 40°"** (section 9, for study 3).
   - *Page:* none. It labels a question study 3 sent to study 2, and no saved file holds it.
   - *Done:* reworded without quotation marks: "answer to study 3's question on foreshortening at 40°".
   - *Meaning:* unchanged.
2. **A number** (section 5.2, option B): "about 1.5 to 3.5 times the flicker of mipmapped reading when texels are under a pixel".
   - *Source:* `R/work2/2/steady/levels.out` gives 1.5–1.9 times at one screen pixel a texel and 2.1–3.4 times at half a pixel.
   - *Done:* "under a pixel" became "a screen pixel or smaller", so the range matches its source.
   - *Meaning:* unchanged.
3. **Bevy issue 25788** (section 3.1 and section 11). The note said the issue "could not be fetched again" and marked it **unverified**.
   - *Check:* the raw page and GitHub's API return 403 to this session, and no search engine holds a copy. WebFetch read the page twice, and both readings agree: a crash in PowerVR's shader compiler while creating a compute pipeline, before the first frame, on a Pixel 11 Pro XL with driver 25.3@6908880. The issue has been open since 14 September 2026.
   - *Done:* both places now say it was read through a summarising fetch, in round 1 and by the checker.
   - *Meaning:* the support for A4.3's rule against compute programs that sample textures is stronger. The recommendation is unchanged.

### Changes to note 3

1. **Alpha to coverage** (section 4.7 and section 6.4, item 3). This change alters the note's meaning.
   - *What the note said:* "Alpha to coverage alone keeps them solid." Section 6.4 added: "Their edges are smoothed by alpha to coverage under MSAA, which keeps them in the solid pass."
   - *The quote:* it holds ("bool has_base_alpha = (uses_alpha && (!uses_alpha_clip || uses_alpha_antialiasing));"), but Godot 4.7.2's source does not support the claim built on it:
     - The `alpha_to_coverage` render mode only sets the blend mode (`scene_shader_forward_mobile.cpp`, lines 106 and 247–249).
     - The flag that moves a material to the blended pass is set by writing `ALPHA_ANTIALIASING_EDGE` or `ALPHA_TEXTURE_COORDINATE` (lines 128–129).
     - Without that edge, the Mobile shader sets a cut-out's alpha to 1: "// If we are not edge antialiasing, we need to remove the output alpha channel from scissor and hash" (`scene_forward_mobile.glsl`, lines 1386–1388). Coverage is then full, so nothing is smoothed.
     - Godot's standard material writes the edge whenever its alpha antialiasing is on (`scene/resources/material.cpp`, lines 1815–1817). Such materials go into the alpha render list (`render_forward_mobile.cpp`, lines 2319–2320).
   - *Done:* section 4.7 now says the render mode alone keeps cut-outs solid but smooths nothing, quoting the shader's comment, and that a cut-out smoothed by alpha to coverage is drawn in the blended pass. Section 6.4, item 3 now says smoothing their edges this way moves them to the blended pass. The three source lines are added to section 11.
   - *Effect:* smooth plant edges are not free in the solid pass; they cost the blended pass, which is what note 2's phone question 3 measures. Note 2 (section 4.6) already had this right. Drawing cut-outs after solid things by render priority still holds. Section 10's "cut-outs drawn after solid things with alpha to coverage" was left as written.
2. **Study 4's early figures** (section 3.3): "about 9–25 ms naive and 4.5–12 ms with savings".
   - *Source:* study 4's handover at the time, which has since been rewritten. No saved file holds 9–25, and only study 1's note repeats 4.5–12.
   - *Done:* marked unverified where they are given. Section 6.12 ("about half of mine") and section 10 ("about twice its first figures") compare against them and were left as they are.
   - *For the builder:* study 4's final model (`R/work2/4/faux3.out`) puts the liked camp at the closest zoom at 13.3–38.3 ms naive and 5.5–16.6 ms with the savings (central 9.6 ms). So "about half of mine" no longer describes study 4's figures. Note 3's conclusion, that full size fits only at the optimistic end, stands either way.
3. **Bevy issue 25788** (section 11). The title, error line and driver build were marked unverified.
   - *Check:* both WebFetch readings give these exact words.
   - *Done:* the line now says so, and that the raw page and API stay closed.
   - *Meaning:* unchanged.

### Numbers checked

**Note 2.** Every number marked *computed here* matches its script's saved output:
- the filter table, the 38–47% cut in flicker and 94–98% crispness (`steady.out`);
- the level contrasts and the 8–28% loss (`levels.out`);
- the lens table, the texel ladder, 41.5 MB for MSAA and 20.8 MB for HDR 2D (`lens.out`);
- the light table, the shade colours, the nights, fire reach, snow and banding (`light_measures2.out`, `fire_reach.out`, `snow.out`, `banding.out`);
- the corner-darkening change (`light_measures.out`);
- the readability table and its means, 39% against 16%, and colour strength 0.081 to 0.069 (`salience-read.out`, `change.out`).

Sourced numbers also hold:
- the four t3ssel8r video dates, Hytale's date (22 December 2025), Sassone's (12 April 2021) and Holland's (24 September 2024);
- Imagination's 22% and 3%, and Godot's 8 decals a mesh;
- the prototype's 25° lens, `KEEP_WIDTH` and 30° tilt at the camp;
- the `.mobile` soft-shadow default, 0, which Godot's enum names "Hard (Fastest)".

The only fix is change 2 above.

**Note 3.** Re-running its scripts (`zoomsB.py`, `figuresB.py`, `full.py`) reproduces every value in sections 1, 3.6, 6.3, 6.4, 6.6, 6.7, 6.12 and 6.13. Sourced numbers hold:
- Imagination's 32 pixels a primitive and 22% to 3%;
- Horizon's 100,000+ objects and about 250 µs (GDC 2017);
- Saber's 5 MB, 300K instances and the 0–127 palette index;
- Zugalu's 1,000 to 20 draw calls (25 June 2025);
- Assassin's Creed Unity's 40, 120 and 10,000 (GDC 2015);
- Dead Cells' 50 pixels (2018);
- Godot's 8 omni lights a mesh;
- HypeHype's ~90% (REAC 2023);
- Unity's 8 bytes instead of 112 and 14×;
- Cities: Skylines II's 4828 of 6705 (November 2023);
- the checkout's commit, ed1daf0, and `PLT-04`'s 8 GiB;
- the cross-study figures for study 4's round 1, study 1, study 5 and study 7.

The only exception is change 2 above.

### Seen but not changed (for the builder)

1. **Godot's Mobile renderer turns blending on for every material that writes ALPHA, plain cut-outs included.**
   - *Source:* `scene_shader_forward_mobile.cpp`, lines 390–400. The default MIX blend also sets `enable_blend = true` (`material_storage.cpp`, lines 655–656).
   - *Why it matters:* note 2's "nothing: it adds blending" (section 5.7) compares alpha to coverage with cut-outs that already blend; what alpha to coverage really adds is the move to the see-through pass. Whether PowerVR treats blend-on cut-outs as Imagination's costly "alpha blended edge" bears on both notes' plant costs and on study 4's model. Only the phone can say.
2. **Note 3, section 6.9: "texels stay within about 5–11% either way of 2 × 2"** with a 5–10° view.
   - It is marked an estimate and no saved script holds it.
   - At a 40° tilt, it fits texel widths for a 5–10° vertical field of view. Heights spread about twice as much.
   - Note 2's `lens.out` gives 0.79 wide and 0.63 tall, top against bottom, for 5° across a portrait screen.
   - Both notes recommend the same narrow lens.
3. **Note 2, risk 7: Godot pull request 123614.** Its author only suspects the compilers "are not thread safe", and its milestone is now 4.8 (round 1 had 4.7). Issue 115171 is as described.
4. **Note 3, section 4.5, calls Filament "Android's own renderer".** It is Google's renderer for Android and other platforms, not the one built into Android.

## Quote check: notes 4 (the phone) and 5 (the content)

Checked on 6 October 2026, following `CHECKER.md`.

**How quotes were checked:**
- Each quotation was matched against the one page or file the note credits it to.
- A `PROJECT.md` quote was matched within its own item, and an `ARCHITECTURE.md` or `IMPLEMENTATION.md` quote within its own section.
- Only differences in typographic quotes, dashes, spacing and line breaks were allowed.
- A passage quoted in both the body and the Sources counts as one quote.

**Where the working files are:**
- The scripts are in `R/work/checker/4-5/` (`check.py` and `quotes.py`).
- `final.txt` lists every quote with the file it was checked in.
- `fetched.txt` records the pages fetched live.

### Totals

| Note | Quotes checked | Held as written | Quotes changed | Other changes | Left unverified |
|---|---|---|---|---|---|
| `4-phone.md` | 78 | 78 | 0 | 1 statement after a quote softened | 0 |
| `5-content.md` | 96 | 95 | 1 replaced | 1 date corrected | 0 |

### Note 4, the phone

**78 quotes, all held.** By kind of check:
- **Repository, 17:**
  - `PROJECT.md`: `PLT-04` (3), `PLT-01` (2), `PLT-03`, `PRN-11`, `VIS-14`, `RES-09`, `RSK-24`, and "made by code" in `MIL-09`, worded the same way in BRIEF2.
  - `ARCHITECTURE.md`: A4.1, A18.1 and A3.9.
  - `IMPLEMENTATION.md` M2, `heat.toml` and `tools/verify-apk.sh`.
- **Owner, 3:** "As I liked" (answers 6 and 7), "only if needed" (25) and "alive" (26), all in `answers.md`.
- **Saved pages, 42:**
  - Imagination's guides: 17. Each saved page's title matches the page cited.
  - Godot's docs at 4.7: 6 (VRS ×3, GPU optimisation, 3D performance, and the fish page from the docs' 4.7 source).
  - Android: 7 (`thermal.h` ×3, `system_health.h` ×2, `PowerStatsService.java` ×2).
  - Vulkan database report 51167: 2.
  - Android Authority 4, Notebookcheck 1, GSMArena 1, the Godot forum post 1 and Basis Universal's README 2.
- **Godot 4.7.2 source, 16:** read in the checkout under `R/work/4/godot`, confirmed as tag `4.7.2-stable`. Every line was read in context and supports the use the note makes of it:
  - the plain colour and depth buffers keep the default `p_discardable = false`, while the MSAA buffers pass `true`;
  - VRS is turned on only for a shading-rate texture or a density map;
  - the depth test is greater-or-equal, or less when inverted, with no equal test;
  - the export's app category defaults to game;
  - the ASTC encoder is built for editor builds only;
  - Basis files become ASTC 4 × 4 where the chip supports it;
  - a grade through Godot's adjustments and a colour table stays in the merged pass.

**Numbers and facts checked, all held:**
- **Pre-production and M1:**
  - P1 9.5–10.1 ms, partly idle; P2's forest at 17.8 ms with 65% on time; P3 4.3 ms; the mirror at most 0.3 ms.
  - P2's 5.5 ms at 120 was indeed "the picture's own pass": git commit `88fef2d` shows P2 timed only the art viewport.
  - M1: 1.0 W at real speed, 5.0–5.6 W at top speed, a heat forecast of 0.77–0.83.
  - "About four minutes": the top-speed scenario runs 240 s (`sim/src/kd/bench/scenarios.cpp`).
  - The screen at about 1 W: `dist/NOTE.md` says the still-camera scenario is "for the screen's own power", and it measured 1.1 W.
- **Sizes and screen:**
  - The APK is 28,985,099 bytes, which is 27.6 MiB.
  - The 50 MB rule is 50 × 1024 × 1024 bytes, which leaves about 22 MiB of room.
  - 390 dpi is in `ARCHITECTURE.md` A4.3.
- **Vulkan database report 51167:** every value listed holds: device and driver, Vulkan 1.4.317, Android 17.0, the three shading-rate flags, `maxFragmentSize` [4,4], texel size [0,0], a lazily allocated memory type and `shaderFloat16`.
- **The database's coverage lists:** they show the shading-rate extension on the Pixel 11, 11 Pro and 11 Pro XL and the Pixel 10 family, and no PowerVR device with the density map.
- **Android:**
  - Thresholds came in API 35, the headroom listener in 36, and GPU headroom in 36.
  - GPU headroom runs from 0 to 100, and an unsupported phone gets ENOTSUP.
  - Power rails refresh every 30 s, with up to 10 J of noise.
  - The Game Dashboard paraphrase holds. It was fetched: "A live FPS counter", "available on all Pixel devices running Android 12 or higher".
- **Reviews:**
  - Android Authority's dates (20 August 2026, 10 September 2026, 26 October 2025) hold, and Genshin Impact is the game in both quotes.
  - Notebookcheck, 29 August 2026, from Geekerwan; GSMArena's 5,115 mAh.
  - The forum post is from January 2026, Godot 4.2, on a desktop.
  - Basis UASTC has its optional rate-distortion step.
- **Study 4's model (`faux3.out`, `patches.out`):**
  - The scene table in 5.2, the main-thread table in 5.6, power at 8 and 10 ms, and the central ranges of ways C, D and E all match.
  - The model's output labels the village "80 in view" but computes it with 90, which is the number the note uses.

**Change:**
1. **Section 4.5, crowds in Godot.**
   - *Old:* … "so 50-100 would be safe". The cost is on the processor.
   - *Page:* `R/work2/4/pages/forum-130409.json`, post 5: "I'm not sure but I think it's more CPU than GPU, you'll have to do your own tests."
   - *Done:* the quote held. The sentence after it now reads: "Its author was not sure, but thought the cost lay more on the processor than on the graphics chip."
   - *Why:* the note stated as fact what its source offered as a guess.

### Note 5, the content

**96 quotes; 95 held, 1 replaced.** By kind of check:
- **Repository, 27:**
  - `PROJECT.md`: `MIL-09`, `PRE-46`, `PRE-27`, `PRE-44`, `PRE-43`, `MAT-07`, `PRE-28`, `PRN-10`, `SCP-20` and `PRC-01`.
  - `ARCHITECTURE.md`: A6.1, A6.3, A5.3 and A5.2.
  - `LESSONS.md` P3.
- **Owner, 2:** answer 13, and answers 6 and 7.
- **GPT's request files, 2:**
  - "no metal" is in `made-things-sheet.prompt`;
  - "lying on its side, legs out" is in `deer-poses.prompt`.
- **Saved pages, 57,** in `R/work2/5/pages`: Adobe, Microsoft ×3, Slynyrd ×3, Kopf ×3, Castaño ×3, GodotGrass ×2, Hytale ×5, Hecker ×3, Jane Ng, the fish page, Claude-of-Duty ×3, Pixel Snapper ×2, Claude's vision page ×3, the French Ministry ×3, Mount Sandel ×3, York ×2, *Antiquity*, PMC ×2, the museum guide ×3, the Iceman museum ×3, OpenAI's 2024 terms ×3 (the windriver copy the note names) and the Copyright Office ×2.
- **Godot 4.7.2 source, 6:** `BaseMaterial3D.xml` ×2, `ProjectSettings.xml` ×2, `Image.xml` and `mesh_storage.cpp`. Each doc line belongs to the constant, setting or method the note names (`TEXTURE_FILTER_NEAREST_WITH_MIPMAPS`, `use_nearest_mipmap_filter`, `create_from_data`).
- **Fetched live, 1:**
  - The ConductAtlas quote. Both saved copies are bot-check pages ("Enable JavaScript and cookies to continue").
  - Both live pages hold the sentence.
  - Their dates match the note: the rest-of-world terms were updated 5 March 2026, the EU terms 11 May 2026, and both were checked by the site on 9 July 2026.

**Numbers and facts checked, all held except the Slynyrd date:**
- **Authors and dates:**
  - Adobe: Frank Grießhammer, 4 March 2021.
  - Kopf, Shamir and Peers, SIGGRAPH Asia 2013 (fetched).
  - Castaño's original was on The Witness's site in September 2010.
  - GodotGrass is MIT-licensed (its LICENSE file, fetched).
  - Hytale: 22 December 2025, by its art director. Characters (and the tools and food they carry) get 64 px a unit, props and blocks 32.
  - Jane Ng: 28 September 2013.
  - Hecker et al.: 2008.
  - Claude-of-Duty: commits of 25 July 2026 (fetched).
  - The fish page is marked outdated.
- **Archaeology:**
  - Mount Sandel: "mid 7th millennium BC (uncorrected C14 dating)", in the 1974 report.
  - Star Carr: "at least 8,500 years BC", posted 10 August 2010.
  - *Antiquity* volume 94 (2020), Pryor et al.
  - PNAS 2004: Frank Hole's commentary; Ohalo II "dated to 23,000 years ago".
- **Terms:** OpenAI's terms took effect on 11 December 2024; the Copyright Office report is from January 2025.
- **Godot identifiers:**
  - `_generate_po2_mipmap` with `average_4_uint8`;
  - the importer options, with no coverage option;
  - `alpha_to_coverage` and `alpha_to_coverage_and_one`;
  - the skeleton-version test.
  - The importer's path, `editor/import/resource_importer_texture.cpp`, is right at the tag. The checkout has no `editor/` folder, so I fetched the file; it is byte-identical to study 5's saved copy.
- **Study 5's own measurements:**
  - From `meas/numbers-round2b.txt`: speckle cut from 3.0–14.9% to 0.2–3.4% on eight swatches; daisies 7/4/3 and bilberries 12/5/1; 5 against 1 and 2 against 0 at the close camp, 18 against 8 and 11 against 5 at 16 m; coverage within 0.006 against drift of up to 0.07; 99.9% of plant pixels partly see-through; the tuft 327/192/86 against 120/60/30; a 30 cm tuft 40 and 9 screen pixels tall.
  - From `meas/tex-describe.json`: the six materials' lightness within 8.9 levels (grass 107.3 against 107.4, hide 154.2 against 145.3); grid 1.30–1.88 against 1.06–1.15; seam steps 2.7–20.9 times (the smoothed measure); corners up to 54.4 levels darker on hide; slopes 0.71 on granite and 0.74–0.80 on birch.
  - `meas/spill.py` re-run, writing nothing: magenta on 94.3% of the birch sheet's edge pixels and 81.8% of the ground cover's.

**Changes:**
1. **Section 6.11, proposal 3 (`PRE-27`).**
   - *Old quote, credited to "studies 1 and 3, whose sentence I support":* "Small 3D figures on one skeleton, with a separate head, torso, arms and legs, posed about 10 times a second (`PRE-44`), in as much detail as the zoom shows, so they read as crisp pixel art from any angle".
   - *Pages:* `notes/3-engine.md` (proposal 11) and `notes/1-look.md` (P-6). The sentence is in neither note, and in no draft or text file anywhere in the research folder.
   - *What the studies say:* study 3 proposes "3D figures as detailed as in the pictures the owner liked, with a separate head, torso, arms and legs that bend at the joints, posed about 10 times a second (`PRE-44`), so they look like crisp pixel art from any angle". Study 1 proposes "Detailed 3D figures, with a separate head, torso, arms and legs, posed about 10 times a second (`PRE-44`), wearing the world's pixel-art textures: …".
   - *Done:* the quote is replaced with study 3's exact words, plus "and study 1 a close wording". Study 5's own addition, "with a face drawn for each band of zoom", stays.
   - *Why:* the quoted words were on neither page, though both pages say much the same in other words.
2. **Sources, Slynyrd.**
   - *Old:* "(Raymond Schlitter, 7 March 2019)".
   - *Page:* `slynyrd-15.html` gives the post's date as 19 March 2019 (`datePublished` 2019-03-19); 7 March is only in its web address.
   - *Done:* changed to "19 March 2019". The body's "2019" was already right.

### Changes that alter a note's meaning or recommendation

No recommendation changes. Two changes touch meaning:
- **Note 5, the `PRE-27` proposal:**
  - The wording now put to the owner is study 3's. It no longer says "on one skeleton" or "in as much detail as the zoom shows". The skeleton remains in proposal 5 (A6.1 and A6.3), where implementation belongs.
  - Studies 1 and 3 word `PRE-27` differently, so whoever merges the proposals must choose one, as note 5 already says for A5.3.
- **Note 4, crowds in Godot:**
  - A supporting statement is weaker: the post's author only guessed the cost was on the processor.
  - The recommendation stands: Godot's skeletons for the camp of thirty, our batched animation for crowds, and C6 to settle the number. It rests on Godot's own docs ("bones are animated on the CPU") and on C6, not on this post.

### Noticed, not changed (no source involved)

Note 5's bands do not quite match its own numbers:
- **What the note says:** the close camp is about 17 texels a metre, because each band is half the one before.
- **What its numbers file says:** 15 texels a metre at the close camp's 36 m across.
- **What it means:** at about 17 texels a metre, the band edge falls near 32 m across, not 36.

## Quote check: notes 6 (the loop) and 7 (the pipeline)

*Checked on 6 October 2026, as `CHECKER.md` and `research/README.md` require. Only `6-loop.md` and `7-pipeline.md` were edited. The scripts, the quote list and the pages fetched for this check are in `R/work/checker/6-7/` (`quotes.py` lists every quote with its claimed source; `check.py` repeats the whole check).*

### How each quote was checked

- **The rule:** the quoted words must appear on the page exactly. Only typographic quotes, dashes, spacing and line breaks may differ (a PDF's hyphen at a line break counts as a line break).
- **Saved copies first:** study 7's saved pages (`R/work2/7/pages/`), round 1's (`R/work/`), TASTE's saved PDF text, and the Godot 4.7.2 checkout (`R/work/4/godot`, at tag `4.7.2-stable`).
- **Godot's importer file** is missing from that partial checkout. The two saved copies (`R/work2/7/godot/`, `R/work/7/`) hash to exactly the tag's blob (`ce08517…`), so they are the 4.7.2 file.
- **Repository quotes:** `PROJECT.md`, `ARCHITECTURE.md`, `IMPLEMENTATION.md`, `LESSONS.md`, `dist/M1-REPORT.md`, `tools/verify-apk.sh`, and `git show 3d62168:ARCHITECTURE.md`. Each `PROJECT.md` quote was also checked inside the item it is credited to. The repository's working tree has uncommitted edits (new proposals in `PROJECT.md`), so every repository quote was checked against both the working tree and the last commit. They hold in both.
- **Owner quotes:** checked against `R/owner/ask/answers.md` and `R/BRIEF2.md`.
- **Pages with no saved copy:** 14 of note 6's sources and one of note 7's had none. Their pages (16 files for note 6, as two papers needed both their abstract and full text, and one for note 7) were saved with curl into `R/work/checker/6-7/web/` and searched for the exact words.
- **OpenAI's Terms of Use** refuse every tool (403; the saved copies are "Enable JavaScript" pages). Both quotes were checked against a search engine's copy of the current page and against study 5's saved copy of the 11 December 2024 version.
- **Numbers credited to a source** were checked the same way. The studies' own measurements from their scripts are not quotes, so they were not re-run.

### Note 6, the loop

**Quotes checked: 94** (128 places in the note, counting a quote again where the Sources list repeats it). **Held as written: 92. Changed: 2.**

| Kind of check | Quotes |
|---|---|
| Pages fetched for this check (arXiv abstracts and full texts, Kellogg, MERL, CVPR, Oulu, NVIDIA) | 44 |
| Repository files and git history | 20 |
| Owner files (`answers.md`, `BRIEF2.md`) | 18 |
| The Codex image skill (`/root/.codex/skills/.system/imagegen/SKILL.md`, on this machine) | 5 |
| Saved copies (TASTE's PDF, Mantiuk 2012) | 4 |
| Study files (GPT prompts, the round-1 note) | 3 |

**Changes:**
1. **"made by code" in `MIL-09` and `PRE-46`** (section 2).
   - The page: `PROJECT.md`. The words are in `MIL-09` only ("the model kit and its textures made by code"). `PRE-46` does not contain them.
   - Done: changed to `MIL-09`'s "the model kit and its textures made by code" (the kit of `PRE-46`). Added it and `PRE-27`'s "tiny blocks" to the Sources list, which had left both out.
   - Why: the quote was credited to an item that does not hold it.
2. **"pick all that feel right"** (section 4.6).
   - The page: `answers.md`. The phrase is not there or in the question pictures. The file only marks questions as "multiple choice".
   - Done: reworded without quotes: "They picked several options where a question allowed it (the answers file's multiple-choice questions)".
   - Why: the words could not be found.

**Numbers and claims confirmed** (no change needed):
- **Visual Aesthetic Benchmark (12 May 2026):** the strongest system's 26.5% and the lower 21.8% and 15.5% are all on the same strict measure (TB-1 pass^3).
- **TASTE:** its own definition of the position-bias rate matches the note's paraphrase ("the fraction of pairs whose verdict is unchanged when image order is flipped"). Its six judges are open-weight.
- **VideoGameQA-Bench:**
  - the best judge's 24.0% is the visual-regression column of its Table 2.
  - 82.8% is the best image-glitch score.
  - The paper is in NeurIPS 2025's Datasets and Benchmarks track (proceedings page, found by search).
- **Authors and dates:** every paper's authors, year and venue match the pages.
- **flip-evaluator 1.7** exists on PyPI.
- **Study 6's own record:** 11 of 32 pictures used (ledger); the allowance went 4, 8, 16, 32 (three raises, `BRIEF2.md`); requests asked for 1024 × 1536 (logs).
- **The phone's screen:** 1080 × 2404 at 390 dpi is in `LESSONS.md` and `ARCHITECTURE.md`.
- **The owner's answers:** six of the nine relit pictures were picked.
- **Question 23's picture** does say "A has people in vests and loincloths that GPT added", so the lesson in 4.6 stands.

**Left as they are:** "steady pixels" (a term from the study's brief, `R/prompts/study6.txt`, not credited to any page); "look lab", "smooth pixel" and "first light" (terms); and the note's own proposed wording and questions.

### Note 7, the pipeline

**Quotes checked: 75** (107 places in the note). **Held as written: 71. Changed: 4.** Two other lines changed: a number's credit, and the terms' source line.

| Kind of check | Quotes |
|---|---|
| Saved copies (Godot 4.7 docs, GitHub, Codex pricing, Imagination, Arm, Khronos, Basis, Heitz, Adobe, Unreal, C2PA, GPU Gems) | 34 |
| Repository files | 14 |
| Godot 4.7.2 source (checkout, or saved copies proven identical to the tag) | 11 |
| Owner files (`answers.md`) | 8 |
| OpenAI terms: a search engine's copy, and the saved 2024 copy | 4 |
| Study files (GPT prompts, the round-1 note) | 3 |
| A page fetched for this check (Microsoft, 2013) | 1 |

**Changes:**
1. **"2 × 2, the finest"** (3.3) → **"2x2, the finest"**.
   - The page: `answers.md` (answer 2).
   - Why: the page writes "2x2". An "x" is not a typographic difference.
2. **"2 × 2"** (section 9, study 1) → **"2x2"**. Same page, same reason.
3. **"Colour alone doesn't bring the feeling"** (3.3, answer 24) → **"colour alone doesn't bring the feeling"**.
   - The page: `answers.md`.
   - Why: the page has a lower-case "colour", after "no,".
4. **"The owner's answer 31 asks for "a design for each zoom band, not just shrinking""** (5.3).
   - The page: `answers.md`. The owner's answer 31 is "neither", with the note that both grounds turn to speckle at the close camp. The quoted words are the builder's summary further down the file, not the owner's.
   - Done: now reads: the owner answered q31 "neither": at the close camp both grounds turn to speckle, which the answers file reads as textures needing "a design for each zoom band, not just shrinking".
   - Why: the words held, but they were credited to the owner.
5. **Not a quote: "at 60 frames a second (`PLT-04`, `PLT-01`)"** (section 2).
   - Neither item says 60. The figure is `ARCHITECTURE.md` A18.1's frame budget for `PLT-04` ("16.7 ms at 60 frames a second").
   - Done: added "the 60 is `ARCHITECTURE.md` A18.1's frame budget".
6. **The OpenAI terms' source line** (section 11).
   - Both quotes held against a search engine's copy of the current page and against the 2024 copy.
   - Done: replaced "the second quote against a copy … which may be out of date" with what the check found.

**Numbers and claims confirmed** (no change needed):
- **Godot documentation:**
  - Its table gives 341 KiB lossless and 85 KiB VRAM-compressed for 256 × 256 with mipmaps.
  - "Detect 3D" changes the mode to VRAM Compressed.
  - "recommended setting for pixel art" is said of Lossless.
  - The DDS sentence is about DDS, and the KTX limits are about KTX.
- **Godot source:**
  - The `p_out` line is in `average_4_uint8`, which `_generate_po2_mipmap` uses for 8-bit formats.
  - The size line is in `Image::initialize_data`. `create_from_data`, `load_png_from_buffer` and `load_ktx_from_buffer` are bound.
  - `texture.mipmaps` is set in `texture_2d_initialize`.
  - The importer's mipmap test is in `_save_ctex`, and its loop in `COMPRESS_LOSSLESS`.
  - The two "All images must share" messages are in `ImageTextureLayered::create_from_images`.
  - The Basis packer is only under `TOOLS_ENABLED`, and the KTX loader is not.
- **C2PA:** the sentence is in version 2.2 (fetched to confirm, since the saved page's canonical link names 2.4).
- **Page labels:** Unreal Engine 5.8; Adobe's page updated 7 April 2026.
- **Git LFS:** 10 GiB of bandwidth and storage on Free and Pro.
- **Codex pricing:** 15-160 local messages per five hours for its current model on Plus.
- **The repository, measured read-only with git:**
  - 1.06 GiB of objects.
  - 49 commits touching `dist/kindling.apk` between 2 and 6 October (two of them delete it).
  - 47 distinct APKs, 1,023 MB.
  - Today's APK is 29 MB.
- **`PROJECT.md` figures:** about 3 seconds, about 8 GiB, about 12 rock kinds, about 20 minutes (`PRC-10`). `PLT-06`, `PLT-09`, `PRE-40` and `SND-06` say what the note says.
- **GPT runs (logs and file times):**
  - Codex 0.160.1 and its image tool.
  - 43, 53, 42 and 42 seconds; the regrade took 55.
  - 23,585 tokens for `meadow-band2`.
  - 1254 × 1254 returned for 1024 × 1024 asked.
  - 5 of 16 used.
  - 72 pictures from 10:00 to 11:57, none failed. Eight further runs logged without an exit were held while the studies were paused, and run later.
- **The pictures' C2PA records:** all five carry "OpenAI OpCo, LLC", "ChatGPT" and "gpt-image" in their `caBX` chunk, and none holds its prompt.

### Changes that alter a note's meaning or recommendation

- **None changes a recommendation.**
- **One changes what is credited to the owner:** note 7's designed levels per zoom band. The owner said "neither" (both grounds turn to speckle). "A design for each zoom band, not just shrinking" is the builder's reading. The recommendation still rests on that answer and on the note's measurements, and the owner has since picked GPT's redrawn level with matched colours (answer 36, in the builder's addendum).
