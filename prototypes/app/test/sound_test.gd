## P14 Sound (IMPLEMENTATION α0.7c): the voice manager keeps SND-01's shares and blends the quietest
## into hums (SND-07); the extension makes every base sound, and talk from the bank shifted for age
## and feeling (SND-03); the screen, with everyone at once in a storm, never plays more than 32
## sounds, fills its shares, and muffles what the cliff hides (SND-08); the probe times the mix; and
## the line for the chat.
extends GdUnitTestSuite

const VOICES := preload("res://sound/voices.gd")
const SOUND := preload("res://sound/sound.gd")
const CAMP := preload("res://sound/camp.gd")
const REPORT := preload("res://sound/report.gd")
const SHARE := VOICES.Share


# checks: SND-01
func test_each_share_holds_its_most_and_work_takes_the_rest() -> void:
	var v := VOICES.new()
	for k in 6:
		assert_int(v.ask(SHARE.PLACE, 0.5, 0.0, 0.0)["id"]).is_greater(0)
	# a seventh place sound finds the six all loops: it is let go
	assert_int(v.ask(SHARE.PLACE, 0.9, 2.0, 0.0)["id"]).is_equal(0)
	for k in 8:
		v.ask(SHARE.MUSIC, 0.3, 0.0, 0.0)
	for k in 4:
		v.ask(SHARE.VOICES, 0.2 + 0.1 * k, 3.0, 0.0)
	# work has what is left but the two kept free: 32 - 2 - 6 - 8 - 4
	assert_int(v.most(SHARE.WORK)).is_equal(12)
	for k in 12:
		assert_int(v.ask(SHARE.WORK, 0.1 + 0.01 * k, 5.0, 0.0)["id"]).is_greater(0)
	assert_int(v.total()).is_equal(30)
	# the two free places are still there for thunder
	assert_int(v.ask(SHARE.FREE, 0.4, 4.0, 0.0)["id"]).is_greater(0)
	assert_int(v.ask(SHARE.FREE, 0.4, 4.0, 0.0)["id"]).is_greater(0)
	assert_int(v.total()).is_equal(32)


# checks: SND-01, SND-07
func test_when_a_share_is_full_its_quietest_join_its_hum() -> void:
	var v := VOICES.new()
	var ids := []
	for k in 4:
		ids.append(v.ask(SHARE.VOICES, 0.1 * (k + 1), 3.0, 0.0)["id"])
	# a louder voice: the hum starts and takes a place, so the two quietest join it
	var answer: Dictionary = v.ask(SHARE.VOICES, 0.9, 3.0, 1.0)
	assert_int(answer["id"]).is_greater(0)
	assert_array(answer["stop"]).contains_exactly_in_any_order([ids[0], ids[1]])
	assert_bool(v.humming(SHARE.VOICES)).is_true()
	assert_int(v.used(SHARE.VOICES)).is_equal(4)
	assert_float(v.hum(SHARE.VOICES)).is_equal_approx(sqrt(0.01 + 0.04), 1e-5)
	# a quieter voice than all playing goes straight to the hum
	assert_int(v.ask(SHARE.VOICES, 0.05, 2.0, 1.0)["id"]).is_equal(0)
	assert_int(v.used(SHARE.VOICES)).is_equal(4)
	# the hum lets go of each as it would have ended
	v.expire(10.0)
	assert_bool(v.humming(SHARE.VOICES)).is_false()


# checks: SND-01
func test_work_yields_to_the_other_shares() -> void:
	var v := VOICES.new()
	for k in 30:
		v.ask(SHARE.WORK, 0.1 + 0.01 * k, 10.0, 0.0)
	assert_int(v.used(SHARE.WORK)).is_equal(30)
	var stopped := 0
	for k in 8:
		stopped += (v.ask(SHARE.MUSIC, 0.5, 0.0, 0.0)["stop"] as Array).size()
	assert_int(v.used(SHARE.WORK)).is_equal(22)
	assert_int(stopped).is_equal(9)
	assert_int(v.total()).is_equal(30)


