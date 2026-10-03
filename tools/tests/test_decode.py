"""tools/decode-bench.py reads back the codes the shells make (A15.4)."""
import base64
import gzip
import importlib.util
import json
import os
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("decode_bench", os.path.join(HERE, "..", "decode-bench.py"))
decode_bench = importlib.util.module_from_spec(spec)
spec.loader.exec_module(decode_bench)


def shell_code(prefix, value):
    """A code made as Kotlin's GZIPOutputStream with Base64 and the web's CompressionStream with btoa make it."""
    raw = json.dumps(value, separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    return prefix + base64.b64encode(gzip.compress(raw)).decode("ascii")


class Decode(unittest.TestCase):
    # checks: PRC-11
    def test_self_check_round_trip(self):
        report = {"v": "a00 · 1000 · abc1234", "dev": "Google Pixel 11 Pro XL SDK 37", "gl": "OpenGL ES 3.2",
                  "fail": ["shader \"upscale\" did not compile:\n0:1 error"]}
        kind, value = decode_bench.decode(shell_code("KDS1:", report))
        self.assertEqual((kind, value), ("self-check report", report))

    # checks: PRC-11
    def test_pasted_with_line_breaks(self):
        code = shell_code("KDS1:", {"fail": []})
        wrapped = "\n".join(code[i:i + 20] for i in range(0, len(code), 20))
        self.assertEqual(decode_bench.decode(" " + wrapped + "\n")[1], {"fail": []})

    # checks: PRC-11
    def test_unknown_or_damaged(self):
        with self.assertRaises(ValueError):
            decode_bench.decode("KDX9:abc")
        with self.assertRaises(ValueError):
            decode_bench.decode("KDS1:" + base64.b64encode(b"not gzip").decode())


if __name__ == "__main__":
    unittest.main()
