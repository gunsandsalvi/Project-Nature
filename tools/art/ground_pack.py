"""Build signed ground runtime candidates from retained drawings (PRE-20, PRE-22, PRE-46).

No colour art is drawn here. The independent critic and owner gates remain separate.
"""

from pathlib import Path
from collections import deque
import json
import hashlib
import argparse
import tomllib
import numpy as np
from PIL import Image

import fixtures
import tiles
import textures
import ingest

REPO = Path(textures.ROOT)
ROOT = REPO / "art/sources/ground29"
STAGE = REPO
coupled = tiles

PALETTES = {
    "bare_earth": ["#B68A58", "#C69B67", "#D2AD79", "#A7794C", "#91633E", "#755438"],
    "bank_gravel": ["#766B59", "#968773", "#B2A18A", "#CBB99B", "#DCCCAC", "#878B82"],
    "river_bed": ["#706956", "#8C826C", "#A1977D", "#B4A68A", "#C5B698", "#848B88"],
}
ALGAE = ["#677347", "#7B8754", "#525F3C"]


def pack_definitions():
    """Optional explicit state contract; historical ground29 remains byte-stable."""
    path = ROOT / "pack-contract.json"
    if path.exists():
        return json.loads(path.read_text())["states"]
    result = []
    for key in ["bare_earth", "bank_gravel", "river_bed", "river_bed_algae", "river_bed_silted", "river_bed_exposed"]:
        base = "river_bed" if key.startswith("river_bed") else key
        palette = [[chip, mid] for chip in PALETTES[base] for mid in ([2] if base == "bare_earth" else [2, 6])]
        if key.endswith("_algae"):
            palette += [[chip, 3] for chip in ALGAE]
        request = {
            "river_bed_algae": "river-algae-r1.txt",
            "river_bed_silted": "river-silted-r2.txt",
            "river_bed_exposed": "river-exposed-r1.txt",
        }.get(key)
        result.append(
            dict(
                id=key,
                catalogue=base,
                state="usual" if key == base else key[len(base) + 1 :],
                palette=palette,
                request=request,
                auxiliary=[],
            )
        )
    return result


def binary_components(mask):
    """Measure drawn coverage regions in native cells before accepting a quilt."""
    seen = np.zeros(mask.shape, bool)
    sizes = []
    height, width = mask.shape
    for y, x in zip(*np.nonzero(mask), strict=True):
        if seen[y, x]:
            continue
        queue = deque([(y, x)])
        seen[y, x] = True
        ys, xs = [], []
        while queue:
            a, b = queue.popleft()
            ys.append(a)
            xs.append(b)
            for da, db in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                c, d = a + da, b + db
                if 0 <= c < height and 0 <= d < width and mask[c, d] and not seen[c, d]:
                    seen[c, d] = True
                    queue.append((c, d))
        sizes.append(max(max(ys) - min(ys) + 1, max(xs) - min(xs) + 1))
    return sizes


