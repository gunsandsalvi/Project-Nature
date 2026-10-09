## Camp alpha front door: actual saved people, a bounded patch, and thumb-sized controls.
extends Control

const Words := preload("res://camp/words.gd")
const Drawing := preload("res://camp/drawing.gd")
const Labels := preload("res://camp/labels.gd")
const Sizing := preload("res://ui/sizing.gd")
var draws_world := true
var navigation_height := 0.0
var root := Worlds.ROOT
var world := KdWorld.new()
var device: Object = KdDevice.new()
var thermal := {}
var camera := KdCanvas.new()
var worlds: KdWorlds
var world_id := ""
var opened := {}
var people: Array = []
var selected_id := 0
var selected_item_id := 0
var items: Array = []
var drawing: Node2D
var state := {}
var frozen := false
var _viewport: SubViewport
var _picture: TextureRect
var _native: Control
var _area: Control
var _dock: PanelContainer
var _details := false
var _more: Button
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
var _dream_button: Button
var _dreams: PanelContainer
var _heat_every := 2.0
var _heat_wait := 0.0
var _item_second := -1
var _item_observer := -1


func _ready() -> void:
	theme = load("res://ui/theme.tres")
	worlds = Worlds.at(root)
	var camps := worlds.list().filter(
		func(w: Dictionary) -> bool: return w.get("kind") == "camp_alpha"
	)
	world_id = worlds.current()
	if not camps.any(func(w: Dictionary) -> bool: return w.id == world_id):
		world_id = (
			str(camps[0].id)
			if not camps.is_empty()
			else worlds.make_discovery("Discovery camp", 17)
		)
	worlds.set_current(world_id)
	GameData.load_into(world)
	var heat := world.entry("tuning/heat", "base:heat")
	_heat_every = float(heat.get("reading", 2))
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
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	column.add_child(scroll)
	_card = Label.new()
	_card.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_card.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_card.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.add_child(_card)
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
	_more = Button.new()
	_more.text = "Details"
	_more.pressed.connect(
		func() -> void:
			_details = not _details
			_resize()
			_refresh_records()
	)
	row.add_child(_more)
	_dream_button = Button.new()
	_dream_button.text = "Dream…"
	_dream_button.pressed.connect(func() -> void: open_dream(selected_id))
	column.add_child(_dream_button)
	_dreams = preload("res://camp/dreams.gd").new()
	_dreams.camp = self
	_native.add_child(_dreams)
	world.set_speed(1)


