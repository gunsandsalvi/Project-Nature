## Camp alpha: real sprite identities, pause, view independence, and saved/exported continuation.
extends GdUnitTestSuite

const Camp := preload("res://pages/camp.gd")
const TEST_ROOT := "user://test-worlds/camp-alpha"


func after_test() -> void:
	Worlds.remove_tree(TEST_ROOT)


func _page() -> Control:
	var page := Camp.new()
	page.root = TEST_ROOT
	page.frozen = true
	add_child(page)
	page.set_process(false)
	page.layout(Vector2(1080, 2400), Rect2(0, 0, 1080, 2400))
	page._process(0)
	return page


func test_drawn_people_select_their_saved_identity_and_position() -> void:
	var page := _page()
	assert_int(page.people.size()).is_equal(25)
	var ids := {}
	for person: Dictionary in page.people:
		assert_bool(ids.has(person.id)).is_false()
		ids[person.id] = true
		var rect: Rect2 = page.drawing.drawn[int(person.id)]
		var at: Vector2 = rect.get_center() * float(page.state.scale) + Vector2(page.state.offset)
		page.tap(at)
		assert_int(page.selected_id).is_equal(int(person.id))
		assert_str(page.selected_person().name).is_equal(str(person.name))
		assert_int(page.selected_person().east_cm).is_equal(int(person.east_cm))
		assert_str(page._card.text).contains(str(person.name))
	page.free()


func test_pause_and_camera_motion_hold_the_authoritative_state() -> void:
	var page := _page()
	page.world.save_now()
	var digest: String = page.world.digest()
	var second: int = page.world.frontier()
	page.select_person(int(page.people[2].id))
	page.camera.focus(3, 4)
	page.camera.zoom(1.3, Vector2(400, 800), false)
	page.layout(Vector2(2400, 1080), Rect2(0, 0, 2400, 1080))
	for i in 4:
		page._process(0.1)
		await await_idle_frame()
	assert_int(page.world.frontier()).is_equal(second)
	assert_str(page.world.digest()).is_equal(digest)
	assert_int(page.selected_id).is_equal(int(page.people[2].id))
	page.free()


func test_reopen_export_import_preserve_people_supplies_and_digest() -> void:
	var page := _page()
	page.world.run_until(43200)
	page.save_camp()
	var people: Array = page.world.people()
	var supplies: Dictionary = page.world.camp_alpha()
	var digest: String = page.world.digest()
	var id: String = page.world_id
	page.free()
	var again := _page()
	assert_bool(again.opened.made).is_false()
	assert_array(again.world.people()).is_equal(people)
	assert_dict(again.world.camp_alpha()).is_equal(supplies)
	assert_str(again.world.digest()).is_equal(digest)
	again.world.save_now()
	var worlds: KdWorlds = again.worlds
	assert_int(worlds.export_begin(id)).is_greater(0)
	var archive := PackedByteArray()
	while true:
		var piece := worlds.export_next(4096)
		if piece.is_empty():
			break
		archive.append_array(piece)
	assert_bool(worlds.import_begin()).is_true()
	assert_bool(worlds.import_feed(archive)).is_true()
	var imported: Dictionary = worlds.import_finish()
	assert_bool(imported.has("problem")).is_false()
	assert_str(str(imported.id)).is_not_equal(id)
	again.free()
	worlds.set_current(imported.id)
	var copy := _page()
	assert_array(copy.world.people()).is_equal(people)
	assert_dict(copy.world.camp_alpha()).is_equal(supplies)
	assert_str(copy.world.digest()).is_equal(digest)
	copy.world.run_until(90000)
	var expected := KdWorld.new()
	GameData.load_into(expected)
	expected.open_camp(ProjectSettings.globalize_path(TEST_ROOT.path_join(id)), 17, "camp-test")
	expected.begin_at(90000)
	assert_str(copy.world.digest()).is_equal(expected.digest())
	expected.save_now()
	expected = null
	copy.free()


func _block_snapshots(page: Control) -> void:
	var folder: String = TEST_ROOT.path_join(page.world_id)
	var snapshots := folder.path_join("snapshots")
	assert_int(DirAccess.rename_absolute(snapshots, folder.path_join("kept-snapshots"))).is_equal(
		OK
	)
	var obstruction := FileAccess.open(snapshots, FileAccess.WRITE)
	assert_object(obstruction).is_not_null()
	obstruction.store_string("Injected write failure: this file blocks the snapshot directory")
	obstruction.close()


func test_failed_manual_save_reports_a_persistent_error() -> void:
	var page := _page()
	_block_snapshots(page)
	page.save_camp()
	assert_bool(page.world.counters().save_failed).is_true()
	assert_str(page._summary.text).contains("could not save")
	page._message_until = 0
	page._process(0)
	assert_str(page._summary.text).contains("could not save")
	assert_str(page._summary.text).not_contains("Camp saved")
	page.free()


func test_failed_background_save_explains_why_the_world_paused() -> void:
	var page := _page()
	page.world.play()
	_block_snapshots(page)
	page.world.save()
	for i in 100:
		await await_idle_frame()
		page._process(0)
		if page.world.counters().save_failed:
			break
	assert_bool(page.world.counters().save_failed).is_true()
	assert_bool(page.world.is_paused()).is_true()
	assert_str(page._summary.text).contains("could not save")
	page.free()


func test_people_and_supply_projection_stay_absolute_after_camera_rebase() -> void:
	var page := _page()
	var digest: String = page.world.digest()
	page.camera.zoom(1.0 / 4096, Vector2(500, 800), true)
	page.camera.focus(20000, 20000)
	page._process(0)
	assert_int(int(page.state.rebase.east_cm)).is_not_equal(0)
	var at := Vector2i(page.people[0].east_cm, page.people[0].north_cm)
	var expected: Vector2 = page.camera.project_world(at.x, at.y, 0)
	assert_vector(page.drawing._absolute(at)).is_equal(expected)
	var water: PackedInt64Array = page.world.camp_alpha().water_at
	assert_vector(page.drawing._site("water_at")).is_equal(
		page.camera.project_world(water[0], water[1], 0)
	)
	assert_str(page.world.digest()).is_equal(digest)
	page.free()


func test_patch_corners_use_the_saved_centre_after_rebase() -> void:
	var page := _page()
	page.camera.focus(20000, 20000)
	page._process(0)
	var supplies: Dictionary = page.world.camp_alpha()
	var origin: PackedInt64Array = page.world.camp_at(0)
	var expected := PackedVector2Array()
	for sign: Vector2i in [Vector2i(-1, -1), Vector2i(1, -1), Vector2i(1, 1), Vector2i(-1, 1)]:
		expected.append(
			page.camera.project_world(
				origin[0] + sign.x * supplies.half_width_cm,
				origin[1] + sign.y * supplies.half_height_cm,
				0
			)
		)
	assert_array(Array(page.drawing.patch_points())).is_equal(Array(expected))
	page.free()
