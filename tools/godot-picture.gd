## Implements PRC-11, see A2.3: draws a Godot project's main scene in the cloud and saves the
## picture, for the note. Run under Xvfb as `godot --path <project> --rendering-method mobile
## --resolution <W>x<H> -s <this file> -- <png> [frames]`.
extends SceneTree


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	if args.is_empty():
		printerr("usage: -- <png> [frames]")
		quit(2)
		return
	var frames := int(args[1]) if args.size() > 1 else 10
	var scene := load(str(ProjectSettings.get_setting("application/run/main_scene"))) as PackedScene
	root.add_child(scene.instantiate())
	for i in frames:
		await process_frame
	await RenderingServer.frame_post_draw
	var error := root.get_texture().get_image().save_png(args[0])
	quit(0 if error == OK else 1)
