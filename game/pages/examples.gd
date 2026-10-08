## Implements PRE-01 PRE-03 PRE-31 PRE-42 PLT-02 PLT-04 (T2.9a): composed candidate scenes.
extends "res://pages/fixtures.gd"

const TerrainDrawing := preload("res://terrain/drawing.gd")
const StreamedAtlas := preload("res://fixtures/streamed_atlas.gd")
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
	var zoom := HBoxContainer.new()
	_controls.add_child(zoom)
	for name: String in ["Close", "Camp", "Landscape"]:
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
	_status = Label.new()
	_status.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_drawer.add_child(_status)
	set_scene("Camp")
	set_light("Noon")


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
	var header_height := 64.0 * _ui_scale
	var dock_height := (232.0 if _drawer.visible else 168.0) * _ui_scale
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
			node.add_theme_font_size_override("font_size", roundi(16 * _ui_scale))
		elif node is Label:
			node.add_theme_font_size_override("font_size", roundi(16 * _ui_scale))
		if node is BoxContainer:
			node.add_theme_constant_override("separation", roundi(gap))
	_title.add_theme_font_size_override("font_size", roundi(22 * _ui_scale))


func set_scene(name: String) -> void:
	_scene = name
	_title.text = name
	drawing.scene_name = "water" if name == "River" else "flat"
	camera.focus(3, 1 if name == "Shelter" else -8)
	for button: Button in _scene_buttons:
		button.set_pressed_no_signal(button.text == name)
	if _started:
		set_zoom("Close" if name == "Shelter" else "Camp")


func set_light(name: String) -> void:
	dusk = name == "Dusk"
	for button: Button in _light_buttons:
		button.set_pressed_no_signal(button.text == name)


func set_zoom(name: String) -> void:
	var target := 64.0 if name == "Close" else 32.0 if name == "Camp" else 4.0
	if not state.is_empty():
		camera.zoom(target / float(state.density), _world_area.size / 2, true)
	for button: Button in _zoom_buttons:
		button.set_pressed_no_signal(button.text == name)


func _inspect_piece(_index: int) -> void:
	pass


func _process(delta: float) -> void:
	if not is_instance_valid(drawing):
		return
	if not frozen:
		world.frame()
	state = camera.frame(
		int(_world_area.size.x), int(_world_area.size.y), Time.get_ticks_usec() / 1000000.0
	)
	if not _started:
		_started = true
		set_zoom("Camp")
	world.set_zoom_density(state.physical_density)
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
	# Drop render references before the persistent service acknowledges retired resources.
	if is_instance_valid(drawing):
		var atlas: RefCounted = drawing.atlas
		drawing.free()
		atlas.release()
	super._exit_tree()
