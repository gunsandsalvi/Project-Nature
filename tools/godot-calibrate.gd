## Implements PLT-04 and PRE-27, see A18.1 and A17: the calibration scenes drawn in the cloud. Under
## Xvfb on the software Vulkan driver, as `godot --path game --rendering-method mobile
## --rendering-driver vulkan --resolution <W>x<H> -s <this file> -- <out.json>`; tools/calibrun.py
## runs it and checks what it writes. The Calibrate page runs every variant of every scene as the
## phone does, a hundred times faster, and this writes the scenes, what each variant drew and the
## run's code, and a picture of each scene's first variant as it drew it, <scene>.png beside the
## run's file, and of every variant of the plants and the fires, <scene>-<variant>.png, which the
## run's check compares. Then it checks C6's bone palettes against Godot's own skeletons: one figure
## as points, a vertex each, bent each way at two moments half a second apart, each vertex drawn at
## a pixel of its own with its bent place as its colour, and writes every vertex's place.
extends SceneTree

const CalibratePage := preload("res://pages/calibrate.gd")
## The ways C6 draws its figures, for the check of the palettes.
const WAYS: Array[String] = ["godot", "palette"]


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() != 1:
		printerr("usage: -- <out.json>")
		quit(2)
		return
	_run(args[0])


func _run(out: String) -> void:
	await process_frame
	var page: Control = CalibratePage.new()
	# the whole window, as the shell gives a page
	page.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	root.add_child(page)
	await process_frame
	if not page.problems.is_empty():
		printerr("the calibration scenes cannot be read: ", page.problems)
		quit(1)
		return
	page.time_scale = 0.01
	page.still_light = true
	page.timed.connect(_picture.bind(page, out.get_base_dir()))
	page.pick_all()
	page.start()
	while page.running():
		await process_frame
	var bent := await _bent_both_ways()
	var file := FileAccess.open(out, FileAccess.WRITE)
	var written := {
		"scenes": page.scenes,
		"counted": page.counted,
		"readings": page.readings,
		"code": page.code,
		"figures_bent": bent,
	}
	file.store_string(JSON.stringify(written))
	file.close()
	print("Calibration run: done")
	quit(0)


## The screen as a scene's first variant drew it, as <scene>.png in the folder, and as each of the
## plants' and the fires' variants drew it, as <scene>-<variant>.png.
func _picture(scene: int, variant: int, page: Control, folder: String) -> void:
	var name: String = page.scenes[scene]["name"]
	if variant == 0:
		root.get_texture().get_image().save_png(folder.path_join(name + ".png"))
	if page.scenes[scene]["draws"] in ["leaves", "fires"]:
		var way: String = page.scenes[scene]["variants"][variant]["name"]
		root.get_texture().get_image().save_png(folder.path_join("%s-%s.png" % [name, way]))


## One figure bent by Godot's skeleton and by the palette, each in a view of its own exactly as many
## pixels as the figure has vertices, so every pixel is a vertex's in both, however Godot orders its
## rows; at two moments half a second of walking apart: {"vertices", "moments": [{way: [x, y, z,
## ...] for each pixel}]}, in metres.
func _bent_both_ways() -> Dictionary:
	var count: int = KdFigures.new().vertices()
	var down := floori(sqrt(count))
	while count % down != 0:
		down -= 1
	var size := Vector2i(count / down, down)
	var views := {}
	for way in WAYS:
		views[way] = _bending_view(way, size)
	var moments := []
	for seconds: float in [0.0, 0.5]:
		for way in WAYS:
			(views[way]["figures"] as KdFigures).frame(seconds)
		# three frames, so each view has drawn the new pose
		for i in 3:
			await RenderingServer.frame_post_draw
		var moment := {}
		for way in WAYS:
			moment[way] = _places(views[way]["view"])
		moments.append(moment)
	for way in WAYS:
		(views[way]["figures"] as KdFigures).clear()
		(views[way]["view"] as Node).queue_free()
	return {"vertices": count, "moments": moments}


## A view of its own, black where nothing is drawn, its colours kept as numbers in half floats, with
## one figure in it as points, bent one way.
func _bending_view(way: String, size: Vector2i) -> Dictionary:
	var view := SubViewport.new()
	view.size = size
	view.use_hdr_2d = true
	view.render_target_update_mode = SubViewport.UPDATE_ALWAYS
	root.add_child(view)
	view.own_world_3d = true
	var eye := Camera3D.new()
	# looking at the figure, so Godot does not cull it, though each point sets its own pixel
	eye.position = Vector3(0.0, 1.0, 3.0)
	var plain := Environment.new()
	plain.background_mode = Environment.BG_COLOR
	plain.background_color = Color.BLACK
	plain.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	eye.environment = plain
	eye.current = true
	view.add_child(eye)
	var shader := Shader.new()
	shader.code = (
		"shader_type spatial;\nrender_mode unshaded, cull_disabled;\n#define KD_DUMP\n"
		+ ("#define KD_PALETTE\n" if way == "palette" else "")
		+ '#include "res://look/figure.gdshaderinc"\n'
	)
	var figures := KdFigures.new()
	figures.build(
		view.find_world_3d().scenario,
		way,
		shader.get_rid(),
		PackedVector3Array([Vector3.ZERO]),
		true
	)
	return {"view": view, "figures": figures, "shader": shader}


## Each pixel's colour as the place it holds, in metres: the shader keeps a place p as p / 2 + 1/2.
func _places(view: SubViewport) -> Array[float]:
	var image := view.get_texture().get_image()
	var out: Array[float] = []
	for y in image.get_height():
		for x in image.get_width():
			var colour := image.get_pixel(x, y)
			out.append_array([colour.r * 2.0 - 1.0, colour.g * 2.0 - 1.0, colour.b * 2.0 - 1.0])
	return out
