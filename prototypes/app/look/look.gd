## P1 The look (IMPLEMENTATION α0.2a): the art book's close camp drawn by Godot's Mobile renderer
## at a quarter of the screen's size, one art pixel to 4 × 4 screen pixels, with a camera locked to
## the art pixels' grid. Switches: the hour, the four outline methods, the mirrored water, and the
## fixes for crawling in free turns and zooms; Measure runs the camera along a set path for each.
## Pre-production code (research 00): the app's README names the items it is about.
extends Control

signal closed

const ART := 4
const BACK := 100.0
const SCENE := "res://look/close_camp.scn"
const OUTLINES := ["none", "A", "B", "C", "D"]
const CRAWLS := ["free", "steps", "ease", "rest"]
## The "ease" fix's steps: a turn settles on the nearest 5°, a zoom on the nearest 1.25 times.
const TURN_STEP := 5.0
const ZOOM_STEP := 1.25
const LAYER_MAIN := 1
const LAYER_GBUF := 2
const LAYER_SKY := 4
const LAYER_MIRROR := 8
const LAYER_OCC := 16
const Shape := preload("res://look/shape.gd")
const SmokePath := preload("res://look/smoke_path.gd")
const Runs := preload("res://look/runs.gd")
const P1Measure := preload("res://look/p1_measure.gd")
## The heights round the fires, for their shadows: the square's side in metres, and its pixels.
const OCC_SIDE := 48.0
## Where a fire's light comes from: this far above the fire's place, in its flames.
const FIRE_LIGHT_UP := 0.5
## The sun's least height, degrees (_high_enough): the art book's own dusk, 5° up, until you choose
## a higher one.
const MIN_SUN := 5.0
const OCC_SIZE := 512
## Flames change and flicker about 10 times a second, as pixel art moves (PRE-44): four frames each.
const FLAME_TIME := 0.1
const FLAME_FRAMES := 4
## The atlas's flame pictures, small, middle and large (about 0.35, 0.55 and 0.8 m), two shapes of
## each, and its dot for sparks (the painter's atlas.js), and the size of its pictures in texels.
const FLAME_TILES := [[14, 15, 16], [25, 26, 27]]
const DOT_TILE := 24
const TILE := 64

var painter: Dictionary
var target := Vector3.ZERO
var yaw := 16.0
var elev := 30.0
var mpp := 0.09
var hour := "noon"
var outline := 3
var reflect := false
var crawl := 2
var shot := ""
## For the cloud's check that a moving picture's outlines keep up: the camera slides into place.
var drift := false
## Whether smoke is drawn: "smoke=off" on the command line leaves it out, for the cloud's picture
## check of what the smoke adds (tools/tests/test_pictures.py).
var smoke_on := true
## Who lights the fires: "ours", the firelight term, or "Godot's" omni lights.
var fire_light := "ours"
## How far a pinch may zoom, in metres an art pixel shows.
var zoom_range := Vector2(0.045, 0.18)
## How far back the camera stands, metres: far enough that the nearest ground in view is in
## front of it.
var back := BACK
## The readout keeps Measure's message, its progress and then its line, until the next touch.
var _hold := false

var _art: SubViewport
var _gbuf: SubViewport
var _mirror: SubViewport
var _sky: SubViewport
var _occ: SubViewport
var _occ_cam: Camera3D
var _occ_floor: SubViewport
var _occ_floor_cam: Camera3D
## The fires lit by our firelight term or by Godot's own lights: each [place, reach, power].
var _fires := []
var _omni: Array[OmniLight3D] = []
## Measure's runner: P1's own, or a screen's runs (_measure_runs).
var _runs: RefCounted
## Each fire's flames: its nodes (the flames and their data copies) and its frames.
var _flames := []
## Each fire's flicker, a share of its power changed with its flames' frame.
var _flicker := PackedFloat32Array([1.0, 1.0, 1.0, 1.0])
var _flame_clock := 0.0
var _flame_step := 0
## The flicker's own sequence, seeded, so a screen flickers the same way each time it opens.
var _rng := RandomNumberGenerator.new()
## The smokes' materials, for the hour's step of their ramp, and their boxes.
var _smokes: Array[ShaderMaterial] = []
var _smoke_boxes: Array[MeshInstance3D] = []
## Where the heights round the fires lie: the square's corner, its side, and 1 while they are drawn.
var _occ_box := Vector4(0.0, 0.0, 1.0, 0.0)
var _view: TextureRect
var _cam: Camera3D
var _gbuf_cam: Camera3D
var _mirror_cam: Camera3D
var _sun: DirectionalLight3D
var _env: Environment
var _post: MeshInstance3D
var _hull: ShaderMaterial
var _solid: ShaderMaterial
var _cards: ShaderMaterial
## Each base material's variants for the data passes, by layer.
var _variants := {}
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


