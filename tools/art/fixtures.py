"""Clean and check image-drawn 2D fixtures (T2.7a.3, A5.3, A5.4).

    python3 tools/art/fixtures.py build    clean the kept inputs and make calibration previews
    python3 tools/art/fixtures.py check    verify pages, provenance, maps, scale and registration

Implements PRE-20, PRE-22 and PRE-46. This draws no colour art.
Raw drawings remain unchanged. Technical passes never approve their appearance or the engine result.
Texture records use the existing catalogue schema; the JSON describes only export and review metadata.
All paths are repository-relative. No engine, simulation, physical catalogue or approved sheet is changed.
"""

import argparse
import json
from pathlib import Path
import sys
import tomllib

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

import ingest
import sheet
import textures
import tiles

ROOT = Path(textures.ROOT)
RECIPE = "art/sources/fixtures27/recipe.json"
MANIFEST = "art/sources/fixtures27/exports.json"
REQUEST = "art/requests/fixtures27/production.txt"
PREVIEW = "art/previews/fixtures27"
DESIGNS = "art/sources/fixtures27/designs.json"
NORMAL_BASIS = "world east, south, up; signed unit XYZ encoded as round((n+1)*127.5)"
# A8.4's declared camera coefficients, shared by this one export recipe. Objects are not squashed again.
SIN37 = 0.6018150231520483
COS37 = 0.7986355100472928
KINDS = ("colour", "material", "normal")
GROUND_INPUTS = {}
WOODY_INPUTS = {}


def colour_flecks(rgba):
    """Flag a colour without an equal eight-neighbour on the periodic opaque ground plane."""
    equal = np.zeros(rgba.shape[:2], bool)
    for dy in (-1, 0, 1):
        for dx in (-1, 0, 1):
            if dx or dy:
                equal |= (rgba == np.roll(rgba, (dy, dx), axis=(0, 1))).all(2)
    return ~equal & (rgba[:, :, 3] != 0)


def merge_colour_flecks(rgba, protected=0):
    """Merge accidental ground pinpricks into existing neighbouring clusters; never draw new marks."""
    out = rgba.copy()
    removed = 0
    h, w = out.shape[:2]
    for _ in range(8):
        bad = colour_flecks(out)
        if protected:
            bad[:protected] = bad[-protected:] = False
            bad[:, :protected] = bad[:, -protected:] = False
        if not bad.any():
            break
        for y, x in zip(*np.nonzero(bad), strict=True):
            adjacent = np.array([out[(y + dy) % h, (x + dx) % w] for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dx or dy])
            adjacent = adjacent[adjacent[:, 3] != 0]
            if not len(adjacent):
                continue
            values, counts = np.unique(adjacent, axis=0, return_counts=True)
            best = values[counts == counts.max()]
            difference = ((best[:, :3].astype(int) - out[y, x, :3].astype(int)) ** 2).sum(1)
            out[y, x] = best[difference.argmin()]
            removed += 1
    return out, removed


def ground_versions(recipe, source):
    """Three compatible variants quilt only pixels from an independently drawn family source."""
    key = (source, tuple(tuple(chip) for chip in recipe["palette"]))
    if key not in GROUND_INPUTS:
        raw = np.asarray(Image.open(ROOT / source).convert("RGB"))
        grid = tiles.block_size(raw)
        cells = recipe.get("source_cells", 320)
        recovered, loss = tiles.snap(raw, grid if grid > 1 else raw.shape[0] / cells, cells=cells)
        recovered = quantise(np.dstack([recovered, np.full(recovered.shape[:2], 255, np.uint8)]), recipe)[0]
        recovered, before = merge_colour_flecks(recovered)
        if "accent_chip" in recipe:
            accent = tuple(bytes.fromhex(recipe["accent_chip"][1:]))
            middle = tuple(bytes.fromhex(recipe["accent_fallback"][1:]))
            mask = (recovered[:, :, :3] == accent).all(2)
            neighbours = sum(
                np.roll(mask, (dy, dx), axis=(0, 1)).astype(int) for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dx or dy
            )
            # Palette repair retains grouped accent cores, calming isolated bright fringes.
            recovered[mask & (neighbours < 4), :3] = middle
            recovered, extra = merge_colour_flecks(recovered)
            before += extra
        versions = tiles.make_versions_open([recovered[:, :, :3]], 3, 4, 16, 112, 27032, 256)
        cleaned = []
        for version in versions:
            rgba = np.dstack([version, np.full(version.shape[:2], 255, np.uint8)])
            rgba, removed = merge_colour_flecks(rgba)
            cleaned.append((rgba, removed + before))
        # The same inspected border is kept by every variant. No rotation changes blade growth.
        ring = tiles.ring_mask(256, 4)
        shared = cleaned[0][0]
        for _ in range(8):
            equal = np.zeros(ring.shape, bool)
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    if dx or dy:
                        equal |= (shared == np.roll(shared, (dy, dx), axis=(0, 1))).all(2) & np.roll(
                            ring, (dy, dx), axis=(0, 1)
                        )
            bad = ring & ~equal
            if not bad.any():
                break
            for y, x in zip(*np.nonzero(bad), strict=True):
                adjacent = [
                    shared[(y + dy) % 256, (x + dx) % 256]
                    for dy in (-1, 0, 1)
                    for dx in (-1, 0, 1)
                    if (dx or dy) and ring[(y + dy) % 256, (x + dx) % 256]
                ]
                values, counts = np.unique(adjacent, axis=0, return_counts=True)
                shared[y, x] = values[counts.argmax()]
        cleaned[0] = (merge_colour_flecks(shared, protected=4)[0], cleaned[0][1])
        for i in range(1, len(cleaned)):
            cleaned[i][0][ring] = cleaned[0][0][ring]
            cleaned[i] = (merge_colour_flecks(cleaned[i][0], protected=4)[0], cleaned[i][1])
        GROUND_INPUTS[key] = (cleaned, grid, loss, raw.shape[0] / cells)
    return GROUND_INPUTS[key]


def read_json(path):
    return json.loads((ROOT / path).read_text())


def save_json(path, value):
    (ROOT / path).parent.mkdir(parents=True, exist_ok=True)
    (ROOT / path).write_text(json.dumps(value, indent=2) + "\n")


def isolated(mask, wrapping=False):
    """Eight-connected singleton pixels, without confusing image edges with neighbours."""
    padded = np.pad(mask, 1, mode="wrap" if wrapping else "constant")
    neighbours = np.zeros(mask.shape, np.uint8)
    for y in range(3):
        for x in range(3):
            if x != 1 or y != 1:
                neighbours += padded[y : y + mask.shape[0], x : x + mask.shape[1]]
    return mask & (neighbours == 0)


def remove_specks(rgba):
    out = rgba.copy()
    bad = isolated(out[:, :, 3] != 0)
    out[bad] = 0
    return out, int(bad.sum())


def palette(recipe):
    return np.array([tuple(bytes.fromhex(c[1:])) for c, _ in recipe["palette"]], np.uint8)


