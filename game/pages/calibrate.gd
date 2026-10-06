## The Calibrate page (A18.1, α2.2a): one tap runs every calibration scene the build lists, each of
## its variants at a 120-frame cap for the graphics chip's and the main thread's time, then at 60
## for the frames on time, the power and the heat, and ends with one code to copy into the chat.
## Each scene's file states, before its first run, what it draws, its switches, its line and the
## decision its number makes (RES-09); the page draws exactly that, straight into the window as the
## Look page does, and the cloud reads the code against the same files. Every switch changes only
## the drawing, never a world (WLD-13). Implements PLT-04.
extends VBoxContainer

## A variant's time at the 120 cap is read: what it drew is on the screen, for the cloud's pictures.
signal timed(scene: int, variant: int)

## Real seconds of each variant: drawn before its readings, so its pipelines compile and the chip's
## clock settles; read at the 120 cap; settled at the 60 cap; read at the 60 cap.
const WARM := 3.0
const FAST := 10.0
const SETTLE := 2.0
const STEADY := 20.0
## Real seconds between the power's readings: the battery's current changes about this often.
const EVERY := 2.0
const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const GOOD := Palette.GOOD
const FAIL := Palette.FAIL

## The page draws the world, so the shell hides its pages' ground (main.gd).
var draws_world := true
## Real time runs this many times faster in the tests; 1 on the phone.
var time_scale := 1.0
## The scenes as the build's files state them, in the order they run, and what is wrong with them;
## the readings so far, a list for each scene of a dictionary for each variant; what each variant
## drew, for the cloud's check; the code once done; and every line the page shows, for the tests.
var calibration := KdCalibration.new()
var scenes: Array = []
var problems := PackedStringArray()
var readings: Array = []
var counted: Array = []
var code := ""
var shown := PackedStringArray()

var _look := KdLook.new()
var _device := KdDevice.new()
var _world: Node3D
var _camera: Camera3D
var _sun: DirectionalLight3D
var _sky: Environment
var _ground_built := false
var _content: Node3D
var _mirror: SubViewport
var _viewports: Array[Viewport] = []
var _closest := 0.0
var _scene := -1
var _variant := 0
var _phase := 0
var _clock := 0
var _elapsed := 0.0
var _gpu := 0.0
var _gpu_frames := 0
var _cpu := 0.0
var _cpu_frames := 0
var _watts: Array[float] = []
var _next_power := 0.0
var _cover: CanvasLayer
var _status: Label
var _run: Button
var _list: VBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	_build_world()
	problems = calibration.read(GameData.calibration_paths(GameData.build()))
	scenes = calibration.scenes()
	_cover = CanvasLayer.new()
	# under the page's words and over the world: the pages' ground while nothing is measured
	_cover.layer = -1
	var ground := ColorRect.new()
	ground.color = Palette.GROUND
	ground.mouse_filter = Control.MOUSE_FILTER_IGNORE
	ground.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_cover.add_child(ground)
	add_child(_cover)
	_status = _label(16, TEXT)
	_run = Button.new()
	_run.text = "Run"
	_run.custom_minimum_size = Vector2(0, 56)
	_run.pressed.connect(start)
	add_child(_run)
	var spacer := Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	spacer.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(spacer)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.add_child(_list)
	if not problems.is_empty() or scenes.is_empty():
		_run.visible = false
		_status.text = "The calibration scenes cannot be read:\n" + "\n".join(problems)
		_status.add_theme_color_override("font_color", FAIL)
		return
	_status.text = (
		(
			"With the phone cool, unplugged and in flight mode, tap Run: %d scenes, %d variants,"
			+ " about %d minutes. The words go while it measures; leave the phone alone until"
			+ " the code shows."
		)
		% [scenes.size(), _variants(), roundi(_planned() / 60.0) + 1]
	)
	# the cloud's picture of a finished run (tools/picture.sh game ... -- Calibrate quick): a run a
	# hundred times faster, the picture held back until it ends
	if "quick" in OS.get_cmdline_user_args():
		time_scale = 0.01
		add_to_group("busy")
		start()


