# Research 15: sound

**Question:** how do games make a living soundscape, sounds made by code, and a murmur of talk without real words, cheaply on a phone?
Can Godot play 32 sounds at once with distance, muffling and echo on your phone (`SND-01` to `SND-12`, `PRE-45`)?

## What `PROJECT.md` asks

- **Everything heard comes from something happening** (`PRN-10`):
  - work and fire near the camera;
  - talk;
  - animals' calls;
  - the place's own ambience by land, weather, hour and season (`SND-01`, `SND-11`).
- **32 sounds at once,** shared:
  - up to 6 for place and weather;
  - 4 for single voices;
  - 8 for music;
  - 2 kept free;
  - at least 12 for work, fire, footsteps and animals.

  When a share is full, its quietest sound joins its blend (`SND-01`).
- **About 44 base sounds made by the game from shaped noise,** changed by the things involved, plus at most about 120 recordings (`SND-06`).
  - Harder is brighter, bigger is deeper, and wetter is duller.
  - No two strikes are the same.
- **The murmur:** talk strung from a bank of spoken syllables in the language's sounds, rendered once in the cloud for two base voices.
  It is shifted for age, build and feeling (`SND-03`).
- **Music** from each song's record of notes and rhythm (`SND-02`).
- **Space:** direction, quieter and duller with distance, muffled by land between, echo in caves (`SND-08`).
- **Sound follows time:** in step with the animation up close, blended into a hum further out (`SND-07`).
- **A last step for the phone's speaker** lifts the deep sounds it plays badly (`SND-06`).

## How others do it

- **Ambience in layers:** CRIWARE's guide cuts long ambiences into chunks and plays them "in random order", crossfaded ([CRIWARE](https://blog.criware.com/index.php/2020/12/14/ambience-design-in-atom-craft-part-1-2/)).
  - It lays randomised one-shots on top, with random timing, pitch, position and chance.
  - The loop keeps the voice count low, while the one-shots keep it from repeating.
- **Sound from base samples and rules:** in No Man's Sky, "the game's audio, including ambient sounds and its underlying soundtrack, also uses procedural generation methods from base samples created by audio designer Paul Weir and the British musical group 65daysofstatic" ([Wikipedia](https://en.wikipedia.org/wiki/No_Man%27s_Sky)).
  It is the same split as `SND-06`: a small base set, varied by rules.
- **Talk without words:** in Animal Crossing's Animalese, "each letter spoken is matched and synthesized with the basic sound of the letter" ([Nookipedia](https://nookipedia.com/wiki/Animalese)).
  - "Happy villagers will speak in a higher pitch, while sad or angry villagers will have a lower one."
  - Later games tie pitch to species, "linked to the species' body size".

  This is `SND-03`'s murmur: syllables of the language, shifted for build and feeling.
