## A chart of P11's report (IMPLEMENTATION α0.6c, TIM-02): one watch from the globe over its
## real minutes, each slowdown a block as long as it lasted and each moment kept in the list a
## tick, so the budget's rhythm shows. Drawn from the numbers so it stays sharp on the phone.
## Pre-production code (research 00).
extends Control

const INK := Color("ebe5da")
const DIM := Color(0.64, 0.61, 0.66, 0.6)
const EDGE := Color("3a3346")
const SLOW := Color("f6a33c")
const PAD := Vector2(34, 16)
const FONT_SIZE := 12

## Each slowdown: [its minute, what it was for]; each listed moment: its minute.
var slowed := []
var listed := []
var minutes := 20.0
## How long each slowdown lasted, in minutes.
var length := 10.0 / 60.0
var title := ""


func _init(watch: Dictionary, chart_title: String) -> void:
	slowed = watch.slowed
	listed = watch.listed
	minutes = maxf(float(watch.minutes), 1.0)
	title = chart_title
	custom_minimum_size = Vector2(0, 70)


## Where a minute falls across the chart.
func x_of(minute: float) -> float:
	return PAD.x + clampf(minute / minutes, 0.0, 1.0) * (size.x - PAD.x - 8.0)


func _draw() -> void:
	var font := get_theme_default_font()
	draw_string(font, Vector2(PAD.x, 12), title, HORIZONTAL_ALIGNMENT_LEFT, -1, FONT_SIZE, INK)
	var bottom := size.y - PAD.y
	var y := bottom - 16.0
	for m: float in listed:
		draw_line(Vector2(x_of(m), y - 7), Vector2(x_of(m), y + 7), DIM)
	for s: Array in slowed:
		var a := x_of(float(s[0]))
		var w := maxf(x_of(float(s[0]) + length) - a, 3.0)
		draw_rect(Rect2(a, y - 10, w, 20), SLOW)
	draw_line(Vector2(PAD.x, bottom), Vector2(size.x - 8.0, bottom), EDGE)
	var m := 0.0
	while m <= minutes + 0.001:
		var x := x_of(m)
		draw_line(Vector2(x, bottom), Vector2(x, bottom + 3), INK)
		draw_string(
			font, Vector2(x - 8, bottom + 14), "%d" % m, HORIZONTAL_ALIGNMENT_CENTER, 16, 10, INK
		)
		m += 5.0
