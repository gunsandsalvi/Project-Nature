## Implements PRE-20 PRE-21 PRE-23 PRE-24 PRE-26 PRE-28 PRE-30 PRE-31 PRE-33 PLT-04 WLD-13.
## T2.8a: explicit restricted receivers, split atlas pieces, owned masks and prepared water layers.
extends "res://fixtures/drawing.gd"

signal entrance_requested

const SURFACE_SHADER := preload("res://terrain/surface.gdshader")
const SPRITE_SHADER := preload("res://terrain/sprite.gdshader")
const WATER_SHADER := preload("res://terrain/water.gdshader")
var candidate_view := false
var stream_service: Node
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
var _mask_versions := {}
var _mask_upload_reserved := false
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
var _meadow: Polygon2D
var _open_mask: Texture2D
var _plain: Texture2D
var _material_table := PackedVector4Array()
var _mask_cpu_ticket := 0
var _mask_gpu_ticket := 0
var _variants := preload("res://terrain/ground_variants.gd").new()


func _ready() -> void:
	super._ready()
	if not absolute_origin.is_empty():
		terrain.set_origin(int(absolute_origin.east), int(absolute_origin.north))
	if stream_service != null:
		var allocation: Dictionary = stream_service.reserve_allocation(
			{
				"category": "masks",
				"prepared": int(terrain.costs().mask_cache_cap_bytes) + 65536,
				"resident": 8
			}
		)
		if not allocation.ok:
			problem = allocation.problem
			return
		_mask_cpu_ticket = allocation.token
	var neutral := Image.create(1, 1, false, Image.FORMAT_RGBA8)
	neutral.fill(Color("b6aa98"))
	_plain = ImageTexture.create_from_image(neutral)
	var tuning: Dictionary = JSON.parse_string(
		FileAccess.get_file_as_string("res://terrain/materials.json")
	)
	for values: Array in tuning.values:
		_material_table.append(Vector4(values[0], values[1], values[2], values[3]))
	light_record = terrain.set_light(hour, weather, direction, fire_enabled)
	var open := Image.create(1, 1, false, Image.FORMAT_RGBA8)
	open.fill(Color.WHITE)
	_open_mask = ImageTexture.create_from_image(open)
	_meadow = Polygon2D.new()
	_meadow.texture_repeat = CanvasItem.TEXTURE_REPEAT_ENABLED
	_meadow.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_meadow.material = ShaderMaterial.new()
	_meadow.material.shader = SURFACE_SHADER
	add_child(_meadow)
	for key: String in ["bed", "reflections"]:
		var target := SubViewport.new()
		target.size = Vector2i(2, 2)
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
	_mask_upload_reserved = false
	var started := Time.get_ticks_usec()
	var revision := "%s/%s/%s/%d/%s" % [scene_name, hour, weather, direction, fire_enabled]
	var changed := revision != _revision
	if changed:
		if not _reserve_mask_change(scene_name != _scene_revision):
			return
		_reset_scene()
		_revision = revision
	_draws.clear()
	var density: float = float(state.density)
	var actors: Array = [] if candidate_view else terrain.actors(second)
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
	for group: Dictionary in [_reflection_sprites, _silhouettes]:
		for copy: Sprite2D in group.values():
			copy.hide()
			copy.texture = null
			copy.material = null
	for id: int in _lit_sprites.keys():
		if not _sprites.has(id):
			_lit_sprites.erase(id)
	# PRE-21 PLT-04: sampling follows scene time; the upload limit follows real time.
	var mask_second := (
		moving_mask_time if moving_mask_time >= 0.0 else Time.get_ticks_usec() / 1000000.0
	)
	var clock := int(floor(mask_second * 10.0))
	var update_masks := changed or (not actors.is_empty() and clock != _mask_clock)
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
	if candidate_view:
		_variants.update(state, absolute_origin, stream_service)
	_background_plane()
	var pieces := []
	if candidate_view and float(state.density) < 2.0:
		for id: int in _surface_nodes:
			_hide_receiver(id)
	else:
		pieces = _receivers(update_masks)
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
		var bounds := _crown_bounds()
		if not bounds.is_empty():
			var centre: Vector3 = (bounds.low + bounds.high) / 2.0
			depth = -float(state.height_basis) * centre.y + float(state.ground_basis) * centre.z
	return _piece(item.id, item.record.get("surface", 0), item.rect, depth, false)


func _reset_scene() -> void:
	if scene_name != _scene_revision:
		_scene_revision = scene_name
		terrain.scene(("candidate-" if candidate_view else "") + scene_name)
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
		_mask_versions.clear()
	light_record = terrain.set_light(hour, weather, direction, fire_enabled)


