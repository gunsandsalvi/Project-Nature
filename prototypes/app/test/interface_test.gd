## P12 The interface (IMPLEMENTATION α0.7a): every gesture scripted on raw touches and none read as
## another (PRE-33); every control at least 48 dp and 8 dp apart, and in portrait in the bottom
## third, on your phone's screen and the full panel, both ways up, with each panel open (PRE-34,
## A15); and the pixel font drawn at whole multiples only.
extends GdUnitTestSuite

const GESTURES := preload("res://interface/gestures.gd")
const INTERFACE := preload("res://interface/interface.gd")
const PIXEL_FONT := preload("res://interface/pixel_font.gd")
const DP := 3.0  # the scripts' touches at 3 pixels a dp
const HANDLE := Rect2(150, 700, 100, 50)  # in dp


## A scripted touch: each step [ms, "down", "move" or "up", finger, where in dp], with the time
## passing between steps in frames of 16 ms; the gestures read, in order.
func _read(steps: Array) -> Array:
	var g := GESTURES.new()
	g.dp = DP
	g.handle = Rect2(HANDLE.position * DP, HANDLE.size * DP)
	var out := []
	var now := 0
	for step: Array in steps:
		var ms: int = step[0]
		while now + 16 < ms:
			now += 16
			out += g.tick(now)
		now = ms
		var at: Vector2 = step[3] * DP
		if step[1] == "move":
			out += g.move(step[2], at, ms)
		else:
			out += g.touch(step[2], at, step[1] == "down", ms)
	for t in range(now + 16, now + 1000, 16):
		out += g.tick(t)
	return out


## The kinds read, each run of the same kind once.
func _kinds(read: Array) -> Array:
	var out := []
	for e: Dictionary in read:
		if out.is_empty() or out[-1] != e.kind:
			out.append(e.kind)
	return out


## One finger from a to b over a time, in steps of 16 ms.
func _path(finger: int, a: Vector2, b: Vector2, from_ms: int, ms: int) -> Array:
	var out := [[from_ms, "down", finger, a]]
	var steps := maxi(1, ms / 16)
	for k in range(1, steps + 1):
		out.append([from_ms + k * 16, "move", finger, a.lerp(b, float(k) / steps)])
	out.append([from_ms + steps * 16 + 8, "up", finger, b])
	return out


## Two fingers moving together, each from its start to its end.
func _two(a0: Vector2, a1: Vector2, b0: Vector2, b1: Vector2, ms: int) -> Array:
	var out := [[0, "down", 0, a0], [10, "down", 1, b0]]
	var steps := ms / 16
	for k in range(1, steps + 1):
		var t := float(k) / steps
		out.append([10 + k * 16, "move", 0, a0.lerp(a1, t)])
		out.append([10 + k * 16 + 1, "move", 1, b0.lerp(b1, t)])
	out.append([20 + steps * 16, "up", 1, b1])
	out.append([30 + steps * 16, "up", 0, a1])
	return out


func _product(read: Array, kind: String) -> float:
	var p := 1.0
	for e: Dictionary in read:
		if e.kind == kind:
			p *= float(e.by)
	return p


# checks: PRE-33
func test_a_tap_a_slow_tap_and_two_taps_are_taps() -> void:
	var at := Vector2(100, 300)
	assert_array(_kinds(_read([[0, "down", 0, at], [90, "up", 0, at]]))).is_equal(["tap"])
	# a finger wandering a little, and a press let go before it is long
	var jitter := [[0, "down", 0, at], [40, "move", 0, at + Vector2(3, 2)], [120, "up", 0, at]]
	assert_array(_kinds(_read(jitter))).is_equal(["tap"])
	assert_array(_kinds(_read([[0, "down", 0, at], [420, "up", 0, at]]))).is_equal(["tap"])
	# two taps in turn, far apart or on the same spot, are two taps
	var far := [[0, "down", 0, at], [80, "up", 0, at], [200, "down", 0, at + Vector2(150, 200)]]
	far.append([280, "up", 0, at + Vector2(150, 200)])
	(
		assert_array(_read(far).map(func(e: Dictionary) -> String: return e.kind))
		. is_equal(["tap", "tap"])
	)
	var same := [[0, "down", 0, at], [80, "up", 0, at], [200, "down", 0, at], [270, "up", 0, at]]
	(
		assert_array(_read(same).map(func(e: Dictionary) -> String: return e.kind))
		. is_equal(["tap", "tap"])
	)


