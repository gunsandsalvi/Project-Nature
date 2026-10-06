## The Look page, headless in the cloud (A17): it builds and loads its textures with their own
## levels; every switch changes only the drawing, never a world; and the camera keeps its metres a
## screen pixel, its focus and its heading through a turn of the phone.
extends GdUnitTestSuite

const LookPage := preload("res://pages/look.gd")


func _page() -> VBoxContainer:
	var page: VBoxContainer = auto_free(LookPage.new())
	add_child(page)
	return page


# checks: PRE-01 PRE-02 PRE-22
func test_the_look_page_builds_with_textures_of_our_own_levels() -> void:
	var page := _page()
	await await_idle_frame()
	assert_str(page.problem).is_empty()
	var state: Dictionary = page.look.state()
	assert_int(state["band"]).is_equal(0)
	assert_float(state["texel_pixels"]).is_equal_approx(2.0, 1e-9)
	# a file that is not a texture is refused with words
	var broken := KdLook.new()
	var problem := broken.load_layers(PackedStringArray(["res://data/build.toml"]))
	assert_str(problem).contains("not a texture file")


# checks: WLD-13 PRE-01
func test_every_switch_changes_only_the_drawing() -> void:
	var world := KdWorld.new()
	GameData.load_into(world)
	world.start_crowd(7, 4)
	world.run_until(9 * 3600)
	var before: String = world.digest()
	var page := _page()
	await await_idle_frame()
	for samples: int in [0, 4, 2]:
		page.set_msaa(samples)
		await await_idle_frame()
	for value: float in [0.5, 0.75, 1.0]:
		page.set_scale_3d(value)
		await await_idle_frame()
	for part: String in ["ground", "pattern", "shadows"]:
		page.set_part(part, false)
		await await_idle_frame()
		page.set_part(part, true)
	page.set_lens(5.0)
	await await_idle_frame()
	assert_int(page.msaa).is_equal(2)
	assert_float(get_viewport().scaling_3d_scale).is_equal(1.0)
	assert_str(world.digest()).is_equal(before)


# checks: PLT-02 PRE-33
func test_the_rig_keeps_its_zoom_focus_and_heading_through_a_turn_of_the_phone() -> void:
	var look := KdLook.new()
	look.set_screen(Vector2(1080, 2404))
	look.zoom_by(0.5, Vector2(540, 1202))
	look.turn_by(30.0, Vector2(300, 900))
	for i in 60:
		look.frame(1.0 / 60.0)
	var before: Dictionary = look.state()
	look.set_screen(Vector2(2404, 1080))
	look.frame(1.0 / 60.0)
	var after: Dictionary = look.state()
	for key: String in [
		"focus_east", "focus_north", "heading", "metres_per_pixel", "band", "texel_pixels"
	]:
		assert_that(after[key]).is_equal(before[key])
	assert_bool(look.pose()["keep_width"]).is_false()
