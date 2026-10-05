## P8's screen: the zoom's scale at each stop, the camera's tilt, the bend onto the globe, how far
## each ring draws, Measure's counting by stop and its results for the chat. A full-size world takes
## as long to make as P7's, so what the screen draws is checked by the cloud's pictures instead.
extends GdUnitTestSuite

const ZOOM := preload("res://zoom/zoom.gd")


# checks: PRE-03
func test_each_stop_has_its_scale_and_the_zoom_passes_them_in_order() -> void:
	for stop: Array in ZOOM.STOPS.slice(0, 6):
		var z: float = ZOOM.zoom_of(stop[0])
		assert_float(ZOOM.metres_a_pixel(z)).is_equal_approx(stop[1], stop[1] * 1e-4)
		assert_str(ZOOM.stop_at(z)).is_equal(stop[0])
	assert_float(ZOOM.zoom_of("globe")).is_equal(1.0)
	assert_str(ZOOM.stop_at(1.0)).is_equal("globe")
	# the scale only grows from the person outward, and holds while the map bends onto the globe
	var last := 0.0
	for k in 101:
		var mpp: float = ZOOM.metres_a_pixel(k / 100.0)
		assert_bool(mpp >= last).is_true()
		last = mpp
	assert_float(ZOOM.metres_a_pixel(1.0)).is_equal(ZOOM.metres_a_pixel(ZOOM.FLAT))


# checks: PRE-03, WLD-02
func test_the_camera_tilts_up_to_the_valley_and_the_map_bends_past_the_world_map() -> void:
	assert_float(ZOOM.pitch_at(0.03)).is_equal(ZOOM.PITCH)
	assert_float(ZOOM.pitch_at(1.1)).is_equal_approx(ZOOM.PITCH, 1e-3)
	assert_float(ZOOM.pitch_at(37.0)).is_equal_approx(90.0, 1e-3)
	assert_float(ZOOM.pitch_at(5500.0)).is_equal(90.0)
	assert_float(ZOOM.bend_at(ZOOM.FLAT)).is_equal(0.0)
	assert_float(ZOOM.bend_at(1.0)).is_equal(1.0)


# checks: PRE-03
func test_each_ring_shrinks_to_nothing_as_the_zoom_leaves_it() -> void:
	for level: Dictionary in ZOOM.LEVELS + [ZOOM.CARDS]:
		var most: float = level.most
		assert_float(ZOOM.ring_at(level, most / 2.0)).is_equal_approx(1.0, 1e-6)
		assert_float(ZOOM.ring_at(level, most * 0.75)).is_between(0.01, 0.99)
		assert_float(ZOOM.ring_at(level, most)).is_equal_approx(0.0, 1e-6)
	# finest first, each reaching farther than the one inside it and drawn as far out
	for k in range(1, ZOOM.LEVELS.size()):
		assert_bool(ZOOM.LEVELS[k].reach > ZOOM.LEVELS[k - 1].reach).is_true()
		assert_bool(ZOOM.LEVELS[k].most >= ZOOM.LEVELS[k - 1].most).is_true()
	# the near rings, the full areas, reach about 300 m (PRE-03), the first a full area across
	assert_float(ZOOM.LEVELS[1].reach).is_equal(300.0)
	assert_float(ZOOM.LEVELS[0].reach).is_equal(ZOOM.AREA_HALF)


# checks: PLT-04, PRE-03
func test_measure_s_results_for_the_chat() -> void:
	var samples := {"globe": [4.0, 6.0], "person": [2.0, 2.0, 2.0, 10.0]}
	var out: PackedStringArray = ZOOM.results(samples, {"person": 1}, 0.42)
	assert_array(Array(out)).is_equal(
		["person 4.0/10.0 75%", "globe 5.0/6.0 100%", "full area 0.42 s"]
	)
	assert_str(ZOOM.results({}, {}, -1.0)[0]).is_equal("full area not made")


# checks: WLD-03, PRE-03
func test_the_generator_gives_the_zoom_the_world_s_size() -> void:
	assert_bool(ClassDB.class_exists("WorldGen")).is_true()
	var gen: RefCounted = ClassDB.instantiate("WorldGen")
	assert_vector(gen.world_size()).is_equal(Vector2(2.0e6, 1.0e6))
	assert_vector(gen.world_cells()).is_equal(Vector2i(2048, 1024))
	# with no world made yet, the ground and its trees come back empty
	assert_int((gen.ground_mesh(0, 0.0, 0.0, 65, 1.0) as Array).size()).is_equal(0)
	assert_int((gen.trees(0, 0.0, 0.0, 64.0, 5.0) as PackedVector3Array).size()).is_equal(0)
