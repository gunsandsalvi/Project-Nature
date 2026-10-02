"""The merge gate and the note check (tools/filecheck.py)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import filecheck  # noqa: E402

HEAD = "0123456789abcdef0123456789abcdef01234567"
BUILDER = ["https://claude.ai/code/session_01Builder"]


def gate(desc, head_passed=True, parent_passed=False, only_results=False):
    return filecheck.gate(desc, HEAD, only_results, head_passed, parent_passed, BUILDER)


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

    # checks: PRC-09 PRC-10
    def test_unchecked_head_fails(self):
        self.assertNotEqual(gate("Review: APPROVE 0123456789ab session_01Reviewer", head_passed=False), [])


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
