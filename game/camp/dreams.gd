## The owner's private choices. A person's card knows only the ordinary dream.
extends PanelContainer

const Words := preload("res://camp/words.gd")
const Sizing := preload("res://ui/sizing.gd")
var camp: Control
var person_id := 0
var stage := ""
var _was_paused := true
var _subject := {}
var _column: VBoxContainer
var _body: VBoxContainer
var _title: Label
var _cancel: Button
var _confirm: Button
var _window := Vector2.ZERO
var _safe := Rect2()


func _ready() -> void:
	var style := StyleBoxFlat.new()
	style.bg_color = Color("203b31")
	add_theme_stylebox_override("panel", style)
	_column = VBoxContainer.new()
	add_child(_column)
	_title = Label.new()
	_title.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_column.add_child(_title)
	var scroll := ScrollContainer.new()
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_column.add_child(scroll)
	_body = VBoxContainer.new()
	_body.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.add_child(_body)
	var bar := HBoxContainer.new()
	_column.add_child(bar)
	_cancel = Button.new()
	_cancel.text = "Cancel"
	_cancel.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_cancel.pressed.connect(back)
	bar.add_child(_cancel)
	_confirm = Button.new()
	_confirm.text = "Send dream"
	_confirm.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_confirm.pressed.connect(send)
	bar.add_child(_confirm)
	hide()


func _clear(next: String, title: String) -> void:
	stage = next
	_title.text = title
	for child: Node in _body.get_children():
		_body.remove_child(child)
		child.queue_free()
	_cancel.text = "Done" if stage == "records" else "Back" if stage == "confirm" else "Cancel"
	_confirm.visible = stage == "confirm"
	_confirm.disabled = false


func _words(text: String) -> void:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_body.add_child(label)


func _begin() -> bool:
	_was_paused = camp.world.is_paused()
	var result: Dictionary = camp.world.prepare_dream()
	if result.has("problem"):
		return false
	show()
	camp._refresh_records()
	return true


func open_person(id: int, ring: bool = false) -> void:
	if not _begin():
		return
	person_id = id
	camp.select_person(id)
	var person: Dictionary = camp.selected_person()
	if person.is_empty():
		close()
		return
	_clear("ring" if ring else "subjects", "Dream for " + str(person.name))
	if ring:
		_words("Camp paused at %s.\nChoose one small influence." % camp.world.time_text())
		var button := Button.new()
		button.text = "Dream of a place"
		button.pressed.connect(subjects)
		_body.add_child(button)
		var idea_button := Button.new()
		idea_button.text = "Idea from a memory"
		idea_button.pressed.connect(ideas)
		_body.add_child(idea_button)
	else:
		subjects()
	layout(_window, _safe)


func subjects() -> void:
	_clear("subjects", "Dream for " + str(camp.selected_person().get("name", "this person")))
	_words(
		(
			(
				"Paused at %s. Only places they remember.\n"
				+ "One dream per sleeper, three each night. Resets at 06:00."
			)
			% camp.world.time_text()
		)
	)
	var choices: Array = camp.world.dream_subjects(person_id)
	for choice: Dictionary in choices:
		var button := Button.new()
		button.text = str(choice.name)
		button.disabled = not str(choice.problem).is_empty()
		button.pressed.connect(choose.bind(choice))
		_body.add_child(button)
		_words(
			(
				("Remembered at " + Words.when(int(choice.seen_at), int(camp.world.screen_time())))
				if str(choice.problem).is_empty()
				else str(choice.problem)
			)
		)
	if choices.is_empty():
		_words("They haven't noticed a place yet.")
	layout(_window, _safe)


func ideas() -> void:
	_clear("ideas", "Recall an action")
	_words("Only their own handled actions and benefits they experienced. An idea remains a guess.")
	var choices: Array = camp.world.idea_memories(person_id)
	for choice: Dictionary in choices:
		var button := Button.new()
		button.text = (
			str(choice.name) + " · " + Words.when(int(choice.at), int(camp.world.screen_time()))
		)
		button.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		button.disabled = not str(choice.problem).is_empty()
		button.pressed.connect(choose_idea.bind(choice))
		_body.add_child(button)
		if not str(choice.problem).is_empty():
			_words(str(choice.problem))
	if choices.is_empty():
		_words("No remembered work yet. Watching alone is not a handled action.")
	layout(_window, _safe)


