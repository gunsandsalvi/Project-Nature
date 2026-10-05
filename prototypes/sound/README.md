# P14 Sound

The fourteenth prototype (IMPLEMENTATION α0.7c, research 15) asks: do 32 sounds at once with distance filters, a
cave's echo and the murmur of talk play without breaks on your phone?

Items it is about: `SND-01` (a lively camp: 32 sounds at once in shares, the quietest of a full share joining its
blend), `SND-03` (the murmur: syllables of the people's language from a bank of two base voices, shifted for age,
build and feeling), `SND-08` (direction and distance, muffled by land, echoing in caves), `SND-11` (ambience by place,
hour and weather) and `SND-12` (a reel for your ears), with `SND-06` (sounds made by code: harder brighter, bigger
deeper and longer, wetter duller, no two the same) and `SND-07` (blends into a hum) as how it is made.

- **The base sounds** (`src/sound.cpp`, `SND-06`): made by code from shaped noise and ringing modes, varied by each
  play's seed and by what they are made of: a strike of stone on stone (a short contact sets the stone's irregular
  modes ringing), a hide scraped (stick-slip ticks), wood chopped, a footstep, a drum, the stream (bubbles over a low
  rush), rain, thunder; and stand-ins for the recordings of birds, a dog's bark and a wolf's howl. The tests check
  each `SND-06` rule on four of them, and that 20 flint strikes in a row all differ.
- **Made live** (A16): the fire, a roar under crackles and pops that grow with its heat, and the wind, a band of noise
  moving with its gusts, are made as they play on the audio thread, following their heat and speed.
- **The murmur's bank** (`bank.py`, `SND-03`): every syllable of eleven consonants before five vowels, and the vowels
  alone, spoken once by two base voices, a woman's and a man's, and the times of each voice's pulses marked (by
  zero-frequency filtering), as the cloud will render the game's bank from the voice you choose by ear. The voices
  here are stand-ins: open voices of the Piper speech engine, LJ Speech (public domain) and Joe (CC0). The bank is
  `prototypes/app/sound/bank/`, 1.8 MB.
- **Talk** (`SND-03`): a phrase strings words of one to three of a stand-in language's syllables, and moves each
  grain of the voice to a new pitch (TD-PSOLA): higher and shorter-throated for a child, lower and breathier with a
  wobble for the old; quicker, louder, higher and swinging for anger, slower, softer and falling for grief. Laughs,
  calls, cries and screams, which the game takes from recordings, are stand-ins made the same way.
- **Blends** (`SND-07`): a hum of copies of a kind's sounds round a loop, which the quietest sounds join when their
  share is full.
- **The extension** (`extension/`): the library in Godot. `SoundMaker` makes sounds and talk as 16-bit samples, from
  any thread; `SoundLive` is the fire and the wind as audio streams; `AudioEffectProbe`, the master bus's last effect,
  measures the audio thread's own CPU time for each block it mixes against the block's length.

`src/cli.cpp` writes every sound, talk in four kinds of voice and three feelings, and the cries, as WAV files. The app's
screen is `prototypes/app/sound/`.
