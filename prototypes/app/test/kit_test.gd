## P3's kit: shapes built from the catalogue in both their materials, the figure's movements and
## face, the hut in two materials with wear, the model sheet and its fires, the figures' steps, and
## the smoke and firelight under a roof.
extends GdUnitTestSuite

const KIT := preload("res://kit/kit.gd")
const KIT_SHAPES := preload("res://kit/kit_shapes.gd")
const SMOKE_PATH := preload("res://look/smoke_path.gd")
const MOVEMENTS := ["walk", "carry", "knap", "scrape", "rest"]


func _screen() -> Control:
	var screen := Control.new()
	screen.set_script(KIT)
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.size = Vector2(400, 890)
	return auto_free(screen)


func _catalogue() -> Dictionary:
	return JSON.parse_string(FileAccess.get_file_as_string(KIT.CATALOGUE))


## The painter's material rows, by name.
func _rows() -> Dictionary:
	var scene: Node3D = (load(KIT.SCENE) as PackedScene).instantiate()
	var rows: Dictionary = (scene.get_meta("painter") as Dictionary).rows
	scene.free()
	return rows


## Each vertex's material row, step bias (128 for none), pattern and flags, four bytes a vertex.
func _attrs(mesh: ArrayMesh) -> PackedByteArray:
	return mesh.surface_get_arrays(0)[Mesh.ARRAY_CUSTOM0]


## How many vertices have the byte at offset (0 row, 1 bias, 2 pattern) equal to value.
func _count(attrs: PackedByteArray, offset: int, value: int) -> int:
	var n := 0
	for i in range(offset, attrs.size(), 4):
		n += 1 if attrs[i] == value else 0
	return n


# checks: PRE-46
func test_every_shape_builds_in_both_its_materials() -> void:
	var rows := _rows()
	var catalogue := _catalogue()
	var count := 0
	for group in ["shapes", "plants", "animals"]:
		for e: Dictionary in catalogue[group]:
			var materials: Array = e.materials
			assert_int(materials.size()).is_equal(2)
			for which in 2:
				var attrs := _attrs(KIT_SHAPES.build(e, which, rows))
				var row: int = rows[materials[which]]
				(
					assert_int(_count(attrs, 0, row))
					. override_failure_message("%s in %s" % [e.name, materials[which]])
					. is_greater(0)
				)
			count += 1
	assert_int(count).is_greater_equal(13)


# checks: PRE-27, PRE-44
func test_the_figure_moves_in_five_movements_with_a_face() -> void:
	var rows := _rows()
	var catalogue := _catalogue()
	var names := []
	for move: Dictionary in catalogue.movements:
		names.append(move.name)
		var poses: Array = move.poses
		assert_int(poses.size()).is_greater_equal(2)
		var first: ArrayMesh = KIT_SHAPES.figure(rows, catalogue.figure, poses[0])
		var second: ArrayMesh = KIT_SHAPES.figure(rows, catalogue.figure, poses[1])
		# eyes and a mouth: the look's face pattern on the head
		assert_int(_count(_attrs(first), 2, 7)).is_greater(0)
		var a: PackedVector3Array = first.surface_get_arrays(0)[Mesh.ARRAY_VERTEX]
		var b: PackedVector3Array = second.surface_get_arrays(0)[Mesh.ARRAY_VERTEX]
		assert_bool(a == b).override_failure_message("%s's poses differ" % move.name).is_false()
	assert_array(names).is_equal(MOVEMENTS)


# checks: PRE-42, PRE-43
func test_the_hut_stands_in_birch_bark_and_in_reed_worn_in_patches() -> void:
	var rows := _rows()
	var hut: Dictionary = {}
	for e: Dictionary in _catalogue().shapes:
		if e.name == "hut":
			hut = e
	assert_array(hut.materials).is_equal(["birch", "reed"])
	# one shape, its cover flagged to take each copy's own material, pattern and wear; its doorway not
	var attrs := _attrs(KIT_SHAPES.build(hut, 0, rows))
	var cover := 0
	for i in range(0, attrs.size(), 4):
		var takes := (attrs[i + 3] & int(KIT_SHAPES.Shape.COPY)) != 0
		cover += 1 if takes else 0
		assert_bool(takes).is_equal(attrs[i] == rows.birch)
	assert_int(cover).is_greater(0)
	# on the sheet, two copies of it: birch bark and reed, with their patterns, bark and thatch, and
	# their wear, each in its own places
	var screen := _screen()
	await await_idle_frame()
	var copies: Array = (screen.get("copies") as Dictionary).hut
	assert_int(copies.size()).is_equal(2)
	var birch: Color = copies[0]
	var reed: Color = copies[1]
	assert_int(roundi(birch.r * 255.0)).is_equal(int(rows.birch))
	assert_int(roundi(reed.r * 255.0)).is_equal(int(rows.reed))
	assert_int(roundi(birch.g * 255.0)).is_equal(3)
	assert_int(roundi(reed.g * 255.0)).is_equal(6)
	assert_float(birch.b).is_equal_approx(0.3, 1e-3)
	assert_float(reed.b).is_equal_approx(0.3, 1e-3)
	assert_float(birch.a).is_greater(0.0)
	assert_float(reed.a).is_not_equal(birch.a)


