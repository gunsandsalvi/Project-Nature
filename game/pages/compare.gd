## The Compare page (A5.5): the blind tests. Pick a test; ten pairs of one view, each drawn two ways
## one above the other, the better way on top or below by chance; tap the picture that answers the
## test's question. After ten, the answers' short code to send, and whether the difference shows:
## eight or more right, which guessing reaches about 5% of the time. The tests: MSAA 4x against 2x
## on the meadow; leaves smoothed by alpha to coverage against plain cut-outs, on calibration scene
## C2's plants. The pictures change only the drawing, never a world (WLD-13). Implements PRE-01 and
## PRE-46.
extends VBoxContainer

const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const GOOD := Palette.GOOD
## The better way and the other in the MSAA test.
const BETTER := Viewport.MSAA_4X
const OTHER := Viewport.MSAA_2X
## The tests' comparisons, as kd::look numbers them (blind.hpp), and those this page draws.
const SHARPNESS := 1
const LEAF_EDGES := 2
const DRAWN := [SHARPNESS, LEAF_EDGES]
## The layers the leaf test's two ways of drawing its plants are on, each seen by one picture.
const PLAIN_LAYER := 1 << 1
const SMOOTH_LAYER := 1 << 2
const ALL_LAYERS := 0xFFFFF

## The look's class: the rig, the ground and the blind test's pairs and code.
var look := KdLook.new()
## The problem that kept the ground from loading, or "".
var problem := ""
## This test's comparison, its seed, its pairs, the answers so far (true: the top picture chosen)
## and, once all are in, its code.
var comparison := SHARPNESS
var test_seed := 0
var pairs: Array = []
var answers: Array[bool] = []
var code := ""

var _world := World3D.new()
var _stage: Node3D
var _views: Array[SubViewport] = []
var _cameras: Array[Camera3D] = []
var _plants: Array[Node3D] = []
var _prompt: Label
var _result: Label
var _pictures: VBoxContainer
var _after: HBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	var tests := HBoxContainer.new()
	add_child(tests)
	for c in look.blind_comparisons():
		if c in DRAWN:
			_button(tests, look.blind_words(c)["name"], start.bind(c))
	_prompt = _label(17, TEXT)
	_pictures = VBoxContainer.new()
	_pictures.size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_child(_pictures)
	for i in 2:
		var holder := SubViewportContainer.new()
		holder.stretch = true
		holder.size_flags_vertical = Control.SIZE_EXPAND_FILL
		holder.gui_input.connect(_on_picture_input.bind(i == 0))
		_pictures.add_child(holder)
		var view := SubViewport.new()
		view.world_3d = _world
		holder.add_child(view)
		var camera := Camera3D.new()
		view.add_child(camera)
		camera.current = true
		_views.append(view)
		_cameras.append(camera)
	_stage = Node3D.new()
	_views[0].add_child(_stage)
	problem = LookScene.build(look, _stage)
	_result = _label(16, TEXT)
	_after = HBoxContainer.new()
	add_child(_after)
	_button(_after, "Copy code", func() -> void: DisplayServer.clipboard_set(code))
	_button(_after, "Again", func() -> void: start(comparison))
	# the cloud's picture of a test (tools/picture.sh game ... -- Compare leaves)
	start(LEAF_EDGES if "leaves" in OS.get_cmdline_user_args() else SHARPNESS)


func _exit_tree() -> void:
	look.clear()


func _process(delta: float) -> void:
	var size := _views[0].size
	if size.x <= 0 or size.y <= 0:
		return
	look.set_screen(Vector2(size))
	look.frame(delta)
	for camera in _cameras:
		LookScene.place_camera(camera, look)


## A new test of a comparison: a new seed, its pairs, no answers yet.
func start(which: int = SHARPNESS) -> void:
	comparison = which
	test_seed = (Time.get_ticks_usec() ^ int(Time.get_unix_time_from_system())) & 0xFFFF
	pairs = look.blind_pairs(comparison, test_seed)
	answers.clear()
	code = ""
	if comparison == LEAF_EDGES:
		CalibrationDrawing.light_stand_ins(_stage.get_node("Sun"), false)
	_show()


