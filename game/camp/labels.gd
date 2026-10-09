## Native bitmap labels remain readable while the world raster settles after a pinch.
extends Node2D

var drawing: Node2D
var state := {}
var density := 1.0
var font: Font = preload("res://ui/fonts/kindling-ui-16.fnt")


func _draw() -> void:
	if state.is_empty() or drawing.supplies.is_empty():
		return
	for site: Array in [
		["water_at", "Water", 1],
		["food_at", "Food", 1],
		["stone_at", "Stone", -1],
		["wood_at", "Wood", -1],
		["shelter_at", "Natural shelter", 0]
	]:
		_caption(drawing._site(site[0]), site[1], site[2])
	if drawing.supplies.has("rock_west"):
		_caption(
			drawing._local(
				float(drawing.supplies.rock_west + drawing.supplies.rock_east) / 200,
				float(drawing.supplies.rock_north) / 100
			),
			"Rock",
			0
		)
	for person: Dictionary in drawing.people:
		if int(person.id) == drawing.selected:
			_caption(drawing._absolute(Vector2i(person.east_cm, person.north_cm)), person.name, 0)


func _caption(at: Vector2, words: String, side: int) -> void:
	var size := 16 * ceili(12 * density / 16)
	var width := font.get_string_size(words, HORIZONTAL_ALIGNMENT_LEFT, -1, size).x
	var offset := Vector2(-width / 2, 20 * density)
	if side != 0:
		offset = Vector2(24 * density if side > 0 else -width - 24 * density, 8 * density)
	var point: Vector2 = at * float(state.scale) * float(state.live_scale) + Vector2(state.offset)
	var baseline := (point + offset).round()
	draw_string_outline(
		font, baseline, words, HORIZONTAL_ALIGNMENT_LEFT, -1, size, 4, Color("253930")
	)
	draw_string(font, baseline, words, HORIZONTAL_ALIGNMENT_LEFT, -1, size, Color("eee8ce"))