def quantise(rgba, recipe, material_ids=None):
    """Nearest declared RGB chip, without dithering; reductions preserve the categorical material vote.

    Initial material labels remain provisional visual annotations. Colour cleanup cannot reassign them later.
    """
    chips = palette(recipe)
    chip_ids = np.array([m for _, m in recipe["palette"]], np.uint8)
    visible = rgba[:, :, 3] != 0
    colour = np.zeros_like(rgba)
    colour[:, :, 3] = rgba[:, :, 3]
    ids = np.zeros(rgba.shape[:2], np.uint8)
    pixels = rgba[visible, :3].astype(np.int32)
    chosen = np.empty(len(pixels), np.int32)
    for start in range(0, len(pixels), 16384):
        distance = ((pixels[start : start + 16384, None, :] - chips[None, :, :].astype(np.int32)) ** 2).sum(2)
        if material_ids is not None:
            kept_ids = material_ids[visible][start : start + len(distance)]
            if not np.isin(kept_ids, chip_ids).all():
                raise ValueError("a material has no declared palette ramp")
            distance = np.where(kept_ids[:, None] == chip_ids[None, :], distance, 1_000_000)
        chosen[start : start + len(distance)] = distance.argmin(1)
    colour[visible, :3] = chips[chosen]
    ids[visible] = chip_ids[chosen]
    return colour, ids


def normals(alpha, ground=False, shape="billboard"):
    """Upward ground or a mild silhouette proxy. Brightness is never treated as physical relief.

    Rounded boulder and conical roof proxies add broad response; their strength stays deliberately mild.
    This is shading metadata, not a depth, caster or collision record.
    """
    h, w = alpha.shape
    vectors = np.zeros((h, w, 3), np.float64)
    if ground:
        vectors[:, :, 2] = 1
    elif shape == "dome":
        ys, xs = np.nonzero(alpha)
        if len(xs):
            x = (np.arange(w)[None, :] - (xs.min() + xs.max()) / 2) / max(1, (xs.max() - xs.min()) / 2)
            y = np.clip((np.arange(h)[:, None] - ys.min()) / max(1, ys.max() - ys.min()), 0, 1)
            # A broad dome turns toward its flanks, upward at the top and south at its front.
            # This annotated geometry proxy is independent of granite colour and its fissure.
            vectors[:, :, 0] = 0.60 * x
            vectors[:, :, 1] = 0.25 + 0.8 * y
            vectors[:, :, 2] = 1.0 - 0.65 * y
            vectors /= np.maximum(np.linalg.norm(vectors, axis=2, keepdims=True), 1e-10)
    else:
        soft = np.asarray(Image.fromarray(alpha).filter(ImageFilter.GaussianBlur(2)), np.float64) / 255
        gy, gx = np.gradient(soft)
        gx, gy = gx * 0.3, gy * 0.3
        if shape in ("rounded", "cone"):
            ys, xs = np.nonzero(alpha)
            if len(xs):
                x = (np.arange(w)[None, :] - (xs.min() + xs.max()) / 2) / max(1, (xs.max() - xs.min()) / 2)
                y = (np.arange(h)[:, None] - ys.min()) / max(1, ys.max() - ys.min())
                gx = gx - 0.2 * np.clip(x, -1, 1)
                gy = gy + (0.2 * (1 - np.clip(y, 0, 1)) if shape == "rounded" else 0.12)
        vectors[:, :, 0] = -gx
        vectors[:, :, 1] = COS37 - gy * SIN37
        vectors[:, :, 2] = SIN37 + gy * COS37
        vectors /= np.linalg.norm(vectors, axis=2, keepdims=True)
    out = np.dstack([np.rint((vectors + 1) * 127.5).astype(np.uint8), alpha])
    out[alpha == 0] = 0
    return out


def material_page(ids, alpha):
    """R = exact material ID; G and B = zero; A = colour coverage. No colour interpretation."""
    zero = np.zeros_like(ids)
    return np.dstack([ids, zero, zero, alpha])


def birch_stem_profile(pivot, ppm, count):
    """Continuous registered axis follows inspected exposed stem, shared by fitting and normals."""
    rows = np.arange(count)
    rise = (pivot[1] - rows) / (COS37 * ppm)
    # Near-source winter axis: x504 at y190/240, x509 at300, x510 at410, x512 at500.
    offset = (
        np.interp(rise, [0, 10, 12, 14, 15, 16, 20], [0, 0, -0.031, -0.047, -0.078, -0.125, -0.125])
        if ppm >= 32
        else np.zeros(count)
    )
    shaft = max(1, round(0.25 * ppm))
    # Taper the upper fork rather than introduce a broad rectangular ledge.
    widths = np.rint(np.interp(rise, [0, 14, 17, 20], [shaft, shaft, max(1, 0.16 * ppm), max(1, 0.10 * ppm)])).astype(
        int
    )
    left = np.rint(pivot[0] + offset * ppm - widths / 2).astype(int)
    centres = left + (widths - 1) / 2
    return left, widths, centres


def birch_repair(recipe, colour, ids, pivot, ppm, family):
    """Registered wood samples and reviewed semantic regions repair the original tree (PRE-20, PRE-46)."""
    source = recipe["shared_wood_sources"][family]
    key = (source, family)
    woody_recipe = next(r for r in read_json(RECIPE)["fixtures"] if r["id"] == "birch-winter").copy()
    woody_recipe["birch_repair"] = False
    if key not in WOODY_INPUTS:
        canonical, _ = source_bundle(woody_recipe, source, family, save_cleaned=False)
        WOODY_INPUTS[key] = canonical["colour"]
    wood = WOODY_INPUTS[key]
    repaired = colour.copy()
    collar = max(2, round(0.38 * ppm))
    start = max(0, round(pivot[1] - 17 * COS37 * ppm))
    stem_left, stem_widths, stem_centres = birch_stem_profile(pivot, ppm, len(colour))
    window = max(collar * 2, round(0.75 * ppm))
    x0, x1 = round(pivot[0]) - window, round(pivot[0]) + window
    # Existing row samples are fitted through the exposed stem. Side intervals retain
    # their outer branch contacts while meeting the narrowed core: no nine-metre splice.
    for y in range(start, int(np.ceil(pivot[1]))):
        line = wood[y, x0:x1]
        centre = round(stem_centres[y]) - x0
        old_radius = max(1, round({"near": 0.27, "middle": 0.27, "far": 0.50}[family] * ppm))
        core = line[centre - old_radius : centre + old_radius + 1]
        visible = np.nonzero(core[:, 3])[0]
        if not len(visible):
            continue
        sample = core[visible.min() : visible.max() + 1]
        rise = pivot[1] - y
        extent = stem_widths[y] if rise > max(1, 0.45 * COS37 * ppm) else collar
        row = np.asarray(Image.fromarray(sample[None, :]).resize((extent, 1), Image.Resampling.NEAREST))[0]
        place = stem_left[y] if extent != collar else round(pivot[0] - extent / 2)
        leaves_here = ids[y, x0:x1] == 3
        if rise > 9 * COS37 * ppm:
            old = repaired[y, x0:x1].copy()
            old_ids = ids[y, x0:x1].copy()
            # Material labels were established on neutral raw bark before quantisation.
            core_lo, core_hi = centre - old_radius, centre + old_radius + 1
            samples = np.interp(
                np.arange(x1 - x0),
                [0, place - x0, place - x0 + extent - 1, x1 - x0 - 1],
                [0, core_lo, core_hi - 1, x1 - x0 - 1],
            )
            samples = np.rint(samples).astype(int)
            existing_bark = old_ids == 4
            mapped_bark = (old_ids[samples] == 4) & ~leaves_here
            repaired[y, x0:x1][existing_bark] = 0
            ids[y, x0:x1][existing_bark] = 0
            # Keep authored foliage pixels fixed; only woody samples are rescaled.
            repaired[y, x0:x1][mapped_bark] = old[samples][mapped_bark]
            ids[y, x0:x1][mapped_bark] = 4
        else:
            repaired[y, x0:x1] = 0
            ids[y, x0:x1] = 0
        exposed = ids[y, place : place + extent] != 3
        repaired[y, place : place + extent][exposed] = row[exposed]
        ids[y, place : place + extent][exposed] = 4
    visible = repaired[:, :, 3] > 0
    # Lower wood is semantic bark including dark scars. Exposed upper wood is annotated by structure.
    rows, columns = np.indices(visible.shape)
    lower = (rows >= pivot[1] - 9 * COS37 * ppm) & (np.abs(columns - pivot[0]) <= window)
    ids[lower & visible] = 4
    if recipe["id"] == "birch-summer":
        # Suppress extreme leaf ramps retained by the edit; moderate intrinsic green clusters remain.
        low = tuple(bytes.fromhex("566638"))
        high = tuple(bytes.fromhex("969F58"))
        leaves = ids == 3
        repaired[leaves & (repaired[:, :, 1] > 159), :3] = high
        repaired[leaves & (repaired[:, :, 1] < 102), :3] = low
    repaired, ids = quantise(repaired, recipe, ids)
    return repaired, ids, source


