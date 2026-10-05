## P12 The interface (IMPLEMENTATION α0.7a, PRE-32, PRE-33, PRE-34, PRE-35, PLT-02, A15): a
## stand-in world, the art book's zoom stops painted upright and sideways, under the interface its
## plates show: the date, the speed and the time controls after any touch, fading; a live moment;
## the views behind the handle; Aru's card and the book of ages, each on the art book's paper or one
## of four other grounds. The screen is drawn in art pixels, four of the phone's each way on yours,
## so every letter is whole; every touch goes through one gesture reader (gestures.gd); and every
## control is at least 48 dp, 8 dp apart, in portrait in the bottom third. Pre-production code
## (research 00).
extends Control

signal closed

const Gestures := preload("res://interface/gestures.gd")
const Grounds := preload("res://interface/styles.gd")
const Panels := preload("res://interface/panels.gd")
const MAIN := preload("res://main.gd")

const ART_ACROSS := 336  # art pixels across the art book's upright plates (PRE-22)
const PHONE_DPI := 486.0  # the Pixel 11 Pro XL's, 1344 by 2992 on 6.8 inches, where none is given
const TARGET_DP := 48.0  # every control at least this each way (A15)
const GAP_DP := 8.0  # and this apart
const EDGE_DP := 24.0  # the bottom edge the phone keeps for its own gestures (PRE-33)
const FADE_AFTER := 4.0  # seconds after a touch before the controls fade (PRE-32)
const MOMENT_FOR := 10.0  # a live moment you don't tap goes after about this long (TIM-02)
const NIGHT := Color(0.086, 0.075, 0.114, 0.92)
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
## The zoom stops (PRE-03) and the speed each asks (TIM-01): name, words, game days a real second.
const STOPS := [
	["person", "real speed", 1.0 / 86400.0],
	["closecamp", "an hour a minute", 1.0 / 1440.0],
	["camp", "a day in a few minutes", 1.0 / 180.0],
	["valley", "a season a minute", 0.25],
	["region", "a few years a minute", 3.0],
	["map", "top speed", 5.0],
	["globe", "top speed", 5.0],
]
## The dial's speeds (TIM-04), the stops' own.
const DIAL := [0, 1, 2, 3, 4, 5]
const TIME := ["pause", "play", "dial", "lock", "skip"]
const SEASONS := ["spring", "summer", "autumn", "winter"]

## The ground the panels stand on, from styles.gd.
var ground_index := 0
## The zoom, from 0 at the person to 6 at the globe, and what the camera has done.
var level := 1.0
var heading := 0.0
## Game days since Year 1, spring, day 1: Year 214, spring, day 12.
var day := 12791.0
var paused := false
var dial := -1
var locked := -1
var skipping := 0.0
## The panel open: "", "card", "book", "views" or "powers"; the dial's strip, when open.
var panel := ""
var dial_open := false

var plan := {}
var _canvas: SubViewport
var _shown: TextureRect
var _world: TextureRect
var _pictures := {}
var _pan := Vector2()
var _dragging := false
var _drag_on_panel := false
var _gestures := Gestures.new()
var _clock := 0.0
var _touched := -100.0
var _moment_at := 0.0
var _notice := ""
var _notice_at := -100.0
var _trail: Array[Vector2] = []
var _handle_rect := Rect2()  # in the screen's pixels
var _overlay: Control
var _panel_node: Control
var _scroll: ScrollContainer
var _date: Label
var _speed: Label
var _controls_group: Control
var _moment: Control


