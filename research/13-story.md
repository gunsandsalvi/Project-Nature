# Research 09: the story director, the book of ages and the writer

**Question:** how can the game find the interesting moments in a simulation, show them to you, and write them up as a readable history (`TIM-02`, `PRE-05`, `PRE-08`, `PRE-37`, `PRE-41`), when nothing may happen for the sake of a story (`PRN-12`) and language models may only reword (`PRN-06`)?

## How others do it

- **RimWorld's storytellers** (Cassandra, Phoebe, Randy), after Left 4 Dead's AI director, read your situation and decide which event to send next to make the best story ([RimWorld wiki](https://rimworldwiki.com/wiki/About_RimWorld)).
  This is exactly what our principles forbid: our world's events come only from its rules.
- **Story sifting** finds stories in a simulation instead of making them.
  - Felt defines patterns of events that make a story, and finds them in the simulation's history ([Felt](https://github.com/mkremins/felt)).
  - Winnow matches such patterns incrementally, so it can spot a story while it is still unfolding, in games like Dwarf Fortress or The Sims ([AIIDE paper](https://ojs.aaai.org/index.php/AIIDE/article/view/18903), [GitHub](https://github.com/mkremins/winnow)).
  - A drama manager built on it, Shepherd, picks what to show from what the sifter finds ([AIIDE](https://ojs.aaai.org/index.php/AIIDE/article/view/31887)).
- **Replacement grammars.**
  Tracery, by Kate Compton, fills templates from rules of symbols, for varied text from data ([ICCC 2018](https://computationalcreativity.net/iccc2018/node/10)).
  Caves of Qud words its histories this way (research 08).
- **The phone's own language model.**
  Android's ML Kit Prompt API runs Gemini Nano on the device, offline, through the AICore system service.
  It performs best on the Pixel 10 series and later, which includes your phone's family ([Android Developers](https://developer.android.com/blog/posts/ml-kit-s-prompt-api-unlock-custom-on-device-gemini-nano-experiences), [InfoQ](https://www.infoq.com/news/2025/11/android-genai-prompt-api)).
  It is in alpha, so it may change.

## What we take

1. **Our director is a story sifter, not a storyteller.**
   It watches the world's event log with Winnow-style patterns: a first flake, a death in the cold, a feud starting.
   It decides only what you see: where a live moment points the camera, when to slow time, what the book of ages records.
   It never adds or changes an event (`PRN-12`), and the same world runs the same with it on or off (`TIM-03`).
2. **The book of ages is written from the event log by pattern sentences**, Tracery-style grammars over each entry's facts.
   So every sentence traces back to recorded events (`PRN-10`, `PRE-37`).
3. **The writer only rewords:**
   - Gemini Nano on the phone, through the Prompt API, reached by a small Android plug-in for Godot;
   - each rewording is checked against its pattern sentence, and dropped if it adds, loses or changes a fact (`PRE-41`, `PRN-06`);
   - without the model, or if the API changes, the pattern sentences stand on their own.
