"""The target card's bands (PRE-01, A5.5): each moment's bands span its chosen pictures widened by the slack, hues
round the circle; the card passes on every picture you chose and fails on a flat, speckled picture made by code."""

import os
import sys
import unittest

import numpy as np

sys.path.insert(0, os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "art"))
import card  # noqa: E402
import look  # noqa: E402


class Bands(unittest.TestCase):
    # checks: PRE-01
    def test_hues_take_the_shortest_arc_round_the_circle(self):
        self.assertEqual(card.arc([350.0, 10.0, 20.0]), (350.0, 20.0))
        self.assertEqual(card.arc([40.0, 80.0, 60.0]), (40.0, 80.0))
        self.assertEqual(card.arc([100.0]), (100.0, 100.0))
        self.assertEqual(card.band("lights_hue", [350.0, 10.0, 20.0], 10.0, False), (340, 30))
        self.assertEqual(card.band("lights_hue", [0.0, 120.0, 240.0], 10.0, False), (350, 250), "one gap left out")
        self.assertEqual(card.band("lights_hue", [0.0, 90.0, 180.0, 270.0], 50.0, False), (0, 359), "the whole circle")

    # checks: PRE-01
    def test_a_band_spans_its_pictures_widened_by_the_slack(self):
        self.assertEqual(card.band("lightness", [56.93, 45.7], 3.0, True), ('"42.7%"', '"60.0%"'))
        self.assertEqual(card.band("dark", [2.0], 6.0, True), ('"0.0%"', '"8.0%"'), "a share stays at 0 or more")
        self.assertEqual(
            card.band("shade", [-2.06, 1.56], 1.5, True), ('"-3.6%"', '"3.1%"'), "yellowness may fall below 0"
        )
        self.assertEqual(card.band("things", [24.54, 28.99], 3.0, False), (21, 32))

    # checks: PRE-01
    def test_tuning_values_read_as_the_card_s_numbers(self):
        self.assertEqual(card.amount("3.5%"), 3.5)
        self.assertEqual(card.amount(10), 10.0)


@unittest.skipUnless(os.path.exists(look.program()), "kindling is not built (set KINDLING to its path)")
class TheRealCard(unittest.TestCase):
    # checks: PRE-01
    def test_every_picture_you_chose_passes_its_moment(self):
        for name, _, pictures in card.moments():
            for p in pictures:
                _, alarms = look.card(card.picture(p), name)
                self.assertEqual(len(alarms), 12)
                self.assertNotIn("red", alarms.values(), f"{p} at {name}: {alarms}")

    # checks: PRE-01
    def test_a_flat_speckled_picture_made_by_code_fails(self):
        t = np.empty((256, 256, 3), np.uint8)
        t[:] = (100, 140, 60)
        t[np.random.default_rng(7).random((256, 256)) < 0.04] = (240, 220, 80)
        _, alarms = look.card(t, "late_afternoon")
        for stat in ("things", "largest_colour", "texture", "masses"):
            self.assertEqual(alarms[stat], "red", stat)


if __name__ == "__main__":
    unittest.main()
