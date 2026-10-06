## The benchmark, headless in the cloud (A17): every scenario runs on its own page, a hundred times
## faster than on the phone, and ends with a code the cloud's decoder reads, every scenario's world
## ending exactly as the cloud's headless run of it (RES-05); and the frames are timed by the
## extension's own clock.
extends GdUnitTestSuite

const BenchPage := preload("res://pages/bench.gd")


func after_test() -> void:
	Worlds.remove_tree(BenchPage.ROOT)


# checks: PLT-04 RES-05
func test_every_scenario_runs_and_ends_as_the_cloud_s_in_a_code_the_cloud_reads() -> void:
	var page: VBoxContainer = auto_free(BenchPage.new())
	page.time_scale = 0.01
	page.size = Vector2(540, 1100)
	add_child(page)
	assert_int(page.scenarios.size()).is_equal(7)
	page.start()
	while page.running():
		await await_idle_frame()
	assert_str(page.code).is_not_empty()
	var read: Dictionary = KdDevice.new().bench_read(page.code)
	assert_str(read["why"]).is_empty()
	var values: Dictionary = read["values"]
	for s: Dictionary in page.scenarios:
		var n: String = s["name"]
		# its world's digest at its mark, taken on the page as the phone takes it, is the cloud's
		assert_int(int(values.get(n + ".digest", 0))).override_failure_message(n).is_equal(1)
		assert_bool(values.has(n + ".on_time")).override_failure_message(n).is_true()
	assert_bool(values.has("saves.open_ms")).is_true()
	assert_bool(values.has("saves.export_ms")).is_true()
	assert_int(int(values["build"])).is_greater(0)
	# the worlds it ran are gone, and none of yours was touched
	assert_bool(DirAccess.dir_exists_absolute(BenchPage.ROOT)).is_false()


# checks: PLT-04
func test_the_app_has_a_bench_page() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	main.open_page("Bench")
	assert_str(main.page_name()).is_equal("Bench")


# checks: PLT-04
func test_the_minutes_to_light_throttling_follow_the_heat_s_rise() -> void:
	var seconds := [0.0, 60.0, 120.0]
	# the forecast rises 0.01 a minute and stands at 0.50: 0.70 is 20 minutes away
	var rising := BenchPage.minutes_to_light(seconds, [0.48, 0.49, 0.50], 0.70)
	assert_float(rising).is_equal_approx(20.0, 1e-6)
	assert_float(BenchPage.minutes_to_light(seconds, [0.5, 0.5, 0.5], 0.70)).is_equal(500.0)
	assert_float(BenchPage.minutes_to_light(seconds, [0.6, 0.7, 0.8], 0.70)).is_equal(0.0)
	# no level to aim at, or a single reading, gives no answer
	assert_float(BenchPage.minutes_to_light(seconds, [0.48, 0.49, 0.50], -1.0)).is_equal(-1.0)
	assert_float(BenchPage.minutes_to_light([0.0], [0.5], 0.70)).is_equal(-1.0)