## Answers the pair on show: true chooses the top picture.
func choose(top: bool) -> void:
	if answers.size() >= pairs.size():
		return
	answers.append(top)
	if answers.size() == pairs.size():
		code = look.blind_code(comparison, test_seed, answers)
	_show()


## How many answers chose the better way.
func right() -> int:
	return look.blind_right(comparison, test_seed, answers) if answers.size() == pairs.size() else 0


func _show() -> void:
	var done := answers.size() == pairs.size()
	_pictures.visible = not done
	_after.visible = done
	_clear_plants()
	if not problem.is_empty():
		_prompt.text = "The ground could not load: " + problem
		return
	var words: Dictionary = look.blind_words(comparison)
	if done:
		var shows := right() >= 8
		_prompt.text = "Done: send this code"
		_result.text = (
			"%s\n%s: %d of %d right: %s"
			% [
				code,
				words["name"],
				right(),
				pairs.size(),
				"the difference shows" if shows else "the difference does not show",
			]
		)
		_result.add_theme_color_override("font_color", GOOD)
		return
	var pair: Dictionary = pairs[answers.size()]
	_prompt.text = "Pair %d of %d: %s Tap it." % [answers.size() + 1, pairs.size(), words["asks"]]
	_result.text = "%s; both are the same view." % words["compares"]
	_result.add_theme_color_override("font_color", QUIET)
	look.set_part("pattern", false)
	look.set_view(int(pair["east"]), int(pair["north"]), float(pair["heading"]), 1.0 / 128.0)
	var better_view := 0 if pair["better_first"] else 1
	for i in 2:
		_views[i].msaa_3d = OTHER
		_cameras[i].cull_mask = ALL_LAYERS
	if comparison == SHARPNESS:
		_views[better_view].msaa_3d = BETTER
	elif comparison == LEAF_EDGES:
		_show_plants(better_view)


## The leaf test's pair: the same plants drawn plainly cut out and smoothed by alpha to coverage,
## each way on a layer of its own, the better way's seen by the picture given.
func _show_plants(better_view: int) -> void:
	var size := Vector2(_views[0].size)
	if size.x <= 0.0 or size.y <= 0.0:
		return
	look.set_screen(size)
	look.frame(0.0)
	for camera in _cameras:
		LookScene.place_camera(camera, look)
	for way: String in ["plain", "coverage"]:
		var plants := CalibrationPlants.plants(_stage, _cameras[0], size, way)
		for node in plants.get_children():
			(node as VisualInstance3D).layers = PLAIN_LAYER if way == "plain" else SMOOTH_LAYER
		_plants.append(plants)
	_cameras[better_view].cull_mask = ALL_LAYERS & ~PLAIN_LAYER
	_cameras[1 - better_view].cull_mask = ALL_LAYERS & ~SMOOTH_LAYER


## The last pair's plants, freed at once: a test can ask a pair a frame.
func _clear_plants() -> void:
	for plants in _plants:
		plants.get_parent().remove_child(plants)
		plants.free()
	_plants.clear()


func _on_picture_input(event: InputEvent, top: bool) -> void:
	var tapped: bool = event is InputEventScreenTouch and not event.pressed
	var clicked: bool = (
		event is InputEventMouseButton
		and event.button_index == MOUSE_BUTTON_LEFT
		and not event.pressed
		and event.device != InputEvent.DEVICE_ID_EMULATION
	)
	if tapped or clicked:
		choose(top)


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	add_child(label)
	return label


func _button(row: HBoxContainer, text: String, pressed: Callable) -> void:
	var button := Button.new()
	button.text = text
	button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	button.custom_minimum_size = Vector2(0, 56)
	button.pressed.connect(pressed)
	row.add_child(button)
