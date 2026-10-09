## Discovery menu routes and item evidence through real viewport touch events.
extends GdUnitTestSuite

const Main := preload("res://main.gd")
const ROOT := "user://test-worlds/discovery-route"
var _window_before := Vector2i.ZERO


func after_test() -> void:
	if _window_before != Vector2i.ZERO:
		get_tree().root.size = _window_before
	Worlds.remove_tree(ROOT)


func _shell() -> Control:
	_window_before = get_tree().root.size
	get_tree().root.size = Vector2i(1080, 2400)
	var shell := Main.new()
	shell.camp_root = ROOT.path_join("camps")
	shell.example_root = ROOT.path_join("example")
	shell.camp_frozen = true
	add_child(shell)
	for i in 3:
		await await_idle_frame()
	return shell


func _press(button: Button) -> void:
	var at := button.get_global_rect().get_center()
	_touch(at)
	await await_idle_frame()


func _touch(at: Vector2) -> void:
	for down: bool in [true, false]:
		var event := InputEventScreenTouch.new()
		event.position = at
		event.pressed = down
		get_viewport().push_input(event, true)
		var mouse := InputEventMouseButton.new()
		mouse.device = InputEvent.DEVICE_ID_EMULATION
		mouse.position = at
		mouse.global_position = at
		mouse.button_index = MOUSE_BUTTON_LEFT
		mouse.pressed = down
		get_viewport().push_input(mouse, true)


func _button(parent: Node, words: String) -> Button:
	for node: Node in parent.find_children("*", "Button", true, false):
		if node.text == words:
			return node
	assert_bool(false).is_true()
	return null


func test_menu_new_discovery_has_finite_stock_and_personal_starting_evidence() -> void:
	var shell := await _shell()
	await _press(shell._menu_button)
	await _press(_button(shell._menu, "Saved camps · export / import"))
	assert_str(shell.page_name()).is_equal("Worlds")
	var worlds: Control = shell._page
	var count: int = worlds.listed.size()
	await _press(_button(worlds, "New Discovery camp"))
	assert_int(worlds.listed.size()).is_equal(count + 1)
	assert_bool(worlds.listed.all(func(w: Dictionary) -> bool: return w.discovery)).is_true()
	var new_id := ""
	for row: Control in worlds._list.get_children():
		if str(row.get_child(0).text).begins_with("Discovery camp %d" % (count + 1)):
			for w: Dictionary in worlds.listed:
				if w.name == "Discovery camp %d" % (count + 1):
					new_id = str(w.id)
			await _press(_button(row, "Open"))
			break
	assert_str(shell.page_name()).is_equal("Camp")
	var page: Control = shell._page
	assert_str(page.world_id).is_equal(new_id)
	assert_int(page.people.size()).is_equal(25)
	assert_bool(page.world.camp_alpha().discovery).is_true()
	var stock: Array = page.world.items()
	assert_bool(stock.is_empty()).is_false()
	var know: Dictionary = page.world.knowledge(int(page.people[0].id))
	assert_int(know.skills.size()).is_equal(5)
	(
		assert_bool(know.hunches.all(func(h: Dictionary) -> bool: return int(h.source_memory) > 0))
		. is_true()
	)
	assert_bool(know.skills.all(func(s: Dictionary) -> bool: return int(s.route) == 0)).is_true()
	var digest: String = page.world.digest()
	page.save_camp()
	shell.open_page("Worlds")
	shell.open_page("Camp")
	assert_str(shell._page.world.digest()).is_equal(digest)
	assert_array(shell._page.world.items()).is_equal(stock)
	shell.free()


