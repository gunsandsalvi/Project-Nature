## Separate ten-minute measurement of the rendered living camp; never opens the owner's camp.
extends "res://pages/camp.gd"

const PHASE_SECONDS := 200.0
var time_scale := 1.0
var samples: Array = []
var phases: Array = []
var report := ""
var _device := KdDevice.new()
var _running := false
var _started := 0
var _phase := -1
var _counting := false
var _next_read := 0.0
var _phase_info := {}
var _backgrounded := 0


func _ready() -> void:
	root = "user://camp-measure"
	Worlds.remove_tree(root)
	frozen = true
	super._ready()
	for button: Button in [_dream_button, _more]:
		for connection: Dictionary in button.pressed.get_connections():
			button.pressed.disconnect(connection.callable)
	_dream_button.pressed.connect(start_measurement)
	_more.pressed.connect(func() -> void: DisplayServer.clipboard_set(report))
	_refresh_records()


func _refresh_records() -> void:
	super._refresh_records()
	_pause.disabled = true
	_speed.disabled = true
	_more.text = "Copy report"
	_more.disabled = report.is_empty()
	_dream_button.text = "Measuring…" if _running else "Run camp test"
	_dream_button.disabled = _running or not report.is_empty()
	if not report.is_empty():
		_card.text = "Finished. Copy report, then return to your camp through Menu.\n\n" + report
	elif _running:
		var elapsed := (Time.get_ticks_usec() - _started) / 1000000.0 * time_scale
		_card.text = (
			(
				"Camp measurement · %d / 600 seconds\nSpeed: %s\n"
				+ "Leave the phone alone. Turn once if wanted.\nYour saved camp is separate."
			)
			% [
				mini(600, int(elapsed)),
				["real time", "1 min / sec", "1 hour / sec"][maxi(0, _phase)]
			]
		)
	else:
		_card.text = (
			"Camp performance test · 10 minutes\n\n"
			+ "Unplug, use flight mode and let the phone cool.\nLeave this camp running. "
			+ "It measures frames, memory, heat, battery and dream latency at three speeds.\n"
			+ "Your own camp stays saved separately."
		)


func start_measurement() -> void:
	if _running or not report.is_empty():
		return
	_started = Time.get_ticks_usec()
	_running = true
	set_process(true)


func _process(delta: float) -> void:
	super._process(delta)
	if not _running:
		return
	var elapsed := (Time.get_ticks_usec() - _started) / 1000000.0 * time_scale
	var phase := mini(2, int(elapsed / PHASE_SECONDS))
	if phase != _phase:
		if _phase >= 0:
			_finish_phase()
		_start_phase(phase)
	if elapsed - _phase * PHASE_SECONDS >= 5 and not _counting:
		_device.frames_reset(1000.0 / 60, DisplayServer.screen_get_refresh_rate())
		_counting = true
	if elapsed >= _next_read:
		_next_read = elapsed + 5
		samples.append(
			{
				"seconds": elapsed,
				"phase": _phase,
				"memory": _device.memory(),
				"thermal": _device.thermal(),
				"battery": _device.battery_supply(),
				"clocks": Array(_device.clocks()),
				"threads": _device.thread_times(),
				"camp": world.counters()
			}
		)
	if elapsed >= 600:
		_finish_phase()
		_running = false
		world.pause()
		report = JSON.stringify(
			{
				"kind": "living-camp",
				"version": ProjectSettings.get_setting("application/config/version"),
				"platform": OS.get_name(),
				"real_seconds": (Time.get_ticks_usec() - _started) / 1000000.0,
				"time_scale": time_scale,
				"background_interruptions": _backgrounded,
				"window": str(get_viewport().get_visible_rect().size),
				"phases": phases,
				"samples": samples,
				"digest": world.digest()
			},
			"\t"
		)
		var file := FileAccess.open("user://camp-measurement.json", FileAccess.WRITE)
		if file != null:
			file.store_string(report)
			file.close()
		_refresh_records()


func _start_phase(phase: int) -> void:
	_phase = phase
	_counting = false
	choose_speed(phase)
	var started := Time.get_ticks_usec()
	var settled: Dictionary = world.prepare_dream()
	var result := {"problem": "No remembered place available"}
	for person: Dictionary in world.people():
		var choices: Array = world.dream_subjects(int(person.id))
		if not choices.is_empty() and str(choices[0].problem).is_empty():
			result = world.send_place_dream(int(person.id), int(choices[0].subject))
			break
	_phase_info = {
		"speed": [1, 60, 3600][phase],
		"start_frontier": world.frontier(),
		"dream_latency_ms": (Time.get_ticks_usec() - started) / 1000.0,
		"settled": settled,
		"request": result
	}
	world.play()


func _finish_phase() -> void:
	_phase_info["frames"] = _device.frames() if _counting else {}
	_phase_info["end_frontier"] = world.frontier()
	phases.append(_phase_info)


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_PAUSED and _running:
		_backgrounded += 1
	super._notification(what)
