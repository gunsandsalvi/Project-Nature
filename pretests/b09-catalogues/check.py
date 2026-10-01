#!/usr/bin/env python3
"""B09 pre-test checker (throwaway; delete once the architecture exists).

Checks catalogue entries (MAT-13) for block B09 / feasibility test T12:
  1. loads entries in three candidate formats: YAML, TOML, Markdown with a machine block;
  2. validates fields, types and units;
  3. checks that every stored number (value, low, high, plus-minus) appears in its own quote;
  4. re-fetches every source through the proxy (cached) and confirms the quote is really
     there, after normalising whitespace and Unicode (RSK-16, PRN-05, MAT-05);
  5. reports sources it could not fetch separately from quotes that were not found;
  6. checks that all formats hold identical data, and that the Markdown table matches its block.

Usage:
  python3 check.py [--format yaml|toml|md|all] [--only ID] [--offline] [--refresh] [--json FILE]
  python3 check.py grep URL REGEX [--width N]   find passages to quote, in the fetched text
  python3 check.py text URL                     print the path of the cached normalised text
Needs Python 3.11+, PyYAML, curl and pdftotext. Cache folder: $B09_CACHE.
"""
import argparse
import datetime as dt
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
import tomllib
import unicodedata
from html.parser import HTMLParser
from pathlib import Path

import yaml

HERE = Path(__file__).resolve().parent
ENTRIES = HERE / "entries"
CACHE = Path(os.environ.get("B09_CACHE") or Path(tempfile.gettempdir()) / "b09-catalogues-cache")
UA = "Mozilla/5.0 (compatible; KindlingSourceCheck/0.1; B09 pre-test)"
EXTRACTOR = 2  # bump when text extraction changes; cached downloads are re-read, not re-fetched

CATALOGUES = {"ingredients", "structures", "laws", "checks"}
SOURCE_KINDS = {"handbook", "government", "paper", "database", "encyclopedia", "aggregator", "book", "website"}
# property -> allowed units (ASCII spellings used in the data)
PROPERTIES = {
    "density": {"g/cm3", "kg/m3"},
    "hardness": {"Mohs", "HV", "GPa", "MPa", "N", "lbf"},
    "fracture_toughness": {"MPa*m^0.5", "kPa*m^0.5"},
    "youngs_modulus": {"GPa", "MPa"},
    "specific_heat": {"J/(kg*K)", "J/(g*K)", "kJ/(kg*K)"},
    "thermal_conductivity": {"W/(m*K)", "mcal/(cm*s*degC)"},
    "thermal_diffusivity": {"mm2/s"},
    "heat_of_combustion": {"MJ/kg", "kJ/g", "kJ/kg"},
    "ignition_temperature": {"degC", "K"},
    "compressive_strength": {"MPa"},
    "tensile_strength": {"MPa"},
    "fracture_habit": {"-"},
}
CATEGORICAL = {"fracture_habit": {"conchoidal", "not conchoidal"}}
NEEDS_CONDITION = {"hardness"}  # the scale or method must be stated
BASES = {"quoted", "midpoint"}
TOP_KEYS = {"id", "name", "catalogue", "layer", "summary", "checks", "properties", "gaps", "sources"}
TOP_REQUIRED = TOP_KEYS - {"checks", "gaps"}
QUALIFIERS = {"about", "at least", "at most"}
PROP_KEYS = {"property", "value", "qualifier", "low", "high", "plusminus", "unit", "basis", "condition",
             "source", "quote", "match", "note"}
PROP_REQUIRED = {"property", "value", "unit", "source", "quote"}
SRC_KEYS = {"title", "url", "doi", "fetched", "kind", "publisher", "licence"}
SRC_REQUIRED = {"title", "url", "fetched", "kind"}

# ---------------------------------------------------------------- text normalising

DASHES = dict.fromkeys(map(ord, "‐‑‒–—―−﹘﹣－"), "-")
QUOTES = {**dict.fromkeys(map(ord, "‘’‚‛′"), "'"),
          **dict.fromkeys(map(ord, "“”„‟″"), '"')}
