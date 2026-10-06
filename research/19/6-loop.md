# Study 6, round 2: reaching a look the owner loves, and keeping it

> Study 6 of [research 19](../19-graphics.md), written on 6 October 2026 and kept as written, its quotes checked ([the check](quote-check.md)).
> `R` was the research session's working folder and is not kept, except your pictures and answers, now in `art/targets/` and `art/reviews/2026-10-06-graphics/`.

*M2 research, round 2, study 6 of 7, 6 October 2026. Research only: nothing here is decided (`PRC-07`). Every change to `PROJECT.md`, `ARCHITECTURE.md` or `IMPLEMENTATION.md` is marked as a proposal for the owner. Every number has its script in `R/work2/6/` (R = the research folder).*

**Words used here.**
- *Direction B:* the look the owner chose (answers 1 and 2): a smooth, sharp 3D world drawn at the phone's full resolution, wearing pixel-art textures.
- *Texel:* one pixel of a texture. In direction B one texel shows as about 2 × 2 screen pixels at the closest zoom.
- *OKLab:* a colour space in which equal steps look about equally different. Lightness runs from 0 (black) to 100 (white); *colour strength* is its chroma × 100.
- *Paintover:* GPT's repaint of an engine frame in the liked style. *Relight:* GPT's edit of an approved picture that changes only the hour, light or season.
- *Following the motion:* moving the last frame by the camera's known movement before comparing it with the next one, so that only flicker is left.
- *FLIP:* NVIDIA's measure of how different two pictures look to a person, for a given screen and viewing distance.
- *Percentile:* "at the 90th percentile" means more striking than 90% of the picture.

## 1. The question, and the answer in brief

**The question:** with the feeling as the goal instead of strict rules, how does the builder reach a look the owner loves and keep it, and how should GPT's pictures be used along the way?

**The answer in brief:**
- **Targets: one approved picture per place, relit for each hour and season.** Six of the nine relit pictures the owner judged kept the feeling (the other three for their light, not the method), and the owner chose a relit dusk over a repaint of the engine's own (answer 23). Repainting an engine frame gives precise targets for each material only where the frame already holds the right content (the repainted far view was turned down, answer 12), and repainting round after round drifts. Targets are measured, never copied.
- **A target card guards the feeling as an alarm, not as rules.** It holds statistics measured from the twelve liked pictures. Tested against the owner's 32 answers, it would have flagged 12 of the 18 pictures the owner turned down, but also 6 of the 17 picked. So it is a traffic light, refitted after every choice, and never a gate.
- **Checks for direction B:**
  - still frames identical;
  - texture shimmer measured after following the motion: in my simulation, 0.4–1.1% of pixels flicker with a "smooth pixel" filter, and 17–28% with plain nearest-texel sampling;
  - accents on the ground at every zoom: the ground the owner called speckle scores 11–12, against 23–30 in accepted pictures;
  - people's contrast with their surroundings, read from the engine's own object picture.
- **Savings must be proved invisible.** The owner turned down every visible trade-off. A saving passes if it differs from full quality far less than the half resolution the owner could see (5.3% of pixels above FLIP 0.2 at 30 cm), and if the owner can't pick it out in a blind test on the phone. Half resolution applied to textures also brings the shimmer back (9–16% of pixels), so any half-resolution saving touches only smooth parts: light, shadow, haze.
- **The owner's choices:** two to four labelled pictures with a one-line cost each, in batches of four, arriving with builds the owner opens anyway. Options are shown free of known faults. Every choice goes into a taste log, and galleries come from the engine, never from GPT.
- **The AI judge advises, never decides.** It sees pairs, asked twice with the order swapped. The best model in a May 2026 test named both the best and the worst picture of a set, in all three orderings tried, in 26.5% of tasks, against experts' 68.9%. The owner's 32 answers become its exam before its advice counts.
- **M2's order:**
  1. the card, the checks and the judge's exam;
  2. a "first light" corner on the phone at the closest zoom;
  3. the phone's risks: full-resolution cost with the liked density, shimmer, blind tests of savings, heat;
  4. the corner at every hour;
  5. then the zoom bands and the three problems the owner named: small plants at game size, the ground's speckle, and people lost in busy scenes.

## 2. What `PROJECT.md` asks

