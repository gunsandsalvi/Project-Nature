## Three reusable stand-in sounds follow sampled work/fire, never timers that invent an action.
extends Node

const Preferences := preload("res://ui/preferences.gd")
const Drawing := preload("res://camp/drawing.gd")
const VOICES := 4
var cue_count := 0
var last_cue := ""
var last_db := -80.0
var _work: Array[AudioStreamPlayer] = []
var _fire: AudioStreamPlayer
var _clips := {}
var _phases := {}
var _voice := 0
var _last_cue := 0


func _ready() -> void:
	for key: String in ["tap", "work", "fire"]:
		_clips[key] = _clip(key)
	for i in VOICES:
		var player := AudioStreamPlayer.new()
		add_child(player)
		_work.append(player)
	_fire = AudioStreamPlayer.new()
	_fire.stream = _clips.fire
	add_child(_fire)


static func _clip(key: String) -> AudioStreamWAV:
	var frames := 11025 if key == "fire" else 6615 if key == "work" else 2205
	var bytes := PackedByteArray()
	bytes.resize(frames * 2)
	var noise := 17
	for i in frames:
		noise = (noise * 1664525 + 1013904223) & 0xffffffff
		var sample := float(noise & 65535) / 32768.0 - 1.0
		var envelope := 0.08 if key == "fire" else pow(1.0 - float(i) / frames, 3.0) * 0.20
		if key == "tap":
			sample = sample * 0.5 + sin(float(i) * 0.37) * 0.5
		bytes.encode_s16(i * 2, roundi(sample * envelope * 32767))
	var clip := AudioStreamWAV.new()
	clip.format = AudioStreamWAV.FORMAT_16_BITS
	clip.mix_rate = 22050
	clip.data = bytes
	if key == "fire":
		clip.loop_mode = AudioStreamWAV.LOOP_FORWARD
		clip.loop_end = frames
	return clip


func _db(at: Vector2i, camera: KdCanvas, state: Dictionary) -> float:
	var local: Vector2 = camera.world_local(at.x, at.y)
	var distance := local.distance_to(Vector2(state.get("camera_centre", Vector2.ZERO)))
	var zoom := minf(1.0, float(state.get("physical_density", 16)) / 16.0)
	return -80.0 if distance >= 32 else -14.0 - distance * 1.5 + linear_to_db(maxf(0.01, zoom))


func update(people: Array, items: Array, camera: KdCanvas, state: Dictionary, paused: bool) -> void:
	var audible := not paused and not Preferences.value("mute")
	var next := {}
	var candidates: Array[Dictionary] = []
	for person: Dictionary in people:
		var id := int(person.id)
		var phase := Drawing.work_phase(person, float(state.get("second", 0)))
		var key := (
			"%d:%d:%d"
			% [int(person.get("action_code", 0)), int(person.get("action_start", 0)), phase]
		)
		next[id] = key
		if not audible or not _phases.has(id) or str(_phases[id]) == key:
			continue
		if int(person.get("action_code", 0)) not in [8, 10, 12]:
			continue
		var db := _db(Vector2i(person.east_cm, person.north_cm), camera, state)
		if db > -65:
			candidates.append(
				{
					"id": id,
					"db": db,
					"key": "tap" if int(person.get("work_action", 0)) == 2 else "work"
				}
			)
	_phases = next
	candidates.sort_custom(
		func(a: Dictionary, b: Dictionary) -> bool:
			return (
				float(a.db) > float(b.db) if float(a.db) != float(b.db) else int(a.id) < int(b.id)
			)
	)
	if not candidates.is_empty() and Time.get_ticks_msec() - _last_cue >= 80:
		_last_cue = Time.get_ticks_msec()
		var cue: Dictionary = candidates[0]
		var player := _work[_voice]
		_voice = (_voice + 1) % VOICES
		player.stream = _clips[cue.key]
		player.volume_db = float(cue.db)
		player.play()
		cue_count += 1
		last_cue = str(cue.key)
		last_db = float(cue.db)
	var fire_db := -80.0
	if audible:
		for item: Dictionary in items:
			if int(item.get("fire_heat", 0)) >= 2:
				fire_db = maxf(
					fire_db, _db(Vector2i(item.east_cm, item.north_cm), camera, state) - 8
				)
	if fire_db > -65:
		_fire.volume_db = fire_db
		if not _fire.playing:
			_fire.play()
	else:
		_fire.stop()
	if not audible:
		for player: AudioStreamPlayer in _work:
			player.stop()
