## The look's light for the moment it shows (A4.3, PRE-30), headless in the cloud: the sun and the
## sky come from base/tuning/light.toml, the sun stands where the tuning says in the world's axes,
## and the light function's globals are published with the tuning's colours.
extends GdUnitTestSuite


func _world() -> KdWorld:
	var world := KdWorld.new()
	GameData.load_into(world)
	return world


# checks: PRE-30
func test_the_sun_stands_where_the_tuning_says_in_the_worlds_axes() -> void:
	# x east, y up, z south: a sun on the horizon in the east, the south, and straight overhead
	var east := Afternoon.toward_the_sun(0, 90)
	assert_float(east.distance_to(Vector3(1.0, 0.0, 0.0))).is_less(1e-6)
	var south := Afternoon.toward_the_sun(0, 180)
	assert_float(south.distance_to(Vector3(0.0, 0.0, 1.0))).is_less(1e-6)
	var overhead := Afternoon.toward_the_sun(90, 0)
	assert_float(overhead.distance_to(Vector3(0.0, 1.0, 0.0))).is_less(1e-6)
	# and half way up in the north-west is a unit vector leaning west and north
	var between := Afternoon.toward_the_sun(45, 315)
	assert_float(between.length()).is_equal_approx(1.0, 1e-6)
	assert_float(between.x).is_less(0.0)
	assert_float(between.z).is_less(0.0)
	assert_float(between.y).is_equal_approx(sin(deg_to_rad(45.0)), 1e-6)


# checks: PRE-30
func test_the_light_is_put_under_a_node_from_the_tuning_and_its_globals_are_published() -> void:
	var world := _world()
	var under: Node3D = auto_free(Node3D.new())
	add_child(under)
	assert_str(Afternoon.apply(world, under)).is_empty()
	var tuning := world.entry("tuning/light", "base:light")
	assert_bool(tuning.is_empty()).is_false()
	var sun := under.get_node("Sun") as DirectionalLight3D
	var toward := Afternoon.toward_the_sun(int(tuning["sun_height"]), int(tuning["sun_turn"]))
	# the sun's light travels from it toward the ground: opposite to the way to the sun
	var travels := -sun.global_transform.basis.z
	assert_float(travels.distance_to(-toward)).is_less(1e-4)
	assert_bool(sun.shadow_enabled).is_true()
	assert_float(sun.light_energy).is_equal_approx(float(tuning["sun_energy"]) / 1.0e6, 1e-6)
	assert_str(sun.light_color.to_html(false)).is_equal(str(tuning["sun_colour"]).trim_prefix("#"))
	var sky := (under.get_node("Sky") as WorldEnvironment).environment
	assert_str(sky.background_color.to_html(false)).is_equal(
		str(tuning["sky_colour"]).trim_prefix("#")
	)
	assert_float(sky.ambient_light_energy).is_equal_approx(
		float(tuning["ambient_energy"]) / 1.0e6, 1e-6
	)

	# every global it publishes is one the project declares, so the light function finds it (the
	# headless renderer keeps no values to read back)
	for name: String in [
		"kd_view_maps",
		"kd_view_tops",
		"kd_maps_place",
		"kd_rim",
		"kd_fire_grid",
		"kd_fire_table",
		"kd_grid_place",
		"kd_sun_toward",
		"kd_bounce",
		"kd_haze",
		"kd_haze_sun",
		"kd_sky"
	]:
		assert_bool(ProjectSettings.has_setting("shader_globals/" + name)).is_true()


# checks: PRE-30
func test_a_catalogue_with_no_light_says_so_and_puts_none_down() -> void:
	var under: Node3D = auto_free(Node3D.new())
	add_child(under)
	var said := Afternoon.apply(KdWorld.new(), under)
	assert_str(said).contains("tuning/light")
	assert_int(under.get_child_count()).is_equal(0)
