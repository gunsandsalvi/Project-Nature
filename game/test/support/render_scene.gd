## Retained renderer regression harness (PRE-22 PRE-28 PRE-33 RES-05).
## No inspector menus or saved examples; tests supply scene, camera and elapsed time.
extends Control

const TerrainDrawing := preload("res://test/support/terrain/drawing.gd")
var drawing_script: Script = TerrainDrawing
var world := KdWorld.new()
var camera := KdCanvas.new()
var drawing: Node2D
var state := {}
var frozen := true
var dusk := false
var pass_name := "colour"
var _preview_second := 0.0
var _window := Vector2(1080, 2400)
var _viewport: SubViewport
var _outside_scene := "shelter"
var _outside_focus := Vector2(3, 3)
var _outside_selected := 0


func _ready() -> void:
	GameData.load_into(world)
	world.start_crowd(KdWorld.crowd_seed(), 4)
	world.begin_at(43200)
	world.pause()
	var origin := world.camp_at(0)
	camera.set_world(world, origin[0], origin[1])
	camera.focus(1, 0)
	_viewport = SubViewport.new()
	_viewport.disable_3d = true
	_viewport.canvas_item_default_texture_filter = (
		Viewport.DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST
	)
	add_child(_viewport)
	drawing = drawing_script.new()
	drawing.camera = camera
	_viewport.add_child(drawing)
	set_process(false)


func layout(window: Vector2, _safe: Rect2) -> void:
	_window = window


func _process(delta: float) -> void:
	state = camera.frame(int(_window.x), int(_window.y))
	_viewport.size = Vector2i(state.size)
	drawing.state = state
	_preview_second += delta
	drawing.second = world.screen_time() + _preview_second
	drawing.dusk = dusk
	drawing.pass_name = pass_name
	if drawing.get("scene_name") != null:
		drawing.hour = "night" if drawing.scene_name == "cave" else "dusk" if dusk else "noon"
	drawing.rebuild()


func set_scene(scene: String) -> void:
	if drawing.scene_name != scene and scene == "water":
		drawing.fire_enabled = false
	drawing.scene_name = scene
	if scene in ["shelter", "cave"]:
		camera.focus(3, 3)
	else:
		camera.focus(1, 0)


func toggle_cave() -> void:
	if drawing.scene_name == "cave":
		set_scene(_outside_scene)
		camera.focus(_outside_focus.x, _outside_focus.y)
		drawing.selected = _outside_selected
	else:
		_outside_scene = drawing.scene_name
		_outside_focus = camera.ground(_window * 0.5, 0)
		_outside_selected = drawing.selected
		set_scene("cave")