func _actor(actor: Dictionary, density: float, source: Dictionary = {}) -> void:
	var p: Vector3 = actor.point
	var record := source.duplicate()
	record.merge({"point": p, "surface": actor.surface, "pixel": _project(p.x, p.y, p.z)}, true)
	record.phase = source.get("phase", fposmod(second, 1.0))
	record.activity = source.get("activity", 1)
	record.facing = source.get("facing", facing)
	_append(entries[actor.art], Vector2(p.x, p.y), actor.id, density, record)


func _object(entry: Dictionary, at: Vector2, id: int, density: float) -> void:
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
			"pixel": _project(p.x, p.y, p.z),
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
	var ratio: float = tree.get("source_scale", 1.0)
	var source_split: float = floor(tree.source.size.y * 0.70)
	var split := source_split * ratio
	crown.id = -11
	crown.code = 1001
	crown.rect = Rect2(tree.rect.position, Vector2(tree.rect.size.x, split))
	crown.source = Rect2(tree.source.position, crown.rect.size / ratio)
	trunk.id = -12
	trunk.code = 1002
	trunk.rect = Rect2(tree.rect.position + Vector2(0, split), tree.rect.size - Vector2(0, split))
	trunk.source = Rect2(tree.source.position + Vector2(0, source_split), trunk.rect.size / ratio)
	var supplied: Dictionary = tree.entry.get("pieces", {})
	for name: String in ["crown", "trunk"]:
		if supplied.has(name) and not candidate_view:
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
		if candidate_view and record.id >= 900:
			continue
		var points := PackedVector2Array()
		var centre := Vector3.ZERO
		for p: Vector3 in record.corners:
			points.append(_project(p.x, p.y, p.z))
			centre += p / 4.0
		var rect := Rect2(points[0], Vector2.ZERO)
		for p: Vector2 in points:
			rect = rect.expand(p)
		if not Rect2(Vector2.ZERO, state.size).intersects(rect):
			_hide_receiver(record.id)
			continue
		if not _surface_nodes.has(record.id):
			var node := Polygon2D.new()
			node.texture = atlas.texture(entries[3], float(state.density), "walk")
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
		var ground_bundle := {}
		if candidate_view and record.material == 4:
			var asset := "meadow"
			if scene_name == "water":
				asset = "bank_gravel" if record.id == 31 else "river_bed"
			ground_bundle = atlas.ground_sample(float(state.density), asset)
		var texture: Texture2D = (
			atlas.texture(entries[3], float(state.density), "walk")
			if record.material == 4
			else _plain
		)
		if not ground_bundle.is_empty():
			texture = ground_bundle.textures.colour
		if texture == null:
			polygon.visible = false
			continue
		polygon.visible = true
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
		material.set_shader_parameter("world_tiled", candidate_view and record.material == 4)
		var span: float = float(ground_bundle.get("tile_metres", _ground_span()))
		material.set_shader_parameter("tile_metres", span)
		material.set_shader_parameter("art_phase", _art_phase(span))
		if candidate_view and record.kind != 4:
			_variants.apply(
				material,
				atlas,
				float(state.density),
				record.material == 4 and scene_name != "water"
			)
		for i in 4:
			material.set_shader_parameter("corner%d" % i, record.corners[i])
		material.set_shader_parameter("surface_normal", record.normal)
		material.set_shader_parameter("material_kind", record.material)
		for kind: String in ["normal", "material"]:
			var provided: Texture2D = (
				atlas.texture(entries[3], float(state.density), "walk", kind)
				if record.material == 4
				else null
			)
			if not ground_bundle.is_empty():
				provided = ground_bundle.textures[kind]
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
				/ _ground_span()
			)
		)
		material.set_shader_parameter(
			"receiver_colour", Color.from_hsv(fposmod(record.id * 0.117, 1.0), 0.35, 0.65)
		)
		var mask_version := _revision + ("" if candidate_view else "/" + str(_mask_clock))
		if update_masks or _mask_versions.get(record.id, "") != mask_version:
			if not _reserve_mask_upload():
				_hide_receiver(record.id)
				continue
			var data: Dictionary = terrain.mask(record.id)
			var image := Image.create_from_data(
				data.width, data.height, false, Image.FORMAT_RGBA8, data.rgba
			)
			if _masks.has(record.id):
				_masks[record.id].update(image)
			else:
				_masks[record.id] = ImageTexture.create_from_image(image)
			_mask_versions[record.id] = mask_version
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
	copy.show()
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
	material.set_shader_parameter("sdr_canvas", not get_viewport().use_hdr_2d)
	material.set_shader_parameter("material_table", _material_table)
	material.set_shader_parameter("sun_direction", light_record.sun)
	material.set_shader_parameter("sunlight", light_record.sunlight)
	material.set_shader_parameter("sky_light", light_record.sky)
	material.set_shader_parameter("fire_position", light_record.fire)
	material.set_shader_parameter("fire_on", fire_enabled)
	material.set_shader_parameter("debug_albedo", debug_mode == "albedo")
	material.set_shader_parameter("debug_normals", debug_mode == "normals")
	material.set_shader_parameter(
		"debug_visibility", ["sun", "sky", "contact"].find(debug_mode) + 1
	)
	material.set_shader_parameter("debug_receivers", debug_mode == "receivers")


