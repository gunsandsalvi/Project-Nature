## The Reports page, headless in the cloud (A17): the cloud's last scene report shows its verdict,
## its rule, each measure's range with a chart drawn to scale, and its runs; the world it brought
## opens exactly as it ended in the cloud; a report's world opens marked as a test's world with
## its switches, imported only once; and a rerun and a provisional verdict are named.
extends GdUnitTestSuite

const ReportsPage := preload("res://pages/reports.gd")
const WorldsPage := preload("res://pages/worlds.gd")
const CrowdPage := preload("res://pages/crowd.gd")
const ROOT := "user://test-reports"


func after_test() -> void:
	Worlds.remove_tree(ROOT)


## The Reports page on a folder of reports, keeping worlds under the tests' own folder.
func _page(folder: String) -> VBoxContainer:
	var page: VBoxContainer = auto_free(ReportsPage.new())
	page.folder = folder
	page.root = ROOT.path_join("worlds")
	page.size = Vector2(540, 1100)
	add_child(page)
	return page


## A small report, as the kindling tool writes one, of a scene whose second run brought its world.
static func _fixture_report() -> Dictionary:
	var run := {
		"index": 0,
		"seed": 1,
		"days": 2,
		"digest": "0",
		"switches": ["no_greetings"],
		"measures": {"greetings": 0},
		"oddities": [],
	}
	var second := run.duplicate(true)
	second["index"] = 1
	second["seed"] = 2
	second["oddities"] = ["greetings 0, outside its expected 1 to 9"]
	return {
		"scene": "fixture",
		"about": "A scene for the tests",
		"checks": ["RES-06"],
		"world": "crowd",
		"camps": 4,
		"seed": 1,
		"runs": 2,
		"until": 172800,
		"switches": ["no_greetings"],
		"one_each": false,
		"rule": "greetings at least 1, in at least 1 of 2 runs",
		"measure": "greetings",
		"at_least": 1,
		"at_most": null,
		"in": 1,
		"passed": false,
		"passes": 0,
		"judged": 2,
		"needed": 1,
		"provisional": true,
		"reran": false,
		"oddities": 1,
		"seconds": 0,
		"budget": 360,
		"over_budget": false,
		"build": "α1.5a",
		"ranges":
		[
			{
				"measure": "greetings",
				"about": "the greetings in the run",
				"lowest": 0,
				"median": 0,
				"highest": 0,
				"runs": 2,
				"expected": [1, 9],
				"inside": 0,
			}
		],
		"each": [run, second],
	}


# checks: RES-06 RES-13
func test_the_cloud_s_last_report_shows_its_verdict_ranges_charts_and_runs() -> void:
	var page := _page(Reports.FOLDER)
	assert_int(page.reports.size()).is_greater_equal(1)
	var r: Dictionary = page.reports[0]
	assert_str(r["scene"]).is_equal("greetings")
	assert_str(Reports.verdict_words(r)).starts_with("Pass: ")
	assert_bool(page.shown.has(Reports.verdict_words(r))).is_true()
	assert_bool(page.shown.has("Its rule: %s" % r["rule"])).is_true()
	assert_bool(page.shown.has(Reports.setup_words(r))).is_true()
	# each measure's range in words, with a chart of every run's value
	var ranges: Array = r["ranges"]
	assert_int(ranges.size()).is_equal(4)
	for m: Dictionary in ranges:
		assert_bool(page.shown.has(Reports.range_words(m))).is_true()
	assert_int(page.charts.size()).is_equal(ranges.size())
	for chart: RangeChart in page.charts:
		assert_int(chart.values.size()).is_equal(r["each"].size())
	# the odd runs at first, and every run on a tap
	for run: Dictionary in r["each"]:
		assert_bool(page.shown.has(Reports.run_words(run))).is_equal(not run["oddities"].is_empty())
	page.show_all_runs("greetings", true)
	for run: Dictionary in r["each"]:
		assert_bool(page.shown.has(Reports.run_words(run))).is_true()


