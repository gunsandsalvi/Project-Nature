## P14 Sound (IMPLEMENTATION α0.7c, research 15): does a camp's sound, 32 sounds at once with
## distance filters, a cave's echo and the murmur of talk, play without breaks on your phone
## (SND-01, SND-03, SND-08, SND-11, SND-12)? The art book's close camp is the map: touch it to stand
## there and drag to walk, and hear the camp from where you stand, through Godot's 3D players: each
## sound from its direction, quieter and duller with distance, muffled by the cliff and echoing
## under the rock shelter. The base sounds are made by code as the screen opens (SND-06), the fire
## and the wind live as they play, talk from the murmur's bank beside the game; a voice manager
## keeps SND-01's shares. Measure plays everything at once for a minute and times the audio thread.
## Pre-production code (research 00): prototypes/sound's README names the items it is about.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const Voices := preload("res://sound/voices.gd")
const Camp := preload("res://sound/camp.gd")
const Report := preload("res://sound/report.gd")
const PICTURE := preload("res://interface/world/closecamp.png")
const BANKS: Array[String] = ["res://sound/bank/woman.bank", "res://sound/bank/man.bank"]
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const PANEL := Color(0.086, 0.075, 0.114, 0.88)
const GAP := 8
const TARGET := 48
const PLAYERS := 32
const LANGUAGE := 7
## The base sounds in the sound library's order (sound::Base).
const BASES: Array[String] = [
	"strike",
	"scrape",
	"chop",
	"step",
	"fire",
	"wind",
	"river",
	"rain",
	"thunder",
	"bird",
	"bark",
	"howl",
	"drum",
]
## Each hour's fire heat, wind speed and the picture's tint.
const HOURS := {
	"day": [0.35, 0.25, Color(1.0, 1.0, 1.0)],
	"dusk": [0.6, 0.15, Color(1.0, 0.8, 0.66)],
	"night": [0.85, 0.3, Color(0.36, 0.42, 0.66)],
	"storm": [0.7, 0.85, Color(0.56, 0.6, 0.68)],
}
## The makers that loop, sounding all through their hours.
const LOOPS: Array[String] = ["fire", "wind", "river", "rain", "drum"]
## How much sooner each maker sounds again with everyone at once.
const BUSY := 0.25
## A sound's low-pass in the open and with the cliff between (SND-08).
const OPEN_HZ := 5000.0
const MUFFLED_HZ := 900.0
const MUFFLED_DB := -6.0
## Wind and rain heard from under the shelter.
const SHELTERED_DB := -6.0
## The whole camp lifted before the limiter, so it is heard at a fair loudness.
const LIFT := 5.0
## Each share's colour on the map: place, voices, music, free, work.
const SHARE_COLOURS: Array[Color] = [
	Color("7fb8e6"), Color("f2d06b"), Color("c79bf2"), Color("f26b6b"), Color("f6a33c")
]
var hour := "dusk"
var busy := false
## Where you stand, in the picture's pixels: at the shelter's edge, near the fire.
var listener_at := Vector2(210, 372)
var voices := Voices.new()
var code := ""
var maker: Object
var _rate := 22050
var _main: GDScript
var _now := 0.0
var _rng := RandomNumberGenerator.new()
var _syllables := PackedStringArray()
var _sets := {}
var _to_make: Array[Callable] = []
var _made := 0
var _players: Array[AudioStreamPlayer3D] = []
var _hums := {}
var _world: Node3D
var _listener: AudioListener3D
var _fire: Resource
var _wind: Resource
var _probe: AudioEffect
var _limiter: AudioEffectHardLimiter
var _cave_bus := -1
var _next_at: Array[float] = []
var _lit: Array[float] = []
var _hummed: Array[float] = []
var _person: Array[float] = []
## The makers whose talk is being made, and its task.
var _talking := {}
var _lineup: Array = []
var _lineup_at := 0.0
var _caption := ""
var _measuring := false
var _measure_t := 0.0
var _measured_from := false
var _counts: Array[int] = []
## The shares when the most sounded at once.
var _fullest := {}
var _heat := []
var _run := Report.RUN
var _reel := false
var _reel_t := 0.0
var _reel_stop := -1
var _walk_from := Vector2.ZERO
var _map := Rect2()
var _k := 1.0
var _dragging := false
var _status_clock := 0.0
var _top_bar: PanelContainer
var _bottom_bar: PanelContainer
var _top: VBoxContainer
var _title: Label
var _status: Label
var _bottom: VBoxContainer
var _hour_buttons := {}
var _busy_button: Button
var _lineup_button: Button
var _measure_button: Button


