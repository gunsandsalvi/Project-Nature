# B09 pre-test: catalogues and real-world data

Throwaway test for block `B09` (also feasibility test T12, "sourcing ten thousand numbers"). Serves `MAT-05`, `MAT-13`, `MAT-16`, `PRN-05`, `RSK-16`. Delete once the architecture exists.

## The question

1. Which file format should catalogue entries use (`MAT-13`)?
2. What does it really cost to source one entry honestly, with every value checked against the fetched source and the supporting passage quoted word for word (`RSK-16`)?
3. May we use values from the likely data sources, and how do we credit them?

## The approaches

- **Formats:** A = YAML, B = TOML, C = Markdown (a table for people plus a fenced machine-readable block).
- **Sourcing:** an agent finds sources, copies quotes, writes the entry; a checker script re-fetches every source and confirms each quote and number.

## Decision rule

Written 2026-10-01 13:30 UTC, before any sourcing. Not changed afterwards.

**Format**

1. A format qualifies only if the checker loads all five entries with zero parse or validation errors, and all formats load to identical data.
2. Of the qualifying formats, choose the one easiest to read on a phone: values readable without sideways scrolling, quotes readable, little syntax noise.
3. Tie-break: fewer errors in my first drafts (parse errors, validation errors, mismatches with the other formats). This answers "can an AI write it without errors".
4. A format with a silent type trap (for example YAML reading `2.5e3` as text) still qualifies only if the checker catches the trap.

**Cost**

5. Time per entry runs from the first search for that entry to the checker passing it (wall clock). Use the median of the five.
   - Up to 20 minutes per entry: the per-entry road (agent sources, checker verifies) is workable for thousands of entries.
   - Over 20 minutes per entry: flag it. Thousands of entries then need a different plan (for example bulk import from a few open databases, with spot checks).
6. Quote failure rate = quotes not found on the first checker run, out of values with fetchable sources. Any failure means quotes must be copied from fetched text by a tool, never typed, and the checker must block the entry.
7. Value mismatch rate = values whose number is not in their own quote, plus values my review finds attached to the wrong thing. Any mismatch means the number-in-quote check becomes mandatory.
8. If more than a quarter of the sources I would choose can't be fetched by the checker (blocked, paywalled, unreadable PDF), plan for saved copies of sources checked once, not live re-fetching.
9. A property with no source found after about 10 minutes is recorded as a gap, never filled by guessing.

**Licences**

