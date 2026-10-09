## Checks PRE-03 PRE-42 PRE-43: wide ground uses repaired far levels, bounded at one texel.
extends GdUnitTestSuite

const Atlas := preload("res://test/support/fixtures/streamed_atlas.gd")


func test_wide_ground_uses_repaired_chain_without_fractional_asset_keys() -> void:
	var atlas := Atlas.new()
	atlas._families["meadow/far"] = {"page_width": 256}
	assert_str(atlas._slot("meadow", 0.0625)).is_equal("meadow/4/6")
	assert_str(atlas._slot("meadow", pow(2.0, -12))).is_equal("meadow/4/8")
	assert_str(atlas._slot("meadow", 2.0)).is_equal("meadow/4/1")
	assert_str(atlas._slot("meadow", 32.0)).is_equal("meadow/64/1")
	atlas._families["meadow/far"].page_width = 16
	assert_str(atlas._slot("meadow", pow(2.0, -12))).is_equal("meadow/4/4")
