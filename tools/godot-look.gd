## Implements PRE-31, see A4.8 and A5.5: the look loop's drawing run. Under Xvfb on the software
## Vulkan driver with one thread, as `godot --path game --rendering-method mobile --rendering-driver
## vulkan --resolution <W>x<H> -s <this file> -- <plan.json> <out folder>`; tools/lookrun.py writes
## the plan and reads what this draws. Time stands still but for the paths' own steps of 1/30 s.
##
## The Look page's rig and ground are drawn through cameras of this script's own, so none of the
## page's controls shows. Each fixed view gives its colour picture, as the game draws it, its
## material and object pictures, drawn with plain light, and a many-sample picture of each patch:
## the patch drawn as many times larger as the plan says, for the runner to shrink. Each path gives,
## frame by frame, its patch, the patch's many-sample picture and where four of the patch's pixels
## were in the frame before, from which the runner finds the camera's motion.
extends SceneTree

const LookPage := preload("res://pages/look.gd")
const STEP := 1.0 / 30.0
const WARM_FRAMES := 3

var _look: KdLook
var _size: Vector2i
var _many := 8
var _frame_vp: SubViewport
var _frame_cam: Camera3D
var _number_vp: SubViewport
var _number_cam: Camera3D
var _many_vps := {}
var _warmed := {}


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() != 2:
		printerr("usage: -- <plan.json> <out folder>")
		quit(2)
		return
	var plan: Variant = JSON.parse_string(FileAccess.get_file_as_string(args[0]))
	if not plan is Dictionary:
		printerr("no plan in ", args[0])
		quit(2)
		return
	_run(plan, args[1])


func _run(plan: Dictionary, out: String) -> void:
	await process_frame
	var page := LookPage.new()
	root.add_child(page)
	await process_frame
	if not page.problem.is_empty():
		printerr("the Look page could not load its ground: ", page.problem)
		quit(1)
		return
	# this script moves the rig itself, a frame at a time, and draws only through its own cameras
	page.set_process(false)
	root.disable_3d = true
	_look = page.look
	_size = Vector2i(int(plan["size"][0]), int(plan["size"][1]))
	_many = int(plan.get("many", 8))
	_look.set_screen(Vector2(_size))
	_frame_vp = _viewport(_size, Viewport.MSAA_2X)
	_frame_cam = _camera(_frame_vp, null)
	_number_vp = _viewport(_size, Viewport.MSAA_DISABLED)
	_number_cam = _camera(_number_vp, _plain_light())
	for view: Dictionary in plan.get("views", []):
		await _draw_view(view, out.path_join("views"))
	for path: Dictionary in plan.get("paths", []):
		await _draw_path(path, out.path_join("paths").path_join(path["name"]))
	print("Look run: done")
	quit(0)


func _viewport(size: Vector2i, msaa: Viewport.MSAA) -> SubViewport:
	var vp := SubViewport.new()
	vp.size = size
	vp.msaa_3d = msaa
	vp.render_target_update_mode = SubViewport.UPDATE_DISABLED
	root.add_child(vp)
	return vp


func _camera(vp: SubViewport, light: Environment) -> Camera3D:
	var camera := Camera3D.new()
	camera.environment = light
	vp.add_child(camera)
	camera.current = true
	return camera


## Light that leaves a surface's emission as it is: no ambient light, no tone curve.
func _plain_light() -> Environment:
	var environment := Environment.new()
	environment.background_mode = Environment.BG_COLOR
	environment.background_color = Color.BLACK
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color.BLACK
	environment.ambient_light_energy = 0.0
	environment.tonemap_mode = Environment.TONE_MAPPER_LINEAR
	return environment


## A camera put where the rig's is, seeing the whole frame.
func _pose(camera: Camera3D) -> void:
	camera.projection = Camera3D.PROJECTION_PERSPECTIVE
	LookScene.place_camera(camera, _look)


## A camera put where the rig's is, seeing only a square patch of the frame: x and y its top left
## corner in the frame's pixels, side its width.
func _pose_patch(camera: Camera3D, patch: Array) -> void:
	_pose(camera)
	var pose: Dictionary = _look.pose()
	var across := float(_size.x if pose["keep_width"] else _size.y)
	var pixel := 2.0 * camera.near * tan(deg_to_rad(camera.fov) / 2.0) / across
	var side := float(patch[2])
	var middle := Vector2(float(patch[0]) + side / 2.0, float(patch[1]) + side / 2.0)
	camera.projection = Camera3D.PROJECTION_FRUSTUM
	camera.keep_aspect = Camera3D.KEEP_WIDTH
	camera.size = side * pixel
	camera.frustum_offset = Vector2(
		(middle.x - _size.x / 2.0) * pixel, (_size.y / 2.0 - middle.y) * pixel
	)


func _capture(vp: SubViewport) -> Image:
	vp.render_target_update_mode = SubViewport.UPDATE_ONCE
	await RenderingServer.frame_post_draw
	return vp.get_texture().get_image()


