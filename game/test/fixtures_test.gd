## checks: PRE-02 PRE-03 PRE-22 PRE-27 PRE-31 PRE-33 PLT-02 WLD-13 TIM-17 RES-05.
extends GdUnitTestSuite

const Fixtures := preload("res://pages/fixtures.gd")
const Atlas := preload("res://fixtures/atlas.gd")
const TEST_ROOT := "user://test-worlds/fixtures"


func after_test() -> void:
	Worlds.remove_tree(TEST_ROOT)


func _page() -> Control:
	var page := Fixtures.new()
	page.frozen = true
	add_child(page)
	page.set_process(false)
	page.layout(Vector2(1080, 2400), Rect2(0, 0, 1080, 2400))
	page._process(0.0)
	return page


# checks: PRE-27 PRE-42 PRE-43 PRE-44 PRE-46 (T2.7a.2/4).
func test_atlases_reject_bad_pivots_units_grids_and_paths() -> void:
	var atlas := Atlas.new()
	assert_bool(atlas.read()).is_true()
	assert_int(atlas.entries.size()).is_equal(6)
	for mutation: String in [
		"pivot",
		"unit",
		"grid",
		"path",
		"types",
		"path_type",
		"mask_type",
		"normal_basis",
		"material_format",
		"pieces"
	]:
		var entries: Array = JSON.parse_string(
			FileAccess.get_file_as_string("res://fixtures/manifest.json")
		)
		match mutation:
			"pivot":
				entries[0].pivot = [-1, 511]
			"unit":
				entries[0].height_unit = "cm"
			"grid":
				entries[0].frame_size = [16, 16]
			"path":
				entries[0].actions.walk["64"] = "../icon.svg"
			"types":
				entries[0].frames = "six"
			"path_type":
				entries[0].actions.walk["64"] = ["tree-64.png"]
			"normal_basis":
				entries[0].normal_levels = entries[0].actions.walk.duplicate()
				entries[0].normal_basis = "screen"
			"material_format":
				entries[0].material_levels = entries[0].actions.walk.duplicate()
				entries[0].material_format = "rgb-colour"
			"pieces":
				entries[0].pieces = {"crown": [0, 0, 9999, 9999]}
			"mask_type":
				entries[0].material_levels = {"64": ["tree-mask-64.png"]}
		var path := "user://fixture-invalid.json"
		var file := FileAccess.open(path, FileAccess.WRITE)
		file.store_string(JSON.stringify(entries))
		file.close()
		assert_bool(atlas.read(path)).is_false()
		assert_str(atlas.problem).is_not_empty()
		assert_int(atlas.entries.size()).is_equal(0)
		DirAccess.remove_absolute(ProjectSettings.globalize_path(path))


# checks: PRE-03 PRE-22 PRE-33 PLT-02 TIM-17 (T2.7a.2).
func test_orientation_keeps_selection_time_focus_and_native_safe_controls() -> void:
	var page := _page()
	var at: Vector2 = page.camera.ground(Vector2(540, 1200), 0.0)
	page.drawing.selected = -5
	var second: float = page.world.screen_time()
	assert_int(int(page.state.scale)).is_equal(2)
	assert_str(page._status.text).not_contains("%s")
	page.layout(Vector2(2400, 1080), Rect2(60, 30, 2280, 1010))
	page._process(0.0)
	assert_vector(page.camera.ground(Vector2(1200, 540), 0.0)).is_equal(at)
	assert_int(page.drawing.selected).is_equal(-5)
	assert_float(page.world.screen_time()).is_equal(second)
	assert_vector(page._native.get_node("Controls").position).is_equal(Vector2(84, 54))
	assert_int(int(page.state.scale)).is_equal(2)
	assert_int(int(page._controls.get_child(0).get_theme_font_size("font_size"))).is_equal(32)
	page.free()


