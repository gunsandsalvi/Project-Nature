"""tools/decode-bench.py round-trips a self-check code."""
import base64
import gzip
import importlib.util
import json
import os
import unittest

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("decode_bench", os.path.join(HERE, "decode-bench.py"))
decode_bench = importlib.util.module_from_spec(spec)
spec.loader.exec_module(decode_bench)


class DecodeTest(unittest.TestCase):
    # checks: PRC-11
    def test_round_trip(self):
        sample = {"v": "a00 abc1234", "dev": "Google Pixel 11 Pro XL SDK 37", "gl": "ANGLE | OpenGL ES 3.2", "fail": ["m::sin"]}
        code = "KDS1:" + base64.b64encode(gzip.compress(json.dumps(sample).encode())).decode()
        self.assertEqual(decode_bench.decode(code), sample)

    # checks: PRC-11
    def test_unknown_prefix_fails(self):
        with self.assertRaises(ValueError):
            decode_bench.decode("XYZ1:abc")


if __name__ == "__main__":
    unittest.main()
