## Factual camp history. Inspecting an entry never teaches a person its recipe.
extends PanelContainer

const Words := preload("res://camp/words.gd")
const Sizing := preload("res://ui/sizing.gd")
var camp: Control
var _body: VBoxContainer
var _was_paused := true
var _previous_person := 0
var _previous_item := 0
var _inspecting := false
var _window := Vector2.ZERO
var _safe := Rect2()


func _ready() -> void:
	var column := VBoxContainer.new()
	add_child(column)
	var title := Label.new()
	title.text = "Camp history"
	column.add_child(title)
	var scroll := ScrollContainer.new()
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	column.add_child(scroll)
	_body = VBoxContainer.new()
	_body.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.add_child(_body)
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
	_inspecting = false
	camp.world.pause()
	for child: Node in _body.get_children():
		_body.remove_child(child)
		child.queue_free()
	var count := 0
	var events: Array = camp.world.craft_history()
	events.reverse()
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
		if int(event.result) != 0:
			_link("Inspect result", int(event.result), true)
		for id: int in event.inputs:
			_link("Inspect input", id, true)
	if count == 0:
		var empty := Label.new()
		empty.text = "No discovery or learning has been recorded yet."
		empty.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		_body.add_child(empty)
	show()
	layout(_window, _safe)


func _link(words: String, id: int, item: bool) -> void:
	var button := Button.new()
	button.text = words
	button.pressed.connect(
		func() -> void:
			if item:
				camp.selected_item_id = id
			else:
				camp.select_person(id)
			_inspecting = true
			hide()
			camp._refresh_records()
	)
	_body.add_child(button)


func close() -> void:
	hide()
	camp.selected_id = _previous_person
	camp.selected_item_id = _previous_item
	if not _was_paused:
		camp.world.play()
	camp._refresh_records()


func layout(window: Vector2, safe: Rect2) -> void:
	_window = window
	_safe = safe
	if window == Vector2.ZERO:
		return
	var density := clampf(minf(window.x, window.y) / 450, 1, 3)
	var gap := 16 * density
	var width := minf(520 * density, safe.size.x - 2 * gap)
	var height := minf(640 * density, safe.size.y - 2 * gap)
	position = safe.position + (safe.size - Vector2(width, height)) / 2
	size = Vector2(width, height)
	Sizing.page(self, density)