## The many-sample picture of a patch: drawn _many times larger, each texture level read as at the
## frame's size, so only the sampling differs. Its viewport is made once for each size, and drawn a
## few times first, so its first picture is whole.
func _capture_many(patch: Array) -> Image:
	var side := int(patch[2]) * _many
	if not _many_vps.has(side):
		var made := _viewport(Vector2i(side, side), Viewport.MSAA_DISABLED)
		_camera(made, null)
		_many_vps[side] = made
	var vp: SubViewport = _many_vps[side]
	var camera: Camera3D = vp.get_child(0)
	_pose_patch(camera, patch)
	_look.set_many(_many)
	_look.frame(0.0)
	if not _warmed.has(side):
		for i in WARM_FRAMES:
			await _capture(vp)
		_warmed[side] = true
	var image: Image = await _capture(vp)
	_look.set_many(1)
	_look.frame(0.0)
	return image


## Puts the rig where a view or a path starts, with the parts of the drawing it shows: the focus in
## centimetres east and north, the heading in degrees and the zoom in metres a screen pixel.
func _place(spec: Dictionary) -> void:
	var parts: Dictionary = spec.get("parts", {})
	for part: String in ["ground", "pattern"]:
		_look.set_part(part, bool(parts.get(part, true)))
	_look.set_view(
		int(spec.get("east", 0)),
		int(spec.get("north", 0)),
		float(spec.get("heading", 0.0)),
		float(spec.get("metres_per_pixel", 1.0 / 128.0))
	)
	_look.frame(0.0)


func _draw_view(view: Dictionary, out: String) -> void:
	DirAccess.make_dir_recursive_absolute(out)
	var name: String = view["name"]
	_place(view)
	_pose(_frame_cam)
	_pose(_number_cam)
	for i in WARM_FRAMES:
		await _capture(_frame_vp)
	(await _capture(_frame_vp)).save_png(out.path_join(name + ".png"))
	for picture in [[1, "material"], [2, "object"]]:
		RenderingServer.global_shader_parameter_set("kd_picture", picture[0])
		await _capture(_number_vp)
		(await _capture(_number_vp)).save_png(out.path_join("%s-%s.png" % [name, picture[1]]))
	RenderingServer.global_shader_parameter_set("kd_picture", 0)
	var patches: Array = view.get("patches", [])
	for i in patches.size():
		(await _capture_many(patches[i])).save_png(out.path_join("%s-patch%d-many.png" % [name, i]))


func _draw_path(path: Dictionary, out: String) -> void:
	DirAccess.make_dir_recursive_absolute(out)
	var patch: Array = path["patch"]
	var side := int(patch[2])
	RenderingServer.global_shader_parameter_set(
		"kd_nearest", 1.0 if path.get("nearest", false) else 0.0
	)
	_place(path)
	_look.play(path["path"])
	var patch_vp := _viewport(Vector2i(side, side), Viewport.MSAA_2X)
	var patch_cam := _camera(patch_vp, null)
	var motion := FileAccess.open(out.path_join("motion.jsonl"), FileAccess.WRITE)
	var corners := [
		Vector2(0, 0), Vector2(side - 1, 0), Vector2(0, side - 1), Vector2(side - 1, side - 1)
	]
	var last_projection := Projection()
	var last_view := Transform3D()
	for f in int(path["frames"]):
		if f > 0:
			_look.frame(STEP)
		_pose(_frame_cam)
		_pose_patch(patch_cam, patch)
		if f == 0:
			for i in WARM_FRAMES:
				await _capture(patch_vp)
		else:
			# where four of the patch's pixels were in the frame before, both in the patch's own pixels
			var was := []
			for corner: Vector2 in corners:
				var at := corner + Vector2(0.5, 0.5) + Vector2(float(patch[0]), float(patch[1]))
				var local := last_view * _ground_at(at)
				var clip := last_projection * Vector4(local.x, local.y, local.z, 1.0)
				var screen := Vector2(
					(clip.x / clip.w * 0.5 + 0.5) * _size.x, (0.5 - clip.y / clip.w * 0.5) * _size.y
				)
				var then := screen - Vector2(float(patch[0]), float(patch[1])) - Vector2(0.5, 0.5)
				was.append([then.x, then.y])
			motion.store_line(
				JSON.stringify(
					{
						"frame": f,
						"from": corners.map(func(c: Vector2) -> Array: return [c.x, c.y]),
						"to": was
					}
				)
			)
		(await _capture(patch_vp)).save_png(out.path_join("%03d.png" % f))
		(await _capture_many(patch)).save_png(out.path_join("%03d-many.png" % f))
		last_projection = _frame_cam.get_camera_projection()
		last_view = _frame_cam.global_transform.affine_inverse()
	motion.close()
	patch_vp.queue_free()
	_look.play("")
	RenderingServer.global_shader_parameter_set("kd_nearest", 0.0)


## The flat ground's point under a point of the frame, in the frame's pixels.
func _ground_at(at: Vector2) -> Vector3:
	var origin := _frame_cam.project_ray_origin(at)
	var normal := _frame_cam.project_ray_normal(at)
	return origin + normal * (-origin.y / normal.y)
