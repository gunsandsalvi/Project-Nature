## The Compare page (A5.5): the blind test. Ten pairs of one view of the meadow, each drawn two ways
## one above the other, the better way on top or below by chance; tap the sharper picture. After
## ten, the answers' short code to send, and whether the difference shows: eight or more right,
## which guessing reaches about 5% of the time. This build's test compares MSAA 4x with 2x. The
## pictures change only the drawing, never a world (WLD-13). Implements PRE-01.
extends VBoxContainer

const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const GOOD := Palette.GOOD
## The better way and the other, as the pairs draw them.
const BETTER := Viewport.MSAA_4X
const OTHER := Viewport.MSAA_2X

## The look's class: the rig, the ground and the blind test's pairs and code.
var look := KdLook.new()
## The problem that kept the ground from loading, or "".
var problem := ""
## This test's seed, its pairs, the answers so far (true: the top picture chosen) and, once all are
## in, its code.
var test_seed := 0
var pairs: Array = []
var answers: Array[bool] = []
var code := ""

var _world := World3D.new()
var _views: Array[SubViewport] = []
var _cameras: Array[Camera3D] = []
var _prompt: Label
var _result: Label
var _pictures: VBoxContainer
var _after: HBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
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
	var stage := Node3D.new()
	_views[0].add_child(stage)
	problem = LookScene.build(look, stage)
	_result = _label(16, TEXT)
	_after = HBoxContainer.new()
	add_child(_after)
	_button(_after, "Copy code", func() -> void: DisplayServer.clipboard_set(code))
	_button(_after, "Again", start)
	start()


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


## A new test: a new seed, its pairs, no answers yet.
func start() -> void:
	test_seed = (Time.get_ticks_usec() ^ int(Time.get_unix_time_from_system())) & 0xFFFF
	pairs = look.blind_pairs(test_seed)
	answers.clear()
	code = ""
	_show()


## Answers the pair on show: true chooses the top picture as the sharper one.
func choose(top: bool) -> void:
	if answers.size() >= pairs.size():
		return
	answers.append(top)
	if answers.size() == pairs.size():
		code = look.blind_code(test_seed, answers)
	_show()


## How many answers chose the better way.
func right() -> int:
	return look.blind_right(test_seed, answers) if answers.size() == pairs.size() else 0


func _show() -> void:
	var done := answers.size() == pairs.size()
	_pictures.visible = not done
	_after.visible = done
	if not problem.is_empty():
		_prompt.text = "The ground could not load: " + problem
		return
	if done:
		var shows := right() >= 8
		_prompt.text = "Done: send this code"
		_result.text = (
			"%s\n%d of %d right: %s"
			% [
				code,
				right(),
				pairs.size(),
				"the difference shows" if shows else "the difference does not show",
			]
		)
		_result.add_theme_color_override("font_color", GOOD)
		return
	var pair: Dictionary = pairs[answers.size()]
	_prompt.text = "Pair %d of %d: which is sharper? Tap it." % [answers.size() + 1, pairs.size()]
	_result.text = "MSAA 4x against 2x on the meadow; both are the same view."
	_result.add_theme_color_override("font_color", QUIET)
	look.set_part("pattern", false)
	look.set_view(int(pair["east"]), int(pair["north"]), float(pair["heading"]), 1.0 / 128.0)
	_views[0].msaa_3d = BETTER if pair["better_first"] else OTHER
	_views[1].msaa_3d = OTHER if pair["better_first"] else BETTER


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
