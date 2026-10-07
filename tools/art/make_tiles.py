"""Makes a big surface's textures from the pictures in art/sources/ and a recipe (A5.3, A5.4): for each of its near,
middle and far tiles, two to four versions that share their ring, each with its designed levels and the levels down to
one texture pixel the engine reads, written with their records under art/textures/<name>/.

    python3 tools/art/make_tiles.py tools/art/recipes/<name>.toml

A tile's recipe names the picture its first level comes from (`sheet`), further pictures of the same material that its
versions are quilted from (`extra`), and for each designed level either the pictures drawn for it (one for each of
those, in order, whose versions are quilted again at that level's own size, with its own `overlap` and `patch`) or
none, in which case code makes it (with a `contrast` in percent to calm it). The raw originals of the pictures are
named for the record (`originals`). Nothing is random, so the same recipe and sources give the same
files. The colour measures it fits to come from `kindling look` (set KINDLING to its path if it is not at
build/sim/kindling).

A recipe with a `key` colour (and a `bleed` colour) makes the water's marks instead: pictures of light marks on that
colour, nothing calibrated or flattened, whose coded levels (a `keep` percent each) come from tiles.reduce_marks and
which are written as RGBA, everything but the marks see-through. A `flatten` of 0 leaves a tile's border as it is (for
a ground of big marks such as cobbles, which flattening would wash out).

Implements PRE-20, PRE-22 and PRE-46, see A5.3 and A5.4.
"""

import os
import sys
import tomllib

import numpy as np

import fit
import ingest
import textures
import tiles


def picture(path):
    """A picture kept in art/sources/ as texture pixels: its block found, each block made one pixel; and the loss."""
    pic = tiles.read_rgb(os.path.join(textures.ROOT, path))
    block = tiles.block_size(pic)
    texels, loss = tiles.regrid(pic, block)
    return texels, block, loss


def coded_level(levels, ring, level, contrast):
    """The next level made by code from every version of the level above, with the numbers fitted on the first version
    and the contrast scaled by `contrast` percent, and each version's ring put back as the first version's."""
    _, amount, change, words = fit.fit_reduction(levels[level - 1][0], contrast=contrast)
    made = [fit.reduced(v, amount, change) for v in levels[level - 1]]
    keep = max(1, ring >> level)
    return [made[0]] + [tiles.impose_ring(m, made[0], keep) for m in made[1:]], words


def coded_marks(levels, ring, level, keep, seed, key):
    """The next level of a tile of marks made by code from every version of the level above (tiles.reduce_marks), each
    version's ring put back as the first version's."""
    made = [tiles.reduce_marks(v, key, keep, seed + level) for v in levels[level - 1]]
    ring_here = max(1, ring >> level)
    return [made[0]] + [tiles.impose_ring(m, made[0], ring_here) for m in made[1:]]


def marks_chain(designed, key, seed):
    """A tile of marks' levels down to one texture pixel: the designed levels as given, each one after them the marks of
    the one before made bolder (the engine's rule that the chain runs down to one pixel; no band shows these)."""
    out = list(designed)
    while out[-1].shape[0] > 1:
        out.append(tiles.reduce_marks(out[-1], key, 100, seed + len(out)))
    return out


def coarse_marks(versions, key, keep, seed, cells=4):
    """A tile of marks for a farther distance, made from the versions of a nearer tile: a `cells` x `cells` mosaic of
    them, each cell one picked by chance (the versions share their ring, so every join is seamless), made the level
    below until it is one tile across (tiles.reduce_marks). Four cells a side is the far tile (64 m) from the middle
    (16 m)."""
    n = versions[0].shape[0]
    chance = tiles.Chance(seed)
    mosaic = np.empty((cells * n, cells * n, 3), np.uint8)
    for cy in range(cells):
        for cx in range(cells):
            mosaic[cy * n : (cy + 1) * n, cx * n : (cx + 1) * n] = versions[chance.below(len(versions))]
    while mosaic.shape[0] > n:
        mosaic = tiles.reduce_marks(mosaic, key, keep, seed + mosaic.shape[0])
    return mosaic


