## Real captured history links never grant knowledge or name unseen recipes.
extends GdUnitTestSuite

const Example := preload("res://pages/first_flake.gd")
const Words := preload("res://camp/words.gd")
const ROOT := "user://test-worlds/history-links"


func after_test() -> void:
	Worlds.remove_tree(ROOT)


func _page() -> Control:
	var page := Example.new()
	page.root = ROOT
	add_child(page)
	page.set_process(false)
	page.layout(Vector2(1080, 2400), Rect2(0, 0, 1080, 2400))
	page._process(0)
	return page


func _link(page: Control, key: String, id: int) -> Button:
	for button: Node in page._history._body.find_children("*", "Button", true, false):
		if int(button.get_meta(key, 0)) == id:
			return button
	assert_bool(false).is_true()
	return null


func test_history_actor_and_input_links_restore_the_original_card_and_digest() -> void:
	var page := _page()
	var original_person: int = page.selected_id
	var original_item: int = page.selected_item_id
	var original_card: String = page._card.text
	var digest: String = page.world.digest()
	var events: Array = page.world.craft_history()
	(
		assert_bool(events.all(func(e: Dictionary) -> bool: return int(e.kind) in [1, 2, 3, 4]))
		. is_true()
	)
	var matches: Array = events.filter(
		func(e: Dictionary) -> bool: return int(e.result) == original_item
	)
	assert_array(matches).is_not_empty()
	var event: Dictionary = matches[0]
	page._history.open()
	_link(page, "history_person", int(event.actor)).pressed.emit()
	assert_int(page.selected_id).is_equal(int(event.actor))
	assert_int(page.selected_item_id).is_equal(0)
	assert_bool(page._details).is_true()
	assert_str(page._history_button.text).is_equal("Back to selection")
	page._history_button.pressed.emit()
	assert_int(page.selected_id).is_equal(original_person)
	assert_int(page.selected_item_id).is_equal(original_item)
	assert_bool(page._details).is_false()
	assert_str(page._card.text).is_equal(original_card)
	assert_array(event.inputs).is_not_empty()
	page._history.open()
	_link(page, "history_item", int(event.inputs[0])).pressed.emit()
	assert_int(page.selected_item_id).is_equal(int(event.inputs[0]))
	assert_dict(page.selected_item()).is_not_empty()
	assert_bool(page._dream_button.disabled).is_true()
	page._history_button.pressed.emit()
	assert_int(page.selected_item_id).is_equal(original_item)
	assert_str(page.world.digest()).is_equal(digest)
	assert_bool(page.world.is_paused()).is_true()
	page.free()


func test_spent_records_are_inspectable_and_never_drawn_as_live_stock() -> void:
	var page := _page()
	var actor: int = page.selected_id
	var records: Array = page.world.items(actor, true)
	var spent: Array = records.filter(func(i: Dictionary) -> bool: return int(i.mass_mg) == 0)
	assert_array(spent).is_not_empty()
	assert_bool(page.items.all(func(i: Dictionary) -> bool: return int(i.mass_mg) > 0)).is_true()
	if not spent.is_empty():
		var digest: String = page.world.digest()
		page._inspect_history_item(int(spent[0].id), actor)
		assert_dict(page.selected_item()).is_equal(spent[0])
		assert_str(page._card.text).contains("Spent input")
		assert_bool(page.drawing.drawn_items.has(int(spent[0].id))).is_false()
		assert_str(page.world.digest()).is_equal(digest)
	page.free()


func test_history_names_only_recorded_sources_and_public_words_omit_sender() -> void:
	var people := [{"id": 7, "name": "Ari"}, {"id": 8, "name": "Bo"}]
	var event := {
		"actor": 7,
		"source": 8,
		"kind": 2,
		"name": "Sharp flake",
		"at": 99,
		"route": 4,
		"east_cm": 0,
		"north_cm": 0,
		"word": "",
		"id": 12,
		"inputs": []
	}
	assert_str(Words.history(event, people, 100)).contains("Ari learned sharp flake from Bo")
	event.source = 0
	var words := Words.history(event, people, 100)
	assert_str(words).contains("no source recorded")
	assert_str(words).not_contains("from Bo")
	assert_str(words).not_contains("player")
	assert_str(words).not_contains("Your dream")


func test_source_link_uses_the_recorded_identity_and_missing_source_stays_missing() -> void:
	var page := _page()
	var original: int = page.selected_id
	var source: int = int(page.people[0].id)
	var digest: String = page.world.digest()
	page._history.open()
	# A presentation-only source-link fixture; it adds no history or knowledge to the world.
	page._history._link("Source: " + Words.name_of(source, page.people), source, false)
	_link(page, "history_person", source).pressed.emit()
	assert_int(page.selected_id).is_equal(source)
	assert_str(page._card.text).contains(Words.name_of(source, page.people))
	page._history_button.pressed.emit()
	assert_int(page.selected_id).is_equal(original)
	page._history.open()
	page._history._link("Source", 123, false)
	var missing := _link(page, "history_person", 123)
	assert_bool(missing.disabled).is_true()
	assert_str(missing.text).is_equal("No source recorded")
	page._history.close()
	assert_str(page.world.digest()).is_equal(digest)
	page.free()
