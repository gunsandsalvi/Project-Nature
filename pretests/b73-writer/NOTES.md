# B73: the writer AI

Pre-test, wave 1. Throwaway: deleted once the architecture is written.

## The question

Which model and runtime write Kindling's text on the phone (`PRE-37`), and at what cost? Can a small model turn simulation data into text without adding facts (`PRE-17`, `PRE-41`), without softening dark events (`RSK-17`), fast enough (`RSK-22`) and well enough (`RSK-08`)?

## The approaches

1. **Gemini Nano**, the phone's built-in model, through Google's ML Kit GenAI Prompt API. It runs in the phone's AICore service, outside the app.
2. **Gemma**, an open model the app downloads and runs itself, through Google's LiteRT-LM runtime. Not on the phone this round. Planned as blocked by the licence, but Gemma 4 turned out to be open to all (see `GEMMA.md`).
3. **Cloud stand-ins only:** small open models on the CPU with llama.cpp: Qwen2.5 1.5B, and, once it proved open, Gemma 4 E2B itself (llama.cpp's official copy). They prove the pipeline and the checker. Their quality says little about the phone's models.

Two voices (`PRE-19`): documentary, and "their own tradition".

## Decision rules (written before measuring; not changed afterwards)

**The checker.** It is fit to guard `PRE-41` if, on the planted-error set, it catches at least 90% of planted errors, catches every softened or left-out dark event, and passes at least 9 of the 10 faithful hand-written texts. Otherwise it is only an aid, and its misses are listed.

**The stand-in.** It proves the pipeline if all 20 prompts give text and the checker runs on all of them. If more than half of its texts fail the check, the prompts are tightened once and rerun, both rounds are reported, and the phone gets the final prompts. Its failure rate is a first feel only.

**A phone model is good enough for the chronicle** if, on the 20 prompts (10 records, 2 voices), all of these hold:

1. **Added facts (`PRE-17`):** at most 2 of 20 texts have a confirmed added fact (a checker flag confirmed by reading, or one found by reading). 3 to 6 texts: usable only with one automatic rewrite on failure, then plain text (`PRE-41`). More than 6: not good enough.
2. **Dark events (`RSK-17`):** none of the 6 dark texts (3 dark records, 2 voices) is softened or left out. One refusal is tolerable, since plain text replaces it; two or more raise `RSK-17`.
3. **Speed (`RSK-22`):** median time to first word at most 3 s, and median speed at least 8 words a second. 4 to 8 words a second is acceptable only if text is written ahead, in pauses (`PRE-41`). Under 4 words a second, or any text taking over 60 s, is too slow.
4. **Heat (`RSK-22`):** if the phone's heat status reaches "severe" during the run, or the battery warms by more than 5 °C, flag `RSK-22`.
5. **Memory (`RSK-15`):** a model inside the app (Gemma) may use at most 2 GiB of the 10 GiB (`PLT-01`); 2 to 3 GiB only if its text is clearly better; over 3 GiB is out. Gemini Nano runs outside the app, so only the phone's free memory is recorded.
6. **Your rating (`RSK-08`):** you rate 6 texts (3 records, 2 voices each) as poor, acceptable or good. At least 4 of 6 must be acceptable or better, and you must be able to tell the two voices apart.

**Choosing between them.** If both pass, the one with fewer texts with confirmed added facts wins. If they are within 2 texts of each other, Gemini Nano wins (no download, no memory inside the app), unless you rate Gemma better on at least 2 more texts. If neither passes, raise `RSK-08`, `RSK-17` or `RSK-22` with options: tighter prompts, simpler fill-in text, or a bigger model.

## Method

- **Sample data** (`data/records.json`): 10 hand-made records, such as a first fire from striking stones, an old man left behind who dies of cold, a birth, a feud killing, a band splitting, a dream, a myth retold with drift, a sky-fire sign, a raid that takes two captives, and a meeting with gifts. Three are dark (`CUL-08`). Names are invented, and things are named in their own concepts ("cutting stone", "red earth", "sky-fire") (`PRE-38`). The "their own tradition" voice never sees facts the people can't know (`PRE-14`).
- **Prompts** (`prompts/`): documentary, and "their own tradition" (`PRE-19`). The DATA block in each prompt is made by fixed rules, so it is also the plain fallback text (`PRE-41`).
- **Checker** (`checker.py`): word lists, a stemmer and a few patterns, with no language model (`PRN-06`), so it can be ported to the phone. It flags every name, number, word, motive, belief, cause or modern word (such as "flint") that the data doesn't contain. On dark records it also flags euphemisms, hedges, left-out core events and the wrong killer, and it flags refusals and echoed prompts.
- **Planted errors** (`data/planted.json`): 95 single errors planted in 10 faithful hand-written texts, all written before the checker.
- **Stand-ins** (`run_standin.py`): 20 prompts, run on 4 x86 cores with the phone's settings (temperature 0.3, top-k 20, at most 256 new tokens). I read every text against its record (`results/review.json`).
- **Phone** (`phone/WriterTest.kt`): the same 20 prompts through Gemini Nano. It was compile-checked in a throwaway module (`phone-check/`); see `INTEGRATION.md`.

## Results

**The checker** (`results/planted-run*.json`)

- **Run 1, blind:** passed all 10 faithful texts, and caught 89 of 95 planted errors (94%), but only 10 of the 11 dark ones. One omission slipped through, because "alone" counted as "left behind".
- **Runs 2 and 3, after fixes (not blind):** caught 91 of 95 (96%), all 11 dark ones, and passed all 10 faithful texts. The fixes were a tighter synonym list, ordinals, the wrong band, echoed prompts, and 13 neutral words such as "occurred".
- **What it can't catch:** a wrong statement made only of words in the data. It missed all 3 planted contradictions and one swap of who gave what to whom. In model text, it missed "her" written for "his", and Kelo named as the killer when the data says the seven men.

**The stand-ins** (v1 prompts; `results/standin-*`)

| | Qwen2.5 1.5B | Gemma 4 E2B |
|---|---|---|
| Texts with an added or wrong fact (my reading) | 6 of 20 (2 minor) | 3 of 20 (2 minor) |
| Prompt echoed back | 3 | 0 |
| Dark texts softened or left out | 0 of 6 | 2 of 6 |
| Refusals | 0 | 0 |
| Texts failing the check (run 2, then run 3) | 6, then 6 | 11, then 4 |
| Of those, false alarms | 0, then 0 | 9, then 2 |
| Real problems the checker missed | 3 of 9 | 1 of 3 |
| First word (median) | 2.5 s | 3.7 s |
| Words a second (median) | 10.5 | 12.7 |
| 4-word runs copied from the data (hand-written texts: 35%) | 67% | 56% |

- **Gemma 4's softening:** in the raid, both voices dropped "made them carry", so the captives seem to carry the meat of their own will. The documentary voice also hid who took them ("were taken captive"). No euphemism was used: the force simply vanished.
- **The stand-in rule:** Gemma 4 failed the run-2 check on 11 of 20 texts, more than half (mostly false alarms), so the prompts were tightened once (`prompts/v2`) and both stand-ins were rerun. The phone gets v2.

**Rerun with the tightened prompts** (v2; checker run 3)

| | Qwen2.5 1.5B | Gemma 4 E2B |
|---|---|---|
| Texts with an added or wrong fact (my reading) | 6 of 20 (1 minor) | 2 of 20 |
| Prompt or data format echoed back | 5, plus 1 text that only repeats "Voice: their own tradition" | 0 |
| Dark texts softened or left out | 1 of 6 (that broken text) | 2 of 6 (the raid again) |
| Texts failing the check | 10 | 2 |
| Of those, false alarms | 2 | 0 |
| Real problems the checker missed | 3 | 0 |
| First word; words a second (medians) | 2.8 s; 11.6 | 4.7 s; 12.5 |
| 4-word runs copied from the data | 75% | 64% |

- **Gemma 4 improved.** It meets the added-facts bar, 2 of 20, but fails the dark rule. Both voices still drop "made them carry" and blame Kelo alone, even with a rule that names "made". The checker caught both.
- **The 1.5B model got worse** with the longer prompt: more embellishment, more echoes, and one broken text. A model that small can't hold many rules.
- **Voices** (`RSK-08`): the 1.5B model sometimes wrote the same text for both voices. Gemma 4's tradition voice is distinct but choppy ("Year 41. Deep winter. Night.").

**The phone part.** `WriterTest.kt` is ready to bundle. It builds in debug and in minified release (AGP 8.13.2, Kotlin 2.3.21, compileSdk 36, minSdk 31), with no warnings, in about 65 s. A test app holding only it and ML Kit is 1.7 MB. It needs `android.useAndroidX=true`, because ML Kit brings in AndroidX; the first build failed without it. Google lists the Pixel 11 series as Gemini Nano v4 on this API. It works only while the app is on screen, refuses phones with an unlocked bootloader, and takes input under 4,000 tokens.

**Gemma** (`GEMMA.md`). Gemma 4 E2B and E4B are now Apache 2.0 with no gate, and there is a Gemma 4 E2B build for the Tensor G6 (3.3 GB). The older Gemma 3 and 3n still need the licence and a token.

## Verdict so far

1. **The pipeline works end to end:** record, prompt, model, checker. The phone file is ready for the next test app.
2. **The checker is an aid, not yet a guard.** By the rule, the blind run missed a dark omission. The fixed version meets the numbers but wasn't tested blind. It catches added things well, but not mix-ups of who did what. Next: a check on roles, a fresh blind planted set, and the milestone skims (`RSK-08` signs) kept.
3. **Gemma 4 E2B is the most promising stand-in.** With the v2 prompts it adds or changes facts in 2 of 20 texts, at the bar, but it quietly softens forced labour in both voices, which an explicit rule didn't fix (`RSK-17`). Since the checker caught it, `PRE-41`'s plain-text fallback would show the plain facts instead. The 1.5B model is too small: 6 of 20 texts had an added or wrong fact, and it got worse with more rules.
4. **Speed is not the worry:** 10 to 13 words a second on 4 ordinary CPU cores already meets the 8 words a second bar. The first word, at 2.5 to 4.7 s on the CPU, is over the 3 s limit for the longer prompts. The phone's AI hardware should do better: the phone decides.
5. **The writing is flat** (`RSK-08`): the models copy 56 to 75% of their 4-word runs straight from the data. Hand-written texts copy 35%.

## Caveats

- The stand-ins are not the phone's models: the runtime and compression differ, and Gemini Nano v4 hasn't been seen at all.
- I, an AI agent, labelled the real errors. Please spot-check `results/review.json`.
- The checker, the planted errors and the faithful texts all come from the same author, and only run 1 was blind.
- 20 texts is a small sample: each text is 5%. Each prompt ran once.
- The v2 rules were written from the v1 failures, and even name this set's forcing words ("made", "left behind"). So v2's results are a best case, not a fresh measure.
- The checker is only as good as its word lists. New kinds of record need new synonyms, or it rejects good text.

## What only the phone can answer

- **Gemini Nano v4:** added facts, softening, refusals (its safety filter is unknown), speed, heat, free memory, whether it was ready or downloading, and FULL against FAST.
- **Gemma 4 E2B on the Tensor G6** (next round, nothing needed from you): speed, memory inside the app, heat.
- **Your rating** of 6 texts: the feud killing, the dream and the sky-fire sign, in both voices.

## How to re-run

```sh
. ../b78-b79-phone/tools/env.sh  # sets CACHE (the shared cache), ANDROID_HOME, GRADLE_USER_HOME
PY=$CACHE/b73/venv/bin/python   # venv with llama-cpp-python 0.3.36 (built from PyPI source), nltk, wordfreq
# models into $CACHE/b73/models, from huggingface.co/<repo>/resolve/main/<file>:
#   Qwen/Qwen2.5-1.5B-Instruct-GGUF qwen2.5-1.5b-instruct-q4_k_m.gguf; ggml-org/gemma-4-E2B-it-GGUF gemma-4-E2B-it-Q4_0.gguf
$PY run_planted.py --out results/planted-run4.json                     # checker on planted errors
flock $CACHE/cpu.lock timeout 840 $PY run_standin.py --name gemma-4-e2b --prompts v2 \
    --model $CACHE/b73/models/gemma-4-E2B-it-Q4_0.gguf                  # stand-in, about 5 min
$PY check_texts.py results/standin-gemma-4-e2b-v2.json                 # check any texts, phone ones too
python3 make_prompts.py        # after editing records or prompts: re-embeds them in phone/WriterTest.kt
# compile check, outputs in $CACHE (about 90 s):
cd phone-check && flock $CACHE/cpu.lock timeout 600 gradle --no-daemon \
    --project-cache-dir $CACHE/b73/phone-check-gradle assembleDebug assembleRelease
```

## Phone results, round 2 (1 October 2026)

Pixel 11 Pro XL, Android 17. Gemini Nano (`nano-v4-full`, through ML Kit) and Gemma 4 E2B (the general build, run by the app on the graphics chip) each wrote all 20 texts. Files: `results/phone-r2-nano.json`, `results/phone-r2-gemma.json`, the checker's `*.checked.json`, and my reading in `results/phone-r2-review.json`. The rules were written before measuring:

| Rule | Gemini Nano | Gemma 4 E2B |
|---|---|---|
| 1. Added or wrong facts (at most 2 of 20) | 3 of 20, 1 of them minor: usable only with one rewrite, then plain text | 1 of 20: pass |
| 2. Dark events (none softened) | 2 of 6 softened: fail | 1 of 6 softened: fail |
| 3. Speed (first word within 3 s, at least 8 words a second) | 0.26 s, 77 words a second: pass | 0.86 s, 14 words a second: pass |
| 4. Heat | no heat warning; battery 0.8 °C cooler: pass | no heat warning; battery 1.8 °C warmer: pass |
| 5. Memory | runs outside the app; 7.4 GB free | 2.0 to 2.7 GiB inside the app (2.4 GiB by Android's own measure): the 2–3 GiB band, allowed only if its text is clearly better, and it isn't |
| 6. Your rating (at least 4 of 6) | 3 of 6: fail | 3 of 6: fail |

- **The same failures in both:** in the raid (`r09`), the captives "carried" the meat, where the data says they were made to carry it (`RSK-17`). Both also named Kelo as the killer, where the data says the seven men (Gemini Nano in both voices, Gemma in one). The checker missed that, as expected: it is a wrong statement made only of words in the data.
- **Copying:** both models copied 65–71% of their four-word runs from the data. In the documentary voice their texts were nearly word for word the same, and `r04` and `r08` were identical, yet the same text got different ratings. So six ratings can't separate the two models.
- **The tradition voice** came out as choppy fragments of the data ("Year 41. Deep winter. Night. Harum."), and you rated 5 of its 6 texts poor. The documentary voice was acceptable or good in 5 of 6. Each text showed its voice, so whether you could tell the voices apart blind wasn't tested.
- **Gemma on the phone:** the 2.6 GB download took 39 s; start-up on the graphics chip took 20 s.

**Verdict:** neither model passes. As the rules say, this raises `RSK-08` (flat writing, worst in the tradition voice) and `RSK-17` (softened forced labour), with three options: tighter prompts, simpler fill-in text (the facts as plain sentences, `PRE-41`), or a bigger model. That choice is yours.

**Your decision (1 October 2026):** Gemini Nano writes the text. Dark events are never left to the model: they appear as plain stated facts taken from the data, so they can't be softened (`RSK-17`). The prompts are tightened when the writer is built.
