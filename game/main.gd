## Implements A3.8 PLT-02 PLT-06 PRE-31: readable navigation and composed example pages.
## Camp is the front door; diagnostics live behind one developer menu.
extends Control

const Sizing := preload("res://ui/sizing.gd")
const PAGES := {
	"Camp": preload("res://pages/camp.gd"),
	"CampMeasure": preload("res://pages/camp_measure.gd"),
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
var camp_root := Worlds.ROOT
var camp_frozen := false
var first_check_code := ""
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
var _developer: VBoxContainer


func _ready() -> void:
	theme = load("res://ui/theme.tres")
	var font := load("res://ui/fonts/kindling-ui-16.fnt") as FontFile
	font.fixed_size_scale_mode = TextServer.FIXED_SIZE_SCALE_INTEGER_ONLY
	font.allow_system_fallback = false
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	get_tree().node_added.connect(_node_added)
	_build()
	get_viewport().size_changed.connect(_layout)
	open_page("Camp")
	for arg in OS.get_cmdline_user_args():
		if PAGES.has(arg):
			open_page(arg)
	_set_frame_cap()
	_first_check.call_deferred()


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
	if page in ["Camp", "Worlds", "Crowd"]:
		_page.root = camp_root
	if page == "Camp":
		_page.frozen = camp_frozen
	if page == "Worlds":
		_page.new_camp_alpha = true
	_page_name = page
	if page == "Examples":
		_page.shell_header_height = _header.size.y
	_content.add_child(_page)
	if page == "Camp" and not first_check_code.is_empty():
		_page.show_check(first_check_code)
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
	_ui.theme = theme
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
	for name: String in ["Menu"]:
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
	_scrim.color = Color("17241e66")
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
	for route: String in ["Camp", "Worlds"]:
		var button := Button.new()
		button.text = "Return to camp" if route == "Camp" else "Saved camps · export / import"
		button.pressed.connect(open_page.bind(route))
		column.add_child(button)
	var save := Button.new()
	save.text = "Save camp now"
	save.pressed.connect(_save_camp)
	column.add_child(save)
	var supplies := Button.new()
	supplies.text = "Camp supplies"
	supplies.pressed.connect(_camp_supplies)
	column.add_child(supplies)
	var dreams := Button.new()
	dreams.text = "Your dreams"
	dreams.pressed.connect(_camp_dreams)
	column.add_child(dreams)
	var developer := Button.new()
	developer.text = "Developer tools"
	column.add_child(developer)
	_developer = VBoxContainer.new()
	_developer.visible = false
	column.add_child(_developer)
	developer.pressed.connect(func() -> void: _developer.visible = not _developer.visible)
	var routes := {
		"Check": "Device check",
		"Examples": "Examples",
		"Crowd": "Marker regression fixture",
		"Time": "Time test",
		"Catalogues": "Catalogues",
		"Reports": "Reports",
		"Bench": "Rendering test",
		"CampMeasure": "Camp performance test",
		"Fixtures": "Art inspector",
		"Terrain": "Terrain inspector"
	}
	for route: String in routes:
		var button := Button.new()
		button.text = routes[route]
		button.pressed.connect(open_page.bind(route))
		_developer.add_child(button)
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
		safe.size.x
		if safe.size.y > safe.size.x
		else minf(safe.size.x, (360.0 if _page_name == "Camp" else 320.0) * scale_ui)
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
	elif _page_name == "Camp" and _page._dreams.visible:
		_page._dreams.close()
	elif _page_name != "Camp":
		open_page("Camp")
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


func _save_camp() -> void:
	if _page_name != "Camp":
		open_page("Camp")
	_page.save_camp()
	_menu.hide()
	_scrim.hide()
	_menu_button.set_pressed_no_signal(false)


## A brief first-launch smoke/thread check; the full report remains in Developer tools.
func _first_check() -> void:
	var file := ConfigFile.new()
	var path := "user://first-check.cfg"
	var version := str(ProjectSettings.get_setting("application/config/version"))
	file.load(path)
	if file.get_value("check", "version", "") == version:
		return
	var build := GameData.build()
	var device := KdDevice.new()
	var expected := str(build.get_value("proof", "smoke", ""))
	var threads := device.thread_check()
	var ok: bool = (
		not expected.is_empty()
		and device.proof("smoke", 1) == expected
		and device.proof("smoke", 4) == expected
		and threads.default_in_work
		and int(threads.stack_mib) >= 8
	)
	var catalogue := PAGES.Check.new()
	catalogue._add_catalogue()
	catalogue._add_graphics()
	catalogue._add_screen()
	catalogue._add_cores(device)
	catalogue._add("Simulation threads", str(threads), "ok" if ok else "fail")
	catalogue._add("Smoke same bits", expected, "ok" if ok else "fail")
	if _page_name == "Camp" and not _page.opened.has("problem"):
		catalogue._add("Saved moment", str(_page.world.frontier()), "info")
	ok = ok and not catalogue.lines.any(func(line: Dictionary) -> bool: return line.state == "fail")
	file.set_value("check", "report", catalogue.details())
	catalogue.free()
	first_check_code = "" if ok else "START-01"
	file.set_value("check", "version", version if ok else "")
	file.save(path)
	if _page_name == "Camp":
		_page.show_check(first_check_code)


func _camp_supplies() -> void:
	if _page_name != "Camp":
		open_page("Camp")
	_page.show_supplies()
	_menu.hide()
	_scrim.hide()
	_menu_button.set_pressed_no_signal(false)


func _camp_dreams() -> void:
	if _page_name != "Camp":
		open_page("Camp")
	_page.show_dream_records()
	_menu.hide()
	_scrim.hide()
	_menu_button.set_pressed_no_signal(false)