- **The gate:** M2 makes "the game's own scenes drawn by the engine, in portrait and landscape, which you judge as the bar for everything built on them" (`MIL-09`), and "nothing is built on the engine until you are happy with it" (`IMPLEMENTATION.md`, M2).
- **The feelings:** wonder (`VIS-07`); "a world that is always busy at every zoom" (`VIS-17`); "Beautiful, smooth and absorbing in your hand" (`VIS-14`).
- **The review:** "a contact sheet made on the phone, on one page, from fixed saved worlds", whose check is "judged by the review and then by you" (`PRE-31`). It stays as decided.
- **Lifted for this research** (round 2's brief): `PRE-01`, `PRE-20`, `PRE-21`, `PRE-22`'s fixed sizes, `PRE-27`'s "tiny blocks", the art bible (A5.2, A5.3), and `MIL-09`'s "the model kit and its textures made by code" (the kit of `PRE-46`); "Pixels that do not crawl or shimmer stay a quality goal". `PRE-22`'s Done when ("with the camera still or panning, frames change only where something moved, or by whole pixels") was written for whole-pixel steps; section 6 proposes how it reads under direction B.
- **Not lifted:** "Nothing is added for show" (`PRN-10`); Stone Age truth (`PRE-42`); readability from far away (`PRE-28`); the phone's limits (`PLT-01`, `PLT-04`); and "AI language models describe, never decide" (`PRN-06`), which this loop keeps in spirit: judges advise, the owner decides.
- **Process:** a pass rule is stated before its test first runs and "never loosened in the change that makes it pass" (`RES-09`); checks within about 20 minutes (`PRC-10`); the builder's own review and an independent one per milestone (`PRC-09`); "Only the stage reviews wait for you" (`PRC-11`); the owner's time is a named risk (`RSK-23`).

## 3. Where we stand

**Round 1** (my note and the others'):
- I proposed three judges in a fixed order (machines measure, the AI describes, the owner decides), a target card measured from the two liked camps, and GPT paintovers of engine frames as choices.
- Study 1 measured the liked look: gold, not green; a quarter dark; small accents; teal water.
- Study 4 estimated the cost; in round 2's brief's words, "Density, not drawing small, is what costs".
- Study 7 built a "look lab" and the L1 to L3 pipelines.

**The owner's input since:**
- The owner liked the GPT style ("I would like something like that"), proposed faux 3D pixel art ("the pixel is just the rendering"), and asked to "Remove the strict rules … It's more about the feeling".
- They raised GPT's allowance for this research three times, from 4 pictures a study to 32.
- Then they answered 32 picture questions in two sittings the same morning (`R/owner/ask/answers.md`). What bears on this study:
  - **the look:** direction B at 2 × 2 (answers 1, 2);
  - **content:** detailed people (3, the pair I made) and animals (14); plants, edges and light "As I liked" (6, 7);
  - **light:** true midday (9); nights A, B and C, including my night paintover (10); winter soft (11); the far views that study 1 made from approved pictures, not my repainted engine frame (12); dusk relit, not repainted (23);
  - **savings:** colour alone doesn't bring the feeling (24); half resolution "only if needed" (25);
  - **three problems to solve, not cut:** small plants lose their charm at game size (21); people in a busy autumn wood are found only "with effort" (27); the ground turns to speckle at the close camp (31).

**What pre-production learnt about judging** (`LESSONS.md`):
- The owner's eye caught the motion faults that pictures missed: outline flicker, snapping turns, zoom hand-overs.
- "verdicts on the phone lag behind the cloud's".
- Side-by-side options worked.

The old Rust renderer (git `3d62168`) counted crawl as "an art pixel whose palette index changes between two frames while the surface it shows, re-projected through the depth buffer, moved less than one art pixel": 1.8% to 9.9% of art pixels a frame in slow turns and zooms, none in a slow pan. Its record also holds a choice of the owner's: one crawl fix "cut both by over 99%, but the owner disliked its look".

**What is weak or missing now:**
1. **Rules gave free checks; the feeling does not.** A palette and 4 to 7 shades could be checked exactly. Under the feeling, only statistics, stability and the owner's eye remain. Round 1 warned of this: "The less strict the pixel art, the less of the look machines can guard".
2. **Direction B moves "steady pixels" from whole-pixel steps to texture shimmer under smooth motion.** The old crawl counter read a palette index, which no longer exists.
3. **The owner turned down every visible saving** (answers 6, 7, 8, 24, 25), and nothing yet proves a saving invisible.
4. **The liked pictures show one angle and one hour each,** mostly low sun. A target taken from them must be read over many views and hours, and must learn from the owner's picks. True midday, which the owner chose, breaks the low-sun numbers (section 4.2).
5. **The three problems the owner named need checks,** or they will return unnoticed.

## 4. What I found

### 4.1 The target card, from the twelve pictures

*Method:*
- Each picture is averaged in cells of 4 × 4 pixels, which is about 4 × 4 screen pixels on the phone, or 2 × 2 texels at the closest zoom. It is measured in OKLab.
- *Small things* counts spots 2 to 5 cells across, per 1,000 cells.
- *Flat patches* is the share of 6 × 6-cell windows of one colour.
- *Largest single colour* is the commonest colour's share.
- The art book's close camp is shown for contrast.
- Script: `card.py`.

| Picture | Median lightness | Dark (<45) | Very dark (<30) | Colour strength, median | Strong colour (>15) | Warm / cool | Brightest 5%: hue | Greens | Flat patches | Small things /1000 | Largest single colour |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Liked, from above | 57 | 23% | 3% | 6.4 | 1.2% | 62% / 11% | 77° gold | 9% | 0.1% | 43 | 1% |
| Liked, sunset | 45 | 50% | 11% | 4.4 | 0.2% | 43% / 22% | 54° orange | 6% | 0% | 41 | 1% |
| 1 Person at dawn | 46 | 47% | 13% | 4.5 | 1.1% | 51% / 20% | 79° | 5% | 0% | 40 | 1% |
| 2 Camp at night | 30 | 89% | 50% | 3.1 | 0.3% | 26% / 26% | 35° fire | 0% | 1.0% | 38 | 1% |
| 3 Winter steppe | 73 | 15% | 3% | 4.7 | 0% | 21% / 52% | 63° | 0% | 5.4% | 35 | 4% |
| 4 Lake, mist | 49 | 39% | 9% | 4.9 | 0.3% | 49% / 20% | 76° | 6% | 3.6% | 35 | 1% |
| 5 Autumn forest | 44 | 52% | 11% | 5.9 | 3.5% | 73% / 2% | 71° | 1% | 0% | 45 | 1% |
| 6 Valley | 55 | 27% | 9% | 5.5 | 0% | 60% / 12% | 72° | 4% | 0.1% | 41 | 1% |
| 7 Coast at dusk | 40 | 62% | 24% | 5.1 | 2.5% | 51% / 25% | 39° | 0% | 0.4% | 43 | 1% |
| 8 Village, copper | 61 | 22% | 2% | 7.4 | 0.2% | 66% / 6% | 72° | 12% | 0% | 44 | 2% |
| 9 Storm | 40 | 67% | 21% | 4.0 | 0.5% | 34% / 30% | 36° fire | 1% | 0.6% | 36 | 2% |
| 10 Cave | 33 | 75% | 43% | 5.6 | 0.7% | 66% / 17% | 55° | 0% | 1.8% | 29 | 5% |
| *Art book, close camp* | *78* | *8%* | *2%* | *13.6* | *0.1%* | *10% / 12%* | *119° green* | *74%* | *57%* | *11* | *60%* |

**What all twelve share** (the card's fixed goals):
1. **No flat ground:** at most about 5% flat patches (snow and calm water are the highest), against 27% to 57% in the art book.
2. **Detail as things:** 29 to 45 small things per 1,000 cells, against 9 to 21 in the art book. No single colour covers more than 5%, against 60% for the art book's meadow.
3. **Golden lights:** the brightest 5% are orange to gold (35° to 79°), never the art book's yellow-green (119°).
4. **Muted greens:** at most 12% of the picture in low sun. On grass alone the liked camp's colour strength is 9.3 at a hue of 105°, against the art book's 13.1 at 120° (`grass.py`).
5. **Strong colour only in specks:** at most 3.5% of the picture, in clusters of one or two cells (flowers, fire, beads, ochre).
6. **Texture as strong as the masses:** fine texture 6 to 10, big masses 6 to 11. In the art book the masses are 10 to 16 and the texture only 4 to 6.
7. **Warm light, shade near neutral:** on OKLab's yellow–blue axis the lightest fifth sits at about +5 to +9 by day, and the darkest fifth at −2.4 to +2.1. The art book's person zoom and night sit at −4.4, bluer.

**The water** (outlined by hand in four pictures; `water.py`): teal to blue (191° to 232°); colour strength only 4 to 6; median lightness 44 to 52, close to the land's; textured like the land (5 to 8), so the bed shows; in clear water a bright edge line 6 to 9 lighter than the water.

**What each moment adds:**
- **day:** median lightness 55 to 61, 22% to 27% dark;
- **low sun:** median 44 to 49, 39% to 52% dark;
- **dusk at the coast:** 62% dark, violet shade;
- **storm:** 67% dark, the most muted;
- **night:** 89% dark, half of it very dark, light only in specks of fire;
- **cave:** 75% dark, the fewest small things;
- **winter:** median 73, cool shade over half the picture, no strong colour.

**A caution:** GPT ignored parts of every prompt (all twelve came out landscape, with horizons and wrong scales). So these numbers describe the pictures the owner liked, not their prompts.

### 4.2 The card against the owner's 32 answers

My earlier note left this open. Would the card have warned about the pictures the owner turned down? I measured the pictures behind the 17 answers that compare scenes (`cardtest.py`, `finescale.py`). A picture counts as flagged when it leaves the card's goals or its moment's band, by more than two runs of one GPT request differ.

| Answer | Turned down | Card | What showed it |
|---|---|---|---|
| 1 | A "pixels are the rendering", C "glow and haze" | missed | the same numbers as B: a way of drawing, not a statistic |
| 6 | A half the plants | caught | small things −14%; dark 19% against 23% |
| 6 | B every plant a solid clump | caught | greens 15%; strong colour 2.4 times |
| 6 | C dense edges, D low grass as clumps | missed | wildness and airiness, not counts |
| 7 | A outlines and lit edges | missed | (the engine knows whether outlines are on) |
| 7 | B light in steps | caught | brightest colours yellow (86°), cool share doubled |
| 7 | C calmer surfaces | caught only at the texel scale | flat 8 × 8-pixel windows 7.0% against 1.2% (two runs: 0.8% to 1.0%) |
| 7 | D no corner darkening | caught | 10% dark against 23%; big contrast −26% in the close-up shown |
| 8 | A overcast | caught | light not warm (+3.3 against +9.3); greens 29% |
| 8 | B a third of the plants | caught | small things −27%; 15% dark |
| 9 | B noon kept warm | missed | the card preferred it: the owner chose truth over warmth |
| 10 | D many fires | caught | warm share 36% against 6% to 17%; strong colour 1% against at most 0.25% |
| 11 | A bright snow, dark river | caught | blue shade (−5.6 against −2.1) |
| 12 | B the engine's far frame repainted | caught | yellow-green lights (101°), colour strength 10, greens 21% |
| 23 | A the engine's dusk repainted | caught | 32% dark, against the relight's 66% |
| 24 | colour-only regrade | caught | 49% flat, 12 small things, one colour covering 34% |

**So the card caught 12 of the 18.** The six it missed were the way of drawing, the wildness of plants, outlines, and a taste for true light over warm light. Those are the owner's eye's work, or a decision made once.

**It also raised false alarms on 6 of the 17 picked pictures:**
- true midday (greens 28%);
- my night shelter (21 small things);
- the far dusk view (blue shade);
- backlight and lake mist (pale yellow lights, 83° to 85°);
- direction B itself (greens 14%).

**What the answers teach the card** (its bands, refitted):
- **true midday:** up to about 30% greens, only about 30% warm, and light only +7 warm;
- **night:** 86% to 90% dark, warm pools at most about a sixth of the frame, strong colour at most 0.25%;
- **dusk:** about two thirds dark, and flatter than the engine stand-in's dusk. The relit dusk the owner chose has a big contrast of 12.4, against 17.4 in that frame, which is only 10% dark;
- **winter:** shade neutral, not blue;
- **far views:** shade may turn blue with distance;
- **facing the sun or mist:** the brightest colours may be pale yellow.

**Two new goals for the problems the owner named:**
- **Accents on the ground at every zoom band** (answers 21 and 31; `speckle.py`, `speckle2.py`).
  - My first guess was wrong. The ground the owner called speckle is *not* noisier from one texel to the next than the accepted pictures' ground. Its texel-to-texel variation is 2.3 to 2.4, against 4.0 to 6.6.
  - What it lacks is accents. Take the colour difference between the most striking 1% of texels and their surroundings, × 100. Accepted grounds measure 23 to 30: the liked camp, direction B's picture, the close camp, the camp of thirty, true midday. The ground shrunk to the close camp's density measures 11 to 12 (on small samples, one to nine windows of 24 texels), and the art book 8.
  - Averaging a texture down mixes flowers, lit tips and dark gaps into one mush of mid-tones. That is what reads as speckle.
  - By my reading the small plants of answer 21 lose their charm the same way; I did not measure them.
- **People against their surroundings** (answer 27; study 1's `salience.py` on my marked positions).
  - In the autumn wood the owner searched "with effort". Its four people stand at the 61st to 90th percentile of the frame's contrast with surroundings, and only the 57th to 77th in colour.
  - In the camp of thirty, which the owner called "alive", the median person stands at the 93rd percentile, and the 89th in colour.

### 4.3 What GPT paintovers give, and where they mislead: eleven tests

All eleven pictures ran through the builder on the owner's plan, about a minute each (section 10).
- **The stand-in "engine frames":** crops of 336 × 504 pixels from the art book, enlarged three times.
- **How I compared:** through masks of each material, made from the frame's own flat colours, as the engine's material picture would give them. Each repaint was compared with its input, region by region, at the frame's own size (`paint.py`, `hours.py`, `abcheck.py`, `relight_cmp.py`, `converge.py`).

**What a paintover gives:**
- **The layout holds.**
  - At the close camp the outlines stayed within one pixel of the frame in 82% of places, and within two in 91%. The second run kept 58% and 69%.
  - Every person, the hearth, the rack, the tents and the ford stayed where they were.
  - At camp zoom it kept the far scale too.
- **It lands on the liked colours from words alone.** Its grass has a lightness of 63, a colour strength of 9.8 and a hue of 108°. The liked camp's grass has 59, 9.3 and 105°, and the art book's 76, 13.1 and 120°.
- **Each material gets a precise target.** For the close camp's meadow:
  - lightness from 78 down to about 63;
  - colour strength from 13.5 to about 10;
  - hue from 120° to 107°;
  - small things from 2 to 55–65 per 1,000.

  The cliff turns from violet-grey to warm pale grey, and the river from blue to teal with its bed showing.
- **Region averages are stable; pixels are not.**
  - Between two runs of one request, a region's average colour moved only 0.002 to 0.016 on the meadow, cliff and tents (0.03 on the river). On the meadow, cliff and river the paintover moved 0.05 to 0.16 away from its input.
  - The tufts' positions did not correlate between the runs (0.03).
  - So a paintover is a reliable target for each material's colour, spread and density, and pure noise for single pixels.
- **It works at night.** It darkened the art book's night from 27% dark to 86%, near the liked night's 89%, and drew firelight in smaller, warmer pools. The owner picked it among the good nights (answer 10 C).

**Where it misleads:**
- **It re-invents details:**
  - reeds became golden stalks that read as ripe grain;
  - the muddy bank vanished under grass;
  - a block figure gained what reads as a metal blade;
  - in the dusk repaint, people wear sleeveless white vests and loincloth-like briefs, though the prompt said "no fur bikinis and no loincloths".

  None of this may pass into the game (`PRE-42`, `PRN-10`).
- **Edits change more than they are asked to.**
  - Asked to change only the six people (the detailed-people picture the owner picked, answer 3 B), GPT left 97% of the rest unchanged at viewing scale. But it redrew the fine grain of 21% of the pixels outside the people, and left the dog a wooden block.
  - Asked to halve the small things, it cut them by 60% to 70%, but also made the grass lighter and more colourful (0.015 to 0.029, above the 0.016 noise).
- **It cannot add what the frame lacks.** The art book's flat far view, repainted, stayed flat, yellow-green and empty. The owner turned it down (answer 12) and the card flags it. The far views the owner picked were GPT's own pictures made from the liked picture. Life and composition at a distance must come from the world itself.
- **Repainted every round, the target drifts** (`converge-v2.png`). I gave GPT its own first paintover, as if the engine had reached it, with the same prompt.
  - It kept the layout exactly (every place within one pixel) and much of the detail (correlation 0.50 on the meadow, 0.76 on the cliff, against 0.03 between independent runs).
  - But it pushed the meadow darker (lightness 62–64 to 59–60), a shift of 0.025 to 0.037, against 0.002 to 0.016 between two runs of one request.
  - Strong-colour specks rose from 0.15% to 0.98%. Small things rose too, but no more than a plain second run does.

  So the loop anchors to the one target the owner approved and never chases a fresh repaint.
- **Crude frames give targets for big surfaces only** (`crude-first-frame.png`, the same frame flattened to 10 flat colours).
  - Meadow, path and floor landed within the noise of the finished frame's paintover (0.004 to 0.015).
  - Tents, cliff, river and shelter were 0.025 to 0.058 off.
  - Two small people became a stick and a shrub, and the man in the river moved to where the prompt's words put him. The cruder the frame, the more the words steer the content.
- **Relighting keeps everything but the light's direction** (`relight-dusk.png`).
  - It kept every tuft: detail correlation 0.68 on the meadow and 0.87 on the cliff, with no layout shift.
  - But the cliff face turned toward the low sun got darker, 0.80 of its noon brightness, where the engine brightens it 2.87 times.
  - The owner still chose it over the repainted engine dusk (answer 23); the answers file describes it as "the same content, softer and flatter dusk light". That question also pointed out the vests and loincloths in the other picture, so answer 23 cannot separate the two reasons.
  - Across all the answers, nine pictures were relit or re-seasoned versions of liked-style pictures: true midday, the warm noon, two nights, two winters, this dusk, the backlight and the overcast. Six kept the feeling. The warm noon, the bright winter and the overcast were turned down for the light they showed, not for being relit.
- **It never keeps a grid, and the tool is narrow.**
  - GPT's scene pictures of round 2 have 196,000 to 993,000 colours (the owner's twelve 227,000 to 816,000), though my prompts asked for "about 32 colours".
  - In all of them but one, 0.05% to 2% of side-by-side pixels are equal, against 96% in the engine stand-in. The exception, the colour-only regrade (16%), kept the engine frame's flat areas.
  - The Codex image skill's built-in edit mode "is for images already visible in the conversation context". Masks are "explicit CLI-only parameters", and the command-line tool requires an `OPENAI_API_KEY` the owner's plan does not give. The builder asks for 1024 × 1536, so paintovers work on 2:3 crops of the phone's 1080 × 2404 frame.

**So:** a paintover or relight is a precise, repeatable target for *what each material is like* and for the frame's mood at that hour. It is no target for single pixels, for content, or for the light's direction. The engine supplies light, shapes and content; GPT supplies the look of materials and moments. Its pixels are measured, never copied into the game.

### 4.4 What can still be checked automatically under direction B

| What | How | Tolerance, and why |
|---|---|---|
| **The card** | Each statistic of 4.1 on fixed views: 8 turns × 4 hours × each zoom band. Each material is measured through the engine's material picture against its approved target | The moment's band, refitted after every answer. Each material within about 0.03 of its target, twice the noise between runs. Shown green, amber or red, never as a gate: it is right about two times in three (4.2) |
| **Ground accents at every zoom band** | The colour difference of the most striking 1% of ground texels from their surroundings | At least about 20: accepted grounds 23 to 30, the speckled one 11 to 12. Set from the owner's verdicts on the phone |
| **Readability** (`PRE-28`) | Each person's contrast with their surroundings in lightness and colour, as a percentile of the frame. Positions come from the engine's object picture, so nothing is guessed | First line: the top tenth (90th percentile) in lightness or colour. The people the owner found "with effort" sat at the 57th to 77th in colour. Tuned on the owner's yes or no over crops |
| **Still frames** | Time frozen, two frames | Identical |
| **Texture shimmer in pans, turns and zooms** | The frame's error against a many-sample picture of the same view, compared from frame to frame after following the motion (engine depth and motion). Pixels whose error changes by more than 0.03 count as flicker | At most 2% on every scripted path, then never more than a tenth higher without a note, as the old renderer did. My simulation (`shimmerB.py`, `shimmerB2.py`) is below |
| **Savings invisible** | The saving's frame against the full one, same view: FLIP at the phone's 30 cm and 390 pixels an inch (`invisible.py`); shimmer as above; then the owner's blind test | Machine line: under a tenth of the difference the owner saw at half resolution (5.3% of pixels above FLIP 0.2). Owner line: ten random pairs, "which is sharper?" Eight or more right means it shows; guessing does that about 5% of the time (my arithmetic) |
| **Regressions** | Golden pictures in the cloud: software Vulkan driver, single thread, time frozen | Exact for code-only changes on a pinned driver. A change of look updates them with before-and-after pairs; the set the owner approved changes only with their OK |
| **Phone against cloud** | At first start the phone draws a few golden views and reports FLIP's share of pixels above 0.2 | A margin set from the first alpha; a jump is a driver problem to chase |

**The shimmer simulation.** A flat ground wearing the liked picture as a pixel-art texture, panned at 0.3 pixels a frame or zoomed 0.5% a frame. The flicker column is the share of pixels; sharpness is the texel-scale detail kept, against the many-sample picture.

| Way of reading the texture | Flicker, closest and mid zoom | Sharpness |
|---|---|---|
| Nearest texel (classic pixel crawl) | 17% to 28% | 1.3 to 1.4 (jagged) |
| Bilinear (the usual smooth filter) | 7% to 13% (sharpness pulses with the phase) | 0.64 to 0.72 (soft) |
| **"Smooth pixel":** flat texels, edges blended over one screen pixel (study 2's sources) | **0.4% to 1.1%** | **0.96 to 0.99** |
| Smooth pixel at half resolution, enlarged | 9% to 16% | 0.40 to 0.43 |

- **Zoomed out without texture levels** made for the zoom (texels at 0.6 of a screen pixel), every filter flickers, 10% to 43%. With a level made for it (about 2.4 pixels a texel) the smooth-pixel filter flickers 0.1%.
- A calmer texture flickers less under every filter. But the owner turned calmer surfaces down (answer 7 C), so the filter must do the work, not the texture.
- **Why follow the motion:** a still comparison is useless for motion. A half-pixel shift of the same picture alone puts 12.5% of pixels above FLIP 0.2.

**What only the owner's eye can judge:** beauty, wonder and life; whether density reads or turns to noise at arm's length; how motion feels (shimmer, the "ease", sway); colour on their own panel; and every choice of taste. The checks catch named faults and drift. They never approve a look.

### 4.5 How far to trust an AI judge

- **Pairs, not sets.** In the May 2026 Visual Aesthetic Benchmark, "the best model falls from 47.3% on two-image tasks to 6.7% on four-image tasks, while human experts decline from 87.1% to 43.6%".
  - The strongest system "identifies both the best and the worst image correctly across three random permutations of the candidate order in only 26.5% of tasks, far below the 68.9% achieved by human experts". Other systems scored lower, down to 15.5%.
  - "Illustration is the most difficult domain".
  - MLLM-as-a-Judge (2024) found "remarkable human-like discernment in Pair Comparison" but "a significant divergence from human preferences in Scoring Evaluation and Batch Ranking".
- **Order bias, and its cure.** In TASTE (2026), open-weight judges gave the same positional answer after the two pictures were swapped in 44% to 87% of pairs. "Restricting to the order-consistent fraction of each model’s verdicts, agreement with the designer majority rises to 0.55−0.66". So: ask twice, swap the order, keep only answers that agree.
- **Taste is personal.** Among professional designers "substantial levels of disagreement exist (Krippendorff's alpha = 0.25 for binary preferences)", and "personalized models consistently outperform aggregated baseline models in predicting individual designers' preferences, even when using 20 times fewer examples" (DesignPref, 2025). Reward models aimed at "average" appeal "fail to capture the inherent subjectivity of aesthetic judgment" (PAMELA, 2026).
- **Not motion, not regressions.** Judges "suffer a catastrophic collapse when required to perform pairwise discrimination of temporal order" (August 2026). For visual regression testing, the best-performing model reached an accuracy of 45.2%, and others as little as 24.0%. Glitches in still pictures are easier (up to 82.8%).
- **A second family helps.** A panel from different model families "exhibits less intra-model bias due to its composition of disjoint model families" (PoLL, 2024). GPT, through the builder, can give that second opinion on pairs at a milestone's end.
- **The exam is ready.** The owner's 32 answers, with their pictures, are a test of the owner's own taste. Before its advice counts, the judge answers them blind, from the pictures alone, each pair twice with the order swapped. Its agreement goes into the taste log. I cannot be that test myself: I saw the answers.
- **My own reading** caught the grain-like reeds, the lost bank and the vests by enlarging crops. I confirmed the metal blade only after the builder pointed it out. Content faults suit a checklist ("any metal? any later crops or animals? clothing as `PRE-42`?") better than a beauty score.

### 4.6 The owner's choices: the method that worked, galleries, the taste log, how often

- **Quick choices work.** The owner answered 32 picture questions in two sittings the same morning, in batches of four. They picked several options where a question allowed it (the answers file's multiple-choice questions), and typed their own answer three times ("As I liked" twice; "Have multiple types that they can choose from, all of these are nice").
  - Forced choice between two "results in the smallest measurement variance and thus produces the most accurate results. This method is also the most time-efficient" (Mantiuk and others, 2012).
  - Experts agree more when choosing directly: "Comparative ranking yields 42 percentage points higher inter-annotator agreement on best-image selection than score-derived rankings" (the 2026 benchmark).
  - Choice overload grows with "higher levels of decision task difficulty, greater choice set complexity, higher preference uncertainty" (Chernev and others, 2015). So: two options when the question is hard, at most four when they differ in one clear way.
- **Three lessons from the answers:**
  - **The owner turns down visible trade-offs** (answers 6, 7, 8, 24, 25). Don't offer cheaper-but-worse pictures: prove a saving invisible (4.4) or don't make it.
  - **A note naming one option's fault may steer the answer** (answer 23). Fix known faults before asking.
  - **Show things complete and in context.** On the rock surface without its layers the owner answered "can't judge yet" (answer 32).
- **Galleries for settings with many values.**
  - Sequential Gallery lets "users sequentially select the best option from the options provided by the interface", with Bayesian optimisation used "to keep necessary queries to users as few as possible" (2020).
  - Design Galleries (1997) instead show "the broadest selection, automatically generated and organized, of perceptually different graphics or animations".
  - The owner's history fits both: narrow variants won the "sharp" look, but the leap they loved came from a different direction. So a gallery first spreads wide, then narrows. It is made by the engine or by code, which change exactly one setting; GPT's density edit changed colour as well.
- **A taste log.**
  - ViPer (2024) invites people "to comment on a small selection of images, explaining why they like or dislike each" and infers "structured liked and disliked visual attributes". A 2026 study gathered pairwise judgments "from 767 users, each providing an average of 70 comparisons".
  - The owner's log already holds about 50 choices: pre-production's (the art book as a start, then preliminary; the "sharp" look and "clear" water; outline way C and "ease"; fire shadows and lit smoke; P3's windbreak, roofs, clothes and 5° turns; P8's crisp glade; the dislike of the Fade crawl fix) and round 2's (the GPT style, "Show me options", faux 3D pixel art, "the feeling", and the 32 answers).
- **How often.** "We find that scheduling questions on phone unlock yields a higher response rate and accuracy" (van Berkel and others, 2018). Here:
  - questions come with the alpha the owner opens anyway, in batches of four, never as interruptions;
  - a sitting of 16 worked twice;
  - only stage reviews block work (`PRC-11`).

## 5. The options

### 5.1 Where the targets come from

| Option | Closeness to the feeling | Cost on the phone | Effort for the AI builder | Risk | Fit with the rules |
|---|---|---|---|---|---|
| T1 The twelve pictures' statistics only | Mood and balance; nothing per material | None | Low | Tuned to low sun: flagged the owner's true midday | Fits |
| T2 Repaint the engine's frame every round | Precise per material, if the frame holds the content | None | About a minute a picture | Drifts darker; invents details; the far and dusk repaints were turned down | Needs a truth check each time |
| **T3 One approved picture per place, relit for hours and seasons** | 6 of 9 relit versions kept the feeling | None | Low: one approval, then edits | Wrong light direction; small content drift with each edit | Fits: never shipped (`PRN-10`) |
| T4 GPT's own scenes made from approved pictures | The far views the owner picked (answer 12 A, C) | None | Low | Scale and period errors | Fits if checked |

T1 alone flags truth as a fault (4.2). T2 gives the best per-material numbers, but chasing it each round drifts and invents. T3 keeps the content the owner approved, and the engine's physics supplies the light's direction. T4 serves where the engine cannot yet show life: far views and new places. Recommended: T3, with its anchor made once by T2 (where the frame holds the right content) or by T4, and T1's card as the alarm over everything.

### 5.2 How a saving is decided

| Option | Closeness to the feeling | Cost on the phone | Effort | Risk | Fit |
|---|---|---|---|---|---|
| V1 Show the owner the cheaper picture and ask | Whatever the owner accepts | As accepted | Low | The owner turned down every one | Fits `PRC-07`, wastes their time |
| **V2 Machine check, then the owner's blind test on the phone** | Kept, by construction | Saves only what doesn't show | Medium | Blind tests cost a minute each | Fits; a new rule for the owner's OK |
| V3 The builder decides by measures alone | Slow erosion | The most saved | Low | Visible losses add up | Weak: the owner judges the look (`MIL-09`) |

### 5.3 The order to build M2

| Option | The feeling seen early | Phone risks learnt | Effort | Risk | Fit |
|---|---|---|---|---|---|
| A One corner at full quality, then widen | High, in the corner | Late for breadth | Medium | Tuned for one place and angle | Fits `PRN-14` |
| B Every system basic first, then polish | Rough for weeks | Early | High rework | The owner sees the look late | The gate comes late |
| **C The card, checks and judge exam; first light; phone risks; corner at all hours; widen** | Early (first light) | Early | Medium | First light waits days for the harness | Fits best |
| D Build to the approved pictures | Depends on the targets | None at first | High | A still promises what the engine can't | Partly done |
| E A ladder: swatches, one model, the corner | Slow | Medium | Medium | Parts can't be judged alone (answer 32) | Slow |

## 6. What we'd recommend

**The loop, at every step that changes the look:**
1. **Draw:** one Godot run draws the fixed views at the phone's 1080 × 2404. It writes the colour, object and material pictures and lossless frames of scripted pans, turns and zooms, and a many-sample picture of a few small patches.
2. **Check:** the checks of 4.4, each pass line fixed before its first run (`RES-09`); the card's alarms beside each view.
3. **Look:** changed views beside their last approved versions, whole and in enlarged crops, with a lettered grid so a fault is "C7".
4. **Judge:** for a step that changes the look on purpose, a fresh subagent sees the pictures, the targets and a checklist (yes or no per Done when and per truth question, with a grid cell), and "which is closer to the target" per pair, asked twice with the order swapped. It reports faults, never approval.
5. **Show** the owner what changed, in pairs, with the alpha.
6. **Ask** only when the look has an open question.

**Targets:**
- For each place, one anchor picture approved by the owner: a paintover of an engine frame that holds the right content, or GPT's own scene from approved pictures.
- Its other hours and seasons come from relights.
- From each target the builder takes each material's colour, spread and density through the engine's masks, and the moment's numbers for the card. Light direction comes from the engine's physics; for the dusk's softness, the relight's flatter balance is the target to tune toward.
- Every GPT picture is checked by code (layout, drift) and against a truth checklist before the owner sees it. None is ever shipped or traced.

**Proposals for the owner** (each needs their OK, `PRC-07`):

1. *Proposal, `PROJECT.md`, `PRE-22`'s Done when (lifted).* Study 1 proposes the look's own wording (texel sizes by zoom). For the check, in place of "with the camera still or panning, frames change only where something moved, or by whole pixels":
   > **Done when:** with the camera still, frames change only where something moved; while it pans, turns or zooms smoothly, no more than 2 in 100 pixels flicker beyond what the movement itself explains, and the owner sees no shimmer in the review's clips on the phone.
2. *Proposal, `PROJECT.md`, a new item in 11.1 (its ID to be given), "Savings don't show":*
   > A simplification made to fit the phone (`PLT-04`) is kept only where the owner cannot tell it from the full picture in a blind test on the phone.
   > - **Done when:** every saving in the build has passed its blind test, recorded with its pictures.
3. *Proposal, `ARCHITECTURE.md` A5:* the target card replaces the art bible's rules (A5.2) as the look's guard. It holds fixed goals and bands per moment, refitted from the taste log, and shows alarms, never gates. It adds two goals for the owner's problems: ground accents at every zoom band, and people's contrast with their surroundings. Texture lookups are never at reduced resolution, and texture levels are made for each zoom band.
4. *Proposal, `ARCHITECTURE.md` A4.4:* golden pictures as colour pictures compared exactly for code-only changes and with FLIP otherwise. The phone draws a few at first start and reports its gap. Approval works in tiers: working goldens update with before-and-after pairs, and the set the owner approved changes only with their OK.
5. *Proposal, `ARCHITECTURE.md` and `IMPLEMENTATION.md`:* targets by anchor and relight, as above. Each target is approved once and never re-repainted to chase; GPT pictures are never shipped, never traced, and always checked for truth.
6. *Proposal, `IMPLEMENTATION.md` M2:* the order C of 5.3:
   1. the card, the checks and the judge's exam;
   2. first light: the camp under the cliff by the river, at the closest zoom, in direction B, on the phone;
   3. phone risks: full-resolution cost with the liked density, shimmer, blind tests of savings, a 20-minute heat run;
   4. the corner at dawn, true midday, dusk, night and winter, judged against its relit targets;
   5. widen to the zoom bands, landscape, and the owner's three problems.
7. *Proposal, process:* questions come with the alpha, in batches of four, free of known faults, never offering visible trade-offs. The taste log lives under `art/reviews/` with all the owner's choices so far, and the AI judge sits its exam on the 32 answers before its advice counts.

`PRE-31` stays as decided; the card's alarms and the shimmer and saving results simply appear on its contact sheet.

## 7. Questions only the phone can answer

1. **Does texture shimmer show on the phone at the level the cloud measures?** Paired clips of scripted pans, turns and zooms: smooth-pixel against nearest, and against bilinear. The owner says which shimmers, and the pass line moves to match.
2. **Which savings are invisible?** Each saving as ten random pairs on the phone, stills and clips.
3. **Does the liked density at full resolution read at arm's length, or turn to noise?** The corner at the liked density, judged on the phone.
4. **Does the close camp's ground read as meadow?** Zoom-band textures at three accent levels; the owner's yes or no sets the accent line.
5. **Can the owner find people at a glance in busy scenes?** Yes or no on ten close-camp crops (autumn, the camp of thirty) sets the readability line.
6. **How far is the phone's drawing from the cloud's?** The first-start self-check's FLIP numbers.
7. **Are dusk and night too dark on the owner's screen?** The relit dusk is two thirds dark. Reviews note the colour mode and brightness.
8. **Can the owner answer an options page one-handed in a minute?** Timed by the page.

## 8. Risks

| Risk | How to retire it early |
|---|---|
| The card is taken for taste | It shows alarms only. It is refitted after every answer, and its hit and false-alarm rates are reported (12 of 18; 6 of 17) |
| Targets drift with repeated repaints | Anchor to the approved picture; a new target only with the owner |
| GPT targets bring later things or wrong dress | A truth checklist before the owner sees any picture; fix or drop |
| Relit targets give the wrong light direction | The engine's physics supplies direction; relights give per-material colour and the moment's balance |
| A saving shows | Machine line, then the owner's blind test; texture lookups never at reduced resolution |
| Shimmer returns through a cheap filter or missing texture levels | The shimmer check on every scripted path in every build |
| The checks outgrow 20 minutes (`PRC-10`) | Many-sample pictures only for small patches; a few short paths; caching; the full set in the background |
| The AI judge is trusted too far | Its exam on the 32 answers; pairs only, order swapped |
| The owner's time, and verdicts that lag (`RSK-23`) | Batches of four with the alpha; one blocking review a milestone |
| A question's wording steers the answer | Neutral wording; known faults fixed first |

## 9. For other studies

- **Study 1 (the look):**
  - The card test of 4.2: true midday holds up to about 30% greens. The chosen dusk is two thirds dark and flatter than the engine stand-in's. Night's warm pools stay under about a sixth of the frame. Far views may take blue shade.
  - The salience numbers for the autumn wood (people at the 57th to 77th percentile in colour) and the camp of thirty (median 93rd).
- **Study 2 (how to draw it):**
  - The shimmer simulation (`shimmerB.py`, `shimmerB2.py`): the smooth-pixel filter is steady (0.4% to 1.1%) and sharp (0.96 to 0.99). Nearest flickers 17% to 28%. Bilinear is soft (0.64 to 0.72) and pulses (7% to 13%).
  - Texture levels made for each zoom band are needed for steadiness as well as for the look.
  - The owner's dusk is flatter than the engine stand-in's: by my reading, more sky light against the sun.
- **Study 3 (the engine):** the checks need test hooks:
  - object and material pictures;
  - depth and motion for following the motion;
  - a many-sample mode for small patches;
  - scripted camera paths;
  - frozen time.
- **Study 4 (the phone):**
  - Half resolution is visible in stills (5.3% of pixels above FLIP 0.2 at 30 cm; 14.5% at 20 cm).
  - Applied to texture lookups, it also flickers 9% to 16%. Keep half resolution to smooth terms, and put every saving through the blind test.
- **Study 5 (content):** small plants and ground cover need accents that survive at game size. Shrinking by averaging drops accents from 23–30 to 11–12, which reads as speckle, so these must be painted or generated for each zoom band.
- **Study 7 (the pipeline):**
  - The look lab runs the card, accent, readability, shimmer and saving checks.
  - GPT's seamless ground repaint was the one surface the owner did not pick (answer 15 B).
  - Each GPT picture takes about a minute, so targets cost the owner no time.

## 10. Pictures from GPT

Eleven of my 32 were used. All are in `R/work2/6/gpt/`, beside their requests (`.txt`) and stand-in inputs (`in-*.png`). I asked for no more: the owner's answers settled the open picture questions, and 21 remain unused.

| Picture | What it asked | What it showed |
|---|---|---|
| `paintover-closecamp.png` | The art book's close camp at noon, repainted in the liked style | Layout kept (82% within one pixel); the liked colours; precise targets for each material; grain-like reeds, a lost bank, a metal blade |
| `paintover-closecamp-rerun.png` | The same request again | Region colours stable (0.002 to 0.016); pixels unrelated (0.03) |
| `paintover-person.png` | The person zoom, block figures kept | Figures loose (median shift 1.7 pixels); the start of the people pair |
| `ab-figures-detailed.png` | Only the people made detailed | 97% of the rest unchanged, but fine grain redrawn on 21%; the dog left a block. The owner picked it (answer 3 B) |
| `ab-density-half.png` | Half the small things on open ground | Cut 60% to 70%, but grass lighter and more colourful too |
| `paintover-closecamp-dusk.png` | The engine's dusk frame repainted | Kept the low sun, cut its contrast; added vests and loincloths. Turned down (answer 23 A) |
| `paintover-night.png` | The art book's rock shelter at night | 86% dark, small warm pools. Among the owner's good nights (answer 10 C) |
| `paintover-campzoom.png` | The art book's camp zoom | Kept the scale but stayed flat, yellow-green and empty. Turned down (answer 12 B) |
| `relight-dusk.png` | The noon paintover relit to dusk | Every tuft kept; light direction wrong; two thirds dark and flatter. Picked (answer 23 B) |
| `crude-first-frame.png` | A 10-colour frame repainted | Big surfaces on target; small things invented from the words |
| `converge-v2.png` | The first paintover repainted, as if reached | Layout exact, but drift toward darker, with more strong specks |

## 11. Sources

*Under each source, the quotes used, exactly as on the page.*

**Internal**
- `PROJECT.md`: `MIL-09`, `VIS-07`, `VIS-14`, `VIS-17`, `PRE-22`, `PRE-27`, `PRE-28`, `PRE-31`, `PRE-42`, `PRN-06`, `PRN-10`, `RES-09`, `PRC-07`, `PRC-09`, `PRC-10`, `PRC-11`, `RSK-23`, quoted in sections 2 and 6.
  - "the game's own scenes drawn by the engine, in portrait and landscape, which you judge as the bar for everything built on them"
  - "the model kit and its textures made by code" (`MIL-09`)
  - "tiny blocks" (`PRE-27`)
  - "a world that is always busy at every zoom"
  - "Beautiful, smooth and absorbing in your hand."
  - "a contact sheet made on the phone, on one page, from fixed saved worlds"
  - "judged by the review and then by you"
  - "with the camera still or panning, frames change only where something moved, or by whole pixels."
  - "Nothing is added for show."
  - "AI language models describe, never decide"
  - "A pass rule is never loosened in the change that makes it pass."
  - "Only the stage reviews wait for you"
- `IMPLEMENTATION.md`, M2: "nothing is built on the engine until you are happy with it"
- `LESSONS.md`: "verdicts on the phone lag behind the cloud's"
- Git `3d62168:ARCHITECTURE.md`, read to learn, not to copy:
  - "an art pixel whose palette index changes between two frames while the surface it shows, re-projected through the depth buffer, moved less than one art pixel"
  - "cut both by over 99%, but the owner disliked its look"
- Round 2's brief, `R/BRIEF2.md`: "Pixels that do not crawl or shimmer stay a quality goal"; its summary of study 4, "Density, not drawing small, is what costs"; the owner's words "I would like something like that", "the pixel is just the rendering", "Remove the strict rules … It's more about the feeling".
- My round 1 note, `R/round1/notes/6-loop.md`: "The less strict the pixel art, the less of the look machines can guard"
- The owner's answers: `R/owner/ask/answers.md` ("As I liked"; "Have multiple types that they can choose from, all of these are nice"; "can't judge yet" and "the same content, softer and flatter dusk light" are the builder's record of answers 32 and 23).
- The Codex image skill installed on the builder's machine, `/root/.codex/skills/.system/imagegen/SKILL.md`:
  - "Built-in edit mode is for images already visible in the conversation context, such as attached images or images generated earlier in the thread."
  - "If a local file still needs direct file-path control, masks, or other explicit CLI-only parameters, use the explicit CLI fallback only when the user asks for it."
  - "Requires `OPENAI_API_KEY`."
- My scripts and their outputs, scratch only: `R/work2/6/` (`card.py`, `cardtest.py`, `finescale.py`, `speckle.py`, `speckle2.py`, `shimmerB.py`, `shimmerB2.py`, `invisible.py`, `converge.py`, `hours.py`, `paint.py`, `abcheck.py`, `relight_cmp.py`, `grass.py`, `water.py`, `sal-autumn.json` with study 1's `R/work2/1/salience.py`). FLIP was run with NVIDIA's `flip-evaluator` 1.7 from PyPI.

**How well models judge**
- Feng and others, Visual Aesthetic Benchmark, 12 May 2026: https://arxiv.org/abs/2605.12684 and https://arxiv.org/html/2605.12684v1
  - "Under TB-1 pass^3, the best model falls from 47.3% on two-image tasks to 6.7% on four-image tasks, while human experts decline from 87.1% to 43.6%."
  - "we find that the strongest system identifies both the best and the worst image correctly across three random permutations of the candidate order in only 26.5% of tasks, far below the 68.9% achieved by human experts."
  - the paper names the strongest system, which reaches 26.5% on TB-1 pass^3 (its name left out here, as for every model)
  - the paper finds one maker's newer models scoring lower than its older ones, from 21.8% to 15.5%
  - "Illustration is the most difficult domain"
  - "Comparative ranking yields 42 percentage points higher inter-annotator agreement on best-image selection than score-derived rankings"
- Chen and others, MLLM-as-a-Judge, 2024: https://arxiv.org/abs/2402.04788
  - "while MLLMs demonstrate remarkable human-like discernment in Pair Comparison, there is a significant divergence from human preferences in Scoring Evaluation and Batch Ranking."
- Zhu and others, TASTE, arXiv 2605.20731 (version 3, 21 September 2026): https://arxiv.org/abs/2605.20731 (read from the saved PDF in `R/work2/6/web/`)
  - "Restricting to the order-consistent fraction of each model’s verdicts, agreement with the designer majority rises to 0.55−0.66."
  - Its position-bias rate, the share of pairs whose verdict stays the same when the two pictures are swapped (paraphrased), "ranges from 0.44 (Gemma-27B) to 0.87 (Kimi-VL-A3B) across the slate"
- Peng, Bigham and Wu, DesignPref, 25 November 2025: https://arxiv.org/abs/2511.20513
  - "We found that among trained designers, substantial levels of disagreement exist (Krippendorff's alpha = 0.25 for binary preferences)."
  - "Our results show that personalized models consistently outperform aggregated baseline models in predicting individual designers' preferences, even when using 20 times fewer examples."
- PAMELA, "Personalizing Text-to-Image Generation to Individual Taste", 8 April 2026: https://arxiv.org/abs/2604.07427
  - "While existing reward models optimize for "average" human appeal, they fail to capture the inherent subjectivity of aesthetic judgment."
- Ianaro and others, Order Matters, 11 August 2026: https://arxiv.org/abs/2608.10908
  - "while models may appear competent in isolated pointwise scoring, they suffer a catastrophic collapse when required to perform pairwise discrimination of temporal order."
- Taesiri and others, VideoGameQA-Bench, NeurIPS 2025: https://arxiv.org/abs/2505.15952 and https://arxiv.org/html/2505.15952v2 (the best judge's 24.0% from its results table)
  - the paper finds that visual regression testing with such models does not yet perform well, the best-performing one reaching an accuracy of 45.2%
  - "Frontier VLMs show good performance on the glitch detection task using images (up to 82.8%)"
- Verga and others, PoLL, 2024: https://arxiv.org/abs/2404.18796
  - "we find that using a PoLL composed of a larger number of smaller models outperforms a single large judge, exhibits less intra-model bias due to its composition of disjoint model families, and does so while being over seven times less expensive."

**The owner's choices**
- Mantiuk, Tomaszewska and Mantiuk, 2012 (old but standard): https://www.cl.cam.ac.uk/~rkm38/pdfs/mantiuk12cfms.pdf
  - "We conclude that the forced-choice pairwise comparison method results in the smallest measurement variance and thus produces the most accurate results. This method is also the most time-efficient, assuming a moderate number of compared conditions."
- Chernev, Böckenholt and Goodman, Journal of Consumer Psychology, 2015: https://www.kellogg.northwestern.edu/faculty/research/detail/2015/when-product-assortment-leads-to-choice-overload-a-conceptual
  - "higher levels of decision task difficulty, greater choice set complexity, higher preference uncertainty, and a more prominent, effort-minimizing goal facilitate choice overload."
- Koyama, Sato and Goto, Sequential Gallery, SIGGRAPH 2020: https://arxiv.org/abs/2005.04107
  - "This method, called sequential plane search, is based on Bayesian optimization to keep necessary queries to users as few as possible."
  - "We call this interactive framework Sequential Gallery since users sequentially select the best option from the options provided by the interface."
- Marks and others, Design Galleries, SIGGRAPH 1997 (old): https://merl.com/publications/TR97-14
  - "Design Gallery (DG) interfaces present the user with the broadest selection, automatically generated and organized, of perceptually different graphics or animations that can be produced by varying a given input-parameter vector."
- Salehi and others, ViPer, 2024: https://arxiv.org/abs/2407.17365
  - "by inviting them to comment on a small selection of images, explaining why they like or dislike each"
  - "we infer a user's structured liked and disliked visual attributes"
- Kim, Yoo and Kim, "Learning Personalized Photographic Style from Pairwise User Preferences", CVPR 2026: https://cvpr.thecvf.com/virtual/2026/poster/38681
  - "First, we introduce PPSD, a dataset containing pairwise preference judgments from 767 users, each providing an average of 70 comparisons."
- van Berkel and others, "Effect of experience sampling schedules on response rate and recall accuracy of objective self-reports", 2018: https://oulurepo.oulu.fi/handle/10024/27618
  - "We find that scheduling questions on phone unlock yields a higher response rate and accuracy."

**Checks**
- NVIDIA developer blog, FLIP, 2020: https://developer.nvidia.com/blog/flip-a-difference-evaluator-for-alternating-images (code: https://github.com/NVlabs/flip)
  - "The algorithm produces a map that approximates the difference perceived by humans when alternating between two images."
  - "It is built on principles of human perception and incorporates dependencies on viewing distance and monitor pixel size."
