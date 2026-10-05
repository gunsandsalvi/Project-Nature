## A chart of P9's report (IMPLEMENTATION α0.6a, WLD-18): a total over the years in each world, one
## line a world, as a share of its settled total on a scale of halvings and doublings, the band it
## must stay inside shaded; or any other number over the years, against its own band. Drawn from the
## numbers so it stays sharp on the phone. Pre-production code (research 00).
extends Control

const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const EDGE := Color("3a3346")
const BAND := Color(0.56, 0.8, 0.48, 0.13)
const LINE := Color(0.96, 0.64, 0.24, 0.55)
const PAD := Vector2(34, 16)
const FONT_SIZE := 12

## Each line: its value for each year from 0.
var lines := []
## The values the chart spans and the band shaded, and whether the scale is of halvings.
var low := 0.25
var high := 4.0
var band := Vector2(0.5, 2.0)
var halvings := true
var years := 100
## The labels down the side, each a value.
var marks := [0.5, 1.0, 2.0]
var title := ""


func _init(chart_lines: Array, chart_title: String) -> void:
	lines = chart_lines
	title = chart_title
	custom_minimum_size = Vector2(0, 118)


func _y_of(v: float) -> float:
	var top := PAD.y + 4.0
	var bottom := size.y - PAD.y
	var t := 0.0
	if halvings:
		t = (log(maxf(v, low)) - log(low)) / (log(high) - log(low))
	else:
		t = (v - low) / (high - low)
	return bottom - clampf(t, 0.0, 1.0) * (bottom - top)


func _x_of(year: float) -> float:
	return PAD.x + clampf(year / years, 0.0, 1.0) * (size.x - PAD.x - 8.0)


func _draw() -> void:
	var font := get_theme_default_font()
	var bottom := size.y - PAD.y
	draw_string(font, Vector2(PAD.x, 12), title, HORIZONTAL_ALIGNMENT_LEFT, -1, FONT_SIZE, INK)
	var a := _y_of(band.y)
	var b := _y_of(band.x)
	draw_rect(Rect2(_x_of(0), a, _x_of(years) - _x_of(0), b - a), BAND)
	for m: float in marks:
		var y := _y_of(m)
		draw_line(Vector2(PAD.x - 3, y), Vector2(_x_of(years), y), EDGE)
		var label := ("%.1f" % m) if m < 10.0 else ("%d" % m)
		draw_string(font, Vector2(0, y + 4), label, HORIZONTAL_ALIGNMENT_RIGHT, PAD.x - 6, 10, DIM)
	for year in range(0, years + 1, 25):
		var x := _x_of(year)
		draw_line(Vector2(x, bottom), Vector2(x, bottom + 3), DIM)
		draw_string(
			font,
			Vector2(x - 12, bottom + 13),
			"%d" % year,
			HORIZONTAL_ALIGNMENT_CENTER,
			24,
			10,
			DIM
		)
	for line: Array in lines:
		if line.size() < 2:
			continue
		var points := PackedVector2Array()
		for i in line.size():
			points.append(Vector2(_x_of(i), _y_of(float(line[i]))))
		draw_polyline(points, LINE, 1.0)
