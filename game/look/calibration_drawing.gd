## What a calibration scene draws for a variant (A18.1, α2.2a), besides the look's own sun and
## ground: solid rocks with an exact number of triangles, many small MultiMesh draws of copies,
## and a second view of the same world at half size for a mirror's pass. Each is laid over a grid
## of the screen, so all of it is in view and none is culled, and every variant draws the same
## places. Built for the Calibrate page; the cloud's run counts what each draws. Implements PLT-04.
class_name CalibrationDrawing
extends RefCounted

## The field's shader, the ground with the full material (A4.3), and the rocks' and copies', solid
## colour lit by the same light: Godot's stock material, which no family uses, stays dark on the
## cloud's driver once the sun's shadow has been drawn.
const FIELD_SHADER := preload("res://look/field.gdshader")
const SOLID_SHADER := preload("res://look/solid.gdshader")
## Rocks in every rocks variant, in the same places: only their triangles change.
const ROCKS := 500
## Copies in each draw of copies, each a small stone of 16 triangles.
const COPIES_EACH := 8
const STONE_AROUND := 4
const STONE_RINGS := 2
## The layer the sun alone is on: the mirror's camera does not see it, so the mirror's pass adds no
## shadow pass of its own (Godot draws a sun's shadow for every view that sees the sun).
const SUN_LAYER := 1 << 19


## A lumpy ball of 2 x around x rings triangles: `rings` rings of `around` points between its
## poles, fans at the poles and bands of two triangles a step between the rings; its faces turned
## out. Placed flattened, it looks like a rock.
static func rock_mesh(around: int, rings: int) -> ArrayMesh:
	var tool := SurfaceTool.new()
	tool.begin(Mesh.PRIMITIVE_TRIANGLES)
	# the poles, then each ring from the top
	tool.add_vertex(_rock_point(0.0, 0.0))
	tool.add_vertex(_rock_point(PI, 0.0))
	for ring in range(1, rings + 1):
		for k in around:
			tool.add_vertex(_rock_point(PI * ring / (rings + 1), TAU * k / around))
	for k in around:
		var next := (k + 1) % around
		_face(tool, 0, _on_ring(around, 1, next), _on_ring(around, 1, k))
		_face(tool, 1, _on_ring(around, rings, k), _on_ring(around, rings, next))
		for ring in range(1, rings):
			var a := _on_ring(around, ring, k)
			var b := _on_ring(around, ring, next)
			var c := _on_ring(around, ring + 1, k)
			var d := _on_ring(around, ring + 1, next)
			_face(tool, a, b, d)
			_face(tool, a, d, c)
	tool.generate_normals()
	return tool.commit()


## Solid rocks of `thousands` thousand triangles in all, laid over the ground the camera sees, under
## a node; the node holding them. screen: the screen's size as the camera's rays take it.
static func rocks(under: Node3D, camera: Camera3D, screen: Vector2, thousands: int) -> Node3D:
	# each rock 2 x thousands triangles, so around x rings = thousands, the rings near half the
	# points around
	var rings := maxi(1, floori(sqrt(thousands / 2.0)))
	while thousands % rings != 0:
		rings -= 1
	var mesh := rock_mesh(thousands / rings, rings)
	mesh.surface_set_material(0, _solid(Color("#8a8277")))
	var cells := _cells(camera, screen, 20, 25)
	var multimesh := MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D
	multimesh.mesh = mesh
	multimesh.instance_count = ROCKS
	for i in ROCKS:
		var cell: Dictionary = cells[i]
		# as wide and as deep as its cell, so the rocks cover the ground, and half as high as wide
		var wide: float = cell["size"] * 0.6
		var deep: float = cell["deep"] * 0.6
		multimesh.set_instance_transform(
			i, Transform3D(Basis().scaled(Vector3(wide, wide * 0.5, deep)), cell["at"])
		)
	var node := MultiMeshInstance3D.new()
	node.multimesh = multimesh
	under.add_child(node)
	return node


## `draws` MultiMesh draws, each of a few small stones, laid over the ground the camera sees, under
## a node; the node holding them.
static func copies(under: Node3D, camera: Camera3D, screen: Vector2, draws: int) -> Node3D:
	var mesh := rock_mesh(STONE_AROUND, STONE_RINGS)
	mesh.surface_set_material(0, _solid(Color("#9a8f80")))
	var aspect := screen.x / screen.y if screen.x > 0.0 and screen.y > 0.0 else 1.0
	var across := maxi(1, ceili(sqrt(draws * aspect)))
	var cells := _cells(camera, screen, across, ceili(float(draws) / across))
	var holder := Node3D.new()
	under.add_child(holder)
	for i in draws:
		var cell: Dictionary = cells[i]
		var multimesh := MultiMesh.new()
		multimesh.transform_format = MultiMesh.TRANSFORM_3D
		multimesh.mesh = mesh
		multimesh.instance_count = COPIES_EACH
		var size: float = cell["size"] * 0.08
		for k in COPIES_EACH:
			var angle := TAU * k / COPIES_EACH
			var at: Vector3 = (
				cell["at"] + Vector3(cos(angle), 0.0, sin(angle)) * float(cell["size"]) * 0.3
			)
			multimesh.set_instance_transform(
				k, Transform3D(Basis().scaled(Vector3(size, size * 0.6, size)), at)
			)
		var node := MultiMeshInstance3D.new()
		node.multimesh = multimesh
		holder.add_child(node)
	return holder


