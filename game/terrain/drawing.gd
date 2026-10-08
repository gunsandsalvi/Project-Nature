## Implements PRE-20 PRE-21 PRE-23 PRE-24 PRE-26 PRE-28 PRE-30 PRE-31 PRE-33 PLT-04 WLD-13.
## T2.8a: explicit restricted receivers, split atlas pieces, owned masks and prepared water layers.
extends "res://fixtures/drawing.gd"

signal entrance_requested

const SURFACE_SHADER := preload("res://terrain/surface.gdshader")
const SPRITE_SHADER := preload("res://terrain/sprite.gdshader")
const WATER_SHADER := preload("res://terrain/water.gdshader")
var terrain := KdTerrain.new()
var scene_name := "flat"
var hour := "noon"
var weather := "dry"
var direction := 0
var fire_enabled := true
var debug_mode := "colour"
var reveal := true
var cutaway := true
var light_record: Dictionary = {}
var problem := ""
var ordering: Array = []
var surface_records: Array = []
var faded: Array[int] = []
var cpu_rebuild_ms := 0.0
var moving_mask_time := -1.0
var _surface_nodes: Dictionary = {}
var _lit_sprites: Dictionary = {}
var _masks: Dictionary = {}
var _revision := ""
var _mask_clock := -1
var _bed: SubViewport
var _reflections: SubViewport
var _copies: Dictionary = {}
var _reflection_sprites: Dictionary = {}
var _silhouettes: Dictionary = {}
var _layer_lines: Dictionary = {}
var _fire: Polygon2D
var _water_height := -100.0
var _water_bounds := Vector4.ZERO
var _scene_revision := ""
var _proxies: Array = []
var _plain: Texture2D
var _material_table := PackedVector4Array()


func _ready() -> void:
	super._ready()
	var neutral := Image.create(1, 1, false, Image.FORMAT_RGBA8)
	neutral.fill(Color("b6aa98"))
	_plain = ImageTexture.create_from_image(neutral)
	var tuning: Dictionary = JSON.parse_string(
		FileAccess.get_file_as_string("res://terrain/materials.json")
	)
	for values: Array in tuning.values:
		_material_table.append(Vector4(values[0], values[1], values[2], values[3]))
	light_record = terrain.set_light(hour, weather, direction, fire_enabled)
	for key: String in ["bed", "reflections"]:
		var target := SubViewport.new()
		target.disable_3d = true
		target.transparent_bg = true
		target.canvas_item_default_texture_filter = (
			Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
		)
		target.render_target_update_mode = SubViewport.UPDATE_ALWAYS
		add_child(target)
		if key == "bed":
			_bed = target
		else:
			_reflections = target
	_fire = Polygon2D.new()
	_fire.color = Color("ffc06a")
	_fire.material = ShaderMaterial.new()
	_fire.material.shader = SPRITE_SHADER
	_fire.set_meta("light_material", _fire.material)
	add_child(_fire)


func _draw() -> void:
	if not state.is_empty():
		draw_rect(
			Rect2(Vector2.ZERO, state.size),
			(
				Color.BLACK
				if pass_name != "colour"
				else Color("07101b") if scene_name == "cave" else Color("233237")
			)
		)
	# Ground is drawn by its measured receiver pieces.


