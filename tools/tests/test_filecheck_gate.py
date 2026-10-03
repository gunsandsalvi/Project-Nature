"""The merge gate and the note check (tools/filecheck.py)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import filecheck  # noqa: E402

HEAD = "0123456789abcdef0123456789abcdef01234567"
BUILDER = ["https://claude.ai/code/session_01Builder"]


PLAN = """| Alpha | Title | Stage | Hours | Status |
|---|---|---|---|---|
| α00 | Skeleton | 1 | 1 | done |
| α00b | Checks | 1 | 1 | done |
| α02a | The island | 1 | 4.5 | Not started |
| α02b | The ground up close | 1 | 5 | Not started |
| α02c | Cliffs and caves | 1 | 5 | Not started |
"""


def gate(desc, head_passed=True, parent_passed=False, only_results=False, alpha="α02c"):
    """The gate on `desc` as the description of `alpha`, which by default ends its number."""
    return filecheck.gate(f"{alpha}, built here.\n\n{desc}", HEAD, only_results, head_passed, parent_passed, BUILDER,
                          PLAN)


class GateTest(unittest.TestCase):
    # checks: PRC-09 PRC-11
    def test_good_description_passes(self):
        self.assertEqual(gate("Facts.\n\nReview: APPROVE 0123456789ab session_01Reviewer\n"), [])

    # checks: PRC-09
    def test_results_only_head_uses_parent_checks(self):
        desc = "Review: APPROVE 0123456789ab session_01Reviewer"
        self.assertEqual(gate(desc, head_passed=False, parent_passed=True, only_results=True), [])
        self.assertNotEqual(gate(desc, head_passed=False, parent_passed=True, only_results=False), [])

    # checks: PRC-09
    def test_missing_approval_fails(self):
        self.assertNotEqual(gate("Facts only.\nReview: CHANGES see findings\n"), [])

    # checks: PRC-09
    def test_stale_commit_fails(self):
        self.assertNotEqual(gate("Review: APPROVE fedcba987654 session_01Reviewer"), [])

    # checks: PRC-09
    def test_builders_own_session_fails(self):
        self.assertNotEqual(gate("Review: APPROVE 0123456789ab https://claude.ai/code/session_01Builder"), [])

    # checks: PRC-09
    def test_subagent_reviewer_passes(self):
        self.assertEqual(gate("Review: APPROVE 0123456789ab subagent:a01a-review-2"), [])

    # checks: PRC-09
    def test_unnamed_reviewer_fails(self):
        # the whole name must match: no underscore, and no session riding behind a subagent's label
        for who in ("someone", "subagent:", "subagent:Bad_Label", "agent:a01a-review", "subagent:a01a_x",
                    "subagent:x/session_01Builder"):
            self.assertNotEqual(gate(f"Review: APPROVE 0123456789ab {who}"), [], who)

    # checks: PRC-09 PRC-10
    def test_unchecked_head_fails(self):
        self.assertNotEqual(gate("Review: APPROVE 0123456789ab session_01Reviewer", head_passed=False), [])

    # checks: PRC-09
    def test_builder_reviews_the_lettered_steps_and_a_subagent_the_number_end(self):
        # the owner's rule of 3 October 2026: the builder approves an alpha a later letter of its number follows,
        # and only the alpha ending its number gets a subagent's (or another session's) review
        own = "Review: APPROVE 0123456789ab builder"
        sub = "Review: APPROVE 0123456789ab subagent:a02-review"
        self.assertEqual(gate(own, alpha="α02a"), [])
        self.assertEqual(gate(own, alpha="α02b"), [])
        self.assertEqual(gate(sub, alpha="α02c"), [])
        self.assertEqual(gate("Review: APPROVE 0123456789ab session_01Reviewer", alpha="α00b"), [])
        self.assertEqual(gate(own, alpha="α00"), [])
        self.assertNotEqual(gate(own, alpha="α02c"), [], "the builder may not approve the number's end")
        self.assertNotEqual(gate(own, alpha="α00b"), [], "the builder may not approve the number's end")
        self.assertNotEqual(gate(sub, alpha="α02a"), [], "a subagent reviews only the number's end")
        self.assertNotEqual(gate("Review: APPROVE 0123456789ab Builder", alpha="α02a"), [])

    # checks: PRC-09
    def test_description_names_its_alpha(self):
        desc = "Facts.\n\nReview: APPROVE 0123456789ab subagent:a02-review"
        self.assertNotEqual(filecheck.gate(desc, HEAD, False, True, False, BUILDER, PLAN), [])

    # checks: PRC-09
    def test_ends_its_number_reads_the_status_table(self):
        self.assertTrue(filecheck.ends_its_number("α02c", PLAN))
        self.assertTrue(filecheck.ends_its_number("α00b", PLAN))
        self.assertFalse(filecheck.ends_its_number("α00", PLAN))
        self.assertFalse(filecheck.ends_its_number("α02b", PLAN))


GOOD_NOTE = """# Kindling α00

## What is new
A cube.

## What to try
Turn it.

## What is rough
Everything.

## IDs delivered
`TIM-16`

## Links
- APK: [kindling.apk](https://github.com/x/y/raw/a00/dist/kindling.apk)
"""


class NoteTest(unittest.TestCase):
    # checks: PRC-11
    def test_good_note_passes(self):
        self.assertEqual(filecheck.check_note(GOOD_NOTE), [])

    # checks: PRC-11
    def test_note_without_apk_link_fails(self):
        self.assertNotEqual(filecheck.check_note(GOOD_NOTE.replace("dist/kindling.apk", "dist/other.zip")), [])

    # checks: PRC-11
    def test_note_without_a_heading_fails(self):
        self.assertNotEqual(filecheck.check_note(GOOD_NOTE.replace("## What is rough", "## Rough")), [])


if __name__ == "__main__":
    unittest.main()