# checks: SND-01
func test_under_any_load_never_more_than_32() -> void:
	var v := VOICES.new()
	var rng := RandomNumberGenerator.new()
	rng.seed = 3
	var now := 0.0
	for k in 3000:
		now += rng.randf() * 0.05
		var share: int = rng.randi() % 5
		var seconds := 0.0 if rng.randf() < 0.05 else rng.randf_range(0.05, 4.0)
		var answer: Dictionary = v.ask(share, rng.randf(), seconds, now)
		if answer["id"] > 0 and rng.randf() < 0.02:
			v.done(answer["id"])
		assert_int(v.total()).is_less_equal(VOICES.CAP)
		for s: int in SHARE.values():
			assert_int(v.used(s)).is_less_equal(v.most(s))
		assert_int(v.most(SHARE.WORK)).is_greater_equal(12)


# checks: SND-06, SND-03
func test_the_extension_makes_every_base_sound_and_talk_shifted_for_age_and_feeling() -> void:
	assert_bool(ClassDB.class_exists("SoundMaker")).is_true()
	var maker: Object = ClassDB.instantiate("SoundMaker")
	for base in SOUND.BASES.size():
		var pcm: PackedByteArray = maker.call("make", base, 0.5, 0.5, 0.1, 2)
		assert_int(pcm.size()).is_greater(2000)
	assert_bool(maker.call("loops", SOUND.BASES.find("river"))).is_true()
	assert_bool(maker.call("loops", SOUND.BASES.find("strike"))).is_false()
	for voice in SOUND.BANKS.size():
		assert_bool(
			maker.call("load_bank", voice, FileAccess.get_file_as_bytes(SOUND.BANKS[voice]))
		)
	var words: PackedStringArray = maker.call("language", SOUND.LANGUAGE, 14)
	assert_int(words.size()).is_equal(14)
	var pitch := {}
	for who: String in ["child", "woman", "man", "old man"]:
		var talk := PackedByteArray()
		for seed in 4:
			var speaker: Array = CAMP.SPEAKERS[who]
			talk.append_array(maker.call("phrase", speaker[0], words, speaker[1], {}, seed))
		pitch[who] = float((maker.call("measure", talk) as Dictionary)["pitch"])
	assert_float(pitch["child"]).is_greater(pitch["woman"] * 1.2)
	assert_float(pitch["woman"]).is_greater(pitch["man"] * 1.6)
	assert_float(pitch["man"]).is_greater(pitch["old man"])
	var calm: PackedByteArray = maker.call("phrase", 1, words, {}, CAMP.FEELINGS["calm"], 9)
	var angry: PackedByteArray = maker.call("phrase", 1, words, {}, CAMP.FEELINGS["angry"], 9)
	var calm_m: Dictionary = maker.call("measure", calm)
	var angry_m: Dictionary = maker.call("measure", angry)
	assert_float(angry_m["loudness"]).is_greater(calm_m["loudness"])
	assert_float(angry_m["seconds"]).is_less(calm_m["seconds"])


## The screen, its sounds made, laid out on a phone-shaped window.
func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(SOUND)
	add_child(screen)
	screen.size = Vector2(400, 890)
	for k in 600:
		if screen.get("_fire") != null:
			break
		await await_idle_frame()
	return auto_free(screen)


# checks: SND-01, SND-07, SND-11, SND-12
func test_with_everyone_at_once_the_camp_plays_32_at_most_with_the_rest_in_hums() -> void:
	var screen := await _screen()
	assert_object(screen.get("_fire")).is_not_null()
	screen.call("set_hour", "storm")
	screen.call("set_busy", true)
	var v: RefCounted = screen.get("voices")
	var most := 0
	var hummed := false
	for k in 900:
		screen.call("step", 1.0 / 30.0)
		var n: int = screen.call("sounding")
		assert_int(n).is_less_equal(32)
		most = maxi(most, n)
		hummed = hummed or v.humming(SHARE.WORK) or v.humming(SHARE.VOICES)
		if k % 10 == 0:
			await await_idle_frame()
	assert_int(most).is_greater_equal(28)
	assert_bool(hummed).is_true()
	assert_int(v.used(SHARE.MUSIC)).is_equal(8)
	# talk came back from the worker threads and played
	assert_int(v.used(SHARE.VOICES)).is_greater(0)