func _refresh_records(counters: Dictionary = {}) -> void:
	people = world.people()
	if _item_second != int(world.screen_time()) or _item_observer != selected_id:
		items = world.items(selected_id)
		_item_second = int(world.screen_time())
		_item_observer = selected_id
	drawing.items = items
	drawing.selected_item = selected_item_id
	drawing.people = people
	drawing.supplies = world.camp_alpha()
	drawing.selected = selected_id
	_summary.text = (
		"Camp · %d people\n%s · %s" % [people.size(), world.date_text(), world.time_text()]
	)
	var person := selected_person()
	if _show_supplies:
		var supplies := world.camp_alpha()
		_card.text = (
			(
				"Available · %.1f L water\n%.1f kg berries · %.0f kg stone\n"
				+ "%.0f kg fallen wood · natural shelter"
			)
			% [
				supplies.water_ml / 1000.0,
				supplies.food_mg / 1000000.0,
				supplies.stone_mg / 1000000.0,
				supplies.wood_mg / 1000000.0
			]
		)
		if supplies.get("discovery", false):
			_card.text += (
				"\n%.1f kg finite food inputs. Raw and prepared portions are counted once."
				% (float(supplies.finite_food_mg) / 1000000)
			)
		if _details and supplies.has("root_water_ml"):
			_card.text += (
				(
					"\nSpring input left: %.1f L.\nStand root water: %.1f L."
					+ "\nSeason's fruit growth left: %.1f kg.\nGrown: %.1f kg; gathered: %.1f kg."
					+ "\nSupplies checked at %s.\nSpring and fruit inputs are finite."
				)
				% [
					float(supplies.upstream_ml) / 1000,
					float(supplies.root_water_ml) / 1000,
					float(supplies.crop_budget_mg) / 1000000,
					float(supplies.food_grown_mg) / 1000000,
					float(supplies.food_taken_mg) / 1000000,
					Words.when(int(supplies.settled_frontier), int(world.screen_time()))
				]
			)
	elif selected_item_id != 0:
		var selected := selected_item()
		_card.text = (
			Words.item(selected, people, _details, int(world.screen_time()))
			if not selected.is_empty()
			else "This portion has been used."
		)
		for pile: Array in drawing.item_piles.values():
			if pile.has(selected_item_id) and pile.size() > 1:
				_card.text += "\n%d similar portions nearby · tap again to inspect." % pile.size()
				break
	elif person.is_empty():
		_card.text = (
			"Tap a person or a thing.\nHold a person for a dream.\nDrag to look · pinch to zoom\n"
			+ "They choose from needs and remembered supplies."
		)
	else:
		_card.text = person_words(person, _details)
		var rect: Rect2 = drawing.drawn.get(selected_id, Rect2())
		if drawing.drawn.values().count(rect) > 1:
			_card.text += "\nOthers here · tap again to select them."
	_more.text = "Back" if _details else "Details"
	_more.disabled = (
		(person.is_empty() and selected_item_id == 0 and not _show_supplies) or _dreams.visible
	)
	_dream_button.disabled = person.is_empty() or selected_item_id != 0 or _dreams.visible
	_pause.disabled = _dreams.visible
	_speed.disabled = _dreams.visible

	if counters.is_empty():
		counters = world.counters()
	var warning := (
		Worlds
		. space_warning(int(counters.get("free_mb", -1)), int(counters.get("warn_below_mb", 0)))
		. replace("Open Worlds", "Menu → Saved camps")
	)
	_summary.remove_theme_color_override("font_color")
	if counters.get("save_failed", false):
		_summary.text = "Camp could not save. Play stopped. Free space, then reopen."
		_summary.add_theme_color_override("font_color", Palette.WARN)
		_pause.disabled = true
		_speed.disabled = true
	elif not warning.is_empty():
		_summary.text += "\n" + warning
		_summary.add_theme_color_override("font_color", Palette.WARN)
	elif not _check_code.is_empty():
		_summary.text = "Self-check " + _check_code + ". Menu → Device check."
	elif Time.get_ticks_msec() < _message_until:
		_summary.text = _message
	_pause.text = "Play" if world.is_paused() else "Pause"


func person_words(person: Dictionary, details: bool) -> String:
	var words := Words.person(person, details, int(world.screen_time()))
	if details:
		words += Words.knowledge(world.knowledge(int(person.id)), people, int(world.screen_time()))
	return words


func selected_person() -> Dictionary:
	for person: Dictionary in people:
		if int(person.id) == selected_id:
			return person
	return {}


func select_person(id: int) -> void:
	if people.any(func(person: Dictionary) -> bool: return int(person.id) == id):
		selected_id = id
		selected_item_id = 0
		_show_supplies = false
		_refresh_records()


func selected_item() -> Dictionary:
	for item: Dictionary in items:
		if int(item.id) == selected_item_id:
			return item
	return {}


func tap(at: Vector2) -> void:
	var exact: int = drawing.pick(camera.from_screen(at), 0, selected_id)
	if exact != 0:
		select_person(exact)
		return
	var item_id: int = drawing.pick_item(camera.from_screen(at), selected_item_id)
	if item_id != 0:
		selected_item_id = item_id
		_show_supplies = false
		_refresh_records()
		return
	var id: int = drawing.pick(
		camera.from_screen(at), _hit_radius / float(state.scale) / float(state.live_scale)
	)
	if id != 0:
		select_person(id)


func open_dream(id: int, ring: bool = false) -> void:
	if is_instance_valid(_dreams) and not _dreams.visible:
		_dreams.open_person(id, ring)


func show_dream_records() -> void:
	if is_instance_valid(_dreams) and not _dreams.visible:
		_dreams.records()


func toggle_pause() -> void:
	if _dreams.visible:
		return
	if world.is_paused():
		world.play()
	else:
		world.pause()
	_refresh_records()


