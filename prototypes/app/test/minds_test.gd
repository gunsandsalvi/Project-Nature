## P6's screen and its extension: a thousand minds live a game day through the app, and the line for
## the chat holds the speed once warm, each part's share of the time, the heat and the checksum.
extends GdUnitTestSuite

const MINDS := preload("res://minds/minds.gd")


# checks: TIM-07, MND-15
func test_the_app_runs_a_thousand_minds_through_its_extension() -> void:
	assert_bool(ClassDB.class_exists("Minds")).is_true()
	var minds: RefCounted = ClassDB.instantiate("Minds")
	minds.make(4)
	var seconds: float = minds.run_days(1)
	assert_float(seconds).is_greater(0.0)
	var stats: Dictionary = minds.stats()
	assert_int(int(stats.people)).is_equal(1000)
	assert_int(int(stats.days)).is_equal(1)
	assert_int(int(stats.decisions)).is_greater(10000)
	assert_str(minds.checksum()).has_length(16)
	assert_str(minds.explain(0)).is_not_empty()


# checks: TIM-07
func test_the_speed_counts_only_once_the_phone_has_warmed() -> void:
	var days: Array[Vector2] = [
		Vector2(60, 30), Vector2(120, 60), Vector2(300, 120), Vector2(600, 240)
	]
	# from minute 2 (120 s, day 60) to the end (600 s, day 240): 180 days in 480 s
	assert_float(MINDS.held_speed(days, 2.0)).is_equal_approx(0.375, 1e-6)


# checks: TIM-07, MND-15
func test_the_line_for_the_chat() -> void:
	var days: Array[Vector2] = [Vector2(120, 240), Vector2(600, 1200)]
	var stats := {
		"days": 1200,
		"people": 1000,
		"decisions": 43200000,
		"choice": 5.0,
		"paths": 2.0,
		"talk": 1.0,
		"results": 1.0,
		"other": 1.0,
	}
	assert_str(MINDS.line("P6 α0.4b Pixel", days, stats, 10.0, "0123456789abcdef", [])).is_equal(
		(
			"P6 α0.4b Pixel | held 2.00 game years a minute | warming 2.00 | 4 threads, 1000 people"
			+ " | choice 50% paths 20% talk 10% results 10% other 10% | 36000 decisions a game day"
			+ " | sum 0123456789abcdef"
		)
	)
