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

To come.

## Results

To come.

## Verdict so far

To come.

## Caveats

To come.

## What only the phone can answer

To come.

## How to re-run

To come.
