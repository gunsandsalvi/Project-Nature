## A measure over a scene's runs, drawn to scale (A17, RES-06): each run a dot, runs that fall
## together stacked, the range the measure is expected in shaded, and the pass rule's bound as a
## line, each labelled with its number, so "in 18 of 20 worlds" shows at a glance. Implements
## RES-06.
class_name RangeChart
extends Control

const DOT := Color("#efe6d8")
const AXIS := Color("#a89f95")
const BAND := Color(0.91, 0.76, 0.35, 0.22)
const BAND_EDGE := Color("#e8c25a")
const BOUND := Color("#9fd38a")
## A dot's radius and the step between stacked dots, in pixels; the room kept for the labels.
const RADIUS := 3.5
const STEP := 8.0
const LABELS := 20.0
const SIDE := 12.0

## Each run's value, the range expected ([from, to], or empty) and the rule's bounds on this
## measure (null where it sets none).
var values: Array[int] = []
var expected: Array = []
var at_least: Variant = null
var at_most: Variant = null


## Sets what the chart shows, and sizes it to its tallest stack.
func setup(run_values: Array[int], expected_range: Array, least: Variant, most: Variant) -> void:
	values = run_values
	expected = expected_range
	at_least = least
	at_most = most
	var lo_hi := axis_of(values, expected, at_least, at_most)
	var tallest := 1
	for dot: Vector2 in dots(values, lo_hi.x, lo_hi.y, 400.0):
		tallest = maxi(tallest, int(dot.y) + 1)
	custom_minimum_size = Vector2(0, LABELS + 16.0 + tallest * STEP)
	queue_redraw()


## The axis's ends: from the least to the most of the values, the expected range and the rule's
## bounds, widened by a tenth either side so no mark sits on an edge.
static func axis_of(
	run_values: Array[int], expected_range: Array, least: Variant, most: Variant
) -> Vector2i:
	var all: Array[int] = run_values.duplicate()
	for x: Variant in expected_range + [least, most]:
		if x != null:
			all.append(int(x))
	if all.is_empty():
		return Vector2i(0, 1)
	var lo: int = all.min()
	var hi: int = all.max()
	var room := maxi(1, ceili((hi - lo) / 10.0))
	return Vector2i(lo - room, hi + room)


## Where a value falls along a width.
static func x_of(value: int, lo: int, hi: int, width: float) -> float:
	return SIDE + (width - 2.0 * SIDE) * float(value - lo) / float(maxi(1, hi - lo))


## Each run's dot: x along the width, and y its place in its stack, counted from the axis, so dots
## closer than a dot's width stack rather than hide each other.
static func dots(run_values: Array[int], lo: int, hi: int, width: float) -> Array[Vector2]:
	var out: Array[Vector2] = []
	var stacks := {}
	var sorted := run_values.duplicate()
	sorted.sort()
	for v: int in sorted:
		var x := x_of(v, lo, hi, width)
		var column := roundi(x / (2.0 * RADIUS + 1.0))
		var row: int = stacks.get(column, 0)
		stacks[column] = row + 1
		out.append(Vector2(column * (2.0 * RADIUS + 1.0), row))
	return out


func _draw() -> void:
	var lo_hi := axis_of(values, expected, at_least, at_most)
	var lo := lo_hi.x
	var hi := lo_hi.y
	var w := size.x
	var base := size.y - LABELS
	var font := ThemeDB.fallback_font
	var font_size := 13
	if expected.size() == 2:
		var x0 := x_of(int(expected[0]), lo, hi, w)
		var x1 := x_of(int(expected[1]), lo, hi, w)
		draw_rect(Rect2(x0, 4.0, maxf(1.0, x1 - x0), base - 4.0), BAND)
		draw_line(Vector2(x0, 4.0), Vector2(x0, base), BAND_EDGE, 1.0)
		draw_line(Vector2(x1, 4.0), Vector2(x1, base), BAND_EDGE, 1.0)
	for bound: Variant in [at_least, at_most]:
		if bound != null:
			var x := x_of(int(bound), lo, hi, w)
			draw_dashed_line(Vector2(x, 0.0), Vector2(x, base), BOUND, 2.0, 4.0)
	draw_line(Vector2(SIDE, base), Vector2(w - SIDE, base), AXIS, 1.0)
	for dot: Vector2 in dots(values, lo, hi, w):
		draw_circle(Vector2(dot.x, base - RADIUS - 2.0 - dot.y * STEP), RADIUS, DOT)
	# the numbers that mean something, under the axis: the rule's bound, the expected range's ends,
	# and the lowest and highest runs; each where it falls, and left out where it would overlap one
	# already placed
	var y := size.y - 4.0
	var placed: Array[Vector2] = []
	for label: Array in labels():
		var words: String = label[1]
		var wide := font.get_string_size(words, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x
		var x := clampf(x_of(label[0], lo, hi, w) - wide / 2.0, 0.0, maxf(0.0, w - wide))
		var free := true
		for span: Vector2 in placed:
			free = free and (x + wide + 6.0 <= span.x or x >= span.y + 6.0)
		if free:
			placed.append(Vector2(x, x + wide))
			draw_string(
				font, Vector2(x, y), words, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, label[2]
			)


## The labels under the axis, most telling first: [value, words, colour].
func labels() -> Array[Array]:
	var out: Array[Array] = []
	if at_least != null:
		out.append([int(at_least), "at least %s" % Worlds.count_words(int(at_least)), BOUND])
	if at_most != null:
		out.append([int(at_most), "at most %s" % Worlds.count_words(int(at_most)), BOUND])
	for end: Variant in expected:
		out.append([int(end), Worlds.count_words(int(end)), BAND_EDGE])
	if not values.is_empty():
		out.append([values.min(), Worlds.count_words(values.min()), DOT])
		out.append([values.max(), Worlds.count_words(values.max()), DOT])
	return out
