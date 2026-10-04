## P2 A full scene (IMPLEMENTATION α0.2b): P1's close camp, busy and at night. Thirty stand-in
## block figures walk and work in steps, two tents stand by the shelter, and three fires light
## them by our firelight term or by Godot's own lights, switchable. Round it, a forest of
## instanced trees seen at camp zoom. Measure runs it for about 90 seconds and reads the phone's
## own forecast of its heat. It draws with P1's answers: outline C and the "ease" crawl fix.
## Pre-production code (research 00): the app's README names the items it is about.
extends "res://look/look.gd"

const FIGURES := 30
const WALKERS := 18
## The figures move in steps, about 10 a second, as pixel art does (PRE-44).
const STEP_TIME := 0.1
const TREES := 12000
const CAMP_MPP := 1.0
const FOREST_SIDE := 1200.0
## The plain reaches past the forest, to the edges of the camp view as it turns.
const PLAIN_SIDE := 1800.0
## Measure's runs (PLT-04): [view, frame rate, seconds]. At 120 a frame has only 8.3 ms, so the
## chip can't hide its cost by slowing its clock; at 60 the camera turns over the camp, then the
## forest.
const P2_RUNS := [["close", 120, 20.0], ["close", 60, 35.0], ["camp", 60, 35.0]]
## Where the phone's heat is read: the forecast this many seconds ahead, every READ_EVERY seconds.
const FORECAST := 30
const READ_EVERY := 10.0
## The look's flags a shape's vertices carry (look.gdshaderinc).
const FLAG_EMISSIVE := 1.0
const FLAG_FOLIAGE := 2.0
const FLAG_CREATURE := 32.0

var fire_light := "ours"
var view := "close"
var _figures: MultiMesh
var _people := []
var _step_clock := 0.0
var _heights := {}
var _hearth_y := 0.0
var _fires := []
var _omni: Array[OmniLight3D] = []
var _forest: MultiMeshInstance3D
## The forest and its plain, with their copies for the data passes: shown at camp zoom only.
var _forest_nodes: Array[Node3D] = []
var _plain: MeshInstance3D
var _p2_run := -1
var _p2_crawl := 0
var _p2_clock := 0.0
var _p2_samples := PackedFloat32Array()
var _p2_late := 0
var _p2_results := PackedStringArray()
var _heat := []
var _read_clock := 0.0


func _ready() -> void:
	outline = 3
	crawl = 2
	hour = "night"
	zoom_range = Vector2(0.045, 1.2)
	super._ready()
	var scene: Node3D = _art.get_child(0)
	_read_heights(scene)
	var hearth: Dictionary = painter.fires[0]
	var at := Vector3(hearth.pos[0], hearth.pos[1], hearth.pos[2])
	_hearth_y = at.y
	_fires = [
		[at, 12.0, 1.0],
		[_on_ground(at + Vector3(-8.0, 0.0, 7.0)), 7.0, 0.8],
		[_on_ground(at + Vector3(9.0, 0.0, 8.0)), 7.0, 0.8],
	]
	_build_camp(scene)
	_build_forest(scene)
	for arg in OS.get_cmdline_user_args():
		if arg.begins_with("view="):
			view = arg.substr(5)
		elif arg.begins_with("fire="):
			fire_light = "Godot's" if arg.substr(5) == "godot" else "ours"
	_set_hour(hour)
	_set_fire_light(fire_light)
	_set_view(view)
	_mark("crawl", CRAWLS[crawl])


func _control_rows() -> Array:
	return [["back", "hour", "view"], ["fire", "crawl", "measure"]]


func _label(key: String) -> String:
	if key == "view":
		return "View: %s"
	if key == "fire":
		return "Fire light: %s"
	return super._label(key)


func _on_button(key: String) -> void:
	match key:
		"hour":
			_set_hour({"noon": "dusk", "dusk": "night", "night": "noon"}[hour])
		"view":
			_set_view("camp" if view == "close" else "close")
		"fire":
			_set_fire_light("Godot's" if fire_light == "ours" else "ours")
		_:
			super._on_button(key)


func _measure() -> void:
	if not _busy():
		_hold = true
		_start_p2()