# checks: PRE-33
func test_a_drag_and_a_flick_move_and_nothing_else() -> void:
	var a := Vector2(100, 300)
	for b: Vector2 in [Vector2(200, 300), Vector2(100, 150), Vector2(40, 420), Vector2(170, 360)]:
		assert_array(_kinds(_read(_path(0, a, b, 0, 300)))).is_equal(["drag", "drag_end"])
	# a flick: 60 dp in 60 ms
	assert_array(_kinds(_read(_path(0, a, a + Vector2(0, -60), 0, 60)))).is_equal(
		["drag", "drag_end"]
	)


# checks: PRE-33
func test_a_long_press_opens_the_powers_and_a_move_after_it_draws() -> void:
	var at := Vector2(160, 400)
	assert_array(_kinds(_read([[0, "down", 0, at], [700, "up", 0, at]]))).is_equal(["long"])
	var drawn := [[0, "down", 0, at]]
	for k in range(1, 20):
		drawn.append([600 + k * 16, "move", 0, at + Vector2(k * 3, k * 2)])
	drawn.append([1000, "up", 0, at + Vector2(60, 40)])
	assert_array(_kinds(_read(drawn))).is_equal(["long", "draw"])


# checks: PRE-33
func test_a_double_tap_dragged_zooms_with_one_thumb() -> void:
	var at := Vector2(160, 400)
	var down := [[0, "down", 0, at], [70, "up", 0, at]]
	down += _path(0, at + Vector2(4, 3), at + Vector2(4, 120), 170, 300)
	var read := _read(down)
	assert_array(_kinds(read)).is_equal(["zoom", "zoom_end"])
	assert_float(_product(read, "zoom")).is_greater(1.5)  # down zooms in
	var up := [[0, "down", 0, at], [70, "up", 0, at]]
	up += _path(0, at, at + Vector2(0, -120), 170, 300)
	read = _read(up)
	assert_array(_kinds(read)).is_equal(["zoom", "zoom_end"])
	assert_float(_product(read, "zoom")).is_less(0.67)


# checks: PRE-33
func test_a_pinch_zooms_and_a_twist_turns_and_neither_reads_as_the_other() -> void:
	var c := Vector2(180, 400)
	# out and in, with a little turn the fingers can't help
	var out_read := _read(
		_two(c + Vector2(-30, 0), c + Vector2(-75, -6), c + Vector2(30, 0), c + Vector2(75, 6), 300)
	)
	assert_array(_kinds(out_read)).is_equal(["zoom", "zoom_end"])
	assert_float(_product(out_read, "zoom")).is_greater(2.0)
	var in_read := _read(
		_two(c + Vector2(-80, 0), c + Vector2(-30, 3), c + Vector2(80, 0), c + Vector2(30, -3), 300)
	)
	assert_array(_kinds(in_read)).is_equal(["zoom", "zoom_end"])
	assert_float(_product(in_read, "zoom")).is_less(0.5)
	# a 40° turn with the fingers a little further apart
	var r := 50.0
	var turned := deg_to_rad(40.0)
	var twist := _read(
		_two(
			c + Vector2(-r, 0),
			c - Vector2(cos(turned), sin(turned)) * r * 1.05,
			c + Vector2(r, 0),
			c + Vector2(cos(turned), sin(turned)) * r * 1.05,
			300
		)
	)
	assert_array(_kinds(twist)).is_equal(["turn", "turn_end"])
	var total := 0.0
	for e: Dictionary in twist:
		if e.kind == "turn":
			total += float(e.by)
	assert_float(total).is_equal_approx(40.0, 1.0)


# checks: PRE-33
func test_the_handle_opens_the_views_by_a_tap_or_a_swipe_up_and_a_drag_beside_it_moves() -> void:
	var on := HANDLE.get_center()
	assert_array(_kinds(_read([[0, "down", 0, on], [80, "up", 0, on]]))).is_equal(["views"])
	assert_array(_kinds(_read(_path(0, on, on + Vector2(0, -60), 0, 160)))).is_equal(["views"])
	assert_array(_kinds(_read(_path(0, on, on + Vector2(-90, 0), 0, 200)))).is_equal(
		["drag", "drag_end"]
	)


## The screen laid out for a window and dpi, with a panel open, the controls showing.
func _screen(window: Vector2i, dpi: float, open: String, dial := false) -> Control:
	var screen := Control.new()
	screen.set_script(INTERFACE)
	add_child(screen)
	screen.size = Vector2(window)
	screen.set("plan", INTERFACE.plan_for(window, dpi))
	(screen.get("_canvas") as SubViewport).size = screen.get("plan").canvas
	screen.set("panel", open)
	screen.set("dial_open", dial)
	screen.set("_touched", 1.0e9)
	screen.call("_build")
	return auto_free(screen)


