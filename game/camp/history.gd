## Factual camp history. Inspecting an entry never teaches a person its recipe.
extends PanelContainer

const Words := preload("res://camp/words.gd")
const Sizing := preload("res://ui/sizing.gd")
var camp: Control
var _body: VBoxContainer
var _cursors: Array[int] = [0]
var _next := 0
var _older: Button
var _newer: Button
var _was_paused := true
var _previous_person := 0
var _previous_item := 0
var _previous_details := false
var _previous_record := {}
var _inspecting := false
var _window := Vector2.ZERO
var _safe := Rect2()


func _ready() -> void:
	var style := StyleBoxFlat.new()
	style.bg_color = Color("203b31")
	add_theme_stylebox_override("panel", style)
	var column := VBoxContainer.new()
	add_child(column)
	var title := Label.new()
	title.text = "Camp history"
	column.add_child(title)
	var scroll := ScrollContainer.new()
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	column.add_child(scroll)
	var content := VBoxContainer.new()
	content.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.add_child(content)
	_body = VBoxContainer.new()
	_body.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	content.add_child(_body)
	var navigation := HBoxContainer.new()
	content.add_child(navigation)
	_newer = Button.new()
	_newer.text = "Newer"
	_newer.pressed.connect(
		func() -> void:
			_cursors.pop_back()
			_show_page()
	)
	navigation.add_child(_newer)
	_older = Button.new()
	_older.text = "Older"
	_older.pressed.connect(
		func() -> void:
			_cursors.append(_next)
			_show_page()
	)
	navigation.add_child(_older)
	var back := Button.new()
	back.text = "Back to camp"
	back.pressed.connect(close)
	column.add_child(back)
	hide()


func open() -> void:
	if not _inspecting:
		_was_paused = camp.world.is_paused()
		_previous_person = camp.selected_id
		_previous_item = camp.selected_item_id
		_previous_details = camp._details
		_previous_record = camp._historical_item
		_cursors = [0]
	_inspecting = false
	camp.world.pause()
	_show_page()
	show()
	layout(_window, _safe)


func _show_page() -> void:
	for child: Node in _body.get_children():
		_body.remove_child(child)
		child.queue_free()
	var count := 0
	var events: Array = camp.world.craft_history_page(_cursors.back(), 33)
	_next = int(events[31].id) if events.size() > 32 else 0
	_older.disabled = _next == 0
	_newer.disabled = _cursors.size() == 1
	if events.size() > 32:
		events.resize(32)
	for event: Dictionary in events:
		if int(event.kind) == 0 or int(event.kind) == 5:
			continue
		count += 1
		var label := Label.new()
		label.text = Words.history(event, camp.people, int(camp.world.screen_time()))
		label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		_body.add_child(label)
		_link("Person: " + Words.name_of(int(event.actor), camp.people), int(event.actor), false)
		if int(event.source) != 0:
			_link(
				"Source: " + Words.name_of(int(event.source), camp.people), int(event.source), false
			)
		else:
			var missing := Label.new()
			missing.text = "No source recorded"
			_body.add_child(missing)
		if int(event.result) != 0:
			_link("Inspect result", int(event.result), true, int(event.actor))
		for id: int in event.inputs:
			_link("Inspect input", id, true, int(event.actor))
		for source in event.get("heat_sources", []):
			_link("Inspect heating fire", int(source.fire), true, int(event.actor))
			if int(source.origin) != 0 and int(source.origin) != int(source.fire):
				_link("Inspect original ember", int(source.origin), true, int(event.actor))
	if count == 0:
		var empty := Label.new()
		empty.text = "No discovery or learning has been recorded yet."
		empty.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		_body.add_child(empty)


func _link(words: String, id: int, item: bool, observer: int = 0) -> void:
	var button := Button.new()
	button.text = words
	if not item and not camp.people.any(func(p: Dictionary) -> bool: return int(p.id) == id):
		button.text = "No source recorded"
		button.disabled = true
	button.set_meta("history_item" if item else "history_person", id)
	button.pressed.connect(
		func() -> void:
			if item:
				camp._inspect_history_item(id, observer)
			else:
				camp.select_person(id)
			camp._details = true
			_inspecting = true
			hide()
			camp._resize()
			camp._refresh_records()
	)
	_body.add_child(button)


func close() -> void:
	hide()
	camp.selected_id = _previous_person
	camp.selected_item_id = _previous_item
	camp._historical_item = _previous_record
	camp._details = _previous_details
	_inspecting = false
	camp._resize()
	if not _was_paused:
		camp.world.play()
	camp._refresh_records()


func layout(window: Vector2, safe: Rect2) -> void:
	_window = window
	_safe = safe
	if window == Vector2.ZERO:
		return
	var density := clampf(minf(window.x, window.y) / 450, 1, 3)
	Sizing.page(self, density)
	_fit_window.call_deferred()


func _fit_window() -> void:
	if _window == Vector2.ZERO:
		return
	var density := clampf(minf(_window.x, _window.y) / 450, 1, 3)
	var gap := 16 * density
	var desired := Vector2(
		minf(520 * density, _safe.size.x - 2 * gap), minf(640 * density, _safe.size.y - 2 * gap)
	)
	size = desired
	position = _safe.position + (_safe.size - desired) / 2
