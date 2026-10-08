## The self-check page, headless in the cloud (A17): the extension loads, the page's lines are
## built, and the simulation's digests on one thread and four equal the ones the build wrote for the
## phone.
extends GdUnitTestSuite

const CheckPage := preload("res://pages/check.gd")


# checks: PLT-01
func test_the_extension_is_loaded() -> void:
	assert_bool(ClassDB.class_exists("KdDevice")).is_true()


# checks: RES-05
func test_every_proof_suite_matches_the_build() -> void:
	var page: VBoxContainer = auto_free(CheckPage.new())
	page.build()
	var same_bits: Array = page.lines.filter(
		func(line: Dictionary) -> bool: return str(line["name"]).begins_with("Same bits")
	)
	assert_int(same_bits.size()).is_greater(0)
	for line: Dictionary in same_bits:
		assert_str(line["state"]).override_failure_message(str(line)).is_equal("ok")


# checks: RES-05
func test_simulation_threads_run_in_the_default_environment() -> void:
	var t: Dictionary = KdDevice.new().thread_check()
	assert_bool(t["default_in_work"]).is_true()
	assert_int(int(t["stack_mib"])).is_greater_equal(8)


# checks: PLT-01
func test_the_summary_counts_failures() -> void:
	var page: VBoxContainer = auto_free(CheckPage.new())
	(
		page
		. lines
		. assign(
			[
				{"name": "a", "value": "", "state": "ok"},
				{"name": "b", "value": "", "state": "fail"},
			]
		)
	)
	assert_str(page.summary()).is_equal("1 check fails")
	page.lines.append({"name": "c", "value": "", "state": "fail"})
	assert_str(page.summary()).is_equal("2 checks fail")
	assert_str(page.details()).contains("✘ b")


# checks: PLT-06
func test_the_app_opens_on_its_saved_camp() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	assert_str(main.page_name()).is_equal("Camp")
