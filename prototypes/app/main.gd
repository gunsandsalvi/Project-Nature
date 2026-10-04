## The prototype app's home screen: the self-check, with the version, the phone, its screen and its
## graphics driver, and the menu of the prototypes, each named with the step that brings it.
## One column of panels, the menu below in portrait and beside the self-check in landscape.
## Pre-production code (research 00): its README names the items it is about.
extends Control

## The prototypes on the phone, and the page of the cloud's reports, in the plan's order:
## the title, the step that brings it, and its screen's script once it has come.
const PROTOTYPES: Array[Array] = [
	["P1 The look", "α0.2a", "res://look/look.gd"],
	["P2 A full scene", "α0.2b", "res://scene/scene.gd"],
	["P3 The kit", "α0.2c", "res://kit/kit.gd"],
	["Reports from the cloud", "α0.3a", "res://reports/reports.gd"],
	["P5 The same bits", "α0.4a"],
	["P6 A thousand minds", "α0.4b"],
	["P7 World generation", "α0.5a"],
	["P8 The zoom", "α0.5b"],
	["P12 The interface", "α0.7a"],
	["P13 The writer", "α0.7b"],
	["P14 Sound", "α0.7c"],
]
## The self-check's rows: the fact's key, and its label.
const ROWS: Array[Array] = [
	["phone", "Phone"],
	["android", "Android"],
	["screen", "Screen"],
	["graphics", "Graphics"],
	["driver", "Driver"],
	["vulkan", "Vulkan"],
]
const GAP := 12
const TARGET := 48  # the smallest height of anything you tap, in the screen's units
const INK := Color("ebe5da")
const DIM := Color("a39ca9")
const FLAME := Color("f6a33c")
const NIGHT := Color("16131d")
const PANEL := Color("221d2b")
const EDGE := Color("3a3346")

var _columns: BoxContainer
var _self_check: PanelContainer
var _self_check_scroll: ScrollContainer
var _menu: PanelContainer
var _values := {}
var _copy: Button
var _safe: MarginContainer


## What the phone says about itself. In the cloud's headless runs most of it is empty.
static func facts() -> Dictionary:
	var screen := DisplayServer.screen_get_size()
	return {
		"version": str(ProjectSettings.get_setting("application/config/version", "")),
		"phone": OS.get_model_name(),
		"android": android_version(OS.get_version()),
		"screen":
		(
			"%d × %d px, %d dpi, %s Hz"
			% [screen.x, screen.y, DisplayServer.screen_get_dpi(), refresh_rate()]
		),
		"graphics":
		(
			(
				"%s %s"
				% [
					RenderingServer.get_video_adapter_vendor(),
					RenderingServer.get_video_adapter_name()
				]
			)
			. strip_edges()
		),
		"driver": driver_version(),
		"vulkan": RenderingServer.get_video_adapter_api_version(),
	}


## The screen's refresh rate in whole hertz, or "?" where the system doesn't give it, as the
## cloud's virtual screen doesn't.
static func refresh_rate() -> String:
	var rate := DisplayServer.screen_get_refresh_rate()
	return str(roundi(rate)) if rate > 0 else "?"


## Android's version as Godot gives it, "<API level>.<build>", such as "37.16238327", shown as
## "API 37 (build 16238327)": the API level is not the release's number (37 is Android 17). A
## custom ROM's own version, which Godot gives instead, is shown as it is.
static func android_version(raw: String) -> String:
	var parts := raw.split(".", false, 1)
	if parts.size() == 2 and parts[0].is_valid_int():
		return "API %s (build %s)" % [parts[0], parts[1]]
	return raw


## The graphics driver's version. Godot gives it only inside the id of its pipeline cache,
## "<uuid>-driver-<number>"; the number is shown as Vulkan packs versions (10, 10 and 12 bits),
## and as it is, since some makers pack it their own way.
static func driver_version() -> String:
	var device := RenderingServer.get_rendering_device()
	if device == null:
		return "unknown"
	var id := device.get_device_pipeline_cache_uuid()
	var at := id.find("-driver-")
	if at < 0:
		return "unknown"
	var packed := id.substr(at + 8).to_int()
	return "%d.%d.%d (%d)" % [packed >> 22, (packed >> 12) & 1023, packed & 4095, packed]


## The facts on one line, to copy into the chat.
static func code(f: Dictionary) -> String:
	var parts := PackedStringArray([f.get("version", "")])
	for row in ROWS:
		parts.append("%s %s" % [row[1], f.get(row[0], "")])
	return " | ".join(parts)


