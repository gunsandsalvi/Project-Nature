## Calibration scene C5's fires (A18.1, α2.2b): 1, 3 or 5 fires at the closest zoom at night,
## each with people, logs and a ring of hearth stones round it, its light listed in the light grid
## and its things stood in the height maps of tops and undersides that fire shadows walk through
## (A4.5). The shadows are drawn none, by a walk at every pixel, by a walk at half resolution read
## by screen position, or from a small map for each fire remade 20 times a second, each way by the
## field's own shader and the pass it needs. Built for the Calibrate and Compare pages. Implements
## PRE-30 and PLT-04.
class_name CalibrationFires
extends RefCounted

## The field's shader for each way of drawing fire shadows.
const FIELDS := {
	"none": preload("res://look/field.gdshader"),
	"walk": preload("res://look/field-walk.gdshader"),
	"walk-half": preload("res://look/field-screen.gdshader"),
	"map": preload("res://look/field-map.gdshader"),
}
const THINGS := preload("res://look/things.gdshader")
const SCREEN := preload("res://look/fire-screen.gdshader")
const MAPS := preload("res://look/fire-maps.gdshader")
## The fires' places in metres east and north of the focus, the camp's hearth first; a variant
## lights as many as it has, and its heat.
const PLACES: Array[Vector2] = [
	Vector2(0.4, 0.8),
	Vector2(-1.8, 4.6),
	Vector2(2.1, -3.4),
	Vector2(-1.6, -7.8),
	Vector2(1.9, 8.9)
]
const HEAT := 0.85
## How high above the ground a fire's light comes from, in metres: its flame's middle.
const FLAME := 0.35
## The light grid: squares of 4 m from 128 m south-west of the focus, up to four fires each, a fire
## listed in every square its light reaches, a tenth by about 4 m (A4.3).
const GRID_PLACE := Vector4(-128.0, -128.0, 4.0, 0.0)
const GRID_SQUARES := 64
const REACH := 5.0
## The height maps: 512 texels over 32 m round the focus, about 6 cm a texel.
const HEIGHTS := 512
const HEIGHTS_PLACE := Vector4(-16.0, -16.0, 32.0, 0.0)
## The fires' maps: 128 texels each, eight side by side (fire.gdshaderinc), remade this often a
## second as people move (A4.5).
const MAP_TEXELS := 128
const MAP_SIDE_BY_SIDE := 8
const MAP_RATE := 20.0
## The layer of the half-resolution picture's ground, which only its own camera sees.
const SCREEN_LAYER := 1 << 18

## What stands round each fire, as offsets from it in metres east and north: people standing and
## sitting, logs lying, and the hearth's ring of stones.
const STANDING := [Vector2(1.3, 0.5), Vector2(-0.9, 1.25)]
const SITTING := [Vector2(-0.4, -1.45)]
const LOGS := [
	[Vector2(0.75, -0.85), Vector2(1.55, -0.25)], [Vector2(-1.3, -0.2), Vector2(-1.25, 0.85)]
]
const STONES := 9
const RING := 0.45

## The flames' material, made once.
static var _flame: ShaderMaterial


## Fires lit round the focus with what stands round them, their light in the light grid and their
## things in the height maps, their shadows drawn the given way: a node holding it all, its passes
## included. For "walk-half" its metadata "follow" holds the half-resolution picture's camera, which
## the page places where its own camera is each frame; its "draws" metadata, the instances it adds
## to the main picture. pixels: the window's size in its own pixels.
static func fires(under: Node3D, count: int, way: String, pixels: Vector2) -> Node3D:
	var holder := Node3D.new()
	under.add_child(holder)
	var heights := Image.create(HEIGHTS, HEIGHTS, false, Image.FORMAT_RGF)
	var lit := PLACES.slice(0, count)
	var draws := 0
	for at: Vector2 in lit:
		draws += _things(holder, heights, at)
	holder.set_meta("draws", draws)
	_set_global("kd_heights", ImageTexture.create_from_image(heights))
	_set_global("kd_heights_place", HEIGHTS_PLACE)
	_light_grid(lit)
	if way == "map":
		_maps(holder)
	elif way == "walk-half":
		holder.set_meta("follow", _screen(holder, under.get_world_3d(), pixels))
	return holder


## Night for the fires: the sky's fill dimmed to the moonless dark, or back to the day's.
static func night(environment: Environment, on: bool) -> void:
	environment.background_color = Color("#0b1020") if on else Color("#a9c4d8")
	environment.ambient_light_color = Color("#2a3550") if on else Color("#a9b8c8")
	environment.ambient_light_energy = 0.15 if on else 0.45


