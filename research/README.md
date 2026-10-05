# Research: how others build each part, whether Godot can do it, and what Kindling takes

Pre-production research, done the way a studio does it before writing the technical design (research 00), bottom up.
Each note gives:
- what `PROJECT.md` asks;
- how other games and researchers do it;
- whether Godot and your phone can do it;
- what Kindling takes;
- its sources.

Every quotation was checked against its page: notes 11 to 17 when written, notes 00 to 10 on 4 October 2026.
Where a site refused the checking tool, the quote was checked against a search engine's copy of the page; quotes that could not be found were reworded or removed.

## The notes

| Note | The decision in one line |
|---|---|
| [00 How teams work](00-process.md) | Pre-production ends with proof: the design, research, art bible, technical design, a prototype for each risk and a vertical slice; then production bottom up, detailing only the next milestone. |
| [01 The engine](01-engine.md) | Godot stays, unchanged: the Mobile renderer, a separate C++ simulation, block figures as MultiMesh parts, our own level of detail and pathfinding. |
| [02 The phone](02-phone.md) | Vulkan only, no compute shaders that read textures, budgets measured on your phone at held speed, time slowed before the phone overheats, 60 Hz. |
| [03 The simulation core](03-core.md) | A C++ library: EnTT entities, content as data, fixed ticks with timers, keyed chance, the same bits on phone and cloud, safe saves. |
| [04 Drawing 3D as pixel art](04-rendering.md) | Low resolution with nearest scaling, outlines from depth with normals rebuilt from it, our own light, built for the Mobile renderer. |
| [05 The art bible](05-art.md) | Eleven rules: your reference look wins, purple shade, hue-shifted ramps, soft nature and crisp made things, coloured outlines, Stone Age props from archaeology. |
| [06 Making the world](06-worldgen.md) | A generator in the order of causes, tuned for the look, many candidates scored and the best three offered, all deterministic. |
| [07 From a person to the globe](07-big-world.md) | Our own level of detail on Godot's servers, a moving origin, impostors and dithered fades, a hillshaded map morphing into a globe. |
| [08 Plants and animals](08-life.md) | Species as data, plants by ecology, animal numbers from Damuth's law, herds by need zones and the green wave, individuals near people only. |
| [09 Bodies and lives](09-bodies.md) | Needs that run down, energy by real numbers, wounds by layer, illness as a race, births by biology, all checked against foragers' real numbers. |
| [10 Minds](10-minds.md) | Utility choice with kept reasons, a small planner, appraised feelings, per-person knowledge, opinions that move in talk, layered pathfinding. |
| [11 Crafts and discovery](11-crafts.md) | Materials carry values, blueprints match properties never names, every reality rule backed by an experiment, discovery belongs to people. |
| [12 Culture](12-culture.md) | A naming language from the seed, customs from real cases, rites from coincidences, societies checked against foragers' bands, killings and gatherings. |
| [13 Story and the writer](13-story.md) | A director with Left 4 Dead's rhythm but no power over events, story-sifting recognisers, pattern sentences, Gemini Nano rewording under a strict check. |
| [14 The interface](14-interface.md) | The world first; one column of panels, at the bottom in portrait and beside the world in landscape; 48 dp targets; integer-scaled pixel text. |
| [15 Sound](15-sound.md) | Layered ambience from what is there, base sounds made by code, Godot's 3D audio for distance and caves, the murmur after Animalese. |
| [16 Testing](16-testing.md) | doctest and property tests in C++, gdUnit4 for Godot, Movie Maker mode for repeatable pictures and reels, Perfetto and AGI on the phone. |
| [17 Models, textures, animation](17-assets.md) | The model kit as code and data, MultiMesh copies with per-copy colour and style, key poses bent by rules, icons rendered from models. |

## Can Godot do it? The summary

Everything `PROJECT.md` asks is within Godot's reach without changing its source.
What Godot does not do for us, we build on top of it.

