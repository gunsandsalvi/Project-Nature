## Implements PRE-01 PRE-03 PRE-31 PRE-42 PLT-02 PLT-04 (T2.9a): composed candidate scenes.
extends "res://pages/fixtures.gd"

const Sizing := preload("res://ui/sizing.gd")
const TerrainDrawing := preload("res://terrain/drawing.gd")
const Preferences := preload("res://fixtures/view_preferences.gd")
const StreamedAtlas := preload("res://fixtures/streamed_atlas.gd")
var shell_header_height := 0.0
var _world_area: Control
var _header: PanelContainer
var _dock: PanelContainer
var _drawer: VBoxContainer
var _title: Label
var _scene_buttons: Array[Button] = []
var _light_buttons: Array[Button] = []
var _zoom_buttons: Array[Button] = []
var _scene := "Camp"
var _ui_scale := 1.0
var _started := false
var _target_ticket := 0
var _target_size := Vector2i.ZERO
var _stream: Node
var _saved_view := {}
var _manual_rate := 0.0
var _locked := false
var _pace_choice: OptionButton
var _lock_button: Button


func _init() -> void:
	save_folder = "user://examples-local"


func _build() -> void:
	world.enable_time_requests(true)
	var layer := CanvasLayer.new()
	layer.layer = 20
	add_child(layer)
	_native = Control.new()
	layer.add_child(_native)
	_world_area = Control.new()
	_world_area.clip_contents = true
	_native.add_child(_world_area)
	_viewport = SubViewport.new()
	_viewport.size = Vector2i(2, 2)
	_viewport.disable_3d = true
	_viewport.canvas_item_default_texture_filter = (
		Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
	)
	_viewport.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	add_child(_viewport)
	drawing = TerrainDrawing.new()
	drawing.camera = camera
	drawing.candidate_view = true
	drawing.fire_enabled = false
	var atlas := StreamedAtlas.new()
	atlas.configure(world, get_tree(), ProjectSettings.globalize_path(save_folder))
	var initial: Dictionary = camera.frame(1, 1, 0)
	atlas.update(initial)
	drawing.absolute_origin = {
		"east": int(initial.footprint.origin_east_cm),
		"north": int(initial.footprint.origin_north_cm)
	}
	_stream = atlas._service
	_reserve_targets(Vector2i(2, 2))
	_stream.detaching.connect(drawing.detach_textures)
	drawing.stream_service = _stream
	drawing.atlas = atlas
	_viewport.add_child(drawing)
	_picture = TextureRect.new()
	_picture.texture = _viewport.get_texture()
	_picture.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	_picture.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	_picture.mouse_filter = Control.MOUSE_FILTER_STOP
	_picture.gui_input.connect(_world_input)
	_world_area.add_child(_picture)
	_header = _panel()
	_native.add_child(_header)
	var heading := HBoxContainer.new()
	_header.add_child(heading)
	if shell_header_height == 0.0:
		_button(heading, "Back", _back)
	_title = Label.new()
	_title.text = "Camp"
	_title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_title.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	heading.add_child(_title)
	var candidate := Label.new()
	candidate.text = "Candidate art"
	candidate.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	heading.add_child(candidate)
	_dock = _panel()
	_native.add_child(_dock)
	_controls = VBoxContainer.new()
	_dock.add_child(_controls)
	var scenes := HBoxContainer.new()
	_controls.add_child(scenes)
	for name: String in ["Camp", "River", "Shelter"]:
		_scene_buttons.append(_choice(scenes, name, set_scene.bind(name)))
	var light := HBoxContainer.new()
	_controls.add_child(light)
	for name: String in ["Noon", "Dusk"]:
		_light_buttons.append(_choice(light, name, set_light.bind(name)))
	_button(light, "Explore", _toggle_explore)
	light.get_child(-1).size_flags_horizontal = Control.SIZE_EXPAND_FILL
	var zoom := HBoxContainer.new()
	_controls.add_child(zoom)
	for name: String in ["Close", "Scene", "Landscape"]:
		_zoom_buttons.append(_choice(zoom, name, set_zoom.bind(name)))
	_drawer = VBoxContainer.new()
	_drawer.visible = false
	_controls.add_child(_drawer)
	var extras := HBoxContainer.new()
	_drawer.add_child(extras)
	_button(
		extras, "Turn sunlight", func() -> void: drawing.direction = (drawing.direction + 1) % 4
	)
	_button(extras, "Recentre", func() -> void: set_scene(_scene))
	var pace := HBoxContainer.new()
	_drawer.add_child(pace)
	_pace_choice = OptionButton.new()
	for label: String in ["Zoom pace", "Real time", "1 hour / minute", "8 hours / minute"]:
		_pace_choice.add_item(label)
	_pace_choice.custom_minimum_size.y = 48
	_pace_choice.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_pace_choice.item_selected.connect(_set_pace)
	pace.add_child(_pace_choice)
	_button(pace, "Lock pace", _toggle_lock)
	_lock_button = pace.get_child(-1)
	_lock_button.toggle_mode = true
	_status = Label.new()
	_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_drawer.add_child(_status)
	set_scene("Camp")
	set_light("Noon")
	if not frozen:
		_saved_view = Preferences.read(_preferences_path())