func _reveal_and_light() -> void:
	for record: Dictionary in surface_records:
		if not _surface_nodes.has(record.id):
			continue
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
		lit.set_shader_parameter(
			"pivot", (item.foot - item.rect.position) / float(item.get("source_scale", 1.0))
		)
		lit.set_shader_parameter("density", item.get("source_density", state.density))
		lit.set_shader_parameter("height_basis", state.height_basis)
		lit.set_shader_parameter("water_height", _water_height if scene_name == "water" else -100.0)
		lit.set_shader_parameter("water_bounds", _water_bounds)
		for kind: String in ["normal", "material"]:
			var texture: Texture2D = atlas.texture(item.entry, float(state.density), "walk", kind)
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
		if item.id == -11:
			var bounds := _crown_bounds()
			if not bounds.is_empty():
				band_low = bounds.low.z - p.z + 0.12
				band_high = bounds.high.z - p.z - 0.08
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
			var mask: Texture2D = atlas.texture(
				item.entry, float(state.density), "walk", "material"
			)
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
	var reflected: Vector2 = _project(p.x, p.y, 2.0 * _water_height - p.z)
	var pivot: Vector2 = item.foot - item.rect.position
	copy.position = reflected + Vector2(-pivot.x, pivot.y)
	copy.scale = Vector2(1, -1) * float(item.get("source_scale", 1.0))
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
	if (
		item.entry.name == "person"
		and item.id != selected
		and reveal
		and float(state.density) <= 16
	):
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
			points.append(_project(p.x, p.y, p.z))
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
	var foot: Vector2 = _project(ground.x, ground.y, ground.z)
	var top: Vector2 = _project(ground.x, ground.y, ground.z + 0.9)
	return PackedVector2Array([foot + Vector2(-5, 0), top, foot + Vector2(5, 0)])


func _fire_preview(depth: int) -> void:
	_fire.visible = fire_enabled
	var p: Vector3 = light_record.fire
	var foot: Vector2 = _project(p.x, p.y, p.z)
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
		var doorway: Vector2 = _project(3, 1, 0.6)
		if pixel.distance_to(doorway) < 14.0:
			entrance_requested.emit()
			return {"found": true, "entrance": true, "second": state.get("second", second)}
	for index in range(ordering.size() - 1, -1, -1):
		var id: int = ordering[index]
		if _surface_nodes.has(id):
			if faded.has(id):
				continue
			if Geometry2D.is_point_in_polygon(pixel, _surface_nodes[id].polygon):
				var hit: Dictionary = terrain.hit_surface(id, camera, pixel)
				if hit.get("found", false):
					selected = 0
					hit.second = state.get("second", second)
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
				and pixel.y > _project(p.x, p.y, _water_height).y
			):
				continue
			var at: Vector2 = (
				(
					(pixel - actor.rect.position) / float(actor.get("source_scale", 1.0))
					+ actor.source.position
				)
				. floor()
			)
			if not Rect2(Vector2.ZERO, actor.image.get_size()).has_point(at):
				continue
			if actor.image.get_pixel(int(at.x), int(at.y)).a > 0.1:
				selected = id
				return {
					"id": id,
					"surface": actor.record.get("surface", 0),
					"point": p,
					"found": true,
					"second": state.get("second", second)
				}
	selected = 0
	return {}


func _ground_span() -> float:
	if atlas.has_method("sample"):
		var sample: Dictionary = atlas.sample(entries[3], float(state.density))
		return float(sample.get("tile_metres", 4.0))
	return 4.0


