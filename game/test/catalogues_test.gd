## The Catalogues page and the catalogue's self-check line, headless in the cloud (A17): the build's
## copy of data/ loads with no problems, the page lists every source, kind and entry, and the
## phone's digests equal the ones the build wrote.
extends GdUnitTestSuite

const CataloguesPage := preload("res://pages/catalogues.gd")
const CheckPage := preload("res://pages/check.gd")


# checks: MAT-13 MAT-14
func test_the_page_lists_every_source_kind_and_entry() -> void:
	var page: VBoxContainer = auto_free(CataloguesPage.new())
	add_child(page)
	await await_idle_frame()
	assert_array(page.loaded["problems"] as Array).is_empty()
	var text := "\n".join(page.shown)
	for wanted: String in [
		"art, version 1",
		"base, version 2",
		"demo, version 1",
		"marker (2 entries)",
		"tuning/crowd (1 entry)",
		"tuning/heat (1 entry)",
		"tuning/saves (1 entry)",
		"tuning/time (1 entry)",
		"demo:walker, from demo/marker/walker.toml",
		"speed = 1400 mm/s",
	]:
		assert_str(text).contains(wanted)


# checks: MAT-13
func test_the_catalogues_are_the_same_as_the_build() -> void:
	var page: VBoxContainer = auto_free(CheckPage.new())
	page.build()
	var lines: Array = page.lines.filter(
		func(line: Dictionary) -> bool: return line["name"] == "Catalogues"
	)
	assert_int(lines.size()).is_equal(1)
	assert_str(lines[0]["state"]).override_failure_message(str(lines[0])).is_equal("ok")
	# the page says what loading the build's files gives: every source, kind and entry; and the art
	# lane's textures and models, an entry each however many there are, are among the entries
	var loaded := GameData.load_into(KdWorld.new())
	var records := Array(GameData.paths(GameData.build())).filter(
		func(path: String) -> bool: return path.ends_with("/record.toml")
	)
	assert_int(GameData.entry_count(loaded)).is_greater_equal(records.size())
	assert_str(lines[0]["value"]).contains(
		(
			"%d sources, %d kinds, %d entries"
			% [loaded["sources"].size(), loaded["kinds"].size(), GameData.entry_count(loaded)]
		)
	)


# checks: PRE-20
func test_the_textures_are_the_same_as_the_build_and_load_within_their_lines() -> void:
	var page: VBoxContainer = auto_free(CheckPage.new())
	page.build()
	var lines: Array = page.lines.filter(
		func(line: Dictionary) -> bool: return line["name"] == "Textures"
	)
	assert_int(lines.size()).is_equal(1)
	assert_str(lines[0]["state"]).override_failure_message(str(lines[0])).is_equal("ok")
	var listed: Array = GameData.build().get_value("textures", "files", [])
	assert_str(lines[0]["value"]).contains("%d textures" % listed.size())


# checks: MAT-13
func test_a_world_reads_an_entry_in_base_units() -> void:
	var world := KdWorld.new()
	GameData.load_into(world)
	var crowd := world.entry("tuning/crowd", "demo:crowd")
	assert_int(int(crowd["camps"])).is_equal(400)
	assert_int(int(crowd["greeting"])).is_equal(60)
	assert_dict(world.entry("marker", "demo:nobody")).is_empty()