func _ready() -> void:
	_main = load("res://main.gd")
	_rng.seed = 14
	texture_filter = TEXTURE_FILTER_NEAREST
	mouse_filter = MOUSE_FILTER_STOP
	for arg: String in OS.get_cmdline_user_args():
		if arg.begins_with("seconds="):
			_run = maxf(1.0, arg.trim_prefix("seconds=").to_float())
	_build()
	resized.connect(_layout)
	_layout()
	for i in Camp.SOURCES.size():
		_next_at.append(-1.0)
		_lit.append(-1.0)
		_hummed.append(-1.0)
		_person.append(_rng.randf_range(0.94, 1.06))
	if not ClassDB.class_exists("SoundMaker"):
		_status.text = "This build has no C++ part, so nothing can sound."
		return
	maker = ClassDB.instantiate("SoundMaker")
	_rate = maker.call("rate")
	_audio()
	_plan_making()
	_status.text = "Making the camp's sounds…"
	# the cloud's picture waits until the camp has sounded a little while
	add_to_group("busy")


## The cave's echo bus, the probe on the master bus, and the 3D world: you, the 32 players and the
## shelter's area, whose sounds echo.
func _audio() -> void:
	_cave_bus = AudioServer.bus_count
	AudioServer.add_bus()
	AudioServer.set_bus_name(_cave_bus, "Cave")
	AudioServer.set_bus_send(_cave_bus, "Master")
	var reverb := AudioEffectReverb.new()
	reverb.room_size = 0.55
	reverb.damping = 0.5
	reverb.spread = 0.8
	reverb.predelay_msec = 35.0
	reverb.predelay_feedback = 0.3
	reverb.hipass = 0.15
	reverb.dry = 0.0
	reverb.wet = 1.0
	AudioServer.add_bus_effect(_cave_bus, reverb)
	# a limiter last but the probe, so 32 sounds at once never clip (SND-12: nothing harsh)
	_limiter = AudioEffectHardLimiter.new()
	_limiter.ceiling_db = -1.0
	_limiter.pre_gain_db = LIFT
	AudioServer.add_bus_effect(0, _limiter)
	if ClassDB.class_exists("AudioEffectProbe"):
		_probe = ClassDB.instantiate("AudioEffectProbe")
		_probe.call("set_rate", AudioServer.get_mix_rate())
		AudioServer.add_bus_effect(0, _probe)
	_world = Node3D.new()
	add_child(_world)
	_listener = AudioListener3D.new()
	_world.add_child(_listener)
	_listener.make_current()
	# Godot's 3D players reckon their volumes for each camera, so a world with no camera is
	# silent: one rides with you, drawing nothing
	var camera := Camera3D.new()
	camera.cull_mask = 0
	_listener.add_child(camera)
	camera.make_current()
	var cave := Area3D.new()
	cave.reverb_bus_enabled = true
	cave.reverb_bus_name = "Cave"
	cave.reverb_bus_amount = 0.7
	cave.reverb_bus_uniformity = 0.3
	var shape := CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(Camp.CAVE.size.x * Camp.METRE, 20.0, Camp.CAVE.size.y * Camp.METRE)
	shape.shape = box
	cave.add_child(shape)
	cave.position = Camp.world(Camp.CAVE.get_center())
	_world.add_child(cave)
	for i in PLAYERS:
		var p := AudioStreamPlayer3D.new()
		p.attenuation_model = AudioStreamPlayer3D.ATTENUATION_INVERSE_DISTANCE
		p.doppler_tracking = AudioStreamPlayer3D.DOPPLER_TRACKING_DISABLED
		p.attenuation_filter_cutoff_hz = OPEN_HZ
		p.set_meta("busy", false)
		_world.add_child(p)
		_players.append(p)


## What to make as the screen opens, one piece a frame: the banks, the language, each maker's
## sounds, a few of each so no two plays are the same (SND-06), and the hums.
func _plan_making() -> void:
	for voice in BANKS.size():
		_to_make.append(
			func() -> void:
				maker.call("load_bank", voice, FileAccess.get_file_as_bytes(BANKS[voice]))
		)
	_to_make.append(func() -> void: _syllables = maker.call("language", LANGUAGE, 14))
	var keys := {}
	for source: Dictionary in Camp.SOURCES:
		var key := _key(source)
		if key.is_empty() or key in keys:
			continue
		keys[key] = true
		var kind: String = source["kind"]
		var count := 1 if kind in LOOPS else (3 if kind in ["howl", "bird"] else 6)
		for v in count:
			_to_make.append(_make_one.bind(key, source, v))
	for v in 6:
		_to_make.append(_make_thunder.bind(v))
	_to_make.append(_make_hums)


## The key of a maker's set of sounds: its kind, its stuff and its seed; none for what is made live.
static func _key(source: Dictionary) -> String:
	var kind: String = source["kind"]
	if kind in ["fire", "wind", "talk", "child", "thunder"]:
		return ""
	return "%s %s %d" % [kind, str(source.get("stuff", [])), int(source.get("seed", 0))]


