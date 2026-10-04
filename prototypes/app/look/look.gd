## P1 The look (IMPLEMENTATION α0.2a): the art book's close camp drawn by Godot's Mobile renderer
## at a quarter of the screen's size, one art pixel to 4 × 4 screen pixels, with a camera locked to
## the art pixels' grid. Switches: the hour, the four outline methods, the mirrored water, and the
## fixes for crawling in free turns and zooms; Measure runs the camera along a set path for each.
## Pre-production code (research 00): the app's README names the items it is about.
extends Control

signal closed

const ART := 4
const BACK := 300.0
const SCENE := "res://look/close_camp.scn"
const OUTLINES := ["none", "A", "B", "C", "D"]
const CRAWLS := ["free", "steps", "ease", "rest"]
const TURN_STEP := 15.0
const ZOOM_STEP := 1.25
const LAYER_MAIN := 1
const LAYER_GBUF := 2
const LAYER_SKY := 4
const LAYER_MIRROR := 8
## What Measure runs (PLT-04): each outline method without and with the mirrored water, each for
## RUN_TIME seconds: a second to settle, then two each of panning, turning and zooming by script.
const RUNS := [
	[0, false],
	[1, false],
	[2, false],
	[3, false],
	[4, false],
	[0, true],
	[1, true],
	[2, true],
	[3, true],
	[4, true]
]
const RUN_TIME := 7.0

var painter: Dictionary
var target := Vector3.ZERO
var yaw := 16.0
var elev := 30.0
var mpp := 0.09
var hour := "noon"
var outline := 3
var reflect := false
var crawl := 0
var shot := ""

var _art: SubViewport
var _gbuf: SubViewport
var _mirror: SubViewport
var _sky: SubViewport
var _view: TextureRect
var _cam: Camera3D
var _gbuf_cam: Camera3D
var _mirror_cam: Camera3D
var _sun: DirectionalLight3D
var _env: Environment
var _post: MeshInstance3D
var _hull: ShaderMaterial
var _solid: ShaderMaterial
var _ground: ShaderMaterial
var _readout: Label
var _buttons := {}
var _touches := {}
var _twist := 0.0
var _pinch := 1.0
var _rest_yaw := 16.0
var _rest_mpp := 0.09
var _moving := 0.0
var _frames := 0
var _gpu := 0.0
var _clock := 0.0
var _run := -1
var _run_clock := 0.0
var _samples := PackedFloat32Array()
var _late := 0
var _results := PackedStringArray()
var _before := []


func _ready() -> void:
	var scene: Node3D = (load(SCENE) as PackedScene).instantiate()
	painter = scene.get_meta("painter")
	var c: Dictionary = painter.camera
	target = Vector3(c.target[0], c.target[1], c.target[2])
	yaw = c.yaw
	elev = c.elev
	mpp = c.mpp
	_rest_yaw = yaw
	_rest_mpp = mpp
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("hour="):
			hour = arg.substr(5)
		elif arg.begins_with("outline="):
			outline = OUTLINES.find(arg.substr(8))
		elif arg.begins_with("mirror="):
			reflect = arg.substr(7) == "on"
		elif arg.begins_with("shot="):
			shot = arg.substr(5)
		elif arg.begins_with("yaw="):
			yaw = float(arg.substr(4))
		elif arg == "measure":
			_start_measure.call_deferred()
	_build_views(scene)
	_build_materials(scene)
	_build_controls()
	_globals_once(scene)
	_set_hour(hour)
	_set_outline(outline)
	_set_reflect(reflect)
	resized.connect(_layout)
	_layout()


