## Work/fire cues read actual immutable samples; mute/camera are presentation only.
extends GdUnitTestSuite

const Sounds := preload("res://camp/sounds.gd")
const Example := preload("res://pages/first_flake.gd")
const Preferences := preload("res://ui/preferences.gd")
const ROOT := "user://test-worlds/sound-cues"
var _mute_before := false


func before_test() -> void:
	_mute_before = Preferences.value("mute")
	Preferences.set_value("mute", false)


func after_test() -> void:
	Preferences.set_value("mute", _mute_before)
	Worlds.remove_tree(ROOT)


func test_actual_starting_fire_is_quiet_when_distant_paused_or_muted_with_the_same_digest() -> void:
	var world := KdWorld.new()
	GameData.load_into(world)
	var shelf := Worlds.at(ROOT)
	var id := shelf.make_discovery("Sound test", 17)
	world.open_camp(
		ProjectSettings.globalize_path(ROOT.path_join(id)),
		17,
		str(ProjectSettings.get_setting("application/config/version"))
	)
	var camera := KdCanvas.new()
	var origin := world.camp_at(0)
	camera.set_world(world, origin[0], origin[1])
	var sound: Node = auto_free(Sounds.new())
	add_child(sound)
	var fires: Array = world.items().filter(
		func(i: Dictionary) -> bool: return int(i.get("fire_heat", 0)) >= 2
	)
	assert_array(fires).is_not_empty()
	if fires.is_empty():
		world = null
		return
	var at: Vector2 = camera.world_local(int(fires[0].east_cm), int(fires[0].north_cm))
	camera.focus(at.x, at.y)
	var state := camera.frame(360, 500, 0)
	var digest: String = world.digest()
	sound.update(world.people(), world.items(), camera, state, false)
	assert_bool(sound._fire.playing).is_true()
	assert_int(sound._work.size()).is_equal(4)
	Preferences.set_value("mute", true)
	sound.update(world.people(), world.items(), camera, state, false)
	assert_bool(sound._fire.playing).is_false()
	Preferences.set_value("mute", false)
	camera.focus(at.x + 100, at.y)
	state = camera.frame(360, 500, 0)
	sound.update(world.people(), world.items(), camera, state, false)
	assert_bool(sound._fire.playing).is_false()
	camera.focus(at.x, at.y)
	state = camera.frame(360, 500, 0)
	sound.update(world.people(), world.items(), camera, state, true)
	assert_bool(sound._fire.playing).is_false()
	assert_str(world.digest()).is_equal(digest)
	world.save_now()
	world = null


func test_actual_work_phase_uses_a_work_cue_and_mute_does_not_change_the_world() -> void:
	var page := Example.new()
	page.root = ROOT
	add_child(page)
	page.set_process(false)
	page._process(0)
	var active: Array = page.people.filter(
		func(p: Dictionary) -> bool: return int(p.action_code) in [8, 10, 12]
	)
	assert_array(active).is_not_empty()
	if active.is_empty():
		page.free()
		return
	var at: Vector2 = page.camera.world_local(int(active[0].east_cm), int(active[0].north_cm))
	page.camera.focus(at.x, at.y)
	var state: Dictionary = page.camera.frame(360, 500, 0)
	var sound: Node = page._sounds
	sound._phases = {}
	sound.update(page.world.people(), page.world.items(), page.camera, state, false)
	var before: int = sound.cue_count
	for i in 12:
		page.world.run_until(page.world.frontier() + 75)
		state = page.camera.frame(360, 500, 0)
		sound.update(page.world.people(), page.world.items(), page.camera, state, false)
		if sound.cue_count > before:
			break
	assert_int(sound.cue_count).is_greater(before)
	assert_bool(sound.last_cue in ["tap", "work"]).is_true()
	assert_float(sound.last_db).is_less_equal(-14)
	var digest: String = page.world.digest()
	Preferences.set_value("mute", true)
	sound.update(page.world.people(), page.world.items(), page.camera, state, false)
	assert_bool(sound._work.all(func(p: AudioStreamPlayer) -> bool: return not p.playing)).is_true()
	assert_str(page.world.digest()).is_equal(digest)
	page.free()