func test_examples_open_captured_maker_and_tool_without_revealing_hidden_properties() -> void:
	var shell := await _shell()
	await _press(shell._menu_button)
	await _press(_button(shell._menu, "Examples"))
	assert_str(shell.page_name()).is_equal("DiscoveryExamples")
	await _press(_button(shell._page, "Open First flake"))
	assert_str(shell.page_name()).is_equal("FirstFlake")
	var page: Control = shell._page
	assert_bool(page.opened.has("problem")).is_false()
	assert_int(page.selected_id).is_equal(str(page.capture.actor).to_int())
	assert_int(page.selected_item_id).is_equal(str(page.capture.result).to_int())
	assert_str(page._summary.text).contains("captured ordinary camp")
	assert_str(page._card.text).contains("Sharp flake")
	assert_bool(page._dream_button.disabled).is_true()
	page._details = true
	page._refresh_records()
	assert_str(page._card.text).contains("Known by")
	var learnt: Array = page.world.knowledge(page.selected_id).skills.filter(
		func(s: Dictionary) -> bool: return s.recipe == "base:sharp_flake"
	)
	assert_int(learnt.size()).is_equal(1)
	assert_int(learnt[0].route).is_equal(int(page.capture.route))
	assert_int(learnt[0].source_event).is_greater(0)
	assert_str(page._card.text).not_contains("Fuel")
	assert_str(page._card.text).not_contains("Burn")
	assert_array(page.capture.switches).is_empty()
	assert_str(page.world.digest()).is_equal(str(page.capture.digest))
	shell.free()


func test_distinct_raw_piles_are_selected_with_real_touch_and_items_have_no_dream_button() -> void:
	var shell := await _shell()
	var page: Control = shell._page
	page.select_person(int(page.people[0].id))
	var selected := {}
	for item: Dictionary in page.items:
		var id: int = int(item.id)
		if not page.drawing.drawn_items.has(id) or int(item.owner) != 0:
			continue
		var rect: Rect2 = page.drawing.drawn_items[id]
		var local: Vector2 = (
			rect.get_center() * float(page.state.scale) * float(page.state.live_scale)
			+ Vector2(page.state.offset)
		)
		if not Rect2(Vector2.ZERO, page._area.size).has_point(local):
			continue
		_touch(page._area.global_position + local)
		assert_int(page.selected_item_id).is_equal(id)
		assert_bool(page._dream_button.disabled).is_true()
		assert_str(page._card.text).not_contains("Fuel")
		selected[item.kind] = true
	assert_int(selected.size()).is_greater_equal(6)
	shell.free()


func test_people_at_the_same_actual_position_can_each_be_selected_by_touch() -> void:
	var shell := await _shell()
	var page: Control = shell._page
	var cluster: Array[int] = []
	var at := Vector2.ZERO
	for id: int in page.drawing.drawn:
		var rect: Rect2 = page.drawing.drawn[id]
		for other: int in page.drawing.drawn:
			if page.drawing.drawn[other] == rect:
				cluster.append(other)
		if cluster.size() > 1:
			at = (
				rect.get_center() * float(page.state.scale) * float(page.state.live_scale)
				+ Vector2(page.state.offset)
				+ page._area.global_position
			)
			break
		cluster.clear()
	assert_int(cluster.size()).is_greater(1)
	var digest: String = page.world.digest()
	var selected := {}
	for i in cluster.size():
		_touch(at)
		selected[page.selected_id] = true
		assert_str(page._card.text).contains("Others here")
	assert_int(selected.size()).is_equal(cluster.size())
	assert_bool(cluster.all(func(id: int) -> bool: return selected.has(id))).is_true()
	assert_str(page.world.digest()).is_equal(digest)
	shell.free()


func test_a_stock_marker_cycles_real_portions_without_changing_stock() -> void:
	var shell := await _shell()
	var page: Control = shell._page
	var representative := 0
	for id: int in page.drawing.item_piles:
		if page.drawing.item_piles[id].size() > 1:
			representative = id
			break
	assert_int(representative).is_greater(0)
	var pile: Array = page.drawing.item_piles[representative]
	var rect: Rect2 = page.drawing.drawn_items[representative]
	var at: Vector2 = (
		rect.get_center() * float(page.state.scale) * float(page.state.live_scale)
		+ Vector2(page.state.offset)
		+ page._area.global_position
	)
	var digest: String = page.world.digest()
	var selected := {}
	for i in pile.size():
		_touch(at)
		selected[page.selected_item_id] = true
		assert_bool(page._dream_button.disabled).is_true()
	assert_int(selected.size()).is_equal(pile.size())
	assert_bool(pile.all(func(id: int) -> bool: return selected.has(id))).is_true()
	assert_str(page.world.digest()).is_equal(digest)
	shell.free()