def make_tile(spec, reference, seed, key=None, given=None):
    """One tile: its versions' chains of levels, the words for each level, and the numbers its record states. With a
    `key` colour the tile is one of marks on a see-through ground (the water's): its pictures are drawn on that colour,
    nothing about them is calibrated or flattened, and its levels below the first are made by tiles.reduce_marks."""
    first, block, loss = given if given is not None else picture(spec["sheet"])
    calibration = ""
    if reference is not None and key is None:
        first, calibration = fit.calibrate(first, reference, float(spec.get("contrast", 100)))
    extras = [
        (picture(p)[0] if key is not None else fit.calibrate(picture(p)[0], first)[0]) for p in spec.get("extra", [])
    ]
    flatten = 0 if key is not None else spec.get("flatten")
    drawn = {t["level"]: t for t in spec.get("level", []) if "pictures" in t}
    coded = {t["level"]: t for t in spec.get("level", []) if "pictures" not in t}
    if sorted(drawn) != list(range(1, len(drawn) + 1)):
        raise ValueError("a tile's drawn levels come first, one after another from level 1")
    sources, pictures, calibrations = [first, *extras], [], []
    for j in sorted(drawn):  # each picture drawn for a level, moved to the colour and contrast of its source above
        pairs = [
            (picture(p)[0], "") if key is not None else fit.calibrate(picture(p)[0], up, match=True)
            for p, up in zip(drawn[j]["pictures"], sources, strict=True)
        ]
        sources = [p for p, _ in pairs]
        pictures.append(sources)
        calibrations.append(pairs[0][1])
    versions, shift = tiles.make_versions(
        first,
        spec["versions"],
        spec["ring"],
        spec["overlap"],
        spec["patch"],
        seed,
        flatten,
        extras,
        [level[0] for level in pictures],
    )
    levels, ways = [versions], [{"calibration": calibration} if calibration else {}]
    for j in range(1, spec["serves"]):
        if j in drawn:
            scale = 2**j
            levels.append(
                tiles.requilt(
                    pictures[j - 1],
                    shift,
                    spec["versions"],
                    max(1, spec["ring"] // scale),
                    drawn[j].get("overlap", max(2, spec["overlap"] // scale)),
                    drawn[j].get("patch", max(8, spec["patch"] // scale)),
                    seed + j,
                    None if flatten is None else flatten // scale,
                    scale,
                )
            )
            ways.append(
                {
                    "way": "drawn for this band, then moved to the level above's colour and contrast; the other "
                    "versions quilted again at this level's own size from the pictures drawn for each of its sources",
                    "calibration": calibrations[j - 1],
                }
            )
        elif key is not None:
            keep = int(coded.get(j, {}).get("keep", 50))
            levels.append(coded_marks(levels, spec["ring"], j, keep, seed, key))
            ways.append(
                {
                    "way": "by code: each 2 x 2 block of the level above a mark where two of its four are, a single "
                    f"speck gone, and {keep}% of the marks kept, so the marks are bolder and fewer",
                }
            )
        else:
            made, words = coded_level(levels, spec["ring"], j, float(coded.get(j, {}).get("contrast", 100)))
            levels.append(made)
            ways.append(
                {
                    "way": "by code: the level above averaged and sharpened until its accents match, then calibrated",
                    "calibration": words,
                }
            )
    if key is not None:
        chains = [marks_chain([level[v] for level in levels], key, seed) for v in range(len(versions))]
    else:
        chains = [tiles.complete_chain([level[v] for level in levels]) for v in range(len(versions))]
    return chains, ways, shift, block, loss


def provenance(spec, specs):
    """The record's lists of sources, originals' digests and C2PA words for every picture a tile came from (a tile made
    from a nearer one has that one's)."""
    if "coarse_of" in spec:
        return provenance(specs[spec["coarse_of"]], specs)
    paths = [spec["sheet"], *spec.get("extra", []), *spec.get("originals", [])]
    for t in spec.get("level", []):
        if "pictures" in t:
            paths += [*t["pictures"], *t.get("originals", [])]
    entries = [ingest.entry(p) for p in paths]
    return paths, [e["sha256"] for e in entries], [e["c2pa"] for e in entries]


def words_of(index, serves, ways, version, shift, spec, block, loss):
    """The record's table of words for one level of one version."""
    if index == 0 and "coarse_of" in spec:
        out = {
            "way": f"by code: a mosaic of 4 x 4 of the {spec['coarse_of']} tile's versions (they join without a seam), "
            f"made the level below twice, each time a mark where two of a 2 x 2 block are and {spec.get('keep', 50)}% "
            "of the marks kept, so the marks are bolder and fewer",
            "regrid_loss": "none: made by code from marks already on the grid",
        }
        out.update(ways[0])
        return out
    if index == 0:
        how = f"the tile's picture (a block of {block}, loss {loss * 100:.1f}%)"
        if ways[0].get("calibration"):
            how += ", moved to the reference's colour"
        if version == 1:
            how += (
                f", shifted {shift[0]} down and {shift[1]} across round its wrap so its ring is its most ordinary part,"
                " its border's broad tone flattened"
            )
        else:
            how += f", its inside quilted from the tile's pictures in patches of {spec['patch']} round the shared ring"
        out = {"way": how, "regrid_loss": f"{loss * 100:.1f}%"}
        out.update(ways[0])
        return out
    if index < serves:
        return ways[index]
    return {"way": "the average of the level above, so the levels run down to one texture pixel; no band shows it"}


def main(argv):
    if len(argv) != 2:
        print(__doc__)
        return 2
    with open(argv[1], "rb") as f:
        recipe = tomllib.load(f)
    name = recipe["name"]
    key = tiles.key_colour(recipe["key"]) if "key" in recipe else None
    bleed = tiles.key_colour(recipe.get("bleed", "#aac4b6")) if key is not None else None
    specs = {t["tile"]: t for t in recipe["tile"]}
    order = [recipe["reference"]] + [t for t in specs if t != recipe["reference"]]
    reference = None
    nearer = {}  # each tile's versions' first levels, on their key colour, for a tile made from a nearer one
    for index, tile in enumerate(order):
        spec = specs[tile]
        given = None
        if "coarse_of" in spec:
            first = coarse_marks(nearer[spec["coarse_of"]], key, int(spec.get("keep", 50)), recipe["seed"] + index)
            given = (first, 1, 0.0)
        chains, ways, shift, block, loss = make_tile(spec, reference, recipe["seed"] + index, key, given)
        nearer[tile] = [chain[0] for chain in chains]
        if reference is None:
            reference = chains[0][0]
        if key is not None:  # the marks' own colours stay, everything else becomes see-through
            chains = [[tiles.to_rgba(level, key, bleed) for level in chain] for chain in chains]
        _, metres, first_band = textures.TILES[tile]
        paths, digests, c2pa = provenance(spec, specs)
        for v, chain in enumerate(chains, start=1):
            fields = {
                "about": f"{recipe['about']}; {tile} tile, {metres} m across, version {v}",
                "route": "picture",
                "tile_texels": 256,
                "texels_a_metre": 64 >> first_band,
                "first_band": first_band,
                "sources": paths,
                "original_sha256": digests,
                "c2pa": c2pa,
                "requests": spec.get("requests", []),
                "made": f"{recipe['how']}; version {v} of {len(chains)}, its ring of {spec['ring']} texture pixels "
                f"shared with the others so any two join without a seam; levels 0 to {spec['serves'] - 1} designed for "
                "the bands it serves",
                "regrid_loss": f"{loss * 100:.1f}%: the picture is already on its grid, each block one colour",
                "truth": recipe["truth"],
                "approved": recipe["approved"],
            }
            table = [words_of(i, spec["serves"], ways, v, shift, spec, block, loss) for i in range(len(chain))]
            where = textures.write_texture(name, tile, v, fields, chain, table)
            print(f"{textures.entry_name(name, tile, v)}: {len(chain)} levels in {where}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