func _ready() -> void:
	theme = _theme()
	_build()
	show_facts(facts())
	resized.connect(_layout)
	_layout()
	# "look", "scene", "kit" or "reports" on the command line opens that screen at once, for the
	# cloud's pictures
	var args := OS.get_cmdline_user_args()
	for screen: String in ["look", "scene", "kit", "reports"]:
		if screen in args:
			open_screen(
				{
					"look": "res://look/look.gd",
					"scene": "res://scene/scene.gd",
					"kit": "res://kit/kit.gd",
					"reports": "res://reports/reports.gd",
				}[screen]
			)
			break


## Opens a prototype's screen over the menu; its closed signal brings the menu back.
func open_screen(script_path: String) -> void:
	var screen := Control.new()
	screen.set_script(load(script_path))
	screen.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	_safe.visible = false
	add_child(screen)
	screen.connect("closed", _close_screen.bind(screen))


func _close_screen(screen: Control) -> void:
	screen.queue_free()
	_safe.visible = true


## Fills the self-check's rows.
func show_facts(f: Dictionary) -> void:
	for row in ROWS:
		var value: String = str(f.get(row[0], ""))
		(_values[row[0]] as Label).text = value if not value.is_empty() else "unknown"
	_copy.set_meta("code", code(f))


## One column in portrait, the menu below; two columns in landscape. The margins keep clear of the
## camera's cutout and the system's bars.
func _layout() -> void:
	var portrait := size.y >= size.x
	_columns.vertical = portrait
	_self_check.size_flags_vertical = SIZE_FILL if portrait else SIZE_EXPAND_FILL
	_self_check_scroll.vertical_scroll_mode = (
		ScrollContainer.SCROLL_MODE_DISABLED if portrait else ScrollContainer.SCROLL_MODE_AUTO
	)
	var inset := _insets()
	_safe.add_theme_constant_override("margin_left", GAP + inset.x)
	_safe.add_theme_constant_override("margin_top", GAP + inset.y)
	_safe.add_theme_constant_override("margin_right", GAP + inset.z)
	_safe.add_theme_constant_override("margin_bottom", GAP + inset.w)


## The safe area's insets, left, top, right and bottom, in the screen's units; none where the
## platform reports no safe area, as in the cloud.
func _insets() -> Vector4i:
	var window := DisplayServer.window_get_size()
	var safe := DisplayServer.get_display_safe_area()
	if safe.size.x <= 0 or safe.size.y <= 0 or window.x <= 0 or size.x <= 0:
		return Vector4i()
	var k := size.x / float(window.x)
	return Vector4i(
		roundi(safe.position.x * k),
		roundi(safe.position.y * k),
		roundi(maxi(window.x - safe.end.x, 0) * k),
		roundi(maxi(window.y - safe.end.y, 0) * k)
	)


func _build() -> void:
	var background := ColorRect.new()
	background.color = NIGHT
	background.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(background)
	_safe = MarginContainer.new()
	_safe.set_anchors_and_offsets_preset(PRESET_FULL_RECT)
	add_child(_safe)
	var page := VBoxContainer.new()
	page.add_theme_constant_override("separation", GAP)
	_safe.add_child(page)
	var header := HBoxContainer.new()
	page.add_child(header)
	header.add_child(_label("Kindling", FLAME, 24, true))
	var version := _label(
		str(ProjectSettings.get_setting("application/config/version", "")), DIM, 16
	)
	version.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	version.size_flags_vertical = SIZE_SHRINK_END
	header.add_child(version)
	_columns = BoxContainer.new()
	_columns.size_flags_vertical = SIZE_EXPAND_FILL
	_columns.add_theme_constant_override("separation", GAP)
	page.add_child(_columns)
	_self_check = _panel("SelfCheck", 0)
	_self_check_scroll = _self_check.get_child(0) as ScrollContainer
	_columns.add_child(_self_check)
	_menu = _panel("Menu", GAP)
	_columns.add_child(_menu)
	_build_self_check(_column(_self_check))
	_build_menu(_column(_menu))


