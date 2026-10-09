## Checks PRE-03 PRE-42 PRE-43 PLT-04: canonical fields and culled receiver ownership.
extends GdUnitTestSuite

const Variants := preload("res://test/support/terrain/ground_variants.gd")
const Drawing := preload("res://test/support/terrain/drawing.gd")
const Stream := preload("res://test/support/fixtures/sprite_stream.gd")
const SurfaceShader := preload("res://test/support/terrain/surface.gdshader")


class UnitAtlas:
	extends "res://test/support/fixtures/atlas.gd"
	var page: ImageTexture
	var span := 4.0

	func _init() -> void:
		var image := Image.create(2, 2, false, Image.FORMAT_RGBA8)
		image.fill(Color.WHITE)
		page = ImageTexture.create_from_image(image)

	func read(_path := "res://test/support/fixtures/manifest.json") -> bool:
		entries = [{}, {}, {}, {"name": "ground"}]
		return true

	func texture(
		_entry: Dictionary, _density: float, _action: String, _channel := "colour"
	) -> Texture2D:
		return page

	func ground_sample(_density: float, _asset := "meadow") -> Dictionary:
		return {
			"density": 4.0,
			"tile_metres": span,
			"textures": {"colour": page, "normal": page, "material": page}
		}


func _service() -> Node:
	var stream: Node = auto_free(Stream.new())
	add_child(stream)
	stream.set_process(false)
	(
		assert_bool(
			(
				stream
				. begin(
					{
						"world_id": "ground-test",
						"data_hash": "data",
						"look_hash": "look",
						"renderer": "test",
						"format_version": 1,
						"epoch": 1
					},
					{}
				)
				. ok
			)
		)
		. is_true()
	)
	return stream


func _field_state() -> Dictionary:
	return {
		"world_width_cm": 200000000,
		"world_height_cm": 100000000,
		"ground_tiles":
		[{"power": 2, "x": 0, "y": 0, "local_west": 0.0, "local_south": 0.0, "variant": 2}]
	}


func _drain(stream: Node) -> void:
	for frame in 3:
		await get_tree().process_frame
	stream._process(0)


func test_identical_field_reopens_after_release_with_real_old_new_overlap() -> void:
	var stream := _service()
	var field: RefCounted = Variants.new()
	var state := _field_state()
	field.update(state, {"east": 0, "north": 0}, stream)
	assert_object(field.texture).is_not_null()
	assert_int(stream.ledger.status().resident_bytes).is_equal(4)
	var old: int = field._ticket
	field.release()
	assert_int(stream.ledger.status().resident_bytes).is_equal(4)
	field.update(state, {"east": 0, "north": 0}, stream)
	assert_object(field.texture).is_not_null()
	assert_int(field._ticket).is_not_equal(old)
	assert_int(stream.ledger.status().resident_bytes).is_equal(8)
	await _drain(stream)
	assert_int(stream.ledger.status().resident_bytes).is_equal(4)
	field.release()
	await _drain(stream)
	assert_int(stream.ledger.status().allocation_count).is_equal(0)
	assert_int(stream.ledger.status().input_bytes).is_equal(0)


func test_fallback_family_cannot_switch_inside_authored_page_and_disable_detaches_samplers(
) -> void:
	var stream := _service()
	var field: RefCounted = Variants.new()
	field.update(_field_state(), {"east": 0, "north": 0}, stream)
	var atlas := UnitAtlas.new()
	var material := ShaderMaterial.new()
	material.shader = SurfaceShader
	atlas.span = 64.0
	field.apply(material, atlas, 32, true)
	assert_bool(material.get_shader_parameter("ground_variants")).is_false()
	assert_object(material.get_shader_parameter("variant_field")).is_null()
	atlas.span = 4.0
	field.apply(material, atlas, 32, true)
	assert_bool(material.get_shader_parameter("ground_variants")).is_true()
	assert_object(material.get_shader_parameter("variant_field")).is_same(field.texture)
	field.apply(material, atlas, 32, false)
	assert_object(material.get_shader_parameter("variant_field")).is_null()
	assert_object(material.get_shader_parameter("variant_b_colour")).is_null()
	assert_object(material.get_shader_parameter("variant_c_material")).is_null()
	field.release()
	await _drain(stream)


func test_culled_receiver_detaches_textures_and_reentry_refreshes_changed_light_with_staging(
) -> void:
	var stream := _service()
	var drawing: Node2D = auto_free(Drawing.new())
	drawing.stream_service = stream
	drawing.atlas = UnitAtlas.new()
	drawing.camera = KdCanvas.new()
	drawing.candidate_view = true
	drawing.absolute_origin = {"east": 0, "north": 0}
	add_child(drawing)
	drawing.terrain.scene("candidate-flat")
	var all: Array = drawing.terrain.surfaces()
	var record: Dictionary = all[0]
	drawing.surface_records = [record]
	var centre: Vector3 = (record.corners[0] + record.corners[2]) / 2.0
	drawing.camera.focus(centre.x, centre.y)
	drawing.state = drawing.camera.frame(128, 128, 0)
	drawing.light_record = drawing.terrain.set_light("noon", "clear", 0, false)
	drawing._revision = "noon"
	assert_int(drawing._receivers(true).size()).is_equal(1)
	var builds: int = drawing.terrain.costs().static_builds
	var node: Polygon2D = drawing._surface_nodes[record.id]
	var material: ShaderMaterial = node.get_meta("light_material")
	assert_object(node.texture).is_not_null()
	assert_object(material.get_shader_parameter("normal_atlas")).is_not_null()
	drawing.camera.focus(centre.x + 1000, centre.y + 1000)
	drawing.state = drawing.camera.frame(128, 128, 0)
	assert_int(drawing._receivers(false).size()).is_equal(0)
	assert_object(node.texture).is_null()
	assert_object(material.get_shader_parameter("normal_atlas")).is_null()
	assert_object(drawing._copies[record.id].texture).is_null()
	drawing._revision = "dusk"
	drawing.light_record = drawing.terrain.set_light("dusk", "clear", 0, false)
	drawing._receivers(true)
	assert_int(drawing.terrain.costs().static_builds).is_equal(builds)
	await _drain(stream)
	assert_int(stream.ledger.status().staging_bytes).is_equal(0)
	drawing._mask_upload_reserved = false
	drawing.camera.focus(centre.x, centre.y)
	drawing.state = drawing.camera.frame(128, 128, 0)
	assert_int(drawing._receivers(false).size()).is_equal(1)
	assert_int(drawing.terrain.costs().static_builds).is_equal(builds + 1)
	assert_str(drawing._mask_versions[record.id]).is_equal("dusk")
	assert_int(stream.ledger.status().staging_bytes).is_greater(0)
	drawing._receivers(false)
	assert_int(drawing.terrain.costs().static_builds).is_equal(builds + 1)


func test_retiring_bundle_detaches_draws_and_hidden_shader_handles() -> void:
	var drawing: Node2D = auto_free(Drawing.new())
	var node := Polygon2D.new()
	var material := ShaderMaterial.new()
	material.shader = SurfaceShader
	var atlas := UnitAtlas.new()
	material.set_shader_parameter("normal_atlas", atlas.page)
	node.texture = atlas.page
	node.set_meta("light_material", material)
	drawing.add_child(node)
	drawing._draws.append({"texture": atlas.page})
	drawing.detach_textures(1)
	assert_object(node.texture).is_null()
	assert_object(material.get_shader_parameter("normal_atlas")).is_null()
	assert_int(drawing._draws.size()).is_equal(0)
