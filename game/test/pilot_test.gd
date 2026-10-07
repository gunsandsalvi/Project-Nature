## The Pilot page, headless in the cloud (T2.3b.2, T2.3b.4): it makes the stand-in area from the
## catalogue's tuning and its surfaces, builds the land and the water, and each band of the zoom
## reads the tile of the ladder A5.3 gives it; the places it shows are on the ground they name.
extends GdUnitTestSuite

const PilotPage := preload("res://pages/pilot.gd")


func _page() -> Control:
	var page: Control = auto_free(PilotPage.new())
	add_child(page)
	return page


# checks: PRE-23 PRE-26 PRE-22
func test_the_pilot_page_makes_the_area_builds_the_land_and_the_water_and_says_what_it_shows(
) -> void:
	var page := _page()
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	var info: Dictionary = page.area.info()
	assert_bool(info["built"]).is_true()
	assert_float(info["level"]).is_equal_approx(-0.4, 1e-9)
	# the carpet, the strip the river runs in, and the water over it: three meshes of triangles
	var triangles: Array = info["triangles"]
	assert_int(triangles.size()).is_equal(3)
	assert_int(triangles[0]).is_equal(4)
	assert_int(triangles[1]).is_greater(10000)
	assert_int(triangles[2]).is_greater(1000)
	assert_str(page.shown[0]).contains("Meadow at band 0")
	assert_str(page.shown[0]).contains("near tile")


# checks: PRE-22
func test_each_band_of_the_zoom_reads_the_tile_that_serves_it() -> void:
	var page := _page()
	await await_idle_frame()
	var tiles := []
	for band in 7:
		tiles.append(page.tile_at("ground", band))
	assert_array(tiles).is_equal(["near", "near", "middle", "middle", "far", "far", "far"])
	# a band is clamped to the seven the ladder serves, and the zoom follows it: a texture pixel is
	# 2 screen pixels wide at the focus at every band
	page.set_band(9)
	assert_int(page.band).is_equal(6)
	page.set_band(-3)
	assert_int(page.band).is_equal(0)
	page.set_band(3)
	await await_idle_frame()
	var state: Dictionary = page.look.state()
	assert_int(state["band"]).is_equal(3)
	assert_float(state["texel_pixels"]).is_equal_approx(2.0, 0.05)
	assert_str(page.shown[page.shown.size() - 1]).contains("middle tile")


# checks: PRE-26
func test_the_places_are_on_the_ground_they_name_the_shore_at_the_water_s_edge() -> void:
	var page := _page()
	await await_idle_frame()
	var level := float(page.area.info()["level"])
	var river: Vector2i = page.place_at("River")
	assert_float(page.area.ground_height(river.x, river.y)).is_less(level - 0.1)
	var meadow: Vector2i = page.place_at("Meadow")
	assert_float(page.area.ground_height(meadow.x, meadow.y)).is_equal_approx(0.0, 1e-9)
	# the shore is where the bank meets the level, to a centimetre of height
	var shore: Vector2i = page.place_at("Shore")
	assert_float(page.area.ground_height(shore.x, shore.y)).is_equal_approx(level, 0.01)
	# the banks wander along the river: not the same at every place
	var here: Vector2 = page.area.banks_at(0)
	var there: Vector2 = page.area.banks_at(30000)
	assert_bool(here.x != there.x or here.y != there.y).is_true()
	page.look_at_place("Shore")
	assert_str(page.place).is_equal("Shore")


# checks: PRE-26 PRE-22
func test_the_water_s_tick_can_be_held_for_a_picture() -> void:
	var page := _page()
	await await_idle_frame()
	page.hold_water(4)
	await await_idle_frame()
	await await_idle_frame()
	assert_int(page.water.tick).is_equal(4)
	page.release_water()
	assert_bool(page.water.held).is_false()


# checks: PRE-23
func test_the_app_has_a_pilot_page() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	main.open_page("Pilot")
	assert_str(main.page_name()).is_equal("Pilot")
