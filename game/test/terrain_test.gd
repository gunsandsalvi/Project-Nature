## checks: PRE-20 PRE-21 PRE-23 PRE-24 PRE-26 PRE-28 PRE-30 PRE-33 WLD-13 PLT-04 (T2.8a).
extends GdUnitTestSuite

const Terrain := preload("res://pages/terrain.gd")


func _page(scene: String) -> Control:
	var page := Terrain.new()
	page.frozen = true
	add_child(page)
	page.set_process(false)
	page.drawing.moving_mask_time = 0.0
	page.layout(Vector2(1080, 2400), Rect2(0, 0, 1080, 2400))
	page.set_scene(scene)
	page._process(0.0)
	return page


# checks: PRE-23 PRE-24 (T2.8a.2): a tall trunk keeps the order of its ground footprint.
func test_trunk_covers_a_rear_person_and_admits_a_front_person() -> void:
	var page := _page("flat")
	var trunk: Dictionary = {}
	for item: Dictionary in page.drawing.draw_list():
		if item.id == -12:
			trunk = item
	var piece: Dictionary = page.drawing._sprite_piece(trunk)
	var foot: Vector3 = trunk.record.point
	var foot_depth := (
		-float(page.state.height_basis) * foot.y + float(page.state.ground_basis) * foot.z
	)
	# Use the real projected trunk overlap, then put actors on either side of its footprint.
	var front: Dictionary = page.drawing._piece(
		-80, trunk.record.surface, trunk.rect, foot_depth + 0.2, false
	)
	var rear: Dictionary = page.drawing._piece(
		-81, trunk.record.surface, trunk.rect, foot_depth - 0.2, false
	)
	var order: Array = page.drawing.terrain.order([front, piece, rear])
	assert_int(order.find(-12)).is_greater(order.find(-81))
	assert_int(order.find(-80)).is_greater(order.find(-12))
	page.free()


# checks: PRE-23 PRE-30 (T2.8a.3): the light sits above a flame grounded on its receiver.
func test_fire_flame_starts_on_flat_and_sloped_ground() -> void:
	for scene: String in ["flat", "slope"]:
		var page := _page(scene)
		var light: Vector3 = page.drawing.light_record.fire
		var floor_point: Vector3 = page.drawing.terrain.walk(light.x, light.y).point
		var flame: PackedVector2Array = page.drawing._fire.polygon
		assert_vector((flame[0] + flame[2]) / 2.0).is_equal(
			page.camera.project(floor_point.x, floor_point.y, floor_point.z)
		)
		assert_float(light.z).is_equal_approx(floor_point.z + 0.6, 0.0001)
		assert_int(page.drawing.id_table["9001"].surface).is_equal(
			page.drawing.terrain.walk(light.x, light.y).surface
		)
		page.free()


# checks: PRE-24 PRE-26 (T2.8a.3): bed refraction uses the current frame's receiver ordering.
func test_prepared_bed_has_current_receiver_order_on_first_frame_and_scene_change() -> void:
	var page := _page("water")
	for next_scene: String in ["cliff", "water", ""]:
		for id: int in page.drawing._copies:
			assert_int(page.drawing._copies[id].z_index).is_equal(
				page.drawing._surface_nodes[id].z_index
			)
		if not next_scene.is_empty():
			page.set_scene(next_scene)
			page._process(0.0)
	page.free()


# checks: PRE-21 PRE-31 PLT-04: an off-screen actor keeps its shadow on visible ground.
func test_culled_person_still_casts_into_the_visible_shadow_halo() -> void:
	var page := _page("flat")
	page.camera.zoom(2.0, Vector2(540, 1200), true)
	page.dusk = true
	page.drawing.direction = 2
	page._process(0.0)
	var visible_ids := []
	for item: Dictionary in page.drawing.draw_list():
		visible_ids.append(item.id)
	assert_bool(visible_ids.has(-7)).is_false()
	assert_float(page.drawing.terrain.visibility(Vector3(-0.5, -6.5, 0), 0).sun).is_less(0.2)
	page.free()


# checks: PRE-21 PRE-31 PLT-04 TIM-17: acceleration does not increase mask upload frequency.
func test_accelerated_scene_samples_only_on_the_real_time_mask_tick() -> void:
	var page := _page("flat")
	var shadow: float = page.drawing.terrain.visibility(Vector3.ZERO, 0).sun
	assert_float(shadow).is_equal(0.0)
	page._preview_second = 3.0
	page.drawing.moving_mask_time = 0.05
	page._process(0.0)
	assert_float(page.drawing.terrain.visibility(Vector3.ZERO, 0).sun).is_equal(shadow)
	page.drawing.moving_mask_time = 0.1
	page._process(0.0)
	assert_float(page.drawing.terrain.visibility(Vector3.ZERO, 0).sun).is_greater(shadow)
	page.free()