## Screen pixels an art pixel, the canvas in art pixels, art pixels a dp and the insets in art
## pixels, for a screen of `window` pixels at `dpi`, its safe area's insets `insets` in art pixels.
static func plan_for(window: Vector2i, dpi: float, insets := Vector4i()) -> Dictionary:
	var scale := maxi(1, roundi(minf(window.x, window.y) / float(ART_ACROSS)))
	var canvas := Vector2i(ceili(window.x / float(scale)), ceili(window.y / float(scale)))
	var art_dp := dpi / 160.0 / scale
	return {
		"scale": scale,
		"canvas": canvas,
		"art_dp": art_dp,
		"target": ceili(TARGET_DP * art_dp),
		"gap": ceili(GAP_DP * art_dp),
		"insets": insets,
		"bottom": canvas.y - maxi(insets.w, ceili(EDGE_DP * art_dp)),
		"portrait": canvas.y > canvas.x,
	}


func _ready() -> void:
	var background := ColorRect.new()
	background.color = Color.BLACK
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(background)
	_canvas = SubViewport.new()
	_canvas.disable_3d = true
	_canvas.canvas_item_default_texture_filter = (
		Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
	)
	_canvas.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	add_child(_canvas)
	_shown = TextureRect.new()
	_shown.texture = _canvas.get_texture()
	_shown.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_shown.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_shown.stretch_mode = TextureRect.STRETCH_SCALE
	_shown.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(_shown)
	_world = TextureRect.new()
	_world.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_canvas.add_child(_world)
	for stop: Array in STOPS:
		for side: String in ["", "-land"]:
			_pictures[stop[0] + side] = load("res://interface/world/%s%s.png" % [stop[0], side])
	_from_command_line()
	resized.connect(_layout)
	_layout()


## For the cloud's pictures: "card", "book", "views", "dial" or "touch", a ground's name, a
## stop's.
func _from_command_line() -> void:
	var args := OS.get_cmdline_user_args()
	for k in Grounds.GROUNDS.size():
		if String(Grounds.GROUNDS[k].name).to_lower() in args:
			ground_index = k
	for k in STOPS.size():
		if STOPS[k][0] in args:
			level = k
	for name: String in ["card", "book", "views"]:
		if name in args:
			panel = name
	dial_open = "dial" in args
	if "touch" in args or dial_open:
		_touched = 1.0e9


func _layout() -> void:
	var window := DisplayServer.window_get_size()
	if window.x <= 0 or size.x <= 0.0:
		return
	var dpi := float(DisplayServer.screen_get_dpi())
	plan = plan_for(window, dpi if dpi >= 200.0 else PHONE_DPI)
	var canvas: Vector2i = plan.canvas
	plan.insets = MAIN.insets(Vector2(canvas))
	plan = plan_for(window, dpi if dpi >= 200.0 else PHONE_DPI, plan.insets)
	_canvas.size = canvas
	var k := window.x / size.x
	_shown.position = Vector2.ZERO
	_shown.size = Vector2(canvas) * float(plan.scale) / k
	_gestures.dp = float(plan.art_dp) * float(plan.scale)
	_build()


## Everything over the world, made anew for the screen's way up, the state kept (PLT-02).
func _build() -> void:
	if _overlay != null:
		_overlay.queue_free()
	if _panel_node != null:
		_panel_node.queue_free()
		_panel_node = null
	_overlay = Control.new()
	_overlay.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_overlay.size = Vector2(plan.canvas)
	_canvas.add_child(_overlay)
	_build_readout()
	_build_moment()
	_build_controls()
	if panel != "":
		_open(panel)
	_show_world()


func _portrait() -> bool:
	return bool(plan.portrait)


## The width the world shows across: all of it, less a panel's column in landscape.
func _world_width() -> int:
	var canvas: Vector2i = plan.canvas
	if not _portrait() and panel in ["card", "book", "views"]:
		return canvas.x - _column_width()
	return canvas.x


func _column_width() -> int:
	return 336 if panel == "book" else 300


func _show_world() -> void:
	var stop: String = STOPS[clampi(roundi(level), 0, STOPS.size() - 1)][0]
	var picture: Texture2D = _pictures[stop + ("" if _portrait() else "-land")]
	_world.texture = picture
	var canvas := Vector2(plan.canvas)
	var at := ((canvas - picture.get_size()) / 2.0).floor()
	_world.position = at + _pan.round()


