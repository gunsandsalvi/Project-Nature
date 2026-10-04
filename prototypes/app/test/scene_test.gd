## P2's screen: the figures' steps, the fire light's switch, the forest view and the chat line.
extends GdUnitTestSuite

const SCENE := preload("res://scene/scene.gd")


func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(SCENE)
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.size = Vector2(400, 890)
	return auto_free(screen)


## Where the figures stand: the screen's own record, since the headless renderer keeps no
## instanced transforms.
func _places(screen: Control) -> Array:
	return (screen.get("_people") as Array).map(func(p: Dictionary) -> Vector3: return p.at)


# checks: PRE-44
func test_the_figures_move_only_in_steps_of_a_tenth_of_a_second() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.set_process(false)
	screen.set("_step_clock", 0.0)
	var before := _places(screen)
	screen.call("_step_figures", 0.04)
	assert_array(_places(screen)).is_equal(before)
	screen.call("_step_figures", 0.07)
	assert_array(_places(screen)).is_not_equal(before)


# checks: MAT-18
func test_the_fire_light_is_ours_or_godots() -> void:
	var screen := _screen()
	await await_idle_frame()
	var omni: Array = screen.get("_omni")
	screen.call("_set_fire_light", "Godot's")
	assert_bool((omni[2] as OmniLight3D).visible).is_true()
	assert_bool(screen.call("fire_powers") == Vector4.ZERO).is_true()
	screen.call("_set_fire_light", "ours")
	assert_bool((omni[2] as OmniLight3D).visible).is_false()
	assert_float((screen.call("fire_powers") as Vector4).z).is_greater(0.0)


# checks: PRE-28
func test_the_camp_view_shows_the_forest() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.call("_set_view", "camp")
	assert_bool((screen.get("_forest") as Node3D).visible).is_true()
	assert_float(screen.mpp).is_equal(SCENE.CAMP_MPP)
	screen.call("_set_view", "close")
	assert_bool((screen.get("_forest") as Node3D).visible).is_false()


# checks: PLT-04
func test_the_line_for_the_chat() -> void:
	assert_str(SCENE.summary("close", PackedFloat32Array([1.0, 2.0, 3.0, 4.0]), 1)).is_equal(
		"close 2.5/4.0 75%"
	)
	var heat := [Vector2(0.41, 0), Vector2(-1, -1), Vector2(0.47, 1)]
	assert_str(SCENE.code("P2 α0.2b", PackedStringArray(["close 9.1/12.0 100%"]), heat)).is_equal(
		"P2 α0.2b | close 9.1/12.0 100% | heat 0.41→0.47 s1"
	)
	(
		assert_str(SCENE.code("P2", PackedStringArray(["camp 1.0/2.0 99%"]), [Vector2(-1, -1)]))
		. is_equal("P2 | camp 1.0/2.0 99% | heat ?")
	)
