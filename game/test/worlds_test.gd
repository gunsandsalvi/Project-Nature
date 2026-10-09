## The Worlds page and the worlds on the phone, headless in the cloud (A17): three worlds switched
## in turn each open exactly where they were left; one exported to a file and imported again runs
## on as the one it came from, and a damaged file is refused with words naming the damage; a world
## is renamed, deleted only after a second tap, and shown with its size by part and the free space;
## and α1.4a's world carries on, its last save kept aside.
extends GdUnitTestSuite

const WorldsPage := preload("res://pages/worlds.gd")
const CrowdPage := preload("res://pages/crowd.gd")
const ROOT := "user://test-worlds"


func after_test() -> void:
	Worlds.remove_tree(ROOT)


## The Worlds page on the tests' own folder, making small worlds of four camps.
func _worlds_page() -> VBoxContainer:
	var page: VBoxContainer = auto_free(WorldsPage.new())
	page.root = ROOT
	page.new_camps = 4
	page.size = Vector2(540, 1100)
	add_child(page)
	return page


## The Crowd page on the world chosen on the Worlds page, as Open shows it.
func _crowd_page() -> VBoxContainer:
	var page: VBoxContainer = CrowdPage.new()
	page.root = ROOT
	page.size = Vector2(540, 1100)
	add_child(page)
	return page


## The Crowd page closed, as switching to another page closes it, which saves its world.
func _close(page: VBoxContainer) -> void:
	remove_child(page)
	page.free()


## Opens a world on the Crowd page, runs it on to a moment and closes it: its digest there.
func _run_to(shelf: VBoxContainer, id: String, moment: int) -> String:
	shelf.open_world(id)
	var crowd := _crowd_page()
	assert_str(crowd.opened.get("problem", "")).is_empty()
	crowd.world.run_until(moment)
	crowd.world.save_now()
	var digest: String = crowd.world.digest()
	_close(crowd)
	return digest


# checks: TIM-08
func test_three_worlds_switched_in_turn_each_open_where_they_were_left() -> void:
	var shelf := _worlds_page()
	var ids: Array[String] = []
	for i in 3:
		ids.append(shelf.make_world())
	assert_int(shelf.listed.size()).is_equal(3)
	var left := {}
	for i in 3:
		left[ids[i]] = _run_to(shelf, ids[i], 9 * 3600 + 1000 * i)
	# three worlds of their own
	assert_int({left[ids[0]]: 0, left[ids[1]]: 0, left[ids[2]]: 0}.size()).is_equal(3)
	# each opened again, before its first frame moves it on, is where it was left; then run on
	for round in 2:
		for i in 3:
			shelf.open_world(ids[i])
			var crowd := _crowd_page()
			assert_bool(crowd.opened["made"]).is_false()
			assert_str(crowd.world.digest()).is_equal(left[ids[i]])
			crowd.world.run_until(crowd.world.frontier() + 600 * (round + 1))
			crowd.world.save_now()
			left[ids[i]] = crowd.world.digest()
			_close(crowd)
	assert_str(Worlds.at(ROOT).current()).is_equal(ids[2])


# checks: PLT-08
func test_a_world_exported_and_imported_runs_on_as_the_one_it_came_from() -> void:
	var shelf := _worlds_page()
	var id: String = shelf.make_world()
	shelf.open_world(id)
	var crowd := _crowd_page()
	crowd.world.run_until(9 * 3600)
	crowd.world.call_home(1)
	crowd.world.run_until(10 * 3600)
	crowd.world.save_now()
	_close(crowd)
	# out to a file, a few megabytes a frame, and in again as a copy
	var file := ROOT.path_join("exported.kindling")
	assert_bool(shelf.export_to(id, file)).is_true()
	while shelf.busy():
		await await_idle_frame()
	assert_str(shelf.status).starts_with("Exported World 1")
	assert_bool(shelf.import_from(file)).is_true()
	while shelf.busy():
		await await_idle_frame()
	assert_str(shelf.status).is_equal("Imported as World 1 (imported)")
	assert_int(shelf.listed.size()).is_equal(2)
	var copy := ""
	for w: Dictionary in shelf.listed:
		if w["id"] != id:
			copy = w["id"]
	# both run on to the same moment, and end the same
	var end := 30 * 3600
	assert_str(_run_to(shelf, copy, end)).is_equal(_run_to(shelf, id, end))
	# a file with one byte changed is refused with words naming the damage, and leaves no world
	var bytes := FileAccess.get_file_as_bytes(file)
	bytes[bytes.size() / 2] ^= 1
	var damaged := ROOT.path_join("damaged.kindling")
	FileAccess.open(damaged, FileAccess.WRITE).store_buffer(bytes)
	assert_bool(shelf.import_from(damaged)).is_true()
	while shelf.busy():
		await await_idle_frame()
	assert_str(shelf.status).starts_with("The file was refused: ")
	assert_str(shelf.status).contains("damaged")
	assert_int(shelf.listed.size()).is_equal(2)
	assert_bool(DirAccess.dir_exists_absolute(ROOT.path_join(".importing"))).is_false()


# checks: TIM-08 PLT-10
func test_a_world_is_renamed_and_deleted_only_after_a_second_tap() -> void:
	var shelf := _worlds_page()
	var id: String = shelf.make_world()
	shelf.rename_world(id, 'Lory\'s "best" camp')
	assert_str(shelf.listed[0]["name"]).is_equal('Lory\'s "best" camp')
	assert_bool(shelf.tap_delete(id)).is_false()
	assert_int(shelf.listed.size()).is_equal(1)
	assert_str(shelf.status).contains("again")
	assert_bool(shelf.tap_delete(id)).is_true()
	assert_int(shelf.listed.size()).is_equal(0)
	assert_bool(DirAccess.dir_exists_absolute(ROOT.path_join(id))).is_false()


# checks: PLT-10
func test_each_world_shows_its_size_by_part_and_a_nearly_full_phone_is_warned_of() -> void:
	var shelf := _worlds_page()
	var id: String = shelf.make_world()
	_run_to(shelf, id, 2 * 86400)
	shelf.refresh()
	var w: Dictionary = shelf.listed[0]
	assert_int(w["snapshots"]).is_greater(0)
	assert_int(w["history"]).is_greater(0)
	assert_int(w["size"]).is_greater_equal(w["snapshots"] + w["journal"] + w["history"])
	assert_int(w["moment"]).is_equal(2 * 86400)
	assert_str(w["moment_text"]).starts_with("Year 1, spring, day 3")
	assert_int(Worlds.at(ROOT).free_space()).is_greater(0)
	# below the tuning's mark the game warns, and asks which worlds to delete
	assert_str(Worlds.space_warning(512, 1024)).contains("nearly full")
	assert_str(Worlds.space_warning(512, 1024)).contains("Open Worlds")
	assert_str(Worlds.space_warning(2048, 1024)).is_empty()
	assert_str(Worlds.space_warning(-1, 1024)).is_empty()
	# and the Crowd page's counters carry the free space it checks at each save
	shelf.open_world(id)
	var crowd := _crowd_page()
	assert_int(crowd.world.counters()["free_mb"]).is_greater(0)
	assert_int(crowd.world.counters()["warn_below_mb"]).is_equal(1024)
	_close(crowd)