func _ready() -> void:
	_rng.seed = 7
	var scene := _load_scene()
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
		elif arg == "drift":
			drift = true
		elif arg.begins_with("yaw="):
			yaw = float(arg.substr(4))
		elif arg.begins_with("crawl="):
			crawl = maxi(CRAWLS.find(arg.substr(6)), 0)
		elif arg == "smoke=off":
			smoke_on = false
		elif arg == "measure":
			_measure.call_deferred()
	_build_views(scene)
	_build_materials(scene)
	_build_controls()
	_globals_once(scene)
	_set_back(BACK, 220.0)
	var hearth: Array = painter.fires[0].pos if painter.fires.size() > 0 else c.target
	_set_occ(Vector3(hearth[0], hearth[1], hearth[2]))
	if painter.fires.size() > 0:
		# the painter's hearth, with its own flames, lit and flickering as the screens' own fires are
		_add_fire(Vector3(hearth[0], hearth[1], hearth[2]), _hearth_reach(), 1.0, scene, false)
		_add_smoke(Vector3(hearth[0], hearth[1], hearth[2]), 1.0, scene)
	_set_smoke_tone()
	_set_hour(hour)
	_set_outline(outline)
	_set_reflect(reflect)
	resized.connect(_layout)
	_layout()


## How far the painter's hearth throws its light, metres.
func _hearth_reach() -> float:
	return 12.0


## The scene to draw: the painter's close camp, its palette, light, look and camera in its meta.
func _load_scene() -> Node3D:
	return (load(SCENE) as PackedScene).instantiate()


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
	_sun.shadow_bias = 0.08
	_sun.shadow_normal_bias = 1.6
	_art.add_child(_sun)
	_gbuf = _viewport(true, true)
	_gbuf.world_3d = _art.world_3d
	_gbuf_cam = _camera(LAYER_GBUF)
	_gbuf.add_child(_gbuf_cam)
	# inside the picture's viewport, so Godot draws it first in each frame: drawn after, the picture
	# would read the last frame's outlines and reflections, which jump as the camera moves
	_art.add_child(_gbuf)
	_mirror = _viewport(false, true)
	_mirror.world_3d = _art.world_3d
	_mirror_cam = _camera(LAYER_MIRROR)
	_mirror.add_child(_mirror_cam)
	_art.add_child(_mirror)
	_occ = _occ_view()
	_occ_cam = _occ.get_child(0)
	_occ_floor = _occ_view()
	_occ_floor_cam = _occ_floor.get_child(0)
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


