## The accelerated smoke proves the measurement route, not phone performance.
extends GdUnitTestSuite


func test_separate_camp_measurement_keeps_three_speed_reports() -> void:
	var page := preload("res://pages/camp_measure.gd").new()
	page.time_scale = 1000
	add_child(page)
	assert_str(page.root).is_equal("user://camp-measure")
	assert_bool(page.world.is_paused()).is_true()
	page.start_measurement()
	var deadline := Time.get_ticks_msec() + 5000
	while page.report.is_empty() and Time.get_ticks_msec() < deadline:
		await await_idle_frame()
	assert_bool(page.report.is_empty()).is_false()
	var result: Dictionary = JSON.parse_string(page.report)
	assert_int(result.phases.size()).is_equal(3)
	(
		assert_array(result.phases.map(func(p: Dictionary) -> int: return int(p.speed)))
		. is_equal([1, 60, 3600])
	)
	assert_float(result.time_scale).is_equal(1000.0)
	assert_bool(page.world.is_paused()).is_true()
	assert_bool(result.samples.is_empty()).is_false()
	assert_bool(FileAccess.file_exists("user://camp-measurement.json")).is_true()
	page.free()
	Worlds.remove_tree("user://camp-measure")
