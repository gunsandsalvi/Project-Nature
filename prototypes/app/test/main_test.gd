## The self-check screen in portrait and in landscape, with facts as long as a phone's.
extends GdUnitTestSuite

const MAIN := preload("res://main.tscn")
## Facts of the lengths the phone gives, so the layout is tried with long names.
const LONG := {
	"version": "α0.1a",
	"phone": "Pixel 11 Pro XL",
	"android": "16",
	"screen": "1344 × 2992 px, 489 dpi, 120 Hz",
	"graphics": "Imagination Technologies PowerVR D-Series DXT-48-1536 MC1",
	"driver": "1.632.4123 (2717474843)",
	"vulkan": "1.4.303",
}


# checks: PLT-01
func test_one_column_in_portrait_two_in_landscape() -> void:
	var screen: Control = auto_free(MAIN.instantiate())
	add_child(screen)
	screen.set_anchors_preset(Control.PRESET_TOP_LEFT)
	screen.show_facts(LONG)
	for shape: Vector2 in [Vector2(400, 890), Vector2(890, 400)]:
		screen.size = shape
		await await_idle_frame()
		await await_idle_frame()
		var check: Rect2 = (
			(screen.find_child("SelfCheck", true, false) as Control).get_global_rect()
		)
		var menu: Rect2 = (screen.find_child("Menu", true, false) as Control).get_global_rect()
		var whole := Rect2(screen.global_position, shape)
		(
			assert_bool(whole.encloses(check))
			. override_failure_message("self-check off screen")
			. is_true()
		)
		assert_bool(whole.encloses(menu)).override_failure_message("menu off screen").is_true()
		if shape.y > shape.x:
			assert_float(check.end.y).is_less_equal(menu.position.y)
			assert_float(check.size.x).is_equal_approx(menu.size.x, 0.5)
			assert_float(menu.size.y).is_greater(150.0)
		else:
			assert_float(check.end.x).is_less_equal(menu.position.x)
			assert_float(check.size.x).is_equal_approx(menu.size.x, 1.0)


# checks: PLT-01
func test_the_code_for_the_chat_holds_every_fact() -> void:
	var line: String = load("res://main.gd").code(LONG)
	assert_str(line).starts_with("α0.1a | Phone Pixel 11 Pro XL | Android 16 | Screen 1344")
	assert_str(line).contains("Driver 1.632.4123 (2717474843)")
	assert_str(line).ends_with("Vulkan 1.4.303")
