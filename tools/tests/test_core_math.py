"""The vendoring of CORE-MATH reads its hard cases exactly (RES-05, A3.4): every value as the .wc files write it,
nothing that is not a finite number, and literals that C++ reads back to the same bits."""

import importlib.util
import os
import tempfile
import unittest

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("core_math", os.path.join(HERE, "core-math.py"))
core_math = importlib.util.module_from_spec(spec)
spec.loader.exec_module(core_math)


class HardCases(unittest.TestCase):
    # checks: RES-05
    def test_values_as_the_files_write_them(self):
        self.assertEqual(core_math.number("0x1.8p-3"), 0.1875)
        self.assertEqual(core_math.number("+0x.8p-1022"), 2.0**-1023)
        self.assertEqual(core_math.number("+0x1p-1074"), 5e-324)
        self.assertEqual(core_math.number("+1"), 1.0)
        self.assertEqual(str(core_math.number("-0")), "-0.0")

    # checks: RES-05
    def test_what_is_not_a_finite_number_is_left_out(self):
        for text in ("+inf", "-inf", "+nan", "-snan", "0x8p-972,0x4p-128): Exception", "0x1p+1024"):
            self.assertIsNone(core_math.number(text), text)

    # checks: RES-05
    def test_literals_keep_every_bit(self):
        for value in (0.1875, -0.0, 0.0, 5e-324, 2.0**-1023, 1.7976931348623157e308, -2.5):
            text = core_math.literal(value)
            self.assertEqual(float.fromhex(text), value)
            self.assertEqual(text.startswith("-"), str(value).startswith("-"))

    # checks: RES-05
    def test_cases_skip_comments_and_specials(self):
        with tempfile.NamedTemporaryFile("w", suffix=".wc", delete=False) as f:
            f.write("# a comment\n+snan,+0\n0x1p+0,0x1.8p+1 # 102\n\n-0x1p-2,+0\n")
        self.addCleanup(os.remove, f.name)
        self.assertEqual(core_math.cases(f.name, 2), [(1.0, 3.0), (-0.25, 0.0)])
        self.assertEqual(core_math.cases(f.name, 1), [])

    # checks: RES-05
    def test_the_sample_is_spread_evenly_and_fixed(self):
        self.assertEqual(core_math.sample(list(range(10)), 3), [0, 3, 6])
        self.assertEqual(core_math.sample([1, 2], 5), [1, 2])


if __name__ == "__main__":
    unittest.main()
