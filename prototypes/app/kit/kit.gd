## P3 The kit (IMPLEMENTATION α0.2c): the kit's shapes built by code at load from the catalogue's
## parameters (kit/catalogue.json), laid out as a model sheet: every shared shape in both its
## materials, two plants and an animal, a hut in birch bark and in reed, and the block figure in its
## five movements, stepping about 10 times a second. Three fires light it, each casting shadows and
## sending up smoke; one burns under a lean-to, whose roof the smoke flows out beneath. It draws
## with P1's answers: outline C and the "ease" crawl fix. Pre-production code (research 00): the
## app's README names the items it is about.
extends "res://look/look.gd"

const KitShapes := preload("res://kit/kit_shapes.gd")
const CATALOGUE := "res://kit/catalogue.json"
## The figures step through their poses about 10 times a second, as pixel art does (PRE-44).
const STEP_TIME := 0.1
## The sheet's plain, metres across, round the origin, reaching past the screen's top; and the
## square of it the sky's map covers.
const SIDE := 120.0
const SKY_SIDE := 64.0
## Room between neighbours in a row, metres.
const GAP := 0.9
## The sheet's rows from back to front: each its place along z and what stands in it, a shape of
## the catalogue in each of its materials, or the figure in one of its movements.
const SHEET := [
	[-21.0, ["broadleaf", "pine"]],
	[-14.5, ["tent", "hut"]],
	[-9.0, ["lean-to", "windbreak"]],
	[-4.5, ["rack", "boulder", "deer"]],
	[-1.0, ["pot", "basket"]],
	[1.5, ["hearth", "log", "stump"]],
	[5.5, ["walk", "carry", "knap", "scrape", "rest"]],
]
## Shapes turned a quarter, their open or long side to the camera's right.
const TURNED := ["lean-to", "deer"]
## Shapes drawn as copies of one shape, each copy carrying its own material, pattern and wear, as
## the game's models will be (A6.2): the hut of birch bark and of reed (PRE-42, PRE-43).
const COPIES := ["hut"]
## How far the figures turn from facing the camera, so their movements read: degrees.
const FIGURE_TURN := 40.0

var catalogue: Dictionary
## Where each thing stands, by name: its places, one for each copy on the sheet.
var placed := {}
## The per-copy data of the shapes drawn as copies, by name: for each copy its material row and
## pattern (over 255), wear and seed, as its MultiMesh carries them.
var copies := {}
## Each figure: its nodes (the shape and its data copies), its poses' meshes, and how many steps
## each pose is held.
var _figures := []
var _step_clock := 0.0
var _steps := 0


func _ready() -> void:
	outline = 3
	crawl = 2
	hour = "noon"
	zoom_range = Vector2(0.02, 0.12)
	catalogue = JSON.parse_string(FileAccess.get_file_as_string(CATALOGUE))
	super._ready()
	var scene: Node3D = _art.get_child(0)
	# the sheet is small: the sun's shadows, the haze and the outline depths need reach only across it
	_set_back(BACK, 70.0)
	_build_sheet(scene)
	# the hearth fire in the first hearth, behind the figures; one under the first lean-to's roof; and
	# one before the figures, so each casts two shadows
	var figures := Vector3.ZERO
	for move: Dictionary in catalogue.movements:
		figures += (placed[move.name][0] as Vector3) / float(catalogue.movements.size())
	var under := Basis(Vector3.UP, PI * 0.5) * Vector3(0.0, 0.0, -0.45)
	_add_fire(placed.hearth[0], 13.0, 1.0, scene)
	_add_fire(placed["lean-to"][0] + under, 10.0, 0.8, scene)
	_add_fire(figures + Vector3(0.0, 0.0, 1.8), 10.0, 0.8, scene)
	for i in _fires.size():
		_add_smoke(_fires[i][0], 0.7 if i == 0 else 0.55, scene)
	# "close" on the command line: the figures up close, two zoom steps in, for the cloud's pictures,
	# low in the frame so the camp behind them fills it
	if "close" in OS.get_cmdline_user_args():
		target = figures + Basis(Vector3.UP, deg_to_rad(yaw)) * Vector3(0.0, 0.0, -12.0)
		mpp = _rest_mpp / pow(ZOOM_STEP, 2.0)
		_apply_camera()
	_set_hour(hour)


