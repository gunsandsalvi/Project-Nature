## Implements PRE-20 PRE-21 PRE-23 PRE-24 PRE-26 PRE-28 PRE-30 PRE-31 PRE-33 PLT-04 WLD-13.
## T2.8a: flat style is reviewed first; terrain is a separate, labelled approval gate.
extends "res://pages/fixtures.gd"

const TerrainDrawing := preload("res://terrain/drawing.gd")
var _scenes: OptionButton
var _outside_scene := "shelter"
var _outside_focus := Vector2(3, 3)
var _outside_selected := 0


func _init() -> void:
	drawing_script = TerrainDrawing
	save_folder = "user://terrain-local"


func _build() -> void:
	super._build()
	# PRE-31: these restricted receivers prove three fixture actors, not a crowd benchmark.
	for button: Button in _controls.find_children("*", "Button", true, false):
		if button.text == "World people":
			button.visible = false
	var tools := HFlowContainer.new()
	_controls.add_child(tools)
	_controls.move_child(tools, 4)
	var scenes := OptionButton.new()
	_scenes = scenes
	for scene: String in ["flat", "slope", "cliff", "shelter", "water", "cave"]:
		scenes.add_item(scene)
	scenes.item_selected.connect(func(index: int) -> void: set_scene(scenes.get_item_text(index)))
	scenes.custom_minimum_size = Vector2(150, 64)
	tools.add_child(scenes)
	_button(tools, "Sun direction", func() -> void: drawing.direction = (drawing.direction + 1) % 4)
	_button(tools, "Fire", func() -> void: drawing.fire_enabled = not drawing.fire_enabled)
	drawing.entrance_requested.connect(toggle_cave)
	_button(tools, "Cave entrance / exit", toggle_cave)
	_button(tools, "Reveal", func() -> void: drawing.reveal = not drawing.reveal)
	_button(
		tools,
		"Select person",
		func() -> void: drawing.selected = -7 if drawing.selected == -5 else -5
	)
	var weather := OptionButton.new()
	for key: String in ["dry", "rain", "winter"]:
		weather.add_item(key)
	weather.item_selected.connect(
		func(index: int) -> void: drawing.weather = weather.get_item_text(index)
	)
	tools.add_child(weather)
	var debug := OptionButton.new()
	for key: String in ["colour", "albedo", "receivers", "normals", "layers"]:
		debug.add_item(key)
	debug.item_selected.connect(
		func(index: int) -> void: drawing.debug_mode = debug.get_item_text(index)
	)
	tools.add_child(debug)
	_controls.get_child(0).text = "Kindling · flat light / terrain"


func set_scene(scene: String) -> void:
	if drawing.scene_name != scene and scene == "water":
		drawing.fire_enabled = false
	drawing.scene_name = scene
	if _scenes != null:
		_scenes.select(["flat", "slope", "cliff", "shelter", "water", "cave"].find(scene))
	if scene in ["shelter", "cave"]:
		camera.focus(3, 3)
	else:
		camera.focus(1, 0)
	if scene == "cave":
		drawing.hour = "night"


func toggle_cave() -> void:
	if drawing.scene_name == "cave":
		set_scene(_outside_scene)
		camera.focus(_outside_focus.x, _outside_focus.y)
		drawing.selected = _outside_selected
	else:
		_outside_scene = drawing.scene_name
		_outside_focus = camera.ground(Vector2(_native.size) * 0.5, 0.0)
		_outside_selected = drawing.selected
		set_scene("cave")


func _process(delta: float) -> void:
	if drawing != null:
		drawing.hour = "night" if drawing.scene_name == "cave" else "dusk" if dusk else "noon"
	super._process(delta)
	if drawing == null:
		return
	_description.text = "DEVELOPER ART · flat approval first · %s fixture" % drawing.scene_name
	var costs: Dictionary = drawing.terrain.costs()
	_status.text = (
		"%s / %s · declared light · %s\nCPU %.2f ms · masks %d KiB · GPU: measure on review route"
		% [
			drawing.hour,
			drawing.weather,
			drawing.scene_name,
			drawing.cpu_rebuild_ms,
			costs.mask_bytes / 1024
		]
	)
	if frozen:
		var frozen_status := "%s / %s · declared light · %s\n"
		frozen_status += "DEVELOPER ART · flat review before terrain · GPU counters in capture"
		_status.text = frozen_status % [drawing.hour, drawing.weather, drawing.scene_name]
	if not drawing.problem.is_empty():
		_status.text = drawing.problem