func rebuild() -> void:
	if state.is_empty() or entries.is_empty() or _bed == null:
		return
	var started := Time.get_ticks_usec()
	var revision := "%s/%s/%s/%d/%s" % [scene_name, hour, weather, direction, fire_enabled]
	var changed := revision != _revision
	if changed:
		_reset_scene()
		_revision = revision
	_draws.clear()
	var density: int = int(state.density)
	var actors: Array = terrain.actors(second)
	for actor: Dictionary in actors:
		_actor(actor, density)
	# Atlas splits are developer metadata; the physical proxies remain independent of visual fading.
	var tree_at := Vector2(-5, -2) if scene_name == "cliff" else POSITIONS.tree
	_object(entries[0], tree_at, -1, density)
	_split_tree()
	if scene_name in ["flat", "slope", "water"]:
		_object(entries[1], POSITIONS.shelter, -2, density)
	_object(entries[2], POSITIONS.boulder, -3, density)
	_submit()
	for copy: Sprite2D in _reflection_sprites.values():
		copy.visible = false
	for copy: Sprite2D in _silhouettes.values():
		copy.visible = false
	# PRE-21 PLT-04: sampling follows scene time; the upload limit follows real time.
	var mask_second := (
		moving_mask_time if moving_mask_time >= 0.0 else Time.get_ticks_usec() / 1000000.0
	)
	var clock := int(floor(mask_second * 10.0))
	var update_masks := changed or clock != _mask_clock
	if update_masks:
		var bodies := []
		# PRE-21: sprites may be culled while their shadows still reach visible receivers.
		for actor: Dictionary in actors:
			bodies.append({"id": actor.id, "point": actor.point})
		terrain.bodies(bodies)
		_mask_clock = clock
	for target: SubViewport in [_bed, _reflections]:
		target.render_target_update_mode = (
			SubViewport.UPDATE_ALWAYS if scene_name == "water" else SubViewport.UPDATE_DISABLED
		)
		if scene_name == "water":
			target.remove_meta("drawn_a_second")
		else:
			target.set_meta("drawn_a_second", 0.0)
	_bed.size = Vector2i(state.size)
	_reflections.size = Vector2i(state.size)
	var pieces := _receivers(update_masks)
	if fire_enabled:
		var fire_point: Vector3 = light_record.fire
		var floor_record: Dictionary = terrain.walk(fire_point.x, fire_point.y)
		var flame: PackedVector2Array = _flame_points()
		var flame_rect := Rect2(flame[0], Vector2.ZERO).expand(flame[1]).expand(flame[2])
		pieces.append(
			_piece(
				-20,
				floor_record.surface,
				flame_rect,
				(
					-float(state.height_basis) * fire_point.y
					+ float(state.ground_basis) * fire_point.z
				),
				false
			)
		)
	for item: Dictionary in _draws:
		pieces.append(_sprite_piece(item))
	ordering = terrain.order(pieces)
	problem = (
		""
		if ordering.size() == pieces.size()
		else "ORDERING FAILED: split the overlapping pieces before adding art."
	)
	if not problem.is_empty():
		return
	faded.clear()
	for index in ordering.size():
		var id: int = ordering[index]
		if _surface_nodes.has(id):
			_surface_nodes[id].z_index = index + 1
			if _copies.has(id):
				_copies[id].z_index = index + 1
		elif _sprites.has(id):
			_sprites[id].z_index = index + 1
	_reveal_and_light()
	_debug_layers()
	_fire_preview(ordering.find(-20) + 1)
	cpu_rebuild_ms = (Time.get_ticks_usec() - started) / 1000.0
	queue_redraw()


func _sprite_piece(item: Dictionary) -> Dictionary:
	var p: Vector3 = item.record.get("point", Vector3.ZERO)
	var depth := -float(state.height_basis) * p.y + float(state.ground_basis) * p.z
	# PRE-24: the trunk orders by its footprint; only the raised crown uses proxy height.
	if item.id == -11:
		for proxy: Dictionary in _proxies:
			if proxy.id == 2:
				var centre: Vector3 = (proxy.low + proxy.high) / 2.0
				depth = -float(state.height_basis) * centre.y + float(state.ground_basis) * centre.z
	return _piece(item.id, item.record.get("surface", 0), item.rect, depth, false)


func _reset_scene() -> void:
	if scene_name != _scene_revision:
		_scene_revision = scene_name
		terrain.scene(scene_name)
		surface_records = terrain.surfaces()
		_proxies = terrain.proxies()
		_water_height = -100.0
		_water_bounds = Vector4.ZERO
		for record: Dictionary in surface_records:
			if record.kind == 4:
				_water_height = record.corners[0].z
				_water_bounds = Vector4(
					record.corners[0].x,
					record.corners[0].y,
					record.corners[2].x,
					record.corners[2].y
				)
		for group: Dictionary in [_surface_nodes, _copies, _layer_lines]:
			for node: Node in group.values():
				node.queue_free()
			group.clear()
		_masks.clear()
	light_record = terrain.set_light(hour, weather, direction, fire_enabled)


