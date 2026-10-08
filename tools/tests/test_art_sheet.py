"""The catalogue sheet (PRE-46, PRE-22, A5.3): each picture from above with a scale stick drawn by code at its true
length, a close-up, a 3 x 3 repeat, the camera views side by side, the states in rows, and every panel labelled on a
page 1080 pixels wide; and an object's views cut from magenta and stood at one true scale beside the adult and the
stick, its poses scaled as one."""

import json
import os
import sys
import tempfile
import unittest
from unittest import mock

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import sheet  # noqa: E402

GREEN = (90, 120, 60)


def stick_lengths(picture):
    """The longest run of the stick's two colours along each row of the picture, in pixels."""
    pixels = np.asarray(picture.convert("RGB")).astype(int)
    on = (np.abs(pixels - sheet.IVORY).sum(axis=2) == 0) | (np.abs(pixels - sheet.CHARCOAL).sum(axis=2) == 0)
    best = 0
    for row in on:
        run = 0
        for v in row:
            run = run + 1 if v else 0
            best = max(best, run)
    return best


RED = (200, 40, 40)
BLUE = (40, 60, 200)


def drawn(size, boxes, speck=True):
    """A picture like GPT's: flat magenta with coloured boxes, each (colour, (x0, y0, x1, y1)), and a stray speck."""
    pixels = np.zeros((size[1], size[0], 3), np.uint8)
    pixels[:] = sheet.KEY
    for colour, (x0, y0, x1, y1) in boxes:
        pixels[y0:y1, x0:x1] = colour
    if speck:
        pixels[5, 5] = (10, 10, 10)
    return Image.fromarray(pixels)


def extent(picture, colour):
    """The width and height of the box holding every pixel of exactly this colour."""
    pixels = np.asarray(picture.convert("RGB")).astype(int)
    ys, xs = np.nonzero(np.abs(pixels - colour).sum(axis=2) == 0)
    return xs.max() - xs.min() + 1, ys.max() - ys.min() + 1


def height(cut):
    """How many rows of a cut-out are more than half opaque."""
    rows = np.nonzero((np.asarray(cut)[:, :, 3] > 127).any(axis=1))[0]
    return rows.max() - rows.min() + 1


def upright_lengths(picture):
    """The longest run of the stick's two colours down any column of the picture, in pixels."""
    return stick_lengths(picture.transpose(Image.Transpose.TRANSPOSE))


