## Camp alpha front door: actual saved people, a bounded patch, and thumb-sized controls.
extends Control

const Drawing := preload("res://camp/drawing.gd")
const Labels := preload("res://camp/labels.gd")
const Sizing := preload("res://ui/sizing.gd")
var draws_world := true
var navigation_height := 0.0
var root := Worlds.ROOT
var world := KdWorld.new()
var camera := KdCanvas.new()
var worlds: KdWorlds
var world_id := ""
var opened := {}
var people: Array = []
var selected_id := 0
var drawing: Node2D
var state := {}
var frozen := false
var _viewport: SubViewport
var _picture: TextureRect
var _native: Control
var _area: Control
var _dock: PanelContainer
var _card: Label
var _summary: Label
var _pause: Button
var _speed: OptionButton
var _origin := Vector2i.ZERO
var _touches := {}
var _paused_before_background := false
var _message := ""
var _check_code := ""
var _message_until := 0
var _show_supplies := false
var _labels: Node2D
var _hit_radius := 24.0
var _dock_bounds := Rect2()


func _ready() -> void:
	theme = load("res://ui/theme.tres")
	worlds = Worlds.at(root)
	var camps := worlds.list().filter(
		func(w: Dictionary) -> bool: return w.get("kind") == "camp_alpha"
	)
	world_id = worlds.current()
	if not camps.any(func(w: Dictionary) -> bool: return w.id == world_id):
		world_id = str(camps[0].id) if not camps.is_empty() else worlds.make_camp("Camp alpha", 17)
	worlds.set_current(world_id)
	GameData.load_into(world)
	opened = world.open_camp(
		ProjectSettings.globalize_path(root.path_join(world_id)),
		17,
		str(ProjectSettings.get_setting("application/config/version"))
	)
	if opened.has("problem"):
		draws_world = false
		var failure := Label.new()
		failure.text = (
			"Camp could not open. Menu → Saved camps to choose another.\n" + str(opened.problem)
		)
		failure.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		add_child(failure)
		failure.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		set_process(false)
		return
	if opened.made:
		world.begin_at(KdWorld.morning())
	world.pause()
	var origin: PackedInt64Array = world.camp_at(0)
	_origin = Vector2i(origin[0], origin[1])
	camera.set_world(world, origin[0], origin[1])
	camera.focus(0, 0)
	camera.zoom(0.5, Vector2(540, 1200), true)
	_build()
	_resize()
	get_viewport().size_changed.connect(_resize)
	_refresh_records()
	if not frozen:
		world.play()


func _build() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 20
	add_child(layer)
	_native = Control.new()
	_native.theme = theme
	_native.mouse_filter = Control.MOUSE_FILTER_IGNORE
	layer.add_child(_native)
	_area = Control.new()
	_area.clip_contents = true
	_native.add_child(_area)
	_viewport = SubViewport.new()
	_viewport.disable_3d = true
	_viewport.canvas_item_default_texture_filter = (
		Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
	)
	_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	add_child(_viewport)
	drawing = Drawing.new()
	drawing.camera = camera
	drawing.origin = _origin
	_viewport.add_child(drawing)
	_picture = TextureRect.new()
	_picture.texture = _viewport.get_texture()
	_picture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_picture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_picture.mouse_filter = Control.MOUSE_FILTER_STOP
	_picture.gui_input.connect(_world_input)
	_area.add_child(_picture)
	_labels = Labels.new()
	_labels.drawing = drawing
	_area.add_child(_labels)
	_dock = PanelContainer.new()
	var style := StyleBoxFlat.new()
	style.bg_color = Color("172b24")
	_dock.add_theme_stylebox_override("panel", style)
	_native.add_child(_dock)
	var column := VBoxContainer.new()
	_dock.add_child(column)
	_summary = Label.new()
	_summary.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	column.add_child(_summary)
	_card = Label.new()
	_card.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_card.size_flags_vertical = Control.SIZE_EXPAND_FILL
	column.add_child(_card)
	var row := HBoxContainer.new()
	column.add_child(row)
	_pause = Button.new()
	_pause.text = "Pause"
	_pause.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_pause.pressed.connect(toggle_pause)
	row.add_child(_pause)
	_speed = OptionButton.new()
	for words: String in ["Real time", "1 min / sec", "1 hour / sec"]:
		_speed.add_item(words)
	_speed.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_speed.item_selected.connect(choose_speed)
	row.add_child(_speed)
	world.set_speed(1)


