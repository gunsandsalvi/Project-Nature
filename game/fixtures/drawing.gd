## Implements PRE-01, PRE-03, PRE-22, PRE-27, PRE-31, PRE-42, PRE-43, PRE-44, PRE-46.
## T2.7a.2/4: one ordered submission list. Sheet crops and diagrams remain developer fixtures.
extends Node2D

const Atlas := preload("res://fixtures/atlas.gd")
const ID_SHADER := preload("res://fixtures/identity.gdshader")
const POSITIONS := {
	"tree": Vector2(-5, -7),
	"shelter": Vector2(3, 4),
	"boulder": Vector2(-3, -4),
	"person": Vector2(0, 0),
	"animal": Vector2(4, -5)
}
var camera: KdCanvas
var atlas := Atlas.new()
var entries: Array = []
var state: Dictionary = {}
var second := 43200.0
var dusk := false
var pass_name := "colour"
var selected: int = 0
var facing := 0
var facing_count := 4
var action := false
var channel := "colour"
var show_crowd := false
var id_table: Dictionary = {}
var _draws: Array[Dictionary] = []
var _sprites: Dictionary = {}
var _codes: Dictionary = {}


func _ready() -> void:
	if atlas.read():
		entries = atlas.entries
	else:
		push_error(atlas.problem)


func _draw() -> void:
	if state.is_empty() or entries.is_empty():
		return
	var density: float = float(state.density)
	var ground_texture: Texture2D = atlas.texture(entries[3], density, "walk")
	if channel in ["normal", "material"]:
		var supplied: Texture2D = atlas.texture(entries[3], density, "walk", channel)
		if supplied != null:
			ground_texture = supplied
	var colour := Color("9c8a70") if dusk and channel == "colour" else Color.WHITE
	if channel == "alpha" and pass_name == "colour":
		colour = Color.WHITE
	elif pass_name == "object":
		colour = identity(1)
	elif pass_name == "material":
		colour = identity(4)
	# Restricted ground patch: absolute shared endpoints, never a repeatedly rounded pitch.
	for north in range(-9, 10):
		for east in range(-5, 6):
			var points := PackedVector2Array()
			for corner: Vector2 in [Vector2(0, 0), Vector2(4, 0), Vector2(4, 4), Vector2(0, 4)]:
				points.append(camera.project(east * 4.0 + corner.x, north * 4.0 + corner.y, 0.0))
			if pass_name == "colour" and channel != "alpha":
				draw_polygon(
					points,
					PackedColorArray([colour]),
					PackedVector2Array(
						[Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1)]
					),
					ground_texture
				)
			else:
				draw_colored_polygon(points, colour)
	for item: Dictionary in _draws:
		if item.id == selected and pass_name == "colour":
			draw_arc(item.foot, maxf(5, density * 0.35), 0, TAU, 24, Color("fff5bb"), 1.0)


func rebuild() -> void:
	if state.is_empty() or entries.is_empty():
		return
	_draws.clear()
	id_table = {"1": {"id": "-4", "source": "ground", "surface": 0}}
	var density: float = float(state.density)
	for entry: Dictionary in entries:
		if entry.name != "ground":
			_append(entry, POSITIONS[entry.name], -int(entry.id), density)
	if show_crowd:
		for record: Dictionary in camera.records():
			# The C++ sampler projects the batch. No extension call per walker.
			_append(entries[4], record.place, int(record.id), density, record)
	_draws.sort_custom(
		func(a: Dictionary, b: Dictionary) -> bool:
			return a.foot.y < b.foot.y if a.foot.y != b.foot.y else a.id < b.id
	)
	_submit()


func _submit() -> void:
	var kept := {}
	for index in _draws.size():
		var item: Dictionary = _draws[index]
		kept[item.id] = true
		if not _sprites.has(item.id):
			var sprite := Sprite2D.new()
			sprite.centered = false
			sprite.region_enabled = true
			var semantic := ShaderMaterial.new()
			semantic.shader = ID_SHADER
			sprite.set_meta("semantic", semantic)
			add_child(sprite)
			_sprites[item.id] = sprite
		var node: Sprite2D = _sprites[item.id]
		node.texture = item.texture
		node.region_rect = item.source
		node.position = item.rect.position
		node.scale = Vector2.ONE * float(item.get("source_scale", 1.0))
		node.z_index = index + 1
		node.modulate = Color("ba99ad") if dusk and channel == "colour" else Color.WHITE
		node.material = null
		if pass_name != "colour" or channel == "alpha":
			var semantic: ShaderMaterial = node.get_meta("semantic")
			semantic.set_shader_parameter(
				"inspect_alpha", channel == "alpha" and pass_name == "colour"
			)
			semantic.set_shader_parameter(
				"identity_colour", identity(item.code if pass_name == "object" else item.entry.id)
			)
			node.material = semantic
			node.modulate = Color.WHITE
		var metadata: Dictionary = item.record.duplicate()
		if metadata.has("id"):
			metadata.id = str(metadata.id)
		id_table[str(item.code)] = {
			"id": str(item.id),
			"source": item.entry.name,
			"surface": item.record.get("surface", 0),
			"record": metadata
		}
	for id: int in _sprites.keys():
		if not kept.has(id):
			_sprites[id].queue_free()
			_sprites.erase(id)
	queue_redraw()