## The painter's palette, light and look without its camp: the sheet stands on a plain of meadow
## of its own round the origin, with the camera on its middle, and no fire of the painter's.
func _load_scene() -> Node3D:
	var scene := super._load_scene()
	var ground := MeshInstance3D.new()
	ground.mesh = Shape.plain(SIDE, 0)
	for child in scene.get_children():
		if child.get_meta("kind", "") == "ground" and not ground.has_meta("kind"):
			for key in ["kind", "layers", "layerPat"]:
				ground.set_meta(key, child.get_meta(key))
		child.free()
	scene.add_child(ground)
	var p: Dictionary = (scene.get_meta("painter") as Dictionary).duplicate(true)
	# the atlas was drawn for the painter's own camera: its cards keep that size (look.gd's flames)
	p.atlasMpp = p.camera.mpp
	p.camera = {"target": [0.0, 0.0, -10.0], "yaw": 15.0, "elev": 30.0, "mpp": 0.06}
	p.extent = [-SKY_SIDE * 0.5, -SKY_SIDE * 0.5, SKY_SIDE * 0.5, SKY_SIDE * 0.5]
	p.fires = []
	p.soot = []
	scene.set_meta("painter", p)
	return scene


func _control_rows() -> Array:
	return [["back", "hour", "measure"]]


## Measure's runs (PLT-04): the sheet at night, with its three fires' light, shadows and smoke, at
## 120 frames a second, where a frame has only 8.3 ms, then at 60; then at noon.
func _measure_name() -> String:
	return "P3"


func _measure_runs() -> Array:
	return [
		["night@120", 120, 12.0, _set_hour.bind("night")],
		["night", 60, 12.0, _set_hour.bind("night")],
		["noon", 60, 12.0, _set_hour.bind("noon")],
	]


func _process(delta: float) -> void:
	_step_clock += delta
	if _step_clock >= STEP_TIME:
		_step_clock = fmod(_step_clock, STEP_TIME)
		_step()
	super._process(delta)


## The figures' next step: each shows the pose of its movement for this step (PRE-44).
func _step() -> void:
	_steps += 1
	for f: Dictionary in _figures:
		var poses: Array = f.poses
		var mesh: ArrayMesh = poses[floori(_steps / float(f.hold)) % poses.size()]
		for node: MeshInstance3D in f.nodes:
			if node.mesh != mesh:
				node.mesh = mesh


## The model sheet (PRE-46): row by row, back to front, each row's things side by side and centred.
func _build_sheet(scene: Node3D) -> void:
	var rows: Dictionary = painter.rows
	var shapes := {}
	for group in ["shapes", "plants", "animals"]:
		for e: Dictionary in catalogue[group]:
			shapes[e.name] = e
	var moves := {}
	for move: Dictionary in catalogue.movements:
		moves[move.name] = move
	for line: Array in SHEET:
		var things := []
		for name: String in line[1]:
			if moves.has(name):
				things.append(_figure_thing(rows, moves[name]))
				continue
			var e: Dictionary = shapes[name]
			var turn := PI * 0.5 if name in TURNED else 0.0
			var shared: ArrayMesh = KitShapes.build(e, 0, rows) if name in COPIES else null
			for which in (e.materials as Array).size():
				var mesh := shared if shared != null else KitShapes.build(e, which, rows)
				var thing := {"name": name, "meshes": [mesh], "turn": turn, "solid": true}
				if shared != null:
					thing.copy = _copy_data(e, which, rows)
				things.append(thing)
		_place_row(scene, things, float(line[0]))
	_add_tufts(scene, rows)


## The meadow's life, as the art book's meadows have it: tufts of grass round everything on the
## sheet, and across the meadow tufts in loose patches and flowers in drifts of white, yellow and
## purple, each kind one instanced shape.
func _add_tufts(scene: Node3D, rows: Dictionary) -> void:
	var rng := RandomNumberGenerator.new()
	rng.seed = 11
	var tufts: Array[Vector3] = []
	for places: Array in placed.values():
		for at: Vector3 in places:
			for k in 6:
				tufts.append(at + _near(rng, 0.6, 1.8))
	for k in 120:
		var centre := Vector3(rng.randf_range(-22.0, 22.0), 0.0, rng.randf_range(-40.0, 22.0))
		for j in rng.randi_range(2, 6):
			tufts.append(centre + _near(rng, 0.2, 1.5))
	_scatter(scene, KitShapes.tuft(rows), tufts, rng)
	var flowers := {"flowerw": [], "flowery": [], "flowerp": []}
	for k in 50:
		var centre := Vector3(rng.randf_range(-22.0, 22.0), 0.0, rng.randf_range(-40.0, 22.0))
		var kind: String = flowers.keys()[rng.randi() % 3]
		for j in rng.randi_range(2, 5):
			(flowers[kind] as Array).append(centre + _near(rng, 0.1, 1.2))
	for kind: String in flowers:
		var spots: Array[Vector3] = []
		spots.assign(flowers[kind])
		_scatter(scene, KitShapes.flower(rows, rows[kind]), spots, rng)