func _build_self_check(box: VBoxContainer) -> void:
	box.add_child(_label("Self-check", INK, 20))
	var grid := GridContainer.new()
	grid.columns = 2
	grid.add_theme_constant_override("h_separation", GAP)
	grid.add_theme_constant_override("v_separation", 6)
	box.add_child(grid)
	for row in ROWS:
		var key := _label(row[1], DIM, 16)
		key.size_flags_vertical = SIZE_SHRINK_BEGIN
		grid.add_child(key)
		var value := _label("", INK, 16)
		value.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		value.size_flags_horizontal = SIZE_EXPAND_FILL
		grid.add_child(value)
		_values[row[0]] = value
	_copy = Button.new()
	_copy.text = "Copy for the chat"
	_copy.custom_minimum_size.y = TARGET
	_copy.pressed.connect(_on_copy)
	box.add_child(_copy)


func _build_menu(box: VBoxContainer) -> void:
	box.add_child(_label("Prototypes", INK, 20))
	for entry in PROTOTYPES:
		if entry.size() > 2:
			var open := Button.new()
			open.text = "%s  ·  %s" % [entry[0], entry[1]]
			open.custom_minimum_size.y = TARGET
			open.alignment = HORIZONTAL_ALIGNMENT_LEFT
			open.pressed.connect(open_screen.bind(entry[2]))
			box.add_child(open)
			continue
		var row := HBoxContainer.new()
		row.custom_minimum_size.y = TARGET
		var title := _label(entry[0], DIM, 16)
		title.size_flags_horizontal = SIZE_EXPAND_FILL
		title.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		row.add_child(title)
		var when := _label("from %s" % entry[1], EDGE.lightened(0.25), 14)
		when.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
		row.add_child(when)
		box.add_child(row)


func _on_copy() -> void:
	DisplayServer.clipboard_set(str(_copy.get_meta("code", "")))
	_copy.text = "Copied"
	await get_tree().create_timer(2.0).timeout
	_copy.text = "Copy for the chat"


## A panel holding a scroll area holding a column, with room on its right for the scroll bar.
func _panel(panel_name: String, bar_room: int) -> PanelContainer:
	var panel := PanelContainer.new()
	panel.name = panel_name
	panel.size_flags_horizontal = SIZE_EXPAND_FILL
	panel.size_flags_vertical = SIZE_EXPAND_FILL
	var scroll := ScrollContainer.new()
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	panel.add_child(scroll)
	var room := MarginContainer.new()
	room.size_flags_horizontal = SIZE_EXPAND_FILL
	room.add_theme_constant_override("margin_right", bar_room)
	scroll.add_child(room)
	var box := VBoxContainer.new()
	box.add_theme_constant_override("separation", 8)
	room.add_child(box)
	return panel


## The column inside a panel made by _panel.
static func _column(panel: PanelContainer) -> VBoxContainer:
	return panel.get_child(0).get_child(0).get_child(0) as VBoxContainer


func _label(text: String, color: Color, font_size: int, expand := false) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_color_override("font_color", color)
	label.add_theme_font_size_override("font_size", font_size)
	if expand:
		label.size_flags_horizontal = SIZE_EXPAND_FILL
	return label


func _theme() -> Theme:
	var t := Theme.new()
	t.default_font_size = 16
	var panel := StyleBoxFlat.new()
	panel.bg_color = PANEL
	panel.border_color = EDGE
	panel.set_border_width_all(1)
	panel.set_corner_radius_all(8)
	panel.set_content_margin_all(GAP)
	t.set_stylebox("panel", "PanelContainer", panel)
	for state in ["normal", "hover", "pressed", "focus"]:
		var button := StyleBoxFlat.new()
		button.bg_color = FLAME.darkened(0.55) if state != "pressed" else FLAME.darkened(0.35)
		button.border_color = FLAME.darkened(0.2)
		button.set_border_width_all(1 if state != "focus" else 2)
		button.set_corner_radius_all(8)
		button.set_content_margin_all(GAP)
		t.set_stylebox(state, "Button", button)
	var grabber := StyleBoxFlat.new()
	grabber.bg_color = EDGE.lightened(0.15)
	grabber.set_corner_radius_all(3)
	for state in ["grabber", "grabber_highlight", "grabber_pressed"]:
		t.set_stylebox(state, "VScrollBar", grabber)
	t.set_stylebox("scroll", "VScrollBar", StyleBoxEmpty.new())
	t.set_color("font_color", "Button", INK)
	t.set_color("font_hover_color", "Button", INK)
	t.set_color("font_pressed_color", "Button", INK)
	t.set_color("font_focus_color", "Button", INK)
	return t
