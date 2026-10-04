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
	await await_idle_frame()
	var start: Vector3 = screen.target
	for i in 25:
		screen.target = start + Vector3(0.0137 * i * i, 0.0, -0.0213 * i)
		screen.call("_apply_camera")
		var cam: Camera3D = screen.get("_cam")
		var basis := cam.transform.basis
		var x: float = cam.transform.origin.dot(basis.x) / screen.mpp
		var y: float = cam.transform.origin.dot(basis.y) / screen.mpp
		assert_float(x - roundf(x)).is_equal_approx(0.0, 1e-3)
		assert_float(y - roundf(y)).is_equal_approx(0.0, 1e-3)
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
