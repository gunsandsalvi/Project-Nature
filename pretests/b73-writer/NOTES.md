# B73: the writer AI

Pre-test, wave 1. Throwaway: deleted once the architecture is written.

## The question

Which model and runtime write Kindling's text on the phone (`PRE-37`), and at what cost? Can a small model turn simulation data into text without adding facts (`PRE-17`, `PRE-41`), without softening dark events (`RSK-17`), fast enough (`RSK-22`) and well enough (`RSK-08`)?

## The approaches

1. **Gemini Nano**, the phone's built-in model, through Google's ML Kit GenAI Prompt API. It runs in the phone's AICore service, outside the app.
2. **Gemma**, an open model the app downloads and runs itself, through Google's LiteRT-LM runtime. Blocked this round: the download needs your licence OK (see "Gemma" below).
3. **Cloud stand-in only:** a small open model (Qwen2.5, 1.5 billion parameters) on the CPU with llama.cpp. It proves the pipeline and the checker. Its quality says little about the phone's models.

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

(to come)

## Results

(to come)
