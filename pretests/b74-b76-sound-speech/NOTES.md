# B74 and B76: sound from properties, and speech in an invented language

Pre-test. Throwaway: deleted once the architecture is written.

## The question

- **B74:** can impact and instrument sounds be made from what things are made of and their shape, and can the phone mix enough of them at once with little delay (`SND-01`, `SND-06`, `SND-08`, `CUL-10`)?
- **B76:** how should people speak their invented languages on the phone, with the language's own sounds (`SND-03`, `CUL-17`)?

## The approaches

**B74, impacts** (flint, granite, dry wood and bone, each struck by a stone):
- **A1 modal:** the object's ringing modes are worked out from its stiffness, density, size, shape and damping; a contact model sets how bright the strike is.
- **A2 shaped noise (the cheap one):** a noise burst whose pitch region, brightness and decay come from the same properties.

**B74, instruments** (`CUL-10`): a bone flute whose notes come from its bore length and finger holes, and a hide drum whose modes come from the membrane's size and tension.

**B74, phone audio:** Android's low-latency audio (AAudio), driven by a small Rust library.

**B76, speech** (a tiny invented language, 15 consonants and 5 vowels):
- **S1 synthetic:** the espeak-ng formant synthesiser, fed the language's phonemes directly.
- **S2 neural:** a Piper neural voice, fed IPA phonetic spelling directly. Two voices: one trained on English (many speakers), one trained on Welsh (a closer set of sounds).

## Decision rules

Written before making anything; not changed afterwards.

**The owner's ear makes the final choice.** The rules below decide what goes on the listening page and what is measured; numbers only rule an approach out or break a tie.

**What goes on the listening page:**
- The four materials, each struck three times, by A1 and by A2 (8 clips), plus one material at three sizes.
- The bone flute playing its holes in turn, a second flute of another length, and two drums (small and tight, large and slack), struck at the centre and near the edge.
- One busy camp mix, to hear many sounds at once.
- All five sentences by S1 and by the English S2 voice, two or three speakers each, and some by the Welsh S2 voice. Each caption gives the sentence and its English meaning.

**What is measured** (cloud timings: one core, under the shared lock, median of 3 runs):
- Cost per second of sound or speech, and how many sounds one core can mix in real time.
- For each material: pitch, brightness and ring time, to check that the properties drive the sound.
- Flute: each note's measured pitch against the pitch worked out from the bore and holes.
- Speech: "sounds kept": of the language's 20 sounds, how many each approach makes as its own sound rather than a substitute from a real language (espeak-ng: the sound exists in the phoneme table used; Piper: the sound occurs in the voice's training language). Size on storage.

**B74 rules:**
1. **Cost gate:** an impact approach passes if one cloud core can run at least 500 of its sounds at once (each sound 0.2% of a core or less). That allows 64 sounds within a quarter of one phone core, with room for a phone core half as fast.
2. If A1 and A2 both pass, cost doesn't decide: the owner's ear does, material by material. If the owner has no preference, A1 wins, because it follows the physics with one general rule (`SND-06`, `PRN-07`).
3. If only A2 passes, use A1 for close sounds only, with a cap, and A2 for the rest.
4. **Instrument check:** each flute note must come out within 10 cents of the pitch worked out from its shape; otherwise the model is wrong, whatever it sounds like.
5. **Phone audio** (from the result code): AAudio stays the plan if the median output delay is 50 ms or less and there are no dropouts at 8 and 32 sounds. Over 50 ms: flag for sound and picture sync. Over 100 ms or dropouts: try another audio path next round. The planned cap on sounds at once is the largest tested count (8, 32 or 128) whose audio work stays within 25% of the time available.

**B76 rules:**
1. **Gate:** an approach fits the phone if it costs no more than a quarter of one cloud core per second of speech, and the engine plus one voice takes 150 MB or less.
2. Among those that pass, the owner's ear chooses, on three things: does it sound natural, does it sound like a language of its own rather than accented English or Welsh, and do the speakers sound like different people.
3. If the owner has no preference: the approach that keeps more of the 20 sounds wins; on a tie, the cheaper one.
4. If the Welsh voice keeps clearly more sounds than the English one, a neural voice bends toward its training language, and that is flagged for the architecture.

## Method

All values are plausible stand-ins, not sourced (`PRN-05`). Sounds are made at 48 kHz.