func _background_plane() -> void:
	_meadow.visible = candidate_view and pass_name == "colour"
	if not _meadow.visible:
		return
	var sample: Dictionary = atlas.ground_sample(float(state.density))
	if sample.is_empty():
		_meadow.visible = false
		return
	_meadow.texture = sample.textures.colour
	var size: Vector2 = state.size
	var corners := PackedVector2Array([Vector2.ZERO, Vector2(size.x, 0), size, Vector2(0, size.y)])
	_meadow.polygon = corners
	var texels: Vector2 = _meadow.texture.get_size()
	_meadow.uv = PackedVector2Array(
		[Vector2.ZERO, Vector2(texels.x, 0), texels, Vector2(0, texels.y)]
	)
	var material: ShaderMaterial = _meadow.material
	_lighting(material)
	var presentation: float = float(state.scale) * float(state.live_scale)
	for i in 4:
		var ground: Vector2 = camera.ground(corners[i] * presentation + state.offset, 0)
		if not absolute_origin.is_empty():
			ground += Vector2(
				float(int(state.footprint.origin_east_cm) - int(absolute_origin.east)) / 100.0,
				float(int(state.footprint.origin_north_cm) - int(absolute_origin.north)) / 100.0
			)
		material.set_shader_parameter("corner%d" % i, Vector3(ground.x, ground.y, 0))
	material.set_shader_parameter("world_tiled", true)
	material.set_shader_parameter("tile_metres", sample.tile_metres)
	material.set_shader_parameter("art_phase", _art_phase(sample.tile_metres))
	_variants.apply(material, atlas, float(state.density), true)
	material.set_shader_parameter("visibility_mask", _open_mask)
	material.set_shader_parameter("material_kind", 4)
	for channel: String in ["normal", "material"]:
		material.set_shader_parameter("has_" + channel, true)
		material.set_shader_parameter(channel + "_atlas", sample.textures[channel])


func _reserve_mask_change(geometry_changed: bool) -> bool:
	if stream_service == null:
		return true
	# The restricted fixture cache has at most64 aligned65×65 pages; reserve before any allocation.
	var bytes := 64 * 65 * 65 * 4
	var upload: Dictionary = stream_service.reserve_allocation(
		{"category": "masks", "staging": bytes}
	)
	if not upload.ok:
		problem = upload.problem
		return false
	if geometry_changed:
		var gpu: Dictionary = stream_service.reserve_allocation(
			{"category": "masks", "resident": bytes}
		)
		if not gpu.ok:
			stream_service.ledger.release_allocation(upload.token)
			problem = gpu.problem
			return false
		stream_service.retire_allocation(_mask_gpu_ticket)
		_mask_gpu_ticket = gpu.token
	stream_service.retire_allocation(upload.token)
	_mask_upload_reserved = true
	return true


func _exit_tree() -> void:
	_variants.release()
	if stream_service != null:
		stream_service.retire_allocation(_mask_cpu_ticket)
		stream_service.retire_allocation(_mask_gpu_ticket)


func _crown_bounds() -> Dictionary:
	var bounds := {}
	for proxy: Dictionary in _proxies:
		if proxy.id != 2 and not (proxy.get("group", 0) == 1 and proxy.id != 1):
			continue
		if bounds.is_empty():
			bounds = {"low": proxy.low, "high": proxy.high}
		else:
			bounds.low = bounds.low.min(proxy.low)
			bounds.high = bounds.high.max(proxy.high)
	return bounds


func _art_phase(span: float) -> Vector2:
	if absolute_origin.is_empty():
		return Vector2.ZERO
	var centimetres := roundi(span * 100)
	return (
		Vector2(
			posmod(int(absolute_origin.east), centimetres),
			posmod(int(absolute_origin.north), centimetres)
		)
		/ 100.0
	)


func _reserve_mask_upload() -> bool:
	if stream_service == null or _mask_upload_reserved:
		return true
	var allocation: Dictionary = stream_service.reserve_allocation(
		{"category": "masks", "staging": 64 * 65 * 65 * 4}
	)
	if not allocation.ok:
		problem = allocation.problem
		return false
	stream_service.retire_allocation(allocation.token)
	_mask_upload_reserved = true
	return true


func _hide_receiver(id: int) -> void:
	if _surface_nodes.has(id):
		var node: Polygon2D = _surface_nodes[id]
		node.hide()
		node.texture = null
		var material: ShaderMaterial = node.get_meta("light_material")
		for key: String in ["normal_atlas", "material_atlas", "variant_field"]:
			material.set_shader_parameter(key, null)
		for prefix: String in ["variant_b_", "variant_c_"]:
			for channel: String in ["colour", "normal", "material"]:
				material.set_shader_parameter(prefix + channel, null)
	if _copies.has(id):
		_copies[id].hide()
		_copies[id].texture = null
		_copies[id].material = null


func detach_textures(_token: int) -> void:
	# Drop draw and shader handles before the atlas retires a published bundle.
	_draws.clear()
	var materials: Array = _lit_sprites.values()
	for node: Node in find_children("*", "CanvasItem", true, false):
		if node is Sprite2D or node is Polygon2D:
			node.texture = null
		if node.get("material") is ShaderMaterial:
			materials.append(node.material)
		if node.has_meta("light_material"):
			materials.append(node.get_meta("light_material"))
	for material: ShaderMaterial in materials:
		for parameter: String in [
			"normal_atlas",
			"material_atlas",
			"variant_field",
			"variant_b_colour",
			"variant_b_normal",
			"variant_b_material",
			"variant_c_colour",
			"variant_c_normal",
			"variant_c_material"
		]:
			material.set_shader_parameter(parameter, null)
