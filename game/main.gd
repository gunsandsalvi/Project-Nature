## Implements A3.8 PLT-02 PLT-06 PRE-31: readable navigation and composed example pages.
## Every alpha still opens on its self-check; diagnostics live behind the menu.
extends Control

const Sizing := preload("res://ui/sizing.gd")
const PAGES := {
	"Check": preload("res://pages/check.gd"),
	"Examples": preload("res://pages/examples.gd"),
	"Time": preload("res://pages/time.gd"),
	"Crowd": preload("res://pages/crowd.gd"),
	"Worlds": preload("res://pages/worlds.gd"),
	"Catalogues": preload("res://pages/catalogues.gd"),
	"Reports": preload("res://pages/reports.gd"),
	"Bench": preload("res://pages/bench.gd"),
	"Fixtures": preload("res://pages/fixtures.gd"),
	"Terrain": preload("res://pages/terrain.gd"),
}
const BACKGROUND := Palette.GROUND
const TEXT := Palette.TEXT
var _ground: ColorRect
var _content: Control
var _page_name := ""
var _header: PanelContainer
var _menu: PanelContainer
var _scrim: ColorRect
var _menu_scroll: ScrollContainer
var _navigation: HBoxContainer
var _menu_button: Button
var _page: Control
var _ui: Control
var _layout_pending := false


func _ready() -> void:
	theme = load("res://ui/theme.tres")
	var font := load("res://ui/fonts/kindling-ui-16.fnt") as FontFile
	font.fixed_size_scale_mode = TextServer.FIXED_SIZE_SCALE_INTEGER_ONLY
	font.allow_system_fallback = false
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	get_tree().node_added.connect(_node_added)
	_build()
	get_viewport().size_changed.connect(_layout)
	open_page("Check")
	for arg in OS.get_cmdline_user_args():
		if PAGES.has(arg):
			open_page(arg)
	_set_frame_cap()


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_RESUMED:
		_set_frame_cap()
	elif what == NOTIFICATION_WM_GO_BACK_REQUEST:
		_go_back()


func page_name() -> String:
	return _page_name


func open_page(page: String) -> void:
	if not PAGES.has(page):
		return
	_menu.hide()
	_scrim.hide()
	_menu_button.set_pressed_no_signal(false)
	if page == _page_name:
		return
	if is_instance_valid(_page):
		_page.free()
	_page = PAGES[page].new()
	_page_name = page
	if page == "Examples":
		_page.shell_header_height = _header.size.y
	_content.add_child(_page)
	_ground.visible = not _page.get("draws_world")
	for button: Button in _navigation.get_children():
		button.set_pressed_no_signal(button.text == page)
	_layout()


func _build() -> void:
	_ground = ColorRect.new()
	_ground.color = BACKGROUND
	_ground.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(_ground)
	_content = Control.new()
	add_child(_content)
	var layer := CanvasLayer.new()
	layer.layer = 40
	add_child(layer)
	_ui = Control.new()
	_ui.mouse_filter = Control.MOUSE_FILTER_IGNORE
	layer.add_child(_ui)
	_header = _panel()
	_ui.add_child(_header)
	var row := HBoxContainer.new()
	_header.add_child(row)
	var title := Label.new()
	title.text = "Kindling"
	title.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	row.add_child(title)
	_navigation = HBoxContainer.new()
	row.add_child(_navigation)
	for name: String in ["Examples", "Worlds", "Menu"]:
		var button := Button.new()
		button.text = name
		button.toggle_mode = true
		_navigation.add_child(button)
		if name == "Menu":
			_menu_button = button
			button.pressed.connect(_toggle_menu)
		else:
			button.pressed.connect(open_page.bind(name))
	_scrim = ColorRect.new()
	_scrim.color = Color("17241e")
	_scrim.hide()
	_ui.add_child(_scrim)
	_scrim.gui_input.connect(
		func(event: InputEvent) -> void:
			if event is InputEventMouseButton and event.pressed:
				_toggle_menu()
	)
	_menu = _panel()
	_ui.add_child(_menu)
	_menu.hide()
	_menu_scroll = ScrollContainer.new()
	_menu_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_menu.add_child(_menu_scroll)
	var column := VBoxContainer.new()
	column.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_menu_scroll.add_child(column)
	var groups := {
		"World tools": {"Time": "Time", "Catalogues": "Catalogues", "Reports": "Reports"},
		"Checks": {"Check": "Check", "Crowd": "People test", "Bench": "Rendering test"},
		"Diagnostics": {"Fixtures": "Art inspector", "Terrain": "Terrain inspector"}
	}
	for heading: String in groups:
		var label := Label.new()
		label.text = heading
		column.add_child(label)
		for route: String in groups[heading]:
			var button := Button.new()
			button.text = groups[heading][route]
			button.pressed.connect(open_page.bind(route))
			column.add_child(button)
	_layout()