def trace_regions(colour, palette, piece, family):
    """Trace bounded authored cobble interiors, then their dark drawn rims.

    This is a review candidate. Broad sand regions remain earth; tiny rounded
    regions retain their complete tan highlights. Buried silt never implies stone.
    """
    chips = np.asarray([list(bytes.fromhex(c[1:])) for c in palette])
    indices = ((colour[..., None, :].astype(int) - chips) ** 2).sum(3).argmin(2)
    ids = np.full(indices.shape, 2, np.uint8)
    if piece == "bare_earth":
        return ids
    green = indices >= 6
    if piece == "bank_gravel" and family != "near":
        # At these independently drawn scales the pale chip is shared by
        # tiny gravel and continuous sand. Annotate the visible broad drifts
        # by their local coverage, rather than inventing individual cobbles.
        pale = indices >= 2
        radius = 4 if family == "middle" else 6
        coverage = (
            sum(
                np.roll(pale, (dy, dx), (0, 1)).astype(float)
                for dy in range(-radius, radius + 1)
                for dx in range(-radius, radius + 1)
            )
            / (2 * radius + 1) ** 2
        )
        return np.where(coverage >= 0.45, 2, 6).astype(np.uint8)
    if piece.endswith("_silted"):
        # Only actual exposed blue-grey stone islands interrupt the sediment.
        ids[indices == 5] = 6
        return ids
    visible = (indices != 0) & ~green
    seen = np.zeros(indices.shape, bool)
    height, width = indices.shape
    for y, x in zip(*np.nonzero(visible), strict=True):
        if seen[y, x]:
            continue
        cells = []
        queue = deque([(y, x)])
        seen[y, x] = True
        while queue:
            a, b = queue.popleft()
            cells.append((a, b))
            for da, db in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                c, d = a + da, b + db
                if 0 <= c < height and 0 <= d < width and visible[c, d] and not seen[c, d]:
                    seen[c, d] = True
                    queue.append((c, d))
        ys, xs = np.asarray(cells).T
        # Large pale regions are sand drifts. Closed small forms or mixed
        # body/highlight regions are complete visible cobbles.
        pale_share = float(np.isin(indices[ys, xs], [3, 4]).mean())
        if len(cells) <= 1200 and (
            piece.startswith("river_bed") and family == "near" or len(cells) <= 100 or pale_share < 0.8
        ):
            ids[ys, xs] = 6
    # The shadow/body ramp and grey aggregates remain stone even inside a
    # broad authored sand field; pale highlights follow their bounded region.
    ids[np.isin(indices, [1, 2, 5])] = 6
    # Trace broad pale sand independently of neighbouring darker cobbles;
    # an incidental pixel bridge must not turn an entire sand drift into stone.
    pale = np.isin(indices, [3, 4])
    seen = np.zeros(indices.shape, bool)
    for y, x in zip(*np.nonzero(pale), strict=True):
        if seen[y, x]:
            continue
        cells = []
        queue = deque([(y, x)])
        seen[y, x] = True
        while queue:
            a, b = queue.popleft()
            cells.append((a, b))
            for da, db in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
                c, d = a + da, b + db
                if 0 <= c < height and 0 <= d < width and pale[c, d] and not seen[c, d]:
                    seen[c, d] = True
                    queue.append((c, d))
        if len(cells) > 240 and not (piece.startswith("river_bed") and family == "near"):
            ys, xs = np.asarray(cells).T
            ids[ys, xs] = 2
    stone = ids == 6
    neighbours = sum(np.roll(stone, (a, b), (0, 1)).astype(np.uint8) for a in [-1, 0, 1] for b in [-1, 0, 1] if a or b)
    ids[(indices == 0) & (neighbours >= 3)] = 6
    ids[green] = 3
    return ids


def quiet_with_labels(rgb, labels, protected=0):
    """Copy a chosen adjacent colour's material label during pinprick cleanup."""
    rgba = np.dstack([rgb, np.full(rgb.shape[:2], 255, np.uint8)])
    labels = labels.copy()
    height, width = labels.shape
    for _ in range(8):
        bad = fixtures.colour_flecks(rgba)
        if protected:
            bad[:protected] = bad[-protected:] = False
            bad[:, :protected] = bad[:, -protected:] = False
        if not bad.any():
            break
        for y, x in zip(*np.nonzero(bad), strict=True):
            positions = [((y + a) % height, (x + b) % width) for a in [-1, 0, 1] for b in [-1, 0, 1] if a or b]
            adjacent = np.asarray([rgba[a, b] for a, b in positions])
            values, counts = np.unique(adjacent, axis=0, return_counts=True)
            best = values[counts == counts.max()]
            differences = ((best[:, :3].astype(int) - rgba[y, x, :3].astype(int)) ** 2).sum(1)
            chosen = best[differences.argmin()]
            i = int(np.nonzero((adjacent == chosen).all(1))[0][0])
            rgba[y, x] = chosen
            labels[y, x] = labels[positions[i]]
    return rgba[:, :, :3], labels


