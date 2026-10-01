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

**B74** (Rust, `src/`). One rule for every material and shape (`PRN-07`):
- **Objects:** a flint slab (16 cm), a granite anvil stone on the ground (30 cm), a dry stick held in one hand (50 cm), a hollow long bone (24 cm), each struck by a 350 g quartzite hammerstone.
- **A1 modal:** ringing notes from beam and plate theory; strike length from the Hertz contact law (200 to 630 microseconds), which sets brightness; ring time from the material's damping plus what holds it; total vibration energy capped by the energy the collision loses; a click from the contact itself starts each strike.
- **A2 shaped noise:** the same click, then noise around the strongest note, with the same brightness and decay.
- **Instruments:** flute notes from the tube's acoustic length, corrected for open ends and each open hole's size; tone from harmonics plus breath noise. Drum modes from the membrane equations, lowered by the air the hide pushes.
- **Mixer:** each sound gets a distance that sets loudness and muffling, and a direction (`SND-08`). Nothing allocates on the audio thread.
- **Cost:** 200 strikes per material rendered to silence, and 8 to 512 always-ringing sounds mixed, on one core.

**B76** (`speech.py`):
- **The language:** consonants `p t k q ʔ m n ŋ s x ɬ l r w j`, vowels `a e i o u`, syllables like `ta` or `tan`, stress first, verb last; five sentences from 16 words. It includes sounds English lacks: `q ʔ x ɬ`, trilled `r`, word-initial `ŋ`, plain `e o`.
- **S1:** espeak-ng fed phonemes directly, Welsh phoneme table; three people by voice type, pitch and speed.
- **S2:** Piper voices fed IPA directly, no text front end: English-trained (904 speakers; three picked by pitch) and Welsh-trained (one speaker; speed varied).
- **Sounds kept:** S1, by checking espeak-ng says each sound as itself. S2, by running the training language's text through espeak-ng (PROJECT.md for English, a Welsh Wikipedia article) and counting each sound: kept if it is at least 0.1% of all sounds.
- **Cost:** CPU time per second of speech, one core.

## Results

1 October 2026. Cloud timings: one core, under the lock, median of 3 runs. Raw numbers in `results/`.

**B74, the four materials** (one strike at full speed; "alone" is one sound rendered until silent):

| Material | Strongest ring | Ring time | Contact | A1 sounds per core, alone | A2 sounds per core, alone |
|---|---|---|---|---|---|
| Flint | 4.2 kHz | 0.53 s | 249 µs | 7,300 | 4,800 |
| Granite | 3.1 kHz | 0.07 s | 196 µs | 5,400 | 4,400 |
| Dry wood | 473 Hz | 0.36 s | 634 µs | 6,700 | 4,700 |
| Bone | 1.3 kHz | 0.12 s | 247 µs | 6,200 | 4,300 |

- **In the mixer** (placing, muffling and panning included, every sound always ringing): A1 about 2,900 to 3,000 sounds per core at 8 to 512 sounds; A2 2,700 to 3,200. 128 sounds use 4.4% of a core (A1) and 3.9% (A2); 512 use 17% and 19%.
- **A1 is no dearer than A2:** its resonators stop as they fade, while A2 filters noise for the whole sound.
- **Instruments:** flute 1,150 sounds per core, drum 5,400. Flute A plays 753 to 1,584 Hz from its holes, flute B 535 to 1,009 Hz. Every note came out within 3.3 cents of the pitch worked out from its shape.
- **Phone library:** builds for arm64 in 12 s, 0.53 MB, exports the JNI entry, 16 KB aligned, needs only Android's own libraries (`results/android-build.txt`). Not yet run on the phone.

**B76, speech:**

| Approach | Share of one core while speaking | Size on the phone | Sounds kept (of 20) | Bent toward a real language |
|---|---|---|---|---|
| S1 espeak-ng, Welsh table | 0.16% | about 1.2 MB | 20 | none |
| S2 Piper, English-trained | 9.1% | 79 MB voice, plus onnxruntime | 13 | `q ʔ x ɬ r a e` |
| S2 Piper, Welsh-trained | 9.1% | 64 MB voice, plus onnxruntime | 18 | `q ʔ` |

- espeak-ng's bending depends only on the table chosen: with its English table, 18 kept (`r` becomes `ɹ`, `a` becomes `æ`).
- Piper takes about 1.2 s to load a voice. The three English-trained speakers picked by pitch have median pitches of 104, 183 and 285 Hz.