func _actor(actor: Dictionary, density: int, source: Dictionary = {}) -> void:
	var p: Vector3 = actor.point
	var record := source.duplicate()
	record.merge(
		{"point": p, "surface": actor.surface, "pixel": camera.project(p.x, p.y, p.z)}, true
	)
	record.phase = source.get("phase", fposmod(second, 1.0))
	record.activity = source.get("activity", 1)
	record.facing = source.get("facing", facing)
	_append(entries[actor.art], Vector2(p.x, p.y), actor.id, density, record)


func _object(entry: Dictionary, at: Vector2, id: int, density: int) -> void:
	var sampled: Dictionary = terrain.walk(at.x, at.y)
	if not sampled.found:
		return
	var p: Vector3 = sampled.point
	_append(
		entry,
		at,
		id,
		density,
		{
			"point": p,
			"surface": sampled.surface,
			"pixel": camera.project(p.x, p.y, p.z),
			"phase": 0.0,
			"activity": 0,
			"facing": 0
		}
	)


func _split_tree() -> void:
	if _draws.is_empty() or _draws[-1].id != -1:
		return
	var tree: Dictionary = _draws.pop_back()
	var crown := tree.duplicate()
	var trunk := tree.duplicate()
	var split: float = floor(tree.rect.size.y * 0.70)
	crown.id = -11
	crown.code = 1001
	crown.rect = Rect2(tree.rect.position, Vector2(tree.rect.size.x, split))
	crown.source = Rect2(tree.source.position, crown.rect.size)
	trunk.id = -12
	trunk.code = 1002
	trunk.rect = Rect2(tree.rect.position + Vector2(0, split), tree.rect.size - Vector2(0, split))
	trunk.source = Rect2(tree.source.position + Vector2(0, split), trunk.rect.size)
	var supplied: Dictionary = tree.entry.get("pieces", {})
	for name: String in ["crown", "trunk"]:
		if supplied.has(name):
			var raw: Array = supplied[name]
			var offset := (Vector2(raw[0], raw[1]) * float(state.density) / 64.0).round()
			var size := (Vector2(raw[2], raw[3]) * float(state.density) / 64.0).round()
			var item: Dictionary = crown if name == "crown" else trunk
			item.rect = Rect2(tree.rect.position + offset, size)
			item.source = Rect2(tree.source.position + offset, size)
	var at: Vector3 = tree.record.point
	crown.record = {"point": at, "surface": tree.record.surface}
	trunk.record = {"point": at, "surface": tree.record.surface}
	_draws.append(crown)
	_draws.append(trunk)


func _piece(id: int, surface: int, rect: Rect2, depth: float, receiver: bool) -> Dictionary:
	return {
		"id": id,
		"surface": surface,
		"covers": 0,
		"low": rect.position,
		"high": rect.end,
		"depth": depth,
		"receiver": receiver,
		"roof": false
	}


