## Real viewport finger events, Android mouse emulation and safe-area gesture cancellation.
extends GdUnitTestSuite

const Camp := preload("res://pages/camp.gd")
const TEST_ROOT := "user://test-worlds/camp-touch"
var _window_before := Vector2i.ZERO
var _mouse_before := false


func after_test() -> void:
	get_tree().root.size = _window_before
	Input.emulate_mouse_from_touch = _mouse_before
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


func _touch_page(landscape: bool = false) -> Control:
	_window_before = get_tree().root.size
	_mouse_before = Input.emulate_mouse_from_touch
	Input.emulate_mouse_from_touch = false
	get_tree().root.size = Vector2i(2400, 1080) if landscape else Vector2i(1080, 2400)
	var page := _page()
	await await_idle_frame()
	page.layout(
		Vector2(get_tree().root.size),
		Rect2(32, 154, get_tree().root.size.x - 64, get_tree().root.size.y - 190)
	)
	page._process(0)
	assert_vector(page._area.position).is_equal(Vector2(32, 154))
	return page


func _touch_target(page: Control, except: int = 0) -> Dictionary:
	for person: Dictionary in page.people:
		if int(person.id) == except:
			continue
		var rect: Rect2 = page.drawing.drawn[int(person.id)]
		var point := (
			rect.get_center() * float(page.state.scale) * float(page.state.live_scale)
			+ Vector2(page.state.offset)
		)
		if not Rect2(Vector2.ZERO, page._area.size).has_point(point):
			continue
		if (
			page.drawing.pick(
				page.camera.from_screen(point),
				page._hit_radius / float(page.state.scale) / float(page.state.live_scale)
			)
			== int(person.id)
		):
			return {
				"id": int(person.id), "name": person.name, "at": page._area.global_position + point
			}
	assert_bool(false).is_true()
	return {}


func _screen_touch(at: Vector2, pressed: bool, finger: int = 0) -> void:
	var event := InputEventScreenTouch.new()
	event.position = at
	event.index = finger
	event.pressed = pressed
	get_viewport().push_input(event, true)
	# Android also supplies a mouse event for finger zero. It must not restart the touch.
	if finger == 0:
		var mouse := InputEventMouseButton.new()
		mouse.device = InputEvent.DEVICE_ID_EMULATION
		mouse.position = at
		mouse.button_index = MOUSE_BUTTON_LEFT
		mouse.pressed = pressed
		get_viewport().push_input(mouse, true)


func _screen_drag(at: Vector2, relative: Vector2, finger: int = 0) -> void:
	var event := InputEventScreenDrag.new()
	event.position = at
	event.relative = relative
	event.index = finger
	get_viewport().push_input(event, true)
	if finger == 0:
		var mouse := InputEventMouseMotion.new()
		mouse.device = InputEvent.DEVICE_ID_EMULATION
		mouse.position = at
		mouse.relative = relative
		mouse.button_mask = MOUSE_BUTTON_MASK_LEFT
		get_viewport().push_input(mouse, true)


func _finger_wait(milliseconds: int) -> void:
	var deadline := Time.get_ticks_msec() + milliseconds
	while Time.get_ticks_msec() < deadline:
		await await_idle_frame()


func test_screen_touch_select() -> void:
	var page := await _touch_page()
	var previous := 0
	for i in 2:
		var target := _touch_target(page, previous)
		assert_int(int(target.id)).is_not_equal(previous)
		previous = int(target.id)
		_screen_touch(target.at, true)
		_screen_touch(target.at, false)
		assert_int(page.selected_id).is_equal(int(target.id))
		assert_str(page._card.text).contains(str(target.name))
	page.free()


func test_screen_touch_hold_ring() -> void:
	var page := await _touch_page()
	var target := _touch_target(page)
	_screen_touch(target.at, true)
	await _finger_wait(400)
	page._process(0)
	assert_bool(page._dreams.visible).is_false()
	await _finger_wait(200)
	page._process(0)
	assert_bool(page._dreams.visible).is_true()
	assert_int(page.selected_id).is_equal(int(target.id))
	assert_str(page._dreams.stage).is_equal("ring")
	_screen_touch(target.at, false)
	page.free()


func test_screen_drag_no_hold() -> void:
	var page := await _touch_page()
	var target := _touch_target(page)
	_screen_touch(target.at, true)
	_screen_drag(target.at + Vector2(90, 0), Vector2(90, 0))
	assert_bool(page._touches[0].moved).is_true()
	await _finger_wait(600)
	page._process(0)
	_screen_touch(target.at + Vector2(90, 0), false)
	assert_bool(page._dreams.visible).is_false()
	assert_int(page.selected_id).is_equal(0)
	page.free()


func test_two_finger_no_hold() -> void:
	var page := await _touch_page()
	var target := _touch_target(page)
	_screen_touch(target.at, true)
	_screen_touch(target.at + Vector2(140, 0), true, 1)
	_screen_drag(target.at + Vector2(200, 0), Vector2(60, 0), 1)
	_screen_touch(target.at + Vector2(200, 0), false, 1)
	assert_bool(page._touches[0].alone).is_false()
	await _finger_wait(600)
	page._process(0)
	_screen_touch(target.at, false)
	assert_bool(page._dreams.visible).is_false()
	assert_int(page.selected_id).is_equal(0)
	page.free()


func test_touch_after_rotation() -> void:
	var page := await _touch_page()
	var target := _touch_target(page)
	_screen_touch(target.at, true)
	get_tree().root.size = Vector2i(2400, 1080)
	await await_idle_frame()
	page.layout(Vector2(2400, 1080), Rect2(40, 154, 2320, 890))
	assert_dict(page._touches).is_empty()
	page._process(0)
	_screen_touch(target.at, false)
	assert_int(page.selected_id).is_equal(0)
	target = _touch_target(page)
	_screen_touch(target.at, true)
	_screen_touch(target.at, false)
	assert_int(page.selected_id).is_equal(int(target.id))
	_screen_touch(target.at, true)
	await _finger_wait(600)
	page._process(0)
	assert_bool(page._dreams.visible).is_true()
	assert_str(page._dreams.stage).is_equal("ring")
	_screen_touch(target.at, false)
	page.free()


func test_canceled_finger_does_not_select_or_hold() -> void:
	var page := await _touch_page()
	var target := _touch_target(page)
	_screen_touch(target.at, true)
	var event := InputEventScreenTouch.new()
	event.position = target.at
	event.canceled = true
	get_viewport().push_input(event, true)
	assert_dict(page._touches).is_empty()
	await _finger_wait(600)
	page._process(0)
	assert_int(page.selected_id).is_equal(0)
	assert_bool(page._dreams.visible).is_false()
	page.free()