## The low-resolution views: the picture, the second camera's normals and depths (outline method C),
## the mirrored scene (the water's reflections) and the heights from above (the sky's light).
func _build_views(scene: Node3D) -> void:
	_art = _viewport(false, false)
	add_child(_art)
	_art.add_child(scene)
	_cam = _camera(LAYER_MAIN)
	_art.add_child(_cam)
	_env = Environment.new()
	_env.background_mode = Environment.BG_COLOR
	_env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	_env.ambient_light_color = Color.BLACK
	_env.ambient_light_energy = 0.0
	_env.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	var world_env := WorldEnvironment.new()
	world_env.environment = _env
	_art.add_child(world_env)
	_sun = DirectionalLight3D.new()
	_sun.shadow_enabled = true
	_sun.directional_shadow_mode = DirectionalLight3D.SHADOW_ORTHOGONAL
	_sun.directional_shadow_max_distance = BACK + 220.0
	_sun.shadow_bias = 0.08
	_sun.shadow_normal_bias = 1.6
	_art.add_child(_sun)
	_gbuf = _viewport(true, true)
	_gbuf.world_3d = _art.world_3d
	_gbuf_cam = _camera(LAYER_GBUF)
	_gbuf.add_child(_gbuf_cam)
	add_child(_gbuf)
	_mirror = _viewport(false, true)
	_mirror.world_3d = _art.world_3d
	_mirror_cam = _camera(LAYER_MIRROR)
	_mirror.add_child(_mirror_cam)
	add_child(_mirror)
	_sky = _viewport(true, true)
	_sky.world_3d = _art.world_3d
	_sky.size = Vector2i(1024, 1024)
	_sky.render_target_update_mode = SubViewport.UPDATE_ONCE
	var sky_cam := Camera3D.new()
	sky_cam.projection = Camera3D.PROJECTION_ORTHOGONAL
	sky_cam.cull_mask = LAYER_SKY
	var ext: Array = painter.extent
	var side := maxf(ext[2] - ext[0], ext[3] - ext[1])
	sky_cam.size = side
	sky_cam.near = 1.0
	sky_cam.far = 2000.0
	var middle := Vector3((ext[0] + ext[2]) * 0.5, target.y + 600.0, (ext[1] + ext[3]) * 0.5)
	sky_cam.transform = Transform3D(Basis.looking_at(Vector3.DOWN, Vector3.FORWARD), middle)
	_sky.add_child(sky_cam)
	add_child(_sky)
	var corner := Vector2(middle.x - side * 0.5, middle.z - side * 0.5)
	RenderingServer.global_shader_parameter_set(
		"look_sky_box", Vector4(corner.x, corner.y, side, 0.0)
	)
	RenderingServer.global_shader_parameter_set("look_sky_height", Vector2(target.y - 40.0, 120.0))
	RenderingServer.global_shader_parameter_set("look_sky_map", _sky.get_texture())
	_view = TextureRect.new()
	_view.texture = _art.get_texture()
	_view.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_view.stretch_mode = TextureRect.STRETCH_SCALE
	_view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_view)
	_post = MeshInstance3D.new()
	var quad := QuadMesh.new()
	quad.size = Vector2(1.0, 1.0)
	_post.mesh = quad
	var post := ShaderMaterial.new()
	post.shader = load("res://look/shaders/outline_post.gdshader")
	post.render_priority = -100
	_post.material_override = post
	_post.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	_post.extra_cull_margin = 16384.0
	_post.layers = LAYER_MAIN
	_cam.add_child(_post)
	_post.position = Vector3(0, 0, -2)


func _viewport(data: bool, clear: bool) -> SubViewport:
	var vp := SubViewport.new()
	vp.msaa_3d = Viewport.MSAA_DISABLED
	vp.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	vp.use_taa = false
	vp.use_hdr_2d = data
	vp.transparent_bg = clear
	vp.positional_shadow_atlas_size = 0
	vp.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	RenderingServer.viewport_set_measure_render_time(vp.get_viewport_rid(), true)
	return vp


func _camera(mask: int) -> Camera3D:
	var cam := Camera3D.new()
	cam.projection = Camera3D.PROJECTION_ORTHOGONAL
	cam.keep_aspect = Camera3D.KEEP_HEIGHT
	cam.near = 1.0
	cam.far = BACK * 3.0
	cam.cull_mask = mask
	return cam


