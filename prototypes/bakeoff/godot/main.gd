# Kindling engine bake-off, the Godot side (research/03-rendering.md). Builds the shared scene from assets/scene.json,
# draws it at one art pixel per 4 x 4 screen pixels with a camera locked to the pixel grid, and shows the frame time.
# Drag to pan, pinch to zoom, twist to turn; the buttons change the hour and reset the view. A throwaway prototype.
extends Node3D

const ART := 4                                  # screen pixels per art pixel (PRE-22)

var scene: Dictionary
var sub: SubViewport
var view: TextureRect
var cam: Camera3D
var sun: DirectionalLight3D
var env: Environment
var hud: Label
var target := Vector3.ZERO
var yaw := 0.0
var pitch := 35.0
var width_m := 24.0
var touches := {}
var hour := 2
var frames := 0
var acc_cpu := 0.0
var acc_gpu := 0.0
var acc_frame := 0.0
var shot_path := ""
var shot_frames := 90

const HOURS := [
	["morning", 14.0, 100.0, "#ffd9a8", 0.95],
	["noon", 62.0, 180.0, "#fff6e6", 1.15],
	["afternoon", 38.0, 230.0, "#fff1d6", 1.1],
	["evening", 7.0, 285.0, "#ffb27a", 0.9],
]


func _ready() -> void:
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("--shot="):
			shot_path = arg.substr(7)
		elif arg.begins_with("--frames="):
			shot_frames = int(arg.substr(9))
		elif arg.begins_with("--yaw="):
			yaw = float(arg.substr(6))
	scene = JSON.parse_string(FileAccess.get_file_as_string("res://assets/scene.json"))
	var c: Dictionary = scene["camera"]
	target = Vector3(c["target"][0], c["target"][1], c["target"][2])
	if yaw == 0.0:
		yaw = c["yaw_deg"]
	pitch = c["pitch_deg"]
	width_m = c["view_width_m"]
	_build_view()
	_build_world()
	_build_hud()
	_set_hour(hour)
	_apply_camera()


# --- The low-resolution image and its upscale. ---

func _build_view() -> void:
	sub = SubViewport.new()
	sub.msaa_3d = Viewport.MSAA_DISABLED
	sub.screen_space_aa = Viewport.SCREEN_SPACE_AA_DISABLED
	sub.use_taa = false
	sub.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	sub.positional_shadow_atlas_size = 0
	add_child(sub)
	RenderingServer.viewport_set_measure_render_time(sub.get_viewport_rid(), true)
	var layer := CanvasLayer.new()
	layer.layer = 0
	add_child(layer)
	view = TextureRect.new()
	view.texture = sub.get_texture()
	view.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	view.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	view.stretch_mode = TextureRect.STRETCH_SCALE
	view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	layer.add_child(view)
	get_viewport().size_changed.connect(_on_resize)
	_on_resize()


func _on_resize() -> void:
	var s := get_viewport().get_visible_rect().size
	# One spare art pixel on each side, so the sub-pixel shift never shows an edge.
	sub.size = Vector2i(int(ceil(s.x / ART)) + 2, int(ceil(s.y / ART)) + 2)
	view.size = Vector2(sub.size) * ART
	if cam:
		_apply_camera()


# --- The scene. ---

var materials := {}


func _ramp(name: String, i: int) -> Color:
	var r: Array = scene["ramps"][name]
	var c: Array = r[clamp(i, 0, r.size() - 1)]
	return Color(c[0], c[1], c[2]).linear_to_srgb()


func _tex(path: String) -> Texture2D:
	return load("res://assets/" + path)


