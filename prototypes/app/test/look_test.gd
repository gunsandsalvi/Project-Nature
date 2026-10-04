## P1's screen: the camera locked to the art pixels' grid, and its switches.
extends GdUnitTestSuite

const LOOK := preload("res://look/look.gd")


func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(LOOK)
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.size = Vector2(400, 890)
	return auto_free(screen)


# checks: PRE-22
func test_the_camera_stands_on_whole_art_pixels_and_the_picture_takes_the_rest() -> void:
	var screen := _screen()
	# free turns, so the view's angle stays where it is; a few hundred metres from the origin a
	# single-precision position rounds to about a thousandth of an art pixel
	screen.crawl = 0
	await await_idle_frame()
	var start: Vector3 = screen.target
	for i in 25:
		screen.target = start + Vector3(0.0137 * i * i, 0.0, -0.0213 * i)
		screen.call("_apply_camera")
		var cam: Camera3D = screen.get("_cam")
		var basis := cam.transform.basis
		var x: float = cam.transform.origin.dot(basis.x) / screen.mpp
		var y: float = cam.transform.origin.dot(basis.y) / screen.mpp
		assert_float(x - roundf(x)).is_equal_approx(0.0, 2e-3)
		assert_float(y - roundf(y)).is_equal_approx(0.0, 2e-3)
		# what the camera could not move goes to the picture, never a whole art pixel or more
		var wanted: Vector3 = screen.target + basis.z * LOOK.BACK
		var rest: Vector2 = (
			Vector2(wanted.dot(basis.x), wanted.dot(basis.y)) / screen.mpp - Vector2(x, y)
		)
		assert_float(absf(rest.x)).is_less_equal(0.5001)
		assert_float(absf(rest.y)).is_less_equal(0.5001)


# checks: PRE-21 PRE-26
func test_the_switches_turn_the_passes_on_and_off() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.call("_set_outline", 3)
	assert_int((screen.get("_gbuf") as SubViewport).render_target_update_mode).is_equal(
		SubViewport.UPDATE_ALWAYS
	)
	screen.call("_set_outline", 1)
	assert_int((screen.get("_gbuf") as SubViewport).render_target_update_mode).is_equal(
		SubViewport.UPDATE_DISABLED
	)
	assert_bool((screen.get("_post") as MeshInstance3D).visible).is_true()
	screen.call("_set_outline", 4)
	assert_object((screen.get("_solid") as ShaderMaterial).next_pass).is_not_null()
	screen.call("_set_reflect", true)
	assert_int((screen.get("_mirror") as SubViewport).render_target_update_mode).is_equal(
		SubViewport.UPDATE_ALWAYS
	)


func _touch(vp: Viewport, index: int, at: Vector2, down: bool) -> void:
	var e := InputEventScreenTouch.new()
	e.index = index
	e.position = at
	e.pressed = down
	vp.push_input(e, true)


func _drag(vp: Viewport, index: int, from: Vector2, to: Vector2) -> void:
	var e := InputEventScreenDrag.new()
	e.index = index
	e.position = to
	e.relative = to - from
	vp.push_input(e, true)


# checks: PRE-22
func test_one_finger_pans_and_two_fingers_turn_the_view() -> void:
	var screen := _screen()
	await await_idle_frame()
	var vp := screen.get_viewport()
	var start: Vector3 = screen.target
	_touch(vp, 0, Vector2(200, 250), true)
	_drag(vp, 0, Vector2(200, 250), Vector2(240, 250))
	# the phone also makes mouse events from the first finger, which must not pan a second time
	var mouse := InputEventMouseMotion.new()
	mouse.device = InputEvent.DEVICE_ID_EMULATION
	mouse.position = Vector2(240, 250)
	mouse.relative = Vector2(40, 0)
	mouse.button_mask = MOUSE_BUTTON_MASK_LEFT
	vp.push_input(mouse, true)
	_touch(vp, 0, Vector2(240, 250), false)
	var pixel: float = screen.mpp * screen.call("_screen_scale") / LOOK.ART
	assert_float((screen.target - start).length()).is_equal_approx(40.0 * pixel, 1e-4)
	var yaw: float = screen.yaw
	_touch(vp, 0, Vector2(150, 250), true)
	_touch(vp, 1, Vector2(250, 250), true)
	var turned := Vector2(150, 250) + Vector2.from_angle(deg_to_rad(20.0)) * 100.0
	_drag(vp, 1, Vector2(250, 250), turned)
	_touch(vp, 1, turned, false)
	_touch(vp, 0, Vector2(150, 250), false)
	assert_float(screen.yaw).is_equal_approx(yaw + 20.0, 1e-3)


# checks: PRE-22
func test_the_crawl_fixes_turn_in_whole_steps_and_ease_to_rest_on_them() -> void:
	var screen := _screen()
	await await_idle_frame()
	var start: float = screen.yaw
	screen.crawl = 1
	screen.call("_turn", 10.0)
	assert_float(screen.yaw).is_equal_approx(start, 1e-4)
	screen.call("_turn", 10.0)
	assert_float(screen.yaw).is_equal_approx(start + LOOK.TURN_STEP, 1e-4)
	screen.crawl = 2
	screen.yaw = 52.0
	for i in 120:
		screen.call("_ease", 1.0 / 60.0)
	assert_float(screen.yaw).is_equal_approx(45.0, 1e-3)
