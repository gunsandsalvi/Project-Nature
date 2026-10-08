## The probes (A4.7, VIS-14): before the 2D view relies on graphics features, each is
## tried once a build, drawing a little at the window's full size for a few frames. Each is noted
## before it runs, so if it closes the app the next start names it and does not try it again. The
## probe names differ from the old spatial tests; those results are not canvas proof.
## Implements VIS-14.
class_name Probes
extends Node

signal finished

const FILE := "user://probes.cfg"
## Frames each probe draws before it counts as passed.
const FRAMES := 4
## Each probe, in the order they run, and what it tries.
const ABOUT := {
	"canvas_nearest": "a 2D canvas texture with nearest sampling",
	"canvas_texture_grad": "a 2D texture read with its slopes given (textureGrad)",
	"canvas_own_levels": "2D textures with levels of our own",
	"shading_rates": "the driver's shading rates, asked through Vulkan",
}
const TEXTURES := [
	"res://data/textures/standin-meadow.kdtex", "res://data/textures/standin-pattern.kdtex"
]

## Each probe's result: "passed", "crashed" or "failed: why", and for shading rates what the driver
## offers.
var results := {}

var _config := ConfigFile.new()
var _order: Array = ABOUT.keys()
var _at := -1
var _frames := 0
var _view: SubViewport
## A probe that ends as it is built, without drawing, says how here.
var _ended := ""


## Runs every probe not yet tried in this build, one after another, then emits finished.
func start() -> void:
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	if _config.load(FILE) != OK or _config.get_value("build", "version", "") != version:
		_config = ConfigFile.new()
		_config.set_value("build", "version", version)
	for name: String in _order:
		var state: String = _config.get_value("probes", name, "")
		# still noted as running: it closed the app last time
		if state == "running":
			state = "crashed"
			_config.set_value("probes", name, state)
		if not state.is_empty():
			results[name] = state
	_config.save(FILE)
	_next()


func _next() -> void:
	_clear()
	_at += 1
	while _at < _order.size() and results.has(_order[_at]):
		_at += 1
	if _at >= _order.size():
		set_process(false)
		finished.emit()
		return
	var name: String = _order[_at]
	_config.set_value("probes", name, "running")
	_config.save(FILE)
	_ended = ""
	var problem := _build(name)
	if not problem.is_empty() or not _ended.is_empty():
		_record(name, _ended if problem.is_empty() else "failed: " + problem)
		_next.call_deferred()
		return
	_frames = 0
	set_process(true)


func _process(_delta: float) -> void:
	_frames += 1
	if _frames >= FRAMES:
		set_process(false)
		_record(_order[_at], "passed")
		_next.call_deferred()


func _exit_tree() -> void:
	# closed part way, as leaving the page does: the probe under way did not close the app, so it
	# runs again next time
	if _at >= 0 and _at < _order.size() and not results.has(_order[_at]):
		_config.erase_section_key("probes", _order[_at])
		_config.save(FILE)
	_clear()


func _record(name: String, state: String) -> void:
	results[name] = state
	_config.set_value("probes", name, state)
	_config.save(FILE)


func _clear() -> void:
	if _view != null:
		_view.queue_free()
		_view = null


## Builds a probe's small scene in a viewport of its own at the window's size; "" or the problem.
func _build(name: String) -> String:
	if name == "shading_rates":
		# asked, not drawn: the self-check lists what the driver offers
		if OS.get_name() != "Android":
			_ended = "not asked off Android"
			return ""
		var rates := KdDevice.new().shading_rates()
		if not rates.get("available", false):
			return "the driver could not be asked"
		_ended = "passed"
		return ""
	_view = SubViewport.new()
	_view.size = DisplayServer.window_get_size()
	_view.disable_3d = true
	_view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	add_child(_view)
	match name:
		"canvas_nearest":
			_show(_checker(64))
		"canvas_texture_grad":
			var material := ShaderMaterial.new()
			var shader := Shader.new()
			shader.code = (
				"shader_type canvas_item; uniform sampler2D tex : filter_linear_mipmap;"
				+ " void fragment() { COLOR = textureGrad(tex, UV * 8.0,"
				+ " dFdx(UV * 8.0), dFdy(UV * 8.0)); }"
			)
			material.shader = shader
			material.set_shader_parameter("tex", _checker(64))
			_show(_checker(64)).material = material
		"canvas_own_levels":
			var decoder := KdLook.new()
			for path: String in TEXTURES:
				var read: Dictionary = decoder.texture_image(path)
				if not str(read.get("problem", "")).is_empty():
					return str(read.problem)
				var image: Image = read.get("image")
				if image == null:
					return "texture decoder returned no image"
				_show(ImageTexture.create_from_image(image))
	return ""


func _show(texture: Texture2D) -> TextureRect:
	var node := TextureRect.new()
	node.texture = texture
	node.texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	node.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_view.add_child(node)
	return node


## A checker of single pixels, uploaded as an ordinary 2D texture.
func _checker(side: int) -> ImageTexture:
	var image := Image.create(side, side, true, Image.FORMAT_RGBA8)
	for y in side:
		for x in side:
			image.set_pixel(
				x, y, Color(0.9, 0.8, 0.3) if (x + y) % 2 == 0 else Color(0.3, 0.4, 0.2)
			)
	image.generate_mipmaps()
	return ImageTexture.create_from_image(image)
