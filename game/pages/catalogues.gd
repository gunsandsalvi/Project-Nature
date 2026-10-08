## The catalogues (A3.6): what the phone loaded from the build's copy of data/, source by source and
## kind by kind, with each entry's values as the simulation holds them, in base units. Implements
## MAT-13 and MAT-14: each source with its version and its rules, world and look digests.
extends VBoxContainer

const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const HEAD := Palette.HEAD
const FAIL := Palette.FAIL

## What loading found, as KdWorld.load_catalogue gives it.
var loaded: Dictionary = {}
## Every line the page shows, in order, for the tests.
var shown := PackedStringArray()

var _list: VBoxContainer
var _kind: OptionButton
var _entry: OptionButton
var _detail: Label
var _fields: Label


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
	_browser()


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


func _line(text: String, _font_size: int, _colour: Color) -> void:
	# Keep the complete inspectable data independent of which entry is selected.
	shown.append(text)


func _gap() -> void:
	pass


func _label(text: String, font_size := 16) -> Label:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	label.add_theme_font_size_override("font_size", font_size)
	_list.add_child(label)
	return label


func _browser() -> void:
	_label("Catalogues", 22)
	_label(shown[0])
	var sources := Button.new()
	sources.text = "Source details"
	sources.custom_minimum_size.y = 48
	_list.add_child(sources)
	var provenance := _label("")
	provenance.visible = false
	var lines := PackedStringArray()
	for source: Dictionary in loaded.sources:
		lines.append(
			(
				"%s · version %d\n%s\nRules %s · world %s · look %s"
				% [
					source.id,
					source.version,
					source.about,
					str(source.rules).left(8),
					str(source.world).left(8),
					str(source.look).left(8)
				]
			)
		)
	provenance.text = "\n\n".join(lines)
	sources.pressed.connect(func() -> void: provenance.visible = not provenance.visible)
	_label("Choose a catalogue")
	_kind = OptionButton.new()
	_kind.fit_to_longest_item = false
	_kind.clip_text = true
	_kind.custom_minimum_size.y = 48
	_list.add_child(_kind)
	for kind: Dictionary in loaded.kinds:
		_kind.add_item("%s (%d)" % [kind.folder, kind.entries.size()])
	_kind.item_selected.connect(_select_kind)
	_entry = OptionButton.new()
	_entry.fit_to_longest_item = false
	_entry.clip_text = true
	_entry.custom_minimum_size.y = 48
	_list.add_child(_entry)
	_entry.item_selected.connect(_select_entry)
	_detail = _label("")
	var fields_button := Button.new()
	fields_button.text = "Fields"
	fields_button.custom_minimum_size.y = 48
	_list.add_child(fields_button)
	_fields = _label("")
	_fields.hide()
	fields_button.pressed.connect(func() -> void: _fields.visible = not _fields.visible)
	if not loaded.kinds.is_empty():
		_select_kind(0)


func _select_kind(index: int) -> void:
	_entry.clear()
	for entry: Dictionary in loaded.kinds[index].entries:
		_entry.add_item(entry.name)
	if _entry.item_count > 0:
		_select_entry(0)
	else:
		_detail.text = "This catalogue is empty."


func _select_entry(index: int) -> void:
	var kind: Dictionary = loaded.kinds[_kind.selected]
	var entry: Dictionary = kind.entries[index]
	_detail.text = "%s\n\n%s" % [kind.about, entry.file]
	_fields.text = str(entry.values).strip_edges()
	_fields.hide()
