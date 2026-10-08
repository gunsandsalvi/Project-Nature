"""Traced numeric tent annotations and shape fields; never draws colour art (PRE-20 PRE-46)."""

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

# One native near-page transform owns both drawn-pixel cleanup and all traced regions.
DOOR_RECT = (230, 410, 288, 493)
DOOR_SOURCE_TOP = 424
DOOR_TARGET_TOP = 432


def door_cleanup_geometry(pivot):
    x0, y0, x1, floor = DOOR_RECT
    box = tuple(round(v + pivot[i % 2] - (256 if i % 2 == 0 else 508)) for i, v in enumerate(DOOR_RECT))
    return box, [[(DOOR_SOURCE_TOP - y0) / (floor - y0), (DOOR_TARGET_TOP - y0) / (floor - y0)]]


def fitted_door_point(p):
    x, y = p
    x0, y0, x1, floor = DOOR_RECT
    if x0 <= x < x1 and y0 <= y <= floor:
        y = round(np.interp(y, [y0, DOOR_SOURCE_TOP, floor], [y0, DOOR_TARGET_TOP, floor]))
    return x, y


# Coordinates refer to the inspected near page, scaled with the registered family pivot.
SEAMS = {
    64: [
        [
            (254, 310),
            (250, 315),
            (244, 330),
            (236, 344),
            (234, 350),
            (230, 361),
            (226, 369),
            (224, 375),
            (228, 386),
            (232, 395),
            (239, 401),
        ],
        [(266, 315), (268, 330), (270, 344), (272, 350), (274, 361), (276, 369), (278, 375), (280, 386), (281, 395)],
        [(239, 399), (251, 400), (267, 400), (282, 402), (296, 404)],
        [(230, 414), (226, 431), (222, 449), (216, 467), (213, 478)],
        [(297, 414), (308, 431), (315, 449), (325, 467), (330, 478)],
    ],
    16: [
        [(63, 79), (61, 85), (59, 91), (56, 96), (57, 98)],
        [(67, 80), (69, 85), (70, 89), (71, 93), (72, 100)],
        [(60, 101), (64, 101), (68, 101), (72, 101)],
    ],
}
STONE_CENTRES = [
    (153, 413),
    (143, 426),
    (135, 442),
    (137, 458),
    (149, 472),
    (171, 480),
    (194, 487),
    (218, 493),
    (240, 498),
    (264, 498),
    (286, 497),
    (307, 492),
    (330, 487),
    (351, 478),
    (373, 467),
    (384, 448),
    (381, 432),
    (370, 415),
]