func _material(name: String) -> Material:
	# Each material name the models use gets its shader from scene.json's table (research 04), made once.
	if materials.has(name):
		return materials[name]
	var spec: Dictionary = scene["materials"].get(name, {"shader": "textured", "texture": "textures/bark.png"})
	var m := ShaderMaterial.new()
	match spec["shader"]:
		"rock":
			m.shader = preload("res://shaders/rock.gdshader")
			m.set_shader_parameter("side", _tex(spec["side"]))
			m.set_shader_parameter("top", _tex(spec["top"]))
		"textured":
			m.shader = preload("res://shaders/textured.gdshader")
			m.set_shader_parameter("tex", _tex(spec["texture"]))
			m.set_shader_parameter("uv_metres", Vector2(spec["uv_metres"][0], spec["uv_metres"][1]))
		"leaf":
			m.shader = preload("res://shaders/leaf.gdshader")
			m.set_shader_parameter("sprites", _tex(scene["sprites"]))
			m.set_shader_parameter("tint", _ramp(spec["ramp"], 3))
			m.render_priority = 1
		"flame":
			m.shader = preload("res://shaders/flame.gdshader")
			m.set_shader_parameter("sprites", _tex(scene["sprites"]))
			m.render_priority = 1
	materials[name] = m
	return m


func _repaint(node: Node) -> void:
	# Every surface of a model gets the material its name asks for.
	if node is MeshInstance3D:
		var mi := node as MeshInstance3D
		for i in mi.mesh.get_surface_count():
			var src := mi.mesh.surface_get_material(i)
			mi.set_surface_override_material(i, _material(src.resource_name if src else ""))
			if src and src.resource_name == "flame":
				mi.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	for child in node.get_children():
		_repaint(child)


func _build_world() -> void:
	var world := Node3D.new()
	sub.add_child(world)
	cam = Camera3D.new()
	cam.projection = Camera3D.PROJECTION_ORTHOGONAL
	cam.near = 0.05
	cam.far = 400.0
	cam.current = true
	world.add_child(cam)

	env = Environment.new()
	env.background_mode = Environment.BG_COLOR
	env.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	env.ambient_light_energy = 0.6
	env.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	var we := WorldEnvironment.new()
	we.environment = env
	world.add_child(we)

	sun = DirectionalLight3D.new()
	sun.shadow_enabled = true
	sun.directional_shadow_mode = DirectionalLight3D.SHADOW_ORTHOGONAL
	sun.directional_shadow_max_distance = 90.0
	sun.shadow_bias = 0.04
	sun.shadow_normal_bias = 0.6
	world.add_child(sun)

	var ground: Node3D = load("res://assets/ground.glb").instantiate()
	world.add_child(ground)
	var gm := ShaderMaterial.new()
	gm.shader = preload("res://shaders/ground.gdshader")
	gm.set_shader_parameter("g0", _ramp("grass", 1))
	gm.set_shader_parameter("g1", _ramp("grass", 2))
	gm.set_shader_parameter("g2", _ramp("grass", 3))
	gm.set_shader_parameter("sand", _ramp("sand", 2))
	gm.set_shader_parameter("bed", _ramp("sand", 0))
	gm.set_shader_parameter("dirt", _tex(scene["dirt"]))
	gm.set_shader_parameter("strata", _tex(scene["materials"]["rock"]["side"]))
	_set_material(ground, gm)

	for p in scene["models"]:
		var m: Node3D = load("res://assets/models/%s.glb" % p["model"]).instantiate()
		var s = p["scale"]
		var sc := Vector3(s[0], s[1], s[2]) if s is Array else Vector3(s, s, s)
		m.transform = Transform3D(Basis(Vector3.UP, p["turn"]).scaled(sc), Vector3(p["x"], p["y"], p["z"]))
		_repaint(m)
		world.add_child(m)

	var water: Node3D = load("res://assets/water.glb").instantiate()
	var wm := ShaderMaterial.new()
	wm.shader = preload("res://shaders/water.gdshader")
	var w: Dictionary = scene["water"]["colours"]
	wm.set_shader_parameter("shallow", Color(w["shallow"]))
	wm.set_shader_parameter("deep", Color(w["deep"]))
	wm.set_shader_parameter("foam", Color(w["foam"]))
	wm.render_priority = 1
	_set_material(water, wm)
	world.add_child(water)

	world.add_child(_grass())

	# The outline pass: a quad over the whole image, drawn after the opaque pass and before grass and water.
	var quad := MeshInstance3D.new()
	var qm := QuadMesh.new()
	qm.size = Vector2(1, 1)
	quad.mesh = qm
	var om := ShaderMaterial.new()
	om.shader = preload("res://shaders/outline.gdshader")
	om.render_priority = 0
	quad.material_override = om
	quad.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	quad.extra_cull_margin = 16384.0
	cam.add_child(quad)
	quad.position = Vector3(0, 0, -1)


