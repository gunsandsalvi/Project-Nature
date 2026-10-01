# Structure review of PROJECT.md

This review looks at how the file is built, not at the game design: whether IDs, statuses and links will hold up over years of AI agents building against the file while it grows.

**How it was checked.** I read `PROJECT.md` (1,445 lines) and `CLAUDE.md` in full. A script read every item and every reference, and I checked the git history.

**Starting point.** There are 322 IDs in 19 areas, with no gaps and no duplicates, and every referenced ID exists. That's a clean base. The problems below are about what happens next.

Line numbers refer to `PROJECT.md` at commit `e87455a` unless marked CLAUDE.md.

## At a glance

**High**
1. The coverage rule can't work as written
2. An item's meaning can change under the same ID, and nothing records it
3. Two agents can give two different items the same ID
4. The thing a task or test can link to is often too big
5. Almost no item says how to tell it is done

**Medium**

6. Status markers aren't uniform, and a missing one can make a decision by accident
7. No defined way to propose a change to a decided item, or to settle a "To test" one
8. The same fact is written in several places, and the copies already disagree
9. The build order is written in five places, in prose, and they already disagree
10. Hand-kept copies of the rules will drift, and CLAUDE.md already has
11. Some binding rules have no ID
12. Lists meant to grow without end have no home, and IDs stop at two digits
13. The file doesn't say which parts of an item are binding

**Low**

14. Hard to navigate on a phone
15. Dated statements will go stale, and "Dropped" has no format

---

## High

### 1. The coverage rule can't work as written

**Severity:** High

**Where:** IDs-and-links rule 4 (line 60), `PRC-12` (1329), `PRC-10` (1323–1325), `PRC-08` (1317), `PRN-09` (295)

**What's wrong:**
- **It's stated twice, differently.** Rule 4 says "Every ID that isn't *Dropped* must appear in at least one task." `PRC-12`'s check "lists any ID in this file that has no task", with no exception for Dropped items.
- **About 40 IDs are things no task delivers.** These are:
  - the vision texts `VIS-01`, `VIS-02`, `VIS-06`, `VIS-07`, `VIS-08`, `VIS-09`, `VIS-11` and `VIS-13`, the inspirations `VIS-04` and the name `VIS-16`;
  - the summary `SCP-13`;
  - the 13 non-goals (things *not* to build);
  - the 12 risks;
  - past-tense process items such as `PRC-05` ("This file was written with you one section at a time").
- **Delivery is staged.** `PRC-08` starts the plan with the first milestone, and `PRN-09` builds only what the next experiment needs. So for years, most IDs will rightly have no task.
- **Rule 4 also covers *Proposed* items.** Every new suggestion turns the check red until someone plans work for something you haven't approved.
- **The check isn't a gate.** `PRC-10` lists the checks before merging and before closing a milestone, and the coverage check is in neither.
- **It doesn't check code or tests.** Rule 5 promises tracing "from this file to the plan to the code, and back". `PRC-12` only compares the file with the plan, so nothing checks that IDs named in code and tests exist or aren't Dropped.

**Why it matters:** During `MIL-01`, the check would list about 300 IDs. A list that is always long gets ignored, or agents add token tasks ("`VIS-09`: keep in mind") to clear it. Either way, the file's main promise, that no feature gets lost (line 5), quietly stops working.

**Resolution (owner decisions):**
1. Decide which items need a task. I suggest three kinds, marked on each item:
   - *feature*: needs a task;
   - *rule*: needs a check, like the principles;
   - *context*: needs nothing (vision text, summaries, risks).
2. Measure coverage against milestones. The plan assigns every feature and rule to a milestone from the start, and tasks are required only for the current milestone.
3. State the rule once, in `PRC-12`. Exclude *Dropped* and *Proposed* explicitly, and make rule 4 point to it.
4. Add the coverage check to the milestone gate in `PRC-10`, and extend it to IDs named in code and tests.

### 2. An item's meaning can change under the same ID, and nothing records it

**Severity:** High

**Where:** IDs-and-links rules 1–5 (57–61), `PRC-07` (1313), `PRC-12` (1329); the git history of the file

