## The accelerated smoke proves the measurement route, not phone performance.
extends GdUnitTestSuite

const Main := preload("res://main.gd")
const Measure := preload("res://pages/camp_measure.gd")
const ROOT := "user://test-worlds/measure-route"
var _window_before := Vector2i.ZERO


func after_test() -> void:
	if _window_before != Vector2i.ZERO:
		get_tree().root.size = _window_before
		_window_before = Vector2i.ZERO
	Worlds.remove_tree(ROOT)
	Worlds.remove_tree("user://camp-measure")


func _press(button: Button) -> void:
	var at := button.get_global_rect().get_center()
	for down: bool in [true, false]:
		var touch := InputEventScreenTouch.new()
		touch.position = at
		touch.pressed = down
		get_viewport().push_input(touch, true)
		var mouse := InputEventMouseButton.new()
		mouse.device = InputEvent.DEVICE_ID_EMULATION
		mouse.position = at
		mouse.global_position = at
		mouse.button_index = MOUSE_BUTTON_LEFT
		mouse.pressed = down
		get_viewport().push_input(mouse, true)
	await await_idle_frame()


func _route() -> Control:
	_window_before = get_tree().root.size
	get_tree().root.size = Vector2i(1080, 2400)
	var shell := Main.new()
	shell.camp_root = ROOT
	shell.camp_frozen = true
	add_child(shell)
	await await_idle_frame()
	await _press(shell._menu_button)
	for node in shell._menu.find_children("*", "Button", true, false):
		if node.text == "Developer tools":
			await _press(node)
	await await_idle_frame()
	for node in shell._developer.get_children():
		if node.text == "Camp performance test":
			shell._menu_scroll.ensure_control_visible(node)
			await await_idle_frame()
			await _press(node)
	assert_str(shell.page_name()).is_equal("CampMeasure")
	return shell


func test_separate_camp_measurement_keeps_three_speed_reports() -> void:
	var shell := await _route()
	var page: Control = shell._page
	page.time_scale = 1000
	assert_str(page.root).is_equal("user://camp-measure")
	assert_bool(page.world.is_paused()).is_true()
	await _press(page._run_button)
	var deadline := Time.get_ticks_msec() + 5000
	while page.report.is_empty() and Time.get_ticks_msec() < deadline:
		await await_idle_frame()
	assert_bool(page.report.is_empty()).is_false()
	var result: Dictionary = JSON.parse_string(page.report)
	assert_int(result.phases.size()).is_equal(3)
	(
		assert_array(result.phases.map(func(p: Dictionary) -> int: return int(p.speed)))
		. is_equal([1, 60, 3600])
	)
	assert_float(result.time_scale).is_equal(1000.0)
	assert_bool(page.world.is_paused()).is_true()
	assert_bool(result.samples.is_empty()).is_false()
	assert_bool(FileAccess.file_exists("user://camp-measurement.json")).is_true()
	shell.free()
	Worlds.remove_tree("user://camp-measure")


func test_measurement_camp_also_feeds_the_heat_governor() -> void:
	var page := Measure.new()
	add_child(page)
	page.set_process(false)
	var hot := preload("res://test/camp_test.gd").HotDevice.new()
	page.device = hot
	page._process(2)
	assert_float(float(page.world.counters().share)).is_equal(0.5)
	assert_int(hot.reads).is_equal(1)
	page._process(2)
	assert_float(float(page.world.counters().share)).is_equal(0.25)
	assert_int(hot.reads).is_equal(2)
	page.free()
	Worlds.remove_tree("user://camp-measure")


func test_real_touch_starts_visible_counter_and_foreground_loss_interrupts() -> void:
	var shell := await _route()
	var page: Control = shell._page
	DisplayServer.screen_set_keep_on(false)
	await _press(page._run_button)
	await await_idle_frame()
	assert_bool(page._running).is_true()
	assert_bool(page.world.is_paused()).is_false()
	assert_str(page._summary.text).contains("Camp measurement")
	assert_str(page._summary.text).contains("left · real time")
	assert_bool(page._summary.is_visible_in_tree()).is_true()
	assert_bool(page._run_button.disabled).is_true()
	if OS.get_name() == "Android":
		assert_bool(DisplayServer.screen_is_kept_on()).is_true()
	page.notification(Node.NOTIFICATION_APPLICATION_PAUSED)
	page.notification(Node.NOTIFICATION_APPLICATION_RESUMED)
	assert_bool(page._running).is_false()
	assert_bool(page.world.is_paused()).is_true()
	var result: Dictionary = JSON.parse_string(page.report)
	assert_bool(result.interrupted).is_true()
	assert_int(int(result.background_interruptions)).is_equal(1)
	assert_bool(DisplayServer.screen_is_kept_on()).is_false()
	shell.free()