## A view of the heights round the fires, from above or below (_set_occ), drawn before the picture.
func _occ_view() -> SubViewport:
	var vp := _viewport(true, true)
	vp.world_3d = _art.world_3d
	vp.size = Vector2i(OCC_SIZE, OCC_SIZE)
	var cam := Camera3D.new()
	cam.projection = Camera3D.PROJECTION_ORTHOGONAL
	cam.cull_mask = LAYER_OCC
	cam.near = 1.0
	cam.far = 2000.0
	vp.add_child(cam)
	_art.add_child(vp)
	return vp


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
	_cards = ShaderMaterial.new()
	_cards.shader = load("res://look/shaders/cards.gdshader")
	var water := ShaderMaterial.new()
	water.shader = load("res://look/shaders/water.gdshader")
	water.set_shader_parameter("water_row", float(rows.water))
	water.set_shader_parameter("foam_row", float(rows.white))
	water.render_priority = 1
	_hull = _variant(_solid, "HULL")
	for base: ShaderMaterial in [_ground, _solid, _cards]:
		_variants[base] = {
			LAYER_GBUF: _variant(base, "PREPASS"),
			LAYER_SKY: _variant(base, "SKYMAP"),
			LAYER_MIRROR: _variant(base, "MIRROR"),
			LAYER_OCC: _variant(base, "OCCMAP"),
		}
	for node: MeshInstance3D in scene.get_children():
		var kind: String = node.get_meta("kind")
		var mat: ShaderMaterial
		if kind == "ground":
			var l: Array = node.get_meta("layers")
			var p: Array = node.get_meta("layerPat")
			for m: ShaderMaterial in [_ground, _variants[_ground][LAYER_MIRROR]]:
				m.set_shader_parameter("layers", Vector4(l[0], l[1], l[2], l[3]))
				m.set_shader_parameter("layer_pat", Vector4(p[0], p[1], p[2], p[3]))
			mat = _ground
		elif kind == "solid":
			mat = _solid
		elif kind == "cards":
			mat = _cards
		elif kind == "water":
			node.material_override = water
			node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
			node.layers = LAYER_MAIN
			continue
		else:
			# the painter's flat puffs give way to the smoke's volume (_add_smoke)
			node.visible = false
			continue
		node.material_override = mat
		node.layers = LAYER_MAIN
		for layer: int in _data_layers(mat):
			var copy := MeshInstance3D.new()
			copy.mesh = node.mesh
			copy.material_override = _variants[mat][layer]
			copy.layers = layer
			copy.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
			scene.add_child(copy)


## Adds a shape drawn with one of the look's materials, with its copies for the data passes (the
## outline data, the sky's heights and the mirror), each in its own layer with its own variant, as
## every shape of the scene has: without them outline C reads the shape as all edge. A copy shares
## its node's mesh or multimesh, so instanced copies move together. Returns the node and its
## copies.
func _add_shape(
	node: GeometryInstance3D, base: ShaderMaterial, parent: Node3D
) -> Array[GeometryInstance3D]:
	node.material_override = base
	node.layers = LAYER_MAIN
	parent.add_child(node)
	var nodes: Array[GeometryInstance3D] = [node]
	for layer: int in _data_layers(base):
		var copy: GeometryInstance3D = node.duplicate()
		copy.material_override = _variants[base][layer]
		copy.layers = layer
		copy.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
		parent.add_child(copy)
		nodes.append(copy)
	return nodes


## The camera's distance, and what follows it: how far it sees, how far the sun's shadows reach,
## where the haze lies and the depths the outline data holds, all measured from the camera.
## reach: how deep the ground in view runs beyond the target, metres; the ground nearer the camera
## than the target runs no deeper, so the view starts there, and the sun's shadow map, which spans
## the view from its start, spends none of its pixels on the empty air before it.
func _set_back(distance: float, reach: float) -> void:
	back = distance
	for cam: Camera3D in [_cam, _gbuf_cam, _mirror_cam]:
		cam.near = maxf(1.0, distance - reach)
		cam.far = maxf(distance * 3.0, distance + reach + 300.0)
	_sun.directional_shadow_max_distance = distance + reach
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_haze_range", Vector2(distance + 20.0, distance + 20.0 + reach * 0.636))
	set_global.call("look_gbuf_depth", Vector2(distance - reach + 20.0, (reach - 20.0) * 2.0))
	_apply_camera()


## The data passes a material's shapes take part in: all but the ground stand in the heights
## round the fires, since the ground casts no fire's shadow.
func _data_layers(base: ShaderMaterial) -> Array[int]:
	if base == _ground:
		return [LAYER_GBUF, LAYER_SKY, LAYER_MIRROR]
	return [LAYER_GBUF, LAYER_SKY, LAYER_MIRROR, LAYER_OCC]


