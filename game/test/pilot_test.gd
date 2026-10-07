## The Pilot page, headless in the cloud (T2.3b.2, T2.3b.4, T2.3c.2): it makes the stand-in area
## from the catalogue's tuning and its surfaces, builds the land and the water, and each band of the
## zoom reads the tile of the ladder A5.3 gives it; the places it shows are on the ground they name;
## the camp's tent and club stand where the tuning says; and a piece's sheet shows above what the
## engine draws of it.
extends GdUnitTestSuite

const PilotPage := preload("res://pages/pilot.gd")


func _page() -> Control:
	var page: Control = auto_free(PilotPage.new())
	# the page fills the window, as the shell gives it
	page.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
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
	assert_str(page.shown[0]).contains("Camp at band 0")
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


# checks: PRE-23 PRE-46
func test_the_camp_s_tent_and_club_stand_on_the_meadow_where_the_tuning_says() -> void:
	var page := _page()
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	var tuning: Dictionary = page.world.entry("tuning/area", "base:area")
	var bank := int(page.area.banks_at(0).x * 100.0)
	var tent: Dictionary = page.camp_thing("Tent")
	var club: Dictionary = page.camp_thing("Club")
	assert_str(str(tent["model"])).is_equal(str(tuning["tent"]))
	assert_str(str(club["model"])).is_equal(str(tuning["club"]))
	# the tent so far north of the north bank (the tuning's lengths are millimetres)
	var back := float(tuning["camp_back"]) / 10.0
	assert_int(int(tent["north"]) - bank).is_equal(roundi(back))
	# the club the tuning's distance from the tent, on its bearing
	var away := float(tuning["club_away"]) / 10.0
	var apart := Vector2(club["east"] - tent["east"], club["north"] - tent["north"])
	assert_float(apart.length()).is_equal_approx(away, 1.5)
	var bearing := rad_to_deg(atan2(apart.x, apart.y))
	assert_float(fposmod(bearing, 360.0)).is_equal_approx(float(tuning["club_bearing"]), 1.0)
	# both stand on the meadow, at the height of the ground under them, and the camp is in view of
	# the river: the view's middle lies between the tent and the water
	for thing: Dictionary in [tent, club]:
		assert_int(int(thing["up"])).is_equal(
			roundi(page.area.ground_height(thing["east"], thing["north"]) * 100.0)
		)
		assert_float(page.area.ground_height(thing["east"], thing["north"])).is_equal_approx(
			0.0, 1e-9
		)
	var middle: Vector2i = page.place_at("Camp")
	assert_bool(middle.y > bank and middle.y < int(tent["north"])).is_true()
	assert_vector(page.place_at("Tent")).is_equal(Vector2i(tent["east"], tent["north"]))
	assert_vector(page.place_at("Club")).is_equal(Vector2i(club["east"], club["north"]))


# checks: PRE-21 PRE-24 PRE-30
func test_the_maps_round_the_camp_s_things_are_made_from_them_with_the_light_s_numbers() -> void:
	var page := _page()
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	var tent: Dictionary = page.camp_thing("Tent")
	var tuning: Dictionary = page.world.entry("tuning/light", "base:light")
	var maps: ViewMaps = page.maps
	assert_object(maps.view).is_not_null()
	assert_object(maps.tops).is_not_null()
	# the numbers are the light's: a texel of 6 cm, the farthest caster's distance
	assert_float(maps.width / float(maps.view.get_width())).is_equal_approx(
		float(tuning["maps_texel"]) / 1000.0, 1e-6
	)
	assert_float(maps.reach).is_equal_approx(float(tuning["shadow_reach"]) / 1000.0, 1e-6)
	# the square covers the tent, and something stands on a good many of its texels: the tent's
	# footprint is a circle about 4 m across, 12 m2, of 3,500 texels of 6 cm and more
	var east := float(tent["east"]) / 100.0
	var north := float(tent["north"]) / 100.0
	assert_bool(east > maps.west and east < maps.west + maps.width).is_true()
	assert_bool(north > maps.south and north < maps.south + maps.width).is_true()
	assert_int(maps.footprint).is_greater(2500)
	# the heights: the tent's apex stands about 2.6 m over the ground, and the tops are the same size
	var tops := maps.tops.get_image()
	assert_int(tops.get_width()).is_equal(maps.view.get_width())
	var apex := 0.0
	for row in tops.get_height():
		for column in tops.get_width():
			apex = maxf(apex, tops.get_pixel(column, row).r)
	assert_float(apex).is_between(2.0, 3.2)
	# the corner of the square is untouched, and somewhere on the ground there is contact darkening
	var view := maps.view.get_image()
	var corner := view.get_pixel(1, 1)
	assert_float(corner.r).is_equal_approx(1.0, 0.01)
	assert_float(corner.g).is_equal_approx(1.0, 0.01)
	assert_float(corner.b).is_equal_approx(0.0, 0.01)
	var darkest := 1.0
	for row in view.get_height():
		for column in view.get_width():
			darkest = minf(darkest, view.get_pixel(column, row).g)
	assert_float(darkest).is_less(0.7)