func _set_material(node: Node, mat: Material) -> void:
	if node is MeshInstance3D:
		(node as MeshInstance3D).material_override = mat
	for child in node.get_children():
		_set_material(child, mat)


func _grass() -> MultiMeshInstance3D:
	var g: Dictionary = scene["grass"]
	var bytes := FileAccess.get_file_as_bytes("res://assets/" + g["file"])
	var f := bytes.to_float32_array()
	var count: int = g["count"]
	var h: float = g["height_m"]
	var quad := QuadMesh.new()
	quad.size = Vector2(h, h)
	quad.center_offset = Vector3(0, h * 0.5, 0)
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.use_custom_data = true
	mm.mesh = quad
	mm.instance_count = count
	var buf := PackedFloat32Array()
	buf.resize(count * 16)
	for i in count:
		var o := i * 8
		var s := f[o + 6]
		var b := i * 16
		buf[b + 0] = s; buf[b + 1] = 0.0; buf[b + 2] = 0.0; buf[b + 3] = f[o]
		buf[b + 4] = 0.0; buf[b + 5] = s; buf[b + 6] = 0.0; buf[b + 7] = f[o + 1]
		buf[b + 8] = 0.0; buf[b + 9] = 0.0; buf[b + 10] = s; buf[b + 11] = f[o + 2]
		buf[b + 12] = f[o + 3]; buf[b + 13] = f[o + 4]; buf[b + 14] = f[o + 5]; buf[b + 15] = f[o + 7]
	mm.buffer = buf
	var mi := MultiMeshInstance3D.new()
	mi.multimesh = mm
	var gm := ShaderMaterial.new()
	gm.shader = preload("res://shaders/grass.gdshader")
	gm.set_shader_parameter("card", _tex(scene["sprites"]))
	gm.render_priority = 1
	mi.material_override = gm
	mi.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	return mi


func _set_hour(i: int) -> void:
	hour = i % HOURS.size()
	var hr: Array = HOURS[hour]
	var el := deg_to_rad(hr[1])
	var az := deg_to_rad(hr[2])
	var to_sun := Vector3(cos(el) * sin(az), sin(el), -cos(el) * cos(az))
	sun.global_transform = Transform3D(Basis.looking_at(-to_sun, Vector3.UP if abs(to_sun.y) < 0.99 else Vector3.BACK), Vector3.ZERO)
	sun.light_color = Color(hr[3])
	sun.light_energy = hr[4]
	env.background_color = Color(scene["light"]["sky"])
	env.ambient_light_color = Color(scene["light"]["ambient"]) if hour != 3 else Color("#6a66a0")
	env.ambient_light_energy = 0.85


# --- The camera, locked to the art-pixel grid (research 03, step 2). ---

func _apply_camera() -> void:
	var basis := Basis.from_euler(Vector3(deg_to_rad(-pitch), deg_to_rad(yaw), 0.0), EULER_ORDER_YXZ)
	var art_w := float(sub.size.x - 2)
	var texel := width_m / art_w
	cam.size = float(sub.size.y) * texel
	var pos := target + basis.z * 150.0
	var cx := pos.dot(basis.x)
	var cy := pos.dot(basis.y)
	var cz := pos.dot(basis.z)
	var sx: float = round(cx / texel) * texel
	var sy: float = round(cy / texel) * texel
	cam.global_transform = Transform3D(basis, basis.x * sx + basis.y * sy + basis.z * cz)
	var frac := Vector2((cx - sx) / texel, (cy - sy) / texel)
	view.position = Vector2(-ART, -ART) + Vector2(-frac.x, frac.y) * ART