def tent_regions(colour, provisional, ppm, pivot, door_landmark_fit=False):
    """Numeric traced masks: cover, pole tips/tails, wraps, thread and stone hem."""
    h, w = provisional.shape
    visible = colour[:, :, 3] > 0

    def point(p):
        if door_landmark_fit and ppm == 64:
            p = fitted_door_point(p)
        return (round(pivot[0] + (p[0] - 256) * ppm / 64), round(pivot[1] + (p[1] - 508) * ppm / 64))

    def polygon(points):
        im = Image.new("L", (w, h))
        ImageDraw.Draw(im).polygon([point(p) for p in points], fill=255)
        return np.asarray(im) > 0

    ids = np.where(visible, 7, 0).astype(np.uint8)
    stone = provisional == 6
    ring = polygon(
        [
            (157, 405),
            (137, 419),
            (126, 442),
            (133, 467),
            (155, 484),
            (212, 503),
            (262, 508),
            (327, 498),
            (368, 479),
            (394, 443),
            (380, 418),
            (366, 405),
            (365, 417),
            (378, 438),
            (374, 457),
            (354, 474),
            (311, 485),
            (269, 490),
            (251, 486),
            (211, 478),
            (167, 469),
            (144, 454),
            (144, 433),
            (156, 419),
        ]
    )
    edge = np.asarray(Image.fromarray((stone * 255).astype(np.uint8)).filter(ImageFilter.MaxFilter(3))) > 0
    dark = colour[:, :, :3].max(2) < 135
    stone |= edge & ring & dark & visible
    ids[stone] = 6
    top = polygon([(239, 263), (286, 263), (279, 288), (248, 288)])
    tails = polygon(
        [
            (249, 296),
            (276, 296),
            (279, 308),
            (273, 306),
            (270, 303),
            (269, 309),
            (263, 309),
            (262, 304),
            (258, 309),
            (253, 307),
            (253, 302),
            (248, 305),
        ]
    )
    wood = (top | tails) & visible & ~stone
    ids[wood] = 5
    wrap = polygon([(249, 286), (273, 286), (273, 296), (249, 296)]) & visible
    ids[wrap] = 8
    thread = np.zeros((h, w), bool)
    if ppm >= 16:
        im = Image.new("L", (w, h))
        d = ImageDraw.Draw(im)
        for points in SEAMS.get(ppm, []):
            coords = [point(p) for p in points] if ppm == 64 else points
            d.line(coords, fill=255, width=1)
        # Only the drawn brown sewing chip within inspected join corridors is thread.
        # The pale/ordinary panel interiors must never become a schematic network.
        sewing = np.all(colour[:, :, :3] == [170, 120, 72], axis=2)
        thread = (np.asarray(im) > 0) & sewing & visible & ~stone & ~wood & ~wrap
        ids[thread] = 8
    flap_points = [(257, 425), (247, 449), (237, 465), (246, 489), (252, 488), (249, 475), (256, 450)]
    flap = polygon(flap_points) & visible & ~stone
    return ids, {"stone": stone, "wood": wood, "wrap": wrap, "thread": thread, "flap": flap}


def tent_normals(colour, ids, regions, ppm, pivot):
    """Cone cover, vertical flap, local cylinders and hem domes in world east/south/up."""
    rows, cols = np.indices(ids.shape)
    x = np.clip((cols - pivot[0]) / (1.9 * ppm), -1, 1)
    vectors = np.dstack([0.75 * x, 0.75 * np.sqrt(np.maximum(0, 1 - x * x)), np.full(x.shape, 0.55)])
    for material in (5, 8):
        mask = ids == material
        if material == 8:
            mask &= regions["wrap"]
        local = np.clip((cols - (pivot[0] + 8 * ppm / 64)) / (0.22 * ppm), -1, 1)
        vectors[mask, 0] = (0.40 * local)[mask]
        vectors[mask, 1] = np.sqrt(1 - (0.40 * local[mask]) ** 2)
        vectors[mask, 2] = 0.08
    # Topmost pole caps face upward; lower visible wood remains cylindrical.
    caps = (ids == 5) & (rows <= pivot[1] - (508 - 274) * ppm / 64)
    vectors[caps] = [0, 0.2, 1]
    vectors[regions["flap"]] = [0.10, 1, 0.08]
    distances = []
    for cx, cy in STONE_CENTRES:
        px = pivot[0] + (cx - 256) * ppm / 64
        py = pivot[1] + (cy - 508) * ppm / 64
        distances.append(((cols - px) / (12 * ppm / 64)) ** 2 + ((rows - py) / (9 * ppm / 64)) ** 2)
    nearest = np.argmin(distances, axis=0)
    for i, (cx, cy) in enumerate(STONE_CENTRES):
        px = pivot[0] + (cx - 256) * ppm / 64
        py = pivot[1] + (cy - 508) * ppm / 64
        localx = np.clip((cols - px) / max(1, 12 * ppm / 64), -1, 1)
        localy = np.clip((rows - py) / max(1, 9 * ppm / 64), -1, 1)
        mask = (ids == 6) & (nearest == i)
        vectors[mask, 0] = (0.4 * localx)[mask]
        vectors[mask, 1] = (0.55 + 0.35 * localy)[mask]
        vectors[mask, 2] = (0.70 - 0.35 * localy)[mask]
    vectors /= np.maximum(np.linalg.norm(vectors, axis=2, keepdims=True), 1e-10)
    out = np.dstack([np.rint((vectors + 1) * 127.5).astype(np.uint8), colour[:, :, 3]])
    out[colour[:, :, 3] == 0] = 0
    return out
