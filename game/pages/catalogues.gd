## The catalogues (A3.6): what the phone loaded from the build's copy of data/, source by source and
## kind by kind, with each entry's values as the simulation holds them, in base units. Implements
## MAT-13 and MAT-14: each source with its version and its rules, world and look digests.
extends VBoxContainer

const TEXT := Color("#efe6d8")
const QUIET := Color("#a89f95")
const HEAD := Color("#e8c25a")
const FAIL := Color("#ef7b6b")

## What loading found, as KdWorld.load_catalogue gives it.
var loaded: Dictionary = {}
## Every line the page shows, in order, for the tests.
var shown := PackedStringArray()

var _list: VBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_list.add_theme_constant_override("separation", 4)
	scroll.add_child(_list)
	loaded = GameData.load_into(KdWorld.new())
	_show()


func _show() -> void:
	_line(
		(
			"%d sources, %d kinds, %d entries, read from %d files in %.1f ms"
			% [
				(loaded["sources"] as Array).size(),
				(loaded["kinds"] as Array).size(),
				GameData.entry_count(loaded),
				loaded["files"],
				int(loaded["microseconds"]) / 1000.0,
			]
		),
		16,
		TEXT
	)
	for problem: String in loaded["problems"]:
		_line(problem, 14, FAIL)
	_gap()
	_line("Sources", 20, HEAD)
	for s: Dictionary in loaded["sources"]:
		_line("%s, version %d" % [s["id"], s["version"]], 17, TEXT)
		_line(str(s["about"]), 14, QUIET)
		_line(
			(
				"rules %s · world %s · look %s"
				% [str(s["rules"]).left(8), str(s["world"]).left(8), str(s["look"]).left(8)]
			),
			13,
			QUIET
		)
	_line("World-making version %d" % loaded["world_making_version"], 13, QUIET)
	for kind: Dictionary in loaded["kinds"]:
		_gap()
		var entries: Array = kind["entries"]
		var count := "1 entry" if entries.size() == 1 else "%d entries" % entries.size()
		_line("%s (%s)" % [kind["folder"], count], 20, HEAD)
		_line(str(kind["about"]), 14, QUIET)
		for entry: Dictionary in entries:
			_line("%s, from %s" % [entry["name"], entry["file"]], 16, TEXT)
			_line(str(entry["values"]).strip_edges(), 13, QUIET)


func _line(text: String, font_size: int, colour: Color) -> void:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	_list.add_child(label)
	shown.append(text)


func _gap() -> void:
	var gap := Control.new()
	gap.custom_minimum_size = Vector2(0, 10)
	_list.add_child(gap)