# checks: PRE-46, PRE-30
func test_the_sheet_shows_every_shape_twice_with_three_fires() -> void:
	var screen := _screen()
	await await_idle_frame()
	var catalogue := _catalogue()
	var placed: Dictionary = screen.get("placed")
	for group in ["shapes", "plants", "animals"]:
		for e: Dictionary in catalogue[group]:
			assert_int((placed.get(e.name, []) as Array).size()).is_equal(2)
	for name: String in MOVEMENTS:
		assert_int((placed.get(name, []) as Array).size()).is_equal(1)
	assert_int((screen.get("_fires") as Array).size()).is_equal(3)
	assert_int((screen.get("_smokes") as Array).size()).is_equal(3)


# checks: PRE-44
func test_the_figures_step_about_ten_times_a_second() -> void:
	var screen := _screen()
	await await_idle_frame()
	screen.set_process(false)
	screen.set("_step_clock", 0.0)
	var steps: int = screen.get("_steps")
	screen.call("_process", 0.05)
	assert_int(screen.get("_steps")).is_equal(steps)
	screen.call("_process", 0.06)
	assert_int(screen.get("_steps")).is_equal(steps + 1)
	# the walk holds each pose for two steps; its outline data and the fires' heights move with it
	var walk: Dictionary = (screen.get("_figures") as Array)[0]
	var poses: Array = walk.poses
	screen.set("_steps", 1)
	for pose in [1, 1, 2, 2, 3, 3, 0]:
		screen.call("_step")
		for node: MeshInstance3D in walk.nodes:
			assert_object(node.mesh).is_same(poses[pose])


# checks: PRE-30
func test_smoke_flows_out_from_under_a_roof_and_its_fire_lights_beneath_it() -> void:
	var screen := _screen()
	await await_idle_frame()
	var scene: Node3D = (screen.get("_art") as SubViewport).get_child(0)
	var fire: Vector3 = (screen.get("_fires") as Array)[1][0]
	var ridge := 1.6
	# under the lean-to's roof the smoke moves out toward its open side before it rises past the
	# ridge, where smoke rising freely would be still over its fire
	var out := false
	for n: Vector4 in SMOKE_PATH.find(fire, 0.55, scene):
		if n.y < ridge and n.x - fire.x > 0.3:
			out = true
	assert_bool(out).is_true()
	# the roof is a slab with an underside above the ground, which the fires' heights from below
	# hold, so the fire's light passes beneath it; a tent has none, so firelight stops at the tent
	var rows := _rows()
	var shapes := {}
	for e: Dictionary in _catalogue().shapes:
		shapes[e.name] = e
	for name: String in ["lean-to", "tent"]:
		var e: Dictionary = shapes[name]
		var mesh := KIT_SHAPES.build(e, 0, rows)
		var arrays := mesh.surface_get_arrays(0)
		var points: PackedVector3Array = arrays[Mesh.ARRAY_VERTEX]
		var normals: PackedVector3Array = arrays[Mesh.ARRAY_NORMAL]
		var attrs: PackedByteArray = arrays[Mesh.ARRAY_CUSTOM0]
		var row: int = rows[(e.materials as Array)[0]]
		# faces of the cover itself, not its poles, turned down above the ground
		var under := 0
		for i in points.size():
			if attrs[i * 4] == row and normals[i].y < -0.3 and points[i].y > 0.3:
				under += 1
		if name == "lean-to":
			assert_int(under).is_greater(0)
		else:
			assert_int(under).is_equal(0)
