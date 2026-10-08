## Implements PRE-22 PRE-28: explicitly labelled inspection marks for owned wide-view records.
## These are study marks; final tiny figures and camp art retain their separate approval gate.
extends Node2D

var snapshot := {}
var selected := 0
var selection := {}


func show_snapshot(value: Dictionary, id: int) -> void:
	snapshot = value
	selected = id
	queue_redraw()


func _draw() -> void:
	for record: Dictionary in snapshot.get("records", []):
		var point: Vector2 = record.pixel
		var radius := 1.0 if int(record.count) == 1 else 2.0
		draw_rect(Rect2(point - Vector2.ONE * radius, Vector2.ONE * (radius * 2 + 1)), Palette.TEXT)
		if int(record.id) == selected or (record.members as PackedInt64Array).has(selected):
			draw_arc(point, radius + 5.0, 0, TAU, 16, Palette.WARN, 1.0)


func pick(pixel: Vector2, reach: float) -> Dictionary:
	var nearest := reach
	var chosen := {}
	for record: Dictionary in snapshot.get("records", []):
		var distance: float = pixel.distance_to(record.pixel)
		if distance < nearest:
			nearest = distance
			chosen = record
	if chosen.is_empty():
		selection = {}
		return {}
	selection = chosen.duplicate(true)
	selection["found"] = true
	selection["second"] = snapshot.second
	selection["form"] = snapshot.form
	selection["epoch"] = snapshot.epoch
	selection["revision"] = snapshot.revision
	return selection