## A fire's flame and what stands round it: people, logs and its hearth's ring of stones, each drawn
## and stood in the height maps; the instances drawn.
static func _things(holder: Node3D, heights: Image, fire: Vector2) -> int:
	var flame := MeshInstance3D.new()
	var cone := CylinderMesh.new()
	cone.top_radius = 0.0
	cone.bottom_radius = 0.16
	cone.height = 0.45
	flame.mesh = cone
	flame.material_override = _flame_material()
	flame.position = _place(fire, 0.22)
	holder.add_child(flame)
	var drawn := 1
	for offset: Vector2 in STANDING:
		drawn += _person(holder, heights, fire + offset, 1.6)
	for offset: Vector2 in SITTING:
		drawn += _person(holder, heights, fire + offset, 0.9)
	for ends: Array in LOGS:
		var a: Vector2 = fire + ends[0]
		var b: Vector2 = fire + ends[1]
		var log := MeshInstance3D.new()
		var cylinder := CylinderMesh.new()
		cylinder.top_radius = 0.09
		cylinder.bottom_radius = 0.09
		cylinder.height = a.distance_to(b)
		log.mesh = cylinder
		log.material_override = _thing_material(Color("#5a4632"))
		var along := Vector3(b.x - a.x, 0.0, -(b.y - a.y)).normalized()
		var side := along.cross(Vector3.UP).normalized()
		# the cylinder's length, its y, along the log
		log.transform = Transform3D(
			Basis(side, along, side.cross(along)), _place((a + b) / 2.0, 0.09)
		)
		holder.add_child(log)
		_stand_line(heights, a, b, 0.09, 0.18)
		drawn += 1
	for k in STONES:
		var at := fire + Vector2.from_angle(TAU * k / STONES) * RING
		var stone := MeshInstance3D.new()
		var ball := SphereMesh.new()
		ball.radius = 0.08
		ball.height = 0.14
		stone.mesh = ball
		stone.material_override = _thing_material(Color("#7d776c"))
		stone.position = _place(at, 0.07)
		holder.add_child(stone)
		_stand_line(heights, at, at, 0.08, 0.14)
		drawn += 1
	return drawn


## A person as a capsule of their height, standing or sitting; the instances drawn.
static func _person(holder: Node3D, heights: Image, at: Vector2, tall: float) -> int:
	var person := MeshInstance3D.new()
	var capsule := CapsuleMesh.new()
	capsule.radius = 0.2
	capsule.height = tall
	person.mesh = capsule
	person.material_override = _thing_material(Color("#8a6a4a"))
	person.position = _place(at, tall / 2.0)
	holder.add_child(person)
	_stand_line(heights, at, at, 0.2, tall)
	return 1


## Something standing on the ground, a line from a to b in metres east and north with its radius,
## stood in the height maps: its top where it is, and its underside the ground.
static func _stand_line(heights: Image, a: Vector2, b: Vector2, radius: float, top: float) -> void:
	var texel := HEIGHTS_PLACE.z / HEIGHTS
	var corner := Vector2(HEIGHTS_PLACE.x, HEIGHTS_PLACE.y)
	var low := (a.min(b) - Vector2.ONE * radius - corner) / texel
	var high := (a.max(b) + Vector2.ONE * radius - corner) / texel
	for j in range(maxi(0, floori(low.y)), mini(HEIGHTS, ceili(high.y) + 1)):
		for i in range(maxi(0, floori(low.x)), mini(HEIGHTS, ceili(high.x) + 1)):
			var p := corner + (Vector2(i, j) + Vector2(0.5, 0.5)) * texel
			var near := Geometry2D.get_closest_point_to_segment(p, a, b)
			if p.distance_to(near) <= radius and heights.get_pixel(i, j).r < top:
				heights.set_pixel(i, j, Color(top, 0.0, 0.0))


