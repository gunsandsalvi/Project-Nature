#!/usr/bin/env python3
"""Turns an alpha's note into its page (PRC-11, A2.3): dist/NOTE.md becomes dist/note/index.html, a private page
with the install button at its top, published at the note's one URL.

    python3 tools/note-page.py [dist/NOTE.md] [dist/note/index.html]

The note is plain Markdown: headings, paragraphs, lists, links, `code` and **bold**. The page follows the
artifact host's page rules: a short title, colour tokens for light and dark themes, a 16 px gutter, no sideways
scrolling at phone width, and no document skeleton of its own (the host wraps it).
"""

import html
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

STYLE = """<style>
  :root {
    --bg: #faf6ef; --fg: #24202b; --dim: #6b6474; --line: #e2dbcf; --code: #f0e9dc;
    --accent: #b8471a; --on-accent: #fffaf3;
  }
  @media (prefers-color-scheme: dark) {
    :root:not([data-theme="light"]) {
      --bg: #15131b; --fg: #ebe5da; --dim: #a39ca9; --line: #2f2b38; --code: #221f2b;
      --accent: #f08a3c; --on-accent: #1b1208; color-scheme: dark;
    }
  }
  :root[data-theme="dark"] {
    --bg: #15131b; --fg: #ebe5da; --dim: #a39ca9; --line: #2f2b38; --code: #221f2b;
    --accent: #f08a3c; --on-accent: #1b1208; color-scheme: dark;
  }
  body { background: var(--bg); color: var(--fg);
         font: 16px/1.55 system-ui, -apple-system, "Segoe UI", Roboto, sans-serif; }
  main { max-width: 40rem; margin: 0 auto; padding-inline: 16px; padding-block: 20px 48px; }
  .install { display: block; text-align: center; border-radius: 12px; text-decoration: none; }
  .install { background: var(--accent); color: var(--on-accent); font-weight: 650; font-size: 1.15rem; padding: 16px; }
  h1 { font-size: 1.55rem; line-height: 1.25; margin: 28px 0 6px; text-wrap: balance; }
  h2 { font-size: 1.1rem; margin: 26px 0 6px; padding-top: 14px; border-top: 1px solid var(--line);
       text-wrap: balance; }
  h3 { font-size: 1rem; margin: 18px 0 4px; }
  p, li { max-width: 65ch; }
  ul, ol { padding-left: 1.3em; }
  li + li { margin-top: 4px; }
  a { color: var(--accent); overflow-wrap: anywhere; }
  code { font: 0.88em ui-monospace, "SF Mono", Menlo, Consolas, monospace; background: var(--code);
         padding: 0.1em 0.35em; border-radius: 4px; overflow-wrap: anywhere; }
  .meta { color: var(--dim); font-size: 0.9rem; }
</style>"""

INLINE = re.compile(r"`([^`]+)`|\*\*(.+?)\*\*|\[([^\]]+)\]\((https?://[^)\s]+)\)|(https?://[^\s<)]+)")
URL = re.compile(r"https?://[^\s<)\]]+")


def inline(text):
    out, pos = [], 0
    for m in INLINE.finditer(text):
        out.append(html.escape(text[pos : m.start()]))
        code, bold, label, url, bare = m.groups()
        if code is not None:
            out.append(f"<code>{html.escape(code)}</code>")
        elif bold is not None:
            out.append(f"<strong>{inline(bold)}</strong>")
        elif label is not None:
            out.append(f'<a href="{html.escape(url, quote=True)}">{html.escape(label)}</a>')
        else:
            out.append(f'<a href="{html.escape(bare, quote=True)}">{html.escape(bare)}</a>')
        pos = m.end()
    out.append(html.escape(text[pos:]))
    return "".join(out)


def blocks(md):
    out, para, kind = [], [], None

    def flush():
        if para:
            out.append(f"<p>{inline(' '.join(para))}</p>")
            para.clear()

    def close():
        nonlocal kind
        if kind:
            out.append(f"</{kind}>")
            kind = None

    for line in md.splitlines():
        s = line.strip()
        if not s:
            flush()
            close()
            continue
        h = re.match(r"(#{1,3}) (.+)", s)
        if h:
            flush()
            close()
            n = len(h.group(1))
            out.append(f"<h{n}>{inline(h.group(2))}</h{n}>")
            continue
        item = re.match(r"(?:[-*]|(\d+)\.) (.+)", s)
        if item:
            flush()
            want = "ol" if item.group(1) else "ul"
            if kind != want:
                close()
                out.append(f"<{want}>")
                kind = want
            out.append(f"<li>{inline(item.group(2))}</li>")
            continue
        if kind and line[:1] in (" ", "\t"):
            out[-1] = out[-1][: -len("</li>")] + " " + inline(s) + "</li>"
            continue
        close()
        para.append(s)
    flush()
    close()
    return "\n".join(out)


def page(md):
    """The note's page; the APK link (a URL ending in kindling.apk) is required."""
    urls = URL.findall(md)
    apk = next((u for u in urls if u.endswith("/kindling.apk")), None)
    if apk is None:
        raise ValueError("the note has no link to dist/kindling.apk")
    title = next((line[2:].strip() for line in md.splitlines() if line.startswith("# ")), "Kindling")
    parts = [
        "<title>Kindling alpha note</title>",
        STYLE,
        "<main>",
        f'<a class="install" href="{html.escape(apk, quote=True)}">Download and install</a>',
    ]
    parts += [blocks(md), f'<p class="meta">{html.escape(title)}</p>', "</main>"]
    return "\n".join(parts) + "\n"


def main(argv):
    src = argv[1] if len(argv) > 1 else os.path.join(ROOT, "dist", "NOTE.md")
    dst = argv[2] if len(argv) > 2 else os.path.join(ROOT, "dist", "note", "index.html")
    try:
        text = page(open(src, encoding="utf-8").read())
    except ValueError as e:
        sys.exit(f"Note page: {e}")
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"Note page: {os.path.relpath(dst, ROOT)}")


if __name__ == "__main__":
    main(sys.argv)
