"""tools/cppcache.py decides what the C++ checks may skip (PRC-10, A17): a file is linted again, and a project tested
again, whenever anything it was built from changes."""

import importlib.util
import os
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("cppcache", os.path.join(HERE, "..", "cppcache.py"))
cppcache = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cppcache)


class LintKey(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = self.tmp.name
        self.src = os.path.join(self.root, "src")
        os.makedirs(self.src)
        self.cpp = os.path.join(self.src, "a.cpp")
        self.hpp = os.path.join(self.src, "a.hpp")
        self.rules = os.path.join(self.root, ".clang-tidy")
        for path, text in ((self.cpp, "int a();\n"), (self.hpp, "#pragma once\n"), (self.rules, "Checks: -*\n")):
            with open(path, "w") as f:
                f.write(text)

    def tearDown(self):
        self.tmp.cleanup()

    def key(self, command="c++ -c a.cpp"):
        rules = cppcache.configs(self.cpp, self.root)
        return cppcache.lint_key(b"clang-tidy 18", "^src/", command, rules, [self.cpp, self.hpp])

    # checks: PRC-10
    def test_the_same_files_give_the_same_key(self):
        self.assertEqual(self.key(), self.key())

    # checks: PRC-10
    def test_a_changed_header_the_file_reads_changes_its_key(self):
        before = self.key()
        with open(self.hpp, "a") as f:
            f.write("int b();\n")
        self.assertNotEqual(before, self.key())

    # checks: PRC-10
    def test_a_changed_rule_or_command_changes_its_key(self):
        before = self.key()
        self.assertNotEqual(before, self.key("c++ -O2 -c a.cpp"))
        with open(self.rules, "a") as f:
            f.write("WarningsAsErrors: '*'\n")
        self.assertNotEqual(before, self.key())

    # checks: PRC-10
    def test_the_rules_are_found_up_to_the_root(self):
        nearer = os.path.join(self.src, ".clang-tidy")
        with open(nearer, "w") as f:
            f.write("InheritParentConfig: true\n")
        self.assertEqual(cppcache.configs(self.cpp, self.root), [nearer, self.rules])


if __name__ == "__main__":
    unittest.main()
