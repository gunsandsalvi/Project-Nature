# Research: how other developers build each part, and what Kindling takes

Pre-production research, done the way a studio does it before writing the design (research 00), bottom up.
Each note gives its sources, and the architecture cites the notes.

| Note | The decision in one line |
|---|---|
| [00 How teams work](00-process.md) | Pre-production first, then bottom-up milestones, each on your phone; detail only the next milestone; a quality gate on the look before anything is built on it. |
| [01 The engine](01-engine.md) | Godot, after a bake-off; the simulation in C++; Godot's source changed only as a last resort. |
| [02 The simulation core](02-core.md) | A C++ library inside Godot, separate from the scene: EnTT entities, content as data, fixed ticks and an event queue, keyed randomness, the same bits on phone and cloud, versioned saves. |
| [03 Drawing 3D as pixel art](03-rendering.md) | Low resolution scaled up, a pixel-locked camera, outlines from depth and normals, three bands of light, real shadows, grass and leaf cards, water. |
| [04 The art guide](04-art.md) | Soft painted nature with pixel-textured made things, one texel density, hue-shifted ramps, Stone Age props; textures made by code. |
| [05 Making and drawing the world](05-world.md) | A generator in the order of causes (plates to biomes) on 1 km cells, tuned for the look; detail made on demand from the seed; rings of ground detail; a moving origin. |
| [06 Plants and animals](06-life.md) | Species as data placed by rules; totals on world cells with predator-and-prey rules; individuals near people; herds by steering rules. |
| [07 Minds](07-minds.md) | Choosing by utility with kept reasons, a small planner on top, feelings and memories at three depths, knowledge per person, social rules as data, layered pathfinding. |
| [08 Culture](08-culture.md) | Naming languages from the seed, customs and beliefs as data rules made by experience, history simulated, never invented after the fact. |
| [09 Story and writing](09-story.md) | A story sifter that only chooses what to show, the book of ages from pattern sentences, and the phone's Gemini Nano only rewording, checked. |
| [10 The interface](10-interface.md) | The world fills the screen; bottom sheets in portrait, a side column in landscape; our own gesture reader; a pixel theme. |
| [11 Sound](11-sound.md) | Layered ambience from what is really there, and a murmur in the Animalese way from each people's language. |
| [12 Testing](12-testing.md) | C++ unit tests, seeded scenes, gdUnit4, golden pictures from the software driver, the same-history check, one check command. |
| [13 Models, textures, animation by code](13-assets.md) | The model kit as code plus catalogue data, textures by code, animation as key poses bent by rules, a model sheet to judge. |
