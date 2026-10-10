## Real conditional memories, durable idea requests and the player's private attempt record.
extends GdUnitTestSuite

const Camp := preload("res://pages/camp.gd")
const Dreams := preload("res://camp/dreams.gd")
const TEST_ROOT := "user://test-worlds/idea-dreams"


func after_test() -> void:
	Worlds.remove_tree(TEST_ROOT)


func _page() -> Control:
	var page := Camp.new()
	page.root = TEST_ROOT
	page.frozen = true
	add_child(page)
	page.set_process(false)
	page.layout(Vector2(1080, 2400), Rect2(0, 0, 1080, 2400))
	page._process(0)
	return page


func _remembered_idea_page() -> Control:
	var output := []
	var folder := ProjectSettings.globalize_path(TEST_ROOT.path_join("idea-scene"))
	var code := OS.execute(
		ProjectSettings.globalize_path("res://../build/sim/kindling"),
		[
			"idea-capture",
			folder,
			"201",
			str(ProjectSettings.get_setting("application/config/version")),
			ProjectSettings.globalize_path("res://../data")
		],
		output
	)
	assert_int(code).is_equal(0)
	if code != 0:
		return null
	var person := int(str(output[0]).split(" ")[0])
	var page := _page()
	assert_bool(page.opened.has("problem")).is_false()
	assert_int(page.world.frontier()).is_equal(101)
	page.select_person(person)
	return page


func test_real_idea_choice_queued_reopen_and_actual_attempt() -> void:
	var page := _remembered_idea_page()
	if page == null:
		return
	var person: int = page.selected_id
	page.open_dream(person, true)
	page._dreams.ideas()
	var choices: Array = page.world.idea_memories(person).filter(
		func(m: Dictionary) -> bool: return str(m.problem).is_empty()
	)
	assert_array(choices).is_not_empty()
	page._dreams.choose_idea(choices[0])
	assert_str(page._dreams._title.text).is_equal("Recall warmth they felt")
	page._dreams.send()
	assert_str(page._dreams.stage).is_equal("records")
	var pending: Array = page.world.dream_records()
	assert_int(pending[0].status).is_equal(1)
	assert_int(pending[0].first_attempt_at).is_equal(-1)
	page._dreams.close()
	page.save_camp()
	var digest: String = page.world.digest()
	page.free()
	page = _page()
	assert_array(page.world.dream_records()).is_equal(pending)
	assert_str(page.world.digest()).is_equal(digest)
	# Advance the display with the producer, so the test retains only the current viewing interval.
	while page.world.frontier() < 4 * 86400:
		page.world.run_until(mini(page.world.frontier() + 900, 4 * 86400))
		page.world.prepare_dream()
		page._process(0)
		if int(page.world.dream_records()[0].first_attempt_at) >= 0:
			break
	var delivered: Dictionary = page.world.dream_records()[0]
	assert_int(delivered.status).is_equal(2)
	assert_int(delivered.first_attempt_at).is_greater_equal(int(delivered.executed))
	assert_int(delivered.first_attempt_at).is_less(int(delivered.executed) + 3 * 86400)
	assert_str(Dreams.record_words(delivered, int(page.world.screen_time()))).contains(
		"First matching action"
	)
	page.select_person(person)
	page._details = true
	page._refresh_records()
	assert_str(page._card.text).not_contains("Your dream")
	assert_str(page._card.text).not_contains("player")
	page.save_camp()
	digest = page.world.digest()
	page.free()
	page = _page()
	assert_str(page.world.digest()).is_equal(digest)
	assert_dict(page.world.dream_records()[0]).is_equal(delivered)
	page.free()