**What's wrong:** IDs are permanent, but the text behind them can change with your OK. Nothing in the file marks that an item changed, and git doesn't reliably say which items changed or why:
- **Commit messages name topics, not IDs.** "Confirm round four of the proposals" (`3cbb0bd`) changed 41 lines and has no description at all. Rounds two and three list topics like "the pixel scale".
- **Searching git for an ID misses edits.** 22 past edits changed an item's text without touching the line that carries its ID:
  - searching git for the ID misses 3 of them outright;
  - 16 more turn up only because the same commit also edited the section 17 list, which named the ID, and that list is now empty.

  The worst case: commit `9807a5c`, titled "Expand section 13 (Platform and performance)", changed the principle `PRN-08` by adding "under the same version of the rules". `git log -G'PRN-08'` doesn't find it, and the commit message doesn't mention it.
- **The only reliable search is awkward.** It's `git log -L` from the item's ID to the next item's ID. IDs are out of order, so you must first look up which item comes next.
- **Long lines make diffs unreadable.** Many items are one long line (`VIS-06`, line 97, is 860 characters), so a one-word change shows as a whole paragraph replaced. That's unreadable on a phone.

**Why it matters:** A principle has already changed without trace (`PRN-08`, above). Or say you change `GOD-04` from "a blessing at most doubles a chance" to "at most triples":
- the tasks and tests linked to `GOD-04` still exist, so coverage stays green;
- the tests still check "doubles", and still pass;
- nothing tells anyone that the code behind `GOD-04` is now out of date.

Over years, this is exactly how code drifts from the file (`RSK-09`) without any check noticing.

**Resolution:**
1. **Name the changed IDs in every commit.** Each commit that changes `PROJECT.md` ends with a fixed line naming the changed IDs and a short reason, for example `Changed: GOD-04 (blessing cap raised to triple; owner OK)`. A merge check rejects file changes without it.
2. **Flag stale work.** The coverage tool lists tasks and tests linked to IDs that changed after the task was finished.
3. **Owner question: edit in place, or new ID?** One option is that if existing code would have to change, the old item is *Dropped* and a new ID is added. The other is to always edit in place and rely on point 2. Either works, but it must be one rule.
4. **Optional:** write one sentence per line inside items. The page looks the same, and diffs show only the sentence that changed.

### 3. Two agents can give two different items the same ID

**Severity:** High

**Where:** IDs-and-links rule 2 (58), `PRC-09` (1321), `PRC-10` (1323–1325); CLAUDE.md rule 3

**What's wrong:** "A new item takes the next free number in its area, wherever it sits in the text." But:
- agents work on separate branches (`PRC-09`);
- they are told to add proposals to this file (CLAUDE.md rule 3);
- two agents adding to the same area at the same time both take the same next number;
- if they insert their items in different subsections, git merges both without a conflict;
- none of the merge checks in `PRC-10` looks at this file.

**Why it matters:** Agent A adds `MND-26` *Teaching by showing* in 9.6. Agent B adds `MND-26` *Grief rites* in 9.3. Both merge. Every plan task, commit and test citing `MND-26` is now ambiguous, and rule 2 ("never renumbered") forbids the obvious fix.

**Resolution:**
1. Add a file check to the merge checks in `PRC-10`. It confirms that:
   - every ID is defined exactly once;
   - every reference points to an existing ID;
   - every status is one of the five words;
   - no live item points to a *Dropped* one.
2. Say that an ID becomes permanent only when it reaches the main version. A branch that loses the race renumbers its own new items before merging.

### 4. The thing a task or test can link to is often too big

**Severity:** High

**Where:** Item format (50); `PRE-07` (1117), `MAT-04` (685), `WLD-16` (622), `SCP-14` (333), `GOD-02` (421), `WLD-07` (568), `WLD-09` (577), `CUL-21` (1000), `PLT-04` (1205), `PRE-33` (1099)

**What's wrong:**
- **Many items bundle things built at different times.** For example:
  - `PRE-07` holds ten map overlays;
  - `MAT-04` holds about 40 laws that arrive over six matter layers (`MAT-16`);
  - `WLD-16` holds climate, daily weather and ice ages;
  - `SCP-14` holds three different starting points;
  - `GOD-02` holds three scales of power and seven kinds of disaster.
