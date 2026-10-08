"""checks: PLT-06 PRC-11: shrink the extension without changing packaged resources or archive metadata."""

import importlib.util
import tempfile
import unittest
import zipfile
from pathlib import Path

SPEC = importlib.util.spec_from_file_location("apk_compress", Path(__file__).parents[1] / "apk-compress.py")
COMPRESS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(COMPRESS)


class ApkCompressionTest(unittest.TestCase):
    # checks: PLT-06 PRC-11
    def test_extension_shrinks_and_every_resource_and_metadata_survives(self):
        with tempfile.TemporaryDirectory() as directory:
            source, destination = [Path(directory) / name for name in ["original.apk", "smaller.apk"]]
            extension = "lib/arm64-v8a/libkindling.android.arm64.so"
            with zipfile.ZipFile(source, "w") as archive:
                archive.comment = b"archive comment"
                for name, payload, method in [
                    (extension, b"ELF fixture\0" * 20000, zipfile.ZIP_STORED),
                    ("lib/arm64-v8a/libgodot_android.so", b"engine" * 5000, zipfile.ZIP_DEFLATED),
                    ("assets/data/textures/one.kdtex", bytes(range(256)), zipfile.ZIP_STORED),
                    ("assets/empty/", b"", zipfile.ZIP_STORED),
                ]:
                    info = zipfile.ZipInfo(name, (2026, 10, 8, 12, 0, 0))
                    info.comment = b"entry comment"
                    info.external_attr = 0o100644 << 16
                    info.compress_type = method
                    archive.writestr(info, payload)
            original = source.read_bytes()
            COMPRESS.compress(source, destination)
            self.assertEqual(source.read_bytes(), original)
            self.assertLess(destination.stat().st_size, source.stat().st_size / 2)
            with zipfile.ZipFile(source) as before, zipfile.ZipFile(destination) as after:
                self.assertEqual(after.namelist(), before.namelist())
                self.assertEqual(after.comment, before.comment)
                for item in before.infolist():
                    changed = after.getinfo(item.filename)
                    self.assertEqual(after.read(item.filename), before.read(item.filename))
                    self.assertEqual(changed.date_time, item.date_time)
                    self.assertEqual(changed.comment, item.comment)
                    self.assertEqual(changed.external_attr, item.external_attr)
                    self.assertEqual(
                        changed.compress_type,
                        zipfile.ZIP_DEFLATED if item.filename == extension else item.compress_type,
                    )


if __name__ == "__main__":
    unittest.main()
