#!/usr/bin/env python3
"""Implements PLT-06 PRC-11: lossless extension compression before APK alignment and signing."""

import copy
import sys
import zipfile
from pathlib import Path

EXTENSION = "lib/arm64-v8a/libkindling.android.arm64.so"


def compress(source, destination):
    """Change only the extension's compression method; preserve all entry contents and metadata."""
    if Path(source).resolve() == Path(destination).resolve():
        raise ValueError("APK compression needs a separate output file")
    with zipfile.ZipFile(source) as before:
        if EXTENSION not in before.namelist():
            raise ValueError("APK has no arm64 Kindling extension")
        with zipfile.ZipFile(destination, "w") as after:
            after.comment = before.comment
            for item in before.infolist():
                info = copy.copy(item)
                if item.filename == EXTENSION:
                    info.compress_type = zipfile.ZIP_DEFLATED
                after.writestr(
                    info,
                    before.read(item),
                    compresslevel=9 if item.filename == EXTENSION else None,
                )


def main(argv):
    if len(argv) != 3:
        sys.exit("usage: apk-compress.py <unsigned.apk> <compressed.apk>")
    try:
        compress(argv[1], argv[2])
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        sys.exit(f"APK compression: FAILED: {error}")


if __name__ == "__main__":
    main(sys.argv)