func _make_one(key: String, source: Dictionary, v: int) -> void:
	var kind: String = source["kind"]
	var stuff: Array = source.get("stuff", [0.5, 0.5, 0.0])
	var seed := int(source.get("seed", 0)) * 100 + v + 1
	if kind == "drum":
		# a drummer: a loop of hits on one beat for all, to its own pattern
		var hits := []
		for h in 3:
			hits.append(
				maker.call("make", BASES.find("drum"), stuff[0], stuff[1], stuff[2], seed + h)
			)
		_add(key, _wav(maker.call("rhythm", hits, 8.0, 0.5, seed), true))
		return
	var pcm: PackedByteArray = maker.call(
		"make", BASES.find(kind), stuff[0], stuff[1], stuff[2], seed
	)
	_add(key, _wav(pcm, maker.call("loops", BASES.find(kind))))


## Thunder near and far: the near with its crack.
func _make_thunder(v: int) -> void:
	var near := v < 3
	var pcm: PackedByteArray = maker.call(
		"make", BASES.find("thunder"), 0.5, 0.9 if near else 0.35, 0.0, v + 1
	)
	_add("thunder near" if near else "thunder far", _wav(pcm))


## The hums the quietest sounds join (SND-07): talk from many in the camp, and its work.
func _make_hums() -> void:
	var talk := []
	var whos := ["man", "woman", "old man", "child", "girl", "big man"]
	for k in 12:
		var who: Array = Camp.SPEAKERS[whos[k % whos.size()]]
		talk.append(maker.call("phrase", who[0], _syllables, who[1], {}, 900 + k))
	_add("hum voices", _wav(maker.call("blend", talk, 8.0, 1.6, 3), true))
	var work := []
	for key: String in _sets:
		if key.begins_with("strike") or key.begins_with("scrape") or key.begins_with("chop"):
			for s: AudioStreamWAV in _sets[key]:
				work.append(s.data)
	_add("hum work", _wav(maker.call("blend", work, 6.0, 7.0, 4), true))


func _add(key: String, stream: AudioStream) -> void:
	if not _sets.has(key):
		_sets[key] = []
	(_sets[key] as Array).append(stream)


func _wav(pcm: PackedByteArray, loop := false) -> AudioStreamWAV:
	var w := AudioStreamWAV.new()
	w.format = AudioStreamWAV.FORMAT_16_BITS
	w.mix_rate = _rate
	w.stereo = false
	w.data = pcm
	if loop:
		w.loop_mode = AudioStreamWAV.LOOP_FORWARD
		w.loop_begin = 0
		w.loop_end = pcm.size() / 2
	return w


## The fire and the wind, made live.
func _live(kind: int, seed: int, amount: float) -> Resource:
	var stream: Resource = ClassDB.instantiate("SoundLive")
	stream.set("kind", kind)
	stream.set("seed", seed)
	stream.set("amount", amount)
	return stream


func _process(delta: float) -> void:
	if maker == null:
		return
	if not _to_make.is_empty():
		# a few pieces a frame, so the screen keeps drawing
		var started := Time.get_ticks_usec()
		while not _to_make.is_empty() and Time.get_ticks_usec() - started < 8000:
			(_to_make.pop_front() as Callable).call()
			_made += 1
		_status.text = "Making the camp's sounds… %d" % _made
		if _to_make.is_empty():
			_begin()
		return
	step(delta)


## Everything made: the fire and wind start, and the camp sounds.
func _begin() -> void:
	_fire = _live(0, 5, HOURS[hour][0])
	_wind = _live(1, 6, HOURS[hour][1])
	var args := OS.get_cmdline_user_args()
	# "hour=night" and "busy" on the command line, for the cloud's pictures
	for arg: String in args:
		if arg.begins_with("hour=") and arg.trim_prefix("hour=") in HOURS:
			hour = arg.trim_prefix("hour=")
	set_hour(hour)
	# Busy or Measure tapped while the sounds were being made still stand
	set_busy(busy or "busy" in args)
	if "reel" in args:
		_reel = true
		_reel_t = 0.0
		listener_at = Report.REEL[0][3]
	elif "measure" in args:
		measure()


## One step of the camp's time: you, the makers that are due, the sounds that ended, the hums,
## Measure, the reel, the voices one by one, and the map.
func step(delta: float) -> void:
	_now += delta
	if _now > 3.0 and is_in_group("busy"):
		remove_from_group("busy")
	if _reel:
		_reel_step(delta)
	_listener.position = Camp.world(listener_at)
	for i in Camp.SOURCES.size():
		if _next_at[i] >= 0.0 and _next_at[i] <= _now:
			var every: Vector2 = Camp.SOURCES[i].get("every", Vector2.ONE)
			var sooner := BUSY if busy else 1.0
			# Measure asks for more than the cap, so all 32 places stay full: every maker twice as
			# often, and thunder more often still for the two free places
			if _measuring:
				sooner *= 0.3 if Camp.SOURCES[i]["share"] == Voices.Share.FREE else 0.5
			_next_at[i] = _now + _rng.randf_range(every.x, every.y) * sooner
			_sound(i)
	_lineup_step()
	_follow()
	voices.expire(_now)
	_hum_update()
	if _measuring:
		_measure_step(delta)
	_status_clock -= delta
	if _status_clock <= 0.0:
		_status_clock = 0.25
		_show_status()
	queue_redraw()