# checks: PLT-05
func test_the_cloud_s_test_world_opens_here_exactly_as_it_ended() -> void:
	var page := _page(Reports.FOLDER)
	var r: Dictionary = page.reports[0]
	var found := Reports.world_of(Reports.FOLDER, r)
	assert_bool(found.is_empty()).is_false()
	assert_bool(page.shown.has("Open run %d's world" % (int(found["index"]) + 1))).is_true()
	var id: String = page.open_world(r)
	assert_str(id).is_not_empty()
	# saved by this version in the cloud, it opens as its own, where the report says it ended
	var crowd: VBoxContainer = CrowdPage.new()
	crowd.root = ROOT.path_join("worlds")
	crowd.size = Vector2(540, 1100)
	add_child(crowd)
	assert_str(crowd.opened.get("problem", "")).is_empty()
	assert_bool(crowd.opened["made"]).is_false()
	assert_str(crowd.opened["update"]).is_equal("none")
	var run: Dictionary = r["each"][found["index"]]
	assert_str(crowd.world.digest()).is_equal(run["digest"])
	remove_child(crowd)
	crowd.free()
	for w: Dictionary in Worlds.at(ROOT.path_join("worlds")).list():
		assert_str(Worlds.test_words(w)).is_equal("A test's world, run with no switches")


# checks: PLT-05 RES-10
func test_a_report_s_world_opens_marked_as_a_test_world_with_its_switches_once() -> void:
	# a test's world with a switch, exported as the build exports a scene's, beside its report
	var made := Worlds.at(ROOT.path_join("made"))
	DirAccess.make_dir_recursive_absolute(ROOT.path_join("made/source"))
	var about := FileAccess.open(ROOT.path_join("made/source/world.toml"), FileAccess.WRITE)
	about.store_string(
		(
			"format = %d\n" % KdWorld.save_format()
			+ (
				'name = "fixture 2"\nkind = "crowd"\nseed = 2\ncamps = 4\ntest = true\n'
				+ 'switches = ["no_greetings"]\n'
			)
		)
	)
	about.close()
	var reports := ROOT.path_join("reports")
	DirAccess.make_dir_recursive_absolute(reports)
	assert_int(made.export_begin("source")).is_greater(0)
	var file := FileAccess.open(reports.path_join("fixture-2.kindling"), FileAccess.WRITE)
	var piece := made.export_next(1 << 20)
	while not piece.is_empty():
		file.store_buffer(piece)
		piece = made.export_next(1 << 20)
	file.close()
	var json := FileAccess.open(reports.path_join("fixture.json"), FileAccess.WRITE)
	json.store_string(JSON.stringify(_fixture_report()))
	json.close()
	# the page shows the report and offers the world of its second run
	var page := _page(reports)
	assert_int(page.reports.size()).is_equal(1)
	assert_bool(page.shown.has("Open run 2's world")).is_true()
	assert_bool(page.shown.has("  greetings 0, outside its expected 1 to 9")).is_true()
	# opened, it is a world on the phone marked as a test's world with its switch
	var id: String = page.open_world(page.reports[0])
	assert_str(id).is_not_empty()
	var listed := Worlds.at(ROOT.path_join("worlds")).list()
	assert_int(listed.size()).is_equal(1)
	assert_str(listed[0]["name"]).is_equal("fixture 2 (imported)")
	assert_bool(listed[0]["test"]).is_true()
	var marked := (
		"A test's world, run with no_greetings; this build has no switches, "
		+ "so it carries on without them"
	)
	assert_str(Worlds.test_words(listed[0])).is_equal(marked)
	assert_str(Worlds.at(ROOT.path_join("worlds")).current()).is_equal(id)
	# the Worlds page marks it so
	var shelf: VBoxContainer = auto_free(WorldsPage.new())
	shelf.root = ROOT.path_join("worlds")
	shelf.size = Vector2(540, 1100)
	add_child(shelf)
	var texts := PackedStringArray()
	for label: Label in shelf.find_children("*", "Label", true, false):
		texts.append(label.text)
	assert_bool(texts.has(Worlds.test_words(listed[0]))).is_true()
	# opened again, it is the same world, not a second copy
	assert_str(page.open_world(page.reports[0])).is_equal(id)
	assert_int(Worlds.at(ROOT.path_join("worlds")).list().size()).is_equal(1)