func _panel() -> PanelContainer:
	var panel := PanelContainer.new()
	var style := StyleBoxFlat.new()
	style.bg_color = Color("17241e")
	panel.add_theme_stylebox_override("panel", style)
	return panel


func _toggle_menu() -> void:
	_menu.visible = not _menu.visible
	_scrim.visible = _menu.visible
	_menu_button.set_pressed_no_signal(_menu.visible)


func _safe_area() -> Rect2:
	var window := Rect2(Vector2.ZERO, get_viewport().get_visible_rect().size)
	if OS.get_name() != "Android":
		return window
	var safe := DisplayServer.get_display_safe_area()
	var area := Rect2(
		Vector2(safe.position - DisplayServer.window_get_position()), Vector2(safe.size)
	)
	return area.intersection(window) if area.has_area() else window


func _layout() -> void:
	if _header == null:
		return
	var safe := _safe_area()
	var scale_ui := clampf(minf(safe.size.x, safe.size.y) / 450.0, 1.0, 3.0)
	var gap := 8.0 * scale_ui
	var header_height := 64.0 * scale_ui
	_ui.size = get_viewport().get_visible_rect().size
	_header.position = safe.position
	_header.size = Vector2(safe.size.x, header_height)
	for panel: PanelContainer in [_header, _menu]:
		var style: StyleBoxFlat = panel.get_theme_stylebox("panel")
		for edge: String in ["left", "right", "top", "bottom"]:
			style.set("content_margin_" + edge, gap)
	for node: Node in _ui.find_children("*", "Control", true, false):
		if node is Button:
			node.custom_minimum_size = Vector2(48, 48) * scale_ui
			node.add_theme_font_size_override("font_size", Sizing.font_size(16, scale_ui))
		elif node is Label:
			node.add_theme_font_size_override("font_size", Sizing.font_size(18, scale_ui))
		if node is BoxContainer:
			node.add_theme_constant_override("separation", roundi(gap))
	_scrim.position = safe.position + Vector2(0, header_height)
	_scrim.size = Vector2(safe.size.x, safe.size.y - header_height)
	var menu_width := (
		safe.size.x if safe.size.y > safe.size.x else minf(safe.size.x, 320.0 * scale_ui)
	)
	_menu.position = safe.position + Vector2(safe.size.x - menu_width, header_height)
	_menu.size = Vector2(menu_width, safe.size.y - header_height)
	_content.position = safe.position + Vector2(gap, header_height + gap)
	_content.size = Vector2(safe.size.x - gap * 2, safe.size.y - header_height - gap * 2)
	if is_instance_valid(_page):
		if _page_name == "Examples":
			_page.shell_header_height = header_height
			_page._resize()
		elif _page.get("draws_world"):
			_page.navigation_height = header_height
			_page._resize()
		if not _page.get("draws_world"):
			_page.scale = Vector2.ONE
			_page.size = _content.size
			Sizing.page(_page, scale_ui)
		else:
			_page.size = _content.size


func _set_frame_cap() -> void:
	await get_tree().process_frame
	await get_tree().process_frame
	Engine.max_fps = 60


func _go_back() -> void:
	if _menu.visible:
		_menu.hide()
		_scrim.hide()
		_menu_button.set_pressed_no_signal(false)
	elif _page_name != "Check":
		open_page("Check")
	elif OS.get_name() == "Android" and Engine.has_singleton("AndroidRuntime"):
		Engine.get_singleton("AndroidRuntime").getActivity().moveTaskToBack(true)


func _node_added(node: Node) -> void:
	if (
		not _layout_pending
		and node is Control
		and is_instance_valid(_page)
		and _page.is_ancestor_of(node)
	):
		_layout_pending = true
		_deferred_layout.call_deferred()


func _deferred_layout() -> void:
	_layout_pending = false
	_layout()