## Sets the hour: what sounds, the fire's heat, the wind's speed.
func set_hour(h: String) -> void:
	hour = h
	for b: String in _hour_buttons:
		(_hour_buttons[b] as Button).button_pressed = b == h
	if _fire != null:
		_fire.set("amount", HOURS[hour][0])
		_wind.set("amount", HOURS[hour][1])
	_refresh()


## Everyone in the camp at once, whatever the hour: the load SND-01's cap is for.
func set_busy(on: bool) -> void:
	busy = on
	if _busy_button != null:
		_busy_button.button_pressed = on
	_refresh()


## Whether a maker sounds now: in its hours, or, with everyone at once, all but the weather and the
## birds, which keep their hours.
func active(i: int) -> bool:
	var source: Dictionary = Camp.SOURCES[i]
	if Camp.sounds_in(source, hour):
		return true
	return busy and not (source["kind"] in ["rain", "thunder", "bird", "wind"])


func _refresh() -> void:
	if _fire == null:
		return
	for i in Camp.SOURCES.size():
		var source: Dictionary = Camp.SOURCES[i]
		var on := active(i)
		if source["kind"] in LOOPS:
			var p := _player_of(i)
			if on and p == null:
				_start_loop(i)
			elif not on and p != null:
				_release(p, true)
		elif on and _next_at[i] < 0.0:
			# its first sound soon after the hour comes, so a wolf at night is heard at once
			var every: Vector2 = source.get("every", Vector2.ONE)
			_next_at[i] = _now + _rng.randf_range(0.0, minf(every.x, 4.0))
		elif not on:
			_next_at[i] = -1.0


func _start_loop(i: int) -> void:
	var source: Dictionary = Camp.SOURCES[i]
	var stream: AudioStream
	match source["kind"]:
		"fire":
			stream = _fire
		"wind":
			stream = _wind
		_:
			stream = (_sets.get(_key(source), [null]) as Array)[0]
	if stream != null:
		_play(i, stream, 0.0, 1.0)


## A maker sounds once.
func _sound(i: int) -> void:
	var source: Dictionary = Camp.SOURCES[i]
	var kind: String = source["kind"]
	match kind:
		"talk", "child":
			_say(i)
		"thunder":
			var near := _rng.randf() < 0.4
			var options: Array = _sets.get("thunder near" if near else "thunder far", [])
			if not options.is_empty():
				var turn := _rng.randf() * TAU
				var far := (
					_rng.randf_range(300.0, 900.0) if near else _rng.randf_range(1200.0, 3000.0)
				)
				var at := Vector3(cos(turn), 0.1, sin(turn)) * far
				_play(i, options[_rng.randi() % options.size()], -1.0, 1.0, at)
		_:
			var options: Array = _sets.get(_key(source), [])
			if not options.is_empty():
				var pitch := _rng.randf_range(0.94, 1.06)
				_play(i, options[_rng.randi() % options.size()], -1.0, pitch)
	if kind == "howl":
		# the dogs answer the wolf
		for j in Camp.SOURCES.size():
			if Camp.SOURCES[j].get("answers", "") == "wolf" and _next_at[j] >= 0.0:
				_next_at[j] = minf(_next_at[j], _now + _rng.randf_range(1.5, 4.0))


## Talk, or a child's laugh, call, cry or scream, made beside the game from the bank (SND-03).
func _say(i: int) -> void:
	if _talking.has(i):
		return
	var source: Dictionary = Camp.SOURCES[i]
	var who: Array = Camp.SPEAKERS[source["who"]]
	var speaker: Dictionary = (who[1] as Dictionary).duplicate()
	speaker["pitch"] = float(speaker.get("pitch", 1.0)) * _person[i]
	var feeling: String = source["feeling"]
	if hour == "night" and feeling == "calm":
		feeling = "hushed"
	var cry := -1
	if source["kind"] == "child":
		var r := _rng.randf()
		cry = 0 if r < 0.35 else (2 if r < 0.55 else (1 if r < 0.62 else (3 if r < 0.65 else -1)))
	_next_at[i] = _now + 600.0
	_talking[i] = WorkerThreadPool.add_task(
		_make_talk.bind(i, who[0], speaker, Camp.FEELINGS[feeling], _rng.randi(), cry)
	)


