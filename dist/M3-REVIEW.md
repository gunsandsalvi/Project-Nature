M3 independent review — T3.13e.4 — 10 October 2026
Reviewed 6088c720..9291b8d1. No source, documentation or Git changes.

Verdict: NOT CLOSE under the current acceptance. This is a working discovery slice with substantial passing mechanical evidence, and the builder honestly leaves M3 unaccepted. Spread fails, additional principle breaches and save-reader holes need repair, and phone/owner acceptance remains open. Closing a partial milestone instead requires an explicit owner decision preserving these failures.

Findings, most severe first

1. Spread fails, with a small counting overstatement. build/m3-close/learning confirms discovery 18/20 and the standalone holder threshold 6/20; fireless evidence confirms 4/40. PROJECT.md:2516 requires spread “in those” timely-discovery runs. Seed 1018 reaches 19 holders but discovers after day 123, beyond two 60-day years: only 5/20 satisfy both conditions. Both readings fail. Fix before closure; retain failed seeds and unchanged thresholds. Correct the joint-gate count in the report.

2. Choices use unseen facts: PRN-01's “scoped pass” is false. sim/src/kd/demo/fire.cpp:455–478 reads physical burn/fuel properties while merely checking that some familiarity exists. My two otherwise identical probes had no learned burn property; changing hidden burn from 0 to 3 changed whether fuel was chosen. Fix now: use perceived evidence, including uncertainty. Teaching also needs a clear learner-acceptance boundary: teaching.cpp:40–45, 184–205 examines another person's exact needs, private meal/allocation and session state without visible evidence or a recorded needs reply (MND-23). The existing truthful knowledge exchange does not supply that information.

3. PRN-07 failure extends beyond named bank/carry recipes. craft_work.cpp:198 and fire_food.cpp:17 hard-code roots/meat. In a temporary in-memory catalogue, an identical food named review_roots could not prepare a roast lesson; ordinary roots could. Thus made-up fitting materials do not work in every fit. Fix now across selection and thermal processing, rather than only removing the two reported blueprint lookups.

4. Strict saves accept inconsistent/orphaned records. With valid checksums, the own-format reader accepted (a) a hearth whose Item.owner is a person but Fire.owner is zero, and (b) a pending idea addressed to a never-allocated future person ID. fire_store.cpp:90 checks ownership agreement only when Fire.owner is nonzero; world.cpp:1515/1649 checks the dream target's family and validates its memory only if the target exists. These are not legitimate ended-person records. Fix now with cross-record identity/ownership validation and rejection regressions. Successful gate reopens do not establish corruption rejection. Reproductions: /tmp/m3-review-probe.cpp and .log.

5. Kept reasons are missing for built choices, not just future full-scale minds. Fire tending/warming calculate scores without storing reasons (fire.cpp:438–495; fire_thermal.cpp:194). Craft reasons are overwritten at the next choice (craft_work.cpp:677); HIST1 Result has no reasons (world/knowledge.hpp:262). words.gd:183–193 substitutes generic explanations. Linked factual History works, but PRN-13 cannot explain these saved choices, and current fire details can retain unrelated earlier craft reasons. Fix now or obtain an explicit owner disposition; do not describe this solely as a later-world obligation.

6. Fire behaviour acceptance lacks an autonomous gate. I found no 20-run chain/control result satisfying RES-23 (PROJECT.md:2506). The 200-trial friction/cooking/banking tests inject supplied work; the cold-hearth test proves its starting state. The idea scene also installs a 100-second twirl to establish its declared prerequisite memory (proof/idea_cases.cpp:78). That is legitimate conditional setup, but does not prove people independently discover and complete the fire chain. Run the required scene before closure, or have the owner explicitly defer it.

Acceptance judgment

- Ordinary finite flake production/use: MET mechanically; personal learning/observation/teaching/fading: PARTLY MET, population spread FAILED.
- Discovery frequency, two routes and unflakable control: MET on recorded closing evidence.
- Finite fire, warmth, cooking/burning and low/high trials: MET mechanically; generic/perceived choices FAILED; autonomous chain PARTLY MET.
- Memory-based idea attempts, paired controls, shared caps/privacy and three reopen phases: MET for the stated conditional scenes, not ordinary-world frequency.
- Format 5 refusal of older formats/no conversion: MET. Own-format roundtrips/continuation/replay: MET in exercised tests; strict malformed-record rejection FAILED.
- History links/Back, layout and bounded action sounds: MET in cloud tests; saved choice explanations PARTLY MET.
- Cloud routine/audit: recorded PASS, not gameplay acceptance. Phone/cloud comparison, frame deadlines, heat/battery, audible quality and unaided owner play: NOT MET yet. Check APK 41305 verifies and is 27.930 MiB; release update remains blocked as disclosed.

Builder reports confirmed

Parsed the closing and earlier fireless/idea outputs. No evidence of seed shopping, forced discovery, numerical relaxation or retroactive passing judgment. Eight cancelled fresh learning results are excluded honestly; even twenty additional successes cannot rescue the failed aggregate. Independently reran seed 1001: exact digest, 17 holders, 24.47 seconds (about 2.6 game years/minute), 77.6 MiB. Idea 2001 matches all three retained digests and controls. All 301 native cases/1,768,842 assertions and 96 headless app cases pass; three affected view cases pass. Index invalidation/stable ordering showed no proven digest defect.

The display-trail repair b7153b27 is real: recovery regression passes; my full app run peaks at 368 MiB. The deliberate native history-window stress still uses about 3.13 GiB. Spent-item growth is real and disclosed: day 45 has 25,895 items, 22,541 spent. Indexing reduces scans, not retained history/save growth. Schedule measured compaction/storage work for M4; carrying the unmet tens-of-years/minute target requires an owner decision.

Document budgets pass (333/5741/2576/1593 words). PROJECT's decided meanings were not changed in these commits. However, the same spent-item paragraph and status story recur across Architecture, Implementation and delivery/check records despite CLAUDE's no-duplication rule. Deduplicate in M4 around one evidence source.

Could not check

No physical phone, release passphrase, audible listening or owner play judgment. Did not repeat the entire cross-compiler/TSan audit or every long seed; checked retained logs and reran representative seeds. The recorded 100-kill test uses the foundation crowd, not the M3 living/fire/lesson world; focused M3 command-cut and continuation tests pass, but random process-kill coverage of the complete M3 camp remains unproved. Long-game memory/storage beyond the recorded run is unmeasured.
