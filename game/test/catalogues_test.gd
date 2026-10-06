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
		"base, version 1",
		"demo, version 1",
		"marker (2 entries)",
		"tuning/crowd (1 entry)",
		"tuning/heat (1 entry)",
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
	assert_str(lines[0]["value"]).contains("2 sources, 4 kinds, 5 entries")


# checks: MAT-13
func test_a_world_reads_an_entry_in_base_units() -> void:
	var world := KdWorld.new()
	GameData.load_into(world)
	var crowd := world.entry("tuning/crowd", "demo:crowd")
	assert_int(int(crowd["camps"])).is_equal(400)
	assert_int(int(crowd["greeting"])).is_equal(60)
	assert_dict(world.entry("marker", "demo:nobody")).is_empty()
