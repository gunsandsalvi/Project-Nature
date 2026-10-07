"""The art lane's checks on a texture set (A5.3, A5.4, A5.6): what holds each record and its levels to the plan's lines,
run in the cloud before a set goes to the builder.

    python3 tools/art/checks.py [--all] <name>...

For each material under art/textures/ it checks, tile by tile:
- the record's levels: every file is there, in the record's own folder, with the digest the record gives, each level
  named by the level above it (made_from), each half the size of the one above and running down to one texture pixel;
- the numbers a record states: 256 texture pixels across at its first band, 64 a metre at band 0 and half that at each
  band after, so a near tile is 4 m across, a middle tile 16 m and a far tile 64 m;
- the versions: two to four of a tile, all sharing their ring at every designed level, each wrapping on itself and
  joining every other without a seam (a join no larger than the tile's own large jumps);
- no hard row or column inside any level (the builder's seam rule, tiles.inner_seams): a version whose quilted patches
  stop short of its shared border shows a straight line at every cell's edge;
- the sources: every picture the record names is there;
- the colour: each tile's first level within 3 of the material's reference in lightness, and each designed level keeping
  at least 90% of the accents of the level above it (less as far as its record says it was calmed), the measures from
  `kindling look`, taken on each level as the screen shows it (a texture pixel 2 x 2 screen pixels), not per texture
  pixel (A4.8, the owner's choice of 7 October 2026); the engine's own frames are measured by the builder;
- for the water's marks (RGBA) instead: alpha wholly 0 or 255, one colour under the see-through, the coverage of the
  first level sensible and each designed level's within 40% to 150% of the level above's.
It prints each failure (every check with --all) and ends with the counts; exit code 1 if there is a failure. The
catalogue's own loader checks the records' fields (`kindling catalogue check data`).
Implements PRE-20, PRE-22 and PRE-42, see A5.3 and A5.4.
"""

import os
import re
import sys

import numpy as np

import look
import textures
import tiles

SEAM = 1.2  # the most a join or wrap may jump over the tile's own large jumps (tiles.wrap_ratio)
ACCENTS = 0.9  # each designed level keeps this share of the level above's accents (A4.8)
COLOUR = 3.0  # the most a tile's lightness may differ from the reference's, in hundredths of OKLab's
SERVES = {"near": 2, "middle": 2, "far": 3}  # bands whose designed levels each tile holds (A5.3)


class Report:
    def __init__(self, verbose=False):
        self.failures = 0
        self.checks = 0
        self.verbose = verbose

    def check(self, ok, what):
        self.checks += 1
        if self.verbose or not ok:
            print(("pass  " if ok else "FAIL  ") + what)
        self.failures += 0 if ok else 1
        return ok


