"""The art lane's GPT tools (PRE-46, A5.4, A6.1): gpt-blender.sh and gpt-run.sh hand Codex their requests as the
plan says, hold them while the pause file exists, refuse a request they cannot follow and log every run. A stand-in
`codex` on the path records what it was given and makes the files a run would, so no test spends a run; each test's
pause file is its own, never the lane's /tmp/kindling-gpt-paused."""

import os
import stat
import subprocess
import tempfile
import unittest

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(TOOLS)
BLENDER_TOOL = os.path.join(TOOLS, "art", "gpt-blender.sh")
PICTURE_TOOL = os.path.join(TOOLS, "art", "gpt-run.sh")

FAKE = """#!/usr/bin/env bash
if [ "${1:-}" = "--version" ]; then echo "codex-cli 9.9.9"; exit 0; fi
printf '%s\\n' "$@" >"$FAKE_ARGS"
dir=""; prev=""
for a in "$@"; do [ "$prev" = "-C" ] && dir="$a"; prev="$a"; done
for f in ${FAKE_CREATE:-}; do echo made >"$dir/$f"; done
exit "${FAKE_STATUS:-0}"
"""

BLENDER_REQUEST = """Purpose: a test of the tool.
Script: camp.py
Output: camp.blend
Input files: tools/art/kit.py tools/art/kitmath.py
Input script: none
Prompt:
Build the camp's parts.
Second line, kept as written.
"""

PICTURE_REQUEST = """Purpose: a test of the tool.
Orientation: square, 1024 x 1024
Input picture: {picture}
Transparent: no
Prompt:
Paint a meadow.
"""