func _panel() -> PanelContainer:
	var panel := PanelContainer.new()
	var style := StyleBoxFlat.new()
	style.bg_color = Color("17241e")
	for edge: String in ["left", "right", "top", "bottom"]:
		style.set("content_margin_" + edge, 12)
	panel.add_theme_stylebox_override("panel", style)
	return panel


func _choice(parent: Control, text: String, callback: Callable) -> Button:
	_button(parent, text, callback)
	var button: Button = parent.get_child(-1)
	button.toggle_mode = true
	button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	return button


func _back() -> void:
	var shell := get_tree().current_scene
	if shell != null and shell.has_method("open_page"):
		shell.open_page("Check")


func _toggle_explore() -> void:
	_drawer.visible = not _drawer.visible
	_resize()


func layout(window_size: Vector2, safe: Rect2) -> void:
	_native.size = window_size
	_ui_scale = clampf(minf(safe.size.x, safe.size.y) / 450.0, 1.0, 3.0)
	var gap := 8.0 * _ui_scale
	var header_height := (40.0 if shell_header_height > 0.0 else 64.0) * _ui_scale
	var dock_height := (328.0 if _drawer.visible else 176.0) * _ui_scale
	_header.position = safe.position
	_header.size = Vector2(safe.size.x, header_height)
	if window_size.x <= window_size.y:
		_dock.position = safe.position + Vector2(0, safe.size.y - dock_height)
		_dock.size = Vector2(safe.size.x, dock_height)
		_world_area.position = safe.position + Vector2(0, header_height)
		_world_area.size = Vector2(safe.size.x, maxf(1, safe.size.y - header_height - dock_height))
	else:
		var width := minf(safe.size.x * 0.36, 320.0 * _ui_scale)
		_dock.position = safe.position + Vector2(safe.size.x - width, header_height)
		_dock.size = Vector2(width, safe.size.y - header_height)
		_world_area.position = safe.position + Vector2(0, header_height)
		_world_area.size = Vector2(safe.size.x - width, safe.size.y - header_height)
	for panel: PanelContainer in [_header, _dock]:
		var style: StyleBoxFlat = panel.get_theme_stylebox("panel")
		for edge: String in ["left", "right", "top", "bottom"]:
			style.set("content_margin_" + edge, gap)
	for node: Node in _native.find_children("*", "Control", true, false):
		if node is Button:
			node.custom_minimum_size = Vector2(48, 48) * _ui_scale
			node.add_theme_font_size_override("font_size", Sizing.font_size(16, _ui_scale))
		elif node is Label:
			node.add_theme_font_size_override("font_size", Sizing.font_size(16, _ui_scale))
		if node is BoxContainer:
			node.add_theme_constant_override("separation", roundi(gap))
	_title.add_theme_font_size_override("font_size", Sizing.font_size(22, _ui_scale))


func set_scene(name: String) -> void:
	drawing.atlas.ground_assets = PackedStringArray(
		(
			["meadow", "meadow_v2", "meadow_v3", "bank_gravel", "river_bed"]
			if name == "River"
			else ["meadow", "meadow_v2", "meadow_v3"]
		)
	)
	_scene = name
	_title.text = name
	drawing.scene_name = "water" if name == "River" else "flat"
	camera.focus(3 if name == "Shelter" else -1, 4 if name == "Shelter" else 2)
	for button: Button in _scene_buttons:
		button.set_pressed_no_signal(button.text == name)
	if _started:
		set_zoom("Close" if name == "Shelter" else "Scene")


func set_light(name: String) -> void:
	dusk = name == "Dusk"
	for button: Button in _light_buttons:
		button.set_pressed_no_signal(button.text == name)


func set_zoom(name: String) -> void:
	var target := 64.0 if name == "Close" else 32.0 if name == "Scene" else 4.0
	if not state.is_empty():
		var current: Dictionary = camera.frame(int(_world_area.size.x), int(_world_area.size.y), 0)
		camera.zoom(
			target / (float(current.density) * float(current.live_scale)),
			_world_area.size / 2,
			true
		)
	for button: Button in _zoom_buttons:
		button.set_pressed_no_signal(button.text == name)


func _inspect_piece(_index: int) -> void:
	pass