func _exit_tree() -> void:
	_restore()
	if running():
		DisplayServer.screen_set_keep_on(false)
		_device.trace_end()
	_look.clear()


## Whether the run is under way.
func running() -> bool:
	return _scene >= 0 and _scene < scenes.size()


## Starts the run: every scene's variants in turn.
func start() -> void:
	if running() or scenes.is_empty() or not problems.is_empty():
		return
	_run.visible = false
	_cover.visible = false
	DisplayServer.screen_set_keep_on(true)
	readings = []
	counted = []
	for scene: Dictionary in scenes:
		var read := []
		var drew := []
		for v in (scene["variants"] as Array).size():
			read.append({})
			drew.append({})
		readings.append(read)
		counted.append(drew)
	code = ""
	for child in _list.get_children():
		child.queue_free()
	_open(0, 0)


func _process(delta: float) -> void:
	_look.set_screen(Vector2(DisplayServer.window_get_size()))
	_look.frame(delta)
	LookScene.place_camera(_camera, _look)
	if _mirror != null:
		LookScene.place_camera(_mirror.get_child(0) as Camera3D, _look)
	if not running():
		return
	var scene: Dictionary = scenes[_scene]
	if scene["path"] != "still" and _look.playing().is_empty():
		_look.play(scene["path"])
	# real time by the steady clock, since Godot's delta is smoothed (A3.9)
	var now := Time.get_ticks_usec()
	_elapsed += (now - _clock) / 1.0e6
	_clock = now
	var t := _elapsed / time_scale
	if _phase == 0 and t >= WARM:
		_phase = 1
	elif _phase == 1:
		_read_fast()
		if t >= WARM + FAST:
			_end_fast()
	elif _phase == 2 and t >= WARM + FAST + SETTLE:
		_phase = 3
		_device.frames_reset(1000.0 / 60.0, DisplayServer.screen_get_refresh_rate())
		_next_power = t
	elif _phase == 3:
		if t >= _next_power:
			_next_power += EVERY
			var power := Phone.power(_device)
			if power.has("watts"):
				_watts.append(float(power["watts"]))
		if t >= WARM + FAST + SETTLE + STEADY:
			_end_steady()
	if _phase < 2 and (scene["variants"][_variant] as Dictionary)["interface"]:
		_status.text = (
			"%d of %d: %s, %s\nat 120 frames a second, %d s left in it"
			% [_done() + 1, _variants(), scene["name"], _variant_name(), ceili(WARM + FAST - t)]
		)