# --- the date, the speed and the time controls ---------------------------------------------------


func _build_readout() -> void:
	var box := Grounds.new(Grounds.GROUNDS[1])
	box.name = "Readout"
	var inner := Panels.column(1)
	inner.position = Vector2(6, 4)
	_date = Panels.words("", INK)
	_speed = Panels.words("", FLAME)
	inner.add_child(_date)
	inner.add_child(_speed)
	box.add_child(inner)
	box.size = Vector2(196, 29)
	var insets: Vector4i = plan.insets
	if _portrait():
		box.position = Vector2(roundi((plan.canvas.x - box.size.x) / 2.0), insets.y + 6)
	else:
		box.position = Vector2(insets.x + 8, insets.y + 6)
	# the date and the speed come and go with the time controls (PRE-33)
	_controls_group = Control.new()
	_controls_group.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_controls_group.size = Vector2(plan.canvas)
	_overlay.add_child(_controls_group)
	_controls_group.add_child(box)


## The time controls (TIM-04, TIM-11) in their bar, the handle for the views below them in portrait,
## and the dial's strip when open; each a control at least 48 dp, 8 dp apart.
func _build_controls() -> void:
	var target: int = plan.target
	var gap: int = plan.gap
	var bottom: int = plan.bottom
	var width := TIME.size() * target + (TIME.size() - 1) * gap
	var left: int
	var bar_y: int
	var handle_rect: Rect2
	if _portrait():
		# the handle above the phone's own edge, the bar above it, both in the thumb's reach
		handle_rect = Rect2(
			roundi(plan.canvas.x / 2.0) - target, bottom - target, target * 2, target
		)
		left = roundi((plan.canvas.x - width) / 2.0)
		bar_y = int(handle_rect.position.y) - gap - target
	else:
		# held in two hands: the bar at the left thumb, the handle beside it under the world
		left = int(plan.insets.x) + 8
		bar_y = bottom - target
		var hx := maxi(roundi(_world_width() / 2.0) - target, left + width + gap)
		handle_rect = Rect2(hx, bottom - target, target * 2, target)
	var bar := Grounds.new(Grounds.GROUNDS[1])
	bar.position = Vector2(left - 3, bar_y - 3)
	bar.size = Vector2(width + 6, target + 6)
	_controls_group.add_child(bar)
	for k in TIME.size():
		var button := _button(
			"time:" + TIME[k], Rect2(left + k * (target + gap), bar_y, target, target)
		)
		var art := Panels.picture("time-" + TIME[k])
		art.position = ((Vector2(target, target) - art.texture.get_size()) / 2.0).floor()
		button.add_child(art)
		_controls_group.add_child(button)
	var handle := _button("views", handle_rect)
	var line := ColorRect.new()
	line.color = Color(INK, 0.8)
	line.size = Vector2(24, 2)
	line.position = Vector2(roundi((handle_rect.size.x - 24) / 2.0), handle_rect.size.y - 6)
	handle.add_child(line)
	_overlay.add_child(handle)
	_handle_rect = Rect2(
		handle_rect.position * float(plan.scale), handle_rect.size * float(plan.scale)
	)
	_gestures.handle = _handle_rect
	if dial_open:
		_build_dial(left, bar_y, width)


