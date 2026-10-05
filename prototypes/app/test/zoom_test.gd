## P8's screen, second round: the zoom's scale at each stop, the camera's tilt by path, the pixels
## shown and drawn by variant, the chunks' triangles and skirts, where the descent ends, and
## Measure's results for the chat. A full-size world takes as long to make as P7's, so what the
## screen draws is checked by the cloud's pictures instead.
extends GdUnitTestSuite

const ZOOM := preload("res://zoom/zoom.gd")


# checks: PRE-03
func test_each_stop_has_its_scale_and_the_zoom_passes_them_in_order() -> void:
	for stop: Array in ZOOM.STOPS:
		var z: float = ZOOM.zoom_of(stop[0])
		assert_float(ZOOM.metres_across(z)).is_equal_approx(stop[1], stop[1] * 1e-4)
		assert_str(ZOOM.stop_at(z)).is_equal(stop[0])
	assert_float(ZOOM.zoom_of("person")).is_equal_approx(0.0, 1e-6)
	assert_float(ZOOM.zoom_of("globe")).is_equal_approx(1.0, 1e-6)
	var last := 0.0
	for k in 101:
		var width: float = ZOOM.metres_across(k / 100.0)
		assert_bool(width > last).is_true()
		last = width


# checks: PRE-03, WLD-02
func test_the_camera_looks_down_from_the_valley_or_flies_lower_to_the_world_map() -> void:
	var camp: float = ZOOM.STOPS[2][1]
	var valley: float = ZOOM.STOPS[3][1]
	var world_map: float = ZOOM.STOPS[5][1]
	for path in 2:
		assert_float(ZOOM.pitch_at(ZOOM.STOPS[0][1], path)).is_equal(30.0)
		assert_float(ZOOM.pitch_at(camp, path)).is_equal(30.0)
		assert_float(ZOOM.pitch_at(world_map, path)).is_equal_approx(90.0, 1e-3)
	assert_float(ZOOM.pitch_at(valley, 0)).is_equal_approx(90.0, 1e-3)
	assert_float(ZOOM.pitch_at(valley, 1)).is_between(40.0, 80.0)


# checks: PRE-03
func test_the_pixels_shown_and_drawn_by_each_variant() -> void:
	var person: float = ZOOM.STOPS[0][1]
	var globe: float = ZOOM.STOPS[6][1]
	# A: the art book's 4, drawn one for one
	assert_float(ZOOM.pixel_at(person, 0)).is_equal(4.0)
	assert_float(ZOOM.texel_at(4.0, 0)).is_equal(4.0)
	# B: whole steps from 2 to 6, crisp
	assert_float(ZOOM.pixel_at(person, 1)).is_equal(2.0)
	assert_float(ZOOM.pixel_at(globe, 1)).is_equal(6.0)
	var region: float = ZOOM.pixel_at(ZOOM.STOPS[4][1], 1)
	assert_float(region).is_equal(roundf(region))
	assert_float(ZOOM.texel_at(region, 1)).is_equal(region)
	# C: smoothly from 2 to 6, drawn two each way for each, never finer than two of the screen's
	assert_float(ZOOM.pixel_at(person, 2)).is_equal_approx(2.0, 1e-6)
	assert_float(ZOOM.pixel_at(globe, 2)).is_equal_approx(6.0, 1e-6)
	assert_float(ZOOM.texel_at(2.0, 2)).is_equal(2.0)
	assert_float(ZOOM.texel_at(6.0, 2)).is_equal_approx(3.0, 1e-6)
	assert_float(ZOOM.texel_at(5.0, 2)).is_equal_approx(2.5, 1e-6)


# checks: PRE-03
func test_a_chunk_s_triangles_cover_it_and_its_skirts_hang_from_each_edge() -> void:
	var n := 5
	var indices: PackedInt32Array = ZOOM.chunk_indices(n)
	assert_int(indices.size()).is_equal((n - 1) * (n - 1) * 6 + 4 * (n - 1) * 6)
	var most := 0
	for i in indices:
		most = maxi(most, i)
	assert_int(most).is_equal(n * n + 4 * n - 1)
	# the edges: south row, north row, west column, east column
	assert_int(ZOOM.edge_point(n, 0, 3)).is_equal(3)
	assert_int(ZOOM.edge_point(n, 1, 3)).is_equal((n - 1) * n + 3)
	assert_int(ZOOM.edge_point(n, 2, 3)).is_equal(3 * n)
	assert_int(ZOOM.edge_point(n, 3, 3)).is_equal(3 * n + n - 1)


# checks: PRE-03
func test_the_descent_ends_beside_the_nearest_river_large_enough() -> void:
	var cells := Vector2i(64, 32)
	var world := Vector2(64000.0, 32000.0)
	var water := PackedByteArray()
	water.resize(cells.x * cells.y * 4)
	water.fill(255)
	var start := Vector2(10500.0, 10500.0)
	assert_vector(ZOOM.river_near(start, water, cells, world)).is_equal(start)
	# a river draining 512 km² five cells east and one draining 8 km² next door, which is too small
	var big := (10 * cells.x + 15) * 4
	water[big] = 0
	water[big + 1] = 144
	var small := (10 * cells.x + 11) * 4
	water[small] = 0
	water[small + 1] = 48
	assert_vector(ZOOM.river_near(start, water, cells, world)).is_equal(Vector2(15500.0, 10500.0))


# checks: PLT-04, PRE-03
func test_measure_s_results_for_the_chat() -> void:
	var samples := {"globe": [4.0, 6.0], "person": [2.0, 2.0, 2.0, 10.0]}
	var out: PackedStringArray = ZOOM.results(samples, {"person": 1}, 0.42)
	assert_array(Array(out)).is_equal(
		["person 4.0/10.0 75%", "globe 5.0/6.0 100%", "full area 0.42 s"]
	)
	assert_str(ZOOM.results({}, {}, -1.0)[0]).is_equal("full area not made")


# checks: WLD-03, WLD-13, PRE-03
func test_the_generator_gives_the_zoom_the_world_s_size_and_the_clouds_noise() -> void:
	assert_bool(ClassDB.class_exists("WorldGen")).is_true()
	var gen: RefCounted = ClassDB.instantiate("WorldGen")
	assert_vector(gen.world_size()).is_equal(Vector2(2.0e6, 1.0e6))
	assert_vector(gen.world_cells()).is_equal(Vector2i(2048, 1024))
	# with no world made yet, a chunk comes back empty
	assert_int((gen.chunk(0, 0.0, 0.0, 33, 1.0) as Array).size()).is_equal(0)
	# the clouds' noise, four bytes a point, with its smaller copies down to one point
	var noise: PackedByteArray = gen.cloud_noise(4)
	assert_int(noise.size()).is_equal((64 + 8 + 1) * 4)
	assert_bool(noise == gen.cloud_noise(4)).is_true()
