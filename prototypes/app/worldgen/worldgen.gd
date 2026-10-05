## P7 World generation (IMPLEMENTATION α0.5a): "New world" as the game will make it, on this
## phone's own cores through its Godot extension: twenty rough candidates, the best four made again
## at full size, the best three offered with their maps, then the first settled for ten years with
## no people (WLD-08, WLD-10). Each part is timed against WLD-11's 3 minutes, and 1 more for
## settling. Make copies one line for the chat. Pre-production code (research 00):
## prototypes/worldgen's README names the items it is about.
extends Control

signal closed

const RUNS := preload("res://look/runs.gd")
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const GAP := 12
const THREADS := 4

var code := ""
var _status: Label
var _make: Button
var _maps: VBoxContainer
var _worker: Thread
var _heat := []
var _read_clock := 0.0
var _started := 0


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
	var outer := VBoxContainer.new()
	outer.add_theme_constant_override("separation", GAP)
	margin.add_child(outer)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	outer.add_child(scroll)
	var column := VBoxContainer.new()
	column.size_flags_horizontal = SIZE_EXPAND_FILL
	column.add_theme_constant_override("separation", GAP)
	scroll.add_child(column)
	_text(column, "P7 World generation", FLAME, 22)
	_text(
		column,
		(
			"Make builds a new world the way the game will: twenty rough candidates, the best "
			+ "four again in full, and the best three offered with their maps. Then it settles the "
			+ "first for ten years with no people. The aim is about 3 minutes for the three worlds "
			+ "and 1 more for settling. Put the phone down until it is done."
		),
		INK,
		15
	)
	_status = _text(column, "", INK, 15)
	_maps = VBoxContainer.new()
	_maps.add_theme_constant_override("separation", GAP)
	column.add_child(_maps)
	_make = Button.new()
	_make.text = "Make"
	_make.custom_minimum_size.y = 48
	_make.pressed.connect(make)
	outer.add_child(_make)
	var back := Button.new()
	back.text = "Back"
	back.custom_minimum_size.y = 48
	back.pressed.connect(func() -> void: closed.emit())
	outer.add_child(back)
	if not ClassDB.class_exists("WorldGen"):
		_status.text = "This build has no C++ part, so nothing can run."
		_make.disabled = true
	elif "run" in OS.get_cmdline_user_args():
		make.call_deferred()


## Starts "New world" on its own thread, so the screen keeps drawing and the heat can be read.
func make() -> void:
	if _worker != null and _worker.is_alive():
		return
	code = ""
	for child in _maps.get_children():
		child.queue_free()
	_heat = [RUNS.thermal()]
	_read_clock = 0.0
	_make.disabled = true
	_started = Time.get_ticks_msec()
	_status.text = "Making twenty rough candidates, then the best four in full…"
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
	var gen: RefCounted = ClassDB.instantiate("WorldGen")
	var made: Dictionary = gen.make(THREADS)
	_offered.call_deferred(gen, made)
	var settled: float = gen.settle()
	_done.call_deferred(made, settled, gen.digest())


## Shows the offered worlds' maps and summaries as soon as they are made.
func _offered(gen: RefCounted, made: Dictionary) -> void:
	var summaries: PackedStringArray = made.get("summaries", PackedStringArray())
	for i in summaries.size():
		var image := Image.create_from_data(
			gen.map_width(), gen.map_height(), false, Image.FORMAT_RGB8, gen.map(i)
		)
		var map := TextureRect.new()
		map.texture = ImageTexture.create_from_image(image)
		map.expand_mode = TextureRect.EXPAND_FIT_WIDTH_PROPORTIONAL
		map.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT
		map.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
		_maps.add_child(map)
		_text(_maps, "World %d: %s" % [i + 1, summaries[i]], DIM, 13)
	_status.text = (
		"Three worlds in %.0f s. Now settling the first for ten years…"
		% (float(made.get("candidates_seconds", 0.0)) + float(made.get("best_seconds", 0.0)))
	)


func _done(made: Dictionary, settled: float, digest: String) -> void:
	_worker.wait_to_finish()
	_make.disabled = false
	_heat.append(RUNS.thermal())
	var main: GDScript = load("res://main.gd")
	var head := (
		"P7 %s %s"
		% [ProjectSettings.get_setting("application/config/version", ""), main.facts().phone]
	)
	code = line(head, made, settled, digest, _heat)
	var three := float(made.get("candidates_seconds", 0.0)) + float(made.get("best_seconds", 0.0))
	_status.text = (
		(
			"Three worlds in %.0f s (the aim: about 3 minutes); settling in %.0f s (the aim: about 1"
			+ " more).\n\nCopied for the chat:\n%s"
		)
		% [three, settled, code]
	)
	DisplayServer.clipboard_set(code)
	print(code)
	if "measure" in OS.get_cmdline_user_args():
		get_tree().quit()


## The line for the chat: the candidates and the best few with their times, settling, the time each
## stage took summed over every world, the threads, the heat and the digest.
static func line(
	head: String, made: Dictionary, settled: float, digest: String, heat: Array
) -> String:
	var candidates := float(made.get("candidates_seconds", 0.0))
	var best := float(made.get("best_seconds", 0.0))
	var runs := PackedStringArray(
		[
			"three worlds in %.1f s" % (candidates + best),
			(
				"%d candidates in %.1f s, %d qualified"
				% [int(made.get("made", 0)), candidates, int(made.get("qualified", 0))]
			),
			"best %d at full size in %.1f s" % [int(made.get("best", 0)), best],
			"settling 10 years in %.1f s" % settled,
			(
				"plates %.1f erosion %.1f climate %.1f life %.1f scoring %.1f"
				% [
					float(made.get("plates", 0.0)),
					float(made.get("erosion", 0.0)),
					float(made.get("climate", 0.0)),
					float(made.get("life", 0.0)),
					float(made.get("score", 0.0)),
				]
			),
			"%d threads" % THREADS,
			"digest %s" % digest,
		]
	)
	return RUNS.code(head, runs, heat)


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