func choose_speed(index: int) -> void:
	if is_instance_valid(_dreams) and _dreams.visible:
		return
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
	_read_heat(delta)
	world.frame()
	for finger: int in _touches.keys():
		var touch: Dictionary = _touches[finger]
		if touch.alone and not touch.moved and Time.get_ticks_msec() - int(touch.started) >= 550:
			touch.moved = true
			var id := int(touch.person)
			if id != 0:
				camera.touch(2, finger, touch.position, Time.get_ticks_usec() / 1000000.0)
				open_dream(id, true)
				_touches.clear()
	state = camera.frame(int(_area.size.x), int(_area.size.y), delta)
	_viewport.size = Vector2i(state.size)
	_picture.size = Vector2(state.size) * float(state.scale) * float(state.live_scale)
	_picture.position = state.offset
	drawing.state = state
	_refresh_records()
	drawing.rebuild()
	_labels.state = state
	_labels.queue_redraw()


func _read_heat(delta: float) -> void:
	_heat_wait -= delta
	if _heat_wait > 0.0:
		return
	_heat_wait = _heat_every
	thermal = device.thermal()
	if thermal.has("light"):
		world.set_heat_light(float(thermal.light))
	if thermal.get("available", false) and not is_nan(float(thermal.forecast_10s)):
		world.heat_reading(float(thermal.forecast_10s))


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
	var old_area := _area.get_rect()
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
	var width := (
		minf((420 if _details else 360) * density, safe.size.x * (0.46 if _details else 0.38))
		if landscape
		else safe.size.x
	)
	var height := (
		safe.size.y
		if landscape
		else minf(440 * density, safe.size.y * 0.60) if _details else 336 * density
	)
	_dock.position = safe.position + Vector2(safe.size.x - width, safe.size.y - height)
	_dock.size = Vector2(width, height)
	_dock_bounds = Rect2(_dock.position, Vector2(width, height))
	_area.position = safe.position
	_area.size = (
		Vector2(safe.size.x - width, safe.size.y)
		if landscape
		else Vector2(safe.size.x, safe.size.y - height)
	)
	if _area.get_rect() != old_area:
		_cancel_touches()
	var style: StyleBoxFlat = _dock.get_theme_stylebox("panel")
	for edge: String in ["left", "right", "top", "bottom"]:
		style.set("content_margin_" + edge, gap)
	Sizing.page(_dock, density)
	_summary.add_theme_font_size_override("font_size", Sizing.font_size(18, density))
	_card.add_theme_font_size_override("font_size", Sizing.font_size(16, density))
	_dreams.layout(window, safe)


func _world_input(event: InputEvent) -> void:
	# Finger zero already drives this gesture; Android also sends its emulated mouse.
	if event is InputEventMouse and event.device == InputEvent.DEVICE_ID_EMULATION:
		return
	if _dreams.visible:
		return
	var at := Vector2.ZERO
	var operation := -1
	var finger := 0
	if event is InputEventScreenTouch:
		if event.canceled:
			_cancel_touches()
			_picture.accept_event()
			return
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
		_touches[finger] = {
			"position": at,
			"alone": _touches.is_empty(),
			"moved": false,
			"started": Time.get_ticks_msec(),
			"person": _person_at(at)
		}
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


func _cancel_touches() -> void:
	for finger: int in _touches:
		camera.touch(2, finger, _touches[finger].position, Time.get_ticks_usec() / 1000000.0)
	_touches.clear()


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_PAUSED:
		_cancel_touches()
		_paused_before_background = world.is_paused()
		world.pause()
		world.save_now()
	elif what == NOTIFICATION_APPLICATION_RESUMED and not _paused_before_background:
		world.play()


func _exit_tree() -> void:
	world.save_now()


func _person_at(at: Vector2) -> int:
	var local := camera.from_screen(at)
	var exact: int = drawing.pick(local)
	if exact != 0:
		if drawing.drawn.get(selected_id) == drawing.drawn.get(exact):
			return selected_id
		return exact
	if drawing.pick_item(local) != 0:
		return 0
	return drawing.pick(local, _hit_radius / float(state.scale) / float(state.live_scale))
