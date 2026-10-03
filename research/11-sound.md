# Research 11: sound

**Question:** how do games make a living soundscape and a murmur of talk without real words (`SND`, `PRE-45`), cheaply on a phone?

## How others do it

- **Layered ambience:**
  - loops of 10 to 30 seconds, of different lengths, so their joins never line up;
  - plus one-shots (a bird, a crack of wood) that play at random times, volumes, pitches and positions, sometimes not at all.

  Dozens of such sounds blend into a soundscape that never repeats ([CRIWARE](https://blog.criware.com/index.php/2020/12/14/ambience-design-in-atom-craft-part-1-2/), [Splice](https://splice.com/blog/audio-soundscape-for-video-games), [Game Audio Implementation](https://oreilly.com/library/view/game-audio-implementation/9781317679455/xhtml/Ch001.xhtml)).
- **Animalese** (Animal Crossing) builds speech from short recorded syllables, sped up and chained.
  Pitch rises with a happy mood and falls with a sad or angry one, giving "a foreign yet familiar feel" ([Nookipedia](https://nookipedia.com/wiki/Animalese), [Matinée](https://matinee.co.uk/blog/video-game-languages/)).

## What we take

1. **Ambience by layers and one-shots, from what is really there** (`PRN-10`).
   - The loops follow the place: wind, river, sea, forest.
   - The one-shots are the animals, fires and work actually near the camera, positioned in 3D with Godot's audio players and buses.
2. **The murmur in the Animalese way, from the people's own language** (research 08).
   - Each spoken word becomes a run of syllables from a small recorded bank, in the speaker's voice.
   - The pitch follows their mood.
   - What is said comes from the simulation's talk events, so the murmur at a camp is real talk (`SND-03`).
3. **Recordings over synthesis where it matters:**
   - recorded sounds, with their sources credited (`SND-06`), for the world;
   - the syllable bank for voices;
   - only simple sounds made by code.
