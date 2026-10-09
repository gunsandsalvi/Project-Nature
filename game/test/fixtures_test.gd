## checks: PRE-02 PRE-03 PRE-22 PRE-27 PRE-31 PRE-33 PLT-02 WLD-13 TIM-17 RES-05.
extends GdUnitTestSuite

const Scene := preload("res://test/support/render_scene.gd")
const Drawing := preload("res://test/support/fixtures/drawing.gd")
const Atlas := preload("res://test/support/fixtures/atlas.gd")


func _page() -> Control:
	var page := Scene.new()
	page.drawing_script = Drawing
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
			FileAccess.get_file_as_string("res://test/support/fixtures/manifest.json")
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
