## The Reports page (IMPLEMENTATION α0.3a, RES-06): the prototypes that run in the cloud, each with
## its question, its answer and its charts, drawn from the numbers the cloud wrote (reports/*.json):
## P4's, then P9's. Pre-production code (research 00) for RES-06;
## each prototype's README names the items its report is about.
extends Control

signal closed

const Chart := preload("res://reports/chart.gd")
const Lines := preload("res://reports/lines.gd")
const REPORTS := ["res://reports/p4.json", "res://reports/p9.json"]
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
		var report: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(path))
		if String(report.title).begins_with("P4"):
			_p4(report)
		else:
			_p9(report)
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size.y = 48
	back.pressed.connect(func() -> void: closed.emit())
	column.add_child(back)


## P4 Discovery pace (RES-03, TIM-19, RES-16): when flakes and fire first came in each run, how many
## could make flakes soon after, the check on fresh seeds, and how the pace moves with each tuned
## value. The sharp-stone test's bars come from the report, as the run had them.
func _p4(r: Dictionary) -> void:
	_text(r.title, FLAME, 20)
	_text(r.question, INK, 15)
	var stone: Dictionary = r.sharp_stone
	var fire: Dictionary = r.fire
	# TIM-19's windows as the run had them, in years from the start; as dates, Year 1 begins at
	# the start
	var windows: Dictionary = r.windows
	var flake_window := Vector2(windows.flake[0], windows.flake[1])
	var fire_window := Vector2(windows.fire[0], windows.fire[1])
	var bars: Dictionary = r.bars
	var said := (
		"%s. Flakes came within %d years, their window, in %d of 20 runs (within %d, the "
		+ "sharp-stone test's bar, in %d), and a world's first fire inside its window in %d of 20, %s."
	)
	_text(
		(
			said
			% [
				"Yes" if r.pass else "Not yet",
				flake_window.y,
				stone.window.inside,
				bars.flake,
				stone.within,
				fire.inside,
				_before(fire.early)
			]
		),
		INK,
		15
	)
	_text(
		(
			(
				"First sharp flakes, years from the start, each run a row (shaded: their window; "
				+ "the line: %d years)"
			)
			% bars.flake
		),
		DIM,
		13
	)
	var flakes := _runs(r.runs, "flake", "flake_route", "")
	var c := Chart.new("runs", flakes, bars.no_flake + 1.0, 1.0)
	c.window = flake_window
	c.limit = bars.flake
	_page.add_child(c)
	_key(r.runs, "flake_route")
	_text(
		(
			"%s after each first flake, 3 in 4 adults could make them in %d of %d runs."
			% [
				"A year" if int(bars.spread_after) == 1 else "%d years" % bars.spread_after,
				stone.spread_ok,
				stone.within
			]
		),
		INK,
		14
	)
	_text(
		(
			"First fire anywhere in a world of 3 or 4 bands, years from the start, each world a row "
			+ (
				"(shaded: its window, Years %d to %d, from %d years in; square: by ploughing)"
				% [fire_window.x + 1, fire_window.y, fire_window.x]
			)
		),
		DIM,
		13
	)
	var fire_end := fire_window.y * 2.0
	c = Chart.new("runs", _runs(r.worlds, "fire", "fire_route", "fire_way"), fire_end, 5.0)
	c.window = fire_window
	_page.add_child(c)
	_key(r.worlds, "fire_route")
	var fresh: Dictionary = r.sharp_stone_fresh
	var fresh_fire: Dictionary = r.fire_fresh
	_text(
		(
			"On 20 seeds never tuned against: flakes within %d years in %d, fire in its window in %d, %s."
			% [bars.flake, fresh.within, fresh_fire.inside, _before(fresh_fire.early)]
		),
		INK,
		14
	)
	var tuned: Dictionary = (r.sensitivity as Array)[0]["1.0"]
	for step: Array in [
		["flake", "Flakes", float(bars.no_flake), 1.0, "40 runs", flake_window],
		["fire", "Fire", fire_end, 5.0, "40 worlds", fire_window]
	]:
		_text(
			(
				(
					"%s: the median year with each value doubled and halved, over %s (orange: as "
					+ "tuned; shaded: the window). The knife's edge is judged on counts at a "
					+ "quarter's change, below."
				)
				% [step[1], step[4]]
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
		c.window = step[5]
		_page.add_child(c)
	var edge: Array = r.knife_edge
	_text(
		(
			(
				"No value holds the pace on a knife's edge: changed by a quarter either way, "
				+ "each keeps both steps' rules."
			)
			if edge.is_empty()
			else (
				"On a knife's edge, breaking a rule when changed by a quarter: %s."
				% ", ".join(edge)
			)
		),
		INK,
		14
	)
	var levers: Array = r.get("levers", [])
	if not levers.is_empty():
		_text(
			"Strong levers, breaking a rule only when halved or doubled: %s." % ", ".join(levers),
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


## P9 Ecology (WLD-18, WLD-30, WLD-31, WLD-32): each species' total in each world over a hundred
## years with nobody in it, as a share of its settled total, against the band from half to twice;
## the plants' the same; and the big plant eaters for each hunter.
func _p9(r: Dictionary) -> void:
	_text(r.title, FLAME, 20)
	_text(r.question, INK, 15)
	var runs: Array = r.runs
	var passed := 0
	var fewest := INF
	var most := 0.0
	for run: Dictionary in runs:
		passed += 1 if run.pass else 0
		fewest = minf(fewest, run.prey_per_hunter.mean)
		most = maxf(most, run.prey_per_hunter.mean)
	var said := (
		"%s. In %d of %d worlds every species stayed within half and twice its settled total for "
		+ "%d years and in every biome it lived in, and the cover held; there were %d to %d big "
		+ "plant eaters for each hunter, where %d to %d are asked."
	)
	_text(
		(
			said
			% [
				"Yes" if r.pass else "Not yet",
				passed,
				runs.size(),
				r.years,
				roundi(fewest),
				roundi(most),
				r.ratio[0],
				r.ratio[1]
			]
		),
		INK,
		15
	)
	_text(
		(
			(
				"Each world is %d by %d cells of about %d km, made by P7, run %d years into its "
				+ "present-day state and settled %d, then %d more. Each line is a world's total as a "
				+ "share of its settled one; shaded, half to twice."
			)
			% [r.cells[0], r.cells[1], roundi(r.km), r.made, r.settle, r.years]
		),
		DIM,
		13
	)
	# round each world's start region, where a hard year shows that the world's totals hide
	var low_here := INF
	var high_here := 0.0
	for run: Dictionary in runs:
		low_here = minf(low_here, run.local.low)
		high_here = maxf(high_here, run.local.high)
	_text(
		(
			(
				"Round each world's start region, about 64 km across, numbers swung further, from "
				+ "%.2f to %.2f of their settled level: below, the three start regions that swung "
				+ "most, each species a line."
			)
			% [low_here, high_here]
		),
		INK,
		14
	)
	var species: Array = r.species
	var swung := runs.duplicate()
	swung.sort_custom(
		func(a: Dictionary, b: Dictionary) -> bool:
			return a.local.high / maxf(a.local.low, 0.01) > b.local.high / maxf(b.local.low, 0.01)
	)
	for run: Dictionary in swung.slice(0, 3):
		var lines := []
		var names := PackedStringArray()
		for row: Dictionary in run.local.species:
			lines.append(row.years)
			names.append(species[int(row.kind)].name)
		var chart := Lines.new(lines, "World %d round its start" % run.seed)
		chart.years = r.years
		_page.add_child(chart)
		_text(", ".join(names), DIM, 12)
	_text("Each species over the whole world:", INK, 14)
	for k in species.size():
		var kind: Dictionary = species[k]
		var lines := []
		var low := INF
		var high := 0.0
		for run: Dictionary in runs:
			var row: Dictionary = run.species[k]
			if (row.years as Array).is_empty():
				continue
			lines.append(row.years)
			low = minf(low, row.low)
			high = maxf(high, row.high)
		if lines.is_empty():
			continue
		var title := (
			"%s, %s kg%s: %d worlds, %.2f to %.2f"
			% [kind.name, kind.kg, ", a hunter" if kind.hunter else "", lines.size(), low, high]
		)
		var chart := Lines.new(lines, title)
		chart.years = r.years
		_page.add_child(chart)
	var plants: Array = r.plants
	for p in plants.size():
		var lines := []
		var low := INF
		var high := 0.0
		for run: Dictionary in runs:
			var row: Dictionary = run.plants[p]
			lines.append(row.years)
			low = minf(low, row.low)
			high = maxf(high, row.high)
		var said_of := {
			"grass": "grass and herbs standing at midsummer",
			"browse": "browse: bushes' and young trees' leaves",
			"mast": "mast: fruit, nuts and roots, a crop that fails and floods by the year",
			"trees": "trees' crowns"
		}
		var chart := Lines.new(lines, "%s: %.2f to %.2f" % [said_of[plants[p]], low, high])
		chart.years = r.years
		_page.add_child(chart)


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