## The mirror's pass: a second view of the same world at half the screen's size in pixels, its
## camera where the main one is and blind to the sun's layer, so it adds one pass and no shadow pass
## of its own; a child of `parent`, drawn every frame and never shown.
static func mirror(parent: Node, world: World3D, camera: Camera3D, screen: Vector2) -> SubViewport:
	var view := SubViewport.new()
	view.size = Vector2i(screen / 2.0)
	view.world_3d = world
	view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	var eye := Camera3D.new()
	eye.transform = camera.transform
	eye.fov = camera.fov
	eye.keep_aspect = camera.keep_aspect
	eye.near = camera.near
	eye.far = camera.far
	eye.cull_mask = ~SUN_LAYER & 0xFFFFF
	eye.current = true
	view.add_child(eye)
	parent.add_child(view)
	return view


## The light's stand-ins for the field of C1 (A4.3 to A4.6), as the view will publish them
## (α2.3b): the view's maps, with patches of shade and contact; a light grid with four fires in
## every square, so every pixel is lit by all four, the most it ever is, or with none, so nothing
## flickers between two pictures; the sun's direction, the bounce and the haze.
static func light_stand_ins(sun: DirectionalLight3D, fires := true) -> void:
	var maps := Image.create(1024, 1024, false, Image.FORMAT_RGBA8)
	# open to the sky, no contact, the sun clear of anything near; then patches of shade and contact
	maps.fill(Color(0.9, 1.0, 0.1, 0.2))
	for i in 64:
		var at := Vector2i((i * 389) % 1000, (i * 677) % 1000)
		maps.fill_rect(Rect2i(at, Vector2i(24, 16)), Color(0.6, 0.7, 0.5, 0.05))
	_set_global("kd_view_maps", ImageTexture.create_from_image(maps))
	_set_global("kd_maps_place", Vector4(-32.0, -32.0, 64.0, 20.0))
	var grid := Image.create(64, 64, false, Image.FORMAT_RGBA8)
	grid.fill(Color8(1, 2, 3, 4) if fires else Color8(0, 0, 0, 0))
	_set_global("kd_fire_grid", ImageTexture.create_from_image(grid))
	var table := Image.create(4, 1, false, Image.FORMAT_RGBAF)
	for i in 4:
		var angle := TAU * i / 4.0
		table.set_pixel(i, 0, Color(3.0 * cos(angle), 0.3, 3.0 * sin(angle), 0.8))
	_set_global("kd_fire_table", ImageTexture.create_from_image(table))
	_set_global("kd_grid_place", Vector4(-128.0, -128.0, 4.0, 0.0))
	_set_global("kd_sun_toward", sun.global_transform.basis.z)
	_set_global("kd_bounce", Vector3(0.10, 0.09, 0.05))
	_set_global("kd_haze", Vector4(0.66, 0.73, 0.80, 0.004))
	_set_global("kd_haze_sun", Vector3(0.95, 0.80, 0.55))


## The colour table (A4.3) on or off in the final step: one that changes nothing, as costly as a
## moment's own.
static func grade(environment: Environment, on: bool) -> void:
	environment.adjustment_enabled = on
	if on and environment.adjustment_color_correction == null:
		var side := 17
		var slices: Array[Image] = []
		for b in side:
			var slice := Image.create(side, side, false, Image.FORMAT_RGB8)
			for g in side:
				for r in side:
					slice.set_pixel(r, g, Color(r, g, b) / (side - 1.0))
			slices.append(slice)
		var table := ImageTexture3D.new()
		table.create(Image.FORMAT_RGB8, side, side, side, false, slices)
		environment.adjustment_color_correction = table


## A grid of the screen, `across` by `down` cells, each cell's centre where the camera sees it on
## the ground: {"at", "size", "deep"}, the cell's width and depth on the ground there.
static func _cells(camera: Camera3D, screen: Vector2, across: int, down: int) -> Array[Dictionary]:
	var out: Array[Dictionary] = []
	for j in down:
		for i in across:
			var at := ground_at(camera, Vector2((i + 0.5) / across, (j + 0.5) / down) * screen)
			var beside := ground_at(camera, Vector2((i + 1.5) / across, (j + 0.5) / down) * screen)
			var below := ground_at(camera, Vector2((i + 0.5) / across, (j + 1.5) / down) * screen)
			out.append({"at": at, "size": at.distance_to(beside), "deep": at.distance_to(below)})
	return out


## Where a screen point's ray meets the ground's plane, or the point far ahead if it does not.
static func ground_at(camera: Camera3D, point: Vector2) -> Vector3:
	var from := camera.project_ray_origin(point)
	var along := camera.project_ray_normal(point)
	if along.y >= -1e-4:
		return from + along * camera.far * 0.5
	return from + along * (-from.y / along.y)


## The index of a ring's point, the rings counted from 1 at the top.
static func _on_ring(around: int, ring: int, k: int) -> int:
	return 2 + (ring - 1) * around + k


static func _rock_point(down: float, around: float) -> Vector3:
	# a few lumps, the same on every rock
	var lump := 1.0 + 0.12 * sin(3.0 * around + 1.7) * sin(2.0 * down) + 0.06 * cos(5.0 * around)
	return Vector3(sin(down) * cos(around), cos(down), sin(down) * sin(around)) * lump


static func _solid(colour: Color) -> ShaderMaterial:
	var material := ShaderMaterial.new()
	material.shader = SOLID_SHADER
	material.set_shader_parameter("kd_colour", colour)
	return material


static func _set_global(name: String, value: Variant) -> void:
	RenderingServer.global_shader_parameter_set(name, value)


## One triangle, its front outward: Godot's front faces wind clockwise seen from the front (A4.7).
static func _face(tool: SurfaceTool, a: int, b: int, c: int) -> void:
	tool.add_index(a)
	tool.add_index(c)
	tool.add_index(b)
