"""PRE-42 PRE-43 PRE-46: derive look-only sprite catalogue records from the art export contract.

Source drawings, normal basis and material IDs remain untouched. Worker preparation converts
source channels for the renderer. The derived TOML joins the ordinary catalogue and its hash.
"""

import hashlib
import json
import math
import re
import tomllib
from pathlib import Path

from PIL import Image

DENSITIES = {"near": 64, "middle": 16, "far": 4}
SOURCE_BASIS = "world east, south, up; signed unit XYZ encoded as round((n+1)*127.5)"
CHANNELS = {"colour", "normal", "material"}


def within(root, relative):
    path = Path(relative)
    if path.is_absolute() or ".." in path.parts or not path.parts or "\\" in relative:
        raise ValueError(f"invalid sprite path: {relative}")
    resolved = (root / path).resolve()
    if not resolved.is_relative_to(root.resolve()):
        raise ValueError(f"sprite path leaves source root: {relative}")
    return resolved


def name(value):
    if not isinstance(value, str) or not re.fullmatch(r"[a-z0-9_-]+", value):
        raise ValueError(f"invalid sprite name: {value}")
    return value.replace("-", "_")


def record_chain(root, relative, size, density):
    if not relative.startswith("art/textures/") or not relative.endswith("/record.toml"):
        raise ValueError(f"invalid texture record path: {relative}")
    record = tomllib.loads(within(root, relative).read_text())
    if record.get("tile_texels") != size or record.get("texels_a_metre") != density:
        raise ValueError(f"sprite page size or density differs from texture: {relative}")
    bands = sorted(record.get("band", []), key=lambda band: band["level"])
    sizes = [max(1, size >> level) for level in range(size.bit_length())]
    if len(bands) != len(sizes) or [b["level"] for b in bands] != list(range(len(sizes))):
        raise ValueError(f"incomplete texture chain: {relative}")
    alphas = []
    for band, width in zip(bands, sizes, strict=True):
        path = within(root, band["file"])
        if hashlib.sha256(path.read_bytes()).hexdigest() != band["sha256"]:
            raise ValueError(f"texture hash mismatch: {band['file']}")
        with Image.open(path) as image:
            if image.size != (width, width):
                raise ValueError(f"texture chain page size mismatch: {band['file']}")
            alphas.append(image.convert("RGBA").getchannel("A").tobytes())
    return "art:" + relative[len("art/textures/") : -len("/record.toml")], alphas


def fixed(value):
    if not isinstance(value, (int, float)) or isinstance(value, bool) or not math.isfinite(value):
        raise ValueError("sprite pivot must be finite")
    scaled = value * 256
    if scaled != round(scaled) or abs(scaled) >= 2**63:
        raise ValueError("sprite pivot cannot be represented in fixed source pixels")
    return round(scaled)


def emit(record, cell):
    lines = [f"{key} = {json.dumps(value, ensure_ascii=False)}" for key, value in record.items()]
    lines += ["", "[[cells]]"]
    lines += [f"{key} = {value}" for key, value in cell.items()]
    return "\n".join(lines) + "\n"


def derive(root, manifest=None, namespace="fixtures27"):
    """Return path -> TOML text, without writing source art or derived output."""
    root = Path(root)
    if manifest is None:
        made = {}
        for group in ("fixtures27", "ground29"):
            path = root / f"art/sources/{group}/exports.json"
            if path.is_file():
                made.update(derive(root, json.loads(path.read_text()), group))
        return made
    if namespace not in ("fixtures27", "ground29"):
        raise ValueError("unsupported sprite namespace")
    if manifest.get("normal_basis") != SOURCE_BASIS:
        raise ValueError("unsupported source normal basis")
    if manifest.get("material_encoding") != "R exact ID, G=B=0, A colour coverage":
        raise ValueError("unsupported source material encoding")
    made = {}
    for entry in manifest["entries"]:
        asset = name(entry["id"])
        for family in entry["families"]:
            family_name = family["family"]
            if family_name not in DENSITIES:
                raise ValueError("unsupported authored family")
            size = family["page_size"]
            if (
                len(size) != 2
                or any(type(v) is not int for v in size)
                or size[0] != size[1]
                or size[0] < 1
                or size[0] > 4096
                or size[0] & (size[0] - 1)
            ):
                raise ValueError("sprite page must be a bounded square power of two")
            for part, bundle in [("whole", family), *family.get("parts", {}).items()]:
                part = name(part)
                records = bundle["records"]
                if set(records) != CHANNELS:
                    raise ValueError("sprite bundle needs all three aligned channels")
                references = {}
                coverage = None
                for channel in sorted(CHANNELS):
                    ref, alphas = record_chain(root, records[channel], size[0], DENSITIES[family_name])
                    if coverage is not None and coverage != alphas:
                        raise ValueError("sprite channels have different alpha coverage")
                    references[channel] = ref
                    coverage = alphas
                pivot = bundle["pivot"]
                if len(pivot) != 2:
                    raise ValueError("sprite pivot needs two coordinates")
                px, py = (fixed(v) for v in pivot)
                season = next((s for s in ("summer", "winter") if asset.endswith("_" + s)), "all")
                record = {
                    "about": "T2.9a.2: derived fixture family; runtime approval remains separate",
                    "asset": asset,
                    "action": "static",
                    "season": season,
                    "part": part,
                    "family": family_name,
                    "density": DENSITIES[family_name],
                    "page_width": size[0],
                    "page_height": size[1],
                    "frames": 1,
                    "facings": 1,
                    **references,
                    "normal_basis": "world-east-south-up",
                    "material_map": "fixture27-v1",
                    "sheet": entry["sheet"] if "sheet" in entry else "catalogue:" + name(entry["catalogue"]),
                    "approved": manifest["approval"],
                }
                cell = {
                    "frame": 0,
                    "facing": 0,
                    "x": 0,
                    "y": 0,
                    "width": size[0],
                    "height": size[1],
                    "pivot_x_256": px,
                    "pivot_y_256": py,
                    "canvas_width": size[0],
                    "canvas_height": size[1],
                    "trim_x": 0,
                    "trim_y": 0,
                    "gutter": 0,
                }
                path = f"art/sprites/{namespace}/{asset}/{family_name}/{part}/record.toml"
                if path in made:
                    raise ValueError(f"duplicate sprite family: {path}")
                made[path] = emit(record, cell)
    return made