func _refresh_records() -> void:
	people = world.people()
	drawing.people = people
	drawing.supplies = world.camp_alpha()
	drawing.selected = selected_id
	_summary.text = (
		"Camp alpha · %d idle adults\n%s · %s"
		% [people.size(), world.date_text(), world.time_text()]
	)
	var person := selected_person()
	if _show_supplies:
		var supplies := world.camp_alpha()
		_card.text = (
			(
				"Initial supplies · %.0f L water\n%.0f kg food plants · %.0f kg stone\n"
				+ "%.0f kg fallen wood · natural shelter"
			)
			% [
				supplies.water_ml / 1000.0,
				supplies.food_mg / 1000000.0,
				supplies.stone_mg / 1000000.0,
				supplies.wood_mg / 1000000.0
			]
		)
	elif person.is_empty():
		_card.text = (
			"Tap a person to meet them.\nDrag to look · pinch to zoom\n"
			+ "Idle adults; daily needs come next."
		)
	else:
		_card.text = (
			"%s · %d years at start\n%s · %.2f m east, %.2f m north"
			% [
				person.name,
				person.age_at_start,
				person.activity,
				float(person.east_cm - _origin.x) / 100,
				float(person.north_cm - _origin.y) / 100
			]
		)
	if world.counters().get("save_failed", false):
		_summary.text = "Camp could not save. Play stopped. Free space, then reopen."
		_summary.add_theme_color_override("font_color", Palette.WARN)
		_pause.disabled = true
		_speed.disabled = true
	elif not _check_code.is_empty():
		_summary.text = "Self-check " + _check_code + ". Menu → Developer tools → Device check."
	elif Time.get_ticks_msec() < _message_until:
		_summary.text = _message
	_pause.text = "Play" if world.is_paused() else "Pause"


func selected_person() -> Dictionary:
	for person: Dictionary in people:
		if int(person.id) == selected_id:
			return person
	return {}


func select_person(id: int) -> void:
	if people.any(func(person: Dictionary) -> bool: return int(person.id) == id):
		selected_id = id
		_show_supplies = false
		_refresh_records()


func tap(at: Vector2) -> void:
	var id: int = drawing.pick(
		camera.from_screen(at), _hit_radius / float(state.scale) / float(state.live_scale)
	)
	if id != 0:
		select_person(id)


func toggle_pause() -> void:
	if world.is_paused():
		world.play()
	else:
		world.pause()
	_refresh_records()


func choose_speed(index: int) -> void:
	world.set_speed([1.0, 60.0, 3600.0][index])
	_speed.select(index)


func show_supplies() -> void:
	if not is_instance_valid(drawing):
		return
	_show_supplies = not _show_supplies
	_refresh_records()


func save_camp() -> void:
	if not is_instance_valid(drawing):
		return
	world.save_now()
	if world.counters().get("save_failed", false):
		world.pause()
		_message_until = 0
	else:
		_message = "Camp saved. People and supplies are kept."
		_message_until = Time.get_ticks_msec() + 4000
	_refresh_records()


func show_check(code: String) -> void:
	_check_code = code
	if is_instance_valid(drawing):
		_refresh_records()


func _process(delta: float) -> void:
	if not is_instance_valid(drawing):
		return
	_dock.size = _dock_bounds.size
	_dock.position = _dock_bounds.position
	world.frame()
	state = camera.frame(int(_area.size.x), int(_area.size.y), delta)
	_viewport.size = Vector2i(state.size)
	_picture.size = Vector2(state.size) * float(state.scale) * float(state.live_scale)
	_picture.position = state.offset
	drawing.state = state
	_refresh_records()
	drawing.rebuild()
	_labels.state = state
	_labels.queue_redraw()