func _busy() -> bool:
	return super._busy() or _p2_run >= 0


## The hour's light (PRE-30), with the three fires: burning low by day, high at dusk and night.
func _set_hour(name: String) -> void:
	super._set_hour(name)
	if _fires.is_empty():
		return
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
		powers[i] = float(_fires[i][2]) * (1.0 if hour != "noon" else 0.35)
	return powers


func _light_fires() -> void:
	var lit := Projection()
	for i in _fires.size():
		var f: Array = _fires[i]
		var at: Vector3 = f[0]
		lit = _with_column(lit, i, Vector4(at.x, at.y + 0.5, at.z, f[1]))
		if i < _omni.size():
			_omni[i].light_energy = 2.0 * float(f[2]) * (1.0 if hour != "noon" else 0.35)
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_fires", lit)
	set_global.call("look_fire_powers", fire_powers())


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


## The close camp at the painter's zoom, or camp zoom over the forest (PRE-28).
func _set_view(which: String) -> void:
	view = which
	var c: Dictionary = painter.camera
	mpp = c.mpp if which == "close" else CAMP_MPP
	_rest_mpp = mpp
	for node in _forest_nodes:
		node.visible = which == "camp"
	# at camp zoom the ground in view runs about 1.2 km deep: the camera stands back past its near
	# edge, and the shadows, haze and outline depths reach across it
	if which == "close":
		_set_back(BACK, 220.0)
	else:
		_set_back(1000.0, 900.0)
	_apply_camera()
	_mark("view", which)


func _process(delta: float) -> void:
	_step_figures(delta)
	if _p2_run >= 0:
		_p2_step(delta)
	super._process(delta)


# --- the camp ---------------------------------------------------------------------------------


## The ground's height in 2 m cells, from the scene's ground meshes, for standing things on it.
func _read_heights(scene: Node3D) -> void:
	for node in scene.get_children():
		if not node.has_meta("kind") or node.get_meta("kind") != "ground":
			continue
		var mesh: Mesh = (node as MeshInstance3D).mesh
		for s in mesh.get_surface_count():
			var points: PackedVector3Array = mesh.surface_get_arrays(s)[Mesh.ARRAY_VERTEX]
			for p in points:
				var key := Vector2i(floori(p.x / 2.0), floori(p.z / 2.0))
				_heights[key] = maxf(_heights.get(key, -1e9), p.y)


func _on_ground(p: Vector3) -> Vector3:
	return Vector3(p.x, _heights.get(Vector2i(floori(p.x / 2.0), floori(p.z / 2.0)), p.y), p.z)


## Whether a figure may stand there: on ground near the hearth's height, not up the cliff.
func _walkable(p: Vector3) -> bool:
	var key := Vector2i(floori(p.x / 2.0), floori(p.z / 2.0))
	return _heights.has(key) and absf(float(_heights[key]) - _hearth_y) < 1.2


func _build_camp(scene: Node3D) -> void:
	var rows: Dictionary = painter.rows
	var rng := RandomNumberGenerator.new()
	rng.seed = 2
	_figures = MultiMesh.new()
	_figures.transform_format = MultiMesh.TRANSFORM_3D
	_figures.mesh = _figure_mesh(rows)
	_figures.instance_count = FIGURES
	var node := MultiMeshInstance3D.new()
	node.multimesh = _figures
	_add_shape(node, _solid, scene)
	var hearth: Vector3 = _fires[0][0]
	for i in FIGURES:
		var at := _place(rng, hearth, 16.0)
		_people.append(
			{
				"at": at,
				"to": _place(rng, hearth, 16.0),
				"walks": i < WALKERS,
				"phase": rng.randf() * TAU
			}
		)
	_step_figures(STEP_TIME)
	for f in _fires.slice(1):
		var tent := MeshInstance3D.new()
		tent.mesh = _cone(1.8, 3.1, 10, rows.hide, 0.0)
		tent.position = _on_ground((f[0] as Vector3) + Vector3(0.0, 0.0, -3.0))
		_add_shape(tent, _solid, scene)
		var flame := MeshInstance3D.new()
		flame.mesh = _cone(0.35, 0.9, 6, rows.fire, FLAG_EMISSIVE, 5)
		flame.material_override = _solid
		flame.position = f[0]
		flame.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
		flame.layers = LAYER_MAIN
		scene.add_child(flame)
	for f in _fires:
		var light := OmniLight3D.new()
		light.position = (f[0] as Vector3) + Vector3(0.0, 0.8, 0.0)
		light.omni_range = f[1]
		light.light_color = Color(painter.moods.night.fire)
		light.layers = LAYER_MAIN
		scene.add_child(light)
		_omni.append(light)