# checks: PRE-34
func test_every_control_is_48_dp_and_8_dp_apart_and_in_portrait_in_the_bottom_third() -> void:
	# your phone at its screen setting, and the full panel, each way up
	for screen_dpi: Array in [
		[Vector2i(1080, 2404), 390.0],
		[Vector2i(1344, 2992), 486.0],
		[Vector2i(2404, 1080), 390.0],
		[Vector2i(2992, 1344), 486.0]
	]:
		var states := [["", false], ["", true], ["card", false], ["book", false], ["views", false]]
		for state: Array in states:
			var screen := _screen(screen_dpi[0], screen_dpi[1], state[0], state[1])
			await await_idle_frame()
			await await_idle_frame()
			var plan: Dictionary = screen.get("plan")
			var target := _target_art(plan)
			var controls: Array = screen.call("controls")
			assert_int(controls.size()).is_greater(4)
			var what := "%s %s %s" % [screen_dpi[0], state[0], state[1]]
			for c: Dictionary in controls:
				var r: Rect2 = c.rect
				(
					assert_float(minf(r.size.x, r.size.y))
					. override_failure_message("%s: %s is %s" % [what, c.action, r])
					. is_greater_equal(target - 0.01)
				)
				if plan.portrait and not String(c.action).begins_with("deeper:What"):
					(
						assert_float(r.position.y)
						. override_failure_message(
							"%s: %s at %s, above the bottom third" % [what, c.action, r]
						)
						. is_greater_equal(plan.canvas.y * 2.0 / 3.0 - 0.01)
					)
			for i in controls.size():
				for j in range(i + 1, controls.size()):
					var a: Rect2 = controls[i].rect
					var b: Rect2 = controls[j].rect
					var apart := maxf(
						maxf(b.position.x - a.end.x, a.position.x - b.end.x),
						maxf(b.position.y - a.end.y, a.position.y - b.end.y)
					)
					var said := (
						"%s: %s and %s are %.1f apart"
						% [what, controls[i].action, controls[j].action, apart]
					)
					assert_float(apart).override_failure_message(said).is_greater_equal(
						float(plan.gap) - 0.01
					)
			screen.queue_free()


## 48 dp in art pixels, on a screen's plan.
func _target_art(plan: Dictionary) -> float:
	return 48.0 * float(plan.art_dp)


# checks: PRE-35
func test_the_pixel_font_is_the_art_books_drawn_at_whole_multiples() -> void:
	var font: FontFile = PIXEL_FONT.plain()
	assert_int(font.fixed_size).is_equal(11)
	assert_int(font.fixed_size_scale_mode).is_equal(TextServer.FIXED_SIZE_SCALE_INTEGER_ONLY)
	# "Aru": A 5 wide, r 4, u 5, with a pixel between
	assert_int(PIXEL_FONT.measure("Aru")).is_equal(16)
	# every label on the card and the book at a whole multiple of the font's size
	for open: String in ["card", "book"]:
		var screen := _screen(Vector2i(1080, 2404), 390.0, open)
		await await_idle_frame()
		var node: Control = screen.get("_panel_node")
		var labels := node.find_children("*", "Label", true, false)
		assert_int(labels.size()).is_greater(10)
		for label: Label in labels:
			assert_int(label.get_theme_font_size("font_size") % 11).is_equal(0)
		screen.queue_free()
	# your phone's screen at its setting takes 3 of its pixels an art pixel, the full panel 4
	assert_int(INTERFACE.plan_for(Vector2i(1080, 2404), 390.0).scale).is_equal(3)
	assert_int(INTERFACE.plan_for(Vector2i(1344, 2992), 486.0).scale).is_equal(4)


# checks: PRE-33
func test_under_a_panel_in_portrait_the_handle_reads_no_touch() -> void:
	var screen := _screen(Vector2i(1080, 2404), 390.0, "")
	var gestures: RefCounted = screen.get("_gestures")
	assert_bool((gestures.get("handle") as Rect2).has_area()).is_true()
	screen.call("_open", "card")
	assert_bool((gestures.get("handle") as Rect2).has_area()).is_false()
	screen.call("_open", "")
	assert_bool((gestures.get("handle") as Rect2).has_area()).is_true()
