## The Look page (A4.1, A4.7, M2): the world drawn straight into the window at the screen's full
## resolution with 2x MSAA, the page's controls over it. A pixel-art meadow at the closest zoom,
## with a test board whose colour names the texture level read; drag, pinch and turn it, flip the
## switches and watch the frame and graphics times. Every switch changes only the drawing, never a
## world (WLD-13). Implements PRE-01, PRE-02 and PRE-33.
extends VBoxContainer

## Frames drawn behind a cover as the page opens, so every material's pipelines compile out of
## sight (A4.7): the warm-up, which grows as materials come.
const WARM_FRAMES := 3
const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const FAIL := Palette.FAIL

## The page draws the world, so the shell hides its pages' ground (main.gd).
var draws_world := true
## The look's own class: the rig, the gestures, the globals and the ground.
var look := KdLook.new()
## What the switches are set to: MSAA (0, 2 or 4), the 3D's scale, the lens and each part.
var msaa := 2
var scale_3d := 1.0
var lens := 10.0
var parts := {"ground": true, "pattern": true, "shadows": true}
## The problem that kept the ground from loading, or "".
var problem := ""

var _world: Node3D
var _camera: Camera3D
var _sun: DirectionalLight3D
var _readout: Label
var _times: Label
var _phone: Label
var _device := KdDevice.new()
var _clock := 0.0
var _frames := 0
var _frame_ms := 0.0
var _viewport_rid: RID
var _cover: ColorRect
var _warmed := 0


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	mouse_filter = Control.MOUSE_FILTER_STOP
	_build_world()
	_add_cover()
	var spacer := Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(spacer)
	_readout = _label(15, TEXT)
	_times = _label(15, TEXT)
	_phone = _label(14, QUIET)
	_add_switches()
	_apply_drawing()


func _exit_tree() -> void:
	look.clear()
	var viewport := get_viewport()
	if viewport:
		viewport.msaa_3d = Viewport.MSAA_DISABLED
		viewport.scaling_3d_scale = 1.0
		RenderingServer.viewport_set_measure_render_time(viewport.get_viewport_rid(), false)


func _process(delta: float) -> void:
	look.set_screen(Vector2(DisplayServer.window_get_size()))
	look.frame(delta)
	_place_camera()
	_count(delta)
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


## Sets MSAA to 0, 2 or 4 samples; only the drawing changes.
func set_msaa(samples: int) -> void:
	msaa = samples
	_apply_drawing()


## Sets the 3D's scale: 1.0, 0.75 or 0.5 of the screen's resolution.
func set_scale_3d(value: float) -> void:
	scale_3d = value
	_apply_drawing()


## Shows or hides a part of the drawing: "ground", "pattern" or "shadows".
func set_part(part: String, on: bool) -> void:
	parts[part] = on
	_apply_drawing()


## Sets the lens across the screen's short side: 5 or 10 degrees.
func set_lens(degrees: float) -> void:
	lens = degrees
	look.set_lens(degrees)


func _build_world() -> void:
	_world = Node3D.new()
	add_child(_world)
	_camera = Camera3D.new()
	_camera.current = true
	_world.add_child(_camera)
	problem = LookScene.build(look, _world)
	_sun = _world.get_node("Sun")


func _add_cover() -> void:
	_cover = ColorRect.new()
	_cover.color = Palette.GROUND
	_cover.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_cover.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var words := Label.new()
	words.text = "Preparing the picture"
	words.add_theme_color_override("font_color", TEXT)
	words.set_anchors_and_offsets_preset(Control.PRESET_CENTER)
	_cover.add_child(words)
	# above everything on the page, the whole window over
	var layer := CanvasLayer.new()
	layer.layer = 10
	layer.add_child(_cover)
	add_child(layer)
	_cover.tree_exited.connect(layer.queue_free)


func _add_switches() -> void:
	var rows := GridContainer.new()
	rows.columns = 3
	add_child(rows)
	_switch(rows, "MSAA", [["off", 0], ["2x", 2], ["4x", 4]], set_msaa, msaa)
	_switch(rows, "3D", [["1.0", 1.0], ["0.75", 0.75], ["0.5", 0.5]], set_scale_3d, scale_3d)
	_switch(rows, "Lens", [["5°", 5.0], ["10°", 10.0]], set_lens, lens)
	var row := HBoxContainer.new()
	add_child(row)
	for part: String in parts:
		var box := CheckBox.new()
		box.text = part.capitalize()
		box.button_pressed = parts[part]
		box.toggled.connect(func(on: bool) -> void: set_part(part, on))
		row.add_child(box)
	var paths := HBoxContainer.new()
	add_child(paths)
	for path: String in ["pan", "turn", "pinch"]:
		var button := Button.new()
		button.text = path.capitalize()
		button.custom_minimum_size = Vector2(0, 48)
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		button.pressed.connect(func() -> void: look.play(path))
		paths.add_child(button)


