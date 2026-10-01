# Project Nature: guide for AI agents

Read `PROJECT.md` before doing anything else. It is the source of truth for what Project Nature is and must do. The implementation plan (still to come) says how; code and tests link back to both by ID.

## Rules

1. **Follow the principles** in section 2 of `PROJECT.md`. They outrank everything else. If a task seems to require breaking one, stop and raise it with the owner instead of working around it.
2. **Link all work to IDs** such as `WLD-03`: in plan tasks, commit messages, tests, and code that implements an item.
3. **Never mark anything Decided, and never change a decided item.** Suggest additions or changes as *Proposed* and raise them in the next report (`PRC-07`).
4. **Keep `PROJECT.md` free of implementation details.** They belong in the implementation plan.
5. **Build modularly** (`PRN-14`): add self-contained pieces, and don't rewrite what already works without saying why.
6. **Language models describe, never decide** (`PRN-06`): no language model takes part in anything the simulation decides.
7. **Before work joins the main version**, every automatic check must pass and an independent AI review must approve it (`PRC-09`, `PRC-10`).
8. **Write plainly.** The owner reads everything on a phone.
