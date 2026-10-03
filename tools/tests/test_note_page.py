"""tools/note-page.py makes the note's page (PRC-11, A15.4)."""
import importlib.util
import os
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("note_page", os.path.join(HERE, "..", "note-page.py"))
note_page = importlib.util.module_from_spec(spec)
spec.loader.exec_module(note_page)

APK = "https://github.com/gunsandsalvi/Project-Nature/raw/branch/dist/kindling.apk"
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
- Web: https://claude.ai/artifact/abc
"""


class NotePage(unittest.TestCase):
    # checks: PRC-11
    def test_install_button_first(self):
        out = note_page.page(SAMPLE)
        self.assertTrue(out.startswith("<title>"))
        install = out.index('class="install"')
        self.assertLess(install, out.index("<h1>"))
        self.assertIn(f'href="{APK}">Download and install</a>', out)
        self.assertIn('class="web" href="https://claude.ai/artifact/abc"', out)

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
    def test_apk_link_required(self):
        with self.assertRaises(ValueError):
            note_page.page("# A note\n\nNo link.\n")


if __name__ == "__main__":
    unittest.main()
