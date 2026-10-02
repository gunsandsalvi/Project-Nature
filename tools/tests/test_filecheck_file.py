"""The file check and the commit check (tools/filecheck.py file)."""
import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import filecheck  # noqa: E402

PROJECT = """# P

## How this file works

- Examples such as `WLD-01` and `RCK-100` are not checked here.

## 1. Area

- `ABC-01` **First** *(Decided)*: cites `ABC-02`.
  - **What:** plain.
- `ABC-02` **Second** *(Decided)*
1. `ABC-03` **Third** *(Dropped)*
  - **Dropped because:** merged into `ABC-01`.
"""

ARCH = """# A

## A1. One

### A1.1 Sub

Cites `ABC-01` and A1.1.
"""

PLAN = """# Plan

### α00b The checks (about 1 hour)

**Goal:** g.

**Serves:** `ABC-01`.

**Architecture:** `A1.1`.

**Needs:** none.

**Crates and files touched:** none.

**Tasks:**

1. `T00b.1` **One (`ABC-01`).**

**Data:** none.

**Tests:** none.

**On the phone:** none.

**Not in this alpha:** none.

**Risks:** none.
"""


def check(project=PROJECT, arch=ARCH, plan=PLAN, commits=()):
    return filecheck.file_check(project, arch, plan, list(commits))[0]


class FileCheckTest(unittest.TestCase):
    # checks: PRC-10
    def test_clean_documents_pass(self):
        problems, items, citations = filecheck.file_check(PROJECT, ARCH, PLAN, [])
        self.assertEqual(problems, [])
        self.assertEqual(items, 3)
        self.assertGreater(citations, 5)

    # checks: PRC-10
    def test_marker_problems(self):
        self.assertTrue(any("defined twice" in p for p in check(PROJECT + "- `ABC-02` **Again** *(Decided)*\n")))
        self.assertTrue(any("not one of" in p for p in check(PROJECT.replace("*(Decided)*\n", "*(Done)*\n"))))
        self.assertTrue(any("is not an ID" in p for p in check(PROJECT + "- `AB-4` **Bad** *(Decided)*\n")))

    # checks: PRC-10
    def test_references(self):
        self.assertTrue(any("`ABC-09` is not defined" in p for p in check(PROJECT + "\nSee `ABC-09`.\n")))
        live = PROJECT.replace("cites `ABC-02`", "cites `ABC-03`")
        self.assertTrue(any("live `ABC-01` cites dropped `ABC-03`" in p for p in check(live)))
        self.assertTrue(any("section A2.4" in p for p in check(arch=ARCH + "And A2.4.\n")))
        self.assertTrue(any("`ABC-03` is Dropped" in p for p in check(plan=PLAN + "\n`ABC-03`\n")))
        # The architecture may cite a dropped item; SHA-256 in prose is not an ID.
        self.assertEqual(check(arch=ARCH + "Once `ABC-03`; SHA-256.\n"), [])

    # checks: PRC-10
    def test_plan_layout(self):
        self.assertTrue(any("field labels" in p for p in check(plan=PLAN.replace("**Data:** none.\n", ""))))
        self.assertTrue(any("does not match α00b" in p for p in check(plan=PLAN.replace("`T00b.1`", "`T00.1`"))))
        two = PLAN.replace("1. `T00b.1` **One (`ABC-01`).**", "1. `T00b.1` **One.**\n2. `T00b.1` **Two.**")
        self.assertTrue(any("repeats" in p for p in check(plan=two)))
        no_id = PLAN.replace("1. `T00b.1` **One (`ABC-01`).**", "1. **One (`ABC-01`).**")
        self.assertTrue(any("no task ID" in p for p in check(plan=no_id)))


class CommitCheckTest(unittest.TestCase):
    NEW = PROJECT.replace("**What:** plain.", "**What:** plainer.")

    # checks: PRC-07
    def test_changed_line_required(self):
        self.assertEqual(filecheck.changed_items(PROJECT, self.NEW), {"ABC-01"})
        self.assertNotEqual(check(commits=[("a" * 40, "Edit\n", PROJECT, self.NEW)]), [])
        msg = "Edit\n\nChanged: ABC-02 (wrong one; owner OK)\n"
        self.assertTrue(any("ABC-01" in p for p in check(commits=[("a" * 40, msg, PROJECT, self.NEW)])))
        msg = "Edit\n\nChanged: ABC-01 (plainer; owner OK)\n"
        self.assertEqual(check(commits=[("a" * 40, msg, PROJECT, self.NEW)]), [])


if __name__ == "__main__":
    unittest.main()