## The dial opened (TIM-04): each speed a control, two to a row, above the bar.
func _build_dial(left: int, bar_y: int, width: int) -> void:
	var target: int = plan.target
	var gap: int = plan.gap
	var rows := ceili(DIAL.size() / 2.0)
	var top := bar_y - gap - rows * target - (rows - 1) * gap
	var cell_w := maxi(target, int((width + 40 - gap) / 2.0))
	left = maxi(int(plan.insets.x) + 24, left)
	var back := Grounds.new(Grounds.GROUNDS[1])
	back.position = Vector2(left - 23, top - 3)
	back.size = Vector2(cell_w * 2 + gap + 6, rows * target + (rows - 1) * gap + 6)
	_controls_group.add_child(back)
	for k in DIAL.size():
		var r := Rect2(
			left - 20 + (k % 2) * (cell_w + gap), top + (k / 2) * (target + gap), cell_w, target
		)
		var cell := _button("dial:%d" % k, r)
		var label := Panels.words(STOPS[DIAL[k]][1], FLAME if dial == k else INK)
		label.size = Vector2(cell_w, target)
		label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
		label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		cell.add_child(label)
		_controls_group.add_child(cell)


## A live moment (PRE-08): the art book's first pot, where a thumb reaches it in portrait.
func _build_moment() -> void:
	var target: int = plan.target
	var gap: int = plan.gap
	_moment = _button("moment", Rect2(0, 0, 0, 0))
	var back := Grounds.new(Grounds.GROUNDS[1])
	var inner := Panels.row(6)
	inner.position = Vector2(5, 4)
	inner.add_child(Panels.picture("talk-pots"))
	var said := Panels.column(1)
	said.add_child(Panels.words("First pot fired whole", FLAME))
	said.add_child(Panels.words("Aru of the Tavu · Year 214", DIM))
	inner.add_child(said)
	back.add_child(inner)
	_moment.add_child(back)
	var w := 190
	var h := maxi(target, 30)
	if _portrait():
		var bar_y: int = plan.bottom - target - gap - target
		_moment.position = Vector2(roundi((plan.canvas.x - w) / 2.0), bar_y - gap - h - 3)
	else:
		_moment.position = Vector2(int(plan.insets.x) + 8, int(plan.insets.y) + 6 + 29 + gap)
	_moment.size = Vector2(w, h)
	back.size = Vector2(w, h)
	_overlay.add_child(_moment)


## A control: a box the size of its hit area, which the touch finds by its action.
func _button(action: String, rect: Rect2) -> Control:
	var button := Control.new()
	button.set_meta("action", action)
	button.position = rect.position
	button.size = rect.size
	button.mouse_filter = Control.MOUSE_FILTER_IGNORE
	return button


# --- the panels -----------------------------------------------------------------------------------