def prepare(only=None):
    rows = json.loads((ROOT / "source-index.json").read_text())
    definitions = {row["id"]: row for row in pack_definitions()}
    for row in rows:
        key = row["id"]
        family = row["family"]
        if only and (key, family) != tuple(only):
            continue
        definition = definitions[key]
        palette = list(dict.fromkeys(chip for chip, _ in definition["palette"]))
        if tiles.sha256(ROOT / row["original"]) != row["original_sha256"]:
            raise ValueError("original source hash changed: " + row["original"])
        raw = np.asarray(Image.open(ROOT / row.get("crop", row["original"])).convert("RGB"))
        cells = row["source_cells"]
        recovered, loss = tiles.snap(raw, raw.shape[0] / cells, cells=cells)
        rgba = fixtures.quantise(
            np.dstack([recovered, np.full(recovered.shape[:2], 255, np.uint8)]), {"palette": [[c, 2] for c in palette]}
        )[0]
        if definition.get("annotation") in ["charcoal", "puddles"]:
            chips = np.asarray([list(bytes.fromhex(c[1:])) for c in palette])
            indices = ((rgba[..., None, :3].astype(int) - chips) ** 2).sum(3).argmin(2)
            labels = np.full(indices.shape, 2, np.uint8)
            if definition["annotation"] == "charcoal":
                # The distinct charcoal chip traces actual grey-black fragments;
                # both dark brown scorched-soil chips remain earth2.
                labels[indices == definition["annotation_chip"]] = 5
            else:
                # Private packed transport only; decoded before material pages.
                labels[indices == definition["annotation_chip"]] |= 128
        else:
            labels = trace_regions(rgba[..., :3], palette, key, family)
        rgb, labels = quiet_with_labels(rgba[..., :3], labels)
        folder = ROOT / "recovered" / key / family
        folder.mkdir(parents=True, exist_ok=True)
        Image.fromarray(rgb).save(folder / "colour.png")
        Image.fromarray(labels & 127).save(folder / "material-ids.png")
        if definition.get("auxiliary"):
            Image.fromarray((labels >= 128).astype(np.uint8) * 255).save(folder / "water-coverage.png")
        tags = np.dstack([rgb, labels])
        if min(tags.shape[:2]) < 256:
            # Quilt larger runtime tiles from repeated existing source samples;
            # full colour/boundary/coverage physical calibration stays aligned.
            tags = np.tile(tags, (int(np.ceil(256 / tags.shape[0])), int(np.ceil(256 / tags.shape[1])), 1))
        attempted = row.get("quilt_attempts", 3)
        versions = coupled.make_versions_open(
            [tags], attempted, 4, 16, row.get("quilt_patch", 112), row.get("quilt_seed", 27032), 256
        )
        if "component_limit_cells" in row:
            accepted = [
                i
                for i, version in enumerate(versions)
                if max(binary_components(version[..., 3] >= 128), default=0) <= row["component_limit_cells"]
            ]
            if len(accepted) < 3:
                raise ValueError("too few physically bounded quilts: " + key + " " + family)
            versions = [versions[i] for i in accepted[:3]]
        else:
            accepted = list(range(3))
        ring = tiles.ring_mask(256, 4)
        shared = versions[0].copy()
        for _ in range(8):
            rgb = shared[..., :3]
            equal = np.zeros(ring.shape, bool)
            for dy in [-1, 0, 1]:
                for dx in [-1, 0, 1]:
                    if dy or dx:
                        equal |= (rgb == np.roll(rgb, (dy, dx), (0, 1))).all(2) & np.roll(ring, (dy, dx), (0, 1))
            bad = ring & ~equal
            if not bad.any():
                break
            for y, x in zip(*np.nonzero(bad), strict=True):
                adjacent = np.asarray(
                    [
                        shared[(y + dy) % 256, (x + dx) % 256]
                        for dy in [-1, 0, 1]
                        for dx in [-1, 0, 1]
                        if (dy or dx) and ring[(y + dy) % 256, (x + dx) % 256]
                    ]
                )
                values, counts = np.unique(adjacent[:, :3], axis=0, return_counts=True)
                chosen = values[counts.argmax()]
                shared[y, x] = adjacent[np.nonzero((adjacent[:, :3] == chosen).all(1))[0][0]]
        for variant, version in enumerate(versions):
            version[ring] = shared[ring]
            rgb, labels = quiet_with_labels(version[..., :3], version[..., 3], protected=4)
            out = ROOT / "pack" / key / family / f"v{variant + 1}"
            out.mkdir(parents=True, exist_ok=True)
            bundle = {
                "colour": np.dstack([rgb, np.full(labels.shape, 255, np.uint8)]),
                "material": fixtures.material_page(labels & 127, np.full(labels.shape, 255, np.uint8)),
                "normal": fixtures.normals(np.full(labels.shape, 255, np.uint8), ground=True),
            }
            if definition.get("auxiliary"):
                coverage = np.zeros_like(bundle["material"])
                coverage[..., 0] = (labels >= 128).astype(np.uint8) * 255
                coverage[..., 3] = 255
                bundle["water_coverage"] = coverage
            for kind, pixels in bundle.items():
                Image.fromarray(pixels).save(out / f"{kind}.png")
        (folder / "annotation.json").write_text(
            json.dumps(
                {
                    "source": row,
                    "grid_loss": loss,
                    "method": (
                        "bounded visible stone regions with traced dark rim; "
                        "broad sediment and sand earth2; visible algae3"
                    ),
                    "review": "candidate trace, pending independent review",
                    **(
                        {
                            "quilt_attempts": attempted,
                            "quilt_selected": accepted[:3],
                            "annotation": definition["annotation"],
                        }
                        if definition.get("annotation")
                        else {}
                    ),
                },
                indent=2,
            )
            + "\n"
        )
        print(key, family, "prepared", flush=True)


