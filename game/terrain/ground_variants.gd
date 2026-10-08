## Implements PRE-03 PRE-42 PRE-43: bounded canonical variants shared by all ground channels.
extends RefCounted

var texture: ImageTexture
var grid := Vector4.ZERO
var phase := Vector2.ZERO
var origin_tile := Vector2.ZERO
var tile_count := Vector2.ZERO
var edges := Vector4.ZERO
var world_span := Vector2.ZERO
var cell_metres := 16.0
var _signature := ""
var _ticket := 0
var _service: Node


func update(state: Dictionary, origin: Dictionary, service: Node) -> void:
	var rows: Array = state.get("ground_tiles", [])
	if rows.is_empty() or origin.is_empty():
		return
	_service = service
	var first: Dictionary = rows[0]
	var first_x := first
	var first_y := first
	for row: Dictionary in rows:
		if row.local_west < first_x.local_west:
			first_x = row
		if row.local_south < first_y.local_south:
			first_y = row
	var pitch_cm := 100 * (1 << int(first.power))
	# The world dimensions are supplied by the canonical tile demand, including its last partial cell.
	var span := Vector2i(int(state.world_width_cm), int(state.world_height_cm))
	var count_x := ceili(float(span.x) / pitch_cm)
	var count_y := ceili(float(span.y) / pitch_cm)
	var width := 1
	var height := 1
	var cells := []
	for row: Dictionary in rows:
		var x := posmod(int(row.x) - int(first_x.x), count_x)
		var y := posmod(int(row.y) - int(first_y.y), count_y)
		width = maxi(width, x + 1)
		height = maxi(height, y + 1)
		cells.append(Vector3i(x, y, int(row.variant)))
	if width * height > 256:
		return
	var signature := str([first.power, first_x.x, first_y.y, cells])
	if signature == _signature:
		return
	var bytes := width * height * 4
	var allocation: Dictionary = service.reserve_allocation(
		{"category": "ground", "prepared": bytes * 2, "staging": bytes, "resident": bytes}
	)
	if not allocation.ok:
		return
	var image := Image.create(width, height, false, Image.FORMAT_RGBA8)
	image.fill(Color(0, 0, 0, 1))
	for cell: Vector3i in cells:
		image.set_pixel(cell.x, cell.y, Color(float(cell.z) / 255.0, 0, 0, 1))
	texture = ImageTexture.create_from_image(image)
	service.retire_allocation(_ticket)
	_ticket = allocation.token
	_signature = signature
	grid = Vector4(first_x.x, first_y.y, width, height)
	cell_metres = pitch_cm / 100.0
	phase = Vector2(posmod(int(origin.east), pitch_cm), posmod(int(origin.north), pitch_cm)) / 100.0
	origin_tile = Vector2(int(origin.east) / pitch_cm, int(origin.north) / pitch_cm).floor()
	tile_count = Vector2(count_x, count_y)
	edges = (
		Vector4(
			float(origin.east),
			float(origin.north),
			float(span.x - int(origin.east)),
			float(span.y - int(origin.north))
		)
		/ 100.0
	)
	world_span = Vector2(span) / 100.0


func apply(material: ShaderMaterial, atlas: RefCounted, density: float, enabled: bool) -> void:
	var second: Dictionary = atlas.ground_sample(density, "meadow_v2") if enabled else {}
	var third: Dictionary = atlas.ground_sample(density, "meadow_v3") if enabled else {}
	var base: Dictionary = atlas.ground_sample(density) if enabled else {}
	enabled = (
		enabled
		and texture != null
		and not base.is_empty()
		and not second.is_empty()
		and not third.is_empty()
	)
	if enabled:
		enabled = (
			base.density == second.density
			and base.density == third.density
			and cell_metres >= float(base.tile_metres)
			and is_zero_approx(fmod(cell_metres, float(base.tile_metres)))
		)
	material.set_shader_parameter("ground_variants", enabled)
	if not enabled:
		material.set_shader_parameter("variant_field", null)
		for prefix: String in ["variant_b_", "variant_c_"]:
			for channel: String in ["colour", "normal", "material"]:
				material.set_shader_parameter(prefix + channel, null)
		return
	material.set_shader_parameter("variant_field", texture)
	material.set_shader_parameter("variant_grid", grid)
	material.set_shader_parameter("variant_phase", phase)
	material.set_shader_parameter("variant_origin_tile", origin_tile)
	material.set_shader_parameter("variant_tile_count", tile_count)
	material.set_shader_parameter("variant_cell_metres", cell_metres)
	material.set_shader_parameter("variant_edges", edges)
	material.set_shader_parameter("variant_world_span", world_span)
	for channel: String in ["colour", "normal", "material"]:
		material.set_shader_parameter("variant_b_" + channel, second.textures[channel])
		material.set_shader_parameter("variant_c_" + channel, third.textures[channel])


func release() -> void:
	texture = null
	_signature = ""
	if is_instance_valid(_service):
		_service.retire_allocation(_ticket)
	_ticket = 0