INVISIBLE = dict.fromkeys(map(ord, "­​‌‍⁠﻿"), None)


def norm(s):
    """Unicode NFKC, one kind of dash and quote, no invisible characters, single spaces."""
    s = unicodedata.normalize("NFKC", s).translate(DASHES).translate(QUOTES).translate(INVISIBLE)
    return re.sub(r"\s+", " ", s).strip()


def squash(s):
    return re.sub(r"\s+", "", s)


def dehyphen(s):  # "conduc- tivity" (a PDF line break) -> "conductivity"
    return re.sub(r"(\w)- (\w)", r"\1\2", s)


TIERS = [
    ("exact", lambda s: s),
    ("ignoring spaces", squash),
    ("joining line-break hyphens", lambda s: squash(dehyphen(s))),
    ("ignoring case", lambda s: squash(dehyphen(s)).casefold()),
]

# ---------------------------------------------------------------- numbers in quotes


def numbers_in(s):
    s = norm(s)
    out = set()
    for m in re.finditer(r"(?<![\d.,])(\d{1,3}(?:[, ]\d{3})+|\d+)(?:\.(\d+))?(?![\d])", s):
        whole, frac = m.group(1), m.group(2)
        out.add(float(re.sub(r"[, ]", "", whole) + ("." + frac if frac else "")))
        for piece in re.split(r"[ ]", whole):  # "2 600" may also be two numbers
            out.add(float(piece.replace(",", "")))
    for m in re.finditer(r"(?<![\d.,])(\d+),(\d+)(?![\d])", s):  # decimal comma, "2,65"
        out.add(float(f"{m.group(1)}.{m.group(2)}"))
    for m in re.finditer(r"(\d+(?:\.\d+)?)\s*[x×]\s*10\^?\s*(-?\d+)", s):  # 1.3 x 10^3
        out.add(float(m.group(1)) * 10 ** int(m.group(2)))
    return out


def close(a, b):
    return abs(a - b) <= 1e-9 * max(1.0, abs(a), abs(b))

# ---------------------------------------------------------------- loading


FENCE = re.compile(r"^```yaml[ \t]*\n(.*?)^```[ \t]*$", re.S | re.M)


def load(path):
    """Return (entry, problems) where problems are parse errors."""
    try:
        if path.suffix == ".yaml":
            return yaml.safe_load(path.read_text(encoding="utf-8")), []
        if path.suffix == ".toml":
            return tomllib.loads(path.read_text(encoding="utf-8")), []
        if path.suffix == ".md":
            text = path.read_text(encoding="utf-8")
            blocks = FENCE.findall(text)
            if len(blocks) != 1:
                return None, [f"parse: expected one ```yaml block, found {len(blocks)}"]
            entry = yaml.safe_load(blocks[0])
            return entry, md_cross_check(text, entry)
    except Exception as exc:  # parse errors are results, not crashes
        return None, [f"parse: {type(exc).__name__}: {str(exc).splitlines()[0]}"]
    return None, [f"parse: unknown suffix {path.suffix}"]


def canonical(entry):
    """Plain JSON-able form so formats can be compared (dates become text, ints floats)."""
    if isinstance(entry, dict):
        return {k: canonical(v) for k, v in entry.items()}
    if isinstance(entry, list):
        return [canonical(v) for v in entry]
    if isinstance(entry, (dt.date, dt.datetime)):
        return entry.isoformat()
    if isinstance(entry, bool):
        return entry
    if isinstance(entry, (int, float)):
        return float(entry)
    return entry

# ---------------------------------------------------------------- Markdown table cross-check


