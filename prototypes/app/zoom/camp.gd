## P8's camp (A8.3, PRE-28, PRE-42): a band's camp on open land beside the river where the descent
## ends, built at load from P3's kit (catalogue.json, kit_shapes.gd) and lit by the look
## (solid.gdshader compiled with ZOOM, so each point stands on the planet as the ground there does):
## a hearth, tents, a hut, a lean-to, a windbreak, racks, logs, pots and baskets on a floor of
## trodden earth, which the ground draws, with a path down to the water. The band works round the
## hearth or walks between the camp and the river, each figure showing the next step of its
## movement about 10 times a second (PRE-44); from the camp's stop out they are drawn larger, as
## tiny figures (PRE-28). Deer graze across the river. Pre-production code (research 00).
extends RefCounted

const KIT := preload("res://kit/kit_shapes.gd")
const CATALOGUE := "res://kit/catalogue.json"
const SOLID := "res://look/shaders/solid.gdshader"
const FIRE := preload("res://zoom/shaders/fire.gdshader")
const STEP_TIME := 0.1
const PEOPLE := 24
const WALKERS := 10
## The trodden floor's radius round the hearth, metres, and the path's half-width.
const FLOOR := 11.0
const PATH := 0.7
## The camp's things, each with where it stands in the camp's own frame, metres (x across, z toward
## the river, the hearth at the middle), which way its front faces (toward the hearth when null,
## else degrees from facing the river), and its material, the kit's first or second.
const THINGS := [
	["hearth", Vector2(0.0, 0.0), 0.0, 0],
	["windbreak", Vector2(0.0, -2.7), null, 0],
	["lean-to", Vector2(5.6, -1.8), null, 1],
	["tent", Vector2(-6.2, -2.6), null, 0],
	["tent", Vector2(-8.8, 2.8), null, 1],
	["hut", Vector2(6.8, 4.4), null, 1],
	["rack", Vector2(2.6, -5.4), 20.0, 0],
	["rack", Vector2(-3.4, 5.6), -35.0, 1],
	["log", Vector2(1.7, 1.3), 30.0, 0],
	["log", Vector2(-1.9, 0.8), -50.0, 1],
	["pot", Vector2(-1.1, -1.5), 0.0, 0],
	["pot", Vector2(0.9, -1.8), 0.0, 1],
	["basket", Vector2(-5.1, 0.3), 0.0, 0],
	["basket", Vector2(4.3, 2.4), 0.0, 1],
	["stump", Vector2(2.2, -2.4), 0.0, 0],
	["boulder", Vector2(-10.5, -6.0), 40.0, 0],
	["boulder", Vector2(9.5, -7.5), -20.0, 0],
	["boulder", Vector2(11.5, -1.5), 70.0, 1],
]
## The movements the band uses: walkers walk or carry, the rest knap, scrape or rest.
const WALKS := ["walk", "carry"]
const WORKS := ["knap", "scrape", "rest"]
## Where the band works, in the camp's own frame, each facing the hearth: round it, clear of the
## windbreak, logs and pots; by the tents, the lean-to, the racks and the hut.
const SEATS := [
	Vector2(2.9, 0.1),
	Vector2(0.5, 2.7),
	Vector2(-1.0, 2.5),
	Vector2(-3.0, -0.3),
	Vector2(-2.7, 2.0),
	Vector2(2.8, 2.3),
	Vector2(3.5, -1.2),
	Vector2(-4.6, -2.0),
	Vector2(-6.3, 0.9),
	Vector2(4.3, 0.6),
	Vector2(1.4, -4.6),
	Vector2(-2.2, 4.4),
	Vector2(5.4, 2.6),
	Vector2(-7.4, -0.2),
]
## Where walkers go to and fro: the tents' doors, the racks, the hut, the lean-to and the hearth's
## edge; a third go down the path to the water instead.
const ROUNDS := [
	Vector2(-5.0, -1.6),
	Vector2(-7.4, 1.6),
	Vector2(2.4, -4.4),
	Vector2(-3.0, 4.6),
	Vector2(5.6, 3.2),
	Vector2(4.4, -0.8),
	Vector2(0.0, 3.6),
	Vector2(-1.5, -4.2),
]

## The camp's middle, its hearth, and where its path meets the river, in the world's metres; the
## way from the hearth toward the river.
var centre := Vector2.ZERO
var bank := Vector2.ZERO
var toward := Vector2(0.0, 1.0)
## Where the camp stands in the picture; the zoom moves it with its origin (place).
var root := Node3D.new()