# checks: SND-08
func test_the_cliff_muffles_what_is_above_it_from_below() -> void:
	var screen := await _screen()
	screen.call("set_hour", "day")
	var chopper := -1
	for i in CAMP.SOURCES.size():
		if CAMP.SOURCES[i]["kind"] == "chop" and "day" in CAMP.SOURCES[i]["hours"]:
			chopper = i
	assert_int(chopper).is_greater_equal(0)
	screen.set("listener_at", Vector2(205, 395))
	screen.call("_sound", chopper)
	var player: AudioStreamPlayer3D = screen.call("_player_of", chopper)
	assert_object(player).is_not_null()
	assert_float(player.attenuation_filter_cutoff_hz).is_equal(SOUND.MUFFLED_HZ)
	# above the cliff you hear it plainly
	screen.set("listener_at", Vector2(290, 150))
	screen.call("_follow")
	assert_float(player.attenuation_filter_cutoff_hz).is_equal(SOUND.OPEN_HZ)
	assert_bool(CAMP.muffled(Vector2(150, 180), Vector2(200, 350))).is_true()
	assert_bool(CAMP.muffled(Vector2(150, 380), Vector2(200, 350))).is_false()


# checks: SND-01, SND-08, PLT-04
func test_the_probe_times_the_mix_and_the_camp_is_heard() -> void:
	var screen := await _screen()
	var probe: AudioEffect = screen.get("_probe")
	assert_object(probe).is_not_null()
	probe.call("reset")
	for k in 30:
		await await_idle_frame()
	var started := Time.get_ticks_msec()
	while Time.get_ticks_msec() - started < 500:
		await await_idle_frame()
	var stats: Dictionary = probe.call("stats")
	assert_int(int(stats["blocks"])).is_greater(0)
	assert_float(float(stats["mean"])).is_greater(0.0)
	assert_int(int(stats["over"])).is_equal(0)
	# and the camp is heard: 3D players are silent in a world with no camera
	var loudest := maxf(
		AudioServer.get_bus_peak_volume_left_db(0, 0),
		AudioServer.get_bus_peak_volume_right_db(0, 0)
	)
	assert_float(loudest).is_greater(-60.0)


# checks: SND-01, PLT-04
func test_the_line_for_the_chat_and_its_verdict() -> void:
	var stats := {
		"blocks": 5000,
		"mean": 0.031,
		"p99": 0.06,
		"worst": 0.12,
		"over": 0,
		"late": 1,
		"threads": 0,
		"frames": 512
	}
	var counts: Array[int] = [32, 31, 32]
	var fullest := {"place": 5, "voices": 4, "music": 8, "free": 2, "work": 13}
	(
		assert_str(REPORT.line("P14 α0.7c Pixel", stats, counts, fullest, 48000.0, 0.025, []))
		. is_equal(
			(
				"P14 α0.7c Pixel | 32 sounds at most (place 5 voices 4 music 8 free 2 work 13), 31.7"
				+ " on average | audio thread 3.1% mean, 6% for 99 in 100, 12.0% worst | 0 over, 1 late,"
				+ " 5000 blocks of 512 frames at 48000 Hz, 0 thread changes | delay 25 ms"
			)
		)
	)
	assert_str(REPORT.verdict(stats)).starts_with("Passes")
	stats["over"] = 3
	assert_str(REPORT.verdict(stats)).contains("breaks")
	stats["over"] = 0
	stats["mean"] = 0.4
	assert_str(REPORT.verdict(stats)).starts_with("No breaks, but over its share")