**B74 impacts** (`src/impact.rs`). One rule for every material and shape (`PRN-07`):
- **Objects:** a flint slab (16 x 9 x 2 cm), a granite anvil stone on the ground (30 x 20 x 6 cm), a dry stick (50 cm, 3 cm thick), a hollow long bone (24 cm). Each material has a stiffness, density, Poisson's ratio, internal damping and a "crunch" for grainy surfaces. The striker is a 350 g quartzite hammerstone at about 2.5 m/s.
- **A1 modal:** ringing frequencies come from beam and plate theory (bending, twisting, along-the-length modes). The Hertz contact law gives how long the strike lasts (90 to 640 microseconds here), which sets how bright it is. Each mode's ring time comes from the material's damping plus what holds it (a hand, the ground). Small or narrow objects radiate low notes poorly. The total vibration energy can't exceed a share of the energy the collision loses. A short click from the contact itself, plus crunch noise, starts every strike. Each mode is a two-pole resonator, eight at a time.
- **A2 shaped noise:** the same click, then noise through one resonance at the strongest mode, a low-pass at the contact's brightness, and the same decay.
- **Instruments** (`src/instrument.rs`): flute notes from an open tube's acoustic length, with the standard corrections for open ends and for each open hole's size and wall thickness. The tone is harmonics plus breath noise filtered by the bore, whose sharpness comes from wall losses. Drum modes come from the zeros of Bessel functions, lowered by the air the hide pushes, with excitation set by where it is struck and by a hand or a stick.
- **Mixer** (`src/mix.rs`): each sound gets a distance (1.5 to 25 m) that sets its loudness and how muffled it is, and a direction (`SND-08`). Nothing allocates on the audio thread.
- **Cost:** a command-line tool (`src/bin/render.rs`) renders 200 strikes per material to silence, and mixes 8 to 512 always-sounding voices, on one core. Run three times under the lock; medians reported.
- **Phone:** the same library plays the mix through AAudio (see `INTEGRATION.md`).

**B76 speech** (`speech.py`):
- **The language:** consonants `p t k q ʔ m n ŋ s x ɬ l r w j`, vowels `a e i o u`, syllables (C)V(N), stress on the first syllable, verb last. Five sentences from a 16-word lexicon. It includes sounds English lacks: `q ʔ x ɬ`, trilled `r`, word-initial `ŋ`, and plain `e o`.
- **S1:** espeak-ng 1.51 with `[[...]]` phoneme input and its Welsh phoneme table, which has all 20 sounds. Three people by voice variant, pitch and speed.
- **S2:** Piper medium voices run directly with onnxruntime, fed IPA phoneme ids, with no text front end. English-trained (LibriTTS-R, 904 speakers; three picked automatically as lowest, middle and highest pitch) and Welsh-trained (one speaker; speed varied).
- **Sounds kept:** S1, by asking espeak-ng for each sound and checking it comes out as itself. S2, by running the voice's training language through espeak-ng (PROJECT.md for English, the Welsh Wikipedia article on Wales for Welsh) and counting how often each sound occurs on its own (diphthongs counted as one sound). A sound kept by the voice is one that makes up at least 0.1% of the training language's sounds.
- **Cost:** CPU time per second of speech on one core (espeak-ng pinned with taskset; onnxruntime with one thread), three runs, median.

## Results

To come.

## Verdict so far

To come.

## Caveats

- **Stand-in values and simple shapes.** Slabs use beam theory and rods are ideal tubes. Knapping really includes a flake breaking off, which isn't modelled. The share of collision energy that becomes vibration (30%) is a guess.
- **The flute check only tests the synthesis.** The tone is built from harmonics, so it plays the pitch it is given. The pitch rule from bore and holes is the textbook one but unchecked against a real bone flute.
- **"Sounds kept" is a count from text, not a listening test.** No phoneme recogniser was run on the clips. Your ear is the real test.
- **Licences.** espeak-ng is GPL-3: fine for an app on your own phone, but it limits ever sharing the app. Piper's original code is MIT; each voice has its own licence: the English voice's data is CC BY 4.0; the Welsh voice's model card only points to Bangor University's corpus page, so it is unclear. The Welsh voice was itself retrained from an English voice.
- **Cloud is not the phone.** Timings use the plain x86-64 instruction set (no AVX), close in width to the phone's vector unit; the phone may still differ by 2x either way.
- **Page audio** is Ogg Opus at 24 to 64 kbps, which can soften the sharpest clicks a little. The neural voices make 22 kHz audio.
- The test language is hand-made, not grown by `CUL-17`'s mechanisms.

## What only the phone can answer

- The real output delay through AAudio, and whether the low-latency and exclusive modes are granted.
- The mixing cost on the Tensor G6's big and small cores, and which core the audio thread lands on.
- Dropouts while the simulation and drawing also run (not tested in this round).
- Speech cost on the phone's cores (onnxruntime on ARM; the phone's AI hardware was not tried).
- How the sounds come across on the phone's own speaker, which can't play the large drum's lowest notes.

## How to re-run

From this folder:
1. `./setup.sh` installs espeak-ng and fetches the Python packages and Piper voices into the shared cache (about 150 MB).
2. `timeout 3600 flock $CACHE/cpu.lock timeout 840 ./run_all.sh` builds and tests the Rust library, renders the clips, times everything three times, builds the Android library, and makes the speech clips. About 8 minutes.
3. `$CACHE/b76/venv/bin/python build_page.py $CACHE/b74-run` writes `listen.html` and `results/summary.json` (under the lock too: it encodes the clips).

Results land in `results/`; clips and build outputs stay in the cache.
