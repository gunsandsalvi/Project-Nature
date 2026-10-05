"""P4's rules and its answer (IMPLEMENTATION α0.3a): a try's chance, what fits, noticing, dreams, learning, skill,
loss, and the sharp-stone test and fire's window with the tuned values."""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import discovery as d  # noqa: E402
import pace  # noqa: E402


def band(seed=1, **scene):
    t = d.load()
    t["scene"].update(scene)
    return d.Band(seed, t)


class Rules(unittest.TestCase):
    # checks: MAT-04
    def test_a_try_succeeds_one_time_in_two_at_its_difficulty(self):
        self.assertEqual(d.chance(4.0, 4), 0.5)
        self.assertAlmostEqual(d.chance(6.0, 4), 0.7)
        self.assertAlmostEqual(d.chance(1.0, 4), 0.2)
        self.assertEqual(d.chance(20.0, 4), 0.95)
        self.assertEqual(d.chance(-20.0, 4), 0.05)

    # checks: RCK-01 RES-03
    def test_without_stone_that_flakes_no_flake_ever_comes(self):
        for seed in range(1, 21):
            b = band(seed, flint=0.0).run(7)
            self.assertNotIn("flake", b.first)

    # checks: RCK-02
    def test_wet_wood_never_gives_an_ember(self):
        t = d.load()
        t["scene"]["dry"] = [0.0, 0.0, 0.0, 0.0]
        for bp in d.FIRE:
            t["blueprints"][bp]["factor"] = 1.0
        for seed in range(1, 11):
            self.assertIsNone(d.fire_first(d.Band(seed, t).run(10)))

    # checks: MND-10
    def test_the_curious_notice_more_and_the_busy_half_as_often(self):
        b = band()
        p = b.people[0]
        rates = {}
        for curious in (0.0, 1.0):
            p.curious = curious
            for busy in (False, True):
                rates[curious, busy] = sum(b.notice(p, busy) for _ in range(20000)) / 20000
        self.assertAlmostEqual(rates[0.0, False], 0.25, delta=0.02)
        self.assertAlmostEqual(rates[1.0, False], 0.75, delta=0.02)
        self.assertAlmostEqual(rates[1.0, True], 0.375, delta=0.02)

    # checks: MND-11
    def test_a_try_with_a_hunch_beats_one_by_accident_ten_times_over(self):
        b = band()
        p = next(q for q in b.people if q.age >= d.ADULT)
        p.curious = 1.0
        t = b.t
        found = {}
        for route in ("accident", "hunch"):
            n = 0
            for _ in range(20000):
                p.practice.pop("flake", None)
                n += b.roll(p, "flake", t["routes"][route], False, route, hints=False)
            found[route] = n
        self.assertGreater(found["accident"], 0)
        self.assertAlmostEqual(found["hunch"] / found["accident"], 10.0, delta=2.5)

    # checks: MND-12
    def test_a_dream_points_only_to_what_its_dreamer_has_done_and_handled(self):
        b = band()
        b.t["minds"].update(dream_hint=1.0, dream_real=1.0)
        live = [p for p in b.people if p.alive and p.age >= d.ADULT]
        for p in live:
            p.handled.add("flint")
        b.night(live, False)
        # nobody has drilled or ploughed yet, so no dream points to fire; flint struck points to flakes
        for p in live:
            self.assertEqual([k for k in p.hunches if isinstance(k, str)], ["flake"])
        p = live[0]
        p.hunches.clear()
        p.practice["flake"] = d.START
        p.actions.add("drill")
        b.night([p], False)
        self.assertEqual(list(p.hunches), ["drill"])

    # checks: MND-13
    def test_watching_teaches_after_about_five_uses_and_teaching_spreads_it_faster(self):
        learned = {}
        for kind in (0.0, 1.0):
            total = 0
            for seed in range(1, 11):
                b = band(seed)
                live = [p for p in b.people if p.alive]
                teacher = next(p for p in live if p.age >= d.ADULT)
                teacher.kind = kind
                b.learn(teacher, "flake", "accident")
                for _ in range(40):
                    b.watch_and_teach(teacher, "flake", live, {})
                total += b.known_by["flake"] - 1
            learned[kind] = total
        self.assertGreater(learned[0.0], 0)
        self.assertGreater(learned[1.0], learned[0.0])

    # checks: MND-06
    def test_knapping_most_days_reaches_skill_5_in_one_and_a_half_to_three_years(self):
        within = 0
        for seed in range(1, 21):
            b = band(seed)
            p = next(q for q in b.people if q.age >= d.ADULT)
            b.learn(p, "flake", "accident")
            day = 0
            while d.skill(p.practice["flake"]) < 5.0 and day < 10 * d.YEAR:
                if b.rng.random() < 0.75:
                    b.use(p, "flake")
                day += 1
            within += 1.5 <= day / d.YEAR <= 3.0
        self.assertGreaterEqual(within, 16)

    # checks: CUL-02
    def test_a_blueprint_dies_with_its_last_holder(self):
        b = band()
        p = next(q for q in b.people if q.age >= d.ADULT)
        b.learn(p, "flake", "accident")
        b.die(p)
        self.assertEqual(b.known_by["flake"], 0)
        self.assertEqual(b.lost, [(0, "flake")])