HUMAN_NAMES = {
    "density": "density", "hardness": "hardness", "fracture toughness": "fracture_toughness",
    "young's modulus": "youngs_modulus", "specific heat": "specific_heat",
    "thermal conductivity": "thermal_conductivity", "heat of combustion": "heat_of_combustion",
    "ignition temperature": "ignition_temperature", "compressive strength": "compressive_strength",
    "tensile strength": "tensile_strength", "how it breaks": "fracture_habit",
    "thermal diffusivity": "thermal_diffusivity",
}
PRETTY_UNITS = {
    "g/cm³": "g/cm3", "kg/m³": "kg/m3", "MPa·m½": "MPa*m^0.5", "J/(kg·K)": "J/(kg*K)",
    "J/(g·K)": "J/(g*K)", "kJ/(kg·K)": "kJ/(kg*K)", "W/(m·K)": "W/(m*K)", "°C": "degC",
    "kPa·m½": "kPa*m^0.5",
    "mcal/(cm·s·°C)": "mcal/(cm*s*degC)", "mm²/s": "mm2/s",
}
QUAL_WORDS = {"about": "about", "≈": "about", "at least": "at least", "≥": "at least", ">": "at least",
              "at most": "at most", "≤": "at most", "<": "at most"}
def parse_cell(cell):
    """'about 2.4 g/cm³', '1.77 MPa·m½ (1.71–1.85)', '74.82 GPa ± 0.9' -> parts, or None."""
    c = cell.replace(",", "").strip()
    out = {"q": None, "lo": None, "hi": None, "pm": None}
    mq = re.match(r"(about|at least|at most|≈|≥|≤|>|<)\s*", c)
    if mq:
        out["q"], c = mq.group(1), c[mq.end():]
    mv = re.match(r"(\d+(?:\.\d+)?)\s*", c)
    if not mv:
        return None
    out["v"], c = mv.group(1), c[mv.end():]
    mr = re.search(r"\s*\((\d+(?:\.\d+)?)\s*(?:–|-|to)\s*(\d+(?:\.\d+)?)\)$", c)
    mp = re.search(r"\s*±\s*(\d+(?:\.\d+)?)$", c)
    if mr:
        out["lo"], out["hi"], c = mr.group(1), mr.group(2), c[:mr.start()]
    elif mp:
        out["pm"], c = mp.group(1), c[:mp.start()]
    out["u"] = c.strip()
    return out


def md_cross_check(text, entry):
    """The table people read must say what the machine block says."""
    probs = []
    rows = []
    for line in text.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")] if line.startswith("|") else None
        if not cells or len(cells) != 3 or cells[0].lower() == "property" or set(cells[0]) <= set("-: "):
            continue
        rows.append(cells)
    props = (entry or {}).get("properties") or []
    if len(rows) != len(props):
        probs.append(f"table: {len(rows)} rows but the block has {len(props)} properties")
    for i, (row, p) in enumerate(zip(rows, props), 1):
        name = HUMAN_NAMES.get(row[0].lower().split(" (")[0])
        if name != p.get("property"):
            probs.append(f"table row {i}: '{row[0]}' but block says {p.get('property')}")
            continue
        keys = list((entry or {}).get("sources") or {})
        ref = keys[int(row[2]) - 1] if row[2].isdigit() and 0 < int(row[2]) <= len(keys) else row[2]
        if ref != str(p.get("source")):
            probs.append(f"table row {i}: source {row[2]} ({ref}) but block says {p.get('source')}")
        if p.get("property") in CATEGORICAL:
            if row[1] != p.get("value"):
                probs.append(f"table row {i}: '{row[1]}' but block says {p.get('value')!r}")
            continue
        m = parse_cell(row[1])
        if not m:
            probs.append(f"table row {i}: can't read value cell '{row[1]}'")
            continue
        unit = PRETTY_UNITS.get(m["u"], m["u"])
        checks = [("value", m["v"]), ("low", m["lo"]), ("high", m["hi"]), ("plusminus", m["pm"])]
        for key, txt in checks:
            have = p.get(key)
            if (txt is None) != (have is None) or (txt is not None and not (
                    isinstance(have, (int, float)) and close(float(txt), float(have)))):
                probs.append(f"table row {i} ({name}): {key} {txt} but block says {have}")
        if QUAL_WORDS.get(m["q"]) != p.get("qualifier"):
            probs.append(f"table row {i} ({name}): qualifier '{m['q']}' but block says {p.get('qualifier')}")
        if unit != p.get("unit"):
            probs.append(f"table row {i} ({name}): unit '{m['u']}' but block says {p.get('unit')}")
    return probs

