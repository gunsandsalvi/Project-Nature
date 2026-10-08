## Implements PRE-01, PRE-02, PRE-03, PRE-22, PRE-31, PRE-33, PLT-02, TIM-17, RES-05.
## T2.7a.2/4: a pixel viewport, physical UI, saved runs and labelled fixture inspection.
extends Control

const Drawing := preload("res://fixtures/drawing.gd")
var drawing_script: Script = Drawing
var draws_world := true
var world := KdWorld.new()
var camera := KdCanvas.new()
var frozen := false
var save_folder := "user://fixtures-local"
var animation_preview := true
var pass_name := "colour"
var dusk := false
var drawing: Node2D
var state: Dictionary = {}
var _preview_second := 0.0
var _selected_label: Label
var _view_mode: OptionButton
var _controls: VBoxContainer
var _viewport: SubViewport
var _picture: TextureRect
var _native: Control
var _status: Label
var _sheet: TextureRect
var _description: Label
var _inspect: OptionButton
var _touches: Dictionary = {}
var _paused_before_background := false


func _ready() -> void:
	GameData.load_into(world)
	if frozen:
		world.start_crowd(KdWorld.crowd_seed(), 4)
		world.begin_at(64800 if dusk else 43200)
		world.pause()
	else:
		world.open_crowd(
			ProjectSettings.globalize_path(save_folder),
			KdWorld.crowd_seed(),
			4,
			str(ProjectSettings.get_setting("application/config/version"))
		)
	var origin: PackedInt64Array = world.camp_at(0)
	camera.set_world(world, origin[0], origin[1])
	camera.focus(1.0, 0.0)
	_build()
	_resize()
	get_viewport().size_changed.connect(_resize)
	_inspect_piece(0)


func _build() -> void:
	var layer := CanvasLayer.new()
	layer.layer = 20
	add_child(layer)
	_native = Control.new()
	layer.add_child(_native)
	_viewport = SubViewport.new()
	_viewport.disable_3d = true
	_viewport.transparent_bg = false
	_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_viewport.canvas_item_default_texture_filter = (
		Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
	)
	add_child(_viewport)
	drawing = drawing_script.new()
	drawing.camera = camera
	_viewport.add_child(drawing)
	_picture = TextureRect.new()
	_picture.texture = _viewport.get_texture()
	_picture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_picture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_picture.mouse_filter = Control.MOUSE_FILTER_STOP
	_picture.gui_input.connect(_world_input)
	_native.add_child(_picture)
	var panel := PanelContainer.new()
	panel.name = "Controls"
	var background := StyleBoxFlat.new()
	background.bg_color = Color("17241eed")
	background.content_margin_left = 16
	background.content_margin_right = 16
	background.content_margin_top = 12
	background.content_margin_bottom = 12
	panel.add_theme_stylebox_override("panel", background)
	_native.add_child(panel)
	var bar := VBoxContainer.new()
	_controls = bar
	panel.add_child(bar)
	bar.position = Vector2(24, 24)
	bar.add_theme_constant_override("separation", 12)
	var title := Label.new()
	title.text = "Kindling · 37° local fixtures"
	title.add_theme_font_size_override("font_size", 32)
	bar.add_child(title)
	_description = Label.new()
	_description.add_theme_font_size_override("font_size", 22)
	_description.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	bar.add_child(_description)
	var buttons := HFlowContainer.new()
	bar.add_child(buttons)
	_button(
		buttons,
		"Back",
		func() -> void:
			if get_tree().current_scene != null:
				get_tree().current_scene.open_page("Check")
			else:
				get_tree().quit()
	)
	_button(
		buttons,
		"Pause / play",
		func() -> void:
			if world.is_paused():
				world.play()
			else:
				world.pause()
	)
	_button(buttons, "1×", func() -> void: world.set_speed(1.0))
	_button(buttons, "60×", func() -> void: world.set_speed(60.0))
	_button(buttons, "Noon / dusk", func() -> void: dusk = not dusk)
	_button(buttons, "World people", func() -> void: drawing.show_crowd = not drawing.show_crowd)
	var tools := HFlowContainer.new()
	bar.add_child(tools)
	_button(tools, "−", func() -> void: camera.zoom(0.5, _native.size / 2, true))
	_button(tools, "+", func() -> void: camera.zoom(2.0, _native.size / 2, true))
	_button(
		tools, "Turn", func() -> void: drawing.facing = (drawing.facing + 1) % drawing.facing_count
	)
	_button(
		tools,
		"4 / 8 facings",
		func() -> void: drawing.facing_count = 8 if drawing.facing_count == 4 else 4
	)
	_button(tools, "Walk / work", func() -> void: drawing.action = not drawing.action)
	_inspect = OptionButton.new()
	for name: String in ["Tree", "Shelter", "Boulder", "Ground", "Person", "Animal"]:
		_inspect.add_item(name)
	_inspect.custom_minimum_size = Vector2(160, 64)
	_inspect.item_selected.connect(_inspect_piece)
	tools.add_child(_inspect)
	var passes := OptionButton.new()
	for name: String in ["colour", "object", "material"]:
		passes.add_item(name)
	passes.item_selected.connect(func(index: int) -> void: pass_name = passes.get_item_text(index))
	tools.add_child(passes)
	_view_mode = OptionButton.new()
	for mode: String in ["colour", "alpha", "normal", "material"]:
		_view_mode.add_item(mode)
	_view_mode.item_selected.connect(
		func(index: int) -> void: drawing.channel = _view_mode.get_item_text(index)
	)
	_view_mode.custom_minimum_size = Vector2(130, 64)
	tools.add_child(_view_mode)
	_selected_label = Label.new()
	_selected_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_selected_label.add_theme_font_size_override("font_size", 22)
	bar.add_child(_selected_label)
	_sheet = TextureRect.new()
	_sheet.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_sheet.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	_sheet.mouse_filter = Control.MOUSE_FILTER_STOP
	_native.add_child(_sheet)
	_status = Label.new()
	_status.add_theme_font_size_override("font_size", 24)
	_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_native.add_child(_status)