func _receivers(update_masks: bool) -> Array:
	var pieces := []
	for record: Dictionary in surface_records:
		var points := PackedVector2Array()
		var centre := Vector3.ZERO
		for p: Vector3 in record.corners:
			points.append(camera.project(p.x, p.y, p.z))
			centre += p / 4.0
		var rect := Rect2(points[0], Vector2.ZERO)
		for p: Vector2 in points:
			rect = rect.expand(p)
		if not _surface_nodes.has(record.id):
			var node := Polygon2D.new()
			node.texture = atlas.texture(entries[3], int(state.density), "walk")
			node.texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
			node.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
			node.material = ShaderMaterial.new()
			node.material.shader = WATER_SHADER if record.kind == 4 else SURFACE_SHADER
			node.set_meta("light_material", node.material)
			var semantic := ShaderMaterial.new()
			semantic.shader = ID_SHADER
			node.set_meta("semantic", semantic)
			add_child(node)
			_surface_nodes[record.id] = node
		var polygon: Polygon2D = _surface_nodes[record.id]
		polygon.polygon = points
		var texture: Texture2D = (
			atlas.texture(entries[3], int(state.density), "walk")
			if record.material == 4
			else _plain
		)
		polygon.texture = texture
		polygon.uv = PackedVector2Array(
			[
				Vector2(0, 0),
				Vector2(texture.get_width(), 0),
				Vector2(texture.get_width(), texture.get_height()),
				Vector2(0, texture.get_height())
			]
		)
		var material: ShaderMaterial = polygon.get_meta("light_material")
		_lighting(material)
		for i in 4:
			material.set_shader_parameter("corner%d" % i, record.corners[i])
		material.set_shader_parameter("surface_normal", record.normal)
		material.set_shader_parameter("material_kind", record.material)
		for kind: String in ["normal", "material"]:
			var provided: Texture2D = (
				atlas.texture(entries[3], int(state.density), "walk", kind)
				if record.material == 4
				else null
			)
			material.set_shader_parameter("has_" + kind, provided != null)
			if provided != null:
				material.set_shader_parameter(kind + "_atlas", provided)
		material.set_shader_parameter(
			"repeats",
			(
				Vector2(
					(record.corners[1] - record.corners[0]).length(),
					(record.corners[3] - record.corners[0]).length()
				)
				/ 4.0
			)
		)
		material.set_shader_parameter(
			"receiver_colour", Color.from_hsv(fposmod(record.id * 0.117, 1.0), 0.35, 0.65)
		)
		if update_masks or not _masks.has(record.id):
			var data: Dictionary = terrain.mask(record.id)
			var image := Image.create_from_data(
				data.width, data.height, false, Image.FORMAT_RGBA8, data.rgba
			)
			if _masks.has(record.id):
				_masks[record.id].update(image)
			else:
				_masks[record.id] = ImageTexture.create_from_image(image)
		material.set_shader_parameter("visibility_mask", _masks[record.id])
		if record.kind == 4:
			_water(material, record)
		else:
			_background(record.id, polygon)
		polygon.material = material
		if pass_name != "colour":
			var semantic: ShaderMaterial = polygon.get_meta("semantic")
			semantic.set_shader_parameter(
				"identity_colour",
				identity(2000 + record.id if pass_name == "object" else record.material)
			)
			polygon.material = semantic
		id_table[str(2000 + record.id)] = {
			"id": str(record.id), "source": "receiver", "surface": record.id
		}
		var piece := _piece(
			record.id,
			record.id,
			rect,
			-float(state.height_basis) * centre.y + float(state.ground_basis) * centre.z,
			true
		)
		piece.covers = record.covers
		piece.roof = record.kind == 2
		piece.water = record.kind == 4
		piece.minimum_height = record.corners[0].z
		piece.maximum_height = record.corners[0].z
		for corner: Vector3 in record.corners:
			piece.minimum_height = minf(piece.minimum_height, corner.z)
			piece.maximum_height = maxf(piece.maximum_height, corner.z)
		pieces.append(piece)
	return pieces


func _background(id: int, node: Polygon2D) -> void:
	if not _copies.has(id):
		var copy := Polygon2D.new()
		_bed.add_child(copy)
		_copies[id] = copy
	var copy: Polygon2D = _copies[id]
	copy.polygon = node.polygon
	copy.uv = node.uv
	copy.texture = node.texture
	copy.texture_repeat = node.texture_repeat
	copy.material = node.get_meta("light_material")
	copy.z_index = node.z_index