# checks: PRE-02 PRE-03 PRE-22 PRE-33 PRE-42 PRE-43 PRE-44 (T2.7a.2).
func test_alpha_picks_undo_presentation_and_animation_pivots_hold_feet() -> void:
	var page := _page()
	var drawing: Node2D = page.drawing
	var foot: Vector2 = page.camera.project(0.0, 0.0, 0.0)
	for frame in 6:
		drawing.second = frame / 6.0
		drawing.rebuild()
		var person: Dictionary = {}
		for item: Dictionary in drawing.draw_list():
			if item.id == -5:
				person = item
		assert_vector(person.foot).is_equal(foot)
		assert_vector(person.rect.position).is_equal(foot - Vector2(16, 56))
	var inside := foot + Vector2(0, -36)
	assert_int(drawing.pick(inside).id).is_equal(-5)
	assert_bool(drawing.pick(foot + Vector2(-16, -54)).is_empty()).is_true()
	page.camera.focus(1.013, 0.047)
	page.camera.zoom(1.3, Vector2(300, 900), false)
	page._process(0.0)
	var raster: Vector2 = page.camera.project(0.0, 0.0, 0.0) + Vector2(0, -36)
	var physical: Vector2 = (
		raster * float(page.state.scale) * float(page.state.live_scale) + Vector2(page.state.offset)
	)
	assert_int(drawing.pick(page.camera.from_screen(physical)).id).is_equal(-5)
	page.camera.zoom(1.0, Vector2(300, 900), true)
	page._process(0.0)
	assert_float(page.state.live_scale).is_equal(1.0)
	page.free()


# checks: WLD-13 TIM-17 RES-05 PLT-07 (T2.7a.1/2).
func test_view_controls_and_legacy_saved_worlds_keep_identical_outcomes() -> void:
	var before := KdWorld.new()
	GameData.load_into(before)
	var folder := ProjectSettings.globalize_path(TEST_ROOT)
	assert_bool(before.open_crowd(folder, 7, 4, "α1.4a").has("problem")).is_false()
	before.call_home_at(0, 32000)
	before.begin_at(40000)
	before.pause()
	before.save_now()
	var digest: String = before.digest()
	var camera := KdCanvas.new()
	var origin: PackedInt64Array = before.camp_at(0)
	camera.set_world(before, origin[0], origin[1])
	camera.frame(1080, 2400)
	camera.zoom(1.9, Vector2(300, 600), false)
	camera.frame(2400, 1080)
	camera.zoom(1.0, Vector2(300, 600), true)
	var records: Array = camera.records()
	assert_int(records.size()).is_equal(100)
	assert_str(before.digest()).is_equal(digest)
	before.save_now()
	camera.set_world(null, 0, 0)
	before = null
	var reopened := KdWorld.new()
	GameData.load_into(reopened)
	assert_bool(reopened.open_crowd(folder, 7, 4, "α2.3a").has("problem")).is_false()
	assert_str(reopened.digest()).is_equal(digest)
	var reference := KdWorld.new()
	GameData.load_into(reference)
	reference.start_crowd(7, 4)
	reference.call_home_at(0, 32000)
	reference.run_until(40000)
	assert_str(reference.digest()).is_equal(digest)

	# The fixture page keeps background saves and manual pause on resume.
	var page := Fixtures.new()
	page.save_folder = TEST_ROOT
	add_child(page)
	page.set_process(false)
	assert_str(page.world.digest()).is_equal(digest)
	page.world.set_speed(60.0)
	page.world.pause()
	page._notification(NOTIFICATION_APPLICATION_PAUSED)
	page._notification(NOTIFICATION_APPLICATION_RESUMED)
	assert_bool(page.world.is_paused()).is_true()
	assert_float(page.world.speed()).is_equal(60.0)
	page.world.play()
	page._notification(NOTIFICATION_APPLICATION_PAUSED)
	assert_bool(page.world.is_paused()).is_true()
	page._notification(NOTIFICATION_APPLICATION_RESUMED)
	assert_bool(page.world.is_paused()).is_false()
	page.free()
