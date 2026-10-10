## The upright discovery route keeps real selection and accessible controls through text/rotation.
extends GdUnitTestSuite

const Example := preload("res://pages/first_flake.gd")
const Main := preload("res://main.gd")
const Preferences := preload("res://ui/preferences.gd")
const ROOT := "user://test-worlds/portrait-route"
var _window_before := Vector2i.ZERO
var _larger_before := false
var _mute_before := false


func before_test() -> void:
	_window_before = get_tree().root.size
	_larger_before = Preferences.value("large_text")
	_mute_before = Preferences.value("mute")


func after_test() -> void:
	Preferences.set_value("large_text", _larger_before)
	Preferences.set_value("mute", _mute_before)
	get_tree().root.size = _window_before
	Worlds.remove_tree(ROOT)


func _page() -> Control:
	var page := Example.new()
	page.root = ROOT
	add_child(page)
	page.set_process(false)
	page.select_person(int(page.capture.actor))
	return page


func _layout(page: Control, window: Vector2) -> Rect2:
	get_tree().root.size = Vector2i(window)
	var density := clampf(minf(window.x, window.y) / 450, 1, 3)
	page.navigation_height = 64 * density
	var safe := Rect2(0, page.navigation_height, window.x, window.y - page.navigation_height)
	page.layout(window, safe)
	for i in 4:
		await await_idle_frame()
		page._process(0)
	return safe


func _targets(page: Control, safe: Rect2, window: Vector2) -> void:
	var density := clampf(minf(window.x, window.y) / 450, 1, 3)
	for button: Control in [
		page._pause, page._speed, page._more, page._dream_button, page._history_button
	]:
		if not safe.grow(1).encloses(button.get_global_rect()):
			print("CLIP ", window, " ", button.name, " ", button.get_global_rect(), " safe ", safe)
		assert_bool(safe.grow(1).encloses(button.get_global_rect())).is_true()
		assert_float(button.size.y).is_greater_equal(48 * density - 1)
	var scroll: ScrollContainer = page._card_scroll
	assert_int(scroll.horizontal_scroll_mode).is_equal(ScrollContainer.SCROLL_MODE_DISABLED)
	assert_float(page._card.size.x).is_less_equal(scroll.size.x + 1)
	if window.x > window.y:
		assert_float(page._area.get_global_rect().end.x).is_less_equal(
			page._dock.get_global_rect().position.x
		)
	else:
		assert_float(page._dock.size.y).is_less_equal(safe.size.y * 0.60 + 1)


func test_portrait_discovery_details_large_text_and_landscape_hold_the_same_person() -> void:
	var page := _page()
	var selected: int = page.selected_id
	var digest: String = page.world.digest()
	assert_int(page._card.text.split("\n").size()).is_equal(2)
	assert_str(page._card.text).contains("Why:")
	assert_str(page._card.text).not_contains("Needs met:")
	assert_str(page._card.text).not_contains("Waiting for the next")
	if page.selected_person().get("work_known", false):
		assert_str(page._card.text).contains("Making")
	page._details = true
	page._refresh_records()
	for larger: bool in [false, true]:
		Preferences.set_value("large_text", larger)
		for window: Vector2 in [Vector2(360, 800), Vector2(1080, 2400), Vector2(800, 360)]:
			var safe: Rect2 = await _layout(page, window)
			_targets(page, safe, window)
			assert_int(page.selected_id).is_equal(selected)
			assert_str(page._card.text).contains("Personal knowledge")
			assert_str(page.world.digest()).is_equal(digest)
	page.free()


func test_rotation_keeps_history_and_the_fixed_dream_confirmation_without_a_request() -> void:
	var page := _page()
	var selected: int = page.selected_id
	var digest: String = page.world.digest()
	Preferences.set_value("large_text", true)
	page._history.open()
	for window: Vector2 in [Vector2(360, 800), Vector2(1080, 2400), Vector2(800, 360)]:
		var safe: Rect2 = await _layout(page, window)
		assert_bool(page._history.visible).is_true()
		assert_bool(safe.grow(1).encloses(page._history.get_global_rect())).is_true()
		assert_int(page.selected_id).is_equal(selected)
	page._history.close()
	page.open_dream(selected)
	var subjects: Array = page.world.dream_subjects(selected)
	assert_array(subjects).is_not_empty()
	page._dreams.choose(subjects[0])
	for window: Vector2 in [Vector2(360, 800), Vector2(1080, 2400), Vector2(800, 360)]:
		var safe: Rect2 = await _layout(page, window)
		assert_str(page._dreams.stage).is_equal("confirm")
		assert_bool(safe.grow(1).encloses(page._dreams.get_global_rect())).is_true()
		assert_bool(safe.grow(1).encloses(page._dreams._confirm.get_global_rect())).is_true()
		assert_bool(safe.grow(1).encloses(page._dreams._cancel.get_global_rect())).is_true()
	page._dreams.close()
	assert_array(page.world.dream_records()).is_empty()
	assert_str(page.world.digest()).is_equal(digest)
	page.free()


func test_menu_large_text_and_mute_stay_inside_the_small_phone_without_simulation_changes() -> void:
	get_tree().root.size = Vector2i(360, 800)
	var shell := Main.new()
	shell.camp_root = ROOT
	shell.camp_frozen = true
	add_child(shell)
	for i in 3:
		await await_idle_frame()
	var digest: String = shell._page.world.digest()
	shell._toggle_menu()
	for button: Node in shell._menu.find_children("*", "CheckButton", true, false):
		button.button_pressed = true
	for i in 4:
		await await_idle_frame()
	assert_bool(Preferences.value("large_text")).is_true()
	assert_bool(Preferences.value("mute")).is_true()
	var scroll: ScrollContainer = shell._menu_scroll
	assert_int(scroll.horizontal_scroll_mode).is_equal(ScrollContainer.SCROLL_MODE_DISABLED)
	for button: Node in shell._menu.find_children("*", "BaseButton", true, false):
		assert_float(button.get_global_rect().end.x).is_less_equal(360)
		assert_float(button.size.y).is_greater_equal(48)
	assert_str(shell._page.world.digest()).is_equal(digest)
	shell.free()