func _water(material: ShaderMaterial, record: Dictionary) -> void:
	material.set_shader_parameter("bed_background", _bed.get_texture())
	material.set_shader_parameter("reflection_layer", _reflections.get_texture())
	material.set_shader_parameter("viewport_size", Vector2(state.size))
	material.set_shader_parameter("fixture_second", second)
	var heights := Vector4.ZERO
	for i in 4:
		var p: Vector3 = record.corners[i]
		heights[i] = terrain.bed(p.x, p.y).point.z
	material.set_shader_parameter("bed_heights", heights)


func _lighting(material: ShaderMaterial) -> void:
	material.set_shader_parameter("material_table", _material_table)
	material.set_shader_parameter("sun_direction", light_record.sun)
	material.set_shader_parameter("sunlight", light_record.sunlight)
	material.set_shader_parameter("sky_light", light_record.sky)
	material.set_shader_parameter("fire_position", light_record.fire)
	material.set_shader_parameter("fire_on", fire_enabled)
	material.set_shader_parameter("debug_albedo", debug_mode == "albedo")
	material.set_shader_parameter("debug_normals", debug_mode == "normals")
	material.set_shader_parameter("debug_receivers", debug_mode == "receivers")


func _reveal_and_light() -> void:
	for record: Dictionary in surface_records:
		var node: Polygon2D = _surface_nodes[record.id]
		node.modulate = Color.WHITE
		if reveal and record.kind == 2 and cutaway:
			node.modulate.a = 0.24
			faded.append(record.id)
	for item: Dictionary in _draws:
		var node: Sprite2D = _sprites[item.id]
		var p: Vector3 = item.record.get(
			"point",
			Vector3(
				POSITIONS.get(item.entry.name, Vector2.ZERO).x,
				POSITIONS.get(item.entry.name, Vector2.ZERO).y,
				0
			)
		)
		if not _lit_sprites.has(item.id):
			var material := ShaderMaterial.new()
			material.shader = SPRITE_SHADER
			_lit_sprites[item.id] = material
		var lit: ShaderMaterial = _lit_sprites[item.id]
		_lighting(lit)
		lit.set_shader_parameter("material_kind", 7 if item.id == -12 else int(item.entry.id))
		lit.set_shader_parameter("foot", p)
		lit.set_shader_parameter("pivot", item.foot - item.rect.position)
		lit.set_shader_parameter("density", state.density)
		lit.set_shader_parameter("height_basis", state.height_basis)
		lit.set_shader_parameter("water_height", _water_height if scene_name == "water" else -100.0)
		lit.set_shader_parameter("water_bounds", _water_bounds)
		for kind: String in ["normal", "material"]:
			var texture: Texture2D = atlas.texture(item.entry, int(state.density), "walk", kind)
			lit.set_shader_parameter("has_" + kind, texture != null)
			if texture != null:
				lit.set_shader_parameter(kind + "_atlas", texture)
		var sun := Vector3.ZERO
		var sky := Vector3.ZERO
		var fire := Vector3.ZERO
		var band_low := 0.12
		var band_high := 1.72
		var proxy_id: int = 2 if item.id == -11 else 1 if item.id == -12 else -item.id
		if item.id in [-11, -12, -3, -2]:
			for proxy: Dictionary in _proxies:
				if proxy.id == proxy_id:
					band_low = proxy.low.z - p.z + 0.12
					band_high = proxy.high.z - p.z - 0.08
		lit.set_shader_parameter("band_min_height", band_low)
		lit.set_shader_parameter("band_max_height", band_high)
		for band in 3:
			var visible: Dictionary = terrain.visibility(
				p + Vector3(0, 0, lerpf(band_low, band_high, band / 2.0)), item.id
			)
			sun[band] = visible.sun
			sky[band] = visible.sky
			fire[band] = visible.fire
		lit.set_shader_parameter("sun_bands", sun)
		lit.set_shader_parameter("sky_bands", sky)
		lit.set_shader_parameter("fire_bands", fire)
		lit.set_shader_parameter("fade", 1.0)
		lit.set_shader_parameter(
			"receiver_colour", Color.from_hsv(fposmod(item.id * 0.117, 1.0), 0.4, 0.8)
		)
		node.modulate = Color.WHITE
		if pass_name == "colour" and channel == "colour":
			node.material = lit
		if pass_name != "colour":
			var semantic: ShaderMaterial = node.get_meta("semantic")
			for key: String in [
				"foot", "pivot", "density", "height_basis", "water_height", "water_bounds"
			]:
				semantic.set_shader_parameter(key, lit.get_shader_parameter(key))
			semantic.set_shader_parameter("clip_water", scene_name == "water")
			var mask: Texture2D = atlas.texture(item.entry, int(state.density), "walk", "material")
			semantic.set_shader_parameter(
				"use_material_index", pass_name == "material" and mask != null
			)
			if mask != null:
				semantic.set_shader_parameter("material_atlas", mask)
			if pass_name == "material" and item.id == -12:
				semantic.set_shader_parameter("identity_colour", identity(7))
		if selected != 0 and item.id == -11 and reveal and _covers_selected(item):
			lit.set_shader_parameter("fade", 0.24)
			faded.append(item.id)
		_reflection(item, node)
		_silhouette(item, node, lit)