- **Coverage only asks whether an ID appears in *some* task.** A bundle counts as covered as soon as its first part is planned.
- **Single checks can't be cited.** The format says the plan and tests link to the *Done when* checks (line 50), but a single check has no ID. A test can cite `VIS-14`, not "the second check of `VIS-14`".
- **Some items are too thin to build or test alone:**
  - `PRE-04` only points to three other items;
  - `PRE-13` ("any further view the simulation's data supports") can never be finished;
  - `MND-15` is a pointer to `PLT-04`;
  - `MND-13` repeats part of `MND-06`.

**Why it matters:** The beliefs overlay ships at `MIL-05`, and its task names `PRE-07`. From then on, `PRE-07` looks covered. If the disease overlay is never built, no check notices.

**Resolution:**
- Approve a rule: "If two parts of an item can be delivered in different milestones, each part gets its own ID." Split the bundles above into new items, keeping the parent as an umbrella.
- For *Done when*, choose one:
  - give each check a short label (a, b, c) that tests can cite; or
  - change line 50 to say tests link to items, not to checks.
- Merge or drop the thin items.

### 5. Almost no item says how to tell it is done

**Severity:** High

**Where:** Item format (50–51); `VIS-14`, `VIS-15` and `PRE-31` (the only items with *Done when*); the principles and `GOD-05` (the only items with *Check*)

**What's wrong:**
- **Most items have no test.** Only 3 of 322 items have *Done when*, and 15 have *Check*. Leaving out the reality checks and Experiment 1's criteria, which are tests in themselves, about 280 items have no stated way to tell they're done.
- **Rules that always apply lack *Check* lines.** The format says *Check* is for "rules that always apply". Many such rules outside section 2 have none:
  - `TIM-03`: the director never causes events;
  - `GOD-07`: no trace in the story view;
  - `BIO-14`: every death has a cause;
  - `BIO-17`: no role assigned by sex;
  - `WLD-13`, `CUL-07`, and the non-goals.
- **Some checks can't be measured as written.**
  - `VIS-15` says "you'd choose to read a world's chronicle for pleasure" and "clearly different stories".
  - `PRN-07`'s check depends on a list of "discovery vocabulary" that the file gives only as "flake, knapping, fire-making, pottery and so on" (228).
- **Numbers say "about" with no tolerance.** Examples: "about 1,000 km" (`WLD-03`), "about three seconds" (`VIS-14`), "about 4 screen pixels" (`PRE-22`), "about 50 animal and 200 plant species" (`WLD-23`).
- **`VIS-14` and `PLT-04` point at each other.** `VIS-14`'s limits are "set from the measurements in `PLT-04`", and `PLT-04` takes "the targets, from `VIS-14`".

**Why it matters:** For most items, the agent writing the plan will decide what "done" means. That's a requirements decision made outside your approval, and you only find out at a milestone review. For example, `WLD-27` (soils) could be "done" with two numbers per patch of ground or with real soil formation. The file doesn't say which.

**Resolution:** First, decide where acceptance criteria live:
- in this file, as *Done when* lines that agents propose before the milestone that builds the item; or
- in the plan, approved by you when each milestone starts.

Because `PRN-09` deepens systems milestone by milestone, "done" will often mean "done for this milestone", and either home must allow for that. State the choice in "How this file works". Then:
- add *Check* lines to the always-apply rules, or list them under a principle's check;
- add one rule for "about" (for example, "within 10% unless stated");
- say plainly that `VIS-15`'s checks are your judgement at the milestone review.

---

## Medium

### 6. Status markers aren't uniform, and a missing one can make a decision by accident

**Severity:** Medium

**Where:** Lines 35–41 and 53; `MOM-01`–`MOM-12` (138–149), `MIL-01`–`MIL-07` (367–373), `RSK-01`–`RSK-12` (1337–1401), `TIM-02` (502), `RES-03` (1263), and 20 items marked "follows from"