func _open(which: String) -> void:
	if _panel_node != null:
		_panel_node.queue_free()
		_panel_node = null
	panel = which
	_overlay.visible = true
	_gestures.handle = _handle_rect
	if which == "":
		_show_world()
		return
	var ground: Dictionary = Grounds.GROUNDS[ground_index]
	if which == "views":
		ground = Grounds.GROUNDS[1]
	var canvas: Vector2i = plan.canvas
	var insets: Vector4i = plan.insets
	var rect: Rect2
	if _portrait():
		# the views as tall as their four rows, their heading and their controls need
		var target: int = plan.target
		var gap: int = plan.gap
		var views: int = int(plan.bottom) - (8 + 11 + gap + 4 * target + 4 * gap + gap + target + 8)
		var top := {"card": int(canvas.y * 0.44), "book": 0, "views": views}
		var at: int = top.get(which, int(canvas.y * 0.5))
		if which == "powers":
			at = int(canvas.y * 0.5)
		rect = Rect2(0, at, canvas.x, canvas.y - at)
	else:
		var w := _column_width()
		rect = Rect2(canvas.x - w, 0, w, canvas.y)
	if which == "powers":
		_open_powers(rect)
		return
	var root := Control.new()
	root.position = rect.position
	root.size = rect.size
	root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var back := Grounds.new(ground)
	back.size = rect.size
	root.add_child(back)
	if ground.has("shadow"):
		# words a pixel's shadow below and right, to read over the world behind the glass
		root.theme = Theme.new()
		root.theme.set_color("font_shadow_color", "Label", ground.shadow)
		root.theme.set_constant("shadow_offset_x", "Label", 1)
		root.theme.set_constant("shadow_offset_y", "Label", 1)
	var target: int = plan.target
	var gap: int = plan.gap
	var controls: Array = []
	var page: Control
	match which:
		"card":
			page = Panels.card(ground, int(rect.size.x) - 16)
			controls = [Panels.card_controls(ground), _ground_and_close(ground, "Close")]
		"book":
			page = Panels.book(ground, int(rect.size.x) - 16)
			controls = [
				Panels.book_controls(ground), _ground_and_close(ground, "Back to the world")
			]
		"views":
			page = Panels.views(ground, int(rect.size.x) - 16)
			var last := []
			for name: String in ["Worlds", "Settings"]:
				last.append(Panels.control(name, "deeper:" + name, ground))
			last.append(Panels.control("Menu", "menu", ground))
			last.append(Panels.control("Close", "close", ground))
			controls = [last]
	# the controls' rows at the panel's foot, above the phone's own edge
	var foot: int = canvas.y - int(plan.bottom) if _portrait() else int(insets.w) + 4
	var rows_h := controls.size() * target + (controls.size() - 1) * gap
	var rows_top := int(rect.size.y) - foot - rows_h - 4
	for r in controls.size():
		var line := Panels.row(gap)
		line.position = Vector2(8, rows_top + r * (target + gap))
		for c: Control in controls[r]:
			c.custom_minimum_size = Vector2(maxf(c.custom_minimum_size.x, target), target)
			c.alignment = BoxContainer.ALIGNMENT_CENTER
			line.add_child(c)
		root.add_child(line)
	_scroll = ScrollContainer.new()
	_scroll.position = Vector2(8, 8 + (insets.y if which == "book" or not _portrait() else 0))
	_scroll.size = Vector2(rect.size.x - 16, rows_top - 4 - _scroll.position.y)
	_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_scroll.vertical_scroll_mode = ScrollContainer.SCROLL_MODE_SHOW_NEVER
	_scroll.mouse_filter = Control.MOUSE_FILTER_IGNORE
	if which == "views":
		for item: Control in page.get_children():
			if item.has_meta("action"):
				item.custom_minimum_size.y = target
		page.add_theme_constant_override("separation", gap)
	_scroll.add_child(page)
	root.add_child(_scroll)
	_panel_node = root
	_canvas.add_child(root)
	_show_world()
	# in portrait a panel stands where the time controls and the handle were: they wait until it closes
	_overlay.visible = not _portrait()
	if _portrait():
		_gestures.handle = Rect2()


## The last row: the ground to try next, and the way back to the world.
func _ground_and_close(ground: Dictionary, close: String) -> Array:
	var next: Dictionary = Grounds.GROUNDS[(ground_index + 1) % Grounds.GROUNDS.size()]
	return [
		Panels.control("Ground: %s, next %s" % [ground.name, next.name], "ground", ground),
		Panels.control(close, "close", ground),
	]


## Your powers at the spot a long press was on (GOD-10): a stand-in that names them.
func _open_powers(rect: Rect2) -> void:
	var root := Control.new()
	root.mouse_filter = Control.MOUSE_FILTER_IGNORE
	var back := Grounds.new(Grounds.GROUNDS[1])
	var w := 236
	var h := 52
	back.size = Vector2(w, h)
	root.position = Vector2(roundi((plan.canvas.x - w) / 2.0), rect.position.y)
	root.size = back.size
	root.add_child(back)
	var said := Panels.column(1)
	said.position = Vector2(6, 4)
	said.add_child(Panels.words("Your powers here (GOD-10)", FLAME))
	said.add_child(Panels.words("lightning, rain, storm, drought, flood,", INK))
	said.add_child(Panels.words("quake, dream, fortune, revelation", INK))
	said.add_child(Panels.words("drag on to draw an area", DIM))
	root.add_child(said)
	_panel_node = root
	_canvas.add_child(root)