def birch_normals(colour, ids, pivot, ppm):
    """Cylinder bark and annotated drooping spray volumes; no colour-to-relief conversion (PRE-20)."""
    alpha = colour[:, :, 3]
    rows, columns = np.indices(alpha.shape)
    vectors = np.zeros((*alpha.shape, 3), float)
    vectors[:, :, 1] = 1
    # A registered local shaft cylinder cannot be tilted by remote branches on its row.
    stem_left, widths, centres = birch_stem_profile(pivot, ppm, len(colour))
    shaft_centre = centres[:, None]
    shaft_radius = np.maximum(0.5, (widths[:, None] - 1) / 2)
    stem = (ids == 4) & (np.abs(columns - shaft_centre) <= shaft_radius)
    radial = np.clip((columns - shaft_centre) / np.maximum(1, shaft_radius), -1, 1)
    vectors[stem, 0] = (0.45 * radial)[stem]
    vectors[stem, 1] = np.sqrt(1 - (0.45 * radial[stem]) ** 2)
    vectors[stem, 2] = 0.08
    # Disconnected branch row runs get their own restrained cylinder, not one full-row fit.
    for y in range(alpha.shape[0]):
        xs = np.nonzero((ids[y] == 4) & ~stem[y])[0]
        for run in np.split(xs, np.nonzero(np.diff(xs) > 1)[0] + 1):
            if not len(run):
                continue
            centre = (run.min() + run.max()) / 2
            local = (run - centre) / max(1, (run.max() - run.min()) / 2)
            vectors[y, run, 0] = 0.18 * local
            vectors[y, run, 1] = np.sqrt(1 - (0.18 * local) ** 2)
            vectors[y, run, 2] = 0.08
    # Volumes are reviewed normalized locations of the source's six visible drooping spray groups.
    top = pivot[1] - 20 * COS37 * ppm
    height = 20 * COS37 * ppm
    sprays = (
        (0, 0.13, 1.5, 0.16),
        (-2.05, 0.30, 1.45, 0.16),
        (2.0, 0.37, 1.5, 0.16),
        (-2.05, 0.48, 1.45, 0.15),
        (2.15, 0.57, 1.5, 0.17),
        (-2.0, 0.65, 1.5, 0.17),
    )
    distances = []
    for x, y, rx, ry in sprays:
        distances.append(
            ((columns - pivot[0] - x * ppm) / (rx * ppm)) ** 2 + ((rows - top - y * height) / (ry * height)) ** 2
        )
    # Smooth inverse-distance weights eliminate nearest-proxy seams inside one leaf spray.
    weights = np.exp(-np.minimum(np.asarray(distances), 50) * 1.5)
    weights /= np.maximum(weights.sum(0), 1e-20)
    canopy = np.zeros_like(vectors)
    for i, (x, y, rx, ry) in enumerate(sprays):
        local_x = np.clip((columns - pivot[0] - x * ppm) / (rx * ppm), -1, 1)
        local_y = np.clip((rows - top - y * height) / (ry * height), -1, 1)
        canopy[:, :, 0] += weights[i] * 0.35 * local_x
        canopy[:, :, 1] += weights[i] * (0.65 + 0.20 * local_y)
        canopy[:, :, 2] += weights[i] * (0.65 - 0.20 * local_y)
    vectors[ids == 3] = canopy[ids == 3]
    vectors /= np.maximum(np.linalg.norm(vectors, axis=2, keepdims=True), 1e-10)
    result = np.dstack([np.rint((vectors + 1) * 127.5).astype(np.uint8), alpha])
    result[alpha == 0] = 0
    return result


