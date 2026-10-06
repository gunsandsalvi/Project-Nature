# Research 19: the check of the studies' quotes

Every quotation in the seven studies' notes was checked against its page on 6 October 2026 by four checkers, each on its own notes, as `research/README.md` requires: saved copies first, the live page where none held it, and the repository for quotes of its own files.
Their reports follow, as written.

*Still running when this was written: the checks of notes 1, 2-3; their reports are added when they finish, and until then those notes stand as their studies checked them.*

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
- **Pages with no saved copy:** 15 pages, all for note 6 except one, saved with curl into `R/work/checker/6-7/web/` and searched for the exact words.
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
