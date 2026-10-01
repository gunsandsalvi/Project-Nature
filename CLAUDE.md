# Kindling: guide for AI agents

Read `PROJECT.md` before doing anything else. It is the source of truth for what Kindling is and must do. The finished project has two more documents, both still to come: the architecture says how it is built, and the implementation plan says in what order. Code and tests link back to them by ID (`PRC-04`). Until the architecture exists, the `pretests` folder holds the list of building blocks (`pretests/BUILDING-BLOCKS.md`) and the throwaway tests that settle how each is best built (`PRC-08`). The whole folder is deleted once the architecture is written.

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
