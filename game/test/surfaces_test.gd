## A big surface's tiles as the catalogue names them (A5.3, A5.4), headless in the cloud: a
## texture's name says which tile and version of which material it is, and a material's files come
## in the order the ladder reads them: the near tile's versions first, then the middle tile's and
## the far's.
extends GdUnitTestSuite


# checks: PRE-20 PRE-22
func test_a_texture_s_name_says_which_tile_and_which_version_of_which_material() -> void:
	assert_str(Surfaces.tile_of("meadow", "art:meadow")).is_equal("near")
	assert_str(Surfaces.tile_of("meadow", "art:meadow/v3")).is_equal("near")
	assert_str(Surfaces.tile_of("meadow", "art:meadow/middle")).is_equal("middle")
	assert_str(Surfaces.tile_of("meadow", "art:meadow/middle/v2")).is_equal("middle")
	assert_str(Surfaces.tile_of("meadow", "art:meadow/far/v4")).is_equal("far")
	# another material's, even one whose name begins with this one's
	assert_str(Surfaces.tile_of("meadow", "art:meadow_wet")).is_empty()
	assert_str(Surfaces.tile_of("meadow", "art:ash")).is_empty()
	assert_int(Surfaces.version_of("art:meadow")).is_equal(1)
	assert_int(Surfaces.version_of("art:meadow/v3")).is_equal(3)
	assert_int(Surfaces.version_of("art:meadow/far/v2")).is_equal(2)
	assert_int(Surfaces.version_of("art:meadow/middle")).is_equal(1)


# checks: PRE-20 PRE-22
func test_a_sheet_lists_names_by_material_then_tile_then_version() -> void:
	assert_str(Surfaces.material_of("art:meadow")).is_equal("meadow")
	assert_str(Surfaces.material_of("art:meadow/far/v2")).is_equal("meadow")
	var names: Array[String] = [
		"art:meadow/far/v2",
		"art:meadow/v10",
		"art:meadow/middle",
		"art:ash",
		"art:meadow/v2",
		"art:meadow",
		"art:meadow/far",
	]
	(
		assert_array(Surfaces.sorted(names))
		. is_equal(
			[
				"art:ash",
				"art:meadow",
				"art:meadow/v2",
				"art:meadow/v10",
				"art:meadow/middle",
				"art:meadow/far",
				"art:meadow/far/v2",
			]
		)
	)


# checks: PRE-20 PRE-22
func test_a_material_s_files_come_near_versions_first_then_the_middle_tile_s_then_the_far() -> void:
	var world := KdWorld.new()
	var loaded := GameData.load_into(world)
	var names := Surfaces.texture_names(loaded)
	assert_array(names).contains(["art:meadow", "art:meadow/middle", "art:meadow/far/v4"])
	var meadow := Surfaces.ladder(world, names, "meadow")
	assert_bool(meadow.has("problem")).is_false()
	var count: PackedInt32Array = meadow["count"]
	var first: PackedInt32Array = meadow["first"]
	var paths: PackedStringArray = meadow["paths"]
	# four versions of each tile, by the owner's choice of 7 October 2026 (A5.3)
	assert_array(Array(count)).is_equal([4, 4, 4])
	assert_int(paths.size()).is_equal(count[0] + count[1] + count[2])
	# the tiles start at the bands A5.3 gives them
	assert_array(Array(first)).is_equal([0, 2, 4])
	# the near tile first, its versions in order, then the middle and the far
	assert_str(paths[0]).ends_with("/meadow.kdtex")
	assert_str(paths[1]).ends_with("/meadow/v2.kdtex")
	assert_str(paths[count[0]]).ends_with("/meadow/middle.kdtex")
	assert_str(paths[count[0] + count[1]]).ends_with("/meadow/far.kdtex")
	for path: String in paths:
		assert_bool(FileAccess.file_exists(path)).is_true()
	# a material of one tile has the near tile alone, which every band reads
	var wood := Surfaces.ladder(world, names, "wood")
	assert_array(Array(wood["count"])).is_equal([1, 0, 0])
	assert_int((wood["first"] as PackedInt32Array)[1]).is_equal(Surfaces.NONE)
	# a material the catalogue has none of is a problem in words
	var none := Surfaces.ladder(world, names, "no_such_material")
	assert_str(none["problem"]).contains("no texture art:no_such_material")
