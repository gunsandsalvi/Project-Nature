#!/usr/bin/env python3
"""Turns dist/NOTE.md into dist/note/index.html, the alpha's note page (A15.4, PRC-11).

Handles headings, paragraphs, lists, links, bold and inline code; the APK link becomes a large button at the top,
the web link sits under it. Usage: tools/note-page.py [NOTE.md] [out.html]
"""
import html
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LINK = re.compile(r"\[([^\]]+)\]\(([^)\s]+)\)")

STYLE = """:root { --bg: #0d0b14; --fg: #f2dcaa; --dim: #8f8790; --line: #28232f; --fire: #de621c; --fire2: #fdac3f; color-scheme: dark; }
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) { --bg: #0d0b14; --fg: #f2dcaa; --dim: #8f8790; } }
:root[data-theme="dark"] { --bg: #0d0b14; --fg: #f2dcaa; --dim: #8f8790; }
html, body { margin: 0; background: var(--bg); color: var(--fg); overflow-x: hidden; }
body { padding: 24px 16px 48px; font: 16px/1.55 system-ui, sans-serif; max-width: 640px; margin: 0 auto; box-sizing: border-box; }
h1 { font-size: 26px; line-height: 1.2; margin: 0 0 16px; }
h2 { font-size: 18px; margin: 28px 0 8px; padding-top: 12px; border-top: 1px solid var(--line); }
h3 { font-size: 16px; margin: 20px 0 6px; }
p, li { overflow-wrap: anywhere; }
ul, ol { padding-left: 22px; }
a { color: var(--fire2); }
code { font: 14px ui-monospace, monospace; background: var(--line); padding: 1px 4px; border-radius: 3px; }
.apk { display: block; text-align: center; padding: 16px; margin: 0 0 12px; border-radius: 8px; background: var(--fire);
  color: #0d0b14; font-weight: 700; font-size: 20px; text-decoration: none; }
.web { display: block; text-align: center; padding: 10px; margin: 0 0 20px; border: 1px solid var(--dim); border-radius: 8px;
  color: var(--fg); text-decoration: none; }"""


def inline(text):
    """Escapes text, then turns links, inline code and bold into HTML."""
    parts = re.split(r"(`[^`]+`)", text)
    out = []
    for part in parts:
        if part.startswith("`") and part.endswith("`") and len(part) > 1:
            out.append(f"<code>{html.escape(part[1:-1])}</code>")
            continue
        s = html.escape(part, quote=False)
        s = LINK.sub(lambda m: f'<a href="{html.escape(m.group(2))}">{m.group(1)}</a>', s)
        s = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", s)
        out.append(s)
    return "".join(out)


def body(md):
    """Markdown blocks to HTML."""
    out, para, items, kind = [], [], [], None

    def flush():
        nonlocal para, items, kind
        if para:
            out.append(f"<p>{inline(' '.join(para))}</p>")
        if items:
            out.append(f"<{kind}>" + "".join(f"<li>{inline(i)}</li>" for i in items) + f"</{kind}>")
        para, items, kind = [], [], None

    for line in md.split("\n"):
        h = re.match(r"^(#{1,3})\s+(.*)$", line)
        li = re.match(r"^\s*[-*]\s+(.*)$", line)
        ol = re.match(r"^\s*\d+\.\s+(.*)$", line)
        if h:
            flush()
            n = len(h.group(1))
            out.append(f"<h{n}>{inline(h.group(2))}</h{n}>")
        elif li or ol:
            if para:
                flush()
            k = "ul" if li else "ol"
            if kind and kind != k:
                flush()
            kind = k
            items.append((li or ol).group(1))
        elif not line.strip():
            flush()
        elif items and line.startswith("  "):
            items[-1] += " " + line.strip()
        else:
            if items:
                flush()
            para.append(line.strip())
    flush()
    return "\n".join(out)


def page(md):
    """The whole note page."""
    links = LINK.findall(md)
    apk = next((u for _, u in links if u.endswith("dist/kindling.apk")), None)
    web = None
    for line in md.split("\n"):
        if re.match(r"^\s*[-*]?\s*\**web\b", line, re.I):
            m = LINK.search(line)
            if m:
                web = m.group(2)
                break
    title = next((m.group(1) for m in re.finditer(r"^#\s+(.+)$", md, re.M)), "Kindling alpha")
    top = ""
    if apk:
        top += f'<a class="apk" href="{html.escape(apk)}">Download the APK</a>\n'
    if web:
        top += f'<a class="web" href="{html.escape(web)}">Open in the browser</a>\n'
    return (
        '<!doctype html>\n<html lang="en"><head><meta charset="utf-8">\n'
        '<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">\n'
        f"<title>Kindling note</title>\n<style>\n{STYLE}\n</style></head>\n<body>\n"
        f"<!-- {html.escape(title)} -->\n{top}{body(md)}\n</body></html>\n"
    )


def main(argv):
    src = argv[0] if argv else os.path.join(ROOT, "dist", "NOTE.md")
    dst = argv[1] if len(argv) > 1 else os.path.join(ROOT, "dist", "note", "index.html")
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(src, encoding="utf-8") as f:
        text = page(f.read())
    with open(dst, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"Note page: {os.path.relpath(dst, ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