def flake_run(year, spread=0.8, route="accident"):
    return {"flake": year, "spread": spread, "flake_route": route}


def fire_world(year):
    return {"fire": year, "fire_way": "drill", "fire_route": "dream"}


class PassRules(unittest.TestCase):
    """The pass rules on made-up runs, so a rule read wrongly fails here whatever the model does."""

    # checks: TIM-19
    def test_fires_window_is_its_dates(self):
        # Years 3 to 15 run from 2 years in up to 15
        w = pace.window([1.99, 2.0, 14.99, 15.0, None], pace.FIRE_WINDOW)
        self.assertEqual((w["early"], w["inside"], w["never"]), (1, 2, 1))
        w = pace.window([0.0, 2.99, 3.0], pace.FLAKE_WINDOW)
        self.assertEqual((w["early"], w["inside"]), (0, 2))

    # checks: TIM-19
    def test_a_window_needs_half_inside_and_at_most_a_quarter_early(self):
        span = pace.FIRE_WINDOW
        self.assertTrue(pace.window([1.0] * 5 + [8.0] * 10 + [20.0] * 5, span)["pass"])
        self.assertFalse(pace.window([1.0] * 6 + [8.0] * 10 + [20.0] * 4, span)["pass"])
        self.assertFalse(pace.window([8.0] * 9 + [20.0] * 11, span)["pass"])

    # checks: RES-03
    def test_the_sharp_stone_rule(self):
        good = [flake_run(1.0)] * 8 + [flake_run(2.0, route="dream")] * 8 + [flake_run(None, None, None)] * 4
        control = [flake_run(None, None, None)] * 20
        self.assertTrue(pace.sharp_stone(good, control)["pass"])
        # 15 of 20 within 5 years
        self.assertFalse(pace.sharp_stone(good[:15] + [flake_run(6.0)] * 5, control)["pass"])
        # one run whose craft did not spread to 3 in 4 adults
        self.assertFalse(pace.sharp_stone([flake_run(1.0, spread=0.5)] + good[1:], control)["pass"])
        # a single route
        self.assertFalse(pace.sharp_stone([flake_run(1.0)] * 16 + good[16:], control)["pass"])
        # a flake without stone that flakes
        self.assertFalse(pace.sharp_stone(good, [flake_run(3.0)] + control[1:])["pass"])

    # checks: TIM-19 RES-03
    def test_a_sweeps_summary_and_what_holds_the_pace(self):
        flakes = [flake_run(1.0)] * 40
        steady = pace.summary(flakes, [fire_world(8.0)] * 40)
        early = pace.summary(flakes, [fire_world(1.0)] * 11 + [fire_world(8.0)] * 29)
        late = pace.summary([flake_run(4.0)] * 40, [fire_world(8.0)] * 40)
        self.assertTrue(steady["flake_pass"] and steady["fire_pass"])
        self.assertFalse(early["fire_pass"])
        self.assertFalse(late["flake_pass"])
        row = {"0.8": steady, "1.25": early, "0.5": steady, "2.0": steady}
        self.assertFalse(pace.holds(row, (0.8, 1.25)))
        self.assertTrue(pace.holds(row, (0.5, 2.0)))


class Answer(unittest.TestCase):
    # checks: RES-02 RES-03 RSK-01
    def test_the_tuned_pace_passes_the_sharp_stone_test(self):
        t = d.load()
        stone = pace.sharp_stone(pace.runs(t, pace.SEEDS), pace.runs(t, pace.SEEDS, flint=0.0))
        self.assertTrue(stone["pass"], stone)

    # checks: TIM-19 RSK-01
    def test_the_tuned_pace_brings_a_worlds_first_fire_inside_its_window(self):
        fire = pace.fire_window(pace.worlds(d.load(), pace.SEEDS))
        self.assertTrue(fire["pass"], fire)


if __name__ == "__main__":
    unittest.main()
