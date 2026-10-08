"""Document structure, ID traceability, delivery notes and item search (PRC-10, PRC-11, PRC-12)."""

import contextlib
import importlib.util
import io
import os
import shutil
import tempfile
import unittest
from unittest import mock

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
        cases = {case[0] for case in filecheck.PLANTED}
        self.assertTrue({"twice", "section", "code names retired", "not mapped"} <= cases)
        self.assertEqual(len({p[2] for p in filecheck.PLANTED}), len(filecheck.PLANTED), "one message a fault")


class Plan(unittest.TestCase):
    # checks: PRC-10 PRC-12
    def test_double_digit_alphas_are_checked_not_silently_skipped(self):
        plan = filecheck.read(os.path.join(filecheck.FIXTURES, "clean", "IMPLEMENTATION.md"))
        for alpha in ("1.9a", "1.10a", "1.11b", "1.12b"):
            with self.subTest(alpha=alpha):
                candidate = plan.replace("1.1a", alpha)
                self.assertEqual(filecheck.check_plan(candidate), [])
                self.assertEqual([step[0] for step in filecheck.alpha_sections(candidate)], [alpha])
                self.assertEqual(filecheck.TASK_ID.findall("T" + alpha + ".1"), ["T" + alpha + ".1"])
                bad = candidate.replace("**Tests:** the fixture's tests.", "")
                self.assertTrue(any("not the six in order" in p for p in filecheck.check_plan(bad)))
                bad = candidate.replace("T" + alpha + ".1", "T1.2a.1")
                self.assertTrue(any("is not one of" in p for p in filecheck.check_plan(bad)))


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


class Scan(unittest.TestCase):
    # checks: PRC-12
    def test_reads_every_layer_and_skips_other_folders_addons_and_builds(self):
        with tempfile.TemporaryDirectory() as root:
            shutil.copytree(os.path.join(filecheck.FIXTURES, "clean"), root, dirs_exist_ok=True)
            for skipped in ("elsewhere/one.gd", "game/addons/gdUnit4/one.gd", "sim/build/one.cpp"):
                os.makedirs(os.path.dirname(os.path.join(root, skipped)), exist_ok=True)
                with open(os.path.join(root, skipped), "w") as f:
                    f.write("## Implements ONE-08\n")
            files = filecheck.repo_files(root)
        for read in (
            "sim/src/one.cpp",
            "sim/tests/one_test.cpp",
            "game/one.gd",
            "game/test/one_test.gd",
            "data/one.toml",
            "tools/tests/test_one.py",
            "PROJECT.md",
            "IMPLEMENTATION.md",
        ):
            self.assertIn(read, files)
        for skipped in ("elsewhere/one.gd", "game/addons/gdUnit4/one.gd", "sim/build/one.cpp"):
            self.assertNotIn(skipped, files)


class Note(unittest.TestCase):
    def test_short_note_and_old_full_note_both_pass(self):
        note = (
            "# α00\n\n## What is new\nA camp.\n\n## What to try\nOpen it.\n\n"
            "## What is rough\nLighting.\n\n"
            "- APK: https://github.com/o/r/raw/branch/dist/kindling.apk\n"
        )
        self.assertEqual(filecheck.check_note(note), [])
        self.assertEqual(filecheck.check_note(note + "\n## IDs delivered\n\n## Links\n"), [])
        self.assertEqual(filecheck.check_note(note.replace("## What to try", "## Other")), ["no heading 'What to try'"])
        self.assertEqual(
            filecheck.check_note(note.replace("kindling.apk", "other.apk")), ["no link to dist/kindling.apk"]
        )


class Documents(unittest.TestCase):
    def test_file_command_works_without_a_git_repository_or_commit_message(self):
        clean = os.path.join(filecheck.FIXTURES, "clean")
        with tempfile.TemporaryDirectory() as root:
            for name in filecheck.DOCUMENTS:
                shutil.copyfile(os.path.join(clean, name), os.path.join(root, name))
            with mock.patch.object(filecheck, "ROOT", root), contextlib.redirect_stdout(io.StringIO()) as out:
                self.assertEqual(filecheck.main(["file"]), 0, out.getvalue())
            self.assertIn("File check: OK", out.getvalue())
            plan = os.path.join(root, "IMPLEMENTATION.md")
            with open(plan, "w") as f:
                f.write(filecheck.read(os.path.join(clean, "IMPLEMENTATION.md")).replace("**Tests:**", "**Other:**"))
            with mock.patch.object(filecheck, "ROOT", root), contextlib.redirect_stdout(io.StringIO()) as out:
                self.assertEqual(filecheck.main(["file"]), 1)
            self.assertIn("not the six in order", out.getvalue())


class Traceability(unittest.TestCase):
    def test_unannotated_tests_still_count_without_claiming_requirement_coverage(self):
        files = filecheck.fixture(os.path.join(filecheck.FIXTURES, "clean"))
        for path, marker in (
            ("sim/tests/one_test.cpp", "// checks:"),
            ("game/test/one_test.gd", "# checks:"),
            ("tools/tests/test_one.py", "# checks:"),
        ):
            files[path] = "\n".join(line for line in files[path].split("\n") if not line.strip().startswith(marker))
        problems, summary = filecheck.coverage(files)
        self.assertEqual(problems, [])
        self.assertIn("1 C++, 1 gdUnit4 and 1 Python tests", summary)

    def test_optional_annotation_can_describe_a_suite_and_is_still_validated(self):
        clean = filecheck.fixture(os.path.join(filecheck.FIXTURES, "clean"))
        path = "sim/tests/one_test.cpp"
        moved = dict(clean)
        moved[path] = moved[path].replace("// checks: ONE-01", "// checks: ONE-01\n" + "\n" * 6)
        self.assertEqual(filecheck.coverage(moved)[0], [])
        for bad_id, message in (("ONE-08", "is not defined"), ("ONE-09", "retired")):
            with self.subTest(bad_id=bad_id):
                files = dict(moved)
                files[path] = files[path].replace("// checks: ONE-01", "// checks: " + bad_id)
                problems, _ = filecheck.coverage(files)
                self.assertEqual(len(problems), 1, problems)
                self.assertIn(message, problems[0])

    def test_partial_implementation_label_does_not_require_a_matching_test_label(self):
        files = filecheck.fixture(os.path.join(filecheck.FIXTURES, "clean"))
        files["sim/src/one.cpp"] += "\n/// Implements ONE-03 (first increment only)\n"
        self.assertEqual(filecheck.coverage(files)[0], [])
        files["game/one.gd"] = files["game/one.gd"].replace("## Implements ONE-02", "## A plainly named module")
        self.assertEqual(filecheck.coverage(files)[0], [])


class Proposals(unittest.TestCase):
    def test_a_decided_item_with_a_proposal_is_listed(self):
        project = "- `ONE-01` **First** *(Decided)*: Its words.\n- `ONE-02` **Second** *(Proposed)*: More.\n"
        proposed = project.replace("Its words.\n", "Its words.\n  - **Proposed change:** New words.\n    Why: x.\n")
        items, _ = filecheck.items_of(proposed)
        self.assertEqual(filecheck.generated_lists(proposed, items)["proposals"], ["ONE-01", "ONE-02"])
        items, _ = filecheck.items_of(project)
        self.assertEqual(filecheck.generated_lists(project, items)["proposals"], ["ONE-02"])


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