## On a worker thread: the phrase or cry, sent back to the screen's thread.
func _make_talk(
	i: int, bank: int, speaker: Dictionary, feeling: Dictionary, seed: int, cry: int
) -> void:
	var pcm: PackedByteArray
	if cry < 0:
		pcm = maker.call("phrase", bank, _syllables, speaker, feeling, seed)
	else:
		pcm = maker.call("cry", bank, cry, speaker, seed)
	_spoken.call_deferred(i, pcm, cry == 3)


func _spoken(i: int, pcm: PackedByteArray, scream: bool) -> void:
	# every task is waited for once; this one has all but returned
	if _talking.has(i):
		WorkerThreadPool.wait_for_task_completion(_talking[i])
		_talking.erase(i)
	var every: Vector2 = Camp.SOURCES[i].get("every", Vector2.ONE)
	var gap := _rng.randf_range(every.x, every.y) * (BUSY if busy else 1.0)
	_next_at[i] = _now + pcm.size() / 2.0 / _rate + gap if active(i) else -1.0
	if pcm.is_empty() or not active(i):
		return
	# a scream is sudden, and takes one of the places kept free
	_play(i, _wav(pcm), -1.0, 1.0, Vector3.INF, Voices.Share.FREE if scream else -1)


## Plays a stream for a maker (-1: the voices one by one, before you), through the voice manager:
## for `seconds` (below 0: the stream's length; 0: until stopped), at a pitch, at its place or at
## `at` in the world, in its share or `share`. The id the manager gave it, or 0 where it went to a
## hum.
func _play(
	i: int, stream: AudioStream, seconds: float, pitch: float, at := Vector3.INF, share := -1
) -> int:
	var source: Dictionary = _source(i)
	if share < 0:
		share = source["share"]
	var place := _place_of(i)
	var around := place == Camp.AROUND
	if at == Vector3.INF:
		at = Camp.world(listener_at if around else place)
	var volume: float = source["volume"] + (_rng.randf_range(-1.5, 1.5) if seconds != 0.0 else 0.0)
	var muffled := not around and Camp.muffled(listener_at, place)
	if seconds < 0.0:
		seconds = stream.get_length() / pitch
	var heard := _heard(at, volume + (MUFFLED_DB if muffled else 0.0), source["unit"])
	var answer := voices.ask(share, heard, seconds, _now)
	_stop_ids(answer["stop"])
	if answer["id"] == 0:
		if i >= 0:
			_hummed[i] = _now + maxf(seconds, 0.5)
		return 0
	var p := _free_player()
	if p == null:
		voices.done(answer["id"])
		return 0
	p.stream = stream
	p.position = at
	p.unit_size = source["unit"]
	p.pitch_scale = pitch
	p.volume_db = volume + (MUFFLED_DB if muffled else 0.0)
	p.attenuation_filter_cutoff_hz = MUFFLED_HZ if muffled else OPEN_HZ
	p.set_meta("busy", true)
	p.set_meta("id", answer["id"])
	p.set_meta("source", i)
	p.set_meta("volume", volume)
	p.set_meta("ends", _now + seconds if seconds > 0.0 else 0.0)
	# what was set down at its own place, as thunder, stays there
	p.set_meta("fixed", around and at != Camp.world(listener_at))
	p.play()
	if i >= 0:
		_lit[i] = _now + (seconds if seconds > 0.0 else 1e9)
	return answer["id"]


## A maker's details, or the voices' one by one: just before you.
func _source(i: int) -> Dictionary:
	if i >= 0:
		return Camp.SOURCES[i]
	return {"share": Voices.Share.VOICES, "volume": -2.0, "unit": 2.0, "kind": "talk"}


## Where a maker is now in the picture; Camp.AROUND for what is all round you, as wind and rain.
func _place_of(i: int) -> Vector2:
	if i < 0:
		return listener_at + Vector2(0.0, -6.0)
	var source: Dictionary = Camp.SOURCES[i]
	if Camp.around(source):
		return Camp.AROUND
	return Camp.where(source, _now)


## How loud a sound is where you are, as Godot's inverse-distance players make it: full within its
## unit distance (at most 3 dB more), falling with distance beyond.
func _heard(at: Vector3, volume_db: float, unit: float) -> float:
	var d := maxf(at.distance_to(Camp.world(listener_at)), 0.1)
	return db_to_linear(volume_db) * minf(1.41, unit / d)


func _free_player() -> AudioStreamPlayer3D:
	for p in _players:
		if not p.get_meta("busy"):
			return p
	return null


func _player_of(i: int) -> AudioStreamPlayer3D:
	for p in _players:
		if p.get_meta("busy") and p.get_meta("source", -9) == i:
			return p
	return null


func _stop_ids(ids: Array) -> void:
	for p in _players:
		if p.get_meta("busy") and ids.has(p.get_meta("id", 0)):
			if p.get_meta("source", -9) >= 0:
				_hummed[p.get_meta("source")] = _now + 1.0
			_release(p, false)