var _material: ShaderMaterial
var _fire: ShaderMaterial
var _hearth_height := 0.0
## One MultiMesh for each step of each movement, holding the people in that pose this step.
var _poses: Array[MultiMesh] = []
var _first := {}
var _steps := {}
var _people := []
var _clock := 0.0
var _height: Callable


## Builds the camp at `at` beside the river, its path down to `to_bank`; `height` gives the ground's
## height at a place in the world's metres.
func build(rows: Dictionary, at: Vector2, to_bank: Vector2, height: Callable) -> void:
	centre = at
	bank = to_bank
	toward = (bank - centre).normalized() if bank.distance_to(centre) > 1.0 else Vector2(0.0, 1.0)
	_height = height
	var shader := Shader.new()
	shader.code = (load(SOLID) as Shader).code.replace(
		"shader_type spatial;", "shader_type spatial;\n#define ZOOM"
	)
	_material = ShaderMaterial.new()
	_material.shader = shader
	var catalogue: Dictionary = JSON.parse_string(FileAccess.get_file_as_string(CATALOGUE))
	var shapes := {}
	for group in ["shapes", "animals"]:
		for e: Dictionary in catalogue[group]:
			shapes[e.name] = e
	var meshes := {}
	for thing: Array in THINGS:
		var key := "%s:%d" % [thing[0], thing[3]]
		if not meshes.has(key):
			meshes[key] = KIT.build(shapes[thing[0]], thing[3], rows)
		var spot: Vector2 = thing[1]
		var front: Vector2 = (
			-spot if thing[2] == null else Vector2(0.0, 1.0).rotated(deg_to_rad(float(thing[2])))
		)
		_add_world(meshes[key], centre + _turned(spot), _turned(front))
	# the hearth's fire (fire.gdshader), casting no shadow: it is the light
	_hearth_height = float(height.call(centre))
	_fire = ShaderMaterial.new()
	_fire.shader = FIRE
	_fire.set_shader_parameter("fire_row", float(rows.fire))
	var flames := MeshInstance3D.new()
	flames.mesh = QuadMesh.new()
	flames.material_override = _fire
	flames.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	flames.custom_aabb = AABB(Vector3(-50.0, -50.0, -50.0), Vector3(100.0, 100.0, 100.0))
	flames.position = Vector3(0.0, _hearth_height + 0.05, 0.0)
	root.add_child(flames)
	# deer grazing on the far bank
	var deer: ArrayMesh = KIT.build(shapes.deer, 0, rows)
	var beyond := bank + toward * 22.0
	for k in 4:
		var off := Vector2(float(k) * 3.1 - 4.0, float(k % 2) * 2.4)
		_add_world(deer, beyond + _turned(off), _turned(Vector2(1.0, 0.0).rotated(0.7 * k)))
	_build_band(rows, catalogue)


## The camp in the picture, at `local`: where its middle stands from the zoom's moving origin.
func place(local: Vector3) -> void:
	root.position = local


## How high the hearth's fire burns, 0 to 1.
func set_fire(burn: float) -> void:
	_fire.set_shader_parameter("power", burn)


## The middle of the hearth's flames, from the camp's middle in the picture.
func flames() -> Vector3:
	return Vector3(0.0, _hearth_height + 0.35, 0.0)


## The band's next step, every STEP_TIME (PRE-44), each figure `size` times its true height.
func step(delta: float, size: float) -> void:
	_clock += delta
	if _clock < STEP_TIME:
		return
	_clock = fmod(_clock, STEP_TIME)
	var counts := PackedInt32Array()
	counts.resize(_poses.size())
	for i in _people.size():
		var p: Dictionary = _people[i]
		p.phase = int(p.phase) + 1
		var at: Vector2 = p.at
		var front := -at
		if p.walks:
			var to: Vector2 = p.to
			front = to - at
			if front.length() < 0.3:
				p.to = p.from
				p.from = to
			else:
				at += front.normalized() * minf(0.14, front.length())
				p.at = at
		var k: int = _first[p.move] + int(p.phase) % int(_steps[p.move])
		var xf := Transform3D(
			Basis(Vector3.UP, _yaw(_turned(front))).scaled(Vector3.ONE * size), _point(at, 0.0)
		)
		_poses[k].set_instance_transform(counts[k], xf)
		counts[k] += 1
	for k in _poses.size():
		_poses[k].visible_instance_count = counts[k]


## A place in the camp's own frame (x across, z toward the river) turned into the world's metres.
func _turned(spot: Vector2) -> Vector2:
	var across := Vector2(toward.y, -toward.x)
	return across * spot.x + toward * spot.y