## Each kind of mesh its material, and copies of each mesh for the data and mirror passes, each in
## its own layer with its own variant of the material, casting no shadow.
func _build_materials(scene: Node3D) -> void:
	var rows: Dictionary = painter.rows
	_ground = ShaderMaterial.new()
	_ground.shader = load("res://look/shaders/ground.gdshader")
	_solid = ShaderMaterial.new()
	_solid.shader = load("res://look/shaders/solid.gdshader")
	var cards := ShaderMaterial.new()
	cards.shader = load("res://look/shaders/cards.gdshader")
	var water := ShaderMaterial.new()
	water.shader = load("res://look/shaders/water.gdshader")
	water.set_shader_parameter("water_row", float(rows.water))
	water.set_shader_parameter("foam_row", float(rows.white))
	water.render_priority = 1
	_hull = _variant(_solid, "HULL")
	var variants := {}
	for base: ShaderMaterial in [_ground, _solid, cards]:
		variants[base] = {
			LAYER_GBUF: _variant(base, "PREPASS"),
			LAYER_SKY: _variant(base, "SKYMAP"),
			LAYER_MIRROR: _variant(base, "MIRROR"),
		}
	for node: MeshInstance3D in scene.get_children():
		var kind: String = node.get_meta("kind")
		var mat: ShaderMaterial
		if kind == "ground":
			var l: Array = node.get_meta("layers")
			var p: Array = node.get_meta("layerPat")
			for m: ShaderMaterial in [_ground, variants[_ground][LAYER_MIRROR]]:
				m.set_shader_parameter("layers", Vector4(l[0], l[1], l[2], l[3]))
				m.set_shader_parameter("layer_pat", Vector4(p[0], p[1], p[2], p[3]))
			mat = _ground
		elif kind == "solid":
			mat = _solid
		elif kind == "cards":
			mat = cards
		elif kind == "water":
			node.material_override = water
			node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
			node.layers = LAYER_MAIN
			continue
		else:
			var puffs := ShaderMaterial.new()
			puffs.shader = load("res://look/shaders/puffs.gdshader")
			puffs.set_shader_parameter("puff_row", float(node.get_meta("row")))
			puffs.set_shader_parameter("puff_shift", float(node.get_meta("shift")))
			puffs.render_priority = 2
			node.material_override = puffs
			node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
			node.layers = LAYER_MAIN
			continue
		node.material_override = mat
		node.layers = LAYER_MAIN
		for layer: int in [LAYER_GBUF, LAYER_SKY, LAYER_MIRROR]:
			var copy := MeshInstance3D.new()
			copy.mesh = node.mesh
			copy.material_override = variants[mat][layer]
			copy.layers = layer
			copy.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
			scene.add_child(copy)


## A material in one of its variants: the same shader compiled with a #define after its type.
func _variant(base: ShaderMaterial, define: String) -> ShaderMaterial:
	var shader := Shader.new()
	shader.code = base.shader.code.replace(
		"shader_type spatial;", "shader_type spatial;\n#define %s" % define
	)
	var m := ShaderMaterial.new()
	m.shader = shader
	return m


func _globals_once(scene: Node3D) -> void:
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_palette", scene.get_meta("palette"))
	set_global.call("look_atlas", scene.get_meta("atlas"))
	set_global.call("look_atlas_grid", float(painter.grid))
	var look: Dictionary = painter.look
	set_global.call("look_contrast", float(look.contrast))
	set_global.call(
		"look_edges", Vector4(look.outline, look.outlineNature, look.lit, painter.edgeK)
	)
	set_global.call("look_sky_cover", float(painter.skyCover))
	set_global.call("look_haze_range", Vector2(BACK + 20.0, BACK + 160.0))
	var tint: Array = painter.waterTint
	set_global.call("look_water_tint", Vector3(tint[0], tint[1], tint[2]))
	set_global.call("look_mirror_y", float(painter.waterLevel))
	set_global.call("look_gbuf_depth", Vector2(BACK - 200.0, 400.0))
	set_global.call("look_gbuf", _gbuf.get_texture())
	set_global.call("look_mirror", _mirror.get_texture())
	set_global.call("look_elev_sin", sin(deg_to_rad(elev)))
	var soot: Array = painter.soot
	var s: Array = soot[0] if soot.size() > 0 else [0, 0, 0, 0]
	set_global.call("look_soot", Vector4(s[0], s[1], s[2], s[3]))


