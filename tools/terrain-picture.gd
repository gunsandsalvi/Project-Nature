## Implements PRE-31 PLT-04 (T2.8a.4): frozen views, receivers and all viewport clocks.
extends "fixture-picture.gd"

const Terrain := preload("res://pages/terrain.gd")


func capture() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() < 3:
		printerr(
			"usage: -- prefix noon|dusk scene [sun-direction] [colour|receivers|normals|layers] [clip]"
		)
		quit(2)
		return
	var page := Terrain.new()
	page.frozen = true
	page.dusk = args[1] == "dusk"
	root.add_child(page)
	for i in 6:
		await process_frame
	page.set_scene(args[2])
	page.drawing.direction = int(args[3]) if args.size() > 3 else 0
	var debug := args[4] if args.size() > 4 else "colour"
	page.drawing.debug_mode = "colour" if debug in ["closed", "band", "rain", "winter"] else debug
	page.drawing.reveal = debug != "closed"
	if debug == "band":
		page.camera.zoom(0.5, Vector2(root.size) * 0.5, true)
		page.drawing.selected = -5
	if debug in ["rain", "winter"]:
		page.drawing.weather = debug
	page.set_process(false)
	page.drawing._scene_revision = ""
	page.drawing._revision = ""
	page._process(0.0)
	await _settle()
	if not page.drawing.problem.is_empty():
		printerr(page.drawing.problem)
		quit(1)
		return
	if args.size() > 5 and args[5] == "clip":
		quit(0 if await _terrain_clip(page, args[0]) else 1)
		return
	var viewports: Array[Viewport] = Timing.viewports(page)
	var cold_costs: Dictionary = page.drawing.terrain.costs()
	cold_costs.script_ms = page.drawing.cpu_rebuild_ms
	var samples := {
		"script_ms": [],
		"cpu_ms": [],
		"gpu_ms": [],
		"reflection_gpu_ms": [],
		"bed_gpu_ms": [],
		"world_gpu_ms": [],
		"ui_gpu_ms": []
	}
	var still_only := args.size() > 5 and args[5] == "still"
	for frame in 0 if still_only else 90:
		page._preview_second = frame / 60.0
		page._process(0.0)
		await _settle()
		if frame >= 10:
			samples.script_ms.append(page.drawing.cpu_rebuild_ms)
			samples.cpu_ms.append(Timing.cpu_ms(viewports))
			samples.gpu_ms.append(Timing.gpu_ms(viewports))
			samples.reflection_gpu_ms.append(Timing.gpu_ms([page.drawing._reflections]))
			samples.bed_gpu_ms.append(Timing.gpu_ms([page.drawing._bed]))
			samples.world_gpu_ms.append(Timing.gpu_ms([page._viewport]))
			samples.ui_gpu_ms.append(Timing.gpu_ms([root]))
	page._preview_second = 0.0
	page._process(0.0)
	await _settle()
	if not _save(root.get_texture().get_image(), args[0] + "-page.png"):
		return
	var colour_draw_calls := RenderingServer.get_rendering_info(
		RenderingServer.RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME
	)
	for pass_kind: String in ["colour", "object", "material"]:
		page.pass_name = pass_kind
		page._process(0.0)
		await _settle()
		if not _save(
			page.drawing.get_viewport().get_texture().get_image(),
			args[0] + "-" + pass_kind + ".png"
		):
			return
	var statistics := {}
	for key: String in samples:
		var values: Array = samples[key]
		if values.is_empty():
			statistics[key] = {"status": "unavailable: still-only capture"}
			continue
		values.sort()
		var sum := 0.0
		for value: float in values:
			sum += value
		statistics[key] = {
			"mean": sum / values.size(),
			"p95": values[int(values.size() * 0.95)],
			"p99": values[-1],
			"samples": values.size()
		}
		if sum == 0:
			statistics[key] = {"status": "unavailable"}
		if args[2] != "water" and key in ["bed_gpu_ms", "reflection_gpu_ms"]:
			statistics[key] = {"status": "inactive"}
	var metadata := {
		"engine": Engine.get_version_info().string,
		"renderer": "Compatibility / Mesa llvmpipe",
		"window": [root.size.x, root.size.y],
		"scene": args[2],
		"hour": page.drawing.hour,
		"weather": page.drawing.weather,
		"declared_light": page.drawing.light_record,
		"costs": page.drawing.terrain.costs(),
		"cold_costs": cold_costs,
		"cold_method": "Fresh scene masks; art and shader variants already loaded",
		"timings": statistics,
		"timing_method":
		"Steady-clock scripts with 10 Hz moving masks; viewport CPU/GPU incl. UI and active targets.",
		"viewports": viewports.size(),
		"active_viewports": 4 if args[2] == "water" else 2,
		"colour_draw_calls": colour_draw_calls,
		"sprite_count": page.drawing.draw_list().size(),
		"receiver_count": page.drawing.surface_records.size(),
		"target_bytes_nominal_rgba8":
		4 * (root.size.x * root.size.y + 3 * page._viewport.size.x * page._viewport.size.y),
		"target_bytes_method":
		"RGBA8 estimate: native frame plus world, bed and reflection allocations",
		"texture_bytes_counter":
		RenderingServer.get_rendering_info(RenderingServer.RENDERING_INFO_TEXTURE_MEM_USED),
		"power_watts": "unavailable: no phone connected",
		"temperature_c": "unavailable: no phone connected",
		"object_codes": page.drawing.id_table,
		"semantic_method":
		"Opaque front-layer IDs; colour reveal cannot blend integer codes. Picking skips faded cover.",
		"material_codes":
		{
			"1": "tree",
			"2": "hide",
			"3": "stone",
			"4": "ground",
			"5": "skin",
			"6": "fur",
			"7": "wood",
			"8": "water",
			"9": "fire"
		},
		"status":
		(
			"Restricted developer fixtures. Flat style review before terrain approval. "
			+ "No physical sky or phone-performance claim."
		)
	}
	var file := FileAccess.open(args[0] + "-capture.json", FileAccess.WRITE)
	if file == null:
		quit(1)
		return
	file.store_string(JSON.stringify(_json_data(metadata), "\t") + "\n")
	quit(0)


