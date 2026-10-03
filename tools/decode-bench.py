#!/usr/bin/env python3
"""Turns a code the game shows back into its JSON (A15.4, A15.10).

    python3 tools/decode-bench.py KDS1:H4sI...     a code pasted from the phone
    python3 tools/decode-bench.py <file>           a code saved in a file

A code is a prefix naming what it holds, then the gzipped JSON in base64, as the shells make it (A15.4).
KDS1 is the self-check's report; the benchmark's KDB1 joins in α07d.
"""
import base64
import binascii
import gzip
import json
import os
import sys

PREFIXES = {"KDS1:": "self-check report"}


def decode(code):
    """(kind, JSON value) for a code; spaces and line breaks a phone may add are ignored."""
    code = "".join(code.split())
    for prefix, kind in PREFIXES.items():
        if code.startswith(prefix):
            try:
                raw = gzip.decompress(base64.b64decode(code[len(prefix):], validate=True))
            except (binascii.Error, OSError, EOFError) as e:
                raise ValueError(f"the {kind} code is damaged: {e}") from e
            return kind, json.loads(raw.decode("utf-8"))
    raise ValueError(f"not a code this tool knows: it starts {code[:6]!r}")


def main(argv):
    if len(argv) != 2:
        sys.exit(__doc__)
    arg = argv[1]
    text = open(arg, encoding="utf-8").read() if os.path.isfile(arg) else arg
    try:
        kind, value = decode(text)
    except ValueError as e:
        sys.exit(f"Decode: {e}")
    print(f"# {kind}")
    print(json.dumps(value, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main(sys.argv)