## The hour's light from the painter (PRE-30): the sun's way and tints, and the hearth, which burns
## higher at dusk (the painter's hearth at level 4 at dusk, level 3 damped at noon).
func _set_hour(name: String) -> void:
	hour = name
	var md: Dictionary = painter.moods[name]
	var set_global := RenderingServer.global_shader_parameter_set
	var to_sun := Vector3(md.sunDir[0], md.sunDir[1], md.sunDir[2])
	_sun.transform = Transform3D(Basis.looking_at(-to_sun, Vector3.UP), Vector3.ZERO)
	set_global.call("look_sun_dir", to_sun)
	set_global.call("look_sun_k", float(md.sunK))
	set_global.call("look_sky_k", float(md.skyK))
	set_global.call("look_sun_pow", float(md.get("sunPow", 1.0)))
	set_global.call("look_shift", float(md.shift))
	for key in ["sun", "shade", "fire", "haze", "sky"]:
		var col := Color(md[key])
		var name_of: String = {
			"sun": "look_sun_tint",
			"shade": "look_shade_tint",
			"fire": "look_fire_tint",
			"haze": "look_haze_col",
			"sky": "look_sky_col"
		}[key]
		set_global.call(name_of, Vector3(col.r, col.g, col.b))
	set_global.call("look_haze_k", float(md.hazeK))
	set_global.call("look_desat", float(md.desat))
	_env.background_color = Color(md.haze)
	var fires: Array = painter.fires
	if fires.size() > 0:
		var f: Dictionary = fires[0]
		var power: float = 1.0 if name == "dusk" else float(f.power)
		var radius: float = 12.0 if name == "dusk" else float(f.radius)
		set_global.call("look_fire", Vector4(f.pos[0], f.pos[1], f.pos[2], radius))
		set_global.call("look_fire_power", power)
	_mark("hour", name)


func _set_outline(method: int) -> void:
	outline = method
	RenderingServer.global_shader_parameter_set("look_outline", method)
	_post.visible = method == 1 or method == 2
	var on := method == 3
	_gbuf.render_target_update_mode = (
		SubViewport.UPDATE_ALWAYS if on else SubViewport.UPDATE_DISABLED
	)
	_solid.next_pass = _hull if method == 4 else null
	_mark("outline", OUTLINES[method])


func _set_reflect(on: bool) -> void:
	reflect = on
	RenderingServer.global_shader_parameter_set("look_reflect", 1.0 if on else 0.0)
	_mirror.render_target_update_mode = (
		SubViewport.UPDATE_ALWAYS if on else SubViewport.UPDATE_DISABLED
	)
	_mark("mirror", "on" if on else "off")


## The picture's size: a quarter of the screen's pixels each way, with one spare art pixel on each
## side so the shift of a pan never shows an edge.
func _layout() -> void:
	var k := _screen_scale()
	var screen := Vector2(DisplayServer.window_get_size())
	if screen.x <= 0.0:
		screen = size * k
	var art := Vector2i(ceili(screen.x / ART) + 2, ceili(screen.y / ART) + 2)
	for vp: SubViewport in [_art, _gbuf, _mirror]:
		vp.size = art
	_view.size = Vector2(art) * ART / k
	_apply_camera()


## Screen pixels to one of the interface's units.
func _screen_scale() -> float:
	var window := DisplayServer.window_get_size()
	return window.x / size.x if size.x > 0.0 and window.x > 0 else 1.0