func _terrain_clip(page: Control, prefix: String) -> bool:
	var frames := []
	for frame in 72:
		if frame < 24:
			page.drawing.direction = frame / 6
		elif frame < 48:
			page.set_scene("shelter")
			page.drawing.selected = -5
			page.dusk = true
			page.drawing.reveal = frame >= 36
		else:
			page.set_scene("water")
			page.dusk = false
		page._process(0.0)
		page.drawing.second = 43200 + frame / 12.0
		page.drawing.rebuild()
		await _settle()
		if not page.drawing.problem.is_empty():
			printerr(page.drawing.problem)
			return false
		var path := "%s-frame-%03d.png" % [prefix, frame]
		if not _save(root.get_texture().get_image(), path):
			return false
		frames.append(
			{
				"path": path,
				"scene": page.drawing.scene_name,
				"sun_direction": page.drawing.direction,
				"revealed": page.drawing.reveal,
				"costs": page.drawing.terrain.costs()
			}
		)
	var file := FileAccess.open(prefix + "-clip.json", FileAccess.WRITE)
	if file == null:
		return false
	file.store_string(
		JSON.stringify({"fps": 12, "frames": frames, "status": "Developer geometry and art"}, "\t")
	)
	return true


func _json_data(value: Variant) -> Variant:
	if value is Vector3:
		return [value.x, value.y, value.z]
	if value is Vector2:
		return [value.x, value.y]
	if value is Dictionary:
		var out := {}
		for key: Variant in value:
			out[key] = _json_data(value[key])
		return out
	if value is Array:
		return value.map(_json_data)
	return value
