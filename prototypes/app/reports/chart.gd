## One chart of a cloud prototype's report (IMPLEMENTATION α0.3a, RES-06), drawn from its numbers so
## it stays sharp on the phone: "runs", each run a row with its year as a dot, coloured by how it
## came, against a window of years; or "ranges", each tuned value a row from the median year with it
## doubled to the median with it halved. Pre-production code (research 00).
extends Control

const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const EDGE := Color("3a3346")
const WINDOW := Color(0.96, 0.64, 0.24, 0.14)
## How each step came (MND-11's routes, and how a learner learned): its colour.
const ROUTES := {
	"accident": Color("f6a33c"),
	"experiment": Color("8fb7ff"),
	"hunch": Color("8fb7ff"),
	"dream": Color("c69cff"),
	"copying": Color("9fd48a"),
	"first": Color("f6a33c"),
	"none": Color("6b6474"),
}
const PAD := Vector2(34, 18)
const FONT_SIZE := 12

var kind := "runs"
## For "runs": each [year or null, route, mark], mark "o" or "s" (drilled or ploughed); for
## "ranges": each [label, low year, high year].
var rows := []
var axis_end := 5.0
var tick := 1.0
## The window of years shaded, or none.
var window := Vector2(-1.0, -1.0)
## A line drawn across, such as RES-03's five years, or none.
var limit := -1.0
## For "ranges": the year with nothing changed.
var middle := -1.0
## Room on the left: for the years' labels, or the tuned values' names.
var left := PAD.x


func _init(chart_kind: String, chart_rows: Array, end: float, step: float) -> void:
	kind = chart_kind
	rows = chart_rows
	axis_end = end
	tick = step
	var per_row := 6.0 if chart_kind == "runs" else 16.0
	left = PAD.x if chart_kind == "runs" else 104.0
	custom_minimum_size = Vector2(0, PAD.y * 2.0 + per_row * maxf(rows.size(), 1))


## Where a year falls across the chart.
func x_of(year: float) -> float:
	var w := size.x - left - 8.0
	return left + clampf(year / axis_end, 0.0, 1.0) * w


func _draw() -> void:
	var font := get_theme_default_font()
	var top := PAD.y
	var bottom := size.y - PAD.y
	if window.x >= 0.0:
		draw_rect(Rect2(x_of(window.x), top, x_of(window.y) - x_of(window.x), bottom - top), WINDOW)
	var year := 0.0
	while year <= axis_end + 0.001:
		var x := x_of(year)
		draw_line(Vector2(x, bottom), Vector2(x, bottom + 3), DIM)
		var label := "%d" % roundi(year)
		draw_string(
			font,
			Vector2(x - 8, bottom + 14),
			label,
			HORIZONTAL_ALIGNMENT_CENTER,
			16,
			FONT_SIZE,
			DIM
		)
		year += tick
	draw_line(Vector2(left, bottom), Vector2(size.x - 8.0, bottom), EDGE)
	if limit >= 0.0:
		draw_dashed_line(Vector2(x_of(limit), top - 4), Vector2(x_of(limit), bottom), INK, 1.0, 3.0)
	var step := (bottom - top) / maxf(rows.size(), 1)
	for i in rows.size():
		var y := top + step * (i + 0.5)
		var row: Array = rows[i]
		if kind == "runs":
			_run_row(row, y, font)
		else:
			_range_row(row, y, font)


func _run_row(row: Array, y: float, _font: Font) -> void:
	var colour: Color = ROUTES.get(row[1], ROUTES.none)
	if row[0] == null:
		# never found: an open mark past the end of the axis
		draw_arc(Vector2(x_of(axis_end), y), 2.5, 0.0, TAU, 8, ROUTES.none, 1.0)
		return
	var at := Vector2(x_of(float(row[0])), y)
	if row.size() > 2 and row[2] == "s":
		draw_rect(Rect2(at - Vector2(2.5, 2.5), Vector2(5, 5)), colour)
	else:
		draw_circle(at, 2.6, colour)


func _range_row(row: Array, y: float, font: Font) -> void:
	draw_string(
		font, Vector2(0, y + 4), str(row[0]), HORIZONTAL_ALIGNMENT_LEFT, left - 4, FONT_SIZE, DIM
	)
	var a := x_of(minf(row[1], row[2]))
	var b := x_of(maxf(row[1], row[2]))
	draw_line(Vector2(a, y), Vector2(b, y), INK, 2.0)
	draw_circle(Vector2(a, y), 2.0, INK)
	draw_circle(Vector2(b, y), 2.0, INK)
	if middle >= 0.0:
		draw_line(Vector2(x_of(middle), y - 5), Vector2(x_of(middle), y + 5), ROUTES.accident, 2.0)
