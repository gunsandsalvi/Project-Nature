# Kindling: guide for AI agents

Read `PROJECT.md` before doing anything else. It is the source of truth for what Kindling is and must do. `ARCHITECTURE.md` says how it is built, and `IMPLEMENTATION.md` says in what order. Code and tests link back to `PROJECT.md` by ID (`PRC-04`, `PRC-12`).

**Now (5 October 2026):** pre-production is closed, as the owner asked. Its answers are decisions in `ARCHITECTURE.md`, and its evidence, numbers and lessons are in `LESSONS.md`; its code is deleted, and git keeps it. Production has begun with M1, the foundations (`IMPLEMENTATION.md`), and writes all its code fresh: never copy or port prototype code from git history.

## Rules

1. **Follow the principles** in the Principles section of `PROJECT.md` (`PRN-16`). If a task seems to require breaking one, stop and raise it with the owner instead of working around it.
2. **Link all work to IDs** such as `WLD-03`: in commit messages, tests, and code that implements an item.
3. **The code is the index.** Code names the items it implements (`/// Implements PRE-20`) and each test the items it checks (`// checks: PRE-20`), so `python3 tools/filecheck.py where PRE-20` shows where anything is done. No document keeps a list of where things are built.
4. **Write each thing once.** Never a second copy of the same logic, such as Rust copies of shader maths.
5. **Nothing becomes Decided, and no decided item changes, without the owner's OK** (`PRC-07`). Suggest additions and changes as set out under "Changing this file" in `PROJECT.md`, list them under "Proposals awaiting confirmation", and raise them in the next report.
6. **Name the changed IDs in every commit that changes `PROJECT.md`**, with the reason, for example `Changed: GOD-04 (blessing cap raised; owner OK)`.
7. **Keep `PROJECT.md` free of implementation details** (`PRC-04`). They belong in the architecture and the implementation plan.
8. **Build modularly** (`PRN-14`).
9. **Language models describe, never decide** (`PRN-06`).
10. **Before work joins the main version**, `tools/check.sh` must pass (`PRC-10`) and its review must approve it (`PRC-09`): the builder reviews each lettered alpha itself, and one independent subagent verifies each numbered alpha once, at its last lettered step. The pull request says so; nothing else is recorded.
11. **Write plainly.** The owner reads everything on a phone, so give every command a short plain description of what it does.

Changes to this guide need the owner's OK.