func _button(parent: Control, text: String, callback: Callable) -> void:
	var button := Button.new()
	button.text = text
	button.add_theme_font_size_override("font_size", 22)
	button.custom_minimum_size = Vector2(88, 64)
	button.pressed.connect(callback)
	parent.add_child(button)


func _resize() -> void:
	layout(Vector2(get_viewport().get_visible_rect().size), safe_area())


## Implements PLT-02: converts the display safe area to window coordinates.
func safe_area() -> Rect2:
	var window := Rect2(Vector2.ZERO, Vector2(get_viewport().get_visible_rect().size))
	if OS.get_name() != "Android":
		return window
	var safe := DisplayServer.get_display_safe_area()
	var area := Rect2(
		Vector2(safe.position - DisplayServer.window_get_position()), Vector2(safe.size)
	)
	return area.intersection(window) if area.has_area() else window


func layout(window_size: Vector2, safe: Rect2) -> void:
	_native.size = window_size
	var panel: PanelContainer = _native.get_node("Controls")
	panel.position = safe.position + Vector2(24, 24)
	var controls_width := (
		minf(640, safe.size.x - 48) if window_size.x > window_size.y else safe.size.x - 48
	)
	panel.size.x = maxf(240, controls_width)
	_controls.custom_minimum_size.x = maxf(208, controls_width - 32)
	_sheet.position = safe.position + Vector2(safe.size.x - 284, 340)
	_sheet.size = Vector2(260, maxf(100, minf(740, safe.size.y - 440)))
	_status.position = safe.position + Vector2(24, safe.size.y - 140)
	_status.size.x = maxf(240, safe.size.x - 48)


