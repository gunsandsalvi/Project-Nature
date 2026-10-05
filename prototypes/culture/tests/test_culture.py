"""P10's rules and its answer (IMPLEMENTATION α0.6b): a strong outcome linked to the most unusual thing before it and
tested by what follows, customs from a band's own cases, a band past its size splitting, the same run from the same
seed, and the report's answer."""

import json
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import culture as c  # noqa: E402
import runs  # noqa: E402


def world(seed=1):
    return c.World(seed, c.load())


class Rules(unittest.TestCase):
    # checks: MND-05
    def test_a_strong_outcome_links_to_the_most_unusual_thing_before_it(self):
        w = world()
        band = w.bands[0]
        place, sang, eggs = ("place", "the red cliff"), ("did", "sang"), ("ate", "eggs")
        band.counts = {place: 40, sang: 1}
        band.last_seen = {place: w.day, sang: w.day}
        self.assertEqual(w.unusual(band, {place, sang}), sang)
        hunter = w.adults(band)[0]
        w.outcome(band, "good hunt", [hunter], {place, sang}, 0)
        # a new link starts at three times the thought's size: 30 for a good hunt's 10
        self.assertEqual(hunter.links["good hunt"][sang][0], 30.0)
        # the same cause before the next good hunt explains it: the link grows by 15, and nothing new forms
        w.outcome(band, "good hunt", [hunter], {sang, eggs}, 1)
        self.assertEqual(hunter.links["good hunt"][sang][0], 45.0)
        self.assertNotIn(eggs, hunter.links["good hunt"])

    # checks: MND-05
    def test_a_cause_not_followed_by_its_outcome_weakens_and_is_forgotten(self):
        w = world()
        band = w.bands[0]
        p = w.adults(band)[0]
        p.links["good hunt"] = {("did", "sang"): [12.0, 0]}
        band.log.extend([{("did", "sang")}, set(), set()])
        band.outcome_log.extend([set(), set(), set()])
        w.test_links(band)
        self.assertEqual(p.links["good hunt"][("did", "sang")][0], 7.0)
        w.test_links(band)
        self.assertNotIn(("did", "sang"), p.links["good hunt"])

    # checks: CUL-06
    def test_a_custom_is_the_way_two_thirds_of_a_bands_cases_went(self):
        w = world()
        band = w.bands[0]
        w.case(band, "dead", "buried", 0)
        w.case(band, "dead", "left", 0)
        self.assertNotIn("dead", band.customs)
        w.case(band, "dead", "buried", 0)
        self.assertEqual(band.customs["dead"], "buried")
        # where couples live, with no way at two thirds: the mixed answer
        for answer in ("his kin", "her kin", "his kin", "her kin"):
            w.case(band, "home", answer, 0)
        self.assertEqual(band.customs["home"], "either")

    # checks: CUL-30
    def test_a_band_past_40_splits_and_those_least_fond_of_its_leader_leave(self):
        w = world(3)
        band = w.bands[0]
        w._families(band, 46)
        w._choose_leader(band)
        leader = w.people[band.leader]
        before = len(band.members)
        w.season_start(band)
        self.assertEqual(len(w.bands), 4)
        new = w.bands[-1]
        left = len(new.members)
        self.assertTrue(6 <= left <= before / 2)
        self.assertGreaterEqual(len(band.members), 10)
        mean = lambda heads: sum(w.opinion(h, leader) for h in heads) / len(heads)  # noqa: E731
        stayed = [h for h in w.heads(band) if h.id != leader.id and h.id not in w.family(leader)]
        self.assertLess(mean(w.heads(new)), mean(stayed))
        self.assertEqual(w.firsts["split"]["name"], f"the {new.name} band, from the {band.name}")

    # checks: CUL-33
    def test_the_same_seed_makes_the_same_history(self):
        a, b = world(5), world(5)
        a.run(6)
        b.run(6)
        self.assertEqual(a.firsts, b.firsts)
        self.assertEqual(a.events, b.events)


class Answer(unittest.TestCase):
    # checks: CUL-05 CUL-06 CUL-30 CUL-34
    def test_a_run_brings_each_first_from_the_events_behind_it(self):
        w = world(1)
        w.run(30)
        for kind in ("custom", "spirit", "rite", "split"):
            self.assertIn(kind, w.firsts)
            story = w.story(kind)
            self.assertGreaterEqual(len(story), 2, kind)

    # checks: CUL-33
    def test_the_report_has_each_first_inside_its_window_in_at_least_half_the_runs(self):
        r = json.loads(runs.REPORT.read_text())
        self.assertTrue(r["pass"])
        for kind, k in r["kinds"].items():
            self.assertGreaterEqual(k["inside"] * 2, len(r["runs"]), kind)
            self.assertEqual(k["traced"], k["came"], kind)
        self.assertEqual(r["kinds"]["rite"]["window"], [3.0, 10.0])


if __name__ == "__main__":
    unittest.main()
