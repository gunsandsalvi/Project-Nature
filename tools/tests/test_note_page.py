"""tools/note-page.py turns a sample note into its page."""
import importlib.util
import os
import unittest

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("note_page", os.path.join(HERE, "note-page.py"))
note_page = importlib.util.module_from_spec(spec)
spec.loader.exec_module(note_page)

SAMPLE = """# Kindling α00: Skeleton on the phone

## What is new
- A golden cube turns on the phone & in the browser.
- Scripts: `tools/check.sh`.

## Links
- APK: [kindling.apk](https://github.com/o/r/raw/a00/dist/kindling.apk)
- Web: [the alpha page](https://claude.ai/artifact/abc)
"""


class NotePageTest(unittest.TestCase):
    # checks: PRC-11
    def test_sample(self):
        out = note_page.page(SAMPLE)
        self.assertIn("<title>Kindling note</title>", out)
        apk = out.index('class="apk" href="https://github.com/o/r/raw/a00/dist/kindling.apk"')
        web = out.index('class="web" href="https://claude.ai/artifact/abc"')
        self.assertLess(apk, web)
        self.assertLess(web, out.index("<h2>What is new</h2>"))
        self.assertIn("<li>A golden cube turns on the phone &amp; in the browser.</li>", out)
        self.assertIn("<code>tools/check.sh</code>", out)
        self.assertIn('<a href="https://claude.ai/artifact/abc">the alpha page</a>', out)
        for token in ["--bg:", "prefers-color-scheme: dark", "overflow-x: hidden", "background: var(--bg)"]:
            self.assertIn(token, out)

    # checks: PRC-11
    def test_image(self):
        import base64
        import tempfile
        png = base64.b64decode("iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==")
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, "cube.png"), "wb") as f:
                f.write(png)
            out = note_page.page("# Note\n\nText.\n\n![The golden cube](cube.png)\n", d)
        data = base64.b64encode(png).decode("ascii")
        self.assertIn(f'<img src="data:image/png;base64,{data}" alt="The golden cube">', out)
        self.assertIn("<figcaption>The golden cube</figcaption>", out)
        self.assertIn("<p>Text.</p>", out)
        self.assertNotIn("cube.png", out, "the page holds the picture itself, so it stays one file")


if __name__ == "__main__":
    unittest.main()