## The heights round the fires (PRE-30), for their shadows: a square OCC_SIDE metres across round
## centre, redrawn each frame as people move, seen from above as the sky's map is, for the tops of
## what stands there, and from below, for the undersides of roofs (mirrored left to right). Nothing
## higher than 2.4 m above centre goes in, so an overhang shades no fire.
func _set_occ(centre: Vector3) -> void:
	_occ_cam.size = OCC_SIDE
	_occ_cam.transform = Transform3D(
		Basis.looking_at(Vector3.DOWN, Vector3.FORWARD), centre + Vector3(0.0, 600.0, 0.0)
	)
	_occ_floor_cam.size = OCC_SIDE
	_occ_floor_cam.transform = Transform3D(
		Basis.looking_at(Vector3.UP, Vector3.FORWARD), centre - Vector3(0.0, 600.0, 0.0)
	)
	var set_global := RenderingServer.global_shader_parameter_set
	var corner := Vector2(centre.x - OCC_SIDE * 0.5, centre.z - OCC_SIDE * 0.5)
	_occ_box = Vector4(corner.x, corner.y, OCC_SIDE, 1.0)
	set_global.call("look_occ_box", _occ_box)
	set_global.call("look_occ_cap", centre.y + 2.4)
	set_global.call("look_occ_map", _occ.get_texture())
	set_global.call("look_occ_floor", _occ_floor.get_texture())


## Smoke from a fire (PRE-30): a volume round a path worked out from the scene, drawn by
## smoke.gdshader. size: 1 for a hearth, less for a small fire.
func _add_smoke(fire: Vector3, size: float, parent: Node3D) -> void:
	var path := SmokePath.find(fire, size, _art.get_child(0))
	var lo := Vector3(INF, INF, INF)
	var hi := -lo
	var nodes := PackedVector4Array()
	for i in range(0, path.size(), maxi(1, path.size() / 16)):
		if nodes.size() == 16:
			break
		var n: Vector4 = path[i]
		var at := Vector3(n.x, n.y, n.z)
		lo = lo.min(at - Vector3.ONE * n.w * 2.0)
		hi = hi.max(at + Vector3.ONE * n.w * 2.0)
		nodes.append(n)
	var count := nodes.size()
	nodes.resize(16)
	var m := ShaderMaterial.new()
	m.shader = load("res://look/shaders/smoke.gdshader")
	m.set_shader_parameter("nodes", nodes)
	m.set_shader_parameter("node_count", count)
	m.set_shader_parameter("box_min", lo)
	m.set_shader_parameter("box_max", hi)
	m.set_shader_parameter("smoke_row", float(painter.rows.smoke))
	m.set_shader_parameter("strength", size)
	m.render_priority = 3
	var box := MeshInstance3D.new()
	var mesh := BoxMesh.new()
	mesh.size = hi - lo
	box.mesh = mesh
	box.position = (lo + hi) * 0.5
	box.material_override = m
	box.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	box.layers = LAYER_MAIN
	box.visible = smoke_on
	parent.add_child(box)
	_smokes.append(m)
	_smoke_boxes.append(box)


## The fires' shadows and smoke on or off: off where they would be a few pixels at most, as at camp
## zoom, so the two maps of the heights round the fires and the smoke's march cost nothing (the
## shaders read the map's w of 0 as off).
func _set_fire_detail(on: bool) -> void:
	for vp: SubViewport in [_occ, _occ_floor]:
		vp.render_target_update_mode = (
			SubViewport.UPDATE_ALWAYS if on else SubViewport.UPDATE_DISABLED
		)
	_occ_box.w = 1.0 if on else 0.0
	RenderingServer.global_shader_parameter_set("look_occ_box", _occ_box)
	for box in _smoke_boxes:
		box.visible = on and smoke_on


## The step of the smoke's ramp at this hour, a mid grey by day and dark at night, and how much of
## the day's broken-up look it takes.
func _set_smoke_tone() -> void:
	for m in _smokes:
		m.set_shader_parameter("tone", {"noon": 4.0, "dusk": 3.5}.get(hour, 2.5))
		m.set_shader_parameter("day", {"noon": 1.0, "dusk": 0.5}.get(hour, 0.0))


