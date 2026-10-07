"""The picture tool (PRE-46, A5.4): gpt-run.sh hands Codex its request as written, holds it while the pause file
exists, refuses a request it cannot follow and logs every run. A stand-in
`codex` on the path records what it was given and makes the files a run would, so no test spends a run; each test's
pause file is its own, never the lane's /tmp/kindling-gpt-paused."""

import os
import stat
import subprocess
import tempfile
import unittest

TOOLS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROOT = os.path.dirname(TOOLS)
PICTURE_TOOL = os.path.join(TOOLS, "art", "gpt-run.sh")

FAKE = """#!/usr/bin/env bash
if [ "${1:-}" = "--version" ]; then echo "codex-cli 9.9.9"; exit 0; fi
printf '%s\\n' "$@" >"$FAKE_ARGS"
dir=""; prev=""
for a in "$@"; do [ "$prev" = "-C" ] && dir="$a"; prev="$a"; done
for f in ${FAKE_CREATE:-}; do echo made >"$dir/$f"; done
exit "${FAKE_STATUS:-0}"
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
    def test_requests_are_held_while_the_pause_file_exists(self):
        open(self.pause, "w").close()
        req = self.request("held-01.txt", PICTURE_REQUEST.format(picture="none"))
        done = self.run_tool(PICTURE_TOOL, req, FAKE_CREATE="held-01.png")
        self.assertEqual((done.returncode, done.stdout.strip()), (0, "held held-01 (paused)"))
        self.assertFalse(os.path.exists(self.args_file), "codex ran while paused")
        self.assertFalse(os.path.exists(os.path.join(self.scratch, "runs.md")))

    # checks: PRE-46
    def test_a_request_it_cannot_follow_is_refused_before_codex_runs(self):
        cases = {
            "no prompt": PICTURE_REQUEST.format(picture="none").split("Prompt:")[0],
            "no input picture": PICTURE_REQUEST.format(picture=os.path.join(self.dir, "none-such.png")),
        }
        for what, text in cases.items():
            req = self.request("bad-01.txt", text)
            done = self.run_tool(PICTURE_TOOL, req, FAKE_CREATE="bad-01.png")
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
