## P5 The same bits (IMPLEMENTATION α0.4a): the toy world of prototypes/samebits, run by this
## phone's own chip through its Godot extension, on one thread and on four, its digest set beside
## the one the cloud recorded on x86-64; arm64 under qemu is checked in the cloud. Every digest the
## same means the simulation can be trusted to end a world identically everywhere (RES-05, TIM-16).
## Run copies one line for the chat. Pre-production code (research 00): prototypes/samebits's
## README names the items it is about.
extends Control

signal closed

const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const GAP := 12
## The thread counts each run uses.
const THREADS := [1, 4]

## The run's digests by thread count, and the line for the chat once all are in.
var results := {}
var code := ""
var _status: Label
var _run: Button
var _worker: Thread


func _ready() -> void:
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
	_text(column, "P5 The same bits", FLAME, 22)
	_text(
		column,
		(
			"A small world of walkers runs here on your phone's chip, on one thread and then on four, "
			+ "and each run's fingerprint is set beside the one the cloud's computer made. "
			+ "If all three match, the game's worlds will run the same everywhere."
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
	if not ClassDB.class_exists("SameBits"):
		_status.text = "This build has no C++ part, so nothing can run."
		_run.disabled = true
	elif "run" in OS.get_cmdline_user_args():
		run.call_deferred()


## Runs the world on each thread count in turn, off the screen's thread, so the screen keeps
## drawing.
func run() -> void:
	if _worker != null and _worker.is_alive():
		return
	results = {}
	code = ""
	_run.disabled = true
	_status.text = "Running on 1 thread, then 4: a few seconds each…"
	_worker = Thread.new()
	_worker.start(_work)


func _work() -> void:
	for threads: int in THREADS:
		var started := Time.get_ticks_msec()
		var out: PackedStringArray = ClassDB.class_call_static("SameBits", "run", threads, _days())
		_done.call_deferred(threads, out[0], Time.get_ticks_msec() - started)


func _done(threads: int, digest: String, ms: int) -> void:
	results[threads] = [digest, ms]
	if results.size() < THREADS.size():
		return
	_worker.wait_to_finish()
	_run.disabled = false
	var cloud: String = ClassDB.class_call_static("SameBits", "cloud_digest")
	code = line(results, cloud, ProjectSettings.get_setting("application/config/version", ""))
	var same := true
	for t: int in THREADS:
		same = same and results[t][0] == cloud
	_status.text = (
		"%s\n\nCopied for the chat:\n%s"
		% [
			"All the same: the phone, the cloud, one thread and four." if same else "Not the same.",
			code
		]
	)
	DisplayServer.clipboard_set(code)


## The line for the chat: each thread count's digest and time, the cloud's digest, and the verdict.
static func line(by_threads: Dictionary, cloud: String, version: String) -> String:
	var parts := PackedStringArray(["P5 %s" % version if version != "" else "P5"])
	var same := true
	for t: int in by_threads:
		var r: Array = by_threads[t]
		parts.append("%d thread%s %s %.1f s" % [t, "" if t == 1 else "s", r[0], r[1] / 1000.0])
		same = same and r[0] == cloud
	parts.append("cloud %s" % cloud)
	parts.append("same" if same else "DIFFERENT")
	return " | ".join(parts)


func _days() -> int:
	return ClassDB.class_call_static("SameBits", "days")


func _exit_tree() -> void:
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
