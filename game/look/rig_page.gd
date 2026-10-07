## The base of the pages that draw the world through the look's rig (A4.1, A8.4): the catalogue's
## world, the rig and its gestures, a camera that follows it, the late afternoon's light (A4.3),
## the picture at the screen's full resolution with 2x MSAA, a cover while the pipelines compile
## (A4.7), and the touches the rig reads. The Kit and Pilot pages build their own content on it, so
## the rig's plumbing is written once for them (CLAUDE.md rule 4). Implements PRE-01, PRE-33 and
## PLT-02.
class_name RigPage
extends VBoxContainer

## Frames drawn behind a cover as the page opens, so every material's pipelines compile out of sight
## (A4.7).
const WARM_FRAMES := 3

## The page draws the world, so the shell hides its pages' ground (main.gd).
var draws_world := true
## The catalogue's world, for the tuning and the recipes, and what loading it found.
var world := KdWorld.new()
var loaded: Dictionary = {}
## The look's own class: the rig, the gestures and the globals.
var look := KdLook.new()
## The problem that kept the page's content from loading, or "".
var problem := ""

var _scene: Node3D
var _camera: Camera3D
var _cover: ColorRect
var _warmed := 0


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	mouse_filter = Control.MOUSE_FILTER_STOP
	loaded = GameData.load_into(world)
	_scene = Node3D.new()
	add_child(_scene)
	_camera = Camera3D.new()
	_camera.current = true
	_scene.add_child(_camera)
	problem = Afternoon.apply(world, _scene)
	_apply_drawing()
	_add_cover()


func _exit_tree() -> void:
	look.clear()
	var viewport := get_viewport()
	if viewport:
		viewport.msaa_3d = Viewport.MSAA_DISABLED


func _process(delta: float) -> void:
	look.set_screen(Vector2(DisplayServer.window_get_size()))
	look.frame(delta)
	LookScene.place_camera(_camera, look)
	_warmed += 1
	if _cover != null and _warmed > WARM_FRAMES:
		_cover.queue_free()
		_cover = null


func _gui_input(event: InputEvent) -> void:
	var now := Time.get_ticks_usec() / 1_000_000.0
	var at := _window_point(event)
	if event is InputEventScreenTouch:
		if event.pressed:
			look.press(event.index, at, now)
		else:
			look.lift(event.index, at, now)
		accept_event()
	elif event is InputEventScreenDrag:
		look.move(event.index, at, now)
		accept_event()
	elif event is InputEventMouseButton and event.device != InputEvent.DEVICE_ID_EMULATION:
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				look.press(0, at, now)
			else:
				look.lift(0, at, now)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_UP:
			look.zoom_by(1.1, at)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			look.zoom_by(1.0 / 1.1, at)
	elif event is InputEventMouseMotion and event.device != InputEvent.DEVICE_ID_EMULATION:
		if event.button_mask & MOUSE_BUTTON_MASK_LEFT:
			look.move(0, at, now)
		elif event.button_mask & MOUSE_BUTTON_MASK_RIGHT:
			look.turn_by(event.relative.x * 0.25, at)


## The scenario the page's 3D draws into, for the families that draw through the RenderingServer.
func scenario() -> RID:
	return _scene.get_world_3d().scenario


## Puts the view at a place: the focus in centimetres east and north, the heading in degrees
## clockwise from north, and the zoom in metres a screen pixel.
func view_at(east: int, north: int, heading: float, metres_per_pixel: float) -> void:
	look.set_view(east, north, heading, metres_per_pixel)


## A button along a row of the page's controls, as wide as its share of the row and tall enough to
## touch.
func _button(parent: Control, label: String, pressed: Callable) -> void:
	var button := Button.new()
	button.text = label
	button.custom_minimum_size = Vector2(0, 48)
	button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	button.pressed.connect(pressed)
	parent.add_child(button)


## A line of the page's words, added to the page.
func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label


func _apply_drawing() -> void:
	var viewport := get_viewport()
	if viewport == null:
		return
	viewport.msaa_3d = Viewport.MSAA_2X
	viewport.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	viewport.use_taa = false
	viewport.use_debanding = true
	viewport.scaling_3d_mode = Viewport.SCALING_3D_MODE_BILINEAR
	viewport.scaling_3d_scale = 1.0


func _add_cover() -> void:
	_cover = ColorRect.new()
	_cover.color = Palette.GROUND
	_cover.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_cover.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var words := Label.new()
	words.text = "Preparing the picture"
	words.add_theme_color_override("font_color", Palette.TEXT)
	words.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
	_cover.add_child(words)
	# above everything on the page, the whole window over
	var layer := CanvasLayer.new()
	layer.layer = 10
	layer.add_child(_cover)
	add_child(layer)
	_cover.tree_exited.connect(layer.queue_free)


## The window's pixels to one of the shell's canvas pixels (window/stretch/mode="canvas_items").
func _stretch() -> float:
	var window := get_viewport().get_visible_rect().size
	var pixels := float(DisplayServer.window_get_size().x)
	return pixels / window.x if window.x > 0.0 and pixels > 0.0 else 1.0


func _window_point(event: InputEvent) -> Vector2:
	if event is InputEventScreenTouch or event is InputEventScreenDrag or event is InputEventMouse:
		return (event.position + global_position) * _stretch()
	return Vector2.ZERO