func _resize() -> void:
	if _native == null:
		return
	var window := get_viewport().get_visible_rect().size
	var safe := Rect2(Vector2.ZERO, window)
	if OS.get_name() == "Android":
		var area := DisplayServer.get_display_safe_area()
		var android_safe := Rect2(
			Vector2(area.position - DisplayServer.window_get_position()), Vector2(area.size)
		)
		if android_safe.has_area():
			safe = android_safe.intersection(safe)
	safe.position.y += navigation_height
	safe.size.y -= navigation_height
	layout(window, safe)


func layout(window: Vector2, safe: Rect2) -> void:
	_native.size = window
	var density := clampf(minf(window.x, window.y) / 450, 1, 3)
	camera.set_pixel_scale(2 if minf(window.x, window.y) >= 1080 else 1)
	_hit_radius = 24 * density
	_labels.density = density
	var popup := _speed.get_popup()
	popup.add_theme_font_size_override("font_size", Sizing.font_size(16, density))
	popup.add_theme_constant_override("v_separation", roundi(32 * density))
	var gap := 12 * density
	var landscape := window.x > window.y
	var width := minf(360 * density, safe.size.x * 0.38) if landscape else safe.size.x
	var height := safe.size.y if landscape else 260 * density
	_dock.position = safe.position + Vector2(safe.size.x - width, safe.size.y - height)
	_dock.size = Vector2(width, height)
	_dock_bounds = Rect2(_dock.position, Vector2(width, height))
	_area.position = safe.position
	_area.size = (
		Vector2(safe.size.x - width, safe.size.y)
		if landscape
		else Vector2(safe.size.x, safe.size.y - height)
	)
	var style: StyleBoxFlat = _dock.get_theme_stylebox("panel")
	for edge: String in ["left", "right", "top", "bottom"]:
		style.set("content_margin_" + edge, gap)
	Sizing.page(_dock, density)
	_summary.add_theme_font_size_override("font_size", Sizing.font_size(18, density))
	_card.add_theme_font_size_override("font_size", Sizing.font_size(20, density))


func _world_input(event: InputEvent) -> void:
	var at := Vector2.ZERO
	var operation := -1
	var finger := 0
	if event is InputEventScreenTouch:
		at = event.position + _picture.position
		finger = event.index
		operation = 0 if event.pressed else 2
	elif event is InputEventScreenDrag:
		at = event.position + _picture.position
		finger = event.index
		operation = 1
	elif event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT:
		at = event.position + _picture.position
		operation = 0 if event.pressed else 2
	elif event is InputEventMouseMotion and event.button_mask & MOUSE_BUTTON_MASK_LEFT:
		at = event.position + _picture.position
		operation = 1
	if operation < 0:
		return
	var seconds := Time.get_ticks_usec() / 1000000.0
	if operation == 0:
		_touches[finger] = {"position": at, "alone": _touches.is_empty(), "moved": false}
		if _touches.size() > 1:
			for touch: Dictionary in _touches.values():
				touch.alone = false
	elif operation == 1 and _touches.has(finger):
		if at.distance_to(_touches[finger].position) > 12:
			_touches[finger].moved = true
	elif operation == 2 and _touches.has(finger):
		var touch: Dictionary = _touches[finger]
		if touch.alone and not touch.moved:
			tap(at)
		_touches.erase(finger)
	camera.touch(operation, finger, at, seconds)
	_picture.accept_event()


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_PAUSED:
		_paused_before_background = world.is_paused()
		world.pause()
		world.save_now()
	elif what == NOTIFICATION_APPLICATION_RESUMED and not _paused_before_background:
		world.play()


func _exit_tree() -> void:
	world.save_now()
