## The worlds on the phone (TIM-08, PLT-08, PLT-10): each with its name, the moment it was saved
## at, when, and its size by part; make a new one, open one on the Crowd page, rename it, export
## it to a file or import one as a copy through Android's file picker, and delete one with a second
## tap. The free space shows, with a warning before the phone is full. Implements TIM-08, PLT-08
## and PLT-10.
extends VBoxContainer

const TEXT := Color("#efe6d8")
const QUIET := Color("#a89f95")
const WARN := Color("#e8c25a")
## How much of a file goes through in a frame while exporting or importing, so the screen stays
## smooth with a world of any size.
const PER_FRAME := 4 * 1024 * 1024
const PIECE := 1024 * 1024

## The folder the worlds are kept in; a test sets its own before the page opens.
var root := Worlds.ROOT
## How many camps a new world has: 0 for the tuning's; a test makes small ones.
var new_camps := 0
## The worlds as the page lists them (KdWorlds.list), and the status line's words.
var listed: Array = []
var status := ""
var worlds: KdWorlds

var _space: Label
var _status: Label
var _list: VBoxContainer
# the world whose Delete was tapped once, the one being renamed, and the export or import under way
var _armed := ""
var _renaming := ""
var _job := {}
var _warn_below := 1024


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	worlds = Worlds.at(root)
	var tuning := KdWorld.new()
	GameData.load_into(tuning)
	_warn_below = int(tuning.entry("tuning/saves", "base:saves").get("warn_below", 1024))
	_space = _label(16, QUIET)
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", 10)
	add_child(row)
	row.add_child(_button("New world", func() -> void: make_world()))
	row.add_child(_button("Import a world", pick_import))
	_status = _label(16, TEXT)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_list.add_theme_constant_override("separation", 14)
	scroll.add_child(_list)
	set_process(false)
	refresh()


## Lists the worlds again, and the free space.
func refresh() -> void:
	listed = worlds.list()
	var free := worlds.free_space()
	var warning := Worlds.space_warning(free / 1048576 if free >= 0 else -1, _warn_below)
	_space.text = warning if warning != "" else "Free space: %s" % Worlds.size_words(free)
	_space.add_theme_color_override("font_color", WARN if warning != "" else QUIET)
	_status.text = status
	for child in _list.get_children():
		child.queue_free()
	var current := Worlds.current_id(worlds)
	for w: Dictionary in listed:
		_list.add_child(_row(w, w["id"] == current))
	if listed.is_empty():
		_list.add_child(_text("No worlds yet: make one, or open Crowd.", 16, QUIET))


## Makes a new world, named after how many there are: its id.
func make_world() -> String:
	if _waiting():
		return ""
	var id := worlds.make("World %d" % (listed.size() + 1), randi(), new_camps)
	status = "Made %s: open it to begin" % id if id != "" else "The new world could not be made"
	refresh()
	return id


## Chooses a world for the Crowd page, and goes there when the page is in the app.
func open_world(id: String) -> void:
	if _waiting():
		return
	worlds.set_current(id)
	var shell := get_parent()
	while shell != null and not shell.has_method("open_page"):
		shell = shell.get_parent()
	if shell != null:
		shell.open_page("Crowd")
	else:
		refresh()


## Gives a world a new name.
func rename_world(id: String, new_name: String) -> void:
	if _waiting():
		return
	_renaming = ""
	if new_name.strip_edges() == "":
		status = "A world needs a name"
	elif worlds.rename(id, new_name.strip_edges()):
		status = "Renamed to %s" % new_name.strip_edges()
	else:
		status = "It could not be renamed"
	refresh()


## Delete, tapped: the first tap asks, the second deletes. Whether it deleted.
func tap_delete(id: String) -> bool:
	if _waiting():
		return false
	if _armed != id:
		_armed = id
		status = "Tap Delete again to delete %s for good" % _name_of(id)
		refresh()
		return false
	_armed = ""
	var gone := _name_of(id)
	status = "Deleted %s" % gone if worlds.remove(id) else "%s could not be deleted" % gone
	refresh()
	return true


## Export, tapped: Android's file picker asks where the file goes.
func pick_export(id: String) -> void:
	var file_name := "%s.kindling" % _name_of(id).validate_filename()
	DisplayServer.file_dialog_show(
		"Export the world",
		"",
		file_name,
		false,
		DisplayServer.FILE_DIALOG_MODE_SAVE_FILE,
		PackedStringArray(["*.kindling"]),
		func(ok: bool, paths: PackedStringArray, _filter: int) -> void:
			if ok and not paths.is_empty():
				export_to(id, paths[0])
	)


## Import, tapped: Android's file picker asks for the file.
func pick_import() -> void:
	DisplayServer.file_dialog_show(
		"Import a world",
		"",
		"",
		false,
		DisplayServer.FILE_DIALOG_MODE_OPEN_FILE,
		PackedStringArray(["*.kindling"]),
		func(ok: bool, paths: PackedStringArray, _filter: int) -> void:
			if ok and not paths.is_empty():
				import_from(paths[0])
	)


