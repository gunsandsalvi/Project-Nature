## P7's screen and its extension: "New world" made through the app at the small size the tests use
## would need settings the screen does not take, so the extension is checked whole, once, and the
## line for the chat from given numbers.
extends GdUnitTestSuite

const WORLDGEN := preload("res://worldgen/worldgen.gd")


# checks: WLD-11, WLD-10
func test_the_line_for_the_chat() -> void:
	var made := {
		"made": 20,
		"qualified": 18,
		"best": 4,
		"candidates_seconds": 4.5,
		"best_seconds": 12.0,
		"plates": 14.2,
		"erosion": 28.0,
		"climate": 7.3,
		"life": 1.2,
		"score": 5.4,
	}
	assert_str(WORLDGEN.line("P7 α0.5a Pixel", made, 9.9, "0123456789abcdef", [])).is_equal(
		(
			"P7 α0.5a Pixel | three worlds in 16.5 s | 20 candidates in 4.5 s, 18 qualified"
			+ " | best 4 at full size in 12.0 s | settling 10 years in 9.9 s"
			+ " | plates 14.2 erosion 28.0 climate 7.3 life 1.2 scoring 5.4 | 4 threads"
			+ " | digest 0123456789abcdef"
		)
	)


# checks: WLD-11
func test_the_app_has_the_generator() -> void:
	assert_bool(ClassDB.class_exists("WorldGen")).is_true()
	var gen: RefCounted = ClassDB.instantiate("WorldGen")
	assert_int(gen.map_width()).is_equal(512)
	assert_int(gen.map_height()).is_equal(256)