func _process(delta: float) -> void:
	if not is_instance_valid(drawing):
		return
	if not frozen:
		world.frame()
	state = camera.frame(int(_native.size.x), int(_native.size.y))
	_viewport.size = Vector2i(state.size)
	var ratio: float = state.live_scale
	var scale_pixels: float = float(state.scale) * ratio
	_picture.size = Vector2(_viewport.size) * scale_pixels
	_picture.position = state.offset
	drawing.state = state
	_preview_second += delta if not frozen else 0.0
	drawing.second = (
		world.screen_time() + _preview_second if animation_preview else world.screen_time()
	)
	drawing.dusk = dusk
	drawing.pass_name = pass_name
	drawing.rebuild()
	_selected_label.text = (
		"Selected %s · %d-facing %s diagram"
		% [str(drawing.selected), drawing.facing_count, "work" if drawing.action else "walk"]
	)
	if not drawing.atlas.problem.is_empty():
		_selected_label.text = drawing.atlas.problem
	elif drawing.channel == "alpha":
		_selected_label.text += " · source alpha"
	elif drawing.channel != "colour":
		_selected_label.text += (
			" · %s %s"
			% [
				drawing.channel,
				(
					"available"
					if not (
						drawing
						. entries[_inspect.selected]
						. get(drawing.channel + "_levels", {})
						. is_empty()
					)
					else "unavailable; showing colour"
				)
			]
		)
	var status_format := "%s · %d px/m · %d× pixels · selected %d\n"
	_status.text = status_format % [world.time_text(), state.density, state.scale, drawing.selected]
	_status.text += "DEVELOPER PLACEHOLDERS · dusk tint; material art pending"


func _inspect_piece(index: int) -> void:
	if drawing.entries.is_empty():
		_description.text = drawing.atlas.problem
		return
	var entry: Dictionary = drawing.entries[index]
	_description.text = "%s · %s" % [entry.name, entry.status]
	_sheet.texture = (
		GameData.sheet_texture("res://fixtures/" + entry.sheet) if entry.sheet != "" else null
	)


func _world_input(event: InputEvent) -> void:
	var position := Vector2.ZERO
	var finger := 0
	var operation := -1
	if event is InputEventScreenTouch:
		position = event.position + _picture.position
		finger = event.index
		operation = 0 if event.pressed else 2
	elif event is InputEventScreenDrag:
		position = event.position + _picture.position
		finger = event.index
		operation = 1
	elif event is InputEventMouseButton:
		position = event.position + _picture.position
		if (
			event.button_index == MOUSE_BUTTON_WHEEL_UP
			or event.button_index == MOUSE_BUTTON_WHEEL_DOWN
		):
			if event.pressed:
				camera.zoom(
					2.0 if event.button_index == MOUSE_BUTTON_WHEEL_UP else 0.5, position, true
				)
			return
		if event.button_index == MOUSE_BUTTON_LEFT:
			operation = 0 if event.pressed else 2
	elif event is InputEventMouseMotion and event.button_mask & MOUSE_BUTTON_MASK_LEFT:
		position = event.position + _picture.position
		operation = 1
	if operation < 0:
		return
	var seconds := Time.get_ticks_usec() / 1000000.0
	if operation == 0:
		_touches[finger] = {
			"position": position, "seconds": seconds, "alone": _touches.is_empty(), "moved": false
		}
		if _touches.size() > 1:
			for touch: Dictionary in _touches.values():
				touch.alone = false
	elif operation == 1 and _touches.has(finger):
		if position.distance_to(_touches[finger].position) >= 24:
			_touches[finger].moved = true
	elif operation == 2 and _touches.has(finger):
		var touch: Dictionary = _touches[finger]
		if (
			touch.alone
			and not touch.moved
			and position.distance_to(touch.position) < 24
			and seconds - touch.seconds < 0.3
		):
			# Undo the exact presentation, including overscan, integer residual and a live pinch scale.
			drawing.pick(camera.from_screen(position))
		_touches.erase(finger)
	camera.touch(operation, finger, position, seconds)
	_picture.accept_event()


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_PAUSED and not frozen:
		_paused_before_background = world.is_paused()
		world.pause()
		world.save_now()
	elif what == NOTIFICATION_APPLICATION_RESUMED and not frozen and not _paused_before_background:
		world.play()


func _exit_tree() -> void:
	if not frozen:
		world.save_now()
