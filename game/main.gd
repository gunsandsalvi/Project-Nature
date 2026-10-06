## The app's shell (A2.3, A3.8): its name and version, the pages along the top, and the open page
## below. Implements PLT-06: every alpha opens on its self-check, the page you check first after an
## install.
extends Control

const PAGES := {
	"Check": preload("res://pages/check.gd"),
	"Time": preload("res://pages/time.gd"),
	"Crowd": preload("res://pages/crowd.gd"),
	"Worlds": preload("res://pages/worlds.gd"),
	"Catalogues": preload("res://pages/catalogues.gd"),
	"Reports": preload("res://pages/reports.gd"),
}
const BACKGROUND := Color("#1f1a24")
const TEXT := Color("#efe6d8")

var _tabs: GridContainer
var _content: MarginContainer
var _page_name := ""


func _ready() -> void:
	_build()
	open_page("Check")
	# a page named after the arguments' "--", as the cloud's pictures ask (tools/picture.sh)
	for arg in OS.get_cmdline_user_args():
		if PAGES.has(arg):
			open_page(arg)
	_set_frame_cap()


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_RESUMED:
		_set_frame_cap()
	elif what == NOTIFICATION_WM_GO_BACK_REQUEST:
		_go_back()


## The page now open.
func page_name() -> String:
	return _page_name


## Opens one of the pages by its name.
func open_page(page: String) -> void:
	for child in _content.get_children():
		child.queue_free()
	var node: Control = PAGES[page].new()
	_content.add_child(node)
	_page_name = page
	for button: Button in _tabs.get_children():
		button.disabled = button.text == page


func _build() -> void:
	var ground := ColorRect.new()
	ground.color = BACKGROUND
	ground.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(ground)
	var column := VBoxContainer.new()
	column.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	column.add_theme_constant_override("separation", 8)
	add_child(column)
	var title := Label.new()
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	title.text = "Kindling  %s" % version
	title.add_theme_font_size_override("font_size", 22)
	title.add_theme_color_override("font_color", TEXT)
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	column.add_child(title)
	# the pages in rows of three, each tab as wide as the screen allows
	_tabs = GridContainer.new()
	_tabs.columns = 3
	column.add_child(_tabs)
	for page: String in PAGES:
		var button := Button.new()
		button.text = page
		button.custom_minimum_size = Vector2(96, 48)
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		button.pressed.connect(open_page.bind(page))
		_tabs.add_child(button)
	_content = MarginContainer.new()
	_content.size_flags_vertical = Control.SIZE_EXPAND_FILL
	for side in ["left", "right", "top", "bottom"]:
		_content.add_theme_constant_override("margin_" + side, 12)
	column.add_child(_content)


## Godot 4.7.2 applies the project's frame cap before the screen's swapchain exists, so the phone
## kept running its screen at 120 Hz; setting the cap again once frames are drawn reaches it
## (research 18).
func _set_frame_cap() -> void:
	await get_tree().process_frame
	await get_tree().process_frame
	Engine.max_fps = 60


## Back never quits outright (A3.7): at the top it sends the app to the background, as Android's own
## apps do.
func _go_back() -> void:
	if OS.get_name() == "Android" and Engine.has_singleton("AndroidRuntime"):
		Engine.get_singleton("AndroidRuntime").getActivity().moveTaskToBack(true)
