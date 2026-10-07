## The Lab page, headless in the cloud (A17, α2.3a): it makes every texture the build lists from
## its own levels, lists every material once, and shows a material's tiles in order, each level at
## true size, a texture pixel 2 × 2 screen pixels; and the app has the page.
extends GdUnitTestSuite

const LabPage := preload("res://pages/lab.gd")


func _art_textures() -> Array:
	var listed: Array = GameData.build().get_value("textures", "files", [])
	return listed.filter(func(item: String) -> bool: return item.begins_with("art/"))


# checks: PRE-20 PRE-22
func test_every_texture_the_build_lists_is_made_and_every_material_listed_once() -> void:
	var page: VBoxContainer = auto_free(LabPage.new())
	add_child(page)
	await await_idle_frame()
	assert_array(page.loaded["problems"] as Array).is_empty()
	assert_int(page.textures.size()).is_equal(_art_textures().size())
	for name: String in page.textures:
		assert_int((page.textures[name]["levels"] as Array).size()).is_greater(0)
	assert_str(page.shown[0]).contains(
		"%d materials, %d textures" % [page.materials.size(), page.textures.size()]
	)
	assert_bool(page.materials.has("meadow")).is_true()
	assert_float(page.load_ms).is_greater(0.0)
	assert_int(page.load_bytes).is_greater(0)


# checks: PRE-22
func test_a_material_shows_its_tiles_near_middle_far_and_its_levels_at_true_size() -> void:
	var page: VBoxContainer = auto_free(LabPage.new())
	add_child(page)
	await await_idle_frame()
	page.open("meadow")
	assert_str(page.showing).is_equal("meadow")
	var headings := Array(page.shown).filter(
		func(line: String) -> bool: return line.contains(": bands ")
	)
	assert_str(headings[0]).starts_with("near tile: bands 0 and 1")
	var middle := headings.find(
		headings.filter(func(h: String) -> bool: return h.begins_with("middle:"))[0]
	)
	var far := headings.find(
		headings.filter(func(h: String) -> bool: return h.begins_with("far:"))[0]
	)
	assert_int(middle).is_less(far)
	# the near tile's first level: 256 texture pixels, each 2 screen pixels
	var pictures := page.find_children("*", "TextureRect", true, false)
	var first: TextureRect = pictures[0]
	assert_float(first.custom_minimum_size.x * page.screen_pixels()).is_equal_approx(512.0, 0.01)
	page.list()
	assert_str(page.showing).is_empty()


# checks: PRE-22
func test_the_bands_each_tile_serves() -> void:
	var page: VBoxContainer = auto_free(LabPage.new())
	assert_array(page.bands_of("meadow", "art:meadow", true)).is_equal([0, 1])
	assert_array(page.bands_of("meadow", "art:meadow/v3", true)).is_equal([0, 1])
	assert_array(page.bands_of("meadow", "art:meadow/middle/v2", true)).is_equal([2, 3])
	assert_array(page.bands_of("meadow", "art:meadow/far", true)).is_equal([4, 5, 6])
	assert_array(page.bands_of("ash", "art:ash", false)).is_equal([0, 1, 2, 3, 4, 5, 6])


# checks: PRE-22
func test_offered_materials_come_first_then_those_held_back_with_why_then_those_decided() -> void:
	var page: VBoxContainer = auto_free(LabPage.new())
	add_child(page)
	await await_idle_frame()
	page.textures = {
		"art:ash": {"record": {"about": "ash", "approved": "yes, 7 October 2026"}, "levels": []},
		"art:bark":
		{"record": {"about": "bark", "approved": "not offered yet: too flat"}, "levels": []},
		"art:moss": {"record": {"about": "moss", "approved": "waiting"}, "levels": []},
		"art:moss/far":
		{"record": {"about": "moss", "approved": "not offered yet: too pale"}, "levels": []},
	}
	page.materials = {
		"ash": ["art:ash"], "bark": ["art:bark"], "moss": ["art:moss", "art:moss/far"]
	}
	assert_str(page.state_of("ash")).is_equal("decided")
	assert_str(page.state_of("bark")).is_equal("held")
	assert_str(page.state_of("moss")).is_equal("offered")
	page.list()
	var lines := Array(page.shown)
	var order := [
		lines.find("moss: moss (far not offered yet)"),
		lines.find("bark: bark"),
		lines.find("ash: ash")
	]
	assert_int(order[0]).is_greater(0)
	assert_bool(order[0] < order[1] and order[1] < order[2]).is_true()
	page.open("bark")
	assert_array(Array(page.shown)).contains(["Not offered yet: too flat"])


# checks: PRE-20
func test_the_app_has_a_lab_page() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	main.open_page("Lab")
	assert_str(main.page_name()).is_equal("Lab")