10. Each source gets one of: *use* (values may be used and quoted briefly, with credit), *cite only* (single values with credit; bulk copying forbidden or unclear), *avoid* (the checker can't see it, or its terms forbid reuse).

## Method

1. Built the checker first (`check.py`, before any sourcing).
2. Sourced the five entries one after another, in YAML. Every quote was copied from the checker's own copy of the fetched page (`check.py grep URL REGEX`), never typed or taken from a summary. Each entry was timed from its first search to the checker passing it (`results/timing.tsv`).
3. Wrote the same five entries by hand in TOML and in Markdown, checked all three, and compared them.
4. Planted 168 errors in copies of the entries to see whether the checker catches them (`selftest.py`).
5. Control: asked the summarising web-fetch tool for 10 word-for-word quotes, and checked them the same way.
6. Read each data source's own terms, through the same checker where the site allowed it.

## Results

**Cost per entry** (one agent, wall clock)

| Entry | Time | Values | Gaps |
|---|---|---|---|
| Flint | 5.0 min | 6 | 1 |
| Obsidian | 2.3 min | 6 | 3 |
| Granite | 1.9 min | 7 | 1 |
| Birch wood, dry | 3.0 min | 8 | 0 |
| Cortical bone | 3.4 min | 7 | 1 |
| **Median** | **3.0 min** | 34 in all | 6 of 40 (15%) |

That is about 27 seconds and 3 tool calls per checked value. The whole test, with the checker, the other formats, the self-test and the licences, took about 35 minutes.

**Checking**

| Measure | Result |
|---|---|
| Quotes not found, first run | 0 of 34 |
| Numbers not in their own quote | 0 of 34 |
| Values my review found on the wrong thing | 0 of 34 |
| Usable quotes from the summarising fetch tool | 5 of 10 |
| Planted errors caught | 168 of 168 |
| Value swapped for another number in its own quote | 0 of 19 caught |
| URLs tried that the checker couldn't read | 24 of 74 (32%) |
| Live re-fetch of the 20 sources used | all read, 34 of 34 found, 37 s |

- My review found no wrong values, but 1 indirect quote (granite "can't be chipped" stands in for "not conchoidal"), 6 stand-ins (wood in general for birch 4 times, quartz for flint, volcanic glasses for obsidian) and 2 readings of a scanned table (flint). All are labelled in the entries.
- The fetch tool couldn't read 3 PDFs, and its conversion altered 2 quotes (added `_italic_` marks and a table `|`).
- Unreadable URLs: 8 Europe PMC full texts (HTTP 500), 7 pages needing a browser or showing a bot check (PubMed Central blocked after its first page), 4 refused (Mindat, Britannica, CAMEO, MDPI), 4 dead links, 1 server error.
- 24 of 34 values come from handbooks, government reports or papers; 10 from Wikipedia, The Engineering ToolBox or a museum page, where the better source was paywalled or unreadable.

**Formats** (the same 34 values in each)

| | YAML | TOML | Markdown |
|---|---|---|---|
| Errors in first draft | 0 | 0 | 0 * |
| Same data as the others | yes | yes | yes |
| Size, 5 entries | 19.7 KB | 20.0 KB | 25.4 KB |
| Syntax marks per value | 18 | 41 | 25 |
| On a phone | plain text, some long lines | plain text, more quotes and brackets | rendered table and links; quotes folded away |

\* The first check of the Markdown tables failed on 6 cells because of a bug in my table parser (units with brackets), not in the entries. After the fix: 0 differences.

- The usual Python YAML loader reads `2.5e3` and `1e-3` as text. The checker's type check catches this (4 of 4 planted).
- TOML can't reuse a quote, so the bone table quote is written four times.
- Markdown states every value twice (table and block). The checker catches drift (34 of 34 planted), but it needs a table parser of its own.

## Licences

| Source | Verdict | Credit |
|---|---|---|
| USGS reports | use: public domain | USGS, report number |
| USDA Wood Handbook | use: US Government work | FPL-GTR-190, chapter |
| NIST web pages | use, unless marked copyright | NIST, page |
| NIST data (Chemistry WebBook) | cite only: "All rights reserved" | NIST SRD number |
| Wikipedia | use: CC BY-SA 4.0 | article, CC BY-SA 4.0, permanent link |
| Open-access papers (PLOS, BMC) | use: CC BY 4.0 | authors, year, DOI |
| arXiv, author copies of papers | cite only: the fact plus a short quote | authors, year, DOI |
| Paywalled papers | avoid, unless an open author copy exists: the checker can't see them | |
| The Engineering ToolBox | cite only, never the only source | its own citation line |
| Handbook of Mineralogy | cite only: © Mineral Data Publishing | mineral page |
| RRUFF | cite only: asks for Lafuente and others 2015 | that citation |
| CRC Handbook | avoid: paid subscription the checker can't see | |
| MatWeb | avoid: blocks the checker; its terms forbid copying | |

- Evidence, quoted from the fetched pages on 2026-10-01: USGS "data and information are considered to be in the U.S. Public Domain"; NIST pages "may be distributed or copied", except material marked copyrighted; the WebBook says "Data compilation copyright by the U.S. Secretary of Commerce on behalf of the U.S.A. All rights reserved."; Wikipedia text is CC BY-SA 4.0; PLOS applies CC BY 4.0; CRC offers only a trial and paid plans. MatWeb's terms page refused the checker, so its terms were seen through a web search only.
- RRUFF has moved: rruff.info now redirects elsewhere; the project is at rruff.net. It holds mineral spectra and crystal data, little of what layer 1 needs.
- Facts themselves are free to use in the US, but copying many values out of one database can break the EU database right. This is not legal advice; you decide.

## Verdict

1. **Format: Markdown (C)**, by the rule. All three formats had zero errors and identical data, so phone readability decided, and rendered Markdown wins. The YAML block inside it is the single source of truth: generate the table from the block with a tool rather than typing it, and keep the checker's drift test. Runner-up: plain YAML.
2. **Cost: about 3 minutes per entry** (median), far under 20. The per-entry road works: an agent sources, the checker verifies. T12's 10,000 values (about 1,400 entries) would take about 76 agent-hours, plus review.
3. **Quotes must be copied by a tool** from the fetched text, never typed (rule 6). Tool-copied quotes: 0 of 34 failed. Quotes from a summarising fetch tool: 5 of 10 failed or were missing.
4. **Number-in-quote check:** no real mismatches happened, so rule 7 didn't trigger. Keep it anyway: it caught 31 of 31 planted typos at no cost. It can't catch a value taken from the wrong column (0 of 19), so an independent reviewer must still check meaning (`PRC-09`).
5. **Keep saved copies of sources** (rule 8): 32% of URLs tried were unreadable to the checker. Save each source's fetched text when the entry is made, check against it, and re-fetch live only as an audit. Pin Wikipedia pages to a permanent link. Keep copies of copyrighted pages out of the public repository.
6. **Gaps, not time, are the real limit.** 15% of property slots had no checkable source, and 6 values are stand-ins. Thousands of entries will need stated rules for mixtures and structures (`MAT-03`) and the "chosen or tuned" label (`PRN-05`).

**How the rule held up.** Rules 6 and 7 were badly framed: they look for failures in my own run, but I copied quotes with a tool from the start, so my run could not show the risk. The control and the planted errors supplied the evidence instead. Rule 8's "sources I would choose" was vague: I used all URLs tried (32%); the 20 sources finally used were all readable because I picked readable ones. The tie-break in rule 3 never came into play.

## Caveats

- One agent, five well-studied materials. The times are this agent's; a slower agent or obscure materials could be several times slower. Tool calls per value (about 3) travel better than minutes.
- I wrote and reviewed the entries myself, so the review isn't independent. First-draft error counts for the three formats come from the same careful writer and say little about other agents.
- Phone readability is judged from rendered GitHub Markdown and line lengths, not on your phone.
- Quotes are matched after normalising Unicode, dashes and spaces. PDF tables only match in pdftotext's layout mode. One USGS table is a scan read by OCR, so its column readings are uncertain.
- A checked quote proves the source says it, not that it's right. The Engineering ToolBox gives bone a specific heat of 0.44 kJ/(kg·K), about a third of measured values, and one CC BY paper prints it as "1.25 J/kg °C", a unit error. Plausibility ranges (`MAT-17`) and a second source for key values would catch these.
- The best bone heat data (IT'IS database) is filled in by JavaScript: the page loads, but the checker can't see the numbers. The Wayback Machine wasn't reachable from here.

## For the lead

- `RSK-16`'s response says each value is checked "against the fetched source". With a third of sources unreadable to a script, it may need to say "against a saved copy of the source". That changes a Decided item, so it needs the owner's OK (`PRC-07`).
- Decide whether entries may use stand-in values (another species, the main mineral of a rock), and how they are labelled.

## How to re-run

```
cd pretests/b09-catalogues
export B09_CACHE=/some/folder/outside/the/repo
python3 check.py             # all formats; fetches sources into the cache
python3 check.py --offline   # fields, units and formats only
python3 check.py --refresh   # ignore the cache and fetch live
python3 selftest.py          # planted errors; run check.py first
python3 check.py grep URL 'regex'   # find a passage to quote
```

Needs Python 3.11+, PyYAML, curl and pdftotext. Results are in `results/`: `timing.tsv`, `check-all.txt`, `selftest.txt`, `format-stats.txt`, `webfetch-quotes.txt`, `check-md-first.txt`.