**What's wrong:** 263 items have a plain `*(Decided)*` or `*(To test)*`. The other 59 don't:
- **31 have no marker.** These are the 12 signature moments and 7 milestones (they inherit their parent's status), and the 12 risks (which have a rating instead).
- **28 have extra words inside the marker:**
  - 20 say "follows from …";
  - 4 say "first layer" or "later layer" (`SND-01`–`SND-04`);
  - 3 say they're summaries or illustrations;
  - `RES-03` says "Decided as starting values; fixed before it runs".
- **Markers also appear below item level:**
  - `TIM-02` has `*(Decided)*` on a sub-heading (502);
  - line 50 allows a *(Proposed)* mark on a single check;
  - italic notes after field names look exactly like statuses: `VIS-14` "**Done when** *(exact limits set from …)*" (187), `TIM-01` "**The scale** *(exact values are tuned …)*" (485), and `BIO-04` "**Typical figures** *(from studies …)*" (816).
- **"Follows from" is undefined and used unevenly.** `BIO-09` says it follows from `PRN-05`; `BIO-10`, which follows from it just as much, doesn't say so. In `WLD-22` it points at "the systems above", which no tool can resolve.
- **Risks can't be *Proposed* or *Dropped* under these rules.** Yet until the last commit, `RSK-12` carried *(Proposed)* and the risks intro said "(the ratings are Proposed)". The rule was already being bent.

**Why it matters:** `RES-12` expects experiments to produce new signature moments. If an agent adds `MOM-13` under `VIS-12` and forgets the marker, line 53 makes it *Decided*, with no OK from you. And a tool needs half a dozen special cases just to read statuses.

**Resolution:**
- Every item, including moments, milestones and risks, starts with exactly one marker: one of the five words, nothing else inside.
- Move "follows from" to its own field, or drop it.
- Move "first/later layer" into the build order (finding 9).
- Make the risk rating a field.
- The file check in finding 3 rejects anything else.

Owner question: may agents update risk ratings in each milestone report without your approval, since ratings are assessments rather than decisions?

### 7. No defined way to propose a change to a decided item, or to settle a "To test" one

**Severity:** Medium

**Where:** Status table (37–39), line 50, `PRC-07` (1313), CLAUDE.md rule 3; `RES-03` (1263), `VIS-14` (187), `PLT-04` (1205–1212), `TIM-01` (485)

**What's wrong:**
- **Changes to decided items have no path.** A status belongs to a whole item, so an agent with a better idea for a *Decided* item can't set the item to *Proposed* (that un-decides it) and can't edit the text (that changes it). The file used to have a list of "Decided items with proposed details"; the last commit removed it. Only line 50 is left, which covers a *(Proposed)* mark on a single check.
- **"To test" has no exit.** Once something is measured, nothing says whether the item becomes *Decided*, who approves it, or where the measured value goes. `VIS-14`'s limits are "set from the measurements in `PLT-04`". Writing them into `VIS-14` changes a decided item; writing them into the plan leaves `VIS-14` without numbers.
- **`RES-03` has, in effect, a sixth status.** "Decided as starting values; fixed before it runs" means "decided, but may still change". That clashes with "no decided item changes without your OK".

**Why it matters:** This is how the file will change most often over the years. Say an agent finds that `WLD-11` (a world generated in under a minute on the phone) can't be met. Every way of recording that breaks a rule, so the agent will either stay silent or edit the decided text directly.

**Resolution:** Add to "How this file works":
- **A change proposal.** The decided text stays as it is. A line beneath it, `**Proposed change** *(Proposed)*: …`, holds the new text and the reason, and 17.2 lists the ID. Your OK replaces the text and removes the line.
- **A way out of "To test".** The measured result is written into the item, and the item becomes *Decided* when you approve it at the milestone review.

Owner question: is `RES-03` *Decided* (so changes need your OK) or *Proposed* until Experiment 1 runs?

### 8. The same fact is written in several places, and the copies already disagree

**Severity:** Medium

**Where:** `VIS-06` (97), `SCP-01` (329), `BIO-02` (775), `CUL-17` (978); `SCP-03` (356) and `RES-01` (1237); `PRN-08` (281) and `RES-05` (1257); `PRN-06`, `SCP-06`, `MND-01`, `PRE-17`; `VIS-14` (191), `PRE-34` (1097), `PLT-02` (1197); `VIS-13`, `SCP-13`

**What's wrong:**
- **One copy already contradicts the others.** `VIS-06` says the first people have "a handful of words"; `SCP-01`, `BIO-02` and `CUL-17` say "a few dozen".
- **`SCP-03` and `RES-01` are the same item twice.** They share a name ("Experiments lead"), a meaning, and a word-for-word *Why*.
- **`PRN-08` and `RES-05`** state reproducibility in nearly the same words.
- **"Language models never decide" has four IDs** (`PRN-06`, `SCP-06`, `MND-01`, `PRE-17`), and is repeated again in the glossary and CLAUDE.md rule 6.
- **The orientation rule appears three times.** "Every screen works one-handed in portrait and two-handed in landscape" is word for word in `VIS-14` and `PRE-34`, and restated in `PLT-02`.
- **Summaries are decided items too.** `VIS-13` and `SCP-13` summarise other sections, and `VIS-13` has the count in its name ("Seven differences").

**Why it matters:**
- When you change one copy, the others stay as they were.
- Every copy is *Decided*, so an agent who spots the clash can't fix it without you.
- Duplicate IDs split the links: half the tasks cite `SCP-03`, half cite `RES-01`, and neither looks fully covered.

**Resolution:**
- Give each fact one home. Other items point to it by ID, without repeating its numbers or limits.
- Add one line to "How this file works": "If a summary and its source disagree, the source wins."
- Drop one of `SCP-03`/`RES-01` and one of `PRN-08`/`RES-05` (your choice which), giving "see …" as the reason.
- Fix `VIS-06` now.

### 9. The build order is written in five places, in prose, and they already disagree

**Severity:** Medium

**Where:** `SCP-16` and `MIL-01`–`MIL-07` (365–373), `RES-07` (1271–1280), `MAT-16` (721–728), `SND-05` (1171), and the "first/later layer" notes on `SND-01`–`SND-04`

**What's wrong:**
- **Voices are in one list and missing from the other.** `SND-05` puts voices in `MIL-05` ("voices with words and beliefs"), but `MIL-05`'s contents don't mention voices.
- **Matter layers don't match milestones.** `MAT-16` says "Each milestone adds a layer". But `MIL-01`, `MIL-05` and `MIL-06` add no matter, and six named layers don't line up with seven milestones.
- **Milestone contents are prose, not IDs.** For example, `MIL-03` says "dreams, your first power". Which of the 322 items belong to which milestone is left for whoever writes the plan to work out.
- **Order rests on list numbers that will drift.** Milestone order comes from the list numbers, which today match the ID numbers. A milestone added between `MIL-03` and `MIL-04` would be `MIL-08` in fourth place.

**Why it matters:** The plan for `MIL-05` will be written from `MIL-05`'s text. Voices get left out, and `SND-05` is broken without anyone noticing. The coverage check can't catch it, because it knows nothing about milestones.

**Resolution (owner decision):** choose one.
- Each milestone lists the IDs it delivers, and `RES-07`, `MAT-16` and `SND-05` point to that list instead of restating it.
- Or the file keeps only each milestone's goal and "Now possible" line. The mapping of IDs to milestones lives only in the plan, and `MAT-16` and `SND-05` state only order ("soundscape, then voices, then music"), not milestones.

### 10. Hand-kept copies of the rules will drift, and CLAUDE.md already has

**Severity:** Medium

**Where:** Contents (7–27), area codes (63–85), section 17 (1403–1414), glossary (1416–1445), `PRC-06` (1311); CLAUDE.md rules 1, 3 and 5

**What's wrong:**
- **Several summaries are kept by hand, and nothing checks them:**
  - section 17.1 repeats the four *To test* items in different words;
  - 17.2 is a hand-kept list of proposals that agents must remember to update (CLAUDE.md rule 3), while each proposal is also marked in place;
  - the glossary restates 28 definitions, with no status;
  - the contents and the area-code table are kept by hand too.
- **CLAUDE.md already differs from the file:**
  - Rule 3 says "Never mark anything Decided", while `PRC-07` and `PRC-06` say "not without your OK". Every confirmation commit so far was an agent marking items *Decided* on your OK, which CLAUDE.md as written forbids.
  - Rule 5 says "don't rewrite what already works *without saying why*", but `PRN-14` says "never by rewriting what already works" (302). `PRN-14`'s own check allows explained rewrites, so the principle also disagrees with itself (see finding 13).
  - Rules 1 and 3 point to "section 2" and "section 17.2" by number.
- **`PRC-06` describes CLAUDE.md in a third wording.** And nothing says CLAUDE.md changes need your OK, even though every agent reads it first.

**Why it matters:** Any rule written twice will eventually say two different things. Agents then follow whichever copy they read first, and CLAUDE.md is always read first.

**Resolution:**
- Make section 17 and the contents generated by a tool, with start and end markers that agents don't edit by hand. Or remove them and let the tool print them on request.
- Have the glossary point to items instead of restating their rules.
- Rewrite CLAUDE.md to cite IDs ("Follow `PRC-07`") instead of rewording them, and to name sections instead of numbering them.

Owner question: should changes to CLAUDE.md need your OK, as `PRC-07` requires for this file?

### 11. Some binding rules have no ID

**Severity:** Medium

**Where:** "How this file works" (31–85); section intros at lines 211, 612, 732, 1010 and 1191; line 1214

**What's wrong:**
- **The file's own rules have no IDs and no status.** That covers the five ID rules, the status meanings, the item format and the nested-status rule. Parts are repeated in `PRC-04`, `PRC-07` and `PRC-12`, already with differences (finding 1).
- **Several section intros set requirements.** Examples:
  - "All of these are simulated in depth, and each feeds the others" (612);
  - "Each form of expression exists as a real thing in the world" (1010);
  - "every item is run again whenever anything changes" (732);
  - "Every milestone review goes through the principles…" (211).
- **Line 1214 is a loose sentence** between `PLT-04` and section 13.3. A simple parser would attach it to `PLT-04`.

**Why it matters:** A requirement without an ID can't be linked, covered or tracked. And CLAUDE.md protects "decided items" only. An agent "tidying" the ID rules, or adding a sixth status, isn't clearly changing anything decided.

**Resolution:**
- Give the file's own rules IDs in the `PRC` area, or declare "How this file works" *Decided* as a whole and covered by `PRC-07`.
- Turn each binding intro sentence into an item, or cut it down to a pointer to an existing item.
- Move line 1214 into `PLT-04`, or delete it, since it repeats `PRN-11`.

### 12. Lists meant to grow without end have no home, and IDs stop at two digits

**Severity:** Medium

**Where:** `MAT-13` (712), `MAT-15` (719), section 7.5 intro (732), `PRN-14` (302), `RES-03` (1263), `RES-07` (1271), `RES-09` (1243); rule 1 (57)

**What's wrong:**
- **Reality checks have no defined home.** Every new ingredient, structure or law brings its own reality checks (`MAT-13`, `MAT-15`), and the checklist "grows with each layer" (732).
  - If each new check must become an `RCK` item, each needs your approval, and adding tin ore stops being "easy and effortless" (`PRN-14`).
  - If not, the checks live outside the file, the coverage check can't see them, and `RCK` becomes a frozen sample.
- **Later experiments' criteria have no home.** Experiment 1's pass criteria are in the file (`RES-03`). Later experiments (`RES-07`) must have criteria fixed before they run (`RES-09`), but nothing says where those criteria go.
- **IDs stop at two digits.** Every ID has two, `RCK` is designed to keep growing, and nothing says what comes after `RCK-99`.
- **New areas have no rule.** Nothing says how to add a new area code, or where its section goes.

**Why it matters:** These lists are where the file will grow fastest. Without a rule, each agent will choose differently, and checks will end up split between the file and the code.

**Resolution (owner decision):**
- Reality checks: the file keeps the ones that define what the world must do (the current 20, plus any you add). The matter catalogues hold the rest, and each catalogue check names the `MAT` or `RCK` item it supports.
- Each later experiment gets its criteria as a new `RES` item, proposed before it runs.
- State that numbers go to three digits after 99 (`RCK-100`).
- Add one sentence on adding new areas.

### 13. The file doesn't say which parts of an item are binding

**Severity:** Medium

**Where:** Item format (43–51); for example `GOD-04` (448), `MAT-09` (667), `WLD-14` (616), `VIS-11` (126), `PRN-14` (301–305)

**What's wrong:**
- **The format names five fields, but the file uses some 25 other labels.** Examples are *In practice*, *What follows*, *How strong*, *Not included*, *Safeguards*, *Typical figures*, *What doesn't*, and *Risk*, *Signs*, *Response*. Nothing says which of them are requirements.
- **Firm limits sit in these fields:**
  - `GOD-04`'s "a blessing at most doubles a chance" is under *How strong*;
  - `MAT-09`'s *Example* says smelting "yields exactly the copper that was in it";
  - `WLD-14`'s *Example* says where flint, obsidian, copper, clay and salt are found.
- **One label implies the rest are binding.** `VIS-11` is marked "an illustration, not a script", which suggests that unmarked examples might be binding.
- **When fields disagree, nothing says which wins.** `PRN-14`'s *What* says "never by rewriting what already works", but its *Check* accepts a rewrite if the report "explains why it had to be".

**Why it matters:** Agents will disagree about whether examples are requirements. One puts flint only in chalk; another treats the example as flavour. The same item gets built two ways in two places.

**Resolution:**
- Add one line to "How this file works", for your approval: "*What*, *Done when* and *Check* are binding. *Why* and *Example* only explain. Any other label counts as part of *What*."
- Then settle `PRN-14`: is it "never", or "only with a stated reason"?

---

## Low

### 14. Hard to navigate on a phone

**Severity:** Low

**Where:** The whole file; rule 2 (58)

**What's wrong:**
- **No reference is a tappable link.** There are 336 ID references and 31 "section N" references, so following any of them means a text search.
- **IDs are out of order in 16 of 19 areas,** as rule 2 allows. `MND`, for example, runs 1, 2, 17, 3, 4, 5, 18, 8, 7…, so an ID's number doesn't tell you where it is.
- **Five pairs of items share a name:**
  - "Experiments lead": `SCP-03`, `RES-01`;
  - "Dreams": `GOD-03`, `MND-12`;
  - "Curiosity": `VIS-08`, `MND-10`;
  - "Their sky and calendar": `CUL-13`, `PRE-11`;
  - "Their maps and names": `CUL-14`, `PRE-12`.
- **It's long.** The file runs to 1,445 lines, and items up to 860 characters on one line are hard to scan.
- **"Section N" references break silently** if a section is ever inserted. Add one after section 7, and the five references to "section 9" (Minds) would point at the wrong section.

**Why it matters:** You read on a phone, so every jump is slow. A renumbered section sends agents to the wrong place without any error.

**Resolution:**
- Keep the source plain, but let a tool produce a phone copy with tappable IDs and an ID index. It should be generated, never kept by hand.
- Refer to sections by name or area code, not by number.
- Rename the same-name pairs, for example "Their sky (simulated)" and "Their sky (view)". Renaming doesn't change IDs.

### 15. Dated statements will go stale, and "Dropped" has no format

**Severity:** Low

**Where:** `PRC-05` (1315), `PRC-08` (1317), section 17.2 (1414), line 60, CLAUDE.md line 3, status table (37), rule 3 (59)

**What's wrong:**
- **Several decided lines describe a moment in time.** They amount to the kind of change log you asked not to have:
  - `PRC-05`: "This file was written with you one section at a time";
  - `PRC-08`: "Next: the implementation plan";
  - 17.2: "Every proposal made while this file was being written has been reviewed";
  - "still to come", for the plan, at line 60 and in CLAUDE.md;
  - "*Decided*: Agreed in our sessions".
- **"Dropped" has no format.** Rule 3 says a cut item becomes *Dropped* "with a one-line reason", but not where the reason goes. Expect yet another marker variant.
- **Nothing covers items that cite a dropped one.** `PRE-14`, for example, is cited seven times.

**Why it matters:** The day the plan exists, `PRC-08` and both "still to come" lines become wrong, and fixing `PRC-08` needs your approval. Decided items that depend on a dropped item will go on promising it.

**Resolution:**
- Turn time-bound items into lasting rules, or drop them once they're true (`PRC-05`, `PRC-08`).
- Cut 17.2's empty-state text to just "None."
- Define the format as `*(Dropped)*` plus a `**Dropped because:**` line.
- Add "no live item cites a dropped one" to the file check in finding 3.

---

## What works and should be kept

- **Permanent IDs.** There's one flat scheme (area code plus number), and IDs are never reused, renumbered or deleted. Today there are 322, with no gaps or duplicates, and every reference resolves. The fixes above add checks around this scheme; they don't replace it.
- **One place for IDs.** Every ID is defined only at the start of a list item, as `` `ABC-NN` **Name** ``, and never in a table or heading. One simple pattern finds all 322. Keep it that way.
- **The *what*/*how* split.** There's no technology or architecture in this file, which is what lets it outlive any technical choice.
- **Principles first, each with a *Check* line.** This is the clearest example of a verifiable rule in the file. Copy the pattern to other always-apply rules; don't change it.
- **The reality checklist's item size.** Each `RCK` item is one testable fact in plain words (`RCK-01` to `RCK-20`). That's the size other items should aim for. Keeping *To test* separate from *Decided* is also right, as long as finding 7 gives it a way out.