# ---------------------------------------------------------------- validation


def is_num(x):
    return isinstance(x, (int, float)) and not isinstance(x, bool)


def validate(e, stem):
    """Return (errors, warnings, mismatches). Mismatch = a number or word not in its own quote."""
    errs, warns, mism = [], [], []
    if not isinstance(e, dict):
        return ["entry is not a mapping"], warns, mism
    for k in e:
        if k not in TOP_KEYS:
            errs.append(f"unknown field '{k}'")
    for k in sorted(TOP_REQUIRED - set(e)):
        errs.append(f"missing field '{k}'")
    if e.get("id") != stem:
        errs.append(f"id '{e.get('id')}' differs from file name '{stem}'")
    if "catalogue" in e and e["catalogue"] not in CATALOGUES:
        errs.append(f"catalogue '{e['catalogue']}' not one of {sorted(CATALOGUES)}")
    if "layer" in e and not (isinstance(e["layer"], int) and 1 <= e["layer"] <= 7):
        errs.append("layer must be a whole number from 1 to 7 (MAT-16)")
    for c in e.get("checks") or []:
        if not (isinstance(c, str) and re.fullmatch(r"RCK-\d{2,3}", c)):
            errs.append(f"check '{c}' is not a reality-check ID")
    sources = e.get("sources") or {}
    if not isinstance(sources, dict) or not sources:
        errs.append("sources must be a non-empty mapping")
        sources = {}
    for key, s in sources.items():
        if not isinstance(s, dict):
            errs.append(f"source {key}: not a mapping")
            continue
        for k in s:
            if k not in SRC_KEYS:
                errs.append(f"source {key}: unknown field '{k}'")
        for k in sorted(SRC_REQUIRED - set(s)):
            errs.append(f"source {key}: missing '{k}'")
        if not str(s.get("url", "")).startswith("https://"):
            errs.append(f"source {key}: url must start with https://")
        f = s.get("fetched")
        try:
            d = f if isinstance(f, dt.date) else dt.date.fromisoformat(str(f))
            if d > dt.date.today():
                errs.append(f"source {key}: fetched date {d} is in the future")
        except ValueError:
            errs.append(f"source {key}: fetched '{f}' is not a YYYY-MM-DD date")
        if s.get("kind") not in SOURCE_KINDS:
            errs.append(f"source {key}: kind '{s.get('kind')}' not one of {sorted(SOURCE_KINDS)}")
    props = e.get("properties") or []
    if not isinstance(props, list) or not props:
        errs.append("properties must be a non-empty list")
        props = []
    used = set()
    for i, p in enumerate(props, 1):
        if not isinstance(p, dict):
            errs.append(f"property #{i}: not a mapping")
            continue
        name = p.get("property")
        where = f"#{i} {name}"
        for k in p:
            if k not in PROP_KEYS:
                errs.append(f"{where}: unknown field '{k}'")
        for k in sorted(PROP_REQUIRED - set(p)):
            errs.append(f"{where}: missing '{k}'")
        if name not in PROPERTIES:
            errs.append(f"{where}: unknown property")
            continue
        if p.get("unit") not in PROPERTIES[name]:
            errs.append(f"{where}: unit '{p.get('unit')}' not allowed (use one of {sorted(PROPERTIES[name])})")
        if name in NEEDS_CONDITION and not p.get("condition"):
            errs.append(f"{where}: needs a 'condition' naming the scale or method")
        if p.get("source") not in sources:
            errs.append(f"{where}: source '{p.get('source')}' is not listed under sources")
        used.add(p.get("source"))
        quote = p.get("quote")
        if not isinstance(quote, str) or len(quote.strip()) < 15:
            errs.append(f"{where}: quote missing or shorter than 15 characters")
            quote = ""
        v = p.get("value")
        if name in CATEGORICAL:
            if v not in CATEGORICAL[name]:
                errs.append(f"{where}: value {v!r} not one of {sorted(CATEGORICAL[name])}")
            words = p.get("match") or [v]
            for w in words if isinstance(words, list) else [words]:
                if str(w).casefold() not in norm(quote).casefold():
                    mism.append(f"{where}: word '{w}' not in the quote")
            continue
        if not is_num(v):
            errs.append(f"{where}: value {v!r} is {type(v).__name__}, not a number")
            continue
        for k in ("low", "high", "plusminus"):
            if k in p and not is_num(p[k]):
                errs.append(f"{where}: {k} {p[k]!r} is {type(p[k]).__name__}, not a number")
        lo, hi = p.get("low"), p.get("high")
        if is_num(lo) and is_num(hi):
            if lo > hi:
                errs.append(f"{where}: low {lo} above high {hi}")
            elif not lo <= v <= hi:
                errs.append(f"{where}: value {v} outside its range {lo} to {hi}")
        elif (lo is None) != (hi is None):
            errs.append(f"{where}: give both low and high, or neither")
        if lo is None and "plusminus" not in p:
            warns.append(f"{where}: no range or uncertainty (source gives none?)")
        if "qualifier" in p and p["qualifier"] not in QUALIFIERS:
            errs.append(f"{where}: qualifier '{p['qualifier']}' not one of {sorted(QUALIFIERS)}")
        basis = p.get("basis", "quoted")
        if basis not in BASES:
            errs.append(f"{where}: basis '{basis}' not one of {sorted(BASES)}")
        nums = numbers_in(quote)
        need = [(k, p[k]) for k in ("low", "high", "plusminus") if is_num(p.get(k))]
        if basis == "midpoint":
            if not (is_num(lo) and is_num(hi) and abs(v - (lo + hi) / 2) <= 0.005 * abs(v)):
                mism.append(f"{where}: value {v} is not the midpoint of {lo} and {hi}")
        else:
            need.append(("value", v))
        for k, x in need:
            if not any(close(float(x), n) for n in nums):
                mism.append(f"{where}: {k} {x} not found in its quote")
    for g in e.get("gaps") or []:
        if not (isinstance(g, dict) and g.get("property") in PROPERTIES and isinstance(g.get("note"), str)):
            errs.append(f"gap {g!r}: needs a known 'property' and a 'note' saying what was searched")
    for key in sources:
        if key not in used:
            warns.append(f"source {key}: listed but never used")
    return errs, warns, mism