| Area | Verdict | Built by us | Risk |
|---|---|---|---|
| The 3D pixel look | Yes, on the Mobile renderer | outlines from rebuilt normals, our light function | medium |
| Steady 60 frames a second | Likely | MultiMesh grouped by area, frame pacing | high: the phone's chip and its drivers |
| Firelight on many figures and huts | Limited: MultiMesh copies stop receiving light past the per-object light limit | a firelight term in our shaders, if needed | medium: new, from note 17 |
| One zoom from a person to the globe | Not built in | our own level of detail, moving origin, map look, globe | high |
| Thousands of full minds | Not Godot's job | the C++ library on the middle cores | high: the processor budget |
| The same history on phone and cloud | Outside Godot | our maths and fixed-order threads | medium |
| People and animals moving | Yes, without skeletons | rigid block parts posed in C++ | low |
| Crafts, discovery and culture | Not Godot's job | the C++ library and checked catalogues | high: the pace is new ground |
| The phone's writer | Yes, through a small Android plugin | the plugin and the check | medium: beta API, foreground only, quotas |
| Sound | Yes: 3D players, distance filters, cave reverb | muffling by land, the voice manager | low to medium |
| Interface in both orientations | Yes: containers, safe areas, integer-scaled fonts | our gesture reader | low |
| Names in the generated language | Yes, if the font has every letter | spellings drawn only from the font's letters | low |
| Tests and pictures in the cloud | Yes: headless runs, Movie Maker mode, software Vulkan | the harness | low |

## The prototypes before production

Each answers one question and is then thrown away (research 00); P1 to P14 is the order the plan builds them in, riskiest first.
Several share one build.

| Prototype | Question | Where | Notes |
|---|---|---|---|
| P1 The look | Do the four outline methods, our light and firelight on MultiMesh reach the art book's look on the Mobile renderer? | phone | 04, 05, 17 |
| P2 A full scene | Does a busy camp hold 60 frames a second, and for how long before the phone heats? | phone | 01, 02 |
| P3 The kit | Do the shared shapes, the block figure's movements and a hut in two materials read well, at noon and at night? | phone | 17 |
| P4 Discovery pace | Can tuning alone make flakes and fire come within their windows? | cloud | 11 |
| P5 The same bits | Do the phone and the cloud, one thread and four, end a world identically? | both | 03, 16 |
| P6 A thousand minds | Do a thousand simple minds with needs, choice, talk and paths keep a year a minute at held speed? | phone | 10 |
| P7 World generation | How long do a candidate and its settling run take on your phone? | phone | 06 |
| P8 The zoom | Does one pinch from the globe to a person stay smooth at every stop? | phone | 07 |
| P9 Ecology | Do the animal and plant totals stay believable for 100 years with nobody in them? | cloud | 08 |
| P10 Culture from causes | Do customs, a spirit, a rite and a band split arise in their windows, each from its own cause? | cloud | 12 |
| P11 The director | Does its budget hold on recorded worlds while catching every named discovery? | cloud | 13 |
| P12 The interface | Do thumb reach, gestures and crisp text work in both orientations? | phone | 14 |
| P13 The writer | How many reworded sentences pass the check, how fast, and when do the quotas bite? | phone | 13 |
| P14 Sound | Do 32 voices with filters, reverb and the murmur play without breaks? | phone | 15 |

Then the **vertical slice:** one band at a cliff camp through a day, at the final look, in the real architecture, on your phone (research 00).
It proves the whole pipeline once and sets the quality bar for production.

## Decided with you

The research found three places where `PROJECT.md` disagreed with the evidence.
On 4 October 2026 you agreed to change all three:

- **`RCK-12` Glue from bark:** a second route.
  Birch bark burning in the open beside a smooth stone leaves tar on the stone (research 11).
- **`RCK-06` Leather:** a second route, brain tanning (research 11).
- **The glossary's "band":** now "a few families, linked by kin and marriage", since censuses of 32 foraging peoples find most band members are not close kin (research 12).

## What comes next

The architecture and the plan now follow the guide of research 00 (4 October 2026), waiting for your OK:
1. a pre-production milestone of the prototypes above, numbered as the plan builds them, riskiest first, and the vertical slice;
2. then production bottom up.
