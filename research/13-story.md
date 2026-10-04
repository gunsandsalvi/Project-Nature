# Research 13: the story director, the book of ages and the writer

**Question:** how can the game find the moments worth seeing in a simulation, and show them to you at the right rhythm?
How can it write them up as readable history, when nothing may happen for the sake of a story (`PRN-12`) and language models may only reword (`PRN-06`)?
Can the phone's own language model do the rewording inside a Godot app (`TIM-02`, `TIM-03`, `PRE-05`, `PRE-08`, `PRE-17`, `PRE-37`, `PRE-39`, `PRE-41`)?

## What `PROJECT.md` asks

- **A director that only watches** (`TIM-02`, `TIM-03`):
  - it scores moments and signs;
  - it slows time and offers live moments within one budget: at most one about every 3 real minutes, and never more than a fifth of the time slowed;
  - it never causes, changes or hides anything.
- **Recognisers** that spot firsts, losses, new peoples, feuds and disasters, and name ages (`PRE-39`).
- **Every text built first from pattern sentences,** about 100 kinds of event with at least 5 phrasings each (`PRE-37`).
- **A writer AI on the phone that rewords those sentences one by one.**
  A check with no language model drops any sentence that adds, loses or changes a fact.
  Dark events never reach the writer (`PRE-41`, `PRE-17`).

## How others direct

- **Left 4 Dead's AI Director** (Michael Booth, Valve, 2009) aims at "dramatic game pacing", learned from Counter-Strike: "Constant, unchanging combat is fatiguing", "Long periods of inactivity are boring", and "Unpredictable peaks and valleys of intensity create a powerfully compelling and replayable experience".
  It tracks each player's "emotional intensity" and cycles through four states ([Valve: The AI Systems of Left 4 Dead](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf)):
  1. Build up.
  2. Sustain peak, "for 3-5 seconds after Survivor Intensity has peaked".
  3. Peak fade, until "a natural break in the action occurs".
  4. Relax, "for 30-45 seconds".
- **RimWorld's storytellers** pick events and their timing ([RimWorld wiki: AI storytellers](https://rimworldwiki.com/wiki/AI_Storytellers)):
  - Cassandra follows "a classic increasing curve of challenge and tension";
  - Phoebe "gives lots of time between disasters";
  - Randy "doesn't care if they make a story of triumph or utter hopelessness".

  They weigh "colony wealth … colonists count … how long it has been since the last major event".
- **Both create events.**
  Kindling's director may not (`TIM-03`).
  What we take from them is the rhythm: peaks, then a guaranteed rest.
  It is the same as `TIM-02`'s budget: one moment every few minutes at most, and a cap on time slowed.

## How others find stories

