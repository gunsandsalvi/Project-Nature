"""tools/filecheck.py: the file, commit and coverage checks, the note check and the item search (PRC-07, PRC-10,
PRC-11, PRC-12)."""

import contextlib
import importlib.util
import io
import os
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("filecheck", os.path.join(HERE, "..", "filecheck.py"))
filecheck = importlib.util.module_from_spec(spec)
spec.loader.exec_module(filecheck)


class Selftest(unittest.TestCase):
    # checks: PRC-10 PRC-12 PRC-07
    def test_planted_faults_fail_alone(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = filecheck.selftest()
        self.assertEqual(code, 0, out.getvalue())
        self.assertGreaterEqual(len(filecheck.PLANTED), 25)
        self.assertEqual(len({p[2] for p in filecheck.PLANTED}), len(filecheck.PLANTED), "one message a fault")


class Where(unittest.TestCase):
    # checks: PRC-12
    def test_finds_the_code_that_names_an_item(self):
        files = filecheck.fixture(os.path.join(filecheck.FIXTURES, "clean"))
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = filecheck.where("ONE-01", files)
        self.assertEqual(code, 0, out.getvalue())
        text = out.getvalue()
        self.assertIn("ONE-01 First (Decided)", text)
        self.assertIn("sim/src/one.cpp", text)
        self.assertNotIn("implements: none", text)
        self.assertNotIn("checks: none", text)
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(filecheck.where("ONE-77", files), 1)


class Note(unittest.TestCase):
    # checks: PRC-11
    def test_headings_and_apk_link(self):
        note = (
            "# α00\n\n## What is new\n\n## What to try\n\n## What is rough\n\n## IDs delivered\n\n## Links\n\n"
            "- APK: https://github.com/o/r/raw/branch/dist/kindling.apk\n"
        )
        self.assertEqual(filecheck.check_note(note), [])
        self.assertEqual(filecheck.check_note(note.replace("## Links", "## Where")), ["no heading 'Links'"])
        self.assertEqual(
            filecheck.check_note(note.replace("kindling.apk", "other.apk")), ["no link to dist/kindling.apk"]
        )


class Commit(unittest.TestCase):
    OLD = (
        "# K\n\n## 1. One\n\n- `ONE-01` **First** *(Decided)*: Its words.\n\n"
        "- `ONE-02` **Second** *(Proposed)*: More.\n"
    )

    def check(self, message, new):
        return filecheck.check_commit("0" * 40, message, self.OLD, new)

    # checks: PRC-07
    def test_made_decided_needs_the_owner(self):
        decided = self.OLD.replace("*(Proposed)*", "*(Decided)*")
        problems = self.check("x\n\nChanged: ONE-02 (agreed)\n", decided)
        self.assertEqual(len(problems), 1, problems)
        self.assertIn("ONE-02 as Decided with no `owner OK`", problems[0])
        self.assertEqual(self.check("x\n\nChanged: ONE-02 (agreed; owner OK, 3 October 2026)\n", decided), [])
        # A reason may run onto the next line, inside its brackets.
        self.assertEqual(self.check("x\n\nChanged: ONE-02 (agreed; owner OK, 3\nOctober 2026)\n", decided), [])
        # Removing a decided item needs the OK too.
        gone = self.OLD.replace("- `ONE-01` **First** *(Decided)*: Its words.\n\n", "")
        self.assertIn("no `owner OK`", " ".join(self.check("x\n\nChanged: ONE-01 (cut)\n", gone)))

    # checks: PRC-07
    def test_reasons_and_what_needs_no_owner(self):
        # A proposal reworded needs a reason but no OK; a decided item rewrapped changes no words.
        reworded = self.OLD.replace("More.", "More words.")
        self.assertEqual(self.check("x\n\nChanged: ONE-02 (clearer)\n", reworded), [])
        rewrapped = self.OLD.replace("Its words.", "Its\n  words.")
        self.assertEqual(self.check("x\n\nChanged: ONE-01 (rewrapped)\n", rewrapped), [])
        bare = self.check("x\n\nChanged: ONE-02\n", reworded)
        self.assertEqual(len(bare), 1, bare)
        self.assertIn("gives no reason in brackets", bare[0])
        self.assertIn("gives no reason", " ".join(self.check("x\n\nChanged: ONE-02 (ONE-01)\n", reworded)))


class Proposals(unittest.TestCase):
    OLD = Commit.OLD

    # checks: PRC-07
    def test_a_proposal_needs_a_reason_not_the_owner(self):
        proposed = self.OLD.replace("Its words.\n", "Its words.\n  - **Proposed change:** New words.\n    Why: x.\n")
        self.assertEqual(
            filecheck.check_commit("0" * 40, "x\n\nChanged: ONE-01 (proposed: new words)\n", self.OLD, proposed), []
        )
        # Words changed beside the proposal still need the owner.
        both = proposed.replace("Its words.", "Its other words.")
        problems = filecheck.check_commit("0" * 40, "x\n\nChanged: ONE-01 (proposed)\n", self.OLD, both)
        self.assertTrue(any("no `owner OK`" in p for p in problems), problems)
        # The owner's OK replaces the text: the proposal goes, the new words stay, with the OK named.
        decided = self.OLD.replace("Its words.", "New words.")
        self.assertEqual(
            filecheck.check_commit("0" * 40, "x\n\nChanged: ONE-01 (new words; owner OK)\n", proposed, decided), []
        )

    # checks: PRC-07 PRC-10
    def test_a_decided_item_with_a_proposal_is_listed(self):
        proposed = self.OLD.replace("Its words.\n", "Its words.\n  - **Proposed change:** New words.\n    Why: x.\n")
        items, _ = filecheck.items_of(proposed)
        self.assertEqual(filecheck.generated_lists(proposed, items)["proposals"], ["ONE-01", "ONE-02"])
        items, _ = filecheck.items_of(self.OLD)
        self.assertEqual(filecheck.generated_lists(self.OLD, items)["proposals"], ["ONE-02"])


class Lists(unittest.TestCase):
    # checks: PRC-10
    def test_contents_as_the_file_writes_them(self):
        project = "# K\n\n## Contents\n\n## How this file works\n\n## 1. Vision\n\n## 8. People: bodies and lives\n"
        items, _ = filecheck.items_of(project)
        want = filecheck.generated_lists(project, items)["contents"]
        self.assertEqual(
            want,
            "- [How this file works](#how-this-file-works)\n- [1.\n  Vision](#1-vision)\n"
            "- [8.\n  People: bodies and lives](#8-people-bodies-and-lives)\n",
        )


if __name__ == "__main__":
    unittest.main()