- **Bass on a small speaker:** the "missing fundamental" lets small speakers seem to play bass.
  Low notes are filtered out, then "harmonics are synthesized above the low notes" and mixed back in ([Wikipedia: missing fundamental](https://en.wikipedia.org/wiki/Missing_fundamental)).
  This is the known technique behind `SND-06`'s last step for the phone's speaker.

## Can Godot do it?

| Need | What Godot has | Verdict |
|---|---|---|
| Direction and distance | `AudioStreamPlayer3D`, whose attenuation model decides "if audio should get quieter with distance linearly, quadratically, logarithmically", with a `max_distance` ([class docs](https://docs.godotengine.org/en/stable/classes/class_audiostreamplayer3d.html)) | Yes |
| Duller with distance | Its built-in low-pass: "A sound above this frequency is attenuated more than a sound below this frequency" (`attenuation_filter_cutoff_hz`) | Yes |
| Muffled by land between | Nothing built in | Ours: the simulation tests the line through the ground and lowers that player's cutoff |
| Echo in caves | `Area3D` reverb: "the area applies reverb to its associated audio", with amount and uniformity ([Area3D](https://docs.godotengine.org/en/stable/classes/class_area3d.html)) | Yes, an area per cave |
| No two strikes the same | `AudioStreamRandomizer`: "Wraps a pool of audio streams with pitch and volume shifting", avoiding repeats ([class docs](https://docs.godotengine.org/en/stable/classes/class_audiostreamrandomizer.html)) | Yes |
| Sounds made by code | `AudioStreamGenerator`, "best used from C# or from a compiled language via GDExtension" ([class docs](https://docs.godotengine.org/en/stable/classes/class_audiostreamgenerator.html)) | Yes, from our C++, or baked into sound buffers at load |
| Mixing and effects | Buses route "from buses on the right to buses further to the left"; silent buses are switched off ([Godot docs: buses](https://docs.godotengine.org/en/stable/tutorials/audio/audio_buses.html)) | Yes |
| The speaker's bass step | Low-pass, high-pass, shelf, compressor, limiter and distortion effects on a bus ([Godot docs: effects](https://docs.godotengine.org/en/stable/tutorials/audio/audio_effects.html)) | Yes: harmonics from a distortion on a low band, mixed back |
| 32 voices at once | Godot sets no number; "Only the hardware of the device … will limit the number of buses and effects" | To measure on your phone (`SND-01`'s Done when) |

**Verdict:** Godot covers all of it but the muffling by land, which is ours to compute.
The cost of 32 voices with filters and reverb must be measured on the phone, as `SND-01` already asks.

## What we take

1. **Ambience as layers plus one-shots,** CRIWARE-style.
   - The loops come from the place's land, water, weather, hour and season (`SND-11`).
   - The one-shots come only from real things near the camera: birds the place holds, work, fire and calls (`PRN-10`).
2. **Base sounds made by code once, then varied:**
   - our C++ shapes noise into the 44 base sounds at load time;
   - `AudioStreamRandomizer`-style pitch and volume variation and each thing's sound blueprint make every play differ (`SND-06`).

   Live synthesis is kept for what must follow the world continuously, such as fire by its heat and wind by its speed.
3. **Space with Godot's own tools:** 3D players for direction and distance, their low-pass for distance, an area with reverb for each cave.
   Muffling by land comes from the simulation's line test (`SND-08`).
4. **A voice manager of our own** keeps `SND-01`'s 32-voice shares, and blends the quietest sound into its kind's hum when a share is full (`SND-07`).
5. **The murmur, Animalese-style:** syllables of the people's language from the cloud-rendered bank, shifted in pitch for age and build and in tune and speed for feeling (`SND-03`).
6. **The speaker step by the missing fundamental:** deep sounds are filtered out on the speaker and their harmonics mixed back; it is switched off with headphones (`SND-06`).
7. **A sound prototype before production,** on your phone: a camp scene with 32 voices, distance filters, a cave reverb and a running murmur.
   It measures the audio thread's time, listens for breaks, and records a reel for your review (`SND-12`).

## Sources

- Practice:
  - [CRIWARE: ambience design](https://blog.criware.com/index.php/2020/12/14/ambience-design-in-atom-craft-part-1-2/)
  - [Wikipedia: No Man's Sky](https://en.wikipedia.org/wiki/No_Man%27s_Sky)
  - [Nookipedia: Animalese](https://nookipedia.com/wiki/Animalese)
  - [Wikipedia: missing fundamental](https://en.wikipedia.org/wiki/Missing_fundamental)
- Godot:
  - [AudioStreamPlayer3D](https://docs.godotengine.org/en/stable/classes/class_audiostreamplayer3d.html)
  - [Area3D](https://docs.godotengine.org/en/stable/classes/class_area3d.html)
  - [AudioStreamRandomizer](https://docs.godotengine.org/en/stable/classes/class_audiostreamrandomizer.html)
  - [AudioStreamGenerator](https://docs.godotengine.org/en/stable/classes/class_audiostreamgenerator.html)
  - [Audio buses](https://docs.godotengine.org/en/stable/tutorials/audio/audio_buses.html)
  - [Audio effects](https://docs.godotengine.org/en/stable/tutorials/audio/audio_effects.html)
