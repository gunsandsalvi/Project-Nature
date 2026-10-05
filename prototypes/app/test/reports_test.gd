## The Reports page: the cloud prototypes' reports drawn from their numbers, P4's, P9's and P10's.
extends GdUnitTestSuite

const REPORTS := preload("res://reports/reports.gd")
const CHART := preload("res://reports/chart.gd")
const LINES := preload("res://reports/lines.gd")


func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(REPORTS)
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.size = Vector2(400, 890)
	return auto_free(screen)


# checks: RES-06
func test_each_report_shows_its_answer_and_charts() -> void:
	var screen := _screen()
	await await_idle_frame()
	var charts := screen.find_children("*", "Control", true, false).filter(
		func(n: Node) -> bool: return n.get_script() == CHART
	)
	# P4's four charts, then P10's four, one for each first; P9's are lines over the years
	assert_int(charts.size()).is_equal(8)
	var lines := screen.find_children("*", "Control", true, false).filter(
		func(n: Node) -> bool: return n.get_script() == LINES
	)
	assert_bool(lines.size() > 0).is_true()
	var texts := screen.find_children("*", "Label", true, false).map(
		func(n: Node) -> String: return (n as Label).text
	)
	for title: String in ["P4 Discovery pace", "P9 Ecology", "P10 Culture from causes"]:
		assert_bool(texts.has(title)).is_true()
	# every run of P4's report is a row of its first chart, and of P10's each of its own
	var p4: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(REPORTS.REPORTS[0]))
	assert_int((charts[0] as Control).get("rows").size()).is_equal((p4.runs as Array).size())
	var p10: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(REPORTS.REPORTS[2]))
	for k in 4:
		assert_int((charts[4 + k] as Control).get("rows").size()).is_equal(
			(p10.runs as Array).size()
		)


# checks: RES-06
func test_a_chart_puts_each_year_in_its_place() -> void:
	var chart: Control = auto_free(CHART.new("runs", [[2.5, "accident", "o"]], 5.0, 1.0))
	chart.size = Vector2(334, 60)
	assert_float(chart.call("x_of", 0.0)).is_equal(CHART.PAD.x)
	assert_float(chart.call("x_of", 5.0)).is_equal(334.0 - 8.0)
	assert_float(chart.call("x_of", 2.5)).is_equal_approx((CHART.PAD.x + 326.0) / 2.0, 1e-4)
	# a year past the axis's end sits at its end
	assert_float(chart.call("x_of", 9.0)).is_equal(334.0 - 8.0)
