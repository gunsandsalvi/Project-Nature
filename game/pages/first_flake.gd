## The actual captured discovery camp, kept separately from the player's camp.
extends "res://pages/camp.gd"

var capture := {}


func _init() -> void:
	root = "user://first-flake-format-%d" % KdWorld.save_format()
	frozen = true


func _ready() -> void:
	capture = JSON.parse_string(
		FileAccess.get_file_as_string("res://data/examples/first-flake.json")
	)
	# A new illustrative capture can share the save format with the previous one.
	# Its cache must open this recorded state, while preserving the old example.
	root = root.path_join("capture-" + str(capture.digest))
	var kept := Worlds.at(root)
	if kept.list().is_empty():
		var source := FileAccess.open("res://data/examples/first-flake.kindling", FileAccess.READ)
		if (
			source == null
			or not kept.import_begin()
			or not kept.import_feed(source.get_buffer(source.get_length()))
		):
			_show_problem("The recorded camp could not be opened.")
			return
		var imported := kept.import_finish()
		if imported.has("problem"):
			_show_problem(str(imported.problem))
			return
		kept.set_current(str(imported.id))
	super._ready()
	if is_instance_valid(drawing) and capture is Dictionary:
		selected_id = str(capture.actor).to_int()
		selected_item_id = str(capture.result).to_int()
		_refresh_records()


func _show_problem(words: String) -> void:
	var label := Label.new()
	label.text = words
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	set_process(false)


func _refresh_records(counters: Dictionary = {}) -> void:
	super._refresh_records(counters)
	if is_instance_valid(_summary):
		var made := int(capture.get("at", world.frontier()))
		_summary.text = (
			"First flake · captured ordinary camp\nDay %d · %02d:%02d"
			% [made / 86400 + 1, posmod(made, 86400) / 3600, posmod(made, 3600) / 60]
		)
