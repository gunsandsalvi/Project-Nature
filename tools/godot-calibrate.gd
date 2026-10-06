## Implements PLT-04, see A18.1 and A17: the calibration scenes drawn in the cloud. Under Xvfb on
## the software Vulkan driver, as `godot --path game --rendering-method mobile --rendering-driver
## vulkan --resolution <W>x<H> -s <this file> -- <out.json>`; tools/calibrun.py runs it and checks
## what it writes. The Calibrate page runs every variant of every scene as the phone does, a
## hundred times faster, and this writes the scenes, what each variant drew and the run's code, and
## a picture of each scene's first variant as it drew it, <scene>.png beside the run's file, and of
## every variant of the plants and the fires, <scene>-<variant>.png, which the run's check compares.
extends SceneTree

const CalibratePage := preload("res://pages/calibrate.gd")


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
	var file := FileAccess.open(out, FileAccess.WRITE)
	var written := {
		"scenes": page.scenes, "counted": page.counted, "readings": page.readings, "code": page.code
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
