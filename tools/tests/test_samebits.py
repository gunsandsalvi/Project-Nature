"""The same-bits scans catch what they are for (RES-05, A3.4): each test plants one fault and the scan must name it.
The compile tests use the phone's own compiler (NDK r30's clang), which fuses a*b+c by default."""

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import samebits  # noqa: E402

NDK_CLANG = os.path.join(
    os.environ.get("ANDROID_NDK_HOME", os.path.expanduser("~/.cache/kindling/android-ndk-r30")),
    "toolchains/llvm/prebuilt/linux-x86_64/bin/clang++",
)


class Planted(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.root)
        self.saved_root = samebits.ROOT
        samebits.ROOT = self.root
        self.addCleanup(setattr, samebits, "ROOT", self.saved_root)
        self.build = os.path.join(self.root, "build")
        os.makedirs(os.path.join(self.root, "sim"))
        os.makedirs(self.build)
        self.commands = []

    def compile(self, name, code, flags):
        source = os.path.join(self.root, "sim", name)
        with open(source, "w") as f:
            f.write(code)
        obj = os.path.join(self.build, name + ".o")
        args = [NDK_CLANG, "--target=aarch64-linux-android24", "-std=c++20", "-O2", *flags, "-c", source, "-o", obj]
        subprocess.run(args, check=True, capture_output=True)
        self.commands.append({"directory": self.build, "file": source, "arguments": args})
        self.write_commands()

    def write_commands(self):
        with open(os.path.join(self.build, "compile_commands.json"), "w") as f:
            json.dump(self.commands, f)

    def problems(self, kind):
        with contextlib.redirect_stdout(io.StringIO()):
            if kind == "flags":
                return samebits.check_flags([self.build])[0]
            return samebits.scan([self.build])[0]

    # checks: RES-05
    def test_a_later_fp_model_is_caught(self):
        self.commands.append(
            {
                "directory": self.build,
                "file": os.path.join(self.root, "sim", "a.cpp"),
                "arguments": ["clang++", "-ffp-contract=off", "-ffp-model=precise", "-c", "a.cpp"],
            }
        )
        self.write_commands()
        found = self.problems("flags")
        self.assertTrue(any("-ffp-model=precise" in p for p in found), found)

    # checks: RES-10
    def test_test_switches_in_the_game_s_own_build_are_caught(self):
        os.makedirs(os.path.join(self.root, "view"))
        for name in ("view/a.cpp", "sim/b.cpp"):
            self.commands.append(
                {
                    "directory": self.build,
                    "file": os.path.join(self.root, name),
                    "arguments": ["clang++", "-DKD_TEST_SWITCHES", "-ffp-contract=off", "-funsigned-char", "-c", name],
                }
            )
        self.write_commands()
        found = self.problems("flags")
        self.assertEqual(len(found), 2, found)
        self.assertTrue(all("test switches" in p for p in found), found)
        # the simulation's own build, for its tests and tool, has them
        self.commands = [c for c in self.commands if "/view/" not in c["file"]]
        self.write_commands()
        self.assertEqual(self.problems("flags"), [])

    # checks: RES-05
    def test_fast_maths_is_caught_even_before_the_contract_flag(self):
        self.commands.append(
            {
                "directory": self.build,
                "file": os.path.join(self.root, "sim", "b.cpp"),
                "arguments": ["clang++", "-ffast-math", "-ffp-contract=off", "-c", "b.cpp"],
            }
        )
        self.write_commands()
        self.assertTrue(any("-ffast-math" in p for p in self.problems("flags")))

    # checks: RES-05
    def test_the_rule_passes_when_the_contract_flag_comes_last(self):
        self.commands.append(
            {
                "directory": self.build,
                "file": os.path.join(self.root, "sim", "c.cpp"),
                "arguments": ["clang++", "-funsigned-char", "-fno-fast-math", "-ffp-contract=off", "-c", "c.cpp"],
            }
        )
        self.write_commands()
        self.assertEqual(self.problems("flags"), [])

    # checks: RES-05
    def test_plain_char_must_be_made_unsigned(self):
        for flags in ([], ["-funsigned-char", "-fsigned-char"]):
            self.commands = [
                {
                    "directory": self.build,
                    "file": os.path.join(self.root, "sim", "d.cpp"),
                    "arguments": ["clang++", *flags, "-ffp-contract=off", "-c", "d.cpp"],
                }
            ]
            self.write_commands()
            found = self.problems("flags")
            self.assertTrue(any("-funsigned-char" in p for p in found), (flags, found))

    @unittest.skipUnless(os.path.exists(NDK_CLANG), "needs the NDK")
    # checks: RES-05
    def test_a_fused_multiply_add_is_found(self):
        self.compile("fused.cpp", "double f(double a, double b, double c) { return a * b + c; }\n", [])
        found = self.problems("scan")
        self.assertTrue(any("fused" in p for p in found), found)

    @unittest.skipUnless(os.path.exists(NDK_CLANG), "needs the NDK")
    # checks: RES-05
    def test_no_fusing_with_the_contract_flag(self):
        code = "double f(double a, double b, double c) { return a * b + c; }\n"
        self.compile("plain.cpp", code, ["-ffp-contract=off"])
        self.assertEqual(self.problems("scan"), [])

    @unittest.skipUnless(os.path.exists(NDK_CLANG), "needs the NDK")
    # checks: RES-05
    def test_a_platform_sine_is_found(self):
        code = "#include <cmath>\ndouble g(double x) { return std::sin(x); }\n"
        self.compile("sine.cpp", code, ["-ffp-contract=off"])
        found = self.problems("scan")
        self.assertTrue(any("sin" in p for p in found), found)

    # checks: RES-05
    def test_two_runs_that_differ_are_named(self):
        a = os.path.join(self.root, "a.txt")
        b = os.path.join(self.root, "b.txt")
        with open(a, "w") as f:
            f.write("smoke 0123456789abcdef 1.0\n")
        with open(b, "w") as f:
            f.write("smoke 0123456789abcdee 1.0\n")
        with contextlib.redirect_stdout(io.StringIO()):
            problems, _ = samebits.same([a, b])
        self.assertEqual(len(problems), 1)


if __name__ == "__main__":
    unittest.main()