func test_idea_journal_write_failure_has_no_dream_or_replayed_request() -> void:
	var page := _remembered_idea_page()
	if page == null:
		return
	var person: int = page.selected_id
	page.world.prepare_dream()
	var digest: String = page.world.digest()
	var at: int = page.world.frontier()
	var choices: Array = page.world.idea_memories(person).filter(
		func(m: Dictionary) -> bool: return str(m.problem).is_empty()
	)
	assert_array(choices).is_not_empty()
	var folder: String = TEST_ROOT.path_join(page.world_id)
	var journal := folder.path_join("journal.log")
	var kept := folder.path_join("kept-journal.log")
	assert_int(DirAccess.rename_absolute(journal, kept)).is_equal(OK)
	assert_int(DirAccess.make_dir_absolute(journal)).is_equal(OK)
	var result: Dictionary = page.world.send_idea_dream(person, int(choices[0].memory))
	assert_str(result.problem).contains("has not acted")
	assert_int(page.world.frontier()).is_equal(at)
	assert_array(page.world.dream_records()).is_empty()
	assert_bool(page.world.counters().save_failed).is_true()
	page.free()
	assert_int(DirAccess.remove_absolute(journal)).is_equal(OK)
	assert_int(DirAccess.rename_absolute(kept, journal)).is_equal(OK)
	page = _page()
	assert_bool(page.opened.has("problem")).is_false()
	assert_array(page.world.dream_records()).is_empty()
	assert_str(page.world.digest()).is_equal(digest)
	page.free()


func test_idea_confirmation_cancel_back_revalidates_without_recipe_names() -> void:
	var page := _page()
	var person := int(page.people[0].id)
	page.open_dream(person, true)
	var digest: String = page.world.digest()
	page._dreams.ideas()
	assert_str(page._dreams.stage).is_equal("ideas")
	# A stale visible choice must be revalidated by the native bridge, never acted from its label.
	var stale := {"memory": 999999, "name": "Twirled dry wood", "benefit": "warmth", "at": 0}
	page._dreams.choose_idea(stale)
	assert_str(page._dreams._title.text).is_equal("Recall warmth they felt")
	assert_str(page._dreams._title.text).not_contains("ember")
	assert_str(page._dreams._title.text).not_contains("fire drill")
	for window: Vector2 in [Vector2(360, 800), Vector2(1080, 2400), Vector2(800, 360)]:
		page.layout(window, Rect2(Vector2.ZERO, window))
		for frame in 4:
			await await_idle_frame()
		for button: Control in [page._dreams._cancel, page._dreams._confirm]:
			assert_float(button.size.y).is_greater_equal(48)
			assert_bool(Rect2(Vector2.ZERO, window).encloses(button.get_global_rect())).is_true()
	page._dreams.send()
	assert_str(page._dreams.stage).is_equal("confirm")
	assert_array(page.world.dream_records()).is_empty()
	assert_str(page.world.digest()).is_equal(digest)
	page._dreams.back()
	assert_str(page._dreams.stage).is_equal("ideas")
	page._dreams.close()
	page.free()


func test_private_idea_times_describe_actual_attempt_without_claiming_causation() -> void:
	var act := {
		"name": "Ada",
		"place": "An idea from remembered work",
		"kind": 1,
		"requested": 100,
		"received": 100,
		"executed": 200,
		"until": 259400,
		"status": 2,
		"reason": 0,
		"decision_at": 300,
		"choice": 4,
		"pull": 60,
		"visited_at": -1,
		"first_attempt_at": 900,
		"result_at": 900,
		"result": "The attempt failed"
	}
	var text: String = _dream_words(act)
	assert_str(text).contains("working with materials")
	assert_str(text).contains("First matching action")
	assert_str(text).contains("attempt failed")
	assert_str(text).contains("timing alone does not show")
	assert_str(text).not_contains("ember_drill")
	act.first_attempt_at = -1
	act.result_at = -1
	assert_str(_dream_words(act)).contains("No matching action recorded yet")
	act.status = 3
	act.reason = 4
	assert_str(_dream_words(act)).contains("memory was lost")


func _dream_words(act: Dictionary) -> String:
	return Dreams.record_words(act, 1000)
