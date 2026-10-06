## The Calibrate page, headless in the cloud (A17, A18.1): it reads the build's calibration scenes,
## picks its own step's, runs every variant of the scenes picked far faster than on the phone, and
## ends with one code that reads back with a reading for each variant run and none for the rest; and
## the app has the page. What each variant draws is held to its scene in tools/calibrun.py, on the
## software driver, where Godot draws for real.
extends GdUnitTestSuite

const CalibratePage := preload("res://pages/calibrate.gd")


# checks: PLT-04 RES-09
func test_the_page_reads_the_build_s_calibration_scenes_in_their_order() -> void:
	var page: VBoxContainer = auto_free(CalibratePage.new())
	add_child(page)
	assert_array(page.problems).is_empty()
	var names := []
	for scene: Dictionary in page.scenes:
		names.append(scene["name"])
	assert_array(names).is_equal(["c1", "c2", "c3", "c3-draws", "c4", "c5"])


# checks: PLT-04
func test_the_page_picks_its_own_step_s_scenes_and_those_their_numbers_take_from() -> void:
	var page: VBoxContainer = auto_free(CalibratePage.new())
	add_child(page)
	var step := str(ProjectSettings.get_setting("application/config/version", ""))
	var own: Array = page.scenes.map(func(scene: Dictionary) -> bool: return scene["step"] == step)
	# this build's own step's scenes, or every one when none is its step's
	if own.has(true):
		assert_array(page.picked).is_equal(own)
	else:
		assert_bool(page.picked.has(false)).is_false()
	for i in page.scenes.size():
		page.pick(i, false)
	# C1 takes C4's bare frame off, so picking C1 picks C4
	page.pick(0, true)
	assert_array(page.picked).is_equal([true, false, false, false, true, false])


# checks: PLT-04
func test_every_variant_runs_to_one_code_with_a_reading_for_each() -> void:
	var page: VBoxContainer = auto_free(CalibratePage.new())
	page.size = Vector2(540, 1100)
	add_child(page)
	page.time_scale = 0.002
	page.pick_all()
	page.start()
	assert_bool(page.running()).is_true()
	while page.running():
		await await_idle_frame()
	assert_str(page.code).is_not_empty()
	var read: Dictionary = page.calibration.read_code(page.code)
	assert_str(read["why"]).is_empty()
	assert_int(int(read["build"])).is_greater(0)
	var got: Array = read["readings"]
	assert_int(got.size()).is_equal(page.scenes.size())
	for i in page.scenes.size():
		var variants: Array = page.scenes[i]["variants"]
		assert_int((got[i] as Array).size()).is_equal(variants.size())
		for j in variants.size():
			# what the page measured, or -1 where headless Godot gives nothing, reads back as written
			for field: String in ["gpu_us", "cpu_us", "on_time", "power_mw", "heat"]:
				assert_int(int(got[i][j][field])).is_equal(int(page.readings[i][j][field]))
	# the code shown, with each scene's verdict
	assert_bool(page.shown.has(page.code)).is_true()
	for scene: Dictionary in page.scenes:
		var named := false
		for line: String in page.shown:
			named = named or line.begins_with(scene["name"] + ": ")
		assert_bool(named).override_failure_message(scene["name"]).is_true()
	# the shell's drawing back as it was
	assert_int(Engine.max_fps).is_equal(60)


# checks: PLT-04
func test_the_app_has_a_calibrate_page() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	main.open_page("Calibrate")
	assert_str(main.page_name()).is_equal("Calibrate")


# checks: PLT-04
func test_a_run_of_some_scenes_gives_a_code_holding_only_theirs() -> void:
	var page: VBoxContainer = auto_free(CalibratePage.new())
	page.size = Vector2(540, 1100)
	add_child(page)
	page.time_scale = 0.002
	for i in page.scenes.size():
		page.pick(i, page.scenes[i]["name"] == "c2")
	page.start()
	while page.running():
		await await_idle_frame()
	var read: Dictionary = page.calibration.read_code(page.code)
	assert_str(read["why"]).is_empty()
	for i in page.scenes.size():
		var ran: bool = page.scenes[i]["name"] == "c2"
		var variants: int = (page.scenes[i]["variants"] as Array).size()
		assert_int((read["readings"][i] as Array).size()).is_equal(variants if ran else 0)
	# only the scene run has its verdict shown
	for line: String in page.shown:
		assert_bool(line.begins_with("c1: ") or line.begins_with("c4: ")).is_false()