func _switch(
	parent: Control, title: String, choices: Array, apply: Callable, current: Variant
) -> void:
	var label := Label.new()
	label.text = title
	label.add_theme_color_override("font_color", TEXT)
	parent.add_child(label)
	var group := ButtonGroup.new()
	var row := HBoxContainer.new()
	row.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	parent.add_child(row)
	for choice: Array in choices:
		var button := Button.new()
		button.text = choice[0]
		button.toggle_mode = true
		button.button_group = group
		button.button_pressed = choice[1] == current
		button.custom_minimum_size = Vector2(72, 44)
		button.pressed.connect(func() -> void: apply.call(choice[1]))
		row.add_child(button)
	parent.add_child(Control.new())


func _apply_drawing() -> void:
	var viewport := get_viewport()
	if viewport == null:
		return
	viewport.msaa_3d = {0: Viewport.MSAA_DISABLED, 2: Viewport.MSAA_2X, 4: Viewport.MSAA_4X}[msaa]
	viewport.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	viewport.use_taa = false
	viewport.use_debanding = true
	viewport.scaling_3d_mode = Viewport.SCALING_3D_MODE_BILINEAR
	viewport.scaling_3d_scale = scale_3d
	_viewport_rid = viewport.get_viewport_rid()
	RenderingServer.viewport_set_measure_render_time(_viewport_rid, true)
	look.set_part("ground", parts["ground"])
	look.set_part("pattern", parts["pattern"])
	_sun.shadow_enabled = parts["shadows"]


func _place_camera() -> void:
	var pose := look.pose()
	_camera.transform = pose["transform"]
	_camera.keep_aspect = Camera3D.KEEP_WIDTH if pose["keep_width"] else Camera3D.KEEP_HEIGHT
	_camera.fov = pose["fov"]
	_camera.near = pose["near"]
	_camera.far = pose["far"]


func _count(delta: float) -> void:
	_clock += delta
	_frames += 1
	_frame_ms += delta * 1000.0
	if _clock < 0.5:
		return
	var state := look.state()
	_readout.text = (
		"%.1f m across · band %d · texel %.2f px · heading %d°"
		% [state["metres_across"], state["band"], state["texel_pixels"], roundi(state["heading"])]
	)
	var gpu := RenderingServer.viewport_get_measured_render_time_gpu(_viewport_rid)
	_times.text = "frame %.1f ms · graphics %.2f ms" % [_frame_ms / _frames, gpu]
	_phone.text = phone_words()
	if not problem.is_empty():
		_times.text = problem
		_times.add_theme_color_override("font_color", FAIL)
	_clock = 0.0
	_frames = 0
	_frame_ms = 0.0


## The phone's readings in a line (PLT-04): Godot's draws, triangles and video memory this
## frame, the graphics chip's headroom, the power drawn, and the heat forecast against the
## phone's own light throttling level, each only where the phone gives it.
func phone_words() -> String:
	var words := PackedStringArray()
	(
		words
		. append(
			(
				"%d draws · %dk triangles · %d MB video"
				% [
					RenderingServer.get_rendering_info(
						RenderingServer.RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME
					),
					(
						RenderingServer.get_rendering_info(
							RenderingServer.RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME
						)
						/ 1000
					),
					(
						RenderingServer.get_rendering_info(
							RenderingServer.RENDERING_INFO_VIDEO_MEM_USED
						)
						/ 1048576
					),
				]
			)
		)
	)
	var headroom := _device.gpu_headroom()
	if headroom.get("available", false):
		words.append("chip headroom %d" % roundi(headroom["headroom"]))
	var power := Phone.power(_device)
	if power.has("watts"):
		words.append("%.2f W" % power["watts"])
	var thermal := _device.thermal()
	if thermal.get("available", false):
		var heat := "heat %.2f" % float(thermal["forecast_10s"])
		if thermal.has("light"):
			heat += " of %.2f" % float(thermal["light"])
		words.append(heat)
	return " · ".join(words)


## The window's pixels to one of the shell's canvas pixels (window/stretch/mode="canvas_items").
func _stretch() -> float:
	var window := get_viewport().get_visible_rect().size
	return DisplayServer.window_get_size().x / window.x if window.x > 0.0 else 1.0


func _window_point(event: InputEvent) -> Vector2:
	if event is InputEventScreenTouch or event is InputEventScreenDrag or event is InputEventMouse:
		return (event.position + global_position) * _stretch()
	return Vector2.ZERO


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label