# ---------------------------------------------------------------- fetching


class TextOut(HTMLParser):
    BLOCK = {"p", "div", "br", "li", "ul", "ol", "tr", "td", "th", "table", "h1", "h2", "h3", "h4",
             "h5", "h6", "section", "article", "header", "footer", "dd", "dt", "dl", "figcaption",
             "caption", "blockquote", "pre", "title", "label", "sec", "p", "abstract"}
    SKIP = {"script", "style", "noscript", "template", "svg", "math"}

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.parts, self.skip = [], 0

    def handle_starttag(self, tag, attrs):
        if tag in self.SKIP:
            self.skip += 1
        elif tag in self.BLOCK:
            self.parts.append(" ")

    def handle_endtag(self, tag):
        if tag in self.SKIP:
            self.skip = max(0, self.skip - 1)
        elif tag in self.BLOCK:
            self.parts.append(" ")

    def handle_data(self, data):
        if not self.skip:
            self.parts.append(data)


def html_text(raw):
    p = TextOut()
    p.feed(raw)
    return "".join(p.parts)


def cache_paths(url):
    h = hashlib.sha256(url.encode()).hexdigest()[:16]
    return CACHE / f"{h}.body", CACHE / f"{h}.meta.json", CACHE / f"{h}.txt"


BOT_MARKERS = ("just a moment...", "cf-chl", "captcha", "are you a robot", "access denied",
               "enable javascript and cookies", "request unsuccessful. incapsula")