def resize_drawn(rgba, size, recipe):
    """Area blocks preserve thin connected twigs; categorical votes keep a mixed edge's material real.

    Colour is premultiplied by Pillow only during the area reduction, then snapped back to a voted ramp.
    Final coverage is binary. No transparent black or filtered material ID becomes visible colour.
    """
    colour, ids = quantise(rgba, recipe)
    if recipe.get("birch_repair"):
        # Reviewed raw bark ramp is neutral beige/grey, including charcoal scars.
        # Its low chroma distinguishes it before dark scars can snap into a leaf chip.
        raw = rgba[:, :, :3].astype(np.int16)
        rows, columns = np.indices(rgba.shape[:2])
        contacts = np.nonzero(rgba[-max(1, len(rgba) // 100) :, :, 3])[1]
        base = (contacts.min() + contacts.max()) / 2
        # This source annotation covers the exposed shaft only. Muted leaf colours
        # elsewhere are never sufficient evidence for bark, even beside a winter twig.
        shaft_region = (rows >= len(rgba) * 0.20) & (np.abs(columns - base) <= max(1.0, len(rgba) * 0.025))
        bark = (
            shaft_region
            & (rgba[:, :, 3] > 0)
            & (raw[:, :, 1] - raw[:, :, 0] < 8)
            & (np.abs(raw[:, :, 0] - raw[:, :, 2]) < 45)
        )
        ids[bark] = 4
        colour, ids = quantise(rgba, recipe, ids)
    reduced = np.asarray(Image.fromarray(colour).resize(size, Image.Resampling.BOX)).copy()
    reduced[:, :, 3] = np.where(reduced[:, :, 3] > 0, 255, 0)
    candidates = sorted({m for _, m in recipe["palette"]})
    votes = np.stack(
        [
            np.asarray(
                Image.fromarray(((ids == m) & (rgba[:, :, 3] > 0)).astype(np.float32)).resize(
                    size, Image.Resampling.BOX
                )
            )
            for m in candidates
        ],
        axis=2,
    )
    labels = np.array(candidates, np.uint8)[votes.argmax(2)]
    return quantise(reduced, recipe, labels)[0]


def halve_bundle(bundle):
    """Reduce aligned 2x2 blocks: foreground medians, categorical votes, alpha-weighted unit normals.

    Any coverage preserves thin limbs in a study. This is deliberately not a substitute for authored far art.
    """
    colour, material, normal = [bundle[k] for k in KINDS]
    n = colour.shape[0]
    if n == 1:
        raise ValueError("cannot halve a one-pixel page")
    blocks = [
        a.reshape(n // 2, 2, n // 2, 2, 4).transpose(0, 2, 1, 3, 4).reshape(n // 2, n // 2, 4, 4)
        for a in (colour, material, normal)
    ]
    c, m, v = blocks
    visible = c[:, :, :, 3] != 0
    alpha = visible.any(2).astype(np.uint8) * 255
    # Integer foreground median, choosing a source value rather than inventing a palette entry.
    ordered = np.sort(np.where(visible[:, :, :, None], c[:, :, :, :3].astype(np.int16), 256), axis=2)
    which = np.maximum(0, (visible.sum(2) - 1) // 2)
    rgb = np.take_along_axis(ordered, which[:, :, None, None], axis=2)[:, :, 0, :]
    rgb[alpha == 0] = 0
    reduced_colour = np.dstack([rgb.astype(np.uint8), alpha])
    # Votes exclude transparent pixels. Ties choose a represented ID; no ID is ever averaged.
    counts = (m[:, :, :, 0, None] == m[:, :, None, :, 0]) & visible[:, :, None, :]
    votes = counts.sum(3) * visible
    choice = votes.argmax(2)
    ids = np.take_along_axis(m[:, :, :, 0], choice[:, :, None], axis=2)[:, :, 0]
    ids[alpha == 0] = 0
    vec = ((v[:, :, :, :3].astype(np.float64) / 127.5 - 1) * visible[:, :, :, None]).sum(2)
    length = np.linalg.norm(vec, axis=2, keepdims=True)
    vec /= np.maximum(length, 1e-10)
    vec[alpha == 0] = [0, COS37, SIN37]
    reduced_normal = np.dstack([np.rint((vec + 1) * 127.5).astype(np.uint8), alpha])
    reduced_normal[alpha == 0] = 0
    return dict(zip(KINDS, (reduced_colour, material_page(ids, alpha), reduced_normal), strict=True))


def source_bundle(recipe, source, family="near", save_cleaned=True):
    """Snap generated colour to its measured grid, palette and physical span; never invent silhouettes.

    Approximate source blocks are explicitly distinguished from exact ones. Normals use a mild silhouette
    proxy, not a brightness-to-height conversion. Their numeric basis needs the engine calibration review.
    """
    im = Image.open(ROOT / source).convert("RGBA")
    rgba = np.asarray(im).copy()
    rgba[:, :, 3] = np.where(rgba[:, :, 3] >= 128, 255, 0)
    grid = tiles.block_size(rgba)
    ppm = {"near": 64, "middle": 16, "far": 4}[family]
    if recipe["class"] == "ground":
        versions, grid, loss, block = ground_versions(recipe, source)
        rgba, flecks = versions[recipe.get("ground_variant", 0)]
        rgba = rgba.copy()
        w, h = im.size
        crop, pivot, scale = [0, 0, w, h], [0.0, 0.0], 256 / w
    else:
        rgba, _ = remove_specks(rgba)
        crop = Image.fromarray(rgba).getbbox()
        if crop is None:
            raise ValueError(f"{source}: empty drawing")
        rgba = rgba[crop[1] : crop[3], crop[0] : crop[2]]
        drawn = rgba.copy()
        if recipe["id"] == "tent":
            # The inspected doorway's dark fill is empty space, not a permanently black surface.
            yy, xx = np.indices(rgba.shape[:2])
            x, y = xx / rgba.shape[1], yy / rgba.shape[0]
            opening = (x > 0.44) & (x < 0.54) & (y > 0.58) & (y < 0.94)
            rgba[opening & (rgba[:, :, :3].max(2) < 90)] = 0
        span = recipe["metres"] * ppm * (COS37 if recipe["measure"] == "vertical" else 1)
        scale = span / (rgba.shape[0] if recipe["measure"] == "vertical" else rgba.shape[1])
        size = [max(1, round(rgba.shape[1] * scale)), max(1, round(rgba.shape[0] * scale))]
        if "crown_width_m" in recipe:
            size[0] = round(recipe["crown_width_m"] * ppm)
        if "height_m" in recipe:
            # These fixtures annotate the top at the ground centre; the root is the front contact.
            rise = COS37 * recipe["height_m"] + SIN37 * recipe["anchor_ground_offset_m"][1]
            size[1] = round(rise * ppm)
        rgba = resize_drawn(rgba, size, recipe)
        reconstructed = np.asarray(
            Image.fromarray(rgba).resize((drawn.shape[1], drawn.shape[0]), Image.Resampling.NEAREST)
        )
        visible = drawn[:, :, 3] > 0
        loss = float((np.abs(reconstructed[visible, :3].astype(np.int16) - drawn[visible, :3]).sum(1) > 24).mean())
        n = recipe["canvas"] // {"near": 1, "middle": 4, "far": 16}[family]
        # Same metre anchor and gutter in every family; root sits at the front ground contact.
        gutter = recipe.get("gutter", 4) / {"near": 1, "middle": 4, "far": 16}[family]
        pivot = [n / 2, n - gutter]
        contacts = np.nonzero(rgba[-max(1, size[1] // 100) :, :, 3])[1]
        base_x = (float(contacts.min()) + float(contacts.max()) + 1) / 2
        place = [round(pivot[0] - base_x), round(pivot[1] - size[1])]
        if min(place) < 0 or place[0] + size[0] > n or place[1] + size[1] > n:
            raise ValueError(f"{source}: measured sprite does not fit its page")
        page = np.zeros((n, n, 4), np.uint8)
        page[place[1] : place[1] + size[1], place[0] : place[0] + size[0]] = rgba
        rgba = page
        block = grid
    rgba, removed = remove_specks(rgba)
    colour, ids = quantise(rgba, recipe)
    if recipe.get("quiet_colour_flecks"):
        colour, _ = merge_colour_flecks(colour)
        colour, ids = quantise(colour, recipe, ids)
    if recipe.get("birch_repair"):
        colour, ids, _ = birch_repair(recipe, colour, ids, pivot, ppm, family)
    if recipe["id"] == "tent":
        ys, xs = np.nonzero(colour[:, :, 3])
        top = ys.min() + (ys.max() - ys.min()) * 0.25
        # Hide panels and wood share brown chips. Pole height is an explicit semantic annotation.
        rows = np.arange(colour.shape[0])[:, None]
        ids[(ids == 5) & (rows >= top)] = 7
        columns = np.arange(colour.shape[1])[None, :]
        poles = (rows < top) & (np.abs(columns - pivot[0]) < 0.10 * recipe["metres"] * ppm)
        ids[poles & (colour[:, :, 3] != 0)] = 5
        colour, ids = quantise(colour, recipe, ids)
    shift = [0, 0]
    bundle = {
        "colour": colour,
        "material": material_page(ids, colour[:, :, 3]),
        "normal": birch_normals(colour, ids, pivot, ppm)
        if recipe.get("birch_repair")
        else normals(colour[:, :, 3], recipe["class"] == "ground", recipe.get("normal_shape", "billboard")),
    }
    suffix = "-cleaned" if not recipe.get("ground_variant", 0) else f"-v{recipe['ground_variant'] + 1}-cleaned"
    cleaned_path = source.replace("-original", suffix)
    if save_cleaned:
        Image.fromarray(colour).save(ROOT / cleaned_path)
    return bundle, {
        "exact_source_block": grid,
        "cleanup_block": block,
        "block_loss": loss if recipe["class"] == "ground" else None,
        "palette_and_scale_loss": loss if recipe["class"] != "ground" else None,
        "repeat_cut_shift": shift,
        "source_grid_review": "exact blocks" if grid > 1 else "nonuniform source; snapped to target grid",
        "scale": scale,
        "crop": list(crop),
        "removed_singletons": removed,
        "merged_colour_flecks": flecks if recipe["class"] == "ground" else 0,
        "cleaned": cleaned_path,
        "pivot": pivot,
    }


def write_chain(recipe, family, chain, sources):
    ppm = {"near": 64, "middle": 16, "far": 4}[family]
    band = {"near": 0, "middle": 2, "far": 4}[family]
    files = {}
    for kind in KINDS:
        catalogue_name = recipe["id"].replace("-", "_")
        where = f"art/textures/fixtures27/{catalogue_name}/{family}/{kind}"
        (ROOT / where).mkdir(parents=True, exist_ok=True)
        levels, prior = [], ""
        for i, bundle in enumerate(chain):
            rel = f"{where}/b{band + i}.png"
            tiles.write_png(ROOT / rel, bundle[kind])
            digest = tiles.sha256(ROOT / rel)
            levels.append(
                {
                    "level": i,
                    "file": rel,
                    "sha256": digest,
                    "made_from": prior,
                    "way": "image-drawn family; block cleanup and palette snap"
                    if i == 0
                    else "aligned 2x2 reduction study; categorical IDs voted; normals renormalised",
                }
            )
            prior = digest
        fields = {
            "about": f"T2.7a.3 {recipe['id']} {kind}: {recipe['status']}; authored {family} family",
            "route": "picture" if kind == "colour" else "code",
            "tile_texels": chain[0][kind].shape[0],
            "texels_a_metre": ppm,
            "first_band": band,
            "sources": sources,
            "original_sha256": [tiles.sha256(ROOT / s) for s in sources],
            "c2pa": ["present" if ingest.has_c2pa(ROOT / s) else "absent" for s in sources],
            "requests": [REQUEST]
            + [p["request"] for p in read_json("art/sources/fixtures27/provenance.json") if p["file"] in sources],
            "made": "image drawing; tools/art/fixtures.py cleanup; " + recipe["status"],
            "regrid_loss": "block loss, exact-grid detection and source scale recorded in exports.json",
            "truth": "8 October 2026: existing sheet design; this export requires the art and engine reviews",
            "approved": "pending owner runtime review and engine lighting calibration",
        }
        (ROOT / where / "record.toml").write_text(textures.record_text(fields, levels))
        files[kind] = where + "/record.toml"
    return files


def complete_chain(recipe, bundle):
    """Reviewed block reductions: retain only real material ramps and unit vectors at every level."""
    chain = [bundle]
    while chain[-1]["colour"].shape[0] > 1:
        smaller = halve_bundle(chain[-1])
        if recipe.get("birch_repair") and smaller["colour"].shape[0] > 1:
            smaller["colour"], _ = remove_specks(smaller["colour"])
            absent = smaller["colour"][:, :, 3] == 0
            smaller["material"][absent] = 0
            smaller["normal"][absent] = 0
        colour, ids = quantise(smaller["colour"], recipe, smaller["material"][:, :, 0])
        smaller["colour"] = colour
        if (recipe["class"] == "ground" or recipe.get("quiet_colour_flecks")) and colour.shape[0] > 2:
            colour, _ = merge_colour_flecks(colour)
            colour, ids = quantise(colour, recipe, ids)
            smaller["colour"] = colour
        smaller["material"] = material_page(ids, colour[:, :, 3])
        chain.append(smaller)
    return chain


def preview(recipe, family, bundle, pivot, chain):
    """Catalogue page style, source picture only; labels and sticks are code, never substitute drawings."""
    colour = Image.fromarray(bundle["colour"])
    box = colour.getbbox()
    cut = colour.crop(box)
    page = sheet.Sheet()
    page.text(recipe["id"] + " — " + family, 36, bold=True)
    page.text(recipe["status"] + ". Runtime approval pending.", 24)
    ppm = {"near": 64, "middle": 16, "far": 4}[family]
    page.text(f"{ppm} internal pixels a metre. 2x phone presentation. Root {pivot}.", 22)
    for k in (1, 2):
        shown = cut.resize((cut.width * k, cut.height * k), Image.Resampling.NEAREST)
        metres = 10 if ppm == 4 else 1  # ten striped segments must each occupy at least a pixel
        gauge = (
            sheet.lying(round(ppm * k * metres), "")
            if recipe["class"] == "ground"
            else sheet.upright(round(COS37 * ppm * k * metres), "")
        )
        if shown.width + gauge.width + sheet.GAP <= sheet.WIDTH - 2 * sheet.MARGIN:
            page.figures([(shown, f"Colour at {k}x"), (gauge, f"{metres} m")])
    # The numeric page previews share the identical crop and a common integer enlargement.
    for kind in ("material", "normal"):
        shown_map = bundle[kind]
        label = kind
        if kind == "material":
            # Review colours only. The exported IDs remain exact numeric R values.
            chips = np.array(
                [
                    [0, 0, 0],
                    [70, 125, 65],
                    [165, 110, 65],
                    [85, 155, 115],
                    [220, 220, 190],
                    [135, 85, 45],
                    [120, 135, 155],
                    [205, 150, 70],
                    [240, 210, 150],
                ],
                np.uint8,
            )
            shown_map = np.dstack([chips[shown_map[:, :, 0]], shown_map[:, :, 3]])
            label = "Material IDs shown in review colours; saved map is numeric"
        im = Image.fromarray(shown_map).crop(box)
        k = max(1, min(4, (sheet.WIDTH - 2 * sheet.MARGIN) // im.width))
        page.figures([(im.resize((im.width * k, im.height * k), Image.Resampling.NEAREST), label)])
    page.text("Numeric material IDs. Mild silhouette normals in world east, south, up; ground faces up.", 22)
    page.text("Noon, dusk, slopes, water and shelter need the running engine review.", 22)
    if len(chain) > 1:
        smaller = Image.fromarray(chain[1]["colour"])
        smaller = smaller.crop(smaller.getbbox())
        page.figures(
            [
                (
                    smaller.resize((smaller.width * 2, smaller.height * 2), Image.Resampling.NEAREST),
                    f"Intermediate {ppm // 2} px/m, 2x phone presentation",
                )
            ]
        )
    if recipe["class"] == "ground":
        repeat = Image.fromarray(np.tile(bundle["colour"], (3, 3, 1)))
        page.pictures([(repeat, "3 x 3 world-space repeat, no camera rounding")])
        projected = project_ground(bundle["colour"])
        page.figures(
            [
                (
                    Image.fromarray(projected).resize((512, projected.shape[0] * 2), Image.Resampling.NEAREST),
                    f"37 degree plane at exact 2x; {recipe['ground_span_m'][family]} m across",
                ),
                (metre_gauge(ppm * 2), "1 m east"),
            ]
        )
    out = f"{PREVIEW}/{recipe['id']}-{family}.webp"
    page.render().save(ROOT / out, lossless=True, method=6)
    return out


def project_ground(rgba):
    """Sample the ground projection once on the internal grid, then present at integer 2x (PRE-22)."""
    rows = np.minimum((np.arange(int(np.ceil(rgba.shape[0] * SIN37))) + 0.5) / SIN37, rgba.shape[0] - 1).astype(int)
    return rgba[rows]


def metre_gauge(length, vertical=False):
    """A one-metre technical gauge stays legible even when far art gives it fewer than ten pixels."""
    im = Image.new("RGB", (12, length) if vertical else (length, 12), sheet.GROUND)
    draw = ImageDraw.Draw(im)
    segments = min(10, length)
    for i in range(segments):
        lo, hi = round(i * length / segments), round((i + 1) * length / segments) - 1
        box = (0, lo, 11, hi) if vertical else (lo, 0, hi, 11)
        draw.rectangle(box, fill=sheet.INK if i % 2 else (225, 213, 185))
    return im


def light_study(recipe, family, bundle):
    """Technical normal probes relight existing colour; these are not engine evidence (PRE-20)."""
    colour = bundle["colour"]
    vec = bundle["normal"][:, :, :3].astype(float) / 127.5 - 1
    page = sheet.Sheet()
    page.text(f"{recipe['id']} {family}: technical directional-light study", 32, bold=True)
    page.text("Offline Lambert probe only. Not engine noon/dusk, shadows, contacts or approval.", 22)
    page.text("World east/south/up. Constant 35% ambient + 65% diffuse. Neutral colour is unchanged.", 22)
    box = Image.fromarray(colour).getbbox()
    studies = []
    for label, direction in (("East", (0.85, 0.25, 0.45)), ("West", (-0.85, 0.25, 0.45)), ("Overhead", (0, 0, 1))):
        light = np.array(direction, float)
        light /= np.linalg.norm(light)
        strength = 0.35 + 0.65 * np.maximum(0, np.sum(vec * light, axis=2))
        shown = colour.copy()
        shown[:, :, :3] = np.rint(colour[:, :, :3] * strength[:, :, None]).astype(np.uint8)
        im = Image.fromarray(shown).crop(box)
        studies.append((im.resize((im.width * 2, im.height * 2), Image.Resampling.NEAREST), label + " at 2x"))
    page.figures(studies)
    for backdrop, label in (((35, 38, 40), "dark"), ((236, 232, 224), "light")):
        im = Image.fromarray(colour).crop(box)
        bg = Image.new("RGB", im.size, backdrop)
        bg.paste(im, (0, 0), im)
        page.figures(
            [
                (
                    bg.resize((bg.width * 2, bg.height * 2), Image.Resampling.NEAREST),
                    f"Neutral colour, {label} edge check at 2x",
                )
            ]
        )
    out = f"{PREVIEW}/{recipe['id']}-{family}-light-study.webp"
    page.render().save(ROOT / out, lossless=True, method=6)
    return out


def ground_review(recipes, exports):
    """Keep full, uncropped mixed/single repeats at exact 2x, with projected metre gauges (PRE-22)."""
    for family, ppm in (("near", 64), ("middle", 16), ("far", 4)):
        tiles_drawn = []
        for recipe in recipes:
            name = recipe["id"].replace("-", "_")
            band = dict(near=0, middle=2, far=4)[family]
            path = f"art/textures/fixtures27/{name}/{family}/colour/b{band}.png"
            tiles_drawn.append(np.asarray(Image.open(ROOT / path).convert("RGBA")))
        for mode in ("single", "mixed"):
            layout = ((0, 0, 0),) * 3 if mode == "single" else ((0, 1, 0), (2, 0, 1), (1, 2, 2))
            repeat = np.concatenate([np.concatenate([tiles_drawn[i] for i in row], axis=1) for row in layout], axis=0)
            plane = Image.fromarray(project_ground(repeat))
            plane = plane.resize((plane.width * 2, plane.height * 2), Image.Resampling.NEAREST)
            canvas = Image.new("RGB", (plane.width + 160, plane.height + 130), sheet.GROUND)
            draw = ImageDraw.Draw(canvas)
            span = recipes[0]["ground_span_m"][family]
            draw.text(
                (20, 12),
                f"meadow {family}, {mode} 3x3 — {span} m tiles, {span * 3} m across",
                fill=sheet.INK,
                font=sheet.font(24, True),
            )
            draw.text(
                (20, 46),
                "37 degree projection once; internal nearest sampling; exact 2x squares.",
                fill=sheet.INK,
                font=sheet.font(22),
            )
            canvas.paste(plane, (0, 90))
            canvas.paste(metre_gauge(2 * ppm), (20, plane.height + 102))
            draw.text((2 * ppm + 36, plane.height + 102), "1 m east", fill=sheet.INK, font=sheet.font(20))
            canvas.paste(metre_gauge(round(2 * ppm * SIN37), vertical=True), (plane.width + 26, 160))
            draw.text((plane.width + 12, 112), "1 m south", fill=sheet.INK, font=sheet.font(19))
            canvas.save(ROOT / f"{PREVIEW}/meadow-{family}-{mode}-projected-2x.png")
        # Cross-variant joins are numerical checks; no new colour is invented for these previews.
        for first in tiles_drawn:
            for second in tiles_drawn:
                joined = np.concatenate([first, second], axis=1)
                if colour_flecks(joined).any():
                    raise ValueError(f"{family}: colour flecks at a variant join")
    for entry in exports["entries"]:
        if entry["id"].startswith("meadow"):
            for f in entry["families"]:
                f["projected_review"] = [
                    f"{PREVIEW}/meadow-{f['family']}-{mode}-projected-2x.png" for mode in ("single", "mixed")
                ]
                f["mixed_preview_layout"] = [[0, 1, 0], [2, 0, 1], [1, 2, 2]]


def split_parts(recipe, bundle, pivot, ppm):
    """Cut already drawn pixels into ordering pieces, sharing the same root and whole-page coordinates."""
    visible = bundle["colour"][:, :, 3] > 0
    y, x = np.indices(visible.shape)
    if recipe["id"].startswith("birch"):
        trunk = visible & (y >= pivot[1] - 7 * COS37 * ppm) & (np.abs(x - pivot[0]) <= ppm * 0.5)
        masks = {"trunk": trunk, "crown": visible & ~trunk}
    elif recipe["id"] == "tent":
        rows = np.nonzero(visible)[0]
        front = visible & (bundle["material"][:, :, 0] == 6) & (y >= rows.min() + 0.82 * (rows.max() - rows.min()))
        masks = {"front": front, "roof": visible & ~front}
    else:
        return {}
    pieces = {}
    for name, mask in masks.items():
        piece = {}
        for kind, array in bundle.items():
            page = array.copy()
            page[~mask] = 0
            piece[kind] = page
        pieces[name] = piece
    return pieces


def build_designs():
    """Compose pending designs from image-drawn boards using the catalogue's shared sheet builder.

    Implements PRE-27, PRE-44 and PRE-46. These are approval inputs, never runtime bodies or walk frames.
    """
    for design in read_json(DESIGNS):
        folder = ROOT / design["folder"]
        for source in design["boards"]:
            raw = Image.open(folder / source["original"]).convert("RGB")
            rgba = sheet.clear(raw)
            rgba, _ = remove_specks(rgba)
            colour, _ = quantise(rgba, {"palette": [[c, 1] for _, c in design["palette"]]})
            board = colour[:, :, :3].copy()
            board[colour[:, :, 3] == 0] = sheet.KEY
            Image.fromarray(board).save(folder / source["original"].replace("-original", "-cleaned"))
            for cell in source["cells"]:
                box = [round(v * raw.width) if i % 2 == 0 else round(v * raw.height) for i, v in enumerate(cell["box"])]
                cut = Image.fromarray(board).crop(box)
                if "logical_height" in cell:
                    rgba = sheet.clear(cut)
                    im = Image.fromarray(rgba).crop(Image.fromarray(rgba).getbbox())
                    size = (max(1, round(im.width * cell["logical_height"] / im.height)), cell["logical_height"])
                    im = im.resize(size, Image.Resampling.NEAREST)
                    cut = Image.new("RGB", (im.width + 4, im.height + 4), tuple(sheet.KEY))
                    cut.paste(im, (2, 2), im)
                cut.save(folder / cell["file"])
        spec = json.loads((folder / "spec.json").read_text())
        spec["pixel_art"] = True
        spec["camera_title"] = (
            "Fixed north camera, 37 degrees above the horizon, neutral light. Actor directions change."
        )
        spec["camera_objects"]["phone_sizes"] = False
        scale = sheet.object_scale(spec, lambda name, folder=folder: sheet.cut_out(Image.open(folder / name), least=1))
        for group in spec.get("groups", []):
            if group.get("scale") == "phone":
                group["scale"] = 128 / scale
        (folder / "spec.json").write_text(json.dumps(spec, indent=2) + "\n")
        sheet.compose(spec, str(folder)).save(ROOT / design["sheet"], lossless=True, method=6)
        print("Prepared owner design sheet: " + design["sheet"])


def build(only=None):
    exports = {
        "normal_basis": NORMAL_BASIS,
        "material_encoding": "R exact ID, G=B=0, A colour coverage",
        "approval": "no runtime or engine approval",
        "entries": [],
    }
    recipes = read_json(RECIPE)["fixtures"]
    if only:
        exports["entries"] = [e for e in read_json(MANIFEST)["entries"] if e["id"] not in only]
    for recipe in recipes:
        if only and recipe["id"] not in only:
            continue
        families = []
        starts = []
        for family in ("near", "middle", "far"):
            bundle, detail = source_bundle(recipe, recipe["sources"][family], family)
            starts.append((family, bundle, detail))
        for family, bundle, detail in starts:
            # Requantise medians to the declared palette and remove accidental foreground singletons.
            cleaned, _ = remove_specks(bundle["colour"])
            colour, ids = quantise(cleaned, recipe, bundle["material"][:, :, 0])
            bundle = {
                "colour": colour,
                "material": material_page(ids, colour[:, :, 3]),
                "normal": np.dstack([bundle["normal"][:, :, :3], colour[:, :, 3]]),
            }
            bundle["normal"][colour[:, :, 3] == 0] = 0
            chain = complete_chain(recipe, bundle)
            sources = list(recipe["sources"].values()) if "sources" in recipe else [recipe["source"]]
            if recipe.get("birch_repair"):
                sources = list(dict.fromkeys(sources + list(recipe["shared_wood_sources"].values())))
            records = write_chain(recipe, family, chain, sources)
            parts = {}
            for name in split_parts(recipe, bundle, detail["pivot"], {"near": 64, "middle": 16, "far": 4}[family]):
                # Slice the already reduced whole at each level: independently reducing parts can overlap.
                levels = [
                    split_parts(
                        recipe,
                        level,
                        [p / 2**i for p in detail["pivot"]],
                        {"near": 64, "middle": 16, "far": 4}[family] / 2**i,
                    )[name]
                    for i, level in enumerate(chain)
                ]
                part_recipe = {**recipe, "id": recipe["id"] + "_" + name}
                parts[name] = {
                    "records": write_chain(part_recipe, family, levels, sources),
                    "pivot": detail["pivot"],
                    "gpu_rgba8_bytes_with_chains": sum(a.size for b in levels for a in b.values()),
                }
            families.append(
                {
                    "family": family,
                    "records": records,
                    "pivot": detail["pivot"],
                    "page_size": list(bundle["colour"].shape[:2]),
                    "source_cleanup": detail,
                    "gpu_rgba8_bytes_with_chains": sum(a.size for b in chain for a in b.values()),
                    "preview": preview(recipe, family, bundle, detail["pivot"], chain),
                    "parts": parts,
                    "review": "pending owner runtime approval; engine lighting and normal calibration required",
                }
            )
            if recipe.get("directional_study"):
                families[-1]["directional_study"] = light_study(recipe, family, bundle)
        exports["entries"].append(
            {"id": recipe["id"], "sheet": recipe["sheet"], "status": recipe["status"], "families": families}
        )
        print(f"Prepared {recipe['id']}: {recipe['status']}")
    ground_review([r for r in recipes if r["class"] == "ground"], exports)
    save_json(MANIFEST, exports)


def check_bundle(bundle, allowed_colours, allowed_ids, allow_cut_pixels=False):
    """Checks numbers, not art approval. Returns concrete failures for malformed aligned pages."""
    errors = []
    if len({a.shape for a in bundle.values()}) != 1:
        return ["colour, material and normal dimensions differ"]
    c, m, n = [bundle[k] for k in KINDS]
    alpha = c[:, :, 3]
    if not np.isin(alpha, [0, 255]).all():
        errors.append("solid colour alpha is not binary")
    if not np.array_equal(alpha, m[:, :, 3]) or not np.array_equal(alpha, n[:, :, 3]):
        errors.append("colour, material and normal alpha differ")
    visible = alpha != 0
    colours = {tuple(p) for p in c[visible, :3]}
    if colours - allowed_colours:
        errors.append("colour is outside declared palette")
    if not np.isin(m[visible, 0], list(allowed_ids)).all() or m[:, :, 1:3].any():
        errors.append("material page has an undeclared or blended ID")
    if m[~visible, 0].any():
        errors.append("material ID is present outside colour coverage")
    vec = n[visible, :3].astype(float) / 127.5 - 1
    if len(vec) and not (np.abs(np.linalg.norm(vec, axis=1) - 1) < 0.015).all():
        errors.append("normal is not a unit vector")
    if not allow_cut_pixels and min(alpha.shape) > 1 and isolated(visible).any():
        errors.append("detached single foreground pixels")
    return errors


def check_scale(colour, recipe, ppm):
    """Check the annotated span, excluding transparent storage. Camera truth still needs visual review."""
    if recipe["class"] == "ground":
        span = recipe["ground_span_m"][{64: "near", 16: "middle", 4: "far"}[ppm]]
        return [] if colour.shape[:2] == (round(span * ppm),) * 2 else ["ground metre scale differs"]
    ys, xs = np.nonzero(colour[:, :, 3])
    if not len(xs):
        return ["empty base sprite"]
    measured = (ys.max() - ys.min() + 1) if recipe["measure"] == "vertical" else (xs.max() - xs.min() + 1)
    expected = recipe["metres"] * ppm * (COS37 if recipe["measure"] == "vertical" else 1)
    # Rasterising both edges can add a pixel at each end, especially on small reduced pages.
    errors = [] if abs(measured - expected) <= 2 else [f"annotated span {measured}px differs from {expected:.2f}px"]
    if "crown_width_m" in recipe and abs(xs.max() - xs.min() + 1 - recipe["crown_width_m"] * ppm) > 2:
        errors.append("crown span differs from its declared metres")
    if "height_m" in recipe:
        rise = COS37 * recipe["height_m"] + SIN37 * recipe["anchor_ground_offset_m"][1]
        if abs(ys.max() - ys.min() + 1 - rise * ppm) > 2:
            errors.append("projected top-to-front-contact span differs from its height and ground offset")
    return errors


def check():
    errors = []
    recipes = {r["id"]: r for r in read_json(RECIPE)["fixtures"]}
    exports = read_json(MANIFEST)
    for source in read_json("art/sources/fixtures27/provenance.json"):
        if tiles.sha256(ROOT / source["file"]) != source["sha256"]:
            errors.append(source["file"] + ": original bytes changed")
    pages = 0
    for entry in exports["entries"]:
        r = recipes[entry["id"]]
        colours = {tuple(c) for c in palette(r)}
        ids = {m for _, m in r["palette"]}
        for family in entry["families"]:
            records = {k: tomllib.loads((ROOT / v).read_text()) for k, v in family["records"].items()}
            count = len(records["colour"]["band"])
            if any(len(v["band"]) != count for v in records.values()):
                errors.append(entry["id"] + ": map chains have different lengths")
                continue
            n = records["colour"]["tile_texels"]
            part_records = {
                name: {k: tomllib.loads((ROOT / path).read_text()) for k, path in part["records"].items()}
                for name, part in family.get("parts", {}).items()
            }
            if any(len(record["band"]) != count for part in part_records.values() for record in part.values()):
                errors.append(entry["id"] + ": cutout chains differ from whole chain")
                continue
            if n & (n - 1) or count != n.bit_length():
                errors.append(entry["id"] + ": chain does not halve down to one pixel")
            for i in range(count):
                bundle = {}
                for k, record in records.items():
                    b = record["band"][i]
                    path = ROOT / b["file"]
                    if tiles.sha256(path) != b["sha256"]:
                        errors.append(str(path) + ": digest differs")
                    if b.get("made_from", "") != (record["band"][i - 1]["sha256"] if i else ""):
                        errors.append(str(path) + ": parent digest differs")
                    bundle[k] = np.asarray(Image.open(path).convert("RGBA"))
                if i == 0:
                    ppm = {"near": 64, "middle": 16, "far": 4}[family["family"]]
                    errors += [f"{entry['id']}/{family['family']}: {e}" for e in check_scale(bundle["colour"], r, ppm)]
                    if r["class"] == "ground":
                        rgb = bundle["colour"][:, :, :3]
                        if max(tiles.wrap_ratio(rgb), tiles.wrap_ratio(rgb, across=False)) > 1.2:
                            errors.append(entry["id"] + ": hard ground wrap seam")
                        if tiles.inner_seams(rgb):
                            errors.append(entry["id"] + ": hard row or column in ground")
                if bundle["colour"].shape[:2] != (n >> i, n >> i):
                    errors.append(entry["id"] + ": wrong halving size")
                errors += [
                    f"{entry['id']}/{family['family']}/level {i}: {e}" for e in check_bundle(bundle, colours, ids)
                ]
                if r["class"] == "ground" and (n >> i) > 2 and colour_flecks(bundle["colour"]).any():
                    errors.append(f"{entry['id']}/{family['family']}/level {i}: isolated ground colour flecks")
                pages += 3
                cutouts = []
                for name, part in part_records.items():
                    cutout = {}
                    for k, record in part.items():
                        band = record["band"][i]
                        path = ROOT / band["file"]
                        if tiles.sha256(path) != band["sha256"]:
                            errors.append(str(path) + ": cutout digest differs")
                        if band.get("made_from", "") != (record["band"][i - 1]["sha256"] if i else ""):
                            errors.append(str(path) + ": cutout parent digest differs")
                        cutout[k] = np.asarray(Image.open(path).convert("RGBA"))
                    if cutout["colour"].shape != bundle["colour"].shape:
                        errors.append(name + ": cutout dimensions differ")
                        continue
                    errors += [name + ": " + e for e in check_bundle(cutout, colours, ids, allow_cut_pixels=True)]
                    cutouts.append(cutout)
                    pages += 3
                if cutouts:
                    for kind in KINDS:
                        combined = sum(c[kind].astype(np.uint16) for c in cutouts)
                        if not np.array_equal(combined, bundle[kind]):
                            errors.append(entry["id"] + ": cutouts overlap or fail to reconstruct " + kind)
            pivot = family["pivot"]
            if not all(0 <= p <= n for p in pivot):
                errors.append(entry["id"] + ": root outside page")
            if any(p["pivot"] != pivot for p in family.get("parts", {}).values()):
                errors.append(entry["id"] + ": cutout root differs from whole root")
        if r["class"] != "ground":
            roots = [
                np.array(f["pivot"]) / {"near": 64, "middle": 16, "far": 4}[f["family"]] for f in entry["families"]
            ]
            if not all(np.array_equal(roots[0], other) for other in roots[1:]):
                errors.append(entry["id"] + ": family pivots drift in metres")
    for problem in errors:
        print("FAIL " + problem)
    print(f"Fixture checks: {pages} aligned pages; {len(errors)} failures. Art and engine approval remain pending.")
    return int(bool(errors))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("build", "check", "design"))
    parser.add_argument("--only", nargs="+", help="rebuild only these fixture IDs; keep other existing exports")
    args = parser.parse_args()
    if args.action == "build":
        build(args.only)
    if args.action == "design":
        build_designs()
        return 0
    return check()


if __name__ == "__main__":
    sys.exit(main())
