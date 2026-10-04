## P2's screen: the figures' steps and poses, the fire light's switch, the forest view and the chat
## line.
extends GdUnitTestSuite

const SCENE := preload("res://scene/scene.gd")
const RUNS := preload("res://look/runs.gd")


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


# checks: PRE-27, PRE-44, PRE-28
func test_everyone_is_one_of_the_kits_figures_in_a_pose_and_tiny_from_afar() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.set_process(false)
	var poses: Array = screen.get("_poses")
	var drawn := 0
	for mm: MultiMesh in poses:
		drawn += mm.visible_instance_count
	assert_int(drawn).is_equal(SCENE.FIGURES)
	# walkers and workers move through the kit's movements, so the drawn poses differ step to step
	var before := poses.map(func(mm: MultiMesh) -> int: return mm.visible_instance_count)
	screen.call("_step_figures", 0.11)
	var after := poses.map(func(mm: MultiMesh) -> int: return mm.visible_instance_count)
	assert_array(after).is_not_equal(before)
	screen.call("_set_view", "camp")
	screen.call("_step_figures", 0.11)
	for person: Dictionary in screen.get("_people"):
		assert_float(person.size).is_equal(SCENE.FAR_SCALE)


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
	assert_str(RUNS.summary("close", PackedFloat32Array([1.0, 2.0, 3.0, 4.0]), 1)).is_equal(
		"close 2.5/4.0 75%"
	)
	var heat := [Vector2(0.41, 0), Vector2(-1, -1), Vector2(0.47, 1)]
	assert_str(RUNS.code("P2 α0.2b", PackedStringArray(["close 9.1/12.0 100%"]), heat)).is_equal(
		"P2 α0.2b | close 9.1/12.0 100% | heat 0.41→0.47 s1"
	)
	(
		assert_str(RUNS.code("P2", PackedStringArray(["camp 1.0/2.0 99%"]), [Vector2(-1, -1)]))
		. is_equal("P2 | camp 1.0/2.0 99% | heat ?")
	)


# checks: PLT-04
func test_measure_turns_the_view_and_ends_with_the_line() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.set_process(false)
	var crawl: int = screen.crawl
	screen.call("_on_button", "measure")
	var yaw: float = screen.yaw
	for i in 60:
		screen.call("_process", 1.0 / 60.0)
	assert_float(absf(angle_difference(deg_to_rad(screen.yaw), deg_to_rad(yaw)))).is_greater(
		deg_to_rad(8.0)
	)
	for i in 400:
		screen.call("_process", 0.25)
	assert_int((screen.get("_runs") as RefCounted).get("index")).is_equal(-1)
	assert_str((screen.get("_readout") as Label).text).contains("P2 ")
	assert_int(screen.crawl).is_equal(crawl)
