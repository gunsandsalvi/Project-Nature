## Camp alpha: readable stand-ins for saved records, using the existing 37-degree projection.
extends Node2D

const SKIN := [Color("e0b48b"), Color("c4936d"), Color("a47353"), Color("825a43"), Color("654735")]
const HAIR := [Color("2e2926"), Color("624330"), Color("b5aaa0"), Color("47382d"), Color("966941")]
var camera: KdCanvas
var state := {}
var people: Array = []
var supplies := {}
var origin := Vector2i.ZERO
var selected := 0
var drawn: Dictionary = {}


func rebuild() -> void:
	drawn.clear()
	for person: Dictionary in people:
		var foot := _absolute(Vector2i(person.east_cm, person.north_cm))
		var density: float = state.get("density", 16.0)
		var height := maxf(12, roundf(density * 1.7))
		drawn[int(person.id)] = Rect2(
			foot - Vector2(height * 0.32, height), Vector2(height * 0.64, height)
		)
	queue_redraw()


func _draw() -> void:
	if state.is_empty() or supplies.is_empty():
		return
	draw_rect(Rect2(Vector2.ZERO, state.size), Color("253930"))
	var x := float(supplies.half_width_cm) / 100.0
	var y := float(supplies.half_height_cm) / 100.0
	var patch := patch_points()
	draw_colored_polygon(patch, Color("738166"))
	# Texture accents are presentation only; the supplied patch is uniformly traversable.
	for north in range(-int(y) + 1, int(y), 2):
		for east in range(-int(x) + 1, int(x), 2):
			var p := _local(east, north)
			draw_rect(Rect2(p, Vector2(2, 1)), Color("7f8b70"))
	var line := patch.duplicate()
	line.append(patch[0])
	draw_polyline(line, Color("a4ad88"), 1)
	_sites()
	var ordered := people.duplicate()
	ordered.sort_custom(func(a: Dictionary, b: Dictionary) -> bool: return a.north_cm > b.north_cm)
	for person: Dictionary in ordered:
		_person(person)


func _absolute(at: Vector2i) -> Vector2:
	return camera.project_world(at.x, at.y, 0)


func _local(east: float, north: float) -> Vector2:
	return _absolute(origin + Vector2i(roundi(east * 100), roundi(north * 100)))


func patch_points() -> PackedVector2Array:
	var x := float(supplies.half_width_cm) / 100
	var y := float(supplies.half_height_cm) / 100
	return PackedVector2Array([_local(-x, -y), _local(x, -y), _local(x, y), _local(-x, y)])


func _site(key: String) -> Vector2:
	var at: PackedInt64Array = supplies[key]
	return _absolute(Vector2i(at[0], at[1]))


func _sites() -> void:
	var density: float = state.density
	var p := _site("water_at")
	draw_rect(
		Rect2(p - Vector2(density * 1.5, density * 0.6), Vector2(density * 3, density * 1.2)),
		Color("557f89")
	)
	draw_line(p - Vector2(density, 0), p + Vector2(density, 0), Color("a4c3c4"), 2)
	p = _site("food_at")
	for i in 3:
		var plant := p + Vector2((i - 1) * density * 0.7, 0)
		draw_rect(
			Rect2(plant - Vector2(density * 0.3, density), Vector2(density * 0.6, density)),
			Color("405b38")
		)
		draw_rect(Rect2(plant - Vector2(2, density * 0.8), Vector2(3, 3)), Color("ce9b6b"))
	p = _site("stone_at")
	draw_rect(
		Rect2(p - Vector2(density * 0.7, density * 0.5), Vector2(density * 1.4, density * 0.5)),
		Color("aaa79a")
	)
	p = _site("wood_at")
	draw_line(
		p - Vector2(density, density * 0.4),
		p + Vector2(density, 0),
		Color("574536"),
		maxf(2, density * 0.25)
	)
	p = _site("shelter_at")
	var cave := PackedVector2Array(
		[
			p + Vector2(-density * 2, 0),
			p + Vector2(-density * 1.5, -density * 1.8),
			p + Vector2(density, -density * 2),
			p + Vector2(density * 2, 0)
		]
	)
	draw_colored_polygon(cave, Color("898a7b"))
	draw_rect(
		Rect2(p - Vector2(density * 0.6, density), Vector2(density * 1.2, density)), Color("2e3430")
	)


func _person(person: Dictionary) -> void:
	var id := int(person.id)
	var rect: Rect2 = drawn[id]
	var h := rect.size.y
	var foot := rect.position + Vector2(rect.size.x / 2, h)
	var look := int(person.appearance)
	var skin: Color = SKIN[look % 5]
	var hair: Color = HAIR[look / 5]
	draw_rect(Rect2(foot - Vector2(h * 0.35, 1), Vector2(h * 0.7, 2)), Color("4b5c46"))
	if id == selected:
		draw_arc(foot, h * 0.5, 0, TAU, 32, Color("fff0b4"), 2)
	var unit := maxf(1, floorf(h / 12))
	var top := (foot - Vector2(unit * 2, unit * 12)).round()
	draw_rect(Rect2(top + Vector2(-unit, -unit), Vector2(unit * 6, unit * 13)), Color("34332c"))
	draw_rect(Rect2(top, Vector2(unit * 4, unit * 4)), skin)
	draw_rect(Rect2(top, Vector2(unit * 4, unit)), hair)
	draw_rect(Rect2(top + Vector2(0, unit), Vector2(unit, unit * 3)), hair)
	draw_rect(Rect2(top + Vector2(-unit, unit * 4), Vector2(unit * 6, unit * 4)), skin)
	draw_rect(Rect2(top + Vector2(0, unit * 8), Vector2(unit * 1.5, unit * 4)), skin)
	draw_rect(Rect2(top + Vector2(unit * 2.5, unit * 8), Vector2(unit * 1.5, unit * 4)), skin)
	draw_rect(Rect2(top + Vector2(unit * 2.5, unit * 2), Vector2(unit, unit)), Color("342b27"))


func pick(at: Vector2, radius: float = 0) -> int:
	var nearest := 0
	var distance := INF
	for id: int in drawn:
		var rect: Rect2 = drawn[id]
		if rect.grow(radius).has_point(at):
			var d := at.distance_squared_to(rect.get_center())
			if d < distance:
				nearest = id
				distance = d
	return nearest