func _append(
	entry: Dictionary, place: Vector2, id: int, density: float, record: Dictionary = {}
) -> void:
	if atlas.has_method("sample"):
		_append_streamed(entry, place, id, density, record)
		return
	var animation := "work" if action and record.is_empty() else "walk"
	var colour_texture: Texture2D = atlas.texture(entry, density, animation)
	var texture: Texture2D = atlas.texture(entry, density, animation, channel)
	if texture == null:
		texture = colour_texture
	if texture == null:
		return
	var frame_size := Vector2(
		texture.get_width() / int(entry.frames), texture.get_height() / int(entry.facings)
	)
	var frame := int(floor(second * 6.0)) % int(entry.frames)
	var face := (
		(facing % facing_count) * (8 / facing_count)
		if int(entry.facings) == 8
		else facing % int(entry.facings)
	)
	var foot: Vector2
	if record.is_empty():
		foot = camera.project(place.x, place.y, float(entry.height_mm) / 1000.0)
	else:
		foot = record.pixel
		frame = int(float(record.phase) * float(entry.frames)) if int(record.activity) == 1 else 0
		face = int(record.facing) % int(entry.facings)
	var pivot := (Vector2(entry.pivot[0], entry.pivot[1]) * density / 64.0).round()
	var rect := Rect2(foot - pivot, frame_size)
	if not Rect2(Vector2.ZERO, state.size).intersects(rect):
		return
	if not _codes.has(id):
		_codes[id] = _codes.size() + 2
	_draws.append(
		{
			"entry": entry,
			"id": id,
			"code": _codes[id],
			"texture": texture,
			"colour_texture": colour_texture,
			"image": atlas.images[entry.actions.get(animation, entry.actions.walk)[str(density)]],
			"foot": foot,
			"rect": rect,
			"source": Rect2(Vector2(frame * frame_size.x, face * frame_size.y), frame_size),
			"record": record
		}
	)


func _append_streamed(
	entry: Dictionary, place: Vector2, id: int, density: float, record: Dictionary
) -> void:
	var sample: Dictionary = atlas.sample(entry, density)
	if sample.is_empty():
		return
	var texture: Texture2D = sample.textures.get(channel, sample.textures.colour)
	var ratio: float = density / sample.density
	var foot: Vector2 = record.get("pixel", camera.project(place.x, place.y, 0.0))
	var source := Rect2(Vector2.ZERO, texture.get_size())
	var rect := Rect2(foot - sample.pivot * ratio, source.size * ratio)
	if not Rect2(Vector2.ZERO, state.size).intersects(rect):
		return
	if not _codes.has(id):
		_codes[id] = _codes.size() + 2
	_draws.append(
		{
			"entry": entry,
			"id": id,
			"code": _codes[id],
			"texture": texture,
			"colour_texture": sample.textures.colour,
			"image": sample.image,
			"foot": foot,
			"rect": rect,
			"source": source,
			"source_scale": ratio,
			"source_density": sample.density,
			"record": record
		}
	)


func pick(pixel: Vector2) -> Dictionary:
	for index in range(_draws.size() - 1, -1, -1):
		var item: Dictionary = _draws[index]
		if not item.rect.has_point(pixel):
			continue
		var at: Vector2 = (
			(
				(pixel - item.rect.position) / float(item.get("source_scale", 1.0))
				+ item.source.position
			)
			. floor()
		)
		var image: Image = item.image
		if image.get_pixel(int(at.x), int(at.y)).a > 0.1:
			selected = item.id
			return item
	selected = 0
	return {}


func draw_list() -> Array[Dictionary]:
	return _draws


static func identity(code: int) -> Color:
	return Color8(code & 255, (code >> 8) & 255, (code >> 16) & 255)