def ring_of(versions, level):
    """How many outer rows and columns every version keeps the same at a level."""
    n = versions[0][level].shape[0]
    for r in range(1, n // 2):
        if not tiles.shares_ring([v[level] for v in versions], r):
            return r - 1
    return n // 2


def tile_checks(report, name, tile):
    """The checks on one tile's versions; returns its first version's levels, for the colour checks."""
    folder, metres, first_band = textures.TILES[tile]
    first = None
    versions = []
    for v in range(1, 5):
        where = textures.folder(name, tile, v)
        if not os.path.isfile(os.path.join(textures.ROOT, where, "record.toml")):
            break
        record, levels = textures.read_texture(where)
        label = textures.entry_name(name, tile, v)
        files_ok = all(
            b["file"].startswith(where + "/") and tiles.sha256(os.path.join(textures.ROOT, b["file"])) == b["sha256"]
            for b, _ in levels
        )
        report.check(files_ok, f"{label}: each level's file is in its folder and has its digest")
        chained = all(
            levels[i][0].get("made_from") == levels[i - 1][0]["sha256"] for i in range(1, len(levels))
        ) and not levels[0][0].get("made_from")
        report.check(chained, f"{label}: each level names the level above it")
        sizes = [p.shape[0] for _, p in levels]
        report.check(
            sizes == [record["tile_texels"] >> i for i in range(len(sizes))] and sizes[-1] == 1,
            f"{label}: levels halve from {record['tile_texels']} down to one texture pixel ({len(sizes)} levels)",
        )
        report.check(
            record["tile_texels"] == 256
            and record["first_band"] == first_band
            and record["texels_a_metre"] == 64 >> first_band
            and record["tile_texels"] / record["texels_a_metre"] == metres,
            f"{label}: 256 texture pixels at {64 >> first_band} a metre from band {first_band}, {metres} m across",
        )
        missing = [s for s in record["sources"] if not os.path.isfile(os.path.join(textures.ROOT, s))]
        report.check(not missing, f"{label}: its sources are kept" + (f" (missing {missing})" if missing else ""))
        seams = [(i, s) for i, (_, p) in enumerate(levels) for s in tiles.inner_seams(p)]
        report.check(
            not seams,
            f"{label}: no hard row or column inside any level"
            + "".join(
                f" (level {i}: {s[0]} pair {s[1]}|{s[1] + 1}, step {s[2]:.1f} against p99 {s[3]:.1f})" for i, s in seams
            ),
        )
        versions.append([p for _, p in levels])
        if first is None:
            first = levels
    report.check(2 <= len(versions) <= 4, f"{name}/{tile}: {len(versions)} versions (two to four)")
    for i in range(SERVES[tile]):
        ring = ring_of(versions, i)
        want = max(1, 4 >> i)
        report.check(
            ring >= want, f"{name}/{tile} level {i}: the versions share {ring} border texture pixels (at least {want})"
        )
        worst = max(tiles.wrap_ratio(v[i]) for v in versions)
        joins = max(tiles.join_ratio(a[i], b[i]) for a in versions for b in versions)
        report.check(
            worst <= SEAM and joins <= SEAM,
            f"{name}/{tile} level {i}: wrap {worst:.2f}, worst join {joins:.2f} (at most {SEAM})",
        )
    return first


def calmed(table):
    """How far a level's record says its contrast was calmed, from its calibration's words, as a share of 1 (a level
    brought up to the contrast above, or past it, is not calmed)."""
    found = re.search(r"contrast (\d+)%", table.get("calibration", ""))
    return min(1.0, int(found.group(1)) / 100.0) if found else 1.0


def screen_view(picture):
    """A level as the screen shows it, in the measures' reach: a texture pixel is 2 x 2 screen pixels at every band, so
    the level's top left 128 texture pixels enlarged twice (the measures take pictures of at most 256 pixels). The
    accents are measured on this, not per texture pixel, as the owner chose on 7 October 2026 (A4.8): marks that are
    bolder and fewer score lower per texture pixel by themselves. The light and the camera's slant are not here; the
    engine's frames, which the builder measures, have them."""
    return np.repeat(np.repeat(picture[:128, :128, :3], 2, axis=0), 2, axis=1)


def marks_checks(report, name, firsts):
    """The checks on a material of marks (RGBA): every pixel wholly opaque or wholly see-through, every see-through
    pixel the one bleed colour, the first level's coverage a sensible share of the water, and each designed level
    keeping at least 40% of the coverage of the level above and at most 150% (bolder and fewer, never none or more)."""
    for tile, levels in firsts.items():
        shares = []
        for i, (_, p) in enumerate(levels[: SERVES[tile]]):
            alpha = p[..., 3]
            report.check(set(np.unique(alpha).tolist()) <= {0, 255}, f"{name}/{tile} level {i}: alpha is 0 or 255 only")
            clear = p[alpha == 0][:, :3]
            report.check(
                len(np.unique(clear, axis=0)) <= 1, f"{name}/{tile} level {i}: see-through pixels share one colour"
            )
            shares.append(tiles.coverage(p))
        report.check(
            0.03 <= shares[0] <= 0.15,
            f"{name}/{tile}: first level covers {shares[0]:.1%} of the water (3% to 15%)",
        )
        for i in range(1, len(shares)):
            report.check(
                0.4 * shares[i - 1] <= shares[i] <= 1.5 * shares[i - 1],
                f"{name}/{tile} level {i}: covers {shares[i]:.1%} against {shares[i - 1]:.1%} above (40% to 150%)",
            )


def colour_checks(report, name, firsts):
    """The colour checks over a material's tiles' first versions: each tile's first level against the reference tile's,
    each designed level's accents, on the screen's scale, against the level above (less as far as its record says it
    was calmed)."""
    if firsts["near"][0][1].shape[2] == 4:
        return marks_checks(report, name, firsts)
    stats = {}
    for tile, levels in firsts.items():
        stats[tile] = [
            {**look.stats(look.tiled(p)), "accents": look.stats(look.tiled(screen_view(p))).get("accents")}
            for _, p in levels[: SERVES[tile]]
        ]
    ref = stats["near"][0]
    for tile, rows in stats.items():
        off = abs(rows[0]["lightness"] - ref["lightness"])
        report.check(
            off <= COLOUR,
            f"{name}/{tile}: first level's lightness {rows[0]['lightness']:.1f} is {off:.1f} from the reference's "
            f"{ref['lightness']:.1f}",
        )
        for i in range(1, len(rows)):
            above, here = rows[i - 1].get("accents") or 0.0, rows[i].get("accents") or 0.0
            share = ACCENTS * calmed(firsts[tile][i][0])
            report.check(
                here >= share * above,
                f"{name}/{tile} level {i}: accents {here:.1f} against {above:.1f} above (at least {share:.0%})",
            )


def main(argv):
    verbose = "--all" in argv
    names = [a for a in argv[1:] if a != "--all"]
    if not names:
        print(__doc__)
        return 2
    report = Report(verbose)
    for name in names:
        firsts = {}
        for tile in textures.TILES:
            if os.path.isfile(os.path.join(textures.ROOT, textures.folder(name, tile, 1), "record.toml")):
                firsts[tile] = tile_checks(report, name, tile)
        if "near" in firsts:
            colour_checks(report, name, firsts)
    print(f"{report.checks} checks, {report.failures} failures")
    return 1 if report.failures else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