class Sheet(unittest.TestCase):
    # checks: PRE-46
    def test_short_scale_caption_fits_without_extending_the_physical_stick(self):
        mark = sheet.lying(24, "10 cm")
        _, box = sheet.label_box("10 cm")
        self.assertGreaterEqual(mark.width, box[2] - box[0] + 8)
        self.assertEqual(stick_lengths(mark.crop((0, mark.height - 12, mark.width, mark.height))), 24)

    # checks: PRE-22 PRE-46
    def test_camera_marks_show_world_lengths_with_projected_heights(self):
        with tempfile.TemporaryDirectory() as d:
            drawn((20, 30), [(RED, (5, 2, 15, 28))], speck=False).save(os.path.join(d, "deer.png"))
            spec = {
                "views": {"items": [{"file": "deer.png", "tall": 1.9}]},
                "camera_objects": {
                    "items": [{"file": "deer.png", "tall": 1.9 * 0.7986355100472928}],
                    "stick": 2.1,
                    "phone_sizes": False,
                    "upright_sticks": [
                        {"metres": 1.2, "projected_metres": 1.2 * 0.7986355100472928, "label": "Shoulder"}
                    ],
                },
            }
            rows = []
            page = mock.Mock()
            page.figures.side_effect = lambda items: rows.extend(items)
            with mock.patch.object(sheet, "object_scale", return_value=128):
                sheet.compose_object(spec, page, d)
            marks = {label: image for image, label in rows if label}
            self.assertIn("Shoulder", marks)
            self.assertEqual(upright_lengths(marks["Shoulder"]), 123)
            self.assertTrue(any(stick_lengths(image) == 269 for image, _ in rows))

    # checks: PRE-46
    def test_a_stick_is_true_to_the_picture_it_is_drawn_on(self):
        for width, across, metres, length in ((508, 4, 1, 127), (508, 16, 10, 318), (336, 4, 1, 84)):
            shown = sheet.with_stick(Image.new("RGB", (width, width), GREEN), across, metres)
            self.assertEqual(stick_lengths(shown), length, f"{metres} m on {across} m shown {width} wide")

    # checks: PRE-46 PRE-22
    def test_a_sheet_shows_its_tiles_its_close_up_its_views_and_its_states(self):
        with tempfile.TemporaryDirectory() as d:
            Image.new("RGB", (1254, 1254), GREEN).save(os.path.join(d, "near.png"))
            Image.new("RGB", (1024, 1536), (120, 110, 70)).save(os.path.join(d, "cam.png"))
            Image.new("RGB", (1254, 1254), (200, 190, 170)).save(os.path.join(d, "snow.png"))
            spec = {
                "number": "1.1",
                "name": "Meadow",
                "about": "A test piece.",
                "palette": [["soil", "#A77950"], ["green", "#62733E"]],
                "tiles": [{"file": "near.png", "label": "Near: 4 m", "metres": 4, "stick": 1}],
                "close_up": {"tile": 0, "metres": 1, "at": [1, 1]},
                "repeat": [0],
                "camera": [["cam.png", "From the south"], ["cam.png", "From the north"]],
                "states": {"title": "States", "metres": 4, "stick": 1, "tiles": [["snow.png", "Snow"]] * 4},
                "notes": ["a note"],
            }
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = Image.open(out)
            self.assertEqual(page.width, sheet.WIDTH)
            self.assertGreater(page.height, 2000)
            # the near tile shown half the page wide: 4 m in 508 pixels, so its 1 m stick is 127 pixels long; the
            # close-up shows 1 m in 508 pixels, so its 10 cm stick is about 51
            right = sheet.MARGIN + sheet.HALF + sheet.GAP
            for name, box, length in (
                ("the near tile's 1 m stick", (0, 0, right, page.height), 127),
                ("the close-up's 10 cm stick", (right, 0, page.width, page.height), 51),
            ):
                column = page.crop(box)
                rows = [stick_lengths(column.crop((0, y, column.width, y + 1))) for y in range(column.height)]
                self.assertIn(length, rows, f"{name} is not {length} pixels long")

    # checks: PRE-46
    def test_a_strip_is_drawn_alone_across_the_page_with_a_stick_true_to_its_metres(self):
        wide = sheet.WIDTH - 2 * sheet.MARGIN
        near, middle = (90, 120, 60), (60, 90, 130)
        with tempfile.TemporaryDirectory() as d:
            Image.new("RGB", (3762, 1254), near).save(os.path.join(d, "near.png"))
            Image.new("RGB", (3072, 1024), middle).save(os.path.join(d, "middle.png"))
            spec = {
                "number": "1.2",
                "name": "Brook",
                "about": "A test piece.",
                "strips": [
                    {"file": "near.png", "label": "Near, three times: 12 m", "metres": 12, "stick": 1},
                    {"file": "middle.png", "label": "Middle, three times: 48 m", "metres": 48, "stick": 10},
                ],
            }
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = np.asarray(Image.open(out).convert("RGB")).astype(int)
            strips = ((near, "near", round(wide * 1 / 12)), (middle, "middle", round(wide * 10 / 48)))
            for colour, name, length in strips:
                rows = np.nonzero((np.abs(page - colour).sum(axis=2) == 0).any(axis=1))[0]
                row = page[rows[len(rows) // 2]]
                across = np.nonzero(np.abs(row - colour).sum(axis=1) == 0)[0]
                self.assertEqual((across.min(), across.max() + 1), (sheet.MARGIN, sheet.MARGIN + wide), f"{name}")
                height = round(wide * (1254 if name == "near" else 1024) / (3762 if name == "near" else 3072))
                self.assertEqual(rows.max() - rows.min() + 1, height, f"the {name} strip's height")
                lengths = [stick_lengths(Image.fromarray(page[y : y + 1].astype(np.uint8))) for y in rows]
                self.assertIn(length, lengths, f"the {name} strip's stick is not {length} pixels long")
            # the two strips stand one above the other, not side by side
            top = np.nonzero((np.abs(page - near).sum(axis=2) == 0).any(axis=1))[0]
            low = np.nonzero((np.abs(page - middle).sum(axis=2) == 0).any(axis=1))[0]
            self.assertLess(top.max(), low.min(), "the strips share a row")

    # checks: PRE-46
    def test_a_view_is_cut_from_magenta_and_scaled_to_its_true_size(self):
        cut = sheet.cut_out(drawn((600, 900), [(RED, (100, 150, 300, 750))]))
        self.assertEqual(cut.size, (200, 600), "the stray speck widened the cut, or the magenta stayed")
        self.assertEqual(sheet.to_scale(cut, 1.5, 200).size, (100, 300))
        self.assertEqual(sheet.to_scale(cut, 0.5, 300, "across").size, (150, 450))

    # checks: PRE-22 PRE-46
    def test_a_cleaned_tiny_figure_keeps_its_single_pixel_head_and_foot(self):
        pixels = np.full((7, 5, 3), sheet.KEY, np.uint8)
        pixels[1, 2] = pixels[5, 3] = RED
        pixels[2:5, 1:4] = RED
        drawing = Image.fromarray(pixels)
        self.assertEqual(sheet.cut_out(drawing).height, 3)
        kept = sheet.cut_out(drawing, least=1)
        self.assertEqual(kept.size, (3, 5))
        self.assertEqual(np.asarray(kept)[0, 1, 3], 255)
        self.assertEqual(np.asarray(kept)[-1, 2, 3], 255)

    # checks: PRE-46 PRE-22
    def test_a_small_tile_is_enlarged_pixel_for_pixel(self):
        checker = np.zeros((4, 4, 3), np.uint8)
        checker[::2, ::2] = checker[1::2, 1::2] = RED
        shown = np.asarray(sheet.fit(Image.fromarray(checker), 16)).reshape(-1, 3)
        self.assertEqual({tuple(c) for c in shown}, {(0, 0, 0), RED}, "the enlarged pixels were blurred")

    # checks: PRE-46
    def test_the_pink_fringe_where_a_drawing_was_blended_into_magenta_is_peeled(self):
        blend = tuple((np.array(RED) + np.array(sheet.KEY)) // 2)
        cut = sheet.cut_out(drawn((300, 300), [(blend, (98, 98, 202, 202)), (RED, (100, 100, 200, 200))]))
        self.assertEqual(cut.size, (100, 100), "the blended edge was kept")
        kept = np.asarray(cut).astype(int)
        pink = (kept[:, :, 3] > 0) & (kept[:, :, 0] - kept[:, :, 1] > 60) & (kept[:, :, 2] - kept[:, :, 1] > 60)
        self.assertFalse(pink.any(), "a pink fringe is left round the cut-out")

    # checks: PRE-46
    def test_a_strip_of_poses_is_scaled_as_one_from_its_first_pose(self):
        strip = drawn((900, 500), [(RED, (50, 60, 150, 460)), (BLUE, (400, 260, 600, 460))])
        first, second = sheet.poses(strip, 1.7, 100)
        self.assertEqual(height(first), 170, "the first pose is not 1.7 m tall")
        self.assertEqual(height(second), 85, "the second pose was not scaled with the first")

    # checks: PRE-46
    def test_an_object_sheet_stands_its_views_the_adult_and_the_stick_at_one_scale(self):
        with tempfile.TemporaryDirectory() as d:
            drawn((800, 1000), [(RED, (200, 100, 600, 900))]).save(os.path.join(d, "front.png"))
            drawn((800, 1000), [(RED, (300, 100, 500, 900))]).save(os.path.join(d, "side.png"))
            drawn((1000, 800), [(BLUE, (100, 200, 900, 600))]).save(os.path.join(d, "cam.png"))
            spec = {
                "number": "16.4",
                "name": "Hide tent",
                "about": "A test piece.",
                "views": {
                    "items": [
                        {"file": "front.png", "label": "Front", "tall": 2.8},
                        {"file": "side.png", "label": "Side", "tall": 2.8},
                    ],
                    "stick": 1,
                    "adult": True,
                },
                "camera_objects": {"items": [{"file": "cam.png", "label": "Sun behind", "across": 3.4}], "stick": 1},
                "groups": [
                    {"title": "Parts", "items": [{"file": "cam.png", "across": 3.4}], "above": True, "scale": 0.5}
                ],
            }
            cut = lambda name: sheet.cut_out(Image.open(os.path.join(d, name)))  # noqa: E731
            s = sheet.object_scale(spec, cut)
            self.assertEqual(s, int(sheet.TALLEST / 2.8))
            low = {"views": json.loads(json.dumps(spec["views"]))}
            for item in low["views"]["items"]:
                item["tall"] = 0.5
            self.assertEqual(
                sheet.object_scale(low, cut), int(sheet.TALLEST / sheet.ADULT), "the adult outgrows TALLEST"
            )
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = Image.open(out)
            self.assertEqual(extent(page, sheet.FIGURE)[1], round(1.7 * s), "the adult is not 1.7 m tall")
            self.assertEqual(upright_lengths(page), round(1 * s), "the upright stick is not 1 m long")
            lying = [stick_lengths(page.crop((0, y, page.width, y + 1))) for y in range(page.height)]
            self.assertIn(round(1 * s), lying, "no lying stick 1 m long")
            self.assertIn(round(0.5 * s), lying, "the group seen from above has no lying stick at its own scale")
            red = np.asarray(page.convert("RGB")).astype(int)
            ys = np.nonzero((np.abs(red - RED).sum(axis=2) == 0).any(axis=1))[0]
            self.assertEqual(ys.max() - ys.min() + 1, round(2.8 * s), "the front view is not 2.8 m tall")

    # checks: PRE-46
    def test_a_scale_stick_never_wraps_onto_a_row_of_its_own(self):
        def rows(widths):
            page = sheet.Sheet()
            page.figures([(Image.new("RGBA", (w, 100), RED + (255,)), "") for w in widths])
            return len(page.blocks)

        room = sheet.WIDTH - 2 * sheet.MARGIN
        self.assertEqual(rows([400, 400, 30]), 1, "a row that fits was wrapped")
        # three pictures and a stick 100 wide, 16 over the page's width with the usual gaps: the stick keeps to the row
        self.assertEqual(rows([300, 300, 300, 100]), 1, "the stick wrapped onto a row of its own")
        self.assertEqual(rows([room, 300]), 2, "a picture as wide as the page and another must take two rows")
        self.assertEqual(rows([500, 500, 500, 500]), 2, "four wide pictures: two rows, not three")

    # checks: PRE-46
    def test_the_true_size_row_leaves_out_a_zoom_where_the_picture_is_taller_than_the_screen(self):
        def shown(metres, px_per_m):
            return Image.new("RGB", (40, round(metres * px_per_m))), ""

        tree = sheet.true_size_row(20, shown)  # 2,560 pixels at the closest zoom, 640 and 160 at the others
        self.assertEqual([label.split(":")[0] for _, label in tree], ["The close camp", "The camp"])
        tent = sheet.true_size_row(3, shown)
        self.assertEqual(len(tent), 3, "a thing that fits every zoom lost one")

    # checks: PRE-46
    def test_wide_views_stand_in_one_row_with_the_adult_and_every_colour_chip_shows(self):
        with tempfile.TemporaryDirectory() as d:
            drawn((1000, 800), [(RED, (100, 200, 900, 800))]).save(os.path.join(d, "front.png"))
            drawn((1000, 800), [(BLUE, (100, 200, 900, 800))]).save(os.path.join(d, "side.png"))
            chips = [["", f"#{40 + 20 * i:02X}5A1E"] for i in range(9)]
            views = [{"file": "front.png", "tall": 3}, {"file": "side.png", "tall": 3}]
            spec = {
                "number": "16.4",
                "name": "Hide tent",
                "about": "A test piece: two views each 4 m wide.",
                "palette": chips,
                "views": {"items": views, "stick": 1, "adult": True},
            }
            with open(os.path.join(d, "spec.json"), "w") as f:
                json.dump(spec, f)
            out = os.path.join(d, "sheet.png")
            self.assertEqual(sheet.main(["sheet.py", os.path.join(d, "spec.json"), d, out]), 0)
            page = np.asarray(Image.open(out).convert("RGB")).astype(int)

            def lowest(colour):
                return np.nonzero((np.abs(page - colour).sum(axis=2) == 0).any(axis=1))[0].max()

            for colour, name in ((RED, "front"), (BLUE, "side")):
                self.assertLessEqual(abs(lowest(colour) - lowest(sheet.FIGURE)), 1, f"the {name} and the adult part")
            last = tuple(int(chips[-1][1][i : i + 2], 16) for i in (1, 3, 5))
            self.assertTrue((np.abs(page - last).sum(axis=2) == 0).any(), "the ninth colour chip is missing")


if __name__ == "__main__":
    unittest.main()