# --- touches --------------------------------------------------------------------------------------


func _input(event: InputEvent) -> void:
	if not is_visible_in_tree():
		return
	var k := DisplayServer.window_get_size().x / maxf(size.x, 1.0)
	var now := Time.get_ticks_msec()
	var out := []
	if event is InputEventScreenTouch:
		var touch := event as InputEventScreenTouch
		out = _gestures.touch(touch.index, touch.position * k, touch.pressed, now)
	elif event is InputEventScreenDrag:
		var drag := event as InputEventScreenDrag
		out = _gestures.move(drag.index, drag.position * k, now)
	else:
		return
	get_viewport().set_input_as_handled()
	_touched = _clock
	for e: Dictionary in out:
		gesture(e)


## What a gesture does here: positions in the screen's pixels.
func gesture(e: Dictionary) -> void:
	_touched = _clock
	match String(e.kind):
		"tap":
			tap(Vector2(e.at) / float(plan.scale))
		"views":
			_open("views")
		"drag":
			var at: Vector2 = Vector2(e.at) / float(plan.scale)
			if not _dragging:
				_dragging = true
				_drag_on_panel = _on_panel(at)
			var by: Vector2 = Vector2(e.by) / float(plan.scale)
			if _drag_on_panel and _scroll != null:
				_scroll.scroll_vertical -= roundi(by.y)
			else:
				_pan += by
				_show_world()
		"drag_end":
			_dragging = false
		"zoom":
			# zooming in is towards the person; each doubling a stop (TIM-01)
			level = clampf(level - log(float(e.by)) / log(2.0), 0.0, STOPS.size() - 1.0)
			_show_world()
		"turn":
			heading = wrapf(heading + float(e.by), 0.0, 360.0)
			_say("facing %d° from north" % roundi(heading))
		"long":
			_trail.clear()
			_open("powers")
		"draw":
			_trail.append(Vector2(e.at) / float(plan.scale))
			_say("drawing an area")


## A tap at a point of the canvas: a control's action if it is on one, else the world's.
func tap(at: Vector2) -> void:
	var hit := action_at(at)
	if hit != "":
		act(hit)
		return
	if panel != "":
		if not _on_panel(at):
			_open("")
		return
	# the world: at the close stops a tap lands on the band, and opens the card of the one
	# beneath (in this stand-in, always Aru)
	if roundi(level) <= 2:
		_open("card")


func _on_panel(at: Vector2) -> bool:
	return _panel_node != null and Rect2(_panel_node.position, _panel_node.size).has_point(at)


## The action of the control under a point of the canvas, or none.
func action_at(at: Vector2) -> String:
	for c: Dictionary in controls():
		if Rect2(c.rect).has_point(at):
			return c.action
	return ""


## Every control showing, its action and its hit area in art pixels: the node's own box, grown about
## its middle to at least 48 dp each way.
func controls() -> Array:
	var out := []
	var target := float(plan.get("target", 1))
	var faded := _clock - _touched > FADE_AFTER + 0.5
	for root: Control in [_panel_node, _overlay]:
		if root == null or not is_instance_valid(root):
			continue
		for node: Node in [root] + root.find_children("*", "Control", true, false):
			var c := node as Control
			if c == null or not c.has_meta("action") or not c.is_visible_in_tree():
				continue
			if faded and _controls_group != null and _controls_group.is_ancestor_of(c):
				continue
			if c == _moment and not _moment.visible:
				continue
			var r := c.get_global_rect()
			if _scroll != null and _scroll.is_ancestor_of(c):
				var view := _scroll.get_global_rect()
				if not view.intersects(r):
					continue
			var grow := Vector2(maxf(0.0, target - r.size.x), maxf(0.0, target - r.size.y))
			out.append(
				{
					"action": c.get_meta("action"),
					"rect":
					r.grow_individual(grow.x / 2.0, grow.y / 2.0, grow.x / 2.0, grow.y / 2.0)
				}
			)
	return out


