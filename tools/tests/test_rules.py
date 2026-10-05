"""The banned list catches what it is for (RES-05, PRC-10, A3.4): a file with one planted use of each banned item
must give exactly those problems, on those lines, and the allowed lines beside them none."""

import contextlib
import io
import json
import os
import shutil
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import rules  # noqa: E402

PLANTED = """\
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <random>
#include <thread>
#include <unordered_map>
#include <vector>

struct Counter { int n = 0; int next() { return ++n; } int peek() const { return n; } };
int take(int a, int b) { return a - b; }

long double wide = 1.0L;  // plant: long-double
float narrow = 1.0F;  // plant: float
double wave(double x) { return std::sin(x); }  // plant: platform-maths
double low(double a, double b) { return std::fmin(a, b); }  // plant: fmin-fmax
int whole(double x) { return static_cast<int>(x); }  // plant: float-to-int
double parsed(const char* s) { return std::strtod(s, nullptr); }  // plant: float-text
int roll(std::mt19937& r) { return std::uniform_int_distribution<int>(1, 6)(r); }  // plant: random
int total(const std::vector<int>& v) { return std::reduce(v.begin(), v.end()); }  // plant: reduce
std::unordered_map<int, int> table;  // plant: hash-unordered
void order(std::vector<int>& v) { std::sort(v.begin(), v.end()); }  // plant: sort
unsigned cores() { return std::thread::hardware_concurrency(); }  // plant: outside-input
int both(Counter& c) { return take(c.next(), c.next()); }  // plant: two-effects
void copy(double& to, const double& from) { std::memcpy(&to, &from, sizeof to); }  // plant: raw-memory

double root(double x) { return std::sqrt(x) + std::floor(x); }
void kept(std::vector<int>& v) { std::stable_sort(v.begin(), v.end()); }
std::uint64_t bits(double x) { return std::bit_cast<std::uint64_t>(x); }
int one(Counter& c) { int a = c.next(); return take(a, c.next()) + take(c.peek(), c.peek()); }
int least(int a, int b) { return std::min(a, b); }
"""


class Planted(unittest.TestCase):
    def setUp(self):
        self.root = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, self.root)
        self.addCleanup(setattr, rules, "ROOT", rules.ROOT)
        rules.ROOT = self.root
        self.build = os.path.join(self.root, "build")
        os.makedirs(os.path.join(self.root, "sim", "src"))
        os.makedirs(self.build)

    def check(self, name, code):
        source = os.path.join(self.root, "sim", "src", name)
        with open(source, "w") as f:
            f.write(code)
        command = {"directory": self.build, "file": source, "arguments": ["clang++", "-std=c++20", "-c", source]}
        with open(os.path.join(self.build, "compile_commands.json"), "w") as f:
            json.dump([command], f)
        with contextlib.redirect_stdout(io.StringIO()):
            problems, _ = rules.check(self.build, [source], cache=False)
        return problems

    # checks: RES-05 PRC-10
    def test_each_planted_item_is_caught_on_its_line_and_nothing_else(self):
        expected = {
            f"sim/src/planted.cpp:{n}: {line.split('// plant: ')[1]}"
            for n, line in enumerate(PLANTED.splitlines(), 1)
            if "// plant: " in line
        }
        self.assertEqual(len(expected), len(rules.RULES))
        found = {p.split(": ")[0] + ": " + p.split(": ")[1] for p in self.check("planted.cpp", PLANTED)}
        self.assertEqual(found, expected)

    # checks: RES-05 PRC-10
    def test_an_exempt_file_may_do_what_it_is_for(self):
        os.makedirs(os.path.join(self.root, "sim", "src", "kd", "num"))
        code = "int whole(double x) { return static_cast<int>(x); }\n"
        self.assertEqual(self.check(os.path.join("kd", "num", "convert.cpp"), code), [])
        self.assertEqual(len(self.check(os.path.join("kd", "num", "other.cpp"), code)), 1)

    # checks: RES-05 PRC-10
    def test_view_keeps_only_the_rules_that_reach_it(self):
        os.makedirs(os.path.join(self.root, "view", "src"))
        source = os.path.join("..", "..", "view", "src", "draw.cpp")
        code = "#include <cmath>\nfloat f(float x) { return x * 2.0F; }\ndouble g(double x) { return std::cos(x); }\n"
        found = self.check(source, code)
        self.assertEqual([p.split(": ")[1] for p in found], ["platform-maths"])


if __name__ == "__main__":
    unittest.main()