## The variant's drawing: its scene's things, its switches, the camera at the closest zoom.
func _open(scene_index: int, variant_index: int) -> void:
	_scene = scene_index
	_variant = variant_index
	_phase = 0
	_elapsed = 0.0
	_clock = Time.get_ticks_usec()
	_gpu = 0.0
	_gpu_frames = 0
	_cpu = 0.0
	_cpu_frames = 0
	_watts.clear()
	_clear_content()
	var scene: Dictionary = scenes[scene_index]
	var v: Dictionary = scene["variants"][variant_index]
	_look.set_view(0, 0, 0.0, _closest)
	LookScene.place_camera(_camera, _look)
	# the screen as the camera's rays take it, the shell's canvas size, and in the window's own pixels
	var screen := get_viewport().get_visible_rect().size
	var pixels := Vector2(DisplayServer.window_get_size())
	if pixels.x <= 0.0 or pixels.y <= 0.0:
		pixels = screen
	var draws: String = scene["draws"]
	_sun.visible = draws != "nothing"
	_sun.shadow_enabled = v["shadows"]
	if draws == "field" and not _ground_built:
		_ground_built = true
		var problem := LookScene.ground(_look, _world, CalibrationDrawing.FIELD_SHADER)
		if not problem.is_empty():
			_line("%s: the field cannot be drawn: %s" % [scene["name"], problem], FAIL)
		CalibrationDrawing.light_stand_ins(_sun)
	CalibrationDrawing.grade(_sky, draws == "field")
	if _ground_built:
		_look.set_part("ground", draws == "field")
		_look.set_part("pattern", false)
	if draws == "rocks":
		_content = CalibrationDrawing.rocks(_world, _camera, screen, v["triangles"])
	elif draws == "copies":
		_content = CalibrationDrawing.copies(_world, _camera, screen, v["copies"])
		if int(v["passes"]) == 3:
			_mirror = CalibrationDrawing.mirror(self, _world.get_world_3d(), _camera, pixels)
	var viewport := get_viewport()
	viewport.msaa_3d = {0: Viewport.MSAA_DISABLED, 2: Viewport.MSAA_2X, 4: Viewport.MSAA_4X}[int(
		v["msaa"]
	)]
	viewport.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	viewport.use_taa = false
	viewport.use_debanding = true
	viewport.scaling_3d_mode = Viewport.SCALING_3D_MODE_BILINEAR
	viewport.scaling_3d_scale = int(v["scale"]) / 100.0
	RenderingServer.viewport_set_disable_2d(viewport.get_viewport_rid(), not v["interface"])
	_viewports = Timing.viewports(self)
	Engine.max_fps = 120
	_device.trace_begin("kd calibrate %s %s" % [scene["name"], v["name"]])
	if scene["path"] != "still":
		_look.play(scene["path"])


## A frame at the 120 cap: the graphics chip's and the main thread's time, and what was drawn.
func _read_fast() -> void:
	var gpu := Timing.gpu_ms(_viewports)
	if gpu > 0.0:
		_gpu += gpu
		_gpu_frames += 1
	var cpu := Timing.cpu_ms(_viewports)
	if cpu > 0.0:
		_cpu += cpu
		_cpu_frames += 1
	var main := get_viewport().get_viewport_rid()
	var drew := {
		"draws": _drawn(main, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_VISIBLE, true),
		"triangles": _drawn(main, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_VISIBLE, false),
		"shadow_draws": _drawn(main, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_SHADOW, true),
		"shadow_triangles": _drawn(main, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_SHADOW, false),
		"mirror_draws": 0,
		"mirror_shadow_draws": 0,
	}
	if _mirror != null:
		var mirror := _mirror.get_viewport_rid()
		drew["mirror_draws"] = _drawn(
			mirror, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_VISIBLE, true
		)
		drew["mirror_shadow_draws"] = _drawn(
			mirror, RenderingServer.VIEWPORT_RENDER_INFO_TYPE_SHADOW, true
		)
	counted[_scene][_variant] = drew


func _end_fast() -> void:
	_phase = 2
	Engine.max_fps = 60
	var reading := {
		"gpu_us": roundi(_gpu / _gpu_frames * 1000.0) if _gpu_frames > 0 else -1,
		"cpu_us": roundi(_cpu / _cpu_frames * 1000.0) if _cpu_frames > 0 else -1,
	}
	readings[_scene][_variant] = reading
	timed.emit(_scene, _variant)


func _end_steady() -> void:
	var reading: Dictionary = readings[_scene][_variant]
	var frames := _device.frames()
	reading["on_time"] = (
		roundi(1000.0 * float(frames["on_time"]) / float(frames["frames"]))
		if int(frames["frames"]) > 0
		else -1
	)
	var watts := 0.0
	for w: float in _watts:
		watts += w
	reading["power_mw"] = roundi(watts / _watts.size() * 1000.0) if not _watts.is_empty() else -1
	var thermal := _device.thermal()
	reading["heat"] = (
		roundi(float(thermal["forecast_10s"]) * 100.0) if thermal.get("available", false) else -1
	)
	_device.trace_end()
	_line(calibration.reading_words(_scene, _variant, reading), QUIET)
	if _variant + 1 < (scenes[_scene]["variants"] as Array).size():
		_open(_scene, _variant + 1)
	elif _scene + 1 < scenes.size():
		_open(_scene + 1, 0)
	else:
		_end()