## Writes a world out to a .kindling file, a few megabytes a frame: whether it began.
func export_to(id: String, path: String) -> bool:
	if busy():
		return false
	var total := worlds.export_begin(id)
	var file := FileAccess.open(path, FileAccess.WRITE)
	if total < 0 or file == null:
		status = "The world could not be exported there"
		refresh()
		return false
	_job = {"export": true, "id": id, "file": file, "done": 0, "total": total}
	set_process(true)
	return true


## Reads a .kindling file into a new world, a few megabytes a frame, checking it as it comes:
## whether it began.
func import_from(path: String) -> bool:
	if busy():
		return false
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		status = "The file could not be read"
		refresh()
		return false
	if not worlds.import_begin():
		status = "There is no room to import a world"
		refresh()
		return false
	_job = {"export": false, "file": file, "done": 0, "total": file.get_length()}
	set_process(true)
	return true


## Whether an export or import is under way.
func busy() -> bool:
	return not _job.is_empty()


## While a file moves, the worlds stay as they are: a tap waits, and says so.
func _waiting() -> bool:
	if busy():
		_status.text = status + ". Wait for it to end."
	return busy()


func _process(_delta: float) -> void:
	var file: FileAccess = _job["file"]
	var moved := 0
	var ended := false
	while moved < PER_FRAME and not ended:
		if _job["export"]:
			var piece := worlds.export_next(PIECE)
			ended = piece.is_empty()
			file.store_buffer(piece)
			moved += piece.size()
		else:
			var piece := file.get_buffer(PIECE)
			ended = piece.is_empty() or not worlds.import_feed(piece)
			moved += piece.size()
	_job["done"] += moved
	status = (
		"%s: %s of %s"
		% [
			"Exporting" if _job["export"] else "Importing",
			Worlds.size_words(_job["done"]),
			Worlds.size_words(_job["total"]),
		]
	)
	_status.text = status
	if ended:
		_end_job()


func _end_job() -> void:
	var file: FileAccess = _job["file"]
	var exported: bool = _job["export"]
	if exported:
		var ok := file.get_error() == OK
		file.close()
		status = (
			"Exported %s, %s" % [_name_of(_job["id"]), Worlds.size_words(_job["done"])]
			if ok
			else "The file could not be written"
		)
	else:
		file.close()
		var result := worlds.import_finish()
		status = (
			"Imported as %s" % Worlds.name_of(result)
			if result.has("id")
			else "The file was refused: %s" % result["why"]
		)
	_job = {}
	set_process(false)
	refresh()


func _row(w: Dictionary, current: bool) -> Control:
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 4)
	var title := "%s%s" % [Worlds.name_of(w), "  (opens on Crowd)" if current else ""]
	box.add_child(_text(title, 20, TEXT))
	var when := "not saved yet"
	if int(w["moment"]) >= 0:
		var ago := maxi(0, int(Time.get_unix_time_from_system()) - int(w["saved"]))
		when = "%s, saved %s ago" % [w["moment_text"], Worlds.ago_words(ago)]
		if w.get("saved_by", "") != "":
			when += " by %s" % w["saved_by"]
	box.add_child(_text(when, 15, QUIET))
	var parts := PackedStringArray()
	for part: Array in [
		["snapshots", "snapshots"],
		["journal", "journal"],
		["history", "history"],
		["previous", "the save before the update"],
	]:
		if int(w[part[0]]) > 0:
			parts.append("%s %s" % [part[1], Worlds.size_words(w[part[0]])])
	box.add_child(_text("%s: %s" % [Worlds.size_words(w["size"]), ", ".join(parts)], 15, QUIET))
	var id: String = w["id"]
	if _renaming == id:
		var edit := LineEdit.new()
		edit.text = Worlds.name_of(w)
		edit.custom_minimum_size = Vector2(0, 48)
		edit.text_submitted.connect(func(text: String) -> void: rename_world(id, text))
		box.add_child(edit)
		box.add_child(_button("Save the name", func() -> void: rename_world(id, edit.text)))
		return box
	var buttons := HBoxContainer.new()
	buttons.add_theme_constant_override("separation", 8)
	box.add_child(buttons)
	buttons.add_child(_button("Open", open_world.bind(id)))
	buttons.add_child(_button("Rename", _start_rename.bind(id)))
	buttons.add_child(_button("Export", pick_export.bind(id)))
	var delete := _button("Tap again" if _armed == id else "Delete", func() -> void: tap_delete(id))
	buttons.add_child(delete)
	return box


func _start_rename(id: String) -> void:
	_renaming = id
	refresh()


func _name_of(id: String) -> String:
	for w: Dictionary in listed:
		if w["id"] == id:
			return Worlds.name_of(w)
	return id


func _button(text: String, pressed: Callable) -> Button:
	var b := Button.new()
	b.text = text
	b.custom_minimum_size = Vector2(0, 48)
	b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	b.pressed.connect(pressed)
	return b


func _text(text: String, font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	return label


func _label(font_size: int, colour: Color) -> Label:
	var label := _text("", font_size, colour)
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	add_child(label)
	return label