func _place(rng: RandomNumberGenerator, around: Vector3, radius: float) -> Vector3:
	for attempt in 40:
		var p := (
			around
			+ Vector3(rng.randf_range(-radius, radius), 0.0, rng.randf_range(-radius, radius))
		)
		if _walkable(p) and p.distance_to(around) > 2.0:
			return _on_ground(p)
	return around + Vector3(2.5, 0.0, 0.0)


## Walkers stride between spots in steps of STEP_TIME; workers stay, bending at their work.
func _step_figures(delta: float) -> void:
	_step_clock += delta
	if _step_clock < STEP_TIME:
		return
	_step_clock = fmod(_step_clock, STEP_TIME)
	var rng := RandomNumberGenerator.new()
	var hearth: Vector3 = _fires[0][0]
	for i in _people.size():
		var p: Dictionary = _people[i]
		var at: Vector3 = p.at
		var facing := 0.0
		var bend := 0.0
		p.phase = float(p.phase) + 1.0
		if p.walks:
			var to: Vector3 = p.to
			var way := Vector3(to.x - at.x, 0.0, to.z - at.z)
			if way.length() < 0.3:
				rng.seed = i * 7919 + int(p.phase)
				p.to = _place(rng, hearth, 16.0)
			else:
				at = _on_ground(at + way.normalized() * 0.14)
				p.at = at
			facing = atan2(way.x, way.z)
		else:
			facing = atan2(hearth.x - at.x, hearth.z - at.z)
			bend = 0.25 if int(p.phase) % 6 < 3 else 0.0
		var xf := Transform3D(Basis(Vector3.UP, facing) * Basis(Vector3.RIGHT, bend), at)
		_figures.set_instance_transform(i, xf)


# --- the forest at camp zoom -------------------------------------------------------------------


## Thick woods round the camp, beyond the painter's scene, on a plain of woodland floor: one
## instanced tree, so the trees cost what the game's will (PRE-28).
func _build_forest(scene: Node3D) -> void:
	var rows: Dictionary = painter.rows
	var ext: Array = painter.extent
	var plain_y := _hearth_y - 3.0
	_plain = MeshInstance3D.new()
	_plain.mesh = _plain_mesh(PLAIN_SIDE)
	_plain.position = Vector3((ext[0] + ext[2]) * 0.5, plain_y, (ext[1] + ext[3]) * 0.5)
	var first := scene.get_child_count()
	_add_shape(_plain, _ground, scene)
	for i in range(first, scene.get_child_count()):
		_forest_nodes.append(scene.get_child(i))
	var trees := MultiMesh.new()
	trees.transform_format = MultiMesh.TRANSFORM_3D
	trees.mesh = _tree_mesh(rows)
	trees.instance_count = TREES
	var rng := RandomNumberGenerator.new()
	rng.seed = 5
	var centre := _plain.position
	var placed := 0
	while placed < TREES:
		var x := centre.x + rng.randf_range(-0.5, 0.5) * FOREST_SIDE
		var z := centre.z + rng.randf_range(-0.5, 0.5) * FOREST_SIDE
		if x > ext[0] - 4.0 and x < ext[2] + 4.0 and z > ext[1] - 4.0 and z < ext[3] + 4.0:
			continue
		var s := rng.randf_range(0.8, 1.3)
		var basis := Basis(Vector3.UP, rng.randf() * TAU).scaled(Vector3(s, s, s))
		trees.set_instance_transform(placed, Transform3D(basis, Vector3(x, plain_y, z)))
		placed += 1
	_forest = MultiMeshInstance3D.new()
	_forest.multimesh = trees
	var before := scene.get_child_count()
	_add_shape(_forest, _solid, scene)
	for i in range(before, scene.get_child_count()):
		_forest_nodes.append(scene.get_child(i))