## The camera locked to the art pixels' grid (PRE-22): it moves in whole art pixels in its own axes,
## and the rest of each move shifts the picture by part of a pixel, so a pan is smooth and still.
func _apply_camera() -> void:
	if _art == null or _art.size.y <= 2:
		return
	var basis := Basis.from_euler(Vector3(deg_to_rad(-elev), deg_to_rad(yaw), 0.0), EULER_ORDER_YXZ)
	var pos := target + basis.z * BACK
	var cx := pos.dot(basis.x)
	var cy := pos.dot(basis.y)
	var cz := pos.dot(basis.z)
	var snap := crawl != 3 or _moving <= 0.0
	var sx: float = round(cx / mpp) * mpp if snap else cx
	var sy: float = round(cy / mpp) * mpp if snap else cy
	var xf := Transform3D(basis, basis.x * sx + basis.y * sy + basis.z * cz)
	for cam: Camera3D in [_cam, _gbuf_cam, _mirror_cam]:
		cam.size = float(_art.size.y) * mpp
		cam.transform = xf
	var frac := Vector2((cx - sx) / mpp, (cy - sy) / mpp)
	var k := _screen_scale()
	_view.position = (Vector2(-ART, -ART) + Vector2(-frac.x, frac.y) * ART) / k
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_mpp", mpp)
	set_global.call("look_cam_right", basis.x)


func _build_controls() -> void:
	var bar := VBoxContainer.new()
	bar.name = "Controls"
	bar.set_anchors_and_offsets_preset(Control.PRESET_BOTTOM_WIDE)
	bar.grow_vertical = Control.GROW_DIRECTION_BEGIN
	bar.add_theme_constant_override("separation", 4)
	add_child(bar)
	_readout = Label.new()
	_readout.add_theme_color_override("font_color", Color.WHITE)
	_readout.add_theme_color_override("font_outline_color", Color.BLACK)
	_readout.add_theme_constant_override("outline_size", 4)
	_readout.add_theme_font_size_override("font_size", 13)
	bar.add_child(_readout)
	var rows := [["back", "hour", "outline"], ["mirror", "crawl", "measure"]]
	for keys: Array in rows:
		var row := HBoxContainer.new()
		row.add_theme_constant_override("separation", 4)
		bar.add_child(row)
		for key: String in keys:
			var b := Button.new()
			b.custom_minimum_size = Vector2(0, 44)
			b.size_flags_horizontal = Control.SIZE_EXPAND_FILL
			b.add_theme_font_size_override("font_size", 14)
			b.pressed.connect(_on_button.bind(key))
			row.add_child(b)
			_buttons[key] = b
	_mark("back", "")
	_mark("measure", "")
	_mark("crawl", CRAWLS[crawl])


func _mark(key: String, value: String) -> void:
	if not _buttons.has(key):
		return
	var b: Button = _buttons[key]
	var names := {
		"back": "Back",
		"hour": "Hour: %s",
		"outline": "Outline: %s",
		"mirror": "Mirror: %s",
		"crawl": "Crawl fix: %s",
		"measure": "Measure",
	}
	var text: String = names[key]
	b.text = text % value if "%s" in text else text


func _on_button(key: String) -> void:
	match key:
		"back":
			closed.emit()
		"hour":
			_set_hour("dusk" if hour == "noon" else "noon")
		"outline":
			_set_outline((outline + 1) % OUTLINES.size())
		"mirror":
			_set_reflect(not reflect)
		"crawl":
			crawl = (crawl + 1) % CRAWLS.size()
			_mark("crawl", CRAWLS[crawl])
		"measure":
			if _run < 0:
				_start_measure()


func _process(delta: float) -> void:
	if _run >= 0:
		_measure_step(delta)
		return
	_frames += 1
	_clock += delta
	_gpu += RenderingServer.viewport_get_measured_render_time_gpu(_art.get_viewport_rid())
	if _gbuf.render_target_update_mode == SubViewport.UPDATE_ALWAYS:
		_gpu += RenderingServer.viewport_get_measured_render_time_gpu(_gbuf.get_viewport_rid())
	if _mirror.render_target_update_mode == SubViewport.UPDATE_ALWAYS:
		_gpu += RenderingServer.viewport_get_measured_render_time_gpu(_mirror.get_viewport_rid())
	if _clock >= 1.0:
		_readout.text = (
			"%d fps · graphics %.1f ms · art %d × %d · %.3f m a pixel"
			% [_frames / _clock, _gpu / _frames, _art.size.x - 2, _art.size.y - 2, mpp]
		)
		_frames = 0
		_gpu = 0.0
		_clock = 0.0
	_ease(delta)
	if shot != "" and Engine.get_process_frames() == 30:
		_save_shot()