# checks: PRE-26 PRE-31: water clipping and picks cover all submerged sprite materials.
func test_submerged_object_feet_are_clipped_from_colour_semantics_and_picking() -> void:
	var page := _page("water")
	for pass_kind: String in ["colour", "object", "material"]:
		page.pass_name = pass_kind
		page._process(0.0)
		for item: Dictionary in page.drawing.draw_list():
			(
				assert_float(
					page.drawing._lit_sprites[item.id].get_shader_parameter("water_height")
				)
				. is_equal(page.drawing._water_height)
			)
			if pass_kind != "colour":
				(
					assert_bool(
						page.drawing._sprites[item.id].get_meta("semantic").get_shader_parameter(
							"clip_water"
						)
					)
					. is_true()
				)
	page.pass_name = "colour"
	page._process(0.0)
	for item: Dictionary in page.drawing.draw_list():
		var submerged: Vector2 = item.foot - Vector2(0, 1)
		var hit: Dictionary = page.drawing.pick(submerged)
		assert_int(hit.get("id", 0)).is_not_equal(item.id)
	page.free()


# checks: PRE-23 PRE-24 PRE-33 (T2.8a.2): real height and order across chunks.
func test_slope_feet_and_foreground_crown_keep_their_surface_order() -> void:
	var page := _page("slope")
	assert_str(page.drawing.problem).is_empty()
	var fire: Vector3 = page.drawing.light_record.fire
	assert_float(fire.z).is_equal_approx(
		page.drawing.terrain.walk(fire.x, fire.y).point.z + 0.6, 0.0001
	)
	for item: Dictionary in page.drawing.draw_list():
		if item.entry.name == "person":
			var p: Vector3 = item.record.point
			assert_float(p.z).is_greater(0.0)
			assert_vector(item.foot).is_equal(page.camera.project(p.x, p.y, p.z))
	page.set_scene("cliff")
	page._process(0.0)
	assert_str(page.drawing.problem).is_empty()
	var order: Array = page.drawing.ordering
	assert_int(order.find(-11)).is_greater(order.find(-5))
	assert_int(order.size()).is_equal(
		(
			(
				page
				. drawing
				. _surface_nodes
				. values()
				. filter(func(node: Polygon2D) -> bool: return node.visible)
				. size()
			)
			+ page.drawing.draw_list().size()
			+ 1
		)
	)
	page.dusk = true
	page.drawing.direction = 1
	page._process(0.0)
	var trunk_sun: Vector3 = page.drawing._lit_sprites[-12].get_shader_parameter("sun_bands")
	assert_float(trunk_sun.x).is_less(trunk_sun.z)
	page.drawing.selected = -5
	page._process(0.0)
	assert_bool(page.drawing.faded.has(-11)).is_true()
	page.camera.zoom(0.5, Vector2(540, 1200), true)
	page._process(0.0)
	assert_bool(page.drawing._silhouettes[-7].visible).is_true()
	page.free()


# checks: PRE-21 PRE-24 PRE-28 PRE-30 WLD-13: reveal keeps physical roofs and state.
func test_roof_reveal_preserves_shadow_state_and_saved_run_digest() -> void:
	var page := _page("shelter")
	assert_str(page.drawing.problem).is_empty()
	var digest: String = page.world.digest()
	var before: PackedByteArray = page.drawing.terrain.mask(20).rgba
	assert_bool(page.drawing.faded.has(21)).is_true()
	page.drawing.selected = -5
	page.drawing.reveal = false
	page._process(0.0)
	assert_bool(page.drawing.faded.is_empty()).is_true()
	assert_object(page.drawing.terrain.mask(20).rgba).is_equal(before)
	assert_str(page.world.digest()).is_equal(digest)
	assert_bool(page.drawing.terrain.visibility(Vector3(3, 3, 0.2), 0).sun == 0.0).is_true()
	page.drawing.reveal = true
	page.toggle_cave()
	page._process(0.0)
	assert_str(page.drawing.problem).is_empty()
	assert_str(page.drawing.hour).is_equal("night")
	for record: Dictionary in page.drawing.surface_records:
		assert_int(record.kind).is_not_equal(2)
	assert_str(page.world.digest()).is_equal(digest)
	page.toggle_cave()
	page._process(0.0)
	assert_str(page.drawing.scene_name).is_equal("shelter")
	assert_int(page.drawing.selected).is_equal(-5)
	assert_vector(page.camera.ground(Vector2(540, 1200), 0.0)).is_equal(Vector2(3, 3))
	assert_str(page.world.digest()).is_equal(digest)
	page.free()


# checks: PRE-24 PRE-26 PRE-30 PLT-04: shared heights; refraction excludes actors.
func test_wading_and_prepared_water_layers_share_the_height_source() -> void:
	var page := _page("water")
	assert_str(page.drawing.problem).is_empty()
	var bed: Dictionary = page.drawing.terrain.bed(0, -6)
	assert_float(bed.point.z).is_equal_approx(-0.416, 0.0001)
	assert_float(page.drawing.terrain.bed(8, -6).point.z).is_equal_approx(-0.416, 0.0001)
	var water: Dictionary = {}
	for record: Dictionary in page.drawing.surface_records:
		if record.kind == 4:
			water = record
	assert_float(page.drawing._water_height).is_equal(water.corners[0].z)
	assert_int(page.drawing._bed.find_children("*", "Sprite2D", true, false).size()).is_equal(0)
	assert_int(page.drawing._reflections.get_child_count()).is_greater(0)
	assert_int(page.drawing.terrain.costs().mask_bytes).is_less(2 * 1024 * 1024)
	var ground_pixel: Vector2 = page.camera.project(2, -4, page.drawing._water_height)
	var hit: Dictionary = page.drawing.pick(ground_pixel)
	assert_bool(hit.get("found", false)).is_true()
	assert_int(hit.surface).is_equal(30)
	page.free()