# --- shapes ------------------------------------------------------------------------------------


## A block figure, about 1.65 m: legs and arms of skin, a hide tunic, a head with hair (PRE-27).
func _figure_mesh(rows: Dictionary) -> ArrayMesh:
	var m := Solid.new()
	m.box(Vector3(0.0, 0.4, 0.0), Vector3(0.34, 0.8, 0.2), rows.skin2, FLAG_CREATURE)
	m.box(Vector3(0.0, 1.05, 0.0), Vector3(0.44, 0.55, 0.26), rows.hide, FLAG_CREATURE)
	m.box(Vector3(0.0, 1.47, 0.0), Vector3(0.24, 0.26, 0.24), rows.skin2, FLAG_CREATURE)
	m.box(Vector3(0.0, 1.6, -0.02), Vector3(0.26, 0.06, 0.26), rows.hair, FLAG_CREATURE)
	return m.commit()


## A broadleaf stand-in as camp zoom needs it, a metre an art pixel: a crown of 8 faces on a
## trunk of two crossed faces, 12 triangles where a close tree takes 44, since the forest's
## geometry, drawn for the picture, the outlines and the shadows, is what cost the frame (PRE-28).
func _tree_mesh(rows: Dictionary) -> ArrayMesh:
	var m := Solid.new()
	for side: Vector3 in [Vector3(0.25, 0.0, 0.0), Vector3(0.0, 0.0, 0.25)]:
		var up := Vector3(0.0, 3.4, 0.0)
		var n := side.cross(Vector3.UP).normalized()
		m.quad(-side, side, side + up, -side + up, n, rows.bark, 0.0)
	m.ball(Vector3(0.0, 4.6, 0.0), 2.4, rows.leaf, FLAG_FOLIAGE, false)
	return m.commit()


func _cone(
	radius: float, height: float, sides: int, row: float, flags: float, bias := 0
) -> ArrayMesh:
	var m := Solid.new()
	m.bias = bias
	var top := Vector3(0.0, height, 0.0)
	for i in sides:
		var a0 := TAU * i / sides
		var a1 := TAU * (i + 1) / sides
		var p0 := Vector3(cos(a0) * radius, 0.0, sin(a0) * radius)
		var p1 := Vector3(cos(a1) * radius, 0.0, sin(a1) * radius)
		var n := (p0 + p1).normalized() * height + Vector3(0.0, radius, 0.0)
		m.tri(p0, p1, top, n.normalized(), row, flags)
	return m.commit()


## A flat square of woodland floor (the ground's fourth cover), side metres across.
func _plain_mesh(side: float) -> ArrayMesh:
	var h := side * 0.5
	var points := PackedVector3Array(
		[
			Vector3(-h, 0, -h),
			Vector3(h, 0, -h),
			Vector3(h, 0, h),
			Vector3(-h, 0, -h),
			Vector3(h, 0, h),
			Vector3(-h, 0, h)
		]
	)
	var normals := PackedVector3Array()
	var cover := PackedByteArray()
	for i in 6:
		normals.append(Vector3.UP)
		cover.append_array(PackedByteArray([0, 0, 0, 255]))
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = points
	arrays[Mesh.ARRAY_NORMAL] = normals
	arrays[Mesh.ARRAY_CUSTOM0] = cover
	var mesh := ArrayMesh.new()
	var format := Mesh.ARRAY_CUSTOM_RGBA8_UNORM << Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays, [], {}, format)
	return mesh