## A fire of the scene's own (PRE-30, MAT-18): its place, reach in metres and power, its flames
## unless the scene has its own, and a Godot omni light for comparison, off unless chosen.
func _add_fire(at: Vector3, reach: float, power: float, parent: Node3D, flames := true) -> void:
	_fires.append([at, reach, power])
	if flames:
		var frames: Array[ArrayMesh] = []
		for k in FLAME_FRAMES:
			frames.append(_flame_frame(power, _fires.size() * 31 + k))
		var flame := MeshInstance3D.new()
		flame.mesh = frames[0]
		flame.position = at
		flame.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
		_flames.append({"nodes": _add_shape(flame, _cards, parent), "frames": frames})
	var light := OmniLight3D.new()
	light.position = at + Vector3(0.0, 0.8, 0.0)
	light.omni_range = reach
	light.light_color = Color(painter.moods.night.fire)
	light.layers = LAYER_MAIN
	light.visible = fire_light == "Godot's"
	parent.add_child(light)
	_omni.append(light)


## A frame of a fire's flames, as the art book's hearths have them (the painter's things.js): the
## atlas's flame pictures standing over the fire, red at their edges and white-hot at their core,
## each frame with its own shapes, places and sides, and a few sparks above; a big fire (power 1)
## has five flames, a smaller one three.
func _flame_frame(power: float, seed: int) -> ArrayMesh:
	var rng := RandomNumberGenerator.new()
	rng.seed = seed
	var rows: Dictionary = painter.rows
	var side := TILE * float(painter.get("atlasMpp", painter.camera.mpp))
	var flags := int(Shape.EMISSIVE + Shape.NO_OUTLINE) + 16
	var list := []
	for size: int in [1, 2, 1, 0, 2] if power >= 1.0 else [1, 0, 1]:
		var tiles: Array = FLAME_TILES[rng.randi() % 2]
		var foot := Vector3(rng.randf_range(-0.12, 0.12), 0.05, rng.randf_range(-0.12, 0.12))
		var w := side * (1.0 if rng.randf() < 0.5 else -1.0)
		list.append([foot, w, side, tiles[size], rows.fire, 3, flags])
	var spark := float(painter.camera.mpp) * 1.2
	for k in 3 if power >= 1.0 else 2:
		var at := Vector3(
			rng.randf_range(-0.3, 0.3), rng.randf_range(0.6, 1.6), rng.randf_range(-0.3, 0.3)
		)
		list.append([at, spark, spark, DOT_TILE, rows.ember, 3 + rng.randi() % 3, flags])
	return Shape.cards(list)


## The flames' next frame and the fires' flicker, about 10 times a second.
func _step_flames() -> void:
	_flame_step += 1
	for f: Dictionary in _flames:
		var frames: Array = f.frames
		var mesh: ArrayMesh = frames[_flame_step % frames.size()]
		for node: MeshInstance3D in f.nodes:
			node.mesh = mesh
	for i in _flicker.size():
		_flicker[i] = _rng.randf_range(0.82, 1.0)
	if not _fires.is_empty():
		_light_fires()


## Our firelight term in the shared light function, fed by the fires' list (A4.1), or Godot's own
## omni lights, to see which of the figures' instanced copies stay lit (MAT-18).
func _set_fire_light(which: String) -> void:
	fire_light = which
	for light in _omni:
		light.visible = which == "Godot's"
	_light_fires()
	_mark("fire", which)


## The fires' powers for our firelight term: none when Godot's lights do the work, and the fires
## burning low by day.
func fire_powers() -> Vector4:
	var powers := Vector4.ZERO
	if fire_light != "ours":
		return powers
	for i in _fires.size():
		powers[i] = float(_fires[i][2]) * _burning()
	return powers


func _light_fires() -> void:
	var lit := Projection()
	for i in _fires.size():
		var f: Array = _fires[i]
		var at: Vector3 = f[0]
		lit = _with_column(lit, i, Vector4(at.x, at.y + FIRE_LIGHT_UP, at.z, f[1]))
		if i < _omni.size():
			_omni[i].light_energy = 2.0 * float(f[2]) * _burning() * _flicker[i]
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_fires", lit)
	set_global.call("look_fire_powers", fire_powers())
	set_global.call(
		"look_fire_flicker", Vector4(_flicker[0], _flicker[1], _flicker[2], _flicker[3])
	)