## The light grid and the fire table for the fires lit (A4.5).
static func _light_grid(lit: Array) -> void:
	var table := Image.create(maxi(1, lit.size()), 1, false, Image.FORMAT_RGBAF)
	var listed := {}
	for row in lit.size():
		var fire: Vector2 = lit[row]
		table.set_pixel(row, 0, Color(fire.x, FLAME, fire.y, HEAT))
		var from := (
			((fire - Vector2.ONE * REACH) - Vector2(GRID_PLACE.x, GRID_PLACE.y)) / GRID_PLACE.z
		)
		var to := (
			((fire + Vector2.ONE * REACH) - Vector2(GRID_PLACE.x, GRID_PLACE.y)) / GRID_PLACE.z
		)
		for j in range(maxi(0, floori(from.y)), mini(GRID_SQUARES, floori(to.y) + 1)):
			for i in range(maxi(0, floori(from.x)), mini(GRID_SQUARES, floori(to.x) + 1)):
				var square := Rect2(
					Vector2(GRID_PLACE.x, GRID_PLACE.y) + Vector2(i, j) * GRID_PLACE.z,
					Vector2.ONE * GRID_PLACE.z
				)
				var nearest := fire.clamp(square.position, square.end)
				var key := Vector2i(i, j)
				if nearest.distance_to(fire) <= REACH and listed.get(key, []).size() < 4:
					var rows: Array = listed.get(key, [])
					rows.append(row)
					listed[key] = rows
	var grid := Image.create(GRID_SQUARES, GRID_SQUARES, false, Image.FORMAT_RGBA8)
	for key: Vector2i in listed:
		var rows: Array = listed[key]
		var bytes := [0, 0, 0, 0]
		for k in rows.size():
			bytes[k] = int(rows[k]) + 1
		grid.set_pixelv(key, Color8(bytes[0], bytes[1], bytes[2], bytes[3]))
	_set_global("kd_fire_grid", ImageTexture.create_from_image(grid))
	_set_global("kd_fire_table", ImageTexture.create_from_image(table))
	_set_global("kd_grid_place", GRID_PLACE)


## The fires' maps: a picture of their own, side by side, remade MAP_RATE times a second.
static func _maps(holder: Node3D) -> void:
	var view := SubViewport.new()
	view.size = Vector2i(MAP_TEXELS * MAP_SIDE_BY_SIDE, MAP_TEXELS)
	view.disable_3d = true
	view.render_target_update_mode = SubViewport.UPDATE_ONCE
	# timed for its share of the frames (Timing)
	view.set_meta("drawn_a_second", MAP_RATE)
	# the map's own size, not its anchors, which follow the window
	var walk := ColorRect.new()
	walk.size = Vector2(view.size)
	var material := ShaderMaterial.new()
	material.shader = MAPS
	walk.material = material
	view.add_child(walk)
	holder.add_child(view)
	var remake := Timer.new()
	remake.wait_time = 1.0 / MAP_RATE
	remake.autostart = true
	remake.timeout.connect(func() -> void: view.render_target_update_mode = SubViewport.UPDATE_ONCE)
	holder.add_child(remake)
	_set_global("kd_fire_maps", view.get_texture())


## The walk at half resolution: a picture of its own at half the window's size, of a flat ground
## on a layer only its camera sees, drawn with plain light; its camera.
static func _screen(holder: Node3D, world: World3D, pixels: Vector2) -> Camera3D:
	var view := SubViewport.new()
	view.size = Vector2i((pixels / 2.0).max(Vector2.ONE))
	view.world_3d = world
	view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	var eye := Camera3D.new()
	eye.cull_mask = SCREEN_LAYER
	var plain := Environment.new()
	plain.background_mode = Environment.BG_COLOR
	plain.background_color = Color.WHITE
	plain.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	eye.environment = plain
	eye.current = true
	view.add_child(eye)
	holder.add_child(view)
	var ground := MeshInstance3D.new()
	var plane := PlaneMesh.new()
	plane.size = Vector2(HEIGHTS_PLACE.z, HEIGHTS_PLACE.z)
	ground.mesh = plane
	var material := ShaderMaterial.new()
	material.shader = SCREEN
	ground.material_override = material
	ground.layers = SCREEN_LAYER
	ground.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	holder.add_child(ground)
	_set_global("kd_fire_screen", view.get_texture())
	return eye


static func _place(at: Vector2, up: float) -> Vector3:
	return Vector3(at.x, up, -at.y)


static func _thing_material(colour: Color) -> ShaderMaterial:
	var material := ShaderMaterial.new()
	material.shader = THINGS
	material.set_shader_parameter("kd_colour", colour)
	return material


static func _flame_material() -> ShaderMaterial:
	if _flame == null:
		var shader := Shader.new()
		shader.code = (
			"shader_type spatial; render_mode unshaded, shadows_disabled;"
			+ " void fragment() { ALBEDO = vec3(1.0, 0.62, 0.22); }"
		)
		_flame = ShaderMaterial.new()
		_flame.shader = shader
	return _flame


static func _set_global(name: String, value: Variant) -> void:
	RenderingServer.global_shader_parameter_set(name, value)