func _end() -> void:
	_scene = scenes.size()
	_restore()
	_clear_content()
	_sun.visible = false
	if _ground_built:
		_look.set_part("ground", false)
	_cover.visible = true
	DisplayServer.screen_set_keep_on(false)
	remove_from_group("busy")
	code = calibration.code(int(GameData.build().get_value("build", "code", 0)), readings)
	_status.text = "Done. Copy the code into the chat:"
	var shown_code := Label.new()
	# spaces rather than dashes between its groups, so the lines break between them
	shown_code.text = code.replace("-", " ")
	shown_code.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	shown_code.add_theme_font_size_override("font_size", 20)
	shown_code.add_theme_color_override("font_color", GOOD)
	add_child(shown_code)
	move_child(shown_code, 1)
	shown.append(code)
	var copy := Button.new()
	copy.text = "Copy the code"
	copy.custom_minimum_size = Vector2(0, 56)
	copy.pressed.connect(func() -> void: DisplayServer.clipboard_set(code))
	add_child(copy)
	move_child(copy, 2)
	for verdict: String in calibration.verdicts(readings):
		_line(verdict, TEXT)


## The drawing back to the shell's: the window's own interface, no 3D switches, the 60 cap.
func _restore() -> void:
	Engine.max_fps = 60
	var viewport := get_viewport()
	if viewport == null:
		return
	RenderingServer.viewport_set_disable_2d(viewport.get_viewport_rid(), false)
	viewport.msaa_3d = Viewport.MSAA_DISABLED
	viewport.scaling_3d_scale = 1.0
	RenderingServer.viewport_set_measure_render_time(viewport.get_viewport_rid(), false)


func _build_world() -> void:
	_world = Node3D.new()
	add_child(_world)
	_camera = Camera3D.new()
	_camera.current = true
	_world.add_child(_camera)
	LookScene.light(_world)
	_sun = _world.get_node("Sun")
	_sky = (_world.get_node("Sky") as WorldEnvironment).environment
	# one sun map of 2,048 texels, as the game's (A4.4), and a layer of its own that the mirror's
	# camera does not see
	_sun.directional_shadow_mode = DirectionalLight3D.SHADOW_ORTHOGONAL
	_sun.layers = CalibrationDrawing.SUN_LAYER
	_sun.visible = false
	_look.set_screen(Vector2(DisplayServer.window_get_size()))
	_closest = float(_look.state()["metres_per_pixel"])


## What the last variant drew, taken out of the tree at once, so the next one's viewports are its
## own.
func _clear_content() -> void:
	for node: Node in [_content, _mirror]:
		if node != null:
			node.get_parent().remove_child(node)
			node.queue_free()
	_content = null
	_mirror = null


## A count of the last frame's draws, or its triangles, in a pass of a viewport.
func _drawn(viewport: RID, pass_type: int, draws: bool) -> int:
	var info := (
		RenderingServer.VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME
		if draws
		else RenderingServer.VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME
	)
	return RenderingServer.viewport_get_render_info(viewport, pass_type, info)


func _variants() -> int:
	var n := 0
	for scene: Dictionary in scenes:
		n += (scene["variants"] as Array).size()
	return n


## The variants done so far, across the scenes.
func _done() -> int:
	var n := _variant
	for i in _scene:
		n += (scenes[i]["variants"] as Array).size()
	return n


func _variant_name() -> String:
	return scenes[_scene]["variants"][_variant]["name"]


## The run's planned length in real seconds.
func _planned() -> float:
	return _variants() * (WARM + FAST + SETTLE + STEADY)


func _line(text: String, colour: Color) -> void:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_font_size_override("font_size", 14)
	label.add_theme_color_override("font_color", colour)
	_list.add_child(label)
	shown.append(text)


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label