## How high the fires burn against the daylight: low at noon, higher at dusk, full at night.
func _burning() -> float:
	return {"noon": 0.35, "dusk": 0.6}.get(hour, 1.0)


## The way to the sun, at least MIN_SUN degrees up. At the art book's own 5°, which you kept, it
## changes nothing; it is where a higher dusk sun would be set.
static func _high_enough(to_sun: Vector3) -> Vector3:
	var flat := Vector2(to_sun.x, to_sun.z).normalized()
	var up := maxf(asin(clampf(to_sun.normalized().y, -1.0, 1.0)), deg_to_rad(MIN_SUN))
	return Vector3(flat.x * cos(up), sin(up), flat.y * cos(up))


static func _with_column(p: Projection, i: int, v: Vector4) -> Projection:
	match i:
		0:
			p.x = v
		1:
			p.y = v
		2:
			p.z = v
		_:
			p.w = v
	return p


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
	var tint: Array = painter.waterTint
	set_global.call("look_water_tint", Vector3(tint[0], tint[1], tint[2]))
	set_global.call("look_mirror_y", float(painter.waterLevel))
	set_global.call("look_gbuf", _gbuf.get_texture())
	set_global.call("look_mirror", _mirror.get_texture())
	set_global.call("look_elev_sin", sin(deg_to_rad(elev)))
	var soot: Array = painter.soot
	var s: Array = soot[0] if soot.size() > 0 else [0, 0, 0, 0]
	set_global.call("look_soot", Vector4(s[0], s[1], s[2], s[3]))


## The hour's light from the painter (PRE-30): the sun's way and tints, and the fires, which burn
## higher after noon (_burning).
func _set_hour(name: String) -> void:
	hour = name
	var md: Dictionary = painter.moods[name]
	var set_global := RenderingServer.global_shader_parameter_set
	var to_sun := _high_enough(Vector3(md.sunDir[0], md.sunDir[1], md.sunDir[2]))
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
	# a fire's light shows on sunlit ground only once the sun is low or gone
	set_global.call("look_fire_in_sun", {"noon": 0.0, "dusk": 0.5}.get(name, 1.0))
	_env.background_color = Color(md.haze)
	_set_smoke_tone()
	if not _fires.is_empty():
		_light_fires()
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
	var pos := target + basis.z * back
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
	for keys: Array in _control_rows():
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


## The buttons, row by row.
func _control_rows() -> Array:
	return [["back", "hour", "outline"], ["mirror", "crawl", "measure"]]


## A button's text, with %s where its state goes.
func _label(key: String) -> String:
	return (
		{
			"back": "Back",
			"hour": "Hour: %s",
			"outline": "Outline: %s",
			"mirror": "Mirror: %s",
			"crawl": "Crawl fix: %s",
			"measure": "Measure",
		}
		. get(key, key)
	)


func _mark(key: String, value: String) -> void:
	if not _buttons.has(key):
		return
	var b: Button = _buttons[key]
	var text := _label(key)
	b.text = text % value if "%s" in text else text


func _on_button(key: String) -> void:
	match key:
		"back":
			closed.emit()
		"hour":
			_set_hour({"noon": "dusk", "dusk": "night", "night": "noon"}[hour])
		"outline":
			_set_outline((outline + 1) % OUTLINES.size())
		"mirror":
			_set_reflect(not reflect)
		"crawl":
			crawl = (crawl + 1) % CRAWLS.size()
			_mark("crawl", CRAWLS[crawl])
		"measure":
			_measure()


func _exit_tree() -> void:
	# a screen closed during Measure leaves no frame rate set behind it
	Engine.max_fps = 0


