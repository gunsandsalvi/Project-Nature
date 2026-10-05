## P6 A thousand minds (IMPLEMENTATION α0.4b): the minds of prototypes/minds, run by this phone's
## own cores through its Godot extension: a thousand people, on four threads, for ten minutes, a
## game day at a time, off the screen's thread. The speed is read once the phone has warmed, in the
## last eight minutes, as game years a real minute (TIM-07), with each part of the minds' share of
## the time (MND-15) and the phone's heat. Run copies one line for the chat. Pre-production code
## (research 00): prototypes/minds's README names the items it is about.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const GAP := 12
const THREADS := 4
## The run, and the warming before its speed counts, in real minutes; "minutes=" on the command line
## shortens the run for the cloud.
const MINUTES := 10.0
const WARM := 2.0
## The parts of the minds, as the extension names them, and as the line names them.
const PARTS := [
	["choice", "choice"],
	["paths", "paths"],
	["talk", "talk"],
	["results", "results"],
	["other", "other"]
]

## Each game day's end: real seconds since the run began, and the days run.
var marks: Array[Vector2] = []
var code := ""
var _minutes := MINUTES
var _status: Label
var _run: Button
var _worker: Thread
var _stop := false
var _heat := []
var _read_clock := 0.0
var _started := 0


func _ready() -> void:
	for arg: String in OS.get_cmdline_user_args():
		if arg.begins_with("minutes="):
			_minutes = maxf(0.05, arg.trim_prefix("minutes=").to_float())
	var background := ColorRect.new()
	background.color = NIGHT
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(background)
	var margin := MarginContainer.new()
	margin.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	for side in ["left", "top", "right", "bottom"]:
		margin.add_theme_constant_override("margin_" + side, GAP)
	add_child(margin)
	var column := VBoxContainer.new()
	column.add_theme_constant_override("separation", GAP)
	margin.add_child(column)
	_text(column, "P6 A thousand minds", FLAME, 22)
	_text(
		column,
		(
			"A thousand people in 40 bands live on your phone's own cores: each feels hunger, thirst, "
			+ "tiredness and the rest, chooses among 50 things to do, talks, and finds its way. "
			+ "Run lets them live for ten minutes on four cores and measures how many game years pass "
			+ "in a real minute once the phone has warmed. Put the phone down until it is done."
		),
		INK,
		15
	)
	_status = _text(column, "", INK, 15)
	_status.size_flags_vertical = SIZE_EXPAND_FILL
	_run = Button.new()
	_run.text = "Run"
	_run.custom_minimum_size.y = 48
	_run.pressed.connect(run)
	column.add_child(_run)
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size.y = 48
	back.pressed.connect(func() -> void: closed.emit())
	column.add_child(back)
	if not ClassDB.class_exists("Minds"):
		_status.text = "This build has no C++ part, so nothing can run."
		_run.disabled = true
	elif "run" in OS.get_cmdline_user_args():
		run.call_deferred()


## Starts the run on its own thread, so the screen keeps drawing and the heat can be read.
func run() -> void:
	if _worker != null and _worker.is_alive():
		return
	marks = []
	code = ""
	_heat = [RUNS.thermal()]
	_read_clock = 0.0
	_stop = false
	_run.disabled = true
	_started = Time.get_ticks_msec()
	_status.text = "Making the world…"
	_worker = Thread.new()
	_worker.start(_work)


func _process(delta: float) -> void:
	if _worker == null or not _worker.is_alive():
		return
	_read_clock += delta
	if _read_clock >= RUNS.READ_EVERY:
		_read_clock = 0.0
		_heat.append(RUNS.thermal())


func _work() -> void:
	var minds: RefCounted = ClassDB.instantiate("Minds")
	var made: float = minds.make(THREADS)
	var start := Time.get_ticks_usec()
	var days := 0
	var end_at := _minutes * 60.0
	while not _stop:
		minds.run_days(1)
		days += 1
		var now := (Time.get_ticks_usec() - start) / 1.0e6
		_mark.call_deferred(Vector2(now, days))
		if now >= end_at:
			break
	if not _stop:
		_done.call_deferred(minds.stats(), minds.checksum(), minds.explain(0), made)


func _mark(mark: Vector2) -> void:
	marks.append(mark)
	if marks.size() % 10 == 1:
		_status.text = (
			"Running, %d of %d minutes: %d game days, %.2f game years a minute so far."
			% [int(mark.x / 60.0), int(_minutes), int(mark.y), mark.y / maxf(mark.x, 0.001)]
		)


func _done(stats: Dictionary, checksum: String, someone: String, made: float) -> void:
	_worker.wait_to_finish()
	_run.disabled = false
	_heat.append(RUNS.thermal())
	var main: GDScript = load("res://main.gd")
	var head := (
		"P6 %s %s"
		% [ProjectSettings.get_setting("application/config/version", ""), main.facts().phone]
	)
	code = line(head, marks, stats, _minutes, checksum, _heat)
	var held := held_speed(marks, WARM * _minutes / MINUTES)
	_status.text = (
		(
			"%s game years a real minute once warm (the aim: at least 1, hoping for 2 to 3).\n\n"
			+ "Made in %.1f s. Someone's choice: %s.\n\nCopied for the chat:\n%s"
		)
		% ["%.2f" % held, made, someone, code]
	)
	DisplayServer.clipboard_set(code)
	print(code)
	if "measure" in OS.get_cmdline_user_args():
		get_tree().quit()


## Game years a real minute after warming, in the run's days: a year is 60 days, so years a minute
## are days a second.
static func held_speed(days: Array[Vector2], warm_minutes: float) -> float:
	if days.is_empty():
		return 0.0
	var from := Vector2.ZERO
	for m: Vector2 in days:
		if m.x <= warm_minutes * 60.0:
			from = m
	var last: Vector2 = days[-1]
	if last.x <= from.x:
		return last.y / maxf(last.x, 0.001)
	return (last.y - from.y) / (last.x - from.x)


## The line for the chat: the held speed and the first minutes', the threads and people, each part's
## share of the minds' time, the decisions a game day, the heat, and the checksum.
static func line(
	head: String,
	days: Array[Vector2],
	stats: Dictionary,
	minutes: float,
	checksum: String,
	heat: Array
) -> String:
	var warm := WARM * minutes / MINUTES
	var held := held_speed(days, warm)
	var first := 0.0
	for m: Vector2 in days:
		if m.x <= warm * 60.0:
			first = m.y / maxf(m.x, 0.001)
	var total := 0.0
	for part: Array in PARTS:
		total += float(stats.get(part[0], 0.0))
	var shares := PackedStringArray()
	for part: Array in PARTS:
		shares.append(
			(
				"%s %d%%"
				% [part[1], roundi(100.0 * float(stats.get(part[0], 0.0)) / maxf(total, 1e-9))]
			)
		)
	var run_days := maxi(int(stats.get("days", 0)), 1)
	var runs := PackedStringArray(
		[
			"held %.2f game years a minute" % held,
			"warming %.2f" % first,
			"%d threads, %d people" % [THREADS, int(stats.get("people", 0))],
			" ".join(shares),
			"%d decisions a game day" % roundi(float(stats.get("decisions", 0)) / run_days),
			"sum %s" % checksum,
		]
	)
	return RUNS.code(head, runs, heat)


func _exit_tree() -> void:
	_stop = true
	if _worker != null and _worker.is_started():
		_worker.wait_to_finish()


func _text(box: VBoxContainer, text: String, colour: Color, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_color_override("font_color", colour)
	label.add_theme_font_size_override("font_size", font_size)
	box.add_child(label)
	return label
