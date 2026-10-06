## The cloud's last scene reports (A17, RES-06): each scene's verdict on its rule, how it was set
## and what it took, each measure's range over its runs with a chart drawn to scale, each run with
## its oddities, and the world it brought, which opens here marked as a test's world (PLT-05).
## Implements RES-06, RES-13 and PLT-05.
extends VBoxContainer

const TEXT := Color("#efe6d8")
const QUIET := Color("#a89f95")
const HEAD := Color("#e8c25a")
const GOOD := Color("#9fd38a")
const FAIL := Color("#ef7b6b")

## Where the reports are read from and the worlds kept; a test sets its own before the page opens.
var folder := Reports.FOLDER
var root := Worlds.ROOT
## The reports as read, every line the page shows, in order, its charts and the status line's
## words, for the tests.
var reports: Array[Dictionary] = []
var shown := PackedStringArray()
var charts: Array[RangeChart] = []
var status := ""

var _list: VBoxContainer
var _status: Label
# the scenes whose runs are all shown, rather than only the odd ones
var _all_runs := {}


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	_status = Label.new()
	_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_status.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_status.add_theme_font_size_override("font_size", 16)
	_status.add_theme_color_override("font_color", TEXT)
	add_child(_status)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_list.add_theme_constant_override("separation", 4)
	scroll.add_child(_list)
	reports = Reports.read_all(folder)
	refresh()


## Shows the reports again.
func refresh() -> void:
	for child in _list.get_children():
		child.queue_free()
	shown.clear()
	charts.clear()
	_status.text = status
	_status.visible = status != ""
	if reports.is_empty():
		_line("No reports yet: the cloud's scenes leave theirs in each build.", 16, QUIET)
	for r: Dictionary in reports:
		_show(r)


## Shows every run of a scene, or only its odd ones.
func show_all_runs(scene: String, all: bool) -> void:
	_all_runs[scene] = all
	refresh()


## Opens the world a report brought, on the Crowd page: imported the first time, and found again
## after. Its id, or nothing when there is none or it was refused.
func open_world(r: Dictionary) -> String:
	var found := Reports.world_of(folder, r)
	if found.is_empty():
		status = "This report brought no world"
		refresh()
		return ""
	var worlds := Worlds.at(root)
	var copy_name := "%s %d (imported)" % [r["scene"], int(found["index"]) + 1]
	var id := ""
	for w: Dictionary in worlds.list():
		if w["name"] == copy_name:
			id = w["id"]
	if id == "":
		var taken := worlds.import_begin()
		taken = taken and worlds.import_feed(FileAccess.get_file_as_bytes(found["path"]))
		var result := worlds.import_finish()
		if not taken or not result.has("id"):
			status = "The world was refused: %s" % result.get("why", "it could not be read")
			refresh()
			return ""
		id = result["id"]
	worlds.set_current(id)
	status = "Opened %s, a test's world" % copy_name
	var shell := get_parent()
	while shell != null and not shell.has_method("open_page"):
		shell = shell.get_parent()
	if shell != null:
		shell.open_page("Crowd")
	else:
		refresh()
	return id


func _show(r: Dictionary) -> void:
	_gap()
	_line(r["scene"], 22, HEAD)
	_line(r["about"], 15, QUIET)
	_line(Reports.verdict_words(r), 18, GOOD if r["passed"] else FAIL)
	_line("Its rule: %s" % r["rule"], 15, TEXT)
	_line(Reports.setup_words(r), 15, QUIET)
	_line(Reports.time_words(r), 15, QUIET)
	_line("It checks %s" % ", ".join(r["checks"]), 15, QUIET)
	_line(Reports.oddity_words(r), 15, TEXT if int(r["oddities"]) == 0 else FAIL)
	for m: Dictionary in r["ranges"]:
		_gap()
		_line(m["measure"], 17, TEXT)
		_line(m["about"], 13, QUIET)
		_line(Reports.range_words(m), 15, TEXT)
		var chart := RangeChart.new()
		var on_rule: bool = m["measure"] == r["measure"]
		chart.setup(
			Reports.values_of(r, m["measure"]),
			m["expected"] if m["expected"] != null else [],
			r["at_least"] if on_rule else null,
			r["at_most"] if on_rule else null
		)
		chart.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		_list.add_child(chart)
		charts.append(chart)
	_gap()
	var all: bool = _all_runs.get(r["scene"], false)
	var runs: Array = r["each"]
	_line("Runs", 17, TEXT)
	for run: Dictionary in runs:
		var odd: Array = run["oddities"]
		if all or not odd.is_empty():
			_line(Reports.run_words(run), 14, TEXT)
			for oddity: String in odd:
				_line("  %s" % oddity, 14, FAIL)
	var scene: String = r["scene"]
	_button(
		"Show only the odd runs" if all else "Show all %d runs" % runs.size(),
		show_all_runs.bind(scene, not all)
	)
	var found := Reports.world_of(folder, r)
	if not found.is_empty():
		_button("Open run %d's world" % (int(found["index"]) + 1), open_world.bind(r))


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
	gap.custom_minimum_size = Vector2(0, 8)
	_list.add_child(gap)


func _button(text: String, pressed: Callable) -> void:
	var b := Button.new()
	b.text = text
	b.custom_minimum_size = Vector2(0, 48)
	b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	b.pressed.connect(pressed)
	_list.add_child(b)
	shown.append(text)
