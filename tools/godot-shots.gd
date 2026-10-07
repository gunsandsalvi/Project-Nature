## Implements PRC-11 and PRE-46, see A2.3, A6.5 and A17: draws pictures of one of the app's pages
## as the steps of a plan ask, for the model sheet and the notes. Run under Xvfb on the software
## Vulkan driver as `godot --path game --rendering-method mobile --rendering-driver vulkan
## --resolution <W>x<H> -s <this file> -- <page> <plan.json> <out folder>`; tools/shots.sh does
## it. The page is the script res://pages/<page>.gd made as a node of the root; the plan is a list
## of steps, each one of
##   {"call": "method", "args": [...]}   the page's method, called with the arguments
##   {"wait": n}                         n frames drawn
##   {"shot": "name.png"}                the window, as drawn, saved in the folder
## A call is followed by frames until the picture has settled, so a step may be a call and a shot
## alone.
extends SceneTree

const SETTLE_FRAMES := 5


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() != 3:
		printerr("usage: -- <page> <plan.json> <out folder>")
		quit(2)
		return
	var plan: Variant = JSON.parse_string(FileAccess.get_file_as_string(args[1]))
	if not plan is Array:
		printerr("no plan in ", args[1])
		quit(2)
		return
	_run(args[0], plan, args[2])


func _run(page_name: String, plan: Array, out: String) -> void:
	await process_frame
	var script := load("res://pages/%s.gd" % page_name) as GDScript
	if script == null:
		printerr("no page ", page_name)
		quit(2)
		return
	DirAccess.make_dir_recursive_absolute(out)
	var page: Control = script.new()
	page.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	root.add_child(page)
	# the page's own cover comes off after a few frames, and its first pictures need every pipeline
	for i in 8:
		await process_frame
	for step: Dictionary in plan:
		if step.has("call"):
			var result: Variant = page.callv(str(step["call"]), step.get("args", []))
			if result is String and not (result as String).is_empty():
				printerr("the call ", step["call"], " said: ", result)
			for i in SETTLE_FRAMES:
				await process_frame
		if step.has("wait"):
			for i in int(step["wait"]):
				await process_frame
		if step.has("shot"):
			await RenderingServer.frame_post_draw
			var error := root.get_texture().get_image().save_png(out.path_join(str(step["shot"])))
			if error != OK:
				printerr("could not save ", step["shot"])
				quit(1)
				return
	print("Shots: done")
	quit(0)