def cleanup_with_coverage(bundle, protected=0):
    """Colour cleanup transports exact materials and separate binary coverage."""
    packed = bundle["material"][..., 0].astype(np.uint16)
    packed |= (bundle["water_coverage"][..., 0] != 0).astype(np.uint16) << 8
    rgb, packed = quiet_with_labels(bundle["colour"][..., :3], packed, protected)
    bundle["colour"][..., :3] = rgb
    bundle["material"] = fixtures.material_page((packed & 255).astype(np.uint8), bundle["colour"][..., 3])
    bundle["water_coverage"][..., 0] = ((packed >> 8) != 0).astype(np.uint8) * 255


def halve_with_coverage(bundle, recipe):
    """Choose coverage from an actual source sample matching reduced material."""
    smaller = fixtures.halve_bundle(bundle)
    colour, ids = fixtures.quantise(smaller["colour"], recipe, smaller["material"][..., 0])
    smaller["colour"] = colour
    smaller["material"] = fixtures.material_page(ids, colour[..., 3])
    n = len(colour)

    def blocks(pixels):
        return pixels.reshape(n, 2, n, 2, 4).transpose(0, 2, 1, 3, 4).reshape(n, n, 4, 4)

    source_colour = blocks(bundle["colour"])
    source_ids = blocks(bundle["material"])[..., 0]
    source_water = blocks(bundle["water_coverage"])
    distance = ((source_colour[..., :3].astype(np.int32) - colour[:, :, None, :3]) ** 2).sum(3)
    eligible = (source_ids == ids[..., None]) & (source_colour[..., 3] != 0)
    distance[~eligible] = np.iinfo(np.int32).max
    chosen = distance.argmin(2)
    smaller["water_coverage"] = np.take_along_axis(source_water, chosen[..., None, None], axis=2)[:, :, 0].copy()
    smaller["water_coverage"][..., 3] = colour[..., 3]
    cleanup_with_coverage(smaller)
    return smaller


def ground_chains(recipe, bundles):
    """Reduce variants together, carrying labels through cleanup and border repair.

    Base drawings are untouched. Reduced borders copy one existing aligned
    colour/material/normal sample; no blended categorical value is introduced.
    """
    chains = [[bundle] for bundle in bundles]
    while len(chains[0][-1]["colour"]) > 1:
        reduced = []
        for chain in chains:
            if "water_coverage" in chain[-1]:
                smaller = halve_with_coverage(chain[-1], recipe)
            else:
                smaller = fixtures.halve_bundle(chain[-1])
                colour, ids = fixtures.quantise(smaller["colour"], recipe, smaller["material"][..., 0])
                rgb, ids = quiet_with_labels(colour[..., :3], ids)
                smaller["colour"] = np.dstack([rgb, colour[..., 3]])
                smaller["material"] = fixtures.material_page(ids, colour[..., 3])
            reduced.append(smaller)
        size = len(reduced[0]["colour"])
        common = {kind: pixels.copy() for kind, pixels in reduced[0].items()}
        for pixels in common.values():
            if size <= 2:
                pixels[:] = pixels[0, 0]
            else:
                pixels[-1, :] = pixels[0, :]
                pixels[:, -1] = pixels[:, 0]
        for chain, smaller in zip(chains, reduced, strict=True):
            for kind, pixels in smaller.items():
                if size <= 2:
                    pixels[:] = common[kind]
                else:
                    pixels[0, :] = common[kind][0, :]
                    pixels[-1, :] = common[kind][-1, :]
                    pixels[:, 0] = common[kind][:, 0]
                    pixels[:, -1] = common[kind][:, -1]
            if size > 2:
                if "water_coverage" in smaller:
                    cleanup_with_coverage(smaller, protected=1)
                else:
                    rgb, ids = quiet_with_labels(smaller["colour"][..., :3], smaller["material"][..., 0], protected=1)
                    smaller["colour"][..., :3] = rgb
                    smaller["material"] = fixtures.material_page(ids, smaller["colour"][..., 3])
            chain.append(smaller)
    return chains


