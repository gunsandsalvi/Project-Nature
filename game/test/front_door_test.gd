## Camp-first shell: real touch selection, saved camp switching and file export/import.
extends GdUnitTestSuite

const Main := preload("res://main.gd")
const ROOT := "user://test-worlds/front-door"


func after_test() -> void:
	Worlds.remove_tree(ROOT)


func _shell() -> Control:
	var shell := Main.new()
	shell.camp_root = ROOT
	shell.camp_frozen = true
	add_child(shell)
	return shell


func _hold(shell: Control) -> Control:
	var camp: Control = shell._page
	camp.world.pause()
	camp.set_process(false)
	camp.layout(Vector2(1080, 2400), Rect2(0, 154, 1080, 2246))
	camp._process(0)
	return camp


func test_front_door_has_few_controls_and_touch_selects_a_saved_person() -> void:
	var shell := _shell()
	var camp := _hold(shell)
	assert_str(shell.page_name()).is_equal("Camp")
	assert_int(shell._navigation.get_child_count()).is_equal(1)
	assert_bool(shell._developer.visible).is_false()
	var person := {}
	var at := Vector2.ZERO
	# Find an unobstructed thumb hit beyond a real glyph in this naturally spread camp.
	for candidate: Dictionary in camp.people:
		var rect: Rect2 = camp.drawing.drawn[int(candidate.id)]
		var centre := rect.get_center() * float(camp.state.scale)
		for offset: Vector2 in [Vector2(42, 0), Vector2(-42, 0), Vector2(0, 42)]:
			var point := centre + offset
			var clear := true
			for other: Dictionary in camp.people:
				if int(other.id) == int(candidate.id):
					continue
				var other_rect: Rect2 = camp.drawing.drawn[int(other.id)]
				if point.distance_to(other_rect.get_center() * float(camp.state.scale)) <= 50:
					clear = false
			if clear:
				person = candidate
				at = point
				break
		if not person.is_empty():
			break
	assert_dict(person).is_not_empty()
	for pressed: bool in [true, false]:
		var event := InputEventScreenTouch.new()
		event.position = at
		event.pressed = pressed
		camp._world_input(event)
	assert_int(camp.selected_id).is_equal(int(person.id))
	assert_str(camp._card.text).contains(str(person.name))
	assert_int(camp.selected_person().east_cm).is_equal(int(person.east_cm))
	assert_int(int(camp.state.scale)).is_equal(2)
	camp.layout(Vector2(2400, 1080), Rect2(0, 154, 2400, 926))
	camp._process(0)
	assert_int(int(camp.state.scale)).is_equal(2)
	assert_int(camp.selected_id).is_equal(int(person.id))
	camp.toggle_pause()
	assert_bool(camp.world.is_paused()).is_false()
	camp.toggle_pause()
	assert_bool(camp.world.is_paused()).is_true()
	camp.choose_speed(2)
	assert_int(camp._speed.selected).is_equal(2)
	assert_float(camp.world.speed()).is_equal(3600.0)
	camp.choose_speed(1)
	assert_float(camp.world.speed()).is_equal(60.0)
	shell.free()


func test_shell_switches_saved_camps_and_exports_imports_actual_people() -> void:
	var shell := _shell()
	var camp := _hold(shell)
	camp.world.run_until(43200)
	var deadline := Time.get_ticks_msec() + 1200
	while (
		camp.world.screen_time() < float(camp.world.frontier()) and Time.get_ticks_msec() < deadline
	):
		await await_idle_frame()
		camp._process(0)
	assert_float(camp.world.screen_time()).is_equal(float(camp.world.frontier()))
	camp.save_camp()
	var id: String = camp.world_id
	var people: Array = camp.world.people()
	var supplies: Dictionary = camp.world.camp_alpha()
	var digest: String = camp.world.digest()
	shell.open_page("Worlds")
	var shelf: Control = shell._page
	assert_bool(shelf.new_camp_alpha).is_true()
	var other: String = shelf.make_world()
	shelf.open_world(other)
	await await_idle_frame()
	assert_str(shell.page_name()).is_equal("Camp")
	camp = _hold(shell)
	assert_str(camp.world_id).is_equal(other)
	shell.open_page("Worlds")
	shelf = shell._page
	var path := ROOT.path_join("camp.kindling")
	assert_bool(shelf.export_to(id, path)).is_true()
	while shelf.busy():
		await await_idle_frame()
	assert_str(shelf.status).starts_with("Exported Camp alpha")
	assert_bool(shelf.import_from(path)).is_true()
	while shelf.busy():
		await await_idle_frame()
	assert_str(shelf.status).starts_with("Imported as Camp alpha")
	var copy := ""
	for record: Dictionary in shelf.listed:
		if record.id != id and record.id != other:
			copy = record.id
	assert_str(copy).is_not_empty()
	shelf.open_world(copy)
	await await_idle_frame()
	camp = _hold(shell)
	assert_array(camp.world.people()).is_equal(people)
	assert_dict(camp.world.camp_alpha()).is_equal(supplies)
	assert_str(camp.world.digest()).is_equal(digest)
	shell.open_page("Worlds")
	shell._page.open_world(id)
	await await_idle_frame()
	camp = _hold(shell)
	assert_str(camp.world_id).is_equal(id)
	assert_str(camp.world.digest()).is_equal(digest)
	# Developer marker fixture is separate and does not replace the player's selected camp.
	shell.open_page("Crowd")
	assert_str(shell._page.folder).is_equal(ROOT.path_join("marker-fixture"))
	assert_str(Worlds.at(ROOT).current()).is_equal(id)
	shell.open_page("Camp")
	camp = _hold(shell)
	assert_str(camp.world_id).is_equal(id)
	assert_str(camp.world.digest()).is_equal(digest)
	shell.free()


func test_failed_open_is_readable_and_menu_actions_remain_safe() -> void:
	var obstruction := FileAccess.open(ROOT, FileAccess.WRITE)
	obstruction.store_string("Injected failure: file blocks camp folder")
	obstruction.close()
	var shell := _shell()
	assert_bool(shell._page.opened.has("problem")).is_true()
	assert_bool(shell._page.draws_world).is_false()
	var labels: Array[Node] = shell._page.find_children("*", "Label", true, false)
	assert_int(labels.size()).is_equal(1)
	assert_str(labels[0].text).contains("Camp could not open")
	shell._save_camp()
	shell._camp_supplies()
	shell._go_back()
	assert_str(shell.page_name()).is_equal("Camp")
	shell.free()
	DirAccess.remove_absolute(ROOT)
