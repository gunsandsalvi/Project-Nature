#!/usr/bin/env python3
"""Decodes a code from the phone or the web page and prints its JSON, indented (A15.4, A15.10).

Usage: tools/decode-bench.py 'KDS1:...'   (the self-check; KDB1: benchmark codes join in α07d)
"""
import base64
import gzip
import json
import sys

PREFIXES = ("KDS1:",)


def decode(code):
    """The JSON object inside a code: prefix stripped, base64 decoded, gunzipped."""
    code = code.strip()
    for p in PREFIXES:
        if code.startswith(p):
            raw = gzip.decompress(base64.b64decode(code[len(p):]))
            return json.loads(raw.decode("utf-8"))
    raise ValueError(f"not a known code; it should start with one of {', '.join(PREFIXES)}")


def main(argv):
    if len(argv) != 1:
        print(__doc__)
        return 2
    print(json.dumps(decode(argv[0]), indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