## Stops a player and frees it; tells the manager unless it already knows.
func _release(p: AudioStreamPlayer3D, tell: bool) -> void:
	p.stop()
	p.set_meta("busy", false)
	if tell and p.get_meta("id", 0) != 0:
		voices.done(p.get_meta("id"))
	var i: int = p.get_meta("source", -9)
	if i >= 0:
		_lit[i] = minf(_lit[i], _now)
	p.set_meta("id", 0)
	p.set_meta("source", -9)


## Each frame: sounds that ended let go; sounds that move, and those all round you, follow; the
## cliff muffles or not as you walk.
func _follow() -> void:
	var here := Camp.world(listener_at)
	for p in _players:
		if not p.get_meta("busy"):
			continue
		var ends: float = p.get_meta("ends", 0.0)
		if ends > 0.0 and ends <= _now:
			_release(p, true)
			continue
		var i: int = p.get_meta("source", -9)
		if i == -2:
			p.position = here
			continue
		if i < -1 or p.get_meta("fixed", false):
			continue
		var place := _place_of(i)
		if place == Camp.AROUND:
			p.position = here
			var sheltered := Camp.CAVE.has_point(listener_at)
			p.volume_db = float(p.get_meta("volume")) + (SHELTERED_DB if sheltered else 0.0)
			continue
		p.position = Camp.world(place)
		var muffled := Camp.muffled(listener_at, place)
		p.volume_db = float(p.get_meta("volume")) + (MUFFLED_DB if muffled else 0.0)
		p.attenuation_filter_cutoff_hz = MUFFLED_HZ if muffled else OPEN_HZ


## The hums: one player for each share whose blend holds something, as loud as all it holds.
func _hum_update() -> void:
	for share: int in Voices.BLENDED:
		var on := voices.humming(share)
		var p: AudioStreamPlayer3D = _hums.get(share)
		if on and p == null:
			p = _free_player()
			var hum: Array = _sets.get("hum " + Voices.NAMES[share], [])
			if p == null or hum.is_empty():
				continue
			p.stream = hum[0]
			p.unit_size = 1.0
			p.pitch_scale = 1.0
			p.attenuation_filter_cutoff_hz = OPEN_HZ
			p.set_meta("busy", true)
			p.set_meta("id", 0)
			p.set_meta("source", -2)
			p.set_meta("ends", 0.0)
			p.play()
			_hums[share] = p
		elif not on and p != null:
			_release(p, false)
			_hums.erase(share)
			continue
		if p != null:
			p.volume_db = linear_to_db(maxf(voices.hum(share), 0.0001)) - 3.0


## The players sounding now.
func sounding() -> int:
	var n := 0
	for p in _players:
		if p.get_meta("busy"):
			n += 1
	return n


## The voices one by one, a little before you (SND-03's check): a child, a woman, a man and an old
## man, then anger and grief.
func lineup() -> void:
	if maker == null or not _to_make.is_empty():
		return
	_lineup = []
	for entry: Array in Report.LINEUP:
		var who: Array = Camp.SPEAKERS[entry[0]]
		var pcm: PackedByteArray = maker.call(
			"phrase", who[0], _syllables, who[1], Camp.FEELINGS[entry[1]], 31
		)
		_lineup.append([_wav(pcm), entry[2]])
	_lineup_at = _now + 0.5


func _lineup_step() -> void:
	if _lineup.is_empty() or _now < _lineup_at:
		return
	var next: Array = _lineup.pop_front()
	_caption = next[1]
	var stream: AudioStreamWAV = next[0]
	_play(-1, stream, -1.0, 1.0)
	_lineup_at = _now + stream.get_length() + 0.9


## Plays everything at once in a storm for a minute and times the audio thread.
func measure() -> void:
	if _probe == null or maker == null:
		_status.text = "This build has no C++ part, so nothing can be measured."
		return
	_measuring = true
	_measure_t = 0.0
	_measured_from = false
	_counts = []
	_fullest = {}
	_heat = [RUNS.thermal()]
	set_hour("storm")
	set_busy(true)
	# the first thunder at once, not up to half a minute on
	for i in Camp.SOURCES.size():
		if Camp.SOURCES[i]["share"] == Voices.Share.FREE and _next_at[i] >= 0.0:
			_next_at[i] = _now + 0.5
	_measure_button.disabled = true


func _measure_step(delta: float) -> void:
	_measure_t += delta
	if not _measured_from and _measure_t >= Report.SETTLE:
		_measured_from = true
		_probe.call("reset")
	if _measured_from:
		_counts.append(sounding())
		if _counts[-1] >= int(_fullest.get("total", 0)):
			_fullest = {"total": _counts[-1]}
			for share: int in Voices.Share.values():
				_fullest[Voices.NAMES[share]] = voices.used(share)
	if _measure_t >= Report.SETTLE + _run:
		_measured()


