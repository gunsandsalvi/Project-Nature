# Kindling: working guide

Read this guide, PROJECT.md's working brief/principles and the current milestone; then the IDs and architecture sections the work touches.
PROJECT.md is the decision source. Its full catalogue is reference material, not a daily reading assignment.

**Now — owner approved 9 October 2026:** M1 is accepted and M2 is delivered (its play answers are still open). M3 proves discovery and learning on the existing C++ simulation and 2D renderer: α3.13a is delivered as 41301; continue with α3.13b. Start from IMPLEMENTATION.md's "Where M3 stands"; no new foundation or renderer.

**Roles:** one builder owns each playable increment. At most one agent runs at any time, and every agent uses the same model (owner, 8 October 2026), so reviews and supporting tasks run only while the builder is stopped. The coordinator owns integration and may resolve implementation choices within approved scope. One independent reviewer checks each milestone. Follow the session's git permissions.

**Documents and size budgets:**

| Document | Purpose | Budget |
|---|---|---|
| This guide | Daily entry point and rules | 400 words |
| PROJECT.md | Decisions, features and acceptance | Working overview ≤1,000 words; full reference shrinks only without losing decisions |
| ARCHITECTURE.md | Built contracts, near-term design, deferred obligations | Aim ≤6,000 words |
| IMPLEMENTATION.md | Next milestone's tasks; later goals and ID map | Aim ≤3,500 words |
| CRITIQUE.md | Evidence and scope recommendations | Aim ≤1,800 words |

Keep short evidence records only when they explain a durable decision. Remove superseded handovers after moving facts; do not maintain duplicate status stories.

**Rules:**

- Follow the principles; simulation decisions stay independent of pictures and generated prose.
- Preserve determinism, each build's own saves and meaningful regressions. Do not convert older saves: a build refuses saves from earlier builds with a plain message (owner, 9 October 2026). Run touched checks during work and the routine check before joining/delivery. The full audit runs only at the end of a milestone (owner, 9 October 2026).
- Self-review each integrated delivery once. Independent review closes each milestone and happens before joining for changes to determinism, threading, save formats or corruption recovery; in M3 only the end-of-milestone review runs (owner, 9 October 2026). Reproduce bugs and use fault injection where risk or uncertain test sensitivity warrants it.
- Keep stable IDs and planned task links. IDs in modules, test suites and ordinary commits are optional navigation. Changes to decided meaning require owner approval and date with the decision (`PRC-07`), not a prescribed commit trailer.
- Partial work retains its ID; notes state proved behaviour and remaining acceptance (`PRC-12`). ID comments are not completion evidence.
- Ship a playable build and short note. Only stage reviews wait for the owner. Readable stand-ins are accepted for early gameplay builds.
- Only the owner changes decided meaning. The camp-first decisions and testing-policy changes are approved, including this guide (owner OK 8 October 2026).
- Write plainly. The owner reads everything on a phone, so give every command a short plain description of what it does.

Changes to this guide need the owner's OK.