func _pan(delta: Vector2) -> void:
	var texel := width_m / float(sub.size.x - 2)
	var m := texel / ART
	var r := Vector3(cos(deg_to_rad(yaw)), 0, -sin(deg_to_rad(yaw)))
	var f := Vector3(-sin(deg_to_rad(yaw)), 0, -cos(deg_to_rad(yaw)))
	target += -r * delta.x * m + f * delta.y * m / sin(deg_to_rad(pitch))
	_apply_camera()


func _zoom(factor: float) -> void:
	var c: Dictionary = scene["camera"]
	width_m = clamp(width_m * factor, c["min_width_m"], c["max_width_m"])
	_apply_camera()


func _unhandled_input(e: InputEvent) -> void:
	if e is InputEventScreenTouch:
		if e.pressed:
			touches[e.index] = e.position
		else:
			touches.erase(e.index)
	elif e is InputEventScreenDrag:
		var old: Vector2 = touches.get(e.index, e.position)
		if touches.size() == 1:
			_pan(e.position - old)
		elif touches.size() >= 2:
			var other: Vector2 = Vector2.ZERO
			for k in touches:
				if k != e.index:
					other = touches[k]
					break
			var d0 := old - other
			var d1: Vector2 = e.position - other
			if d0.length() > 4.0 and d1.length() > 4.0:
				width_m = clamp(width_m * d0.length() / d1.length(), scene["camera"]["min_width_m"],
					scene["camera"]["max_width_m"])
				yaw += rad_to_deg(d1.angle() - d0.angle())
				_apply_camera()
		touches[e.index] = e.position
	elif e is InputEventMouseMotion and (e.button_mask & MOUSE_BUTTON_MASK_LEFT):
		_pan(e.relative)
	elif e is InputEventMouseButton and e.pressed:
		if e.button_index == MOUSE_BUTTON_WHEEL_UP:
			_zoom(0.9)
		elif e.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			_zoom(1.1)


# --- The readout and buttons. ---

func _build_hud() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 2
	add_child(layer)
	var box := VBoxContainer.new()
	box.position = Vector2(16, 48)
	layer.add_child(box)
	hud = Label.new()
	hud.add_theme_font_size_override("font_size", 28)
	hud.add_theme_color_override("font_color", Color.WHITE)
	hud.add_theme_color_override("font_outline_color", Color.BLACK)
	hud.add_theme_constant_override("outline_size", 8)
	box.add_child(hud)
	var row := HBoxContainer.new()
	box.add_child(row)
	for label in ["Hour", "Reset"]:
		var b := Button.new()
		b.text = label
		b.add_theme_font_size_override("font_size", 30)
		b.custom_minimum_size = Vector2(150, 72)
		row.add_child(b)
		b.pressed.connect(_on_button.bind(label))


func _on_button(label: String) -> void:
	if label == "Hour":
		_set_hour(hour + 1)
	else:
		var c: Dictionary = scene["camera"]
		target = Vector3(c["target"][0], c["target"][1], c["target"][2])
		yaw = c["yaw_deg"]
		width_m = c["view_width_m"]
		_apply_camera()


func _process(delta: float) -> void:
	var rid := sub.get_viewport_rid()
	acc_cpu += RenderingServer.viewport_get_measured_render_time_cpu(rid)
	acc_gpu += RenderingServer.viewport_get_measured_render_time_gpu(rid)
	acc_frame += delta
	frames += 1
	if acc_frame >= 1.0:
		hud.text = "Godot %s · Forward+ · %s\nart %d×%d · %d fps\nframe %.1f ms · render cpu %.1f gpu %.1f ms\n%s" % [
			Engine.get_version_info()["string"].split(".stable")[0], OS.get_name(), sub.size.x - 2, sub.size.y - 2,
			Engine.get_frames_per_second(), 1000.0 * acc_frame / frames, acc_cpu / frames, acc_gpu / frames,
			HOURS[hour][0]]
		frames = 0
		acc_cpu = 0.0
		acc_gpu = 0.0
		acc_frame = 0.0
	if shot_path != "":
		shot_frames -= 1
		if shot_frames == 0:
			await RenderingServer.frame_post_draw
			sub.get_texture().get_image().save_png(shot_path.replace(".png", "-art.png"))
			get_viewport().get_texture().get_image().save_png(shot_path)
			get_tree().quit()