func _process(delta: float) -> void:
	if not is_instance_valid(drawing):
		return
	if not frozen:
		world.frame()
	state = camera.frame(int(_world_area.size.x), int(_world_area.size.y), delta)
	if not _started:
		_started = true
		set_zoom("Scene")
		_restore_view()
	world.set_zoom_density(float(state.density) * float(state.live_scale))
	if not _reserve_targets(Vector2i(state.size)):
		_status.text = "Preparing the view…"
		return
	_viewport.size = Vector2i(state.size)
	_picture.size = Vector2(_viewport.size) * float(state.scale) * float(state.live_scale)
	_picture.position = state.offset
	drawing.state = state
	drawing.atlas.update(state)
	_preview_second += delta if not frozen else 0.0
	drawing.second = world.screen_time() + _preview_second
	drawing.hour = "dusk" if dusk else "noon"
	drawing.rebuild()
	_status.text = (
		drawing.atlas.problem
		if not drawing.atlas.problem.is_empty()
		else "Drag to explore · pinch to zoom"
	)


func _exit_tree() -> void:
	_save_view()
	# Drop render references before the persistent service acknowledges retired resources.
	if is_instance_valid(drawing):
		var atlas: RefCounted = drawing.atlas
		drawing.free()
		atlas.release()
	if _stream != null:
		_stream.retire_allocation(_target_ticket)
	super._exit_tree()


func safe_area() -> Rect2:
	var safe := super.safe_area()
	safe.position.y += shell_header_height
	safe.size.y = maxf(1.0, safe.size.y - shell_header_height)
	return safe


func _reserve_targets(target_size: Vector2i) -> bool:
	if target_size == _target_size:
		return true
	# Three RGBA8 targets include retained water buffers; backend overhead is separate.
	var allocation: Dictionary = _stream.reserve_allocation(
		{"category": "targets", "target": target_size.x * target_size.y * 4 * 3}
	)
	if not allocation.ok:
		return false
	_stream.retire_allocation(_target_ticket)
	_target_ticket = allocation.token
	_target_size = target_size
	return true


func _preferences_path() -> String:
	return "user://view-state/" + save_folder.sha256_text() + ".cfg"


func _save_view() -> void:
	if frozen or state.is_empty() or not is_instance_valid(drawing):
		return
	var focus: Vector2 = camera.ground(_world_area.size / 2.0, 0)
	Preferences.write(
		_preferences_path(),
		{
			"east": focus.x,
			"north": focus.y,
			"density": float(state.density),
			"scene": _scene,
			"light": "Dusk" if dusk else "Noon",
			"selected": drawing.selected,
			"origin_east_cm": int(state.footprint.origin_east_cm),
			"origin_north_cm": int(state.footprint.origin_north_cm),
			"raster_origin_east_cm": int(state.raster_origin_east_cm),
			"raster_origin_north_cm": int(state.raster_origin_north_cm),
			"manual_rate": _manual_rate,
			"locked": _locked
		}
	)


func _restore_view() -> void:
	if _saved_view.is_empty():
		return
	set_scene(_saved_view.scene)
	set_light(_saved_view.light)
	if not camera.restore_origin(
		int(_saved_view.origin_east_cm),
		int(_saved_view.origin_north_cm),
		int(_saved_view.raster_origin_east_cm),
		int(_saved_view.raster_origin_north_cm)
	):
		_saved_view.clear()
		return
	camera.focus(_saved_view.east, _saved_view.north)
	var current: Dictionary = camera.frame(int(_world_area.size.x), int(_world_area.size.y), 0)
	camera.zoom(
		_saved_view.density / (float(current.density) * float(current.live_scale)),
		_world_area.size / 2,
		true
	)
	drawing.selected = _saved_view.selected
	_manual_rate = _saved_view.manual_rate
	_locked = _saved_view.locked
	if _manual_rate > 0:
		world.set_manual_rate(_manual_rate)
	world.set_speed_lock(_locked)
	_lock_button.set_pressed_no_signal(_locked)
	var rates := [0.0, 1.0, 60.0, 480.0]
	_pace_choice.select(maxi(0, rates.find(_manual_rate)))
	_saved_view.clear()


func _set_pace(index: int) -> void:
	_manual_rate = [0.0, 1.0, 60.0, 480.0][index]
	if index == 0:
		world.clear_manual_rate()
		_locked = false
		world.set_speed_lock(false)
		_lock_button.set_pressed_no_signal(false)
	else:
		world.set_manual_rate(_manual_rate)


func _toggle_lock() -> void:
	_locked = not _locked
	world.set_speed_lock(_locked)
	_lock_button.set_pressed_no_signal(_locked)


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_PAUSED:
		_save_view()
	super._notification(what)