# checks: PRE-21 PRE-24
func test_the_maps_params_are_the_light_s_tuning_in_metres_and_shares() -> void:
	var tuning := {
		"maps_texel": 60,
		"open_reach": 2000,
		"open_strength": 800000,
		"contact_width": 180,
		"contact_strength": 550000,
		"shadow_reach": 14000,
	}
	var params := ViewMaps.params_of(tuning)
	assert_float(params["texel"]).is_equal_approx(0.06, 1e-9)
	assert_float(params["open_reach"]).is_equal_approx(2.0, 1e-9)
	assert_float(params["open_strength"]).is_equal_approx(0.8, 1e-9)
	assert_float(params["contact_width"]).is_equal_approx(0.18, 1e-9)
	assert_float(params["contact_strength"]).is_equal_approx(0.55, 1e-9)
	assert_float(params["shadow_reach"]).is_equal_approx(14.0, 1e-9)


# checks: PRE-42 PLT-04
func test_the_build_ships_each_pilot_piece_s_sheet_and_it_reads_as_a_picture() -> void:
	var files: Dictionary = GameData.sheets(GameData.build())
	for piece: String in ["meadow", "river", "river_bed", "club", "hide_tent_cone"]:
		assert_bool(files.has(piece)).is_true()
		var texture := GameData.sheet_texture(str(files[piece]))
		assert_object(texture).is_not_null()
		# a sheet is 1080 pixels across, one of the phone's pixels to each of its own
		assert_int(texture.get_width()).is_equal(1080)
		assert_int(texture.get_height()).is_greater(1000)
	assert_object(GameData.sheet_texture("res://data/sheets/none.kdsheet")).is_null()


# checks: PRE-42 PRE-22
func test_a_piece_s_sheet_shows_at_the_top_with_its_place_below_and_the_view_set_lower() -> void:
	var page := _page()
	await await_idle_frame()
	page.look_at_place("Camp")
	var tent: Dictionary = page.camp_thing("Tent")
	page.show_sheet("hide_tent_cone")
	await await_idle_frame()
	assert_str(page.sheet).is_equal("hide_tent_cone")
	assert_str(page.place).is_equal("Tent")
	assert_bool(page._sheet_panel.visible).is_true()
	assert_str(page.shown[page.shown.size() - 1]).contains("the sheet of Tent above")
	# the sheet covers the top of the screen, so the focus lies north of the tent and the tent
	# shows lower, in the middle of what the sheet leaves free
	var state: Dictionary = page.look.state()
	assert_int(int(state["focus_north"])).is_greater(int(tent["north"]))
	# the sheet is read at its own pixels
	assert_float(page._sheet_picture.custom_minimum_size.x * page._stretch()).is_equal_approx(
		1080.0, 1.5
	)
	# a place button puts the sheet away, and the view is on the place itself
	page.look_at_place("Club")
	await await_idle_frame()
	assert_str(page.sheet).is_empty()
	assert_bool(page._sheet_panel.visible).is_false()
	state = page.look.state()
	assert_int(int(state["focus_north"])).is_equal(int(page.camp_thing("Club")["north"]))
	# a piece with no sheet shipped is left alone
	page.show_sheet("no_such_piece")
	assert_str(page.sheet).is_empty()


# checks: PRE-22
func test_a_texture_pixel_can_be_enlarged_to_8_screen_pixels_and_the_band_brings_it_back() -> void:
	var page := _page()
	await await_idle_frame()
	page.set_enlarged(true)
	await await_idle_frame()
	await await_idle_frame()
	var state: Dictionary = page.look.state()
	assert_float(state["texel_pixels"]).is_equal_approx(8.0, 0.1)
	assert_str(page.shown[page.shown.size() - 1]).contains("enlarged")
	page.set_band(2)
	await await_idle_frame()
	await await_idle_frame()
	state = page.look.state()
	assert_bool(page.enlarged).is_false()
	assert_float(state["texel_pixels"]).is_equal_approx(2.0, 0.05)


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
