"""tools/filecheck.py: the file, commit and coverage checks, the note check and the merge gate (PRC-07, PRC-09,
PRC-10, PRC-11, PRC-12)."""
import contextlib
import importlib.util
import io
import os
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("filecheck", os.path.join(HERE, "..", "filecheck.py"))
filecheck = importlib.util.module_from_spec(spec)
spec.loader.exec_module(filecheck)

HEAD = "5f4f0e7f1fa4dd455ca33ef23b2ae583b198b0c9"
PLAN = "| α00b | Foundations | 1 | 5 | Not started |\n| α01a | Colour | 1 | 6 | Not started |\n" \
       "| α01b | Ground | 1 | 6 | Not started |\n"


def gate(description, **kw):
    args = dict(head=HEAD, only_results=True, head_passed=False, parent_passed=True,
                sessions=["https://claude.ai/code/session_01Builder"], plan=PLAN)
    args.update(kw)
    return filecheck.gate(description, args["head"], args["only_results"], args["head_passed"],
                          args["parent_passed"], args["sessions"], args["plan"])


class Selftest(unittest.TestCase):
    # checks: PRC-10 PRC-12 PRC-07
    def test_planted_faults_fail_alone(self):
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            code = filecheck.selftest()
        self.assertEqual(code, 0, out.getvalue())
        self.assertGreaterEqual(len(filecheck.PLANTED), 25)
        self.assertEqual(len({p[2] for p in filecheck.PLANTED}), len(filecheck.PLANTED), "one message a fault")


class Gate(unittest.TestCase):
    # checks: PRC-09
    def test_builder_reviews_letters_a_later_one_follows(self):
        # α00's own row has left the plan: α00b remains, so α00 does not end its number, and its builder reviews it.
        self.assertEqual(gate("α00: Skeleton\n\nReview: APPROVE 5f4f0e7f1fa4 builder"), [])
        self.assertEqual(gate("α01a: Colour\n\n`Review: APPROVE 5f4f0e7f1fa4 builder`", plan=PLAN), [])
        # α00b's row has left too: it ends α00, so the builder may not approve it.
        plan = PLAN.replace("| α00b | Foundations | 1 | 5 | Not started |\n", "")
        problems = gate("α00b: Foundations\n\nReview: APPROVE 5f4f0e7f1fa4 builder", plan=plan)
        self.assertTrue(any("ends its number" in p for p in problems), problems)

    # checks: PRC-09
    def test_number_end_needs_another_reviewer(self):
        plan = PLAN.replace("| α00b | Foundations | 1 | 5 | Not started |\n", "")
        self.assertEqual(gate("α00b: Foundations\n\nReview: APPROVE 5f4f0e7f1fa4 subagent:a00-review", plan=plan), [])
        self.assertEqual(gate("α00b: x\n\nReview: APPROVE 5f4f0e7f1fa4 https://claude.ai/code/session_02Other",
                              plan=plan), [])
        same = gate("α00b: x\n\nReview: APPROVE 5f4f0e7f1fa4 https://claude.ai/code/session_01Builder", plan=plan)
        self.assertTrue(any("also built this branch" in p for p in same), same)
        early = gate("α01a: x\n\nReview: APPROVE 5f4f0e7f1fa4 subagent:early", plan=PLAN)
        self.assertTrue(any("does not end its number" in p for p in early), early)

    # checks: PRC-09 PRC-10
    def test_head_approved_and_checked(self):
        self.assertEqual(gate("α00: x"), ["no 'Review: APPROVE <commit> <reviewer>' line"])
        other = gate("α00: x\n\nReview: APPROVE 0123456789ab builder")
        self.assertTrue(any("is not the head" in p for p in other), other)
        unchecked = gate("α00: x\n\nReview: APPROVE 5f4f0e7f1fa4 builder", parent_passed=False)
        self.assertTrue(any("no passing results/checks" in p for p in unchecked), unchecked)
        not_results = gate("α00: x\n\nReview: APPROVE 5f4f0e7f1fa4 builder", only_results=False)
        self.assertTrue(any("no passing results/checks" in p for p in not_results), not_results)
        self.assertEqual(gate("α00: x\n\nReview: APPROVE 5f4f0e7f1fa4 builder", only_results=False, head_passed=True), [])
        unnamed = gate("Skeleton\n\nReview: APPROVE 5f4f0e7f1fa4 builder")
        self.assertTrue(any("does not open with the alpha's name" in p for p in unnamed), unnamed)


class Note(unittest.TestCase):
    # checks: PRC-11
    def test_headings_and_apk_link(self):
        note = "# α00\n\n## What is new\n\n## What to try\n\n## What is rough\n\n## IDs delivered\n\n## Links\n\n" \
               "- APK: https://github.com/o/r/raw/branch/dist/kindling.apk\n"
        self.assertEqual(filecheck.check_note(note), [])
        self.assertEqual(filecheck.check_note(note.replace("## Links", "## Where")), ["no heading 'Links'"])
        self.assertEqual(filecheck.check_note(note.replace("kindling.apk", "other.apk")),
                         ["no link to dist/kindling.apk"])


class Lists(unittest.TestCase):
    # checks: PRC-10
    def test_contents_as_the_file_writes_them(self):
        project = "# K\n\n## Contents\n\n## How this file works\n\n## 1. Vision\n\n## 8. People: bodies and lives\n"
        items, _ = filecheck.items_of(project)
        want = filecheck.generated_lists(project, items)["contents"]
        self.assertEqual(want, "- [How this file works](#how-this-file-works)\n- [1.\n  Vision](#1-vision)\n"
                               "- [8.\n  People: bodies and lives](#8-people-bodies-and-lives)\n")


if __name__ == "__main__":
    unittest.main()
