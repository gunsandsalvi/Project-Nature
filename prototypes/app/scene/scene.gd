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

var view := "close"
var _figures: MultiMesh
var _people := []
var _step_clock := 0.0
var _heights := {}
var _hearth_y := 0.0
var _forest: MultiMeshInstance3D
## The forest and its plain, with their copies for the data passes: shown at camp zoom only.
var _forest_nodes: Array[Node3D] = []
var _plain: MeshInstance3D


func _ready() -> void:
	outline = 3
	crawl = 3
	hour = "night"
	zoom_range = Vector2(0.045, 1.2)
	super._ready()
	var scene: Node3D = _art.get_child(0)
	_read_heights(scene)
	var hearth: Dictionary = painter.fires[0]
	var at := Vector3(hearth.pos[0], hearth.pos[1], hearth.pos[2])
	_hearth_y = at.y
	# the painter's hearth, with its own flames, and a fire before each tent
	_add_fire(at, 16.0, 1.0, scene, false)
	_add_fire(_on_ground(at + Vector3(-8.0, 0.0, 7.0)), 10.0, 0.8, scene)
	_add_fire(_on_ground(at + Vector3(9.0, 0.0, 8.0)), 10.0, 0.8, scene)
	_build_camp(scene)
	_build_forest(scene)
	var centre := Vector3.ZERO
	for f in _fires:
		centre += (f[0] as Vector3) / float(_fires.size())
	_set_occ(centre)
	for f in _fires.slice(1):
		_add_smoke(f[0], 0.55, scene)
	_set_smoke_tone()
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
		"view":
			_set_view("camp" if view == "close" else "close")
		"fire":
			_set_fire_light("Godot's" if fire_light == "ours" else "ours")
		_:
			super._on_button(key)


## Measure's runs (PLT-04): the close camp at 120 frames a second, where a frame has only 8.3 ms,
## so the chip can't hide its cost by slowing its clock; then at 60 the camera turns over the camp,
## then the forest.
func _measure_name() -> String:
	return "P2"


func _measure_runs() -> Array:
	return [
		["close@120", 120, 20.0, _set_view.bind("close")],
		["close", 60, 35.0, _set_view.bind("close")],
		["camp", 60, 35.0, _set_view.bind("camp")],
	]


## The hour's light (PRE-30), with the three fires: burning low by day, high at dusk and night.
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
		var m := Shape.new()
		m.cone(1.8, 3.1, 10, rows.hide, 0.0)
		tent.mesh = m.commit()
		tent.position = _on_ground((f[0] as Vector3) + Vector3(0.0, 0.0, -3.0))
		_add_shape(tent, _solid, scene)


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
	_plain.mesh = Shape.plain(PLAIN_SIDE, 3)
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
	var m := Shape.new()
	m.box(Vector3(0.0, 0.4, 0.0), Vector3(0.34, 0.8, 0.2), rows.skin2, Shape.CREATURE)
	m.box(Vector3(0.0, 1.05, 0.0), Vector3(0.44, 0.55, 0.26), rows.hide, Shape.CREATURE)
	m.box(Vector3(0.0, 1.47, 0.0), Vector3(0.24, 0.26, 0.24), rows.skin2, Shape.CREATURE)
	m.box(Vector3(0.0, 1.6, -0.02), Vector3(0.26, 0.06, 0.26), rows.hair, Shape.CREATURE)
	return m.commit()


## A broadleaf stand-in as camp zoom needs it, a metre an art pixel: a crown of 8 faces on a
## trunk of two crossed faces, 12 triangles where a close tree takes 44, since the forest's
## geometry, drawn for the picture, the outlines and the shadows, is what cost the frame (PRE-28).
func _tree_mesh(rows: Dictionary) -> ArrayMesh:
	var m := Shape.new()
	for side: Vector3 in [Vector3(0.25, 0.0, 0.0), Vector3(0.0, 0.0, 0.25)]:
		var up := Vector3(0.0, 3.4, 0.0)
		var n := side.cross(Vector3.UP).normalized()
		m.quad(-side, side, side + up, -side + up, n, rows.bark, 0.0)
	m.ball(Vector3(0.0, 4.6, 0.0), 2.4, rows.leaf, Shape.FOLIAGE, false)
	return m.commit()