func choose_idea(memory: Dictionary) -> void:
	_subject = memory
	_clear("confirm", "Recall " + str(memory.benefit) + " they felt")
	_words(
		(
			(
				"%s. Recall %s they experienced.\n\n"
				+ "They may try the remembered action during the next three days. "
				+ "Their needs, materials and their own choices decide; the attempt can fail.\n\n"
				+ "Asked at %s. Cancel leaves no dream."
			)
			% [str(memory.name), str(memory.benefit), camp.world.time_text()]
		)
	)
	layout(_window, _safe)


func back() -> void:
	if stage == "confirm":
		if _subject.has("memory"):
			ideas()
		else:
			subjects()
		return
	close()


func choose(subject: Dictionary) -> void:
	_subject = subject
	_clear("confirm", "Dream of " + str(subject.name).to_lower())
	_words(
		(
			(
				"%s will dream of this remembered place during their next sleep, or now if sleeping.\n\n"
				+ "It may draw a visit for three days. Their needs and their own choices still decide.\n\n"
				+ "Asked at %s. Cancel leaves no dream."
			)
			% [camp.selected_person().name, camp.world.time_text()]
		)
	)
	layout(_window, _safe)


func send() -> void:
	if stage != "confirm":
		return
	_confirm.disabled = true
	var result: Dictionary
	if _subject.has("memory"):
		result = camp.world.send_idea_dream(person_id, int(_subject.memory))
	else:
		result = camp.world.send_place_dream(person_id, int(_subject.subject))
	if result.has("problem"):
		_words(str(result.problem))
		layout(_window, _safe)
		return
	camp._refresh_records()
	records(false)


func records(begin: bool = true) -> void:
	if begin and not _begin():
		return
	_clear("records", "Your dreams")
	_words("Your choices are kept here. People remember only their own dreams.")
	var acts: Array = camp.world.dream_records()
	if acts.is_empty():
		_words("No dreams sent yet. Hold a person, then choose Dream of a place.")
	acts.reverse()
	for act: Dictionary in acts:
		_words(record_words(act, int(camp.world.screen_time())))
	layout(_window, _safe)


static func record_words(act: Dictionary, now: int) -> String:
	var text := (
		"\n%s · %s\nAsked at %s · received at %s."
		% [
			act.name,
			act.place,
			Words.when(int(act.requested), now),
			Words.when(int(act.received), now)
		]
	)
	if int(act.status) == 1:
		return text + "\nWaiting for their next sleep."
	if int(act.status) == 3:
		var reasons := {
			1: "The place is gone",
			2: "The night's dream limit was reached",
			3: "The person is gone",
			4: "That action memory was lost",
			5: "The remembered action no longer fits an experienced benefit"
		}
		return (
			text
			+ (
				"\nCancelled at %s: %s."
				% [Words.when(int(act.executed), now), reasons[int(act.reason)]]
			)
		)
	text += (
		"\nDreamt at %s. Fades after %s."
		% [Words.when(int(act.executed), now), Words.when(int(act.until), now)]
	)
	if int(act.decision_at) >= 0:
		text += (
			"\nAfter waking, chose %s at %s."
			% [
				[
					"food",
					"water",
					"rest",
					"a visit" if int(act.pull) > 0 else "looking around",
					"working with materials"
				][int(act.choice)],
				Words.when(int(act.decision_at), now)
			]
		)
		text += (
			"\nThe dream made this feel a little more worthwhile."
			if int(act.pull) > 0
			else "\nTheir needs led this choice; the dream added no pull."
		)
	else:
		text += "\nNo waking choice recorded yet."
	if int(act.visited_at) >= 0:
		text += "\nReached the remembered place at %s." % Words.when(int(act.visited_at), now)
	if int(act.get("kind", 0)) == 1:
		var tried := int(act.get("first_attempt_at", -1))
		text += (
			"\nFirst matching action at %s." % Words.when(tried, now)
			if tried >= 0
			else "\nNo matching action recorded yet."
		)
		if int(act.get("result_at", -1)) >= 0:
			text += "\n%s at %s." % [str(act.result), Words.when(int(act.result_at), now)]
		text += "\nLater timing alone does not show the dream caused this."
	return text


func close() -> void:
	hide()
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
	var style: StyleBoxFlat = get_theme_stylebox("panel")
	for edge: String in ["left", "right", "top", "bottom"]:
		style.set("content_margin_" + edge, gap)
	Sizing.page(self, density)
	_title.add_theme_font_size_override("font_size", Sizing.font_size(20, density))
	position = safe.position + (safe.size - Vector2(width, height)) / 2
	size = Vector2(width, height)
	# Containers settle their new text minimum after font changes on rotation.
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