- **Overgenerate and curate:** James Ryan's thesis, *Curating Simulated Storyworlds* (2018), starts from the fact that simulations make far more events than stories.
  It shows ways to sift the stories out ([Emily Short on Ryan](https://emshort.blog/2019/05/28/curating-simulated-storyworlds-james-ryan-ch-6f/)).
- **Felt** is "a simple story sifting and simulation engine for emergent narrative play experiences".
  A sifting pattern is a query over the database of events, by type, actor, target and order.
  Its example, "violation of hospitality", finds a guest arriving, a host welcoming them, then the host harming them ([GitHub: Felt](https://github.com/mkremins/felt)).
- **Winnow** (Kreminski, Dickinson and Mateas, AIIDE 2021) is "a declarative domain-specific query language for story sifting" ([GitHub: Winnow](https://github.com/mkremins/winnow)).
  It adds "incremental sifting": "identifying the beginnings of compelling event sequences that haven't yet been completed".
  That is exactly `TIM-02`'s signs: time slows when a pattern is half matched (a predator stalking, two hostile groups in sight), never by looking ahead.
- **Text from grammars:** Tracery, by Kate Compton, is "a super-simple tool and language to generate text" from rules of symbols.
  It is used by "middle school students, humanities professors, indie game developers, professional bot makers" ([Tracery](https://tracery.io/)).
  Caves of Qud words its histories with such a "replacement grammar" (research 12).
  Our pattern sentences are a small grammar of this kind: each event's kind has several phrasings, picked by the event's seed and filled from its records.

## The writer on your phone

### Gemini Nano through ML Kit

- **What it is:** Gemini Nano runs in Android's AICore system service, "without needing a network connection or sending data to the cloud".
  AICore "doesn't store any record of input data or outputs" ([Android: Gemini Nano](https://developer.android.com/ai/gemini-nano)).
- **The Prompt API** sends "natural language requests on-device to Gemini Nano" ([ML Kit: Prompt API](https://developers.google.com/ml-kit/genai/prompt/android), [get started](https://developers.google.com/ml-kit/genai/prompt/android/get-started)):
  - It is now "offered in beta, and is not subject to any SLA or deprecation policy".
  - "Input must be under 4000 tokens (or approximately 3000 English words)".
  - It takes temperature, topK and maxOutputTokens, and a "seed" that "enables generating stable and deterministic results".
  - It "is not supported on devices with an unlocked bootloader".
- **The Rewriting API** offers fixed styles: "Elaborate, Emojify, Shorten, Friendly, Professional, Rephrase".
  "Input should be less than 256 tokens", in English and six other languages ([ML Kit: rewriting](https://developers.google.com/ml-kit/genai/rewriting/android)).
  "Rephrase" fits one pattern sentence at a time, but takes no voice instruction.
- **The limits that shape our design** ([ML Kit GenAI overview](https://developers.google.com/ml-kit/genai)):
  - "GenAI API inference is permitted only when the app is the top foreground application."
  - "Making too many GenAI API requests in a short period will result in an `ErrorCode.BUSY` response."
  - "`ErrorCode.PER_APP_BATTERY_USE_QUOTA_EXCEEDED` can be returned if an app exceeds a long-duration quota (e.g. daily quota)."
  - The feature APIs list the Google Pixel 9 to 11 series among supported phones.
    The Prompt API runs on the "nano-v2, nano-v3, and nano-v4" model versions.

### Other ways to run a model

- **NobodyWho** runs models through llama.cpp, for "Godot 4.5+", on Android from "Snapdragon 855+".
  It needs "roughly 2× the model file size in available RAM", and "models under 1 GB run smoothly on most phones" ([GitHub: NobodyWho](https://github.com/nobodywho-ooo/nobodywho)).
  Your phone's Tensor chip is not on its list, so it would need testing.
- **godot-llm**, also on llama.cpp, supports Android on the CPU only ([GitHub: godot-llm](https://github.com/Adriankhl/godot-llm)).
- **LiteRT-LM** is "Google's production-ready, high-performance, open-source inference framework for deploying Large Language Models on edge devices".
  It runs Gemma models on Android with GPU and NPU acceleration and powers on-device AI in Chrome and Pixel Watch ([GitHub: LiteRT-LM](https://github.com/google-ai-edge/LiteRT-LM)).
  A small Gemma through it is the fallback if Gemini Nano is ever unavailable.

### From Godot

- **Godot's Android plugins** (version 2) are Android libraries (AAR) that Godot loads at export.
  GDScript reaches them with `Engine.get_singleton("MyPlugin")`; methods marked `@UsedByGodot` are callable, and the plugin answers through signals with `emitSignal` ([Godot docs: Android plugins](https://docs.godotengine.org/en/stable/tutorials/platform/android/android_plugin.html)).
- **So the writer is a small Kotlin plugin:**
  - it takes one pattern sentence and the voice's instructions;
  - it calls ML Kit;
  - it returns the new sentence as a signal.

  The check itself is plain text logic, written once in the C++ library so the tests use the same code (`PRN-14`).

### Can it be trusted?

- **No.** Language models are "prone to hallucinate unintended text", and data-to-text writing is among the tasks most studied for it (Ji and others, ACM Computing Surveys 2022) ([arXiv](https://arxiv.org/abs/2202.03629)).
- **So `PRE-41`'s check stays strict and model-free:**
  - each sentence must keep its pattern's names, numbers, dates and places in order;
  - it must keep its marked words or their listed synonyms;
  - it may use no other word beyond a short list of joining words.

  It will reject many good sentences, and that is the right trade: a rejected sentence shows its pattern, never a wrong fact.

## Can Godot do it?

| Need | Verdict |
|---|---|
| The director and recognisers | Simulation-side C++ reading the event log (research 03); no engine part |
| Slowing time and live moments | Our own clock and Godot UI (research 14) |
| Pattern sentences | Our own grammar in the C++ library; text shown with Godot's labels |
| The writer | Feasible: a Kotlin plugin calling ML Kit's Prompt API, which is in beta and listed for Pixel phones |
| Writing overnight | Allowed only because overnight mode keeps the app in front (`TIM-12`); the daily battery quota may stop it, and pattern text then stands |
| Same text when reopened | The API's seed gives stable output, and history texts are stored once checked (`PRE-41`) |

## What we take

1. **The director keeps Left 4 Dead's rhythm without its power:** peaks, then a guaranteed rest, within `TIM-02`'s budget.
   It reads the world and sets only speed and live moments (`TIM-03`); a test runs a world with it on and off and compares the results.
2. **Recognisers are story-sifting patterns over the event log,** Felt-style.
   Half-matched patterns, Winnow-style, are the director's signs, so time slows before an outcome without looking ahead.
3. **Pattern sentences are a small Tracery-like grammar:** each kind of event has at least 5 phrasings, picked by the event's seed and filled from its records (`PRE-37`).
4. **The writer is Gemini Nano through ML Kit's Prompt API,** with a fixed seed, sentence by sentence, behind a small Android plugin.
   - The Rewriting API's "Rephrase" is a second option.
   - A small Gemma through LiteRT-LM is the fallback.
   - All are optional: pattern text always works.
5. **The check stays strict and model-free,** because hallucination is a known property of these models (`PRE-41`).
   Dark events never reach the writer (`PRE-17`).
6. **The writer respects the API's limits:**
   - it writes only while the app is in front, overnight mode included;
   - it queues requests and backs off on BUSY;
   - it stops for the day on the battery quota.
7. **Two prototypes before production:**
   - **The writer on your phone:** 100 pattern sentences and the 50 trap records through the Prompt API and the Rewriting API.
     It measures how many pass the check, how fast each sentence comes, and when the quotas bite in a night.
   - **The director on recorded worlds:** checks that its budget holds and that it catches every named discovery.

## Sources

- Directors:
  - [Valve: The AI Systems of Left 4 Dead](https://steamcdn-a.akamaihd.net/apps/valve/2009/ai_systems_of_l4d_mike_booth.pdf)
  - [RimWorld wiki: AI storytellers](https://rimworldwiki.com/wiki/AI_Storytellers)
- Story sifting and text:
  - [Emily Short on Ryan's thesis](https://emshort.blog/2019/05/28/curating-simulated-storyworlds-james-ryan-ch-6f/)
  - [GitHub: Felt](https://github.com/mkremins/felt)
  - [GitHub: Winnow](https://github.com/mkremins/winnow)
  - [Tracery](https://tracery.io/)
- The writer:
  - [Android: Gemini Nano](https://developer.android.com/ai/gemini-nano)
  - [ML Kit: Prompt API](https://developers.google.com/ml-kit/genai/prompt/android)
  - [ML Kit: Prompt API, get started](https://developers.google.com/ml-kit/genai/prompt/android/get-started)
  - [ML Kit: rewriting](https://developers.google.com/ml-kit/genai/rewriting/android)
  - [ML Kit GenAI overview](https://developers.google.com/ml-kit/genai)
  - [GitHub: NobodyWho](https://github.com/nobodywho-ooo/nobodywho)
  - [GitHub: godot-llm](https://github.com/Adriankhl/godot-llm)
  - [GitHub: LiteRT-LM](https://github.com/google-ai-edge/LiteRT-LM)
  - [Godot docs: Android plugins](https://docs.godotengine.org/en/stable/tutorials/platform/android/android_plugin.html)
  - [Ji et al.: hallucination survey](https://arxiv.org/abs/2202.03629)
