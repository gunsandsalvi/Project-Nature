## Implements PRE-31, RES-05 (T2.7a.4): frozen 2D captures, separate from old 3D proofs.
## Compatibility under Xvfb: -- <prefix> <noon|dusk> [second] [facing] [walk|work] [piece|clip].
## Saves the native page, unscaled semantic passes, ID table and actual supplied art channels.
extends SceneTree

const Fixtures := preload("res://pages/fixtures.gd")


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() < 2:
		printerr("usage: -- <prefix> <noon|dusk> [second] [facing] [walk|work] [piece]")
		quit(2)
		return
	var page := Fixtures.new()
	page.frozen = true
	page.dusk = args[1] == "dusk"
	root.add_child(page)
	for i in 6:
		await process_frame
	page.drawing.second = float(args[2]) if args.size() > 2 else page.world.screen_time()
	page.drawing.facing_count = 8
	page.drawing.facing = int(args[3]) if args.size() > 3 else 0
	page.drawing.action = args.size() > 4 and args[4] == "work"
	if args.size() > 5 and args[5] == "clip":
		page.set_process(false)
		var saved := await _clip(page, args[0])
		quit(0 if saved else 1)
		return
	if args.size() > 5:
		page._inspect_piece(int(args[5]))
	page.set_process(false)
	page.drawing.rebuild()
	await _settle()
	if not _save(root.get_texture().get_image(), args[0] + "-page.png"):
		return
	for pass_name: String in ["colour", "object", "material"]:
		page.drawing.pass_name = pass_name
		page.drawing.rebuild()
		await _settle()
		if not _save(
			page.drawing.get_viewport().get_texture().get_image(),
			args[0] + "-" + pass_name + ".png"
		):
			return
	var channels := {}
	for entry: Dictionary in page.drawing.entries:
		var supplied := {}
		for channel: String in ["colour", "normal", "material"]:
			var texture: Texture2D = page.drawing.atlas.texture(
				entry, int(page.state.density), "walk", channel
			)
			if texture != null:
				var path: String = args[0] + "-" + entry.name + "-" + channel + ".png"
				if not _save(texture.get_image(), path):
					return
				supplied[channel] = path
		channels[entry.name] = supplied
	var metadata := {
		"renderer": "Compatibility / Mesa llvmpipe",
		"engine": Engine.get_version_info().string,
		"window": [root.size.x, root.size.y],
		"viewport": [page.state.size.x, page.state.size.y],
		"density": page.state.density,
		"pixel_scale": page.state.scale,
		"second": page.world.screen_time(),
		"animation_second": page.drawing.second,
		"facing": page.drawing.facing,
		"object_codes": page.drawing.id_table,
		"material_codes":
		{"1": "tree", "2": "shelter", "3": "boulder", "4": "ground", "5": "person", "6": "animal"},
		"channels": channels,
		"status":
		"Developer placeholders. Dusk is a diagnostic tint. Normals and material masks pending art."
	}
	var file := FileAccess.open(args[0] + "-capture.json", FileAccess.WRITE)
	if file == null:
		printerr("Cannot write capture metadata.")
		quit(1)
		return
	file.store_string(JSON.stringify(metadata, "\t") + "\n")
	quit(0)


## Implements PRE-22, PRE-31, PRE-33: frames at 12 fps, including a slow pan and pinch release.
func _clip(page: Control, prefix: String) -> bool:
	var frames := []
	for frame in 72:
		page.drawing.action = frame >= 24 and frame < 48
		page.drawing.facing_count = 4 if frame < 48 else 8
		page.drawing.facing = (frame / 6) % page.drawing.facing_count
		if frame == 0:
			page.camera.touch(0, 9, Vector2(500, 1000), 0.0)
		elif frame < 24:
			page.camera.touch(1, 9, Vector2(500 + frame * 0.25, 1000), frame / 12.0)
		elif frame == 24:
			page.camera.touch(2, 9, Vector2(506, 1000), 2.0)
		elif frame == 48:
			page.camera.touch(0, 0, Vector2(450, 1000), 4.0)
			page.camera.touch(0, 1, Vector2(550, 1000), 4.0)
		elif frame > 48 and frame < 71:
			var spread := 50.0 + (frame - 48) * 1.4
			page.camera.touch(1, 0, Vector2(500 - spread, 1000), frame / 12.0)
			page.camera.touch(1, 1, Vector2(500 + spread, 1000), frame / 12.0)
		elif frame == 71:
			page.camera.touch(2, 0, Vector2(419.2, 1000), frame / 12.0)
			page.camera.touch(2, 1, Vector2(580.8, 1000), frame / 12.0)
		page._process(0.0)
		page.drawing.second = page.world.screen_time() + frame / 6.0
		page.drawing.rebuild()
		await _settle()
		var path := "%s-frame-%03d.png" % [prefix, frame]
		if not _save(root.get_texture().get_image(), path):
			return false
		frames.append(
			{
				"path": path,
				"density": page.state.density,
				"scale": page.state.live_scale,
				"action": "work" if page.drawing.action else "walk",
				"facings": page.drawing.facing_count
			}
		)
	var file := FileAccess.open(prefix + "-clip.json", FileAccess.WRITE)
	if file == null:
		printerr("Cannot write clip metadata.")
		return false
	file.store_string(
		JSON.stringify(
			{"fps": 12, "status": "Developer diagrams, not art approval", "frames": frames}, "\t"
		)
	)
	return true


func _settle() -> void:
	for i in 3:
		await process_frame
	await RenderingServer.frame_post_draw


func _save(image: Image, path: String) -> bool:
	if image.save_png(path) != OK:
		printerr("Cannot write " + path)
		quit(1)
		return false
	return true