def export():
    entries = []
    source_index = json.loads((ROOT / "source-index.json").read_text())
    source_prefix = str(ROOT.relative_to(REPO))
    texture_prefix = "art/textures/" + ROOT.name
    request_prefix = "art/requests/" + ROOT.name
    prepared_chains = {}
    for definition in pack_definitions():
        piece = definition["id"]
        base = definition["catalogue"]
        palette = definition["palette"]
        chips = list(dict.fromkeys(chip for chip, _ in palette))
        kinds = [*fixtures.KINDS, *definition.get("auxiliary", [])]
        recipe = {"id": piece, "class": "ground", "palette": palette, "ground_span_m": dict(near=4, middle=16, far=64)}
        for variant in [1, 2, 3]:
            families = []
            name = piece if variant == 1 else piece + f"_v{variant}"
            inputs = [row for row in source_index if row["id"] == piece]
            sources = list(dict.fromkeys(source_prefix + "/" + row["original"] for row in inputs))
            sources += list(dict.fromkeys(source_prefix + "/" + row["crop"] for row in inputs if row.get("crop")))
            requests = [request_prefix + "/production.txt"]
            state_request = definition.get("request")
            if state_request:
                requests.append(request_prefix + "/" + state_request)
            requests += [
                request_prefix + "/" + name
                for name in dict.fromkeys(row["request"] for row in inputs if row.get("request"))
                if request_prefix + "/" + name not in requests
            ]
            for family, ppm, start in [("near", 64, 0), ("middle", 16, 2), ("far", 4, 4)]:
                source = ROOT / "pack" / piece / family / f"v{variant}"
                if not source.exists():
                    raise ValueError("unfinished family " + str(source))
                bundle = {kind: np.asarray(Image.open(source / f"{kind}.png").convert("RGBA")) for kind in kinds}
                cache_key = (piece, family)
                if cache_key not in prepared_chains:
                    base_bundles = [
                        {
                            kind: np.asarray(
                                Image.open(ROOT / "pack" / piece / family / f"v{v}" / f"{kind}.png").convert("RGBA")
                            )
                            for kind in kinds
                        }
                        for v in [1, 2, 3]
                    ]
                    prepared_chains[cache_key] = ground_chains(recipe, base_bundles)
                chain = prepared_chains[cache_key][variant - 1]
                records = {}
                for kind in kinds:
                    folder = f"{texture_prefix}/{name}/{family}/{kind}"
                    (STAGE / folder).mkdir(parents=True, exist_ok=True)
                    levels = []
                    prior = ""
                    for i, level in enumerate(chain):
                        path = f"{folder}/b{start + i}.png"
                        Image.fromarray(level[kind]).save(STAGE / path)
                        sha = hashlib.sha256((STAGE / path).read_bytes()).hexdigest()
                        levels.append(
                            dict(
                                level=i,
                                file=path,
                                sha256=sha,
                                made_from=prior,
                                way="coupled source-pixel and material-region quilt"
                                if i == 0
                                else "aligned categorical reduction; coupled cleanup and shared periodic border",
                            )
                        )
                        prior = sha
                        errors = fixtures.check_bundle(
                            level,
                            {tuple(bytes.fromhex(c[1:])) for c in chips},
                            {mid for _, mid in palette},
                        )
                        if errors:
                            raise ValueError(f"{name} {family} {i}: {errors}")
                    fields = dict(
                        about=f"{name} {family} {kind}: signed design runtime candidate",
                        route="picture" if kind == "colour" else "code",
                        tile_texels=256,
                        texels_a_metre=ppm,
                        first_band=start,
                        sources=sources,
                        original_sha256=[tiles.sha256(REPO / p) for p in sources],
                        c2pa=["present" if ingest.has_c2pa(REPO / p) else "absent" for p in sources],
                        requests=requests,
                        made="retained image drawings, coupled source-pixel quilt and numeric traced materials",
                        approved="pending independent exported-art, engine and owner runtime review",
                        truth="signed design source; world overlays and water separate",
                        regrid_loss="raw-to-grid measurements and region annotations kept beside recovered sources",
                    )
                    path = f"{folder}/record.toml"
                    (STAGE / path).write_text(textures.record_text(fields, levels))
                    records[kind] = path
                families.append(
                    dict(
                        family=family,
                        records={kind: records[kind] for kind in fixtures.KINDS},
                        pivot=[0, 0],
                        page_size=[256, 256],
                        parts={},
                        ground_span_m=recipe["ground_span_m"][family],
                        gpu_rgba8_bytes_with_chains=sum(a.size for level in chain for a in level.values()),
                        review="candidate; engine terrain normal binding required",
                    )
                )
                if "water_coverage" in records:
                    families[-1]["water_coverage"] = records["water_coverage"]
                if variant == 1:
                    colour = Image.fromarray(fixtures.project_ground(bundle["colour"]))
                    shown = colour.resize((colour.width * 2, colour.height * 2), Image.Resampling.NEAREST)
                    shown.save(ROOT / f"{piece}-{family}-actual-2x.png")
            entries.append(
                dict(
                    id=name,
                    class_="ground",
                    families=families,
                    normal_binding="actual terrain normal field",
                    state=definition["state"],
                    catalogue=base,
                    runtime_approved=False,
                )
            )
            entries[-1]["class"] = entries[-1].pop("class_")
    manifest = dict(
        normal_basis=fixtures.NORMAL_BASIS,
        material_encoding="R exact ID, G=B=0, A colour coverage",
        approval="candidate; no owner/runtime pass",
        entries=entries,
    )
    (STAGE / source_prefix).mkdir(parents=True, exist_ok=True)
    (STAGE / source_prefix / "exports.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(len(entries), "state/variant entries with3families and complete chains prepared")


def check():
    """Read back candidate files, hashes, coverage and exact aligned reductions."""
    manifest = json.loads((ROOT / "exports.json").read_text())
    rows = json.loads((ROOT / "source-index.json").read_text())
    for row in rows:
        if tiles.sha256(ROOT / row["original"]) != row["original_sha256"]:
            raise ValueError("original source hash changed: " + row["original"])
    pages = 0
    variants = {}
    definitions = {row["id"]: row for row in pack_definitions()}
    if len(manifest["entries"]) != len(definitions) * 3:
        raise ValueError("expected three variants per declared state pack")
    for entry in manifest["entries"]:
        key = entry["id"].removesuffix("_v2").removesuffix("_v3")
        definition = definitions[key]
        if entry["catalogue"] != definition["catalogue"] or entry["state"] != definition["state"]:
            raise ValueError("incorrect catalogue/state mapping " + entry["id"])
        palette = definition["palette"]
        chips = list(dict.fromkeys(chip for chip, _ in palette))
        ids = {mid for _, mid in palette}
        kinds = [*fixtures.KINDS, *definition.get("auxiliary", [])]
        recipe = {"id": key, "class": "ground", "palette": palette}
        if [f["family"] for f in entry["families"]] != ["near", "middle", "far"]:
            raise ValueError("incomplete family set " + entry["id"])
        for family in entry["families"]:
            name = family["family"]
            ppm, start, span = {"near": (64, 0, 4), "middle": (16, 2, 16), "far": (4, 4, 64)}[name]
            if family["ground_span_m"] != span or family["page_size"] != [256, 256] or family["pivot"] != [0, 0]:
                raise ValueError("incorrect physical family contract " + entry["id"] + " " + name)
            records = {}
            paths = dict(family["records"])
            if "water_coverage" in kinds:
                paths["water_coverage"] = family["water_coverage"]
            if set(paths) != set(kinds):
                raise ValueError("incomplete aligned field set " + entry["id"] + " " + name)
            for kind, path in paths.items():
                record = tomllib.loads((REPO / path).read_text())
                if record["texels_a_metre"] != ppm or record["first_band"] != start:
                    raise ValueError("incorrect density or first band " + path)
                if len(record["band"]) != 9:
                    raise ValueError("incomplete chain " + path)
                for source, digest in zip(record["sources"], record["original_sha256"], strict=True):
                    if tiles.sha256(REPO / source) != digest:
                        raise ValueError("changed provenance source " + source)
                for request in record["requests"]:
                    if not (REPO / request).is_file():
                        raise ValueError("missing request " + request)
                records[kind] = record
            bundles = []
            for level in range(9):
                bundle = {}
                for kind, record in records.items():
                    band = record["band"][level]
                    if band["level"] != level or tiles.sha256(REPO / band["file"]) != band["sha256"]:
                        raise ValueError("invalid page hash or level " + band["file"])
                    prior = "" if level == 0 else record["band"][level - 1]["sha256"]
                    if band.get("made_from", "") != prior:
                        raise ValueError("invalid parent hash " + band["file"])
                    pixels = np.asarray(Image.open(REPO / band["file"]).convert("RGBA"))
                    if pixels.shape != (256 >> level, 256 >> level, 4):
                        raise ValueError("invalid dimensions " + band["file"])
                    bundle[kind] = pixels
                    pages += 1
                errors = fixtures.check_bundle(bundle, {tuple(bytes.fromhex(c[1:])) for c in chips}, ids)
                if errors:
                    raise ValueError(f"{entry['id']} {family['family']} {level}: {errors}")
                if "water_coverage" in bundle:
                    coverage = bundle["water_coverage"]
                    if (
                        not np.isin(coverage[..., 0], [0, 255]).all()
                        or coverage[..., 1:3].any()
                        or not np.array_equal(coverage[..., 3], bundle["colour"][..., 3])
                    ):
                        raise ValueError("invalid binary coverage " + entry["id"] + " " + name)
                    if level == 0:
                        source_row = next(row for row in rows if row["id"] == key and row["family"] == name)
                        limit = source_row.get("component_limit_m", 2) * ppm
                        if max(binary_components(coverage[..., 0] != 0), default=0) > limit:
                            raise ValueError("drawn coverage exceeds physical contract " + entry["id"] + " " + name)
                bundles.append(bundle)
            variants.setdefault((key, name), []).append((recipe, bundles))
    for (key, family), rows in variants.items():
        if len(rows) != 3:
            raise ValueError("incomplete variants " + key + " " + family)
        actual_chains = [row[1] for row in rows]
        expected = ground_chains(rows[0][0], [chain[0] for chain in actual_chains])
        for actual, wanted in zip(actual_chains, expected, strict=True):
            if any(
                not np.array_equal(a[kind], b[kind]) for a, b in zip(actual, wanted, strict=True) for kind in actual[0]
            ):
                raise ValueError("unaligned coupled reduction " + key + " " + family)
        for level in range(9):
            bundles = [chain[level] for chain in actual_chains]
            for bundle in bundles:
                if len(bundle["colour"]) > 2 and fixtures.colour_flecks(bundle["colour"]).any():
                    raise ValueError(f"isolated colour fleck {key} {family} level{level}")
            for kind in bundles[0]:
                if level == 0:
                    if not tiles.shares_ring([b[kind] for b in bundles], 4):
                        raise ValueError("incompatible base borders " + key + " " + family + " " + kind)
                else:
                    for a in bundles:
                        for b in bundles:
                            if not np.array_equal(a[kind][:, 0], b[kind][:, -1]) or not np.array_equal(
                                a[kind][0], b[kind][-1]
                            ):
                                raise ValueError(f"incompatible reduction borders {key} {family} {kind} level{level}")
            if level == 0:
                for a in bundles:
                    for b in bundles:
                        for across in [False, True]:
                            if tiles.join_ratio(a["colour"][..., :3], b["colour"][..., :3], across) > 1.2:
                                raise ValueError("visible numeric join " + key + " " + family)
    return {"entries": len(manifest["entries"]), "pages": pages, "failures": 0}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", nargs="?", choices=["build", "check"], default="build")
    parser.add_argument("--batch", default="ground29", help="directory name under art/sources, textures and requests")
    args = parser.parse_args()
    if Path(args.batch).name != args.batch or args.batch in ["", ".", ".."]:
        parser.error("batch must be one directory name")
    ROOT = REPO / "art/sources" / args.batch
    if args.command == "check":
        print(json.dumps(check(), sort_keys=True))
    else:
        prepare()
        export()
