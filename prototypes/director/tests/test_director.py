"""P11's rules and its answer (IMPLEMENTATION α0.6c): the recognisers' patterns over an event log, the director's one
budget, its rest after each slowdown and its only-slower speed, the clock it plays a world by, ages from turning
points, and the same world run with the director on and off."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import director as dr  # noqa: E402
import watch  # noqa: E402

T = dr.load()


def ev(day, kind, who="", what="", people="A", band="A", hour=12.0, how=""):
    return (day, hour, people, band, kind, who, what, how)


def kinds(found):
    return [f.kind for f in found]


class Recognisers(unittest.TestCase):
    # checks: PRE-39
    def test_a_peoples_first_success_is_a_named_discovery_and_a_world_first_once(self):
        r = dr.Recognisers(T)
        self.assertEqual(kinds(r.see(ev(1, "learn", "a1", "flake"))), ["world first discovery"])
        self.assertEqual(kinds(r.see(ev(2, "learn", "a2", "flake"))), [])
        self.assertEqual(kinds(r.see(ev(3, "learn", "b1", "flake", people="B", band="B"))), ["named discovery"])
        self.assertEqual([step for _, step, _ in r.turning], ["sharp stone flakes"])

    # checks: PRE-39
    def test_a_craft_dies_with_its_last_holder_and_its_return_is_a_rediscovery(self):
        r = dr.Recognisers(T)
        r.see(ev(1, "learn", "a1", "drill"))
        r.see(ev(2, "learn", "a2", "drill"))
        self.assertEqual(kinds(r.see(ev(3, "death", "a1"))), [])
        lost = r.see(ev(4, "death", "a2"))
        self.assertEqual(kinds(lost), ["craft lost"])
        self.assertEqual(lost[0].what, "drill")
        # a fire gone out with no one left who can make one is a band losing its last fire
        self.assertEqual(kinds(r.see(ev(5, "fire out"))), ["last fire"])
        self.assertEqual(kinds(r.see(ev(6, "learn", "a3", "drill"))), ["rediscovery"])
        self.assertEqual(kinds(r.see(ev(7, "fire out"))), [])

    # checks: PRE-39
    def test_firsts_count_each_kind_once_and_feuds_are_a_second_fight(self):
        r = dr.Recognisers(T)
        custom = "the dead are buried"
        self.assertEqual(kinds(r.see(ev(1, "custom", what=custom))), ["world first"])
        self.assertEqual(kinds(r.see(ev(2, "custom", what=custom, band="B"))), [])
        self.assertEqual(kinds(r.see(ev(3, "custom", what="the dead are left where they fell"))), ["world first"])
        self.assertEqual(kinds(r.see(ev(4, "fight", "a1", "a2"))), ["world first"])
        self.assertEqual(kinds(r.see(ev(5, "fight", "a2", "a1"))), ["feud"])
        # a lightning strike that kills is a disaster and, the first time, a world first
        self.assertEqual(
            kinds(r.see(ev(6, "lightning", "a3", "the red cliff"))), ["world first", "lightning at a camp"]
        )

    # checks: PRE-39
    def test_the_lives_of_those_you_follow(self):
        r = dr.Recognisers(T, followed=["a1"])
        self.assertEqual(kinds(r.see(ev(1, "birth", "a9", "a1"))), ["birth to one you follow"])
        self.assertEqual(kinds(r.see(ev(2, "birth", "a8", "a2"))), [])
        self.assertEqual(kinds(r.see(ev(3, "death", "a2"))), [])
        self.assertEqual(kinds(r.see(ev(4, "death", "a1"))), ["death of one you follow"])

    # checks: TIM-02, PRE-39
    def test_a_half_matched_pattern_is_a_sign_and_its_end_is_counted(self):
        r = dr.Recognisers(T, signs="end")
        r.see(ev(1, "hunch", "a1", "drill", hour=22.0))
        self.assertEqual(kinds(r.see(ev(2, "try", "a1", "drill", hour=7.0))), [])
        signs = r.see(ev(3, "try", "a1", "drill", hour=7.0))
        self.assertEqual(kinds(signs), ["a hunch tried again"])
        # scored at what its end would be worth: a world first discovery
        self.assertEqual(signs[0].score, 100.0)
        self.assertEqual(kinds(r.see(ev(4, "try", "a1", "drill", hour=7.0))), ["a hunch tried again"])
        # the try that succeeds ends the pattern: its sign came true within what a slowdown shows, the earlier not
        self.assertEqual(kinds(r.see(ev(4, "learn", "a1", "drill", hour=8.75, how="hunch"))), ["world first discovery"])
        self.assertEqual([came for _, _, came in r.signs], [False, True])
        # a hunch for what its people can already make is no sign; nor one for nothing
        r.see(ev(5, "hunch", "a2", "drill"))
        r.see(ev(5, "try", "a2", "drill"))
        self.assertEqual(kinds(r.see(ev(6, "try", "a2", "drill"))), [])
        r.see(ev(5, "hunch", "a3", "nothing"))
        r.see(ev(5, "try", "a3", "nothing"))
        self.assertEqual(kinds(r.see(ev(6, "try", "a3", "nothing"))), [])

    # checks: TIM-02
    def test_a_signs_expected_score_is_its_ends_times_how_often_it_came(self):
        r = dr.Recognisers(T)
        sign = r.see(ev(1, "storm", band="A"))[0]
        self.assertAlmostEqual(sign.score, 30.0 * T["signs"]["a storm over a camp"])
        self.assertEqual(kinds(r.see(ev(1, "lightning", band="A", hour=13.0))), ["lightning at a camp"])
        self.assertEqual(r.signs[0][2], True)


class Director(unittest.TestCase):
    def finding(self, score, span=6.0):
        return dr.Finding("named discovery", ev(1, "learn"), score, span)

    # checks: TIM-02
    def test_one_budget_at_most_one_slowdown_every_three_minutes(self):
        top = 5.0
        for taps, length in ((False, 10.0), (True, 30.0)):
            d = dr.Director(T, top, taps)
            for k in range(360):
                d.consider(self.finding(100.0), k * 10.0)
            starts = [s for s, _, _, _ in d.slowdowns]
            self.assertEqual(len(starts), 20)
            self.assertTrue(all(b - a >= 180.0 for a, b in zip(starts, starts[1:], strict=False)))
            self.assertTrue(all(e - s == length for s, e, _, _ in d.slowdowns))
            self.assertLessEqual(d.slowed(0.0, 3600.0), 0.2 * 3600.0)
            self.assertEqual(len(d.listed), 340)

    # checks: TIM-02
    def test_after_a_slowdown_only_a_higher_score_slows_time_soon(self):
        d = dr.Director(T, 5.0)
        # a watch opens rested: only a major moment slows time at first
        self.assertFalse(d.consider(self.finding(60.0), 0.0))
        self.assertTrue(d.consider(self.finding(100.0), 0.0))
        self.assertFalse(d.consider(self.finding(100.0), 100.0))  # inside the gap
        self.assertFalse(d.consider(self.finding(60.0), 200.0))  # the rest still stands
        self.assertTrue(d.consider(self.finding(100.0), 200.0))
        self.assertFalse(d.consider(self.finding(60.0), 600.0))
        self.assertTrue(d.consider(self.finding(60.0), 1000.0))  # the rest has fallen back by 10 minutes after
        self.assertFalse(d.consider(self.finding(40.0), 2000.0))  # below the bar: not listed either
        self.assertEqual(len(d.listed), 4)

    # checks: TIM-02
    def test_only_slower_and_half_a_minute(self):
        d = dr.Director(T, 5.0)
        d.consider(self.finding(100.0, span=6.0), 0.0)
        self.assertAlmostEqual(d.speed(5.0)[0], 6.0 / 24.0 / 30.0)
        self.assertEqual(d.speed(10.0), (5.0, None))
        d.consider(self.finding(100.0, span=24.0 * 1000.0), 200.0)
        self.assertEqual(d.speed(205.0)[0], 5.0)

    # checks: TIM-02
    def test_the_watch_plays_game_time_at_the_speed_asked(self):
        d = dr.Director(T, 5.0)
        r = dr.Recognisers(T)
        events = [ev(10, "learn", "a1", "flake", hour=0.0), ev(20, "death", "a2", hour=0.0)]
        real = dr.watch(iter(events), r, d, 30)
        # 10 days at 5 a second, 10 seconds slowed to a day in 2 minutes, then the rest at 5 a second
        slow = 6.0 / 24.0 / 30.0
        self.assertAlmostEqual(real, 2.0 + 10.0 + (20.0 - 10.0 * slow) / 5.0)


class Ages(unittest.TestCase):
    # checks: PRE-39
    def test_an_age_begins_only_at_a_turning_point_and_lasts_its_least_years(self):
        turning = [(60.0, "sharp stone flakes", None), (300.0, "making fire", None)]
        self.assertEqual(dr.ages(turning, 20), [(60.0, "The age of sharp stone flakes")])
        self.assertEqual(
            [n for _, n in dr.ages(turning, 3)], ["The age of sharp stone flakes", "The age of making fire"]
        )


class HandsOff(unittest.TestCase):
    # checks: TIM-03, PRE-39
    def test_no_path_from_the_director_into_a_world(self):
        self.assertTrue(watch.hands_off())
        self.assertTrue(watch.plain(ev(1, "learn", "a1", "flake")))
        self.assertFalse(watch.plain((1, 2.0, "A", "A", "learn", "a1", ["flake"], "")))

    # checks: TIM-03
    def test_the_same_world_with_the_director_on_and_off_ends_identical(self):
        days = 2 * dr.YEAR
        top = T["speed"]["top"] * dr.YEAR / 60.0
        on = watch.Valley(3)
        d = dr.Director(T, top)
        dr.watch(watch.live(on, days), dr.Recognisers(T, on.followed()), d, days)
        self.assertGreater(len(d.slowdowns), 0)
        off = watch.Valley(3)
        off.run(2)
        self.assertEqual(on.digest(), off.digest())
        # a host that lets the director reach the world's own chance is caught
        meddled = watch.Valley(3)
        d2 = dr.Director(T, top)
        dr.watch(
            watch.live(meddled, days, watch.careless(meddled, d2)), dr.Recognisers(T, meddled.followed()), d2, days
        )
        self.assertNotEqual(meddled.digest(), off.digest())


class Answer(unittest.TestCase):
    def row(self, gap=200.0, share=0.05, kept=0.95):
        return {
            "minutes": 20.0, "slowdowns": 6, "per_hour": 18.0, "least_gap": gap, "worst_share": share, "kept": kept,
            "listed": 50, "listed_per_hour": 150.0, "listed_early": 30, "kinds": {}, "signs": {}, "discoveries": 9,
            "discoveries_caught": 9, "discoveries_slowed": 2, "foretold": 0, "sign_slowdowns": 0,
        }  # fmt: skip

    # checks: TIM-02
    def test_the_budget_holds_only_if_every_watch_keeps_it(self):
        budget = T["budget"]
        runs = [{"ways": {"never taps": self.row()}}] * 3
        self.assertTrue(watch.summary(runs, "never taps", budget)["held"])
        for bad in (self.row(gap=170.0), self.row(share=0.25), self.row(kept=0.75)):
            runs = [{"ways": {"never taps": self.row()}}] * 2 + [{"ways": {"never taps": bad}}]
            self.assertFalse(watch.summary(runs, "never taps", budget)["held"])


if __name__ == "__main__":
    unittest.main()