# checks: RES-13
func test_a_rerun_a_provisional_verdict_and_a_range_are_named() -> void:
	var rerun := {
		"passed": false,
		"passes": 27,
		"judged": 40,
		"needed": 32,
		"reran": true,
		"provisional": false
	}
	rerun["runs"] = 20
	var reran := (
		"Fail: 27 of 40 runs met the rule, 32 needed; it failed on its first 20, "
		+ "so it ran 20 more on fresh seeds"
	)
	assert_str(Reports.verdict_words(rerun)).is_equal(reran)
	var few := {
		"passed": true,
		"passes": 10,
		"judged": 12,
		"needed": 10,
		"reran": false,
		"provisional": true
	}
	few["runs"] = 12
	assert_str(Reports.verdict_words(few)).is_equal(
		"Pass: 10 of 12 runs met the rule, 10 needed; provisional, on fewer than 20 runs"
	)
	var m := {
		"lowest": 1352,
		"highest": 1406,
		"median": 1391,
		"expected": [0, 2000],
		"inside": 18,
		"runs": 20
	}
	assert_str(Reports.range_words(m)).is_equal(
		"1,352 to 1,406, 1,391 in the middle; inside its expected 0 to 2,000 in 18 of 20 worlds"
	)
	assert_str(Reports.time_words(_fixture_report())).is_equal(
		"It ran under α1.5a and took under a second of its budget of 6 min"
	)
	assert_str(Reports.setup_words(_fixture_report())).is_equal(
		"2 worlds of 4 camps, seeds 1 to 2, each 2 game days; with the switches no_greetings"
	)


# checks: RES-06
func test_a_chart_is_drawn_to_scale_with_runs_that_fall_together_stacked() -> void:
	var values: Array[int] = [16, 17, 17, 18, 17]
	# the axis holds the values, the expected range and the rule's bound, with a tenth more each side
	var axis := RangeChart.axis_of(values, [5, 30], 10, null)
	assert_that(axis).is_equal(Vector2i(2, 33))
	var width := 524.0
	var dots := RangeChart.dots(values, axis.x, axis.y, width)
	assert_int(dots.size()).is_equal(5)
	# sorted, the three 17s stack in one column, between the 16 and the 18
	assert_float(dots[1].x).is_equal(dots[2].x)
	assert_float(dots[2].x).is_equal(dots[3].x)
	assert_array([dots[1].y, dots[2].y, dots[3].y]).is_equal([0.0, 1.0, 2.0])
	assert_float(dots[0].x).is_less(dots[1].x)
	assert_float(dots[4].x).is_greater(dots[3].x)
	# labelled with the numbers that mean something: the rule's bound, the expected range's ends,
	# and the lowest and highest runs
	var chart: RangeChart = auto_free(RangeChart.new())
	chart.setup(values, [5, 30], 10, null)
	var words := PackedStringArray()
	for label: Array in chart.labels():
		words.append(label[1])
	assert_array(Array(words)).is_equal(["at least 10", "5", "30", "16", "18"])
	# to scale: each dot within one column of where its value falls
	for i in values.size():
		var v: int = [16, 17, 17, 17, 18][i]
		var at := RangeChart.x_of(v, axis.x, axis.y, width)
		assert_float(absf(dots[i].x - at)).is_less_equal(RangeChart.RADIUS + 0.5)


# checks: RES-06
func test_the_app_has_a_reports_page() -> void:
	var main: Control = auto_free(preload("res://main.gd").new())
	add_child(main)
	await await_idle_frame()
	main.open_page("Reports")
	assert_str(main.page_name()).is_equal("Reports")