## A solid shape in the look's format: per vertex its material row, step bias, pattern and flags
## (custom0) and its place in its own shape (custom1), with Godot's clockwise front faces.
class Solid:
	var points := PackedVector3Array()
	var normals := PackedVector3Array()
	var attrs := PackedByteArray()
	## Steps up or down from the light's own, for the vertices to come: a flame burns bright.
	var bias := 0
	var locs := PackedFloat32Array()

	func tri(a: Vector3, b: Vector3, c: Vector3, n: Vector3, row: float, flags: float) -> void:
		# Godot's front faces wind clockwise seen from outside, so their right-hand normal points in
		if (b - a).cross(c - a).dot(n) > 0.0:
			var t := b
			b = c
			c = t
		for p in [a, b, c]:
			points.append(p)
			normals.append(n)
			attrs.append_array(PackedByteArray([int(row), 128 + bias, 0, int(flags)]))
			locs.append_array(PackedFloat32Array([p.x, p.y, p.z, 0.0]))

	func quad(
		a: Vector3, b: Vector3, c: Vector3, d: Vector3, n: Vector3, row: float, flags: float
	) -> void:
		tri(a, b, c, n, row, flags)
		tri(a, c, d, n, row, flags)

	func box(centre: Vector3, size: Vector3, row: float, flags: float) -> void:
		var h := size * 0.5
		for axis in 3:
			for side in [-1.0, 1.0]:
				var n := Vector3.ZERO
				n[axis] = side
				var u := Vector3.ZERO
				u[(axis + 1) % 3] = h[(axis + 1) % 3]
				var v := Vector3.ZERO
				v[(axis + 2) % 3] = h[(axis + 2) % 3]
				var c := centre + n * h[axis]
				quad(c - u - v, c + u - v, c + u + v, c - u + v, n, row, flags)

	## A round crown: an octahedron's faces, split once and pushed out to the sphere when fine.
	func ball(centre: Vector3, radius: float, row: float, flags: float, fine := true) -> void:
		var o := [
			Vector3.UP, Vector3.DOWN, Vector3.LEFT, Vector3.RIGHT, Vector3.FORWARD, Vector3.BACK
		]
		var faces := [
			[0, 2, 4], [0, 4, 3], [0, 3, 5], [0, 5, 2], [1, 4, 2], [1, 3, 4], [1, 5, 3], [1, 2, 5]
		]
		for f in faces:
			var a: Vector3 = o[f[0]]
			var b: Vector3 = o[f[1]]
			var c: Vector3 = o[f[2]]
			var ab := (a + b).normalized()
			var bc := (b + c).normalized()
			var ca := (c + a).normalized()
			var parts := [[a, b, c]]
			if fine:
				parts = [[a, ab, ca], [ab, b, bc], [ca, bc, c], [ab, bc, ca]]
			for t in parts:
				var p0: Vector3 = t[0]
				var p1: Vector3 = t[1]
				var p2: Vector3 = t[2]
				var n := (p0 + p1 + p2).normalized()
				tri(centre + p0 * radius, centre + p1 * radius, centre + p2 * radius, n, row, flags)

	func commit() -> ArrayMesh:
		var arrays := []
		arrays.resize(Mesh.ARRAY_MAX)
		arrays[Mesh.ARRAY_VERTEX] = points
		arrays[Mesh.ARRAY_NORMAL] = normals
		arrays[Mesh.ARRAY_CUSTOM0] = attrs
		arrays[Mesh.ARRAY_CUSTOM1] = locs
		var format := (
			Mesh.ARRAY_CUSTOM_RGBA8_UNORM << Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT
			| Mesh.ARRAY_CUSTOM_RGBA_FLOAT << Mesh.ARRAY_FORMAT_CUSTOM1_SHIFT
		)
		var mesh := ArrayMesh.new()
		mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays, [], {}, format)
		return mesh


# --- Measure: the frame and the heat --------------------------------------------------------------


func _start_p2() -> void:
	# free turns while Measure turns the view: "ease" would pull it back to a whole step each frame
	_p2_crawl = crawl
	crawl = 0
	_p2_results = PackedStringArray()
	_heat = []
	_read_clock = READ_EVERY
	_p2_begin(0)


func _p2_begin(i: int) -> void:
	_p2_run = i
	var run: Array = P2_RUNS[i]
	_set_view(run[0])
	Engine.max_fps = run[1]
	_p2_clock = 0.0
	_p2_samples = PackedFloat32Array()
	_p2_late = 0
	_readout.text = (
		"Measuring %d of %d: %s view, %d frames a second" % [i + 1, P2_RUNS.size(), run[0], run[1]]
	)


