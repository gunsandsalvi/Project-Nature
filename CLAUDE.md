# Kindling: guide for AI agents

Read `PROJECT.md` before doing anything else. It is the source of truth for what Kindling is and must do. `ARCHITECTURE.md` says how it is built, and `IMPLEMENTATION.md` says in what order: it lists the playable alphas, and its "How to use this plan" says how to build the next one. Code and tests link back to them by ID (`PRC-04`, `PRC-12`) and by architecture section (`A6.3`). The pre-tests that settled how each building block is best built (`PRC-08`) are recorded in `ARCHITECTURE.md` (A1.4); their code was removed from the working tree and stays in git history (A2.9).

## Rules

1. **Follow the principles** in the Principles section of `PROJECT.md` (`PRN-16`). If a task seems to require breaking one, stop and raise it with the owner instead of working around it.
2. **Link all work to IDs** such as `WLD-03`: in plan tasks, commit messages, tests, and code that implements an item.
3. **Nothing becomes Decided, and no decided item changes, without the owner's OK** (`PRC-07`). Suggest additions and changes as set out under "Changing this file" in `PROJECT.md`, list them under "Proposals awaiting confirmation", and raise them in the next report.
4. **Name the changed IDs in every commit that changes `PROJECT.md`**, with the reason, for example `Changed: GOD-04 (blessing cap raised; owner OK)`.
5. **Keep `PROJECT.md` free of implementation details** (`PRC-04`). They belong in the architecture and the implementation plan.
6. **Build modularly** (`PRN-14`).
7. **Language models describe, never decide** (`PRN-06`).
8. **Before work joins the main version**, the checks in `PRC-10` must pass and an independent AI review must approve it (`PRC-09`).
9. **Write plainly.** The owner reads everything on a phone.

Changes to this guide need the owner's OK.
