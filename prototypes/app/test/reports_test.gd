## The Reports page: a cloud prototype's report drawn from its numbers, P4's first.
extends GdUnitTestSuite

const REPORTS := preload("res://reports/reports.gd")
const CHART := preload("res://reports/chart.gd")


func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(REPORTS)
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.size = Vector2(400, 890)
	return auto_free(screen)


# checks: RES-06
func test_p4s_report_shows_its_answer_and_four_charts() -> void:
	var screen := _screen()
	await await_idle_frame()
	var charts := screen.find_children("*", "Control", true, false).filter(
		func(n: Node) -> bool: return n.get_script() == CHART
	)
	assert_int(charts.size()).is_equal(4)
	var texts := screen.find_children("*", "Label", true, false).map(
		func(n: Node) -> String: return (n as Label).text
	)
	assert_bool(texts.has("P4 Discovery pace")).is_true()
	# every run of the report is a row of the first chart
	var report: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(REPORTS.REPORTS[0]))
	assert_int((charts[0] as Control).get("rows").size()).is_equal((report.runs as Array).size())


# checks: RES-06
func test_a_chart_puts_each_year_in_its_place() -> void:
	var chart: Control = auto_free(CHART.new("runs", [[2.5, "accident", "o"]], 5.0, 1.0))
	chart.size = Vector2(334, 60)
	assert_float(chart.call("x_of", 0.0)).is_equal(CHART.PAD.x)
	assert_float(chart.call("x_of", 5.0)).is_equal(334.0 - 8.0)
	assert_float(chart.call("x_of", 2.5)).is_equal_approx((CHART.PAD.x + 326.0) / 2.0, 1e-4)
	# a year past the axis's end sits at its end
	assert_float(chart.call("x_of", 9.0)).is_equal(334.0 - 8.0)