func act(action: String) -> void:
	var parts := action.split(":")
	match parts[0]:
		"time":
			_time(parts[1])
		"dial":
			dial = int(parts[1])
			locked = -1
			paused = false
			dial_open = false
			_build()
		"views":
			_open("views")
		"book":
			_open("book")
		"moment":
			level = 1.0
			_moment_at = -100.0
			_open("card")
		"ground":
			ground_index = (ground_index + 1) % Grounds.GROUNDS.size()
			_open(panel)
		"close":
			_open("")
		"menu":
			closed.emit()
		"tab":
			if parts[1] != "Ages":
				_say("%s: comes with production" % parts[1])
		"deeper":
			_say("%s: comes with production" % parts[1])


## The time controls (TIM-04, TIM-11, TIM-15): pause stops time, play hands it back to the zoom,
## the dial sets a speed that holds, the lock keeps the speed now, skip races to the next moment.
func _time(which: String) -> void:
	match which:
		"pause":
			paused = true
		"play":
			paused = false
			dial = -1
			locked = -1
		"dial":
			dial_open = not dial_open
			_build()
		"lock":
			locked = -1 if locked >= 0 else _stop_now()
		"skip":
			paused = false
			skipping = 2.0


func _stop_now() -> int:
	return dial if dial >= 0 else clampi(roundi(level), 0, STOPS.size() - 1)


func _say(text: String) -> void:
	_notice = text
	_notice_at = _clock


# --- time passing ---------------------------------------------------------------------------------


func _process(delta: float) -> void:
	_clock += delta
	for e: Dictionary in _gestures.tick(Time.get_ticks_msec()):
		gesture(e)
	var words := _speed_words()
	day += _speed_days() * delta
	if skipping > 0.0:
		skipping -= delta
		if skipping <= 0.0:
			_moment_at = _clock
	if _date != null:
		var d := int(day)
		var year := d / 60 + 1
		var season: String = SEASONS[(d % 60) / 15]
		_date.text = "Year %d, %s, day %d" % [year, season, d % 15 + 1]
		_speed.text = _notice if _clock - _notice_at < 2.5 else words
	if _controls_group != null:
		var since := _clock - _touched
		_controls_group.modulate.a = clampf(1.0 - (since - FADE_AFTER) / 0.5, 0.0, 1.0)
	if _moment != null:
		var age := _clock - _moment_at
		_moment.visible = age >= 0.0 and age < MOMENT_FOR and not dial_open and panel == ""
	if not _dragging and _pan != Vector2.ZERO:
		_pan = _pan.lerp(Vector2.ZERO, minf(1.0, delta * 10.0))
		if _pan.length() < 0.5:
			_pan = Vector2.ZERO
		_show_world()


## The speed asked, in game days a real second, by TIM-15's order: pause, skip, the dial or the
## lock, then the zoom.
func _speed_days() -> float:
	if paused:
		return 0.0
	if skipping > 0.0:
		return STOPS[5][2]
	if dial >= 0:
		return STOPS[DIAL[dial]][2]
	if locked >= 0:
		return STOPS[locked][2]
	return STOPS[clampi(roundi(level), 0, STOPS.size() - 1)][2]


func _speed_words() -> String:
	if paused:
		return "paused"
	if skipping > 0.0:
		return "skipping to the next moment"
	if dial >= 0:
		return "the dial: " + String(STOPS[DIAL[dial]][1])
	if locked >= 0:
		return "locked: " + String(STOPS[locked][1])
	return STOPS[clampi(roundi(level), 0, STOPS.size() - 1)][1]


func _unhandled_key_input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_cancel"):
		closed.emit()


func _notification(what: int) -> void:
	if what == NOTIFICATION_WM_GO_BACK_REQUEST:
		if panel != "":
			_open("")
		else:
			closed.emit()