func _measured() -> void:
	_measuring = false
	_measure_button.disabled = false
	_heat.append(RUNS.thermal())
	var stats: Dictionary = _probe.call("stats")
	var head := (
		"P14 %s %s"
		% [ProjectSettings.get_setting("application/config/version", ""), _main.facts().phone]
	)
	code = Report.line(
		head,
		stats,
		_counts,
		_fullest,
		AudioServer.get_mix_rate(),
		AudioServer.get_output_latency(),
		_heat
	)
	DisplayServer.clipboard_set(code)
	print(code)
	_caption = Report.verdict(stats) + " Copied for the chat:\n" + code
	if "measure" in OS.get_cmdline_user_args():
		get_tree().quit()


## The reel: the hour, everyone at once, where you walk and the caption, stop by stop; it ends the
## run when it is done.
func _reel_step(delta: float) -> void:
	_reel_t += delta
	var stop := 0
	for k in Report.REEL.size():
		if _reel_t >= Report.REEL[k][0]:
			stop = k
	if stop != _reel_stop:
		_reel_stop = stop
		var entry: Array = Report.REEL[stop]
		if (entry[1] as String).is_empty():
			get_tree().quit()
			return
		_walk_from = listener_at
		_caption = entry[4]
		if entry[1] != hour:
			set_hour(entry[1])
		if entry[2] != busy:
			set_busy(entry[2])
		if entry[5]:
			lineup()
	var target: Vector2 = Report.REEL[stop][3]
	var walked := clampf((_reel_t - float(Report.REEL[stop][0])) / 8.0, 0.0, 1.0)
	listener_at = _walk_from.lerp(target, smoothstep(0.0, 1.0, walked))


func _show_status() -> void:
	var shares := PackedStringArray()
	for share: int in [
		Voices.Share.PLACE, Voices.Share.VOICES, Voices.Share.WORK, Voices.Share.FREE
	]:
		shares.append("%s %d" % [Voices.NAMES[share], voices.used(share)])
	var hums := PackedStringArray()
	for share: int in Voices.BLENDED:
		var held: int = (voices.blends[share] as Array).size()
		if held > 0:
			hums.append("%s %d" % [Voices.NAMES[share], held])
	var text := "%s · %d of 32 sounds: %s" % [hour.capitalize(), sounding(), ", ".join(shares)]
	if not hums.is_empty():
		text += " · in the hums: " + ", ".join(hums)
	# recording a movie, Godot mixes on the main thread between frames, so the probe's figure means
	# nothing there
	if _probe != null and Engine.get_write_movie_path().is_empty():
		var stats: Dictionary = _probe.call("stats")
		if int(stats.get("blocks", 0)) > 0:
			text += (
				" · audio thread %.1f%%, worst %d%%"
				% [
					float(stats.get("mean", 0.0)) * 100.0,
					roundi(float(stats.get("worst", 0.0)) * 100.0)
				]
			)
	if _measuring:
		text += (
			"\nMeasuring: %d s left. Keep the phone as it is."
			% ceili(Report.SETTLE + _run - _measure_t)
		)
	if not _caption.is_empty():
		text += "\n" + _caption
	_status.text = text


func _build() -> void:
	_top_bar = _bar(PRESET_TOP_WIDE)
	_top = VBoxContainer.new()
	_top.add_theme_constant_override("separation", 2)
	_top_bar.add_child(_top)
	_title = _label("P14 Sound", FLAME, 20)
	_top.add_child(_title)
	_status = _label("", INK, 13)
	_top.add_child(_status)
	_top.add_child(
		_label(
			"Touch the camp to stand there; drag to walk. Headphones show direction best.", DIM, 12
		)
	)
	_bottom_bar = _bar(PRESET_BOTTOM_WIDE)
	_bottom_bar.grow_vertical = GROW_DIRECTION_BEGIN
	_bottom = VBoxContainer.new()
	_bottom.add_theme_constant_override("separation", GAP)
	_bottom_bar.add_child(_bottom)
	var hours := HBoxContainer.new()
	hours.add_theme_constant_override("separation", GAP)
	_bottom.add_child(hours)
	for h: String in HOURS:
		var b := _button(h.capitalize(), hours)
		b.toggle_mode = true
		b.pressed.connect(set_hour.bind(h))
		_hour_buttons[h] = b
	var row := HBoxContainer.new()
	row.add_theme_constant_override("separation", GAP)
	_bottom.add_child(row)
	_busy_button = _button("Busy", row)
	_busy_button.toggle_mode = true
	_busy_button.toggled.connect(set_busy)
	_lineup_button = _button("Voices", row)
	_lineup_button.pressed.connect(lineup)
	_measure_button = _button("Measure", row)
	_measure_button.pressed.connect(measure)
	var back := _button("Back", row)
	back.pressed.connect(func() -> void: closed.emit())


