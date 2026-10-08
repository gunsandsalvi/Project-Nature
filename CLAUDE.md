# Kindling: working guide

Read this guide, PROJECT.md's working brief/principles and the current milestone; then the IDs and architecture sections the work touches.
PROJECT.md is the decision source. Its full catalogue is reference material, not a daily reading assignment.

**Now — owner approved 8 October 2026:** M1 stays accepted. M2 builds a saved living camp and a place dream on the existing C++ simulation and 2D renderer. M3 proves discovery and learning. Start with the baseline repairs in A17.0 and α2.13a; no new foundation or renderer.

**Roles:** one builder owns each playable increment. At most one supporting task runs alongside it. The coordinator owns integration and may resolve implementation choices within approved scope. One independent reviewer checks each milestone. Follow the session's git permissions.

**Documents and size budgets:**

| Document | Purpose | Budget |
|---|---|---|
| This guide | Daily entry point and rules | 400 words |
| PROJECT.md | Decisions, features and acceptance | Working overview ≤1,000 words; full reference shrinks only without losing decisions |
| ARCHITECTURE.md | Built contracts, near-term design, deferred obligations | Aim ≤6,000 words |
| IMPLEMENTATION.md | Next milestone's tasks; later goals and ID map | Aim ≤3,500 words |
| CRITIQUE.md | Current evidence and undecided process recommendations | Aim ≤1,800 words |

Keep short evidence records only when they explain a durable decision. Remove superseded handovers after moving facts; do not maintain duplicate status stories.

**Rules:**

- Follow the principles; simulation decisions stay independent of pictures and generated prose.
- Preserve saves, determinism and meaningful regressions. Run touched checks during work, the routine check before joining/delivery, and the audit when PRC-10 requires it.
- Review each step and completed alpha as PRC-09 requires; independent review closes the milestone. Every new test must be made to fail once on its relevant fault under the current rule.
- Keep requirement IDs in implementing code/tests and commit messages. PROJECT changes need their changed IDs, reasons and owner approval in the commit (`PRC-07`).
- Partial work retains its ID; notes state proved behaviour and remaining acceptance (`PRC-12`). ID comments are not completion evidence.
- Ship a playable build and short note. Only stage reviews wait for the owner. Readable stand-ins are accepted for early gameplay builds.
- Only the owner changes decided meaning. The 22 first-review proposals are approved; CRITIQUE.md's later testing simplifications are not. Do not apply the separate testing patch without approval.
- Write plainly. The owner reads everything on a phone, so give every command a short plain description of what it does.

Changes to this guide need the owner's OK.
