## Checks PRE-03 PRE-28 PRE-33 PLT-04 PLT-07 TIM-17 (T2.9a).
extends GdUnitTestSuite

const Drawing := preload("res://test/support/terrain/drawing.gd")
const Targets := preload("res://test/support/render_targets.gd")
const Stream := preload("res://test/support/fixtures/sprite_stream.gd")


func test_scaled_fallback_alpha_pick_keeps_display_time_and_ground_receiver() -> void:
	var drawing: Node2D = auto_free(Drawing.new())
	drawing.camera = KdCanvas.new()
	drawing.camera.zoom(0.5, Vector2(270, 600), true)
	drawing.state = drawing.camera.frame(540, 1200, 0)
	drawing.state.second = 123.25
	drawing.second = -1
	var image := Image.create(2, 2, false, Image.FORMAT_RGBA8)
	image.fill(Color.TRANSPARENT)
	image.set_pixel(1, 0, Color.WHITE)
	var middle: Vector2 = drawing.camera.project(-2, -2, 0)
	var rect := Rect2(middle - Vector2(4, 4), Vector2(8, 8))
	drawing._draws.append(
		{
			"id": -7,
			"rect": rect,
			"foot": middle,
			"source_scale": 4.0,
			"source": Rect2(0, 0, 2, 2),
			"image": image,
			"record": {"point": Vector3(-2, -2, 0), "surface": 77}
		}
	)
	var floor_point: Dictionary = drawing.terrain.walk(-2, -2)
	var floor_id: int = floor_point.surface
	var polygon: Polygon2D = auto_free(Polygon2D.new())
	polygon.polygon = PackedVector2Array(
		[
			middle - Vector2(50, 50),
			middle + Vector2(50, -50),
			middle + Vector2(50, 50),
			middle + Vector2(-50, 50)
		]
	)
	drawing._surface_nodes[floor_id] = polygon
	drawing.ordering = [floor_id, -7]
	var opaque: Dictionary = drawing.pick(rect.position + Vector2(6, 2))
	assert_bool(opaque.get("found", false)).is_true()
	assert_int(opaque.get("id", 0)).is_equal(-7)
	assert_int(opaque.get("surface", 0)).is_equal(77)
	assert_float(opaque.get("second", -1.0)).is_equal(123.25)
	var transparent: Dictionary = drawing.pick(rect.position + Vector2(2, 6))
	assert_bool(transparent.get("found", false)).is_true()
	assert_int(transparent.get("id", 0)).is_not_equal(-7)
	assert_int(transparent.get("surface", 0)).is_equal(floor_id)
	assert_float(transparent.get("second", -1.0)).is_equal(123.25)
	drawing.scene_name = "shelter"
	var entrance: Dictionary = drawing.pick(drawing.camera.project(3, 1, 0.6))
	assert_bool(entrance.get("entrance", false)).is_true()
	assert_float(entrance.get("second", -1.0)).is_equal(123.25)


func test_target_resize_failure_preserves_old_allocation_across_cancel_and_world_swap() -> void:
	var stream: Node = auto_free(Stream.new())
	add_child(stream)
	stream.set_process(false)
	var identity := {
		"world_id": "targets",
		"data_hash": "data",
		"look_hash": "look",
		"renderer": "test",
		"format_version": 1,
		"epoch": 1
	}
	assert_bool(stream.begin(identity, {"target_bytes": 800}).ok).is_true()
	var page := Targets.new()
	page._stream = stream
	assert_bool(page._reserve_targets(Vector2i(4, 4))).is_true()
	assert_int(stream.ledger.status().target_bytes).is_equal(192)
	assert_bool(page._reserve_targets(Vector2i(5, 5))).is_true()
	assert_int(stream.ledger.status().target_bytes).is_equal(492)
	var kept: int = page._target_ticket
	assert_bool(page._reserve_targets(Vector2i(7, 7))).is_false()
	assert_int(page._target_ticket).is_equal(kept)
	assert_vector(page._target_size).is_equal(Vector2i(5, 5))
	identity.world_id = "next-world"
	identity.epoch = 2
	assert_bool(stream.begin(identity, {"target_bytes": 800}).ok).is_true()
	assert_int(stream.ledger.status().target_bytes).is_equal(492)
	for frame in 3:
		await get_tree().process_frame
	stream._process(0)
	assert_int(stream.ledger.status().target_bytes).is_equal(300)
	assert_bool(page._reserve_targets(Vector2i(6, 6))).is_true()
	assert_int(stream.ledger.status().target_bytes).is_equal(732)
	stream.retire_allocation(page._target_ticket)
	assert_int(stream.ledger.status().target_bytes).is_equal(732)
	for frame in 3:
		await get_tree().process_frame
	stream._process(0)
	assert_int(stream.ledger.status().target_bytes).is_equal(0)
	assert_int(stream.ledger.status().allocation_count).is_equal(0)