## Measure (PLT-04): the graphics time of every pass and the share of frames on time at 60 a
## second, for each run of RUNS from the painter's view, then the results as a code for the chat.
func _start_measure() -> void:
	_before = [target, yaw, mpp, outline, reflect, crawl]
	crawl = 0
	_results = PackedStringArray()
	Engine.max_fps = 60
	_begin_run(0)


func _begin_run(i: int) -> void:
	_run = i
	var run: Array = RUNS[i]
	_set_outline(run[0])
	_set_reflect(run[1])
	_run_clock = 0.0
	_samples = PackedFloat32Array()
	_late = 0
	_readout.text = (
		"Measuring %d of %d: outline %s, mirror %s"
		% [i + 1, RUNS.size(), OUTLINES[run[0]], "on" if run[1] else "off"]
	)


func _measure_step(delta: float) -> void:
	_run_clock += delta
	var t := _run_clock
	var c: Dictionary = painter.camera
	var home := Vector3(c.target[0], c.target[1], c.target[2])
	var right := Vector3(cos(deg_to_rad(c.yaw)), 0.0, -sin(deg_to_rad(c.yaw)))
	target = home
	yaw = c.yaw
	mpp = c.mpp
	if t >= 1.0 and t < 3.0:
		target = home + right * 4.0 * sin((t - 1.0) * PI)
	elif t >= 3.0 and t < 5.0:
		yaw = c.yaw + 25.0 * sin((t - 3.0) * PI)
	elif t >= 5.0:
		mpp = c.mpp * (1.0 + 0.4 * sin((t - 5.0) * PI * 0.5))
	_apply_camera()
	if t >= 1.0:
		var gpu := RenderingServer.viewport_get_measured_render_time_gpu(_art.get_viewport_rid())
		for vp: SubViewport in [_gbuf, _mirror]:
			if vp.render_target_update_mode == SubViewport.UPDATE_ALWAYS:
				gpu += RenderingServer.viewport_get_measured_render_time_gpu(vp.get_viewport_rid())
		_samples.append(gpu)
		if delta > 1.15 / 60.0:
			_late += 1
	if t < RUN_TIME:
		return
	var sorted := _samples.duplicate()
	sorted.sort()
	var total := 0.0
	for g in sorted:
		total += g
	var n := maxi(sorted.size(), 1)
	var run: Array = RUNS[_run]
	_results.append(
		(
			"%s%s %.1f/%.1f %d%%"
			% [
				OUTLINES[run[0]],
				"+m" if run[1] else "",
				total / n,
				sorted[mini(int(n * 0.95), n - 1)] if sorted.size() > 0 else 0.0,
				roundi(100.0 * (n - _late) / n)
			]
		)
	)
	if _run + 1 < RUNS.size():
		_begin_run(_run + 1)
		return
	_run = -1
	Engine.max_fps = 0
	target = _before[0]
	yaw = _before[1]
	mpp = _before[2]
	crawl = _before[5]
	_set_outline(_before[3])
	_set_reflect(_before[4])
	_apply_camera()
	var phone: String = load("res://main.gd").facts().phone
	var line := (
		"P1 %s %s | %s"
		% [
			ProjectSettings.get_setting("application/config/version", ""),
			phone,
			" | ".join(_results)
		]
	)
	DisplayServer.clipboard_set(line)
	print(line)
	if "measure" in OS.get_cmdline_user_args():
		get_tree().quit()
	_readout.text = (
		"Copied for the chat (graphics ms, average/slowest 5%%, frames on time):\n%s" % line
	)