func _process(delta: float) -> void:
	if _busy():
		_runs.step(delta)
		if _runs is P1Measure:
			return
	_flame_clock += delta
	if _flame_clock >= FLAME_TIME:
		_flame_clock = fmod(_flame_clock, FLAME_TIME)
		_step_flames()
	_frames += 1
	_clock += delta
	_gpu += graphics_time()
	if _clock >= 1.0:
		if not _hold:
			_readout.text = (
				"%d fps · graphics %.1f ms · art %d × %d · %.3f m a pixel"
				% [_frames / _clock, _gpu / _frames, _art.size.x - 2, _art.size.y - 2, mpp]
			)
		_frames = 0
		_gpu = 0.0
		_clock = 0.0
	_ease(delta)
	if drift and Engine.get_process_frames() <= 30:
		# two art pixels a frame along the view's right, arriving where the painter's camera stands
		var c: Dictionary = painter.camera
		var right := Vector3(cos(deg_to_rad(yaw)), 0.0, -sin(deg_to_rad(yaw)))
		var home := Vector3(c.target[0], c.target[1], c.target[2])
		target = home + right * mpp * 2.0 * (30 - Engine.get_process_frames())
		_apply_camera()
	if shot != "" and Engine.get_process_frames() == 30:
		_save_shot()


## The Measure button, and "measure" on the command line for the cloud: the same path.
func _measure() -> void:
	if _busy():
		return
	_hold = true
	var runs := _measure_runs()
	_runs = P1Measure.new(self) if runs.is_empty() else Runs.new(self, _measure_name(), runs)
	_runs.start()


## Measure's runs for a screen that has them, each [label, frames a second, seconds, setup]; none
## for P1, whose Measure is its own.
func _measure_runs() -> Array:
	return []


## The name Measure's line starts with.
func _measure_name() -> String:
	return "P1"


## The graphics chip's time for the last frame, ms (PLT-04): every pass that drew it, the picture
## and its data views, each of which Godot times on its own; the sky's map, drawn once, aside.
func graphics_time() -> float:
	var total := 0.0
	for vp: SubViewport in [_art, _gbuf, _mirror, _occ, _occ_floor]:
		if vp.render_target_update_mode == SubViewport.UPDATE_ALWAYS:
			total += RenderingServer.viewport_get_measured_render_time_gpu(vp.get_viewport_rid())
	return total


## Whether a Measure is running.
func _busy() -> bool:
	return _runs != null and _runs.get("index") >= 0


## The crawl fixes (PRE-22): with "ease", a turn or zoom eases gently to rest on the nearest whole
## step once the fingers lift; with "rest", the camera snaps to the pixel grid only when at rest.
func _ease(delta: float) -> void:
	_moving = maxf(_moving - delta, 0.0)
	if crawl != 2 or not _touches.is_empty():
		return
	var to_yaw := roundf(yaw / TURN_STEP) * TURN_STEP
	var to_mpp := _rest_mpp * pow(ZOOM_STEP, roundf(log(mpp / _rest_mpp) / log(ZOOM_STEP)))
	if absf(yaw - to_yaw) > 0.01 or absf(mpp - to_mpp) > 1e-5:
		var t := 1.0 - exp(-delta * 6.0)
		yaw = lerpf(yaw, to_yaw, t) if absf(yaw - to_yaw) > 0.05 else to_yaw
		mpp = lerpf(mpp, to_mpp, t) if absf(mpp - to_mpp) > 1e-4 else to_mpp
		_apply_camera()


func _save_shot() -> void:
	await RenderingServer.frame_post_draw
	var image := _art.get_texture().get_image()
	var inner := image.get_region(Rect2i(1, 1, image.get_width() - 2, image.get_height() - 2))
	inner.save_png(shot)
	get_tree().quit()


## Gestures: one finger pans, two turn and pinch. They arrive here, as the screen's own input: a
## control covering the screen takes every touch on it, so none reaches _unhandled_input.
func _gui_input(e: InputEvent) -> void:
	if e is InputEventMouse and e.device == InputEvent.DEVICE_ID_EMULATION:
		return  # the phone's mouse events made from the first finger, already read as touches
	if e is InputEventScreenTouch:
		var t := e as InputEventScreenTouch
		if t.pressed:
			_hold = _hold and _busy()
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
				_turn(rad_to_deg(angle_difference(d0.angle(), d1.angle())))
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
	mpp = clampf(mpp * factor, zoom_range.x, zoom_range.y)
	_moving = 0.3
	_apply_camera()
