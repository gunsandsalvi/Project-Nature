"""checks: PRE-31 RES-05 (T2.7a.4): changed pixels and unmapped object codes cannot pass frozen comparisons."""

import importlib.util
import json
import tempfile
import unittest
from pathlib import Path

from PIL import Image

SPEC = importlib.util.spec_from_file_location("fixture_goldens", Path(__file__).parents[1] / "fixture-goldens.py")
GOLDENS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(GOLDENS)


class FixtureGoldensTest(unittest.TestCase):
    # checks: PRE-31 RES-05
    def test_changed_pixels_and_unmapped_ids_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for view in GOLDENS.VIEWS:
                (root / f"{view}-capture.json").write_text(
                    json.dumps(
                        {
                            "engine": "pinned",
                            "renderer": "Compatibility",
                            "object_codes": {"1": {}},
                            "material_codes": {"1": {}},
                        }
                    )
                )
                for kind in GOLDENS.PASSES:
                    Image.new("RGBA", (2, 2), (1, 0, 0, 255)).save(root / f"{view}-{kind}.png")
            expected = GOLDENS.read(root)
            self.assertEqual(GOLDENS.check(root, expected), expected)
            path = root / "portrait-noon-colour.png"
            Image.new("RGBA", (2, 2), (2, 0, 0, 255)).save(path)
            with self.assertRaisesRegex(ValueError, "pixels or renderer changed"):
                GOLDENS.check(root, expected)
            path = root / "portrait-noon-object.png"
            Image.new("RGBA", (2, 2), (2, 0, 0, 255)).save(path)
            with self.assertRaisesRegex(ValueError, "codes missing"):
                GOLDENS.read(root)


if __name__ == "__main__":
    unittest.main()