func _p2_step(delta: float) -> void:
	var run: Array = P2_RUNS[_p2_run]
	_p2_clock += delta
	yaw = wrapf(yaw + 12.0 * delta, -180.0, 180.0)
	_apply_camera()
	_read_clock += delta
	if _read_clock >= READ_EVERY:
		_read_clock = 0.0
		_heat.append(thermal())
	if _p2_clock >= 1.0:
		_p2_samples.append(
			RenderingServer.viewport_get_measured_render_time_gpu(_art.get_viewport_rid())
		)
		if delta > 1.15 / float(run[1]):
			_p2_late += 1
	if _p2_clock < float(run[2]):
		return
	var label: String = run[0] + ("@%d" % run[1] if run[1] != 60 else "")
	_p2_results.append(summary(label, _p2_samples, _p2_late))
	if _p2_run + 1 < P2_RUNS.size():
		_p2_begin(_p2_run + 1)
		return
	_p2_run = -1
	Engine.max_fps = 0
	crawl = _p2_crawl
	_rest_yaw = yaw
	var main: GDScript = load("res://main.gd")
	var line := code(
		(
			"P2 %s %s %d×%d %s Hz"
			% [
				ProjectSettings.get_setting("application/config/version", ""),
				main.facts().phone,
				_art.size.x - 2,
				_art.size.y - 2,
				main.refresh_rate()
			]
		),
		_p2_results,
		_heat
	)
	DisplayServer.clipboard_set(line)
	print(line)
	if "measure" in OS.get_cmdline_user_args():
		get_tree().quit()
	_readout.text = (
		"Copied for the chat (graphics ms, average/slowest 5%%, frames on time):\n%s" % line
	)


## One run's result: the graphics time's average and slowest 5%, and the share of frames on time.
static func summary(label: String, samples: PackedFloat32Array, late: int) -> String:
	var sorted := samples.duplicate()
	sorted.sort()
	var n := maxi(sorted.size(), 1)
	var total := 0.0
	for g in sorted:
		total += g
	var slow: float = sorted[mini(int(n * 0.95), n - 1)] if sorted.size() > 0 else 0.0
	return "%s %.1f/%.1f %d%%" % [label, total / n, slow, roundi(100.0 * (n - late) / n)]


## The line for the chat: the runs, then the heat as the forecast headroom at the first and last
## readings (1 is where the phone starts to slow itself) and the worst thermal status (0 none,
## 1 light, 2 moderate, 3 severe), or "?" where the phone gives none.
static func code(head: String, runs: PackedStringArray, heat: Array) -> String:
	var known := heat.filter(func(h: Vector2) -> bool: return h.x >= 0.0)
	var text := "heat ?"
	if not known.is_empty():
		var worst := 0
		for h: Vector2 in known:
			worst = maxi(worst, int(h.y))
		text = "heat %.2f→%.2f s%d" % [known[0].x, known[-1].x, worst]
	return "%s | %s | %s" % [head, " | ".join(runs), text]


## The phone's forecast of its heat FORECAST seconds ahead, and its thermal status, from
## Android's PowerManager through Godot's Android runtime; (-1, -1) where there is none.
static func thermal() -> Vector2:
	if OS.get_name() != "Android" or not Engine.has_singleton("AndroidRuntime"):
		return Vector2(-1.0, -1.0)
	var runtime: Object = Engine.get_singleton("AndroidRuntime")
	var context: Object = runtime.call("getApplicationContext")
	if context == null:
		return Vector2(-1.0, -1.0)
	var power: Object = context.call("getSystemService", "power")
	if power == null:
		return Vector2(-1.0, -1.0)
	var headroom: Variant = power.call("getThermalHeadroom", FORECAST)
	var status: Variant = power.call("getCurrentThermalStatus")
	var h: float = headroom if typeof(headroom) == TYPE_FLOAT and not is_nan(headroom) else -1.0
	return Vector2(h, status if typeof(status) == TYPE_INT else -1)