func _covers_selected(cover: Dictionary) -> bool:
	for person: Dictionary in _draws:
		if person.id == selected and ordering.find(person.id) < ordering.find(cover.id):
			return person.rect.intersects(cover.rect)
	return false


func _reflection(item: Dictionary, node: Sprite2D) -> void:
	if not _reflection_sprites.has(item.id):
		var copy := Sprite2D.new()
		copy.centered = false
		copy.region_enabled = true
		_reflections.add_child(copy)
		_reflection_sprites[item.id] = copy
	var copy: Sprite2D = _reflection_sprites[item.id]
	copy.visible = scene_name == "water"
	copy.texture = node.texture
	copy.region_rect = node.region_rect
	var p: Vector3 = item.record.get("point", Vector3(0, 0, 0))
	var reflected: Vector2 = camera.project(p.x, p.y, 2.0 * _water_height - p.z)
	var pivot: Vector2 = item.foot - item.rect.position
	copy.position = reflected + Vector2(-pivot.x, pivot.y)
	copy.scale = Vector2(1, -1)
	copy.modulate = Color(0.65, 0.72, 0.75, 0.55)
	copy.material = node.material


func _silhouette(item: Dictionary, node: Sprite2D, lit: ShaderMaterial) -> void:
	if not _silhouettes.has(item.id):
		var copy := Sprite2D.new()
		copy.centered = false
		copy.region_enabled = true
		add_child(copy)
		_silhouettes[item.id] = copy
	var copy: Sprite2D = _silhouettes[item.id]
	var hidden := false
	if item.entry.name == "person" and item.id != selected and reveal and int(state.density) <= 16:
		for other: Dictionary in _draws:
			if other.id == -11 and ordering.find(other.id) > ordering.find(item.id):
				hidden = hidden or other.rect.intersects(item.rect)
		for record: Dictionary in surface_records:
			hidden = hidden or (record.kind == 2 and item.record.get("surface", 0) == record.covers)
	copy.visible = hidden and pass_name == "colour"
	copy.texture = node.texture
	copy.region_rect = node.region_rect
	copy.position = node.position
	copy.z_index = ordering.size() + 1
	if copy.material == null:
		copy.material = ShaderMaterial.new()
		copy.material.shader = SPRITE_SHADER
	for uniform: Dictionary in SPRITE_SHADER.get_shader_uniform_list():
		copy.material.set_shader_parameter(uniform.name, lit.get_shader_parameter(uniform.name))
	copy.material.set_shader_parameter("silhouette", true)