**Listening page:** `listen.html`, 0.84 MB, 32 clips, each comparison matched for loudness.

## Verdict so far

- **B74:** both impact approaches pass the cost gate about six times over (3,000 sounds per core against 500 needed), so cost doesn't decide (rule 2). **Your ear chooses, material by material.** With no preference, A1 (modal) wins, and it costs no more. The instrument check passes (rule 4). The audio delay and load on the phone wait for the result code (rule 5).
- **B76:** both approaches pass the gate (rule 1). **Your ear chooses** on naturalness, own-language feel and distinct people (rule 2). With no preference, S1 wins: it keeps all 20 sounds (rule 3).
- **Flag for the architecture (rule 4):** the Welsh-trained voice keeps 18 sounds and the English-trained one 13, so a neural voice bends toward its training language. A neural voice for many invented languages would need training on a wide range of sounds.

## Caveats

- **Stand-ins and simple shapes.** A flake breaking off isn't modelled; the 30% of collision energy that becomes vibration is a guess.
- **The flute check tests only the synthesis.** The pitch rule from bore and holes is the textbook one, unchecked against a real bone flute.
- **"Sounds kept" is counted from text, not heard.** Your ear is the real test.
- **Licences.** espeak-ng is GPL-3: fine on your own phone, but it limits ever sharing the app. Piper's code is MIT; the English voice's data is CC BY 4.0; the Welsh voice's licence is unclear, and it was retrained from an English voice.
- **Cloud is not the phone.** Timings use plain x86-64 (no AVX), close to the phone's vector width; the phone may still differ by 2x either way.
- **Page audio** is Ogg Opus at 24 to 64 kbps; the neural voices make 22 kHz audio. The test language is hand-made, not grown by `CUL-17`.

## What only the phone can answer

- The real output delay through AAudio, and whether the low-latency and exclusive modes are granted.
- The mixing cost on the Tensor G6's big and small cores, and which core the audio thread lands on.
- Dropouts while the simulation and drawing also run (not tested in this round).
- Speech cost on the phone's cores (onnxruntime on ARM; the phone's AI hardware was not tried).
- How the sounds come across on the phone's own speaker, which can't play the large drum's lowest notes.

## How to re-run

From this folder:
1. `./setup.sh` installs espeak-ng and fetches the Python packages and Piper voices into the shared cache (about 150 MB).
2. `timeout 3600 flock $CACHE/cpu.lock timeout 840 ./run_all.sh` builds and tests the Rust library, renders the clips, times everything three times, builds the Android library, makes the speech clips, and writes `listen.html` and `results/summary.json`. About 2 minutes. `STEPS=page` (or any of `rust bench android speech page`) runs only those steps.

Results land in `results/`; clips and build outputs stay in the cache.

## The owner's verdict (1 October 2026)

- **Impacts:** A2 (noise shaped by the material's properties) sounds much better than A1 (ringing modes). A2 is chosen.
- **Speech:** the synthetic voice (S1, espeak-ng) is terrible. A neural voice (S2) is chosen; the next step is a neural voice that keeps more of an invented language's sounds.
- **Instruments:** the flutes are fine; the drums sound a bit weak.

## Drums, round 2: more body (after your verdict)

**Why they sound weak.** The page plays each clip as loud as its peaks allow. A drum strike is one short spike, so its peak stops it early. Through a stand-in for a phone speaker (it loses almost everything below about 350 Hz), the page played the small drum about 14 dB quieter than the flutes, and the large drum about 25 dB quieter: 96% of the large drum's sound is below what a phone speaker can play.

**What's tried.** A last step before the speaker, used only when the phone plays through its own speaker (headphones keep the full sound). The drums themselves don't change.
- **Louder:** a fast limiter that holds back only the first instant of each strike, so a drum can play as loud as a flute.
- **Louder, with phone bass:** the same, plus overtones of the deep notes in the range a phone can play, so the ear still hears the deep note. Most phones and small speakers use this effect.

**Decision rule (set before measuring).** Loudness is measured through the phone stand-in, with the standard loudness weighting, over the louder moments.
- "Louder" passes if both drums come within 3 dB of the flutes with at most 12 dB of limiting.
- "Phone bass" earns its place if it raises the large drum's loudness on the phone stand-in by at least 6 dB more than "Louder" alone, with the same limit on limiting.
- Your ear then picks among "as before", "louder" and "louder, with phone bass". The step goes into the app's sound only if you pick it.
