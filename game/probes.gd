## The probes (A4.7, VIS-14): before the look relies on a feature of the phone's graphics, each is
## tried once a build, drawing a little at the window's full size for a few frames. Each is noted
## before it runs, so if it closes the app the next start names it and does not try it again. The
## GPU particles probe is expected to fail, since it runs a compute program that reads pictures,
## which stopped another engine on this phone's driver; it runs last. Implements VIS-14.
class_name Probes
extends Node

signal finished

const FILE := "user://probes.cfg"
## Frames each probe draws before it counts as passed.
const FRAMES := 4
## Each probe, in the order they run, and what it tries.
const ABOUT := {
	"msaa": "2x MSAA at the window's full size",
	"texture_grad": "a texture read with its slopes given (textureGrad)",
	"own_levels": "a texture array with levels of our own",
	"alpha_to_coverage": "cut-out edges smoothed by MSAA (alpha to coverage)",
	"shading_rates": "the driver's shading rates, asked through Vulkan",
	"multimesh_bones": "a MultiMesh shader reading bone weights",
	"gpu_particles": "Godot's GPU particles (expected to fail)",
}
const TEXTURES := [
	"res://data/textures/standin-meadow.kdtex", "res://data/textures/standin-pattern.kdtex"
]
const GROUND_SHADER := preload("res://look/ground.gdshader")

## Each probe's result: "passed", "crashed" or "failed: why", and for shading rates what the driver
## offers.
var results := {}

var _config := ConfigFile.new()
var _order: Array = ABOUT.keys()
var _at := -1
var _frames := 0
var _view: SubViewport
var _look: KdLook
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
	if _look != null:
		_look.clear()
		_look = null
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
	_view.own_world_3d = true
	_view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	_view.msaa_3d = Viewport.MSAA_2X
	add_child(_view)
	var camera := Camera3D.new()
	camera.position = Vector3(0.0, 2.0, 4.0)
	camera.rotation_degrees = Vector3(-25.0, 0.0, 0.0)
	camera.current = true
	_view.add_child(camera)
	var sun := DirectionalLight3D.new()
	sun.rotation_degrees = Vector3(-50.0, 30.0, 0.0)
	_view.add_child(sun)
	match name:
		"msaa":
			_view.add_child(_mesh(BoxMesh.new(), StandardMaterial3D.new()))
		"texture_grad":
			var material := ShaderMaterial.new()
			material.shader = _shader(
				(
					"shader_type spatial; uniform sampler2D tex : filter_linear_mipmap;"
					+ " void fragment() { ALBEDO = textureGrad(tex, UV * 8.0,"
					+ " dFdx(UV * 8.0), dFdy(UV * 8.0)).rgb; }"
				)
			)
			material.set_shader_parameter("tex", _checker(64, true))
			_view.add_child(_mesh(PlaneMesh.new(), material))
		"own_levels":
			_look = KdLook.new()
			var problem := _look.load_layers(PackedStringArray(TEXTURES))
			if not problem.is_empty():
				return problem
			_look.build(_view.find_world_3d().scenario, GROUND_SHADER.get_rid())
		"alpha_to_coverage":
			var material := StandardMaterial3D.new()
			material.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA_SCISSOR
			material.alpha_antialiasing_mode = BaseMaterial3D.ALPHA_ANTIALIASING_ALPHA_TO_COVERAGE
			material.albedo_texture = _checker(64, false)
			_view.add_child(_mesh(PlaneMesh.new(), material))
		"multimesh_bones":
			_view.add_child(_boned_multimesh())
		"gpu_particles":
			var particles := GPUParticles3D.new()
			particles.amount = 64
			particles.process_material = ParticleProcessMaterial.new()
			particles.draw_pass_1 = BoxMesh.new()
			particles.emitting = true
			_view.add_child(particles)
	return ""


func _mesh(mesh: Mesh, material: Material) -> MeshInstance3D:
	var node := MeshInstance3D.new()
	node.mesh = mesh
	node.material_override = material
	return node


func _shader(code: String) -> Shader:
	var shader := Shader.new()
	shader.code = code
	return shader


## A checker of single pixels with mipmaps; with see-through squares when cut out.
func _checker(side: int, opaque: bool) -> ImageTexture:
	var image := Image.create(side, side, true, Image.FORMAT_RGBA8)
	for y in side:
		for x in side:
			var on := (x + y) % 2 == 0
			image.set_pixel(x, y, Color(0.9, 0.8, 0.3, 1.0 if on or opaque else 0.0))
	image.generate_mipmaps()
	return ImageTexture.create_from_image(image)


## A MultiMesh of a triangle whose three corners each carry a bone and its weight, read by its
## shader as a poser would (A6.3).
func _boned_multimesh() -> MultiMeshInstance3D:
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = PackedVector3Array(
		[Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(0, 1, 0)]
	)
	arrays[Mesh.ARRAY_BONES] = PackedInt32Array([0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0])
	arrays[Mesh.ARRAY_WEIGHTS] = PackedFloat32Array([1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0])
	var mesh := ArrayMesh.new()
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	var material := ShaderMaterial.new()
	material.shader = _shader(
		(
			"shader_type spatial; render_mode cull_disabled; void vertex() {"
			+ " VERTEX.y += float(BONE_INDICES.x) * 0.1 * BONE_WEIGHTS.x; }"
		)
	)
	mesh.surface_set_material(0, material)
	var multimesh := MultiMesh.new()
	multimesh.transform_format = MultiMesh.TRANSFORM_3D
	multimesh.mesh = mesh
	multimesh.instance_count = 16
	for i in 16:
		multimesh.set_instance_transform(i, Transform3D(Basis(), Vector3(i % 4, 0, i / 4)))
	var node := MultiMeshInstance3D.new()
	node.multimesh = multimesh
	return node
