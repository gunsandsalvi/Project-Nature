## The Catalogues page and the catalogue's self-check line, headless in the cloud (A17): the build's
## copy of data/ loads with no problems, the page lists every source, kind and entry, and the
## phone's digests equal the ones the build wrote.
extends GdUnitTestSuite

const CheckPage := preload("res://pages/check.gd")


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


# checks: PRE-40
func test_the_camp_font_loads() -> void:
	var page: VBoxContainer = auto_free(CheckPage.new())
	page.build()
	var lines: Array = page.lines.filter(
		func(line: Dictionary) -> bool: return line.name == "Camp font"
	)
	assert_int(lines.size()).is_equal(1)
	assert_str(lines[0].state).is_equal("ok")


# checks: MAT-13
func test_a_world_reads_an_entry_in_base_units() -> void:
	var world := KdWorld.new()
	GameData.load_into(world)
	var crowd := world.entry("tuning/crowd", "demo:crowd")
	assert_int(int(crowd["camps"])).is_equal(400)
	assert_int(int(crowd["greeting"])).is_equal(60)
	assert_dict(world.entry("marker", "demo:nobody")).is_empty()