## The crawl fixes (PRE-22): with "ease", a turn or zoom eases to rest on the nearest whole step
## once the fingers lift; with "rest", the camera snaps to the pixel grid only when at rest.
func _ease(delta: float) -> void:
	_moving = maxf(_moving - delta, 0.0)
	if crawl != 2 or not _touches.is_empty():
		return
	var to_yaw := roundf(yaw / TURN_STEP) * TURN_STEP
	var to_mpp := _rest_mpp * pow(ZOOM_STEP, roundf(log(mpp / _rest_mpp) / log(ZOOM_STEP)))
	if absf(yaw - to_yaw) > 0.01 or absf(mpp - to_mpp) > 1e-5:
		var t := 1.0 - exp(-delta * 10.0)
		yaw = lerpf(yaw, to_yaw, t) if absf(yaw - to_yaw) > 0.05 else to_yaw
		mpp = lerpf(mpp, to_mpp, t) if absf(mpp - to_mpp) > 1e-4 else to_mpp
		_apply_camera()


func _save_shot() -> void:
	await RenderingServer.frame_post_draw
	var image := _art.get_texture().get_image()
	var inner := image.get_region(Rect2i(1, 1, image.get_width() - 2, image.get_height() - 2))
	inner.save_png(shot)
	get_tree().quit()


func _unhandled_input(e: InputEvent) -> void:
	if e is InputEventScreenTouch:
		var t := e as InputEventScreenTouch
		if t.pressed:
			_touches[t.index] = t.position
		else:
			_touches.erase(t.index)
		_twist = 0.0
		_pinch = 1.0
	elif e is InputEventScreenDrag:
		var d := e as InputEventScreenDrag
		var old: Vector2 = _touches.get(d.index, d.position)
		if _touches.size() == 1:
			_pan(d.position - old)
		elif _touches.size() >= 2:
			var other := Vector2.ZERO
			for k: int in _touches:
				if k != d.index:
					other = _touches[k]
					break
			var d0 := old - other
			var d1 := d.position - other
			if d0.length() > 4.0 and d1.length() > 4.0:
				_turn(rad_to_deg(d1.angle() - d0.angle()))
				_zoom(d0.length() / d1.length())
		_touches[d.index] = d.position
	elif e is InputEventMouseMotion:
		var m := e as InputEventMouseMotion
		if m.button_mask & MOUSE_BUTTON_MASK_LEFT:
			_pan(m.relative)
		elif m.button_mask & MOUSE_BUTTON_MASK_RIGHT:
			_turn(m.relative.x * 0.3)
	elif e is InputEventMouseButton and (e as InputEventMouseButton).pressed:
		var b := e as InputEventMouseButton
		if b.button_index == MOUSE_BUTTON_WHEEL_UP:
			_zoom(0.9)
		elif b.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			_zoom(1.1)


func _pan(by: Vector2) -> void:
	var k := _screen_scale() / ART
	var r := Vector3(cos(deg_to_rad(yaw)), 0, -sin(deg_to_rad(yaw)))
	var f := Vector3(-sin(deg_to_rad(yaw)), 0, -cos(deg_to_rad(yaw)))
	target += (-r * by.x + f * by.y / sin(deg_to_rad(elev))) * mpp * k
	_moving = 0.3
	_apply_camera()


## A turn by degrees: with "steps" it waits for a whole step of 15°.
func _turn(degrees: float) -> void:
	if crawl == 1:
		_twist += degrees
		if absf(_twist) < TURN_STEP:
			return
		degrees = signf(_twist) * TURN_STEP
		_twist = 0.0
	yaw = wrapf(yaw + degrees, -180.0, 180.0)
	_moving = 0.3
	_apply_camera()


## A zoom by a factor of the metres an art pixel shows: with "steps" in whole steps of 1.25 times.
func _zoom(factor: float) -> void:
	if crawl == 1:
		_pinch *= factor
		if absf(log(_pinch)) < log(ZOOM_STEP):
			return
		factor = ZOOM_STEP if _pinch > 1.0 else 1.0 / ZOOM_STEP
		_pinch = 1.0
	mpp = clampf(mpp * factor, 0.045, 0.18)
	_moving = 0.3
	_apply_camera()
