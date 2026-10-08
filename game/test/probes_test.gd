## The probes, headless in the cloud (A4.7): each is noted before it runs, so one that closed the
## app is named on the next start and not tried again; a new build tries them all again.
extends GdUnitTestSuite


func _probes() -> Probes:
	var probes: Probes = auto_free(Probes.new())
	add_child(probes)
	return probes


func _note(version: String, canvas_nearest: String) -> void:
	var config := ConfigFile.new()
	config.set_value("build", "version", version)
	config.set_value("probes", "canvas_nearest", canvas_nearest)
	config.save(Probes.FILE)


# checks: VIS-14
func test_a_probe_left_running_is_named_as_closing_the_app_and_not_tried_again() -> void:
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	_note(version, "running")
	var probes := _probes()
	probes.start()
	await await_signal_on(probes, "finished", [], 20000)
	assert_str(probes.results["canvas_nearest"]).is_equal("crashed")
	assert_str(probes.results["canvas_own_levels"]).is_equal("passed")
	assert_str(probes.results["canvas_texture_grad"]).is_equal("passed")
	assert_int(probes.results.size()).is_equal(Probes.ABOUT.size())
	# a new build tries every probe again
	_note("an older build", "crashed")
	var again := _probes()
	again.start()
	await await_signal_on(again, "finished", [], 20000)
	assert_str(again.results["canvas_nearest"]).is_equal("passed")
