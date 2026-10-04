## The Reports page (IMPLEMENTATION α0.3a, RES-06): the prototypes that run in the cloud, each with
## its question, its answer and its charts, drawn from the numbers the cloud wrote (reports/*.json),
## P4's first; the later cloud prototypes add theirs. Pre-production code (research 00): the app's
## README names the items it is about.
extends Control

signal closed

const Chart := preload("res://reports/chart.gd")
const REPORTS := ["res://reports/p4.json"]
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const GAP := 12
## MND-11's routes in words, for the key under a chart.
const SAYS := {
	"accident": "by accident",
	"experiment": "by experimenting",
	"dream": "from a dream",
	"copying": "by copying",
}

var _page: VBoxContainer


func _ready() -> void:
	var background := ColorRect.new()
	background.color = NIGHT
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(background)
	var margin := MarginContainer.new()
	margin.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	for side in ["left", "top", "right", "bottom"]:
		margin.add_theme_constant_override("margin_" + side, GAP)
	add_child(margin)
	var column := VBoxContainer.new()
	column.add_theme_constant_override("separation", GAP)
	margin.add_child(column)
	var scroll := ScrollContainer.new()
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	scroll.size_flags_vertical = SIZE_EXPAND_FILL
	column.add_child(scroll)
	_page = VBoxContainer.new()
	_page.size_flags_horizontal = SIZE_EXPAND_FILL
	_page.add_theme_constant_override("separation", 8)
	scroll.add_child(_page)
	_text("Reports from the cloud", INK, 22)
	for path: String in REPORTS:
		_p4(JSON.parse_string(FileAccess.get_file_as_string(path)))
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size.y = 48
	back.pressed.connect(func() -> void: closed.emit())
	column.add_child(back)


## P4 Discovery pace (RES-03, TIM-19, RES-16): when flakes and fire first came in each run, how many
## could make flakes two years on, the check on fresh seeds, and how the pace moves with each tuned
## value.
func _p4(r: Dictionary) -> void:
	_text(r.title, FLAME, 20)
	_text(r.question, INK, 15)
	var stone: Dictionary = r.sharp_stone
	var fire: Dictionary = r.fire
	var said := (
		"%s. Flakes came within 5 years in %d of 20 runs, and a world's first fire inside its "
		+ "window in %d of 20, %s."
	)
	_text(
		said % ["Yes" if r.pass else "Not yet", stone.within_5, fire.inside, _before(fire.early)],
		INK,
		15
	)
	_text("First sharp flakes, each run a row (the line: 5 years)", DIM, 13)
	var flakes := _runs(r.runs, "flake", "flake_route", "")
	var c := Chart.new("runs", flakes, 8.0, 1.0)
	c.limit = 5.0
	_page.add_child(c)
	_key(r.runs, "flake_route")
	_text(
		(
			"Two years after each first flake, 3 in 4 adults could make them in %d of %d runs."
			% [stone.spread_ok, stone.within_5]
		),
		INK,
		14
	)
	_text(
		(
			"First fire anywhere in a world of 3 or 4 bands, each world a row "
			+ "(shaded: its window, years 5 to 30; square: by ploughing)"
		),
		DIM,
		13
	)
	c = Chart.new("runs", _runs(r.worlds, "fire", "fire_route", "fire_way"), 60.0, 10.0)
	c.window = Vector2(5.0, 30.0)
	_page.add_child(c)
	_key(r.worlds, "fire_route")
	var fresh: Dictionary = r.sharp_stone_fresh
	var fresh_fire: Dictionary = r.fire_fresh
	_text(
		(
			"On 20 seeds never tuned against: flakes within 5 years in %d, fire in its window in %d, %s."
			% [fresh.within_5, fresh_fire.inside, _before(fresh_fire.early)]
		),
		INK,
		14
	)
	var tuned: Dictionary = (r.sensitivity as Array)[0]["1.0"]
	for step: Array in [["flake", "Flakes", 5.0, 1.0], ["fire", "Fire", 60.0, 10.0]]:
		_text(
			(
				"%s: the median year with each value doubled and halved, over 40 runs (orange: as tuned)"
				% step[1]
			),
			DIM,
			13
		)
		var rows := []
		for k: Dictionary in r.sensitivity:
			if k.has("0.5"):
				rows.append([k.knob, k["2.0"][step[0] + "_median"], k["0.5"][step[0] + "_median"]])
		c = Chart.new("ranges", rows, step[2], step[3])
		c.middle = tuned[step[0] + "_median"]
		_page.add_child(c)
	var edge: Array = r.knife_edge
	_text(
		(
			"No value holds the pace on a knife's edge."
			if edge.is_empty()
			else "On a knife's edge: %s." % ", ".join(edge)
		),
		INK,
		14
	)
	var t: Dictionary = r.tuning
	_text(
		(
			"Tuned discovery factors: flakes %s, drilling %s, ploughing %s."
			% [t.flake, t.drill, t.plough]
		),
		DIM,
		13
	)


## Each run as a chart row, earliest first: its year, how it came, and a square for ploughing.
func _runs(runs: Array, key: String, route: String, way: String) -> Array:
	var rows := []
	for run: Dictionary in runs:
		var mark := "s" if way != "" and run.get(way) == "plough" else "o"
		rows.append([run[key], run[route] if run[route] != null else "none", mark])
	rows.sort_custom(
		func(a: Array, b: Array) -> bool:
			return (a[0] if a[0] != null else INF) < (b[0] if b[0] != null else INF)
	)
	return rows


## How many runs found a step before its window opened, in words.
func _before(n: float) -> String:
	return "none before it" if n == 0.0 else "%d before it" % n


## The routes a chart shows, in their colours.
func _key(runs: Array, route: String) -> void:
	var label := RichTextLabel.new()
	label.bbcode_enabled = true
	label.fit_content = true
	label.add_theme_font_size_override("normal_font_size", 13)
	var seen := []
	for run: Dictionary in runs:
		if run[route] != null and not run[route] in seen:
			seen.append(run[route])
	var parts := PackedStringArray()
	for name: String in SAYS:
		if name in seen:
			var colour: Color = Chart.ROUTES[name]
			parts.append("[color=#%s]●[/color] %s" % [colour.to_html(false), SAYS[name]])
	label.text = "   ".join(parts)
	_page.add_child(label)


func _text(text: String, colour: Color, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_color_override("font_color", colour)
	label.add_theme_font_size_override("font_size", font_size)
	_page.add_child(label)
	return label
