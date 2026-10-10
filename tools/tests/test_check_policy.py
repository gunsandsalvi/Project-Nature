"""The reduced audit keeps distinct checks, rejects mismatches, and skips identical passes."""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

CHECK = Path(__file__).resolve().parents[1] / "check.sh"


class AuditPolicy(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        for name in ["tools", "bin", "sim", "data"]:
            (self.root / name).mkdir()
        shutil.copyfile(CHECK, self.root / "tools/check.sh")
        (self.root / "sim/CMakeLists.txt").touch()
        (self.root / "data/input.toml").write_text("version = 1\n")
        (self.root / "tools/env.sh").write_text('export PATH="$PWD/bin:$PATH"\nexport ANDROID_NDK_HOME=/mock-ndk\n')
        self.script("bin/git", "echo cafef00d\n")
        self.script("bin/qemu-aarch64-static", 'exec "$@"\n')
        self.script(
            "bin/cmake",
            """echo "cmake $*" >> calls
if [ "$1" = -S ]; then
  while [ "$1" != -B ]; do shift; done
  shift
  mkdir -p "$1"
  cp tools/fake-kindling "$1/kindling"
  cp tools/fake-tests "$1/kd_sim_tests"
fi
""",
        )
        self.script("tools/fake-tests", 'echo tsan >> calls\nexit "${FAKE_TSAN_STATUS:-0}"\n')
        self.script(
            "tools/fake-kindling",
            """echo "$(basename "$(dirname "$0")") $*" >> calls
if [ "$1" = proof ]; then echo same; exit; fi
DIGEST=0123456789abcdef
if [ "${FAKE_BAD_NDK:-0}" = 1 ] && [[ "$0" = *sim-a64-ndk* ]]; then DIGEST=fedcba9876543210; fi
python3 - "$1" "$DIGEST" <<'JSON'
import json, sys
if sys.argv[1] == 'learning-gate':
    row = {'digest': sys.argv[2], 'reopen_failures': 0}
else:
    row = dict.fromkeys(['sent_digest', 'control_digest', 'missing_digest'], sys.argv[2])
    row.update(pending_reopen=True, delivered_reopen=True, final_reopen=True)
print(json.dumps(row))
JSON
""",
        )
        (self.root / "tools/cppcache.py").write_text(
            "import hashlib\nfrom pathlib import Path\n"
            "print(hashlib.sha256(Path('data/input.toml').read_bytes()).hexdigest())\n"
        )
        for name in ["samebits", "killtest", "scenecheck"]:
            (self.root / f"tools/{name}.py").write_text(
                f"from pathlib import Path\nwith Path('calls').open('a') as f: f.write('{name}\\n')\n"
            )

    def tearDown(self):
        self.tmp.cleanup()

    def script(self, name, body):
        path = self.root / name
        path.write_text("#!/usr/bin/env bash\nset -e\n" + body)
        path.chmod(0o755)

    def run_audit(self, **extra):
        result = subprocess.run(
            ["bash", "tools/check.sh", "--audit"],
            cwd=self.root,
            env={**os.environ, **extra},
            capture_output=True,
            text=True,
            timeout=30,
        )
        return result, (self.root / "calls").read_text()

    def test_only_distinct_audit_checks_run_and_identical_passes_are_skipped(self):
        result, calls = self.run_audit()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for needed in ["tsan", "sim-a64-tie1 proof", "sim-a64-tie2 proof", "killtest", "scenecheck"]:
            self.assertIn(needed, calls)
        self.assertNotIn("sim-a64-gcc", calls)
        self.assertNotIn("ctest", calls)
        self.assertNotIn("view", calls)
        self.assertIn("--target kd_sim_tests", calls)
        self.assertIn("sim-a64-ndk learning-gate 1001 1", calls)
        self.assertIn("sim-gcc idea-gate 2001 1", calls)
        (self.root / "calls").write_text("")
        result, calls = self.run_audit()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for repeated in ["tsan\n", " proof ", "learning-gate", "idea-gate", "killtest", "scenecheck"]:
            self.assertNotIn(repeated, calls)

    def test_changed_runtime_data_invalidates_passed_checks(self):
        self.assertEqual(self.run_audit()[0].returncode, 0)
        (self.root / "calls").write_text("")
        (self.root / "data/input.toml").write_text("version = 2\n")
        result, calls = self.run_audit()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for rerun in ["tsan\n", "learning-gate", "killtest", "scenecheck"]:
            self.assertIn(rerun, calls)

    def test_m3_digest_mismatch_fails_without_caching_a_pass(self):
        result, _ = self.run_audit(FAKE_BAD_NDK="1")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("cross-compiler digests differ", result.stderr)
        self.assertFalse((self.root / "build/passed/audit-digests").exists())

    def test_failed_thread_checker_cannot_be_marked_passed(self):
        result, _ = self.run_audit(FAKE_TSAN_STATUS="66")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Thread checker: FAIL", result.stdout)
        self.assertFalse((self.root / "build/sim-tsan/tests.passed").exists())

    def test_changed_binary_invalidates_digest_and_recovery_passes(self):
        self.assertEqual(self.run_audit()[0].returncode, 0)
        (self.root / "calls").write_text("")
        with (self.root / "tools/fake-kindling").open("a") as out:
            out.write("\n# rebuilt tool binary\n")
        result, calls = self.run_audit()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        for rerun in ["learning-gate", "killtest", "scenecheck"]:
            self.assertIn(rerun, calls)
