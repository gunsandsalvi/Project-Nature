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

V2_RESULTS

**The phone part.** `WriterTest.kt` is ready to bundle. BUILD_RESULT It needs `android.useAndroidX=true`, because ML Kit brings in AndroidX; the first build failed without it. Google lists the Pixel 11 series as Gemini Nano v4 on this API. It works only while the app is on screen, refuses phones with an unlocked bootloader, and takes input under 4,000 tokens.

**Gemma** (`GEMMA.md`). Gemma 4 E2B and E4B are now Apache 2.0 with no gate, and there is a Gemma 4 E2B build for the Tensor G6 (3.3 GB). The older Gemma 3 and 3n still need the licence and a token.

## Verdict so far

1. **The pipeline works end to end:** record, prompt, model, checker. The phone file is ready for the next test app.
2. **The checker is an aid, not yet a guard.** By the rule, the blind run missed a dark omission. The fixed version meets the numbers but wasn't tested blind. It catches added things well, but not mix-ups of who did what. Next: a check on roles, a fresh blind planted set, and the milestone skims (`RSK-08` signs) kept.
3. **Small models add or change facts in 15 to 30% of texts**, well above the bar of 2 in 20. Gemma 4 quietly softened forced labour despite an explicit rule. Expect the phone to need `PRE-41`'s rewrite and plain-text fallback, and a check on roles. The phone numbers decide.
4. **Speed is not the worry:** 10 to 13 words a second on 4 ordinary CPU cores already meets the 8 words a second bar. The first word, at 2.5 to 3.7 s, sits at the 3 s limit, which the phone's AI hardware should beat.
5. **The writing is flat** (`RSK-08`): the models copy 56 to 67% of their 4-word runs straight from the data.

## Caveats

- The stand-ins are not the phone's models: the runtime and compression differ, and Gemini Nano v4 hasn't been seen at all.
- I, an AI agent, labelled the real errors. Please spot-check `results/review.json`.
- The checker, the planted errors and the faithful texts all come from the same author, and only run 1 was blind.
- 20 texts is a small sample: each text is 5%. Each prompt ran once.
- The checker is only as good as its word lists. New kinds of record need new synonyms, or it rejects good text.

## What only the phone can answer

- **Gemini Nano v4:** added facts, softening, refusals (its safety filter is unknown), speed, heat, free memory, whether it was ready or downloading, and FULL against FAST.
- **Gemma 4 E2B on the Tensor G6** (next round, nothing needed from you): speed, memory inside the app, heat.
- **Your rating** of 6 texts: the feud killing, the dream and the sky-fire sign, in both voices.

## How to re-run

```sh
CACHE=/tmp/claude-0/-home-user-Project-Nature/d9fdddff-7118-505f-be5c-63935305a20b/scratchpad/cache
PY=$CACHE/b73/venv/bin/python   # venv with llama-cpp-python 0.3.36 (built from PyPI source), nltk, wordfreq
# models into $CACHE/b73/models, from huggingface.co/<repo>/resolve/main/<file>:
#   Qwen/Qwen2.5-1.5B-Instruct-GGUF qwen2.5-1.5b-instruct-q4_k_m.gguf; ggml-org/gemma-4-E2B-it-GGUF gemma-4-E2B-it-Q4_0.gguf
$PY run_planted.py --out results/planted-run4.json                     # checker on planted errors
flock $CACHE/cpu.lock timeout 840 $PY run_standin.py --name gemma-4-e2b --prompts v2 \
    --model $CACHE/b73/models/gemma-4-E2B-it-Q4_0.gguf                  # stand-in, about 5 min
$PY check_texts.py results/standin-gemma-4-e2b-v2.json                 # check any texts, phone ones too
python3 make_prompts.py        # after editing records or prompts: re-embeds them in phone/WriterTest.kt
# compile check (outputs go to $CACHE): cd phone-check; then, with ANDROID_HOME=$CACHE/android-sdk,
# GRADLE_USER_HOME=$CACHE/gradle-home and CACHE set:
#   flock $CACHE/cpu.lock gradle --no-daemon --project-cache-dir $CACHE/b73/phone-check-gradle assembleDebug assembleRelease
```