## A point of the picture, relative to the camp's middle, for a place in the camp's own frame,
## standing on the ground, `lift` metres above it.
func _point(spot: Vector2, lift: float) -> Vector3:
	var w := centre + _turned(spot)
	var d := w - centre
	return Vector3(d.x, float(_height.call(w)) + lift, -d.y)


## The turn about the up axis that faces a kit shape's front (its +z) along a way in the world's
## metres, east and north, as the picture has them: x east, z south.
static func _yaw(way: Vector2) -> float:
	return atan2(way.x, -way.y)


## A thing standing at a place in the world's metres, its front facing along `front`.
func _add_world(mesh: ArrayMesh, w: Vector2, front: Vector2) -> void:
	var node := MeshInstance3D.new()
	node.mesh = mesh
	node.material_override = _material
	var d := w - centre
	node.transform = Transform3D(
		Basis(Vector3.UP, _yaw(front)), Vector3(d.x, float(_height.call(w)), -d.y)
	)
	root.add_child(node)


## The band (PRE-27, PRE-44): the kit's figure in each step of the movements, one MultiMesh a step
## with room for everyone; the workers sit or stand round the hearth, the walkers go to and fro
## between the camp and the river along the path, or across the floor.
func _build_band(rows: Dictionary, catalogue: Dictionary) -> void:
	for move: Dictionary in catalogue.movements:
		if not move.name in WALKS + WORKS:
			continue
		var steps := KIT.steps(move)
		_first[move.name] = _poses.size()
		_steps[move.name] = steps.size()
		for pose: Dictionary in steps:
			var mm := MultiMesh.new()
			mm.transform_format = MultiMesh.TRANSFORM_3D
			mm.mesh = KIT.figure(rows, catalogue.figure, pose, move.materials)
			mm.instance_count = PEOPLE
			mm.visible_instance_count = 0
			var node := MultiMeshInstance3D.new()
			node.multimesh = mm
			node.material_override = _material
			root.add_child(node)
			_poses.append(mm)
	var rng := RandomNumberGenerator.new()
	rng.seed = 7
	var path_end := _local(bank - toward * 1.0)
	for i in PEOPLE:
		var walks := i < WALKERS
		var at: Vector2
		var to: Vector2
		if walks:
			# some go down to the water and back, the rest between the camp's places
			at = ROUNDS[i % ROUNDS.size()]
			to = path_end if i % 3 == 0 else ROUNDS[(i * 3 + 2) % ROUNDS.size()]
		else:
			at = SEATS[(i - WALKERS) % SEATS.size()]
			to = at
		(
			_people
			. append(
				{
					"at": at,
					"from": at,
					"to": to,
					"walks": walks,
					"move": WALKS[i % 2] if walks else WORKS[i % 3],
					"phase": rng.randi() % 24,
				}
			)
		)


## A place in the world's metres in the camp's own frame.
func _local(w: Vector2) -> Vector2:
	var d := w - centre
	var across := Vector2(toward.y, -toward.x)
	return Vector2(d.dot(across), d.dot(toward))


## Where the camp stands, from the probe's picture (probe.gdshader) taken `spacing` metres a texel
## round the focus: the nearest place on open land, out of the forests, 24 to 50 m above the
## nearest river's banks, as metres east and north of the focus, and the nearest place in that
## river, where the camp's path meets the water; none if there is no such place.
static func place_of(probe: Image, spacing: float) -> Array:
	var n := probe.get_width()
	var best := Vector2.ZERO
	var best_gap := INF
	for j in n:
		for i in n:
			var c := probe.get_pixel(i, j)
			var gap := c.r * 255.0 - 20.0
			if gap < 24.0 or gap > 50.0 or c.g > 0.3 or c.b * 255.0 < 3.0:
				continue
			var off := Vector2((i + 0.5) / n - 0.5, 0.5 - (j + 0.5) / n) * float(n) * spacing
			if off.length() < best_gap:
				best_gap = off.length()
				best = off
	if best_gap == INF:
		return []
	var bank := best
	var nearest := INF
	for j in n:
		for i in n:
			if probe.get_pixel(i, j).r * 255.0 - 20.0 > 0.0:
				continue
			var off := Vector2((i + 0.5) / n - 0.5, 0.5 - (j + 0.5) / n) * float(n) * spacing
			if off.distance_to(best) < nearest:
				nearest = off.distance_to(best)
				bank = off
	return [best, bank]


## How many times their height the band is drawn (PRE-28): true size while a person spans a few
## pixels, then larger as the zoom leaves them, so they stay tiny figures at the camp's stop.
## `metres` is a shown pixel's width at the focus.
static func people_size(metres: float) -> float:
	return clampf(metres / 0.25, 1.0, 5.0)
