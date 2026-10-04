"""tools/note-page.py makes the note's page (PRC-11, A2.3)."""

import base64
import importlib.util
import os
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("note_page", os.path.join(HERE, "..", "note-page.py"))
note_page = importlib.util.module_from_spec(spec)
spec.loader.exec_module(note_page)

APK = "https://github.com/gunsandsalvi/Project-Nature/raw/branch/dist/kindling.apk"
# A 1 x 1 PNG.
PNG = base64.b64decode(
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg=="
)
SAMPLE = f"""# Kindling a00: Skeleton on the phone

## What is new
- A test card at art resolution, with `4 × 4` pixels.
- Signed with **the release key**.

## What to try
1. Uninstall the old Kindling first.
2. Install from [the APK]({APK}) and open it:
   a checker shows.

## What is rough
Nothing <yet> & no world.

## IDs delivered
`PRC-11`, `PLT-06`

## Links
- APK: {APK}
"""


class NotePage(unittest.TestCase):
    # checks: PRC-11
    def test_install_button_first(self):
        out = note_page.page(SAMPLE)
        self.assertTrue(out.startswith("<title>"))
        install = out.index('class="install"')
        self.assertLess(install, out.index("<h1>"))
        self.assertIn(f'href="{APK}">Download and install</a>', out)

    # checks: PRC-11
    def test_markdown(self):
        out = note_page.page(SAMPLE)
        self.assertIn("<h2>What is new</h2>", out)
        self.assertIn("<code>4 × 4</code>", out)
        self.assertIn("<strong>the release key</strong>", out)
        self.assertIn("<ol>", out)
        self.assertIn("open it: a checker shows.</li>", out)
        self.assertIn("Nothing &lt;yet&gt; &amp; no world.", out)
        self.assertNotIn("<html", out)

    # checks: PRC-11
    def test_pictures_travel_inside_the_page(self):
        with tempfile.TemporaryDirectory() as base:
            os.makedirs(os.path.join(base, "pictures"))
            with open(os.path.join(base, "pictures", "one.png"), "wb") as f:
                f.write(PNG)
            out = note_page.page(SAMPLE + "\n![The self-check](pictures/one.png)\n", base)
        self.assertIn(
            '<img src="data:image/png;base64,' + base64.b64encode(PNG).decode() + '" alt="The self-check">', out
        )
        self.assertIn("<figcaption>The self-check</figcaption>", out)

    # checks: PRC-11
    def test_apk_link_required(self):
        with self.assertRaises(ValueError):
            note_page.page("# A note\n\nNo link.\n")


if __name__ == "__main__":
    unittest.main()