class Tools(unittest.TestCase):
    def setUp(self):
        d = tempfile.TemporaryDirectory()
        self.addCleanup(d.cleanup)
        self.dir = d.name
        bin_dir = os.path.join(self.dir, "bin")
        os.makedirs(bin_dir)
        fake = os.path.join(bin_dir, "codex")
        with open(fake, "w") as f:
            f.write(FAKE)
        os.chmod(fake, os.stat(fake).st_mode | stat.S_IEXEC)
        self.args_file = os.path.join(self.dir, "args.txt")
        self.pause = os.path.join(self.dir, "paused")
        self.scratch = os.path.join(self.dir, "scratch")
        self.env = dict(
            os.environ,
            PATH=bin_dir + os.pathsep + os.environ["PATH"],
            FAKE_ARGS=self.args_file,
            KINDLING_GPT_PAUSE=self.pause,
        )

    def request(self, name, text):
        path = os.path.join(self.dir, name)
        with open(path, "w") as f:
            f.write(text)
        return path

    def run_tool(self, tool, req, **env):
        e = dict(self.env, **env)
        return subprocess.run([tool, req, self.scratch], capture_output=True, text=True, env=e, cwd=ROOT)

    def given(self):
        with open(self.args_file) as f:
            return f.read().split("\n")[:-1]

    def runs(self):
        with open(os.path.join(self.scratch, "runs.md")) as f:
            return f.read().splitlines()

    # checks: PRE-46
    def test_a_blender_request_runs_in_its_own_folder_and_is_logged(self):
        req = self.request("camp-blend-01.txt", BLENDER_REQUEST)
        done = self.run_tool(BLENDER_TOOL, req, FAKE_CREATE="camp.py camp.blend")
        self.assertEqual(done.returncode, 0, done.stdout + done.stderr)
        self.assertEqual(done.stdout.strip(), "exit 0 camp-blend-01/camp.blend")
        work = os.path.join(self.scratch, "camp-blend-01")
        args = self.given()
        self.assertEqual(args[:6], ["exec", "--skip-git-repo-check", "-s", "workspace-write", "-C", work])
        self.assertEqual(args[-2], "--")
        how = args[-1]
        for words in ("camp-blend-01.prompt", "camp.py", "-- camp.blend", "blender -b --factory-startup", "no numpy"):
            self.assertIn(words, how)
        with open(os.path.join(work, "camp-blend-01.prompt")) as f:
            self.assertEqual(f.read(), "Build the camp's parts.\nSecond line, kept as written.\n")
        for name in ("kit.py", "kitmath.py"):
            self.assertTrue(os.path.exists(os.path.join(work, name)), f"{name} not copied in")
        (line,) = self.runs()
        cells = [c.strip() for c in line.strip("|").split("|")]
        self.assertEqual(cells[1:4], [req, "none", "camp-blend-01/camp.blend"])
        self.assertEqual(cells[5], "Codex 9.9.9, writing a Blender script")

    # checks: PRE-46
    def test_a_script_to_start_from_is_copied_in_under_the_scripts_name(self):
        start = self.request("earlier.py", "print('the first pass')\n")
        req = self.request("camp-blend-02.txt", BLENDER_REQUEST.replace("Input script: none", f"Input script: {start}"))
        self.run_tool(BLENDER_TOOL, req, FAKE_CREATE="camp.blend")
        with open(os.path.join(self.scratch, "camp-blend-02", "camp.py")) as f:
            self.assertEqual(f.read(), "print('the first pass')\n")
        self.assertIn(f"| {start} |", self.runs()[0])

    # checks: PRE-46
    def test_guide_pictures_are_shown_with_the_request(self):
        text = BLENDER_REQUEST.replace("Prompt:", "Input pictures: art/targets/deer-poses.webp\nPrompt:")
        req = self.request("deer-blend-01.txt", text)
        self.run_tool(BLENDER_TOOL, req, FAKE_CREATE="camp.py camp.blend")
        args = self.given()
        self.assertEqual(args[args.index("-i") + 1], os.path.join(ROOT, "art", "targets", "deer-poses.webp"))
        self.assertLess(args.index("-i"), args.index("--"))
        self.assertIn("attached pictures are the guides", args[-1])
        missing = self.request("deer-blend-02.txt", text.replace("deer-poses", "none-such"))
        os.remove(self.args_file)
        self.assertEqual(self.run_tool(BLENDER_TOOL, missing).returncode, 2)
        self.assertFalse(os.path.exists(self.args_file), "codex ran without its picture")

    # checks: PRE-46
    def test_a_run_that_saves_nothing_is_logged_as_failed(self):
        req = self.request("camp-blend-03.txt", BLENDER_REQUEST)
        done = self.run_tool(BLENDER_TOOL, req, FAKE_CREATE="camp.py", FAKE_STATUS="3")
        self.assertEqual(done.stdout.strip(), "exit 3 failed")
        self.assertIn("| failed |", self.runs()[0])

    # checks: PRE-46
    def test_requests_are_held_while_the_pause_file_exists(self):
        open(self.pause, "w").close()
        for tool, text in ((BLENDER_TOOL, BLENDER_REQUEST), (PICTURE_TOOL, PICTURE_REQUEST.format(picture="none"))):
            req = self.request("held-01.txt", text)
            done = self.run_tool(tool, req, FAKE_CREATE="camp.py camp.blend held-01.png")
            self.assertEqual((done.returncode, done.stdout.strip()), (0, "held held-01 (paused)"))
            self.assertFalse(os.path.exists(self.args_file), "codex ran while paused")
            self.assertFalse(os.path.exists(os.path.join(self.scratch, "runs.md")))

    # checks: PRE-46
    def test_a_request_it_cannot_follow_is_refused_before_codex_runs(self):
        cases = {
            "no prompt": BLENDER_REQUEST.split("Prompt:")[0],
            "no script": BLENDER_REQUEST.replace("Script: camp.py\n", ""),
            "a path": BLENDER_REQUEST.replace("Output: camp.blend", "Output: ../camp.blend"),
            "no input": BLENDER_REQUEST.replace("tools/art/kitmath.py", "tools/art/none-such.py"),
        }
        for what, text in cases.items():
            req = self.request("bad-01.txt", text)
            done = self.run_tool(BLENDER_TOOL, req, FAKE_CREATE="camp.py camp.blend")
            self.assertEqual(done.returncode, 2, f"{what}: {done.stdout}")
            self.assertFalse(os.path.exists(self.args_file), f"{what}: codex ran")

    # checks: PRE-46
    def test_a_picture_request_passes_its_input_picture_and_is_logged(self):
        picture = self.request("in.png", "a stand-in picture")
        req = self.request("meadow-09.txt", PICTURE_REQUEST.format(picture=picture))
        done = self.run_tool(PICTURE_TOOL, req, FAKE_CREATE="meadow-09.png")
        self.assertEqual(done.stdout.strip(), "exit 0 meadow-09.png")
        args = self.given()
        self.assertEqual(args[:5], ["exec", "--skip-git-repo-check", "-s", "workspace-write", "-C"])
        self.assertIn(picture, args)
        self.assertEqual(args[args.index("-i") + 1], picture)
        self.assertIn("Square, 1024 x 1024", args[-1])
        with open(os.path.join(self.scratch, "meadow-09.prompt")) as f:
            self.assertEqual(f.read(), "Paint a meadow.\n")
        cells = [c.strip() for c in self.runs()[0].strip("|").split("|")]
        self.assertEqual(cells[1:4], [req, picture, "meadow-09.png"])
        self.assertEqual(cells[5], "Codex 9.9.9, its image tool")


if __name__ == "__main__":
    unittest.main()
