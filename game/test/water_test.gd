## The river's water for the pages that draw it (A4.5, PRE-26), headless in the cloud: its globals
## come from the tuning and the river's level, and its tick runs ten times a second, holds for a
## picture and goes on from where it was held.
extends GdUnitTestSuite


func _water() -> Water:
	var world := KdWorld.new()
	GameData.load_into(world)
	var water := Water.new()
	assert_str(water.apply(world, -0.4)).is_empty()
	return water


# checks: PRE-26
func test_the_water_s_globals_are_the_tuning_s_and_every_one_is_declared_in_the_project() -> void:
	var water := _water()
	assert_int(water.rate).is_equal(10)
	# every global the shaders read is one the project declares, so they find it (the headless
	# renderer keeps no values to read back)
	for name: String in [
		"kd_water",
		"kd_water_absorb",
		"kd_water_deep",
		"kd_water_wet",
		"kd_water_surface",
		"kd_water_glint",
		"kd_water_shore",
		"kd_origin_near",
		"kd_origin_middle",
		"kd_origin_far"
	]:
		assert_bool(ProjectSettings.has_setting("shader_globals/" + name)).is_true()
	# a world with no water says so
	assert_str(Water.new().apply(KdWorld.new(), -0.4)).contains("tuning/water")


# checks: PRE-26 PRE-22
func test_the_tick_runs_ten_times_a_second_holds_for_a_picture_and_goes_on_from_where_it_was(
) -> void:
	var water := _water()
	assert_int(water.tick).is_equal(0)
	water.step(0.05)
	assert_int(water.tick).is_equal(0)
	water.step(0.06)
	assert_int(water.tick).is_equal(1)
	water.step(1.0)
	assert_int(water.tick).is_equal(11)
	# held, it stays put however long the frames
	water.hold_at(7)
	water.step(5.0)
	assert_int(water.tick).is_equal(7)
	assert_bool(water.held).is_true()
	# let go, it runs on from the number it was held at
	water.release()
	assert_bool(water.held).is_false()
	water.step(0.31)
	assert_int(water.tick).is_equal(10)