def fetch(url, refresh=False):
    """Fetch through the proxy (curl reads HTTPS_PROXY), cache it, return meta with 'status'."""
    body, meta_p, txt_p = cache_paths(url)
    if meta_p.exists() and not refresh:
        m = json.loads(meta_p.read_text())
        if m.get("extractor") == EXTRACTOR:
            return m
        m["status"], text = classify(m, body)  # newer text extraction, same download
        return save_meta(m, text, meta_p, txt_p)
    CACHE.mkdir(parents=True, exist_ok=True)
    cmd = ["curl", "-sS", "-L", "--compressed", "--max-time", "120", "-A", UA,
           "-H", "Accept: text/html,application/xhtml+xml,application/xml,application/pdf;q=0.9,*/*;q=0.8",
           "-o", str(body), "-w", "%{http_code}\t%{content_type}\t%{url_effective}", url]
    r = subprocess.run(cmd, capture_output=True, text=True)
    code, ctype, final = (r.stdout.split("\t") + ["", "", ""])[:3]
    m = {"url": url, "curl_exit": r.returncode, "stderr": r.stderr.strip()[-300:], "http": code,
         "content_type": ctype, "final_url": final,
         "fetched_at": dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")}
    m["status"], text = classify(m, body)
    return save_meta(m, text, meta_p, txt_p)


def save_meta(m, text, meta_p, txt_p):
    m["extractor"] = EXTRACTOR
    if text is not None:
        txt_p.write_text(norm(text), encoding="utf-8")
        m["text_chars"] = len(norm(text))
    meta_p.write_text(json.dumps(m, indent=1))
    return m


def classify(m, body):
    if m["curl_exit"] != 0:
        e, err = m["curl_exit"], m["stderr"]
        if e == 56 and ("CONNECT" in err or "proxy" in err.lower()):
            return "unfetchable: host blocked by proxy policy", None
        return {28: "unfetchable: timed out", 6: "unfetchable: no such host",
                35: "unfetchable: TLS failure", 60: "unfetchable: TLS failure"}.get(
            e, f"unfetchable: network error (curl {e})"), None
    code = int(m["http"] or 0)
    if code in (401, 402, 403, 429, 451):
        return f"unfetchable: refused by site (HTTP {code})", None
    if code in (404, 410):
        return f"unfetchable: dead link (HTTP {code})", None
    if code >= 400 or code == 0:
        return f"unfetchable: HTTP {code}", None
    raw = body.read_bytes() if body.exists() else b""
    if raw[:5] == b"%PDF-":
        r = subprocess.run(["pdftotext", "-enc", "UTF-8", str(body), "-"], capture_output=True)
        text = r.stdout.decode("utf-8", "replace")
        if r.returncode != 0 or len(text.strip()) < 200:
            return "unfetchable: PDF without readable text", None
        # Tables keep their rows only in layout mode, so search both readings of the PDF.
        lay = subprocess.run(["pdftotext", "-layout", "-enc", "UTF-8", str(body), "-"], capture_output=True)
        return "ok (pdf)", text + "\n\n[layout reading]\n\n" + lay.stdout.decode("utf-8", "replace")
    charset = re.search(r"charset=([\w-]+)", m["content_type"] or "")
    try:
        raw_text = raw.decode(charset.group(1) if charset else "utf-8")
    except (LookupError, UnicodeDecodeError):
        raw_text = raw.decode("utf-8", "replace")
    text = html_text(raw_text) if "<" in raw_text[:2000] else raw_text
    low = norm(text).lower()
    if re.search(r"recaptcha|challenge-platform|cf-chl|captcha", raw_text[:5000], re.I) and len(low) < 2000:
        return "unfetchable: bot check page", None
    if len(low) < 300:
        return "unfetchable: page has almost no text (needs a browser?)", None
    if any(b in low[:3000] for b in BOT_MARKERS) and len(low) < 20000:
        return "unfetchable: bot check page", None
    return "ok", text


def source_text(url, refresh=False):
    m = fetch(url, refresh)
    txt = cache_paths(url)[2]
    return m, (txt.read_text(encoding="utf-8") if m["status"].startswith("ok") and txt.exists() else None)


def find_quote(quote, variants):
    """variants: tier name -> transformed page text. Returns the first tier that matches."""
    parts = [x for x in re.split(r"\s*(?:\.\.\.|…)\s*", norm(quote)) if x]  # '...' skips text
    for name, f in TIERS:
        page, pos, ok = variants[name], 0, True
        for part in parts:
            i = page.find(f(part), pos)
            if i < 0:
                ok = False
                break
            pos = i + len(f(part))
        if ok:
            return name
    return None


def longest_prefix(quote, page):
    words = norm(quote).split(" ")
    page = squash(page)
    lo = 0
    for n in range(1, len(words) + 1):
        if squash(" ".join(words[:n])) in page:
            lo = n
        else:
            break
    return lo, len(words)

# ---------------------------------------------------------------- main check


def entry_files(fmt):
    exts = {"yaml": ".yaml", "toml": ".toml", "md": ".md"}
    fmts = exts if fmt == "all" else {fmt: exts[fmt]}
    return {f: sorted((ENTRIES / f).glob("*" + ext)) for f, ext in fmts.items()}


def run(args):
    report, out = [], {"formats": {}, "sources": {}, "values": {}}
    say = report.append
    loaded = {}
    for fmt, files in entry_files(args.format).items():
        stats = {"entries": 0, "parse_errors": 0, "validation_errors": 0, "mismatches": 0,
                 "warnings": 0, "table_mismatches": 0}
        say(f"== format {fmt}: {len(files)} files")
        for path in files:
            if args.only and path.stem != args.only:
                continue
            stats["entries"] += 1
            entry, probs = load(path)
            parse = [p for p in probs if p.startswith("parse")]
            table = [p for p in probs if not p.startswith("parse")]
            stats["parse_errors"] += len(parse)
            stats["table_mismatches"] += len(table)
            if entry is None:
                say(f"  {path.name}: PARSE ERROR {'; '.join(parse)}")
                continue
            errs, warns, mism = validate(entry, path.stem)
            stats["validation_errors"] += len(errs)
            stats["mismatches"] += len(mism)
            stats["warnings"] += len(warns)
            loaded.setdefault(path.stem, {})[fmt] = entry
            state = "ok" if not (errs or mism or table) else "PROBLEMS"
            say(f"  {path.name}: {state} ({len(entry.get('properties') or [])} values)")
            for x in errs:
                say(f"    ERROR    {x}")
            for x in mism:
                say(f"    MISMATCH {x}")
            for x in table:
                say(f"    TABLE    {x}")
            for x in warns:
                say(f"    warning  {x}")
        out["formats"][fmt] = stats
    # cross-format equality
    say("== same data in every format?")
    diffs = 0
    for stem, by_fmt in sorted(loaded.items()):
        forms = {f: json.dumps(canonical(e), sort_keys=True) for f, e in by_fmt.items()}
        base_f = next(iter(forms))
        for f, js in forms.items():
            if js != forms[base_f]:
                diffs += 1
                a, b = canonical(by_fmt[base_f]), canonical(by_fmt[f])
                say(f"  {stem}: {f} differs from {base_f}: {describe_diff(a, b)}")
        if len(forms) > 1 and all(js == forms[base_f] for js in forms.values()):
            say(f"  {stem}: identical in {', '.join(forms)}")
    out["cross_format_differences"] = diffs
    if args.offline:
        print("\n".join(report))
        return out, report
    # quotes against fetched sources; one entry form per id (the first format loaded)
    say("== quotes against the fetched sources")
    counts = {"values": 0, "found": 0, "not_found": 0, "unfetchable": 0}
    tiers = {}
    for stem, by_fmt in sorted(loaded.items()):
        e = next(iter(by_fmt.values()))
        sources = e.get("sources") or {}
        for i, p in enumerate(e.get("properties") or [], 1):
            s = sources.get(p.get("source")) or {}
            url = s.get("url")
            if not url:
                continue
            counts["values"] += 1
            m, text = source_text(url, args.refresh)
            out["sources"][url] = m["status"]
            label = f"{stem} #{i} {p.get('property')} [{p.get('source')}]"
            if text is None:
                counts["unfetchable"] += 1
                say(f"  SKIP      {label}: {m['status']}  {url}")
                out["values"][label] = m["status"]
                continue
            if url not in tiers:
                tiers[url] = {name: f(text) for name, f in TIERS}
            hit = find_quote(p.get("quote", ""), tiers[url])
            if hit:
                counts["found"] += 1
                say(f"  found     {label}: {hit}")
                out["values"][label] = f"found ({hit})"
            else:
                counts["not_found"] += 1
                n, total = longest_prefix(p.get("quote", ""), text)
                say(f"  NOT FOUND {label}: first {n} of {total} words found  {url}")
                out["values"][label] = f"not found (first {n} of {total} words)"
    out["quotes"] = counts
    srcs = out["sources"]
    bad = {u: st for u, st in srcs.items() if not st.startswith("ok")}
    say(f"== sources: {len(srcs)} fetched, {len(srcs) - len(bad)} readable, {len(bad)} unfetchable")
    for u, st in sorted(bad.items()):
        say(f"  {st}: {u}")
    say(f"== quotes: {counts['found']} found, {counts['not_found']} not found, "
        f"{counts['unfetchable']} not checkable (source unfetchable), of {counts['values']} values")
    print("\n".join(report))
    return out, report


def describe_diff(a, b, path=""):
    if type(a) is not type(b):
        return f"{path or 'top'}: {a!r} vs {b!r}"
    if isinstance(a, dict):
        for k in sorted(set(a) | set(b)):
            if a.get(k) != b.get(k):
                return describe_diff(a.get(k), b.get(k), f"{path}.{k}" if path else k)
    if isinstance(a, list):
        if len(a) != len(b):
            return f"{path}: {len(a)} items vs {len(b)}"
        for i, (x, y) in enumerate(zip(a, b)):
            if x != y:
                return describe_diff(x, y, f"{path}[{i}]")
    return f"{path}: {a!r} vs {b!r}"


def main():
    if len(sys.argv) > 1 and sys.argv[1] in ("grep", "text"):
        ap = argparse.ArgumentParser(prog="check.py " + sys.argv[1])
        ap.add_argument("url")
        if sys.argv[1] == "grep":
            ap.add_argument("regex")
            ap.add_argument("--width", type=int, default=220)
            ap.add_argument("--max", type=int, default=12)
        ap.add_argument("--refresh", action="store_true")
        a = ap.parse_args(sys.argv[2:])
        m, text = source_text(a.url, a.refresh)
        print(f"[{m['status']}] http {m['http']} {m['content_type']} -> {m['final_url']}"
              f" ({m.get('text_chars', 0)} chars)", file=sys.stderr)
        if text is None:
            sys.exit(2)
        if sys.argv[1] == "text":
            print(cache_paths(a.url)[2])
            return
        for k, mt in enumerate(re.finditer(a.regex, text, re.I)):
            if k >= a.max:
                break
            s, e = max(0, mt.start() - a.width), min(len(text), mt.end() + a.width)
            print(f"--- @{mt.start()}\n{text[s:e]}\n")
        return
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--format", default="all", choices=["all", "yaml", "toml", "md"])
    ap.add_argument("--only", help="check one entry id")
    ap.add_argument("--offline", action="store_true", help="validate only, no fetching")
    ap.add_argument("--refresh", action="store_true", help="ignore the cache and re-fetch")
    ap.add_argument("--json", help="also write a JSON summary here")
    a = ap.parse_args()
    out, report = run(a)
    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1, sort_keys=True))
    st = out["formats"]
    bad = sum(s["parse_errors"] + s["validation_errors"] + s["mismatches"] + s["table_mismatches"]
              for s in st.values()) + out["cross_format_differences"] + out.get("quotes", {}).get("not_found", 0)
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