## A point on the ground between two distances from the origin, in any direction.
static func _near(rng: RandomNumberGenerator, lo: float, hi: float) -> Vector3:
	var a := rng.randf() * TAU
	var r := rng.randf_range(lo, hi)
	return Vector3(cos(a) * r, 0.0, sin(a) * r)


## One small shape at many spots, each turned and sized a little differently, as one MultiMesh.
func _scatter(
	scene: Node3D, mesh: ArrayMesh, spots: Array[Vector3], rng: RandomNumberGenerator
) -> void:
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.mesh = mesh
	mm.instance_count = spots.size()
	for i in spots.size():
		var size := rng.randf_range(0.8, 1.3)
		var basis := Basis(Vector3.UP, rng.randf() * TAU).scaled(Vector3.ONE * size)
		mm.set_instance_transform(i, Transform3D(basis, spots[i]))
	var node := MultiMeshInstance3D.new()
	node.multimesh = mm
	node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	_add_shape(node, _solid, scene)


## A copy's own material row and pattern, over 255, its wear and its seed, above 0.
func _copy_data(e: Dictionary, which: int, rows: Dictionary) -> Color:
	var row: float = rows[(e.materials as Array)[which]]
	var pat: int = Shape.PAT[(e.patterns as Array)[which]]
	return Color(row / 255.0, pat / 255.0, float(e.get("wear", 0.0)), 0.25 + 0.5 * which)


## The figure in a movement, in its own clothes: a mesh for each step of the movement, its key
## poses and the poses between them, one a step (PRE-44).
func _figure_thing(rows: Dictionary, move: Dictionary) -> Dictionary:
	var meshes := []
	for pose: Dictionary in KitShapes.steps(move):
		meshes.append(KitShapes.figure(rows, catalogue.figure, pose, move.get("materials", [])))
	return {
		"name": move.name,
		"meshes": meshes,
		"turn": deg_to_rad(yaw + FIGURE_TURN),
		"hold": 1,
		"solid": false,
	}


## A row of things along x at z, GAP apart, each where its own extent says, the row centred on
## the line the camera looks along, which runs aslant as the camera is turned.
func _place_row(scene: Node3D, things: Array, z: float) -> void:
	var spans := []
	var width := -GAP
	for t: Dictionary in things:
		var turn := Transform3D(Basis(Vector3.UP, t.turn), Vector3.ZERO)
		var box: AABB = turn * (t.meshes[0] as ArrayMesh).get_aabb()
		spans.append(box)
		width += box.size.x + GAP
	var x := target.x + (z - target.z) * tan(deg_to_rad(yaw)) - width * 0.5
	var copied := {}
	for i in things.size():
		var t: Dictionary = things[i]
		var box: AABB = spans[i]
		var at := Vector3(x - box.position.x, 0.0, z)
		x += box.size.x + GAP
		var list: Array = placed.get(t.name, [])
		list.append(at)
		placed[t.name] = list
		if t.has("copy"):
			var group: Array = copied.get(t.name, [])
			group.append([t, at])
			copied[t.name] = group
			continue
		var node := MeshInstance3D.new()
		node.mesh = t.meshes[0]
		node.transform = Transform3D(Basis(Vector3.UP, t.turn), at)
		var nodes := _add_shape(node, _solid, scene)
		if t.solid:
			# the smoke's path flows round what stands over a fire (smoke_path.gd)
			node.set_meta("kind", "solid")
		if t.has("hold"):
			_figures.append({"nodes": nodes, "poses": t.meshes, "hold": t.hold})
	for name: String in copied:
		_add_copies(scene, name, copied[name])


## A shape's copies as one MultiMesh, each copy carrying its own material, pattern and wear in its
## per-copy data (A6.2), as the game's huts will: one shape, no new art (PRE-42, PRE-43).
func _add_copies(scene: Node3D, name: String, group: Array) -> void:
	var mm := MultiMesh.new()
	mm.transform_format = MultiMesh.TRANSFORM_3D
	mm.use_custom_data = true
	mm.mesh = (group[0][0] as Dictionary).meshes[0]
	mm.instance_count = group.size()
	var data := []
	for i in group.size():
		var t: Dictionary = group[i][0]
		mm.set_instance_transform(i, Transform3D(Basis(Vector3.UP, t.turn), group[i][1]))
		mm.set_instance_custom_data(i, t.copy)
		data.append(t.copy)
	copies[name] = data
	var node := MultiMeshInstance3D.new()
	node.multimesh = mm
	_add_shape(node, _solid, scene)