## A bar across the screen's top or foot, behind the words or the buttons, sized to them.
func _bar(preset: LayoutPreset) -> PanelContainer:
	var bar := PanelContainer.new()
	var style := StyleBoxFlat.new()
	style.bg_color = PANEL
	style.set_content_margin_all(GAP)
	bar.add_theme_stylebox_override("panel", style)
	add_child(bar)
	bar.set_anchors_and_offsets_preset(preset)
	return bar


func _button(text: String, row: HBoxContainer) -> Button:
	var b := Button.new()
	b.text = text
	b.custom_minimum_size = Vector2(0, TARGET)
	b.size_flags_horizontal = SIZE_EXPAND_FILL
	row.add_child(b)
	return b


func _label(text: String, colour: Color, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_color_override("font_color", colour)
	label.add_theme_font_size_override("font_size", font_size)
	return label


## The picture as large as whole screen pixels allow, the words above it and the buttons below.
func _layout() -> void:
	var inset: Vector4i = _main.insets(size)
	var window := DisplayServer.window_get_size()
	var per_pixel := size.x / maxf(float(window.x), 1.0)
	var scale_px := floori(minf(window.x / 336.0, window.y / 748.0)) if window.x > 0 else 1
	_k = maxf(1.0, float(scale_px)) * per_pixel if window.x > 0 else size.x / 336.0
	if 336.0 * _k > size.x or 748.0 * _k > size.y:
		_k = minf(size.x / 336.0, size.y / 748.0)
	var picture := Vector2(336.0, 748.0) * _k
	_map = Rect2((size - picture) / 2.0, picture)
	# the bars keep their words and buttons inside the safe area
	for bar: PanelContainer in [_top_bar, _bottom_bar]:
		var style := bar.get_theme_stylebox("panel") as StyleBoxFlat
		style.content_margin_left = GAP + inset.x
		style.content_margin_right = GAP + inset.z
	(_top_bar.get_theme_stylebox("panel") as StyleBoxFlat).content_margin_top = GAP + inset.y
	(_bottom_bar.get_theme_stylebox("panel") as StyleBoxFlat).content_margin_bottom = GAP + inset.w


func _draw() -> void:
	draw_rect(Rect2(Vector2.ZERO, size), NIGHT)
	draw_texture_rect(PICTURE, _map, false, HOURS[hour][2])
	# the shelter where sounds echo
	var cave := Rect2(_at(Camp.CAVE.position), Camp.CAVE.size * _k)
	draw_rect(cave, Color(0.7, 0.8, 1.0, 0.35), false, 1.0)
	for i in Camp.SOURCES.size():
		var place := _place_of(i)
		if not Rect2(0, 0, 336, 748).has_point(place) or not active(i):
			continue
		var colour: Color = SHARE_COLOURS[Camp.SOURCES[i]["share"]]
		var at := _at(place)
		if _lit[i] > _now:
			draw_circle(at, 3.0 * _k, colour)
			draw_arc(at, (3.0 + 3.0 * fmod(_now * 2.0, 1.0)) * _k, 0.0, TAU, 16, colour, 1.0)
		elif _hummed[i] > _now:
			draw_arc(at, 2.5 * _k, 0.0, TAU, 12, colour, 1.0)
		else:
			draw_circle(at, 1.2 * _k, colour.darkened(0.3))
	var you := _at(listener_at)
	draw_arc(you, 6.0 * _k, 0.0, TAU, 24, Color.WHITE, 2.0)
	draw_line(you + Vector2(0, -9) * _k, you + Vector2(0, -4) * _k, Color.WHITE, 2.0)


func _at(p: Vector2) -> Vector2:
	return _map.position + p * _k


func _gui_input(event: InputEvent) -> void:
	var press := event as InputEventMouseButton
	if press != null and press.button_index == MOUSE_BUTTON_LEFT:
		_dragging = press.pressed and _map.has_point(press.position)
		if _dragging:
			_stand(press.position)
		accept_event()
	var motion := event as InputEventMouseMotion
	if motion != null and _dragging:
		_stand(motion.position)
		accept_event()


func _stand(at: Vector2) -> void:
	_reel = false
	listener_at = ((at - _map.position) / _k).clamp(Vector2.ZERO, Vector2(335, 747))


func _exit_tree() -> void:
	for i: int in _talking:
		WorkerThreadPool.wait_for_task_completion(_talking[i])
	_talking.clear()
	for p in _players:
		p.stop()
	for effect: AudioEffect in [_probe, _limiter]:
		for k in range(AudioServer.get_bus_effect_count(0) - 1, -1, -1):
			if effect != null and AudioServer.get_bus_effect(0, k) == effect:
				AudioServer.remove_bus_effect(0, k)
	if _cave_bus >= 0 and _cave_bus < AudioServer.bus_count:
		AudioServer.remove_bus(_cave_bus)