func _debug_layers() -> void:
	for line: Line2D in _layer_lines.values():
		line.visible = debug_mode == "layers" and pass_name == "colour"
	if debug_mode != "layers":
		return
	for record: Dictionary in surface_records:
		if not _layer_lines.has(record.id):
			var line := Line2D.new()
			line.width = 1.0
			line.default_color = Color("f2d4a6")
			line.z_index = 1024
			var label := Label.new()
			label.add_theme_font_size_override("font_size", 10)
			line.add_child(label)
			add_child(line)
			_layer_lines[record.id] = line
		var line: Line2D = _layer_lines[record.id]
		var points := PackedVector2Array()
		for p: Vector3 in record.corners:
			points.append(camera.project(p.x, p.y, p.z))
		points.append(points[0])
		line.points = points
		line.visible = pass_name == "colour"
		var label: Label = line.get_child(0)
		label.position = points[0]
		label.text = "%d:%d" % [record.id, ordering.find(record.id)]


func _flame_points() -> PackedVector2Array:
	# PRE-23 PRE-30: the emitter remains above ground, while the visible flame touches its receiver.
	var p: Vector3 = light_record.fire
	var ground: Vector3 = terrain.walk(p.x, p.y).point
	var foot: Vector2 = camera.project(ground.x, ground.y, ground.z)
	var top: Vector2 = camera.project(ground.x, ground.y, ground.z + 0.9)
	return PackedVector2Array([foot + Vector2(-5, 0), top, foot + Vector2(5, 0)])


func _fire_preview(depth: int) -> void:
	_fire.visible = fire_enabled
	var p: Vector3 = light_record.fire
	var foot: Vector2 = camera.project(p.x, p.y, p.z)
	_fire.polygon = _flame_points()
	_fire.z_index = depth
	var material: ShaderMaterial = _fire.get_meta("light_material")
	_lighting(material)
	material.set_shader_parameter("emission", 0.0)
	material.set_shader_parameter("material_kind", 9)
	material.set_shader_parameter("emission_colour", Vector3(1.0, 0.32, 0.045))
	material.set_shader_parameter("height_basis", state.height_basis)
	material.set_shader_parameter("foot", p)
	material.set_shader_parameter("density", state.density)
	material.set_shader_parameter("pivot", foot)
	_fire.material = material
	if pass_name != "colour":
		var semantic := ShaderMaterial.new()
		semantic.shader = ID_SHADER
		semantic.set_shader_parameter(
			"identity_colour", identity(9001 if pass_name == "object" else 9)
		)
		_fire.material = semantic
	id_table["9001"] = {
		"id": "-20", "source": "developer flame", "surface": terrain.walk(p.x, p.y).surface
	}


func pick(pixel: Vector2) -> Dictionary:
	# PRE-28: the declared doorway enters a separate interior, never the geological slice.
	if scene_name in ["shelter", "cave"]:
		var doorway: Vector2 = camera.project(3, 1, 0.6)
		if pixel.distance_to(doorway) < 14.0:
			entrance_requested.emit()
			return {"found": true, "entrance": true}
	for index in range(ordering.size() - 1, -1, -1):
		var id: int = ordering[index]
		if _surface_nodes.has(id):
			if faded.has(id):
				continue
			if Geometry2D.is_point_in_polygon(pixel, _surface_nodes[id].polygon):
				var hit: Dictionary = terrain.hit_surface(id, camera, pixel)
				if hit.get("found", false):
					selected = 0
					return hit
		for actor: Dictionary in _draws:
			if actor.id != id or not actor.rect.has_point(pixel) or faded.has(id):
				continue
			var p: Vector3 = actor.record.get("point", Vector3.ZERO)
			var east: float = p.x + (pixel.x - actor.foot.x) / float(state.density)
			var within_water := (
				east >= _water_bounds.x
				and east <= _water_bounds.z
				and p.y >= _water_bounds.y
				and p.y <= _water_bounds.w
			)
			if (
				scene_name == "water"
				and within_water
				and pixel.y > camera.project(p.x, p.y, _water_height).y
			):
				continue
			var at: Vector2 = (pixel - actor.rect.position + actor.source.position).floor()
			if actor.image.get_pixel(int(at.x), int(at.y)).a > 0.1:
				selected = id
				return {
					"id": id, "surface": actor.record.get("surface", 0), "point": p, "found": true
				}
	selected = 0
	return {}
