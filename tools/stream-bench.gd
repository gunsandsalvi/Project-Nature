## Stream benchmark: controller ledger, decode and upload costs.
## Run pinned Godot under Xvfb with --path game --rendering-method gl_compatibility
## --resolution 1080x2400 --script tools/stream-bench.gd -- /tmp/<unique-prefix> run|reopen.
## Reopen deletes only this run's disposable view preferences, preserving its authoritative world.
extends SceneTree

const Examples := preload("res://pages/examples.gd")
var page: Control
var device := KdDevice.new()
var prefix := ""
var mode := "run"
var folder := ""
var report := {"stages": [], "problems": [], "phone_measurement": false}
var viewports: Array[Viewport] = []


func _initialize() -> void:
	call_deferred("run")


func _problem(message: String) -> void:
	report.problems.append(message)
	printerr(message)


func _open(save: String, held_token := 0) -> void:
	page = Examples.new()
	page.save_folder = save
	root.add_child(page)
	page.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	# Opening uses the real persistent world route; pause before its first process frame.
	page.world.pause()
	page.frozen = true
	if held_token != 0:
		var held := false
		for job: Dictionary in page._stream.ledger.status().jobs:
			if int(job.token) == held_token and job.worker_owned and job.state == "retiring":
				held = true
		report["worldswap_retained_worker"] = held
		report["worldswap_after_begin"] = page._stream.ledger.status()
		if not held:
			_problem("world-swap did not retain cancelled worker reservation")
	await process_frame
	viewports = Timing.viewports(page)


func _zoom(density: float) -> void:
	var current: Dictionary = page.camera.frame(
		int(page._world_area.size.x), int(page._world_area.size.y), 0
	)
	page.camera.zoom(
		density / (float(current.density) * float(current.live_scale)),
		page._world_area.size / 2.0,
		true
	)


func _ready_bundles() -> bool:
	if page.state.is_empty():
		return false
	var atlas: RefCounted = page.drawing.atlas
	var density := maxf(2.0, float(page.state.density))
	for asset: String in atlas._assets():
		for requested: float in [4.0, density] if float(page.state.density) >= 2.0 else [4.0]:
			var row: Dictionary = atlas._requests.get(atlas._slot(asset, requested), {})
			if row.is_empty() or atlas._service.bundle(row.token).is_empty():
				return false
	for job: Dictionary in atlas.metrics().jobs:
		if job.state != "visible":
			return false
	return true


func _snapshot() -> Dictionary:
	return {
		"stream": page.drawing.atlas.metrics(),
		"terrain": page.drawing.terrain.costs(),
		"device_memory": device.memory(),
		"thread_times": device.thread_times(),
		"gpu_ms": Timing.gpu_ms(viewports),
		"draw_cpu_ms": Timing.cpu_ms(viewports),
		"draw_calls":
		RenderingServer.get_rendering_info(
			RenderingServer.RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME
		),
		"video_bytes":
		RenderingServer.get_rendering_info(RenderingServer.RENDERING_INFO_VIDEO_MEM_USED),
		"density": page.state.get("density", 0),
		"variant_enabled": page.drawing._meadow.material.get_shader_parameter("ground_variants")
	}


func _percentile(sorted: Array, fraction: float) -> float:
	return (
		float(sorted[mini(sorted.size() - 1, maxi(0, ceili(sorted.size() * fraction) - 1))])
		if not sorted.is_empty()
		else 0.0
	)


func _stage(label: String, seconds: float, wait_for_bundles := true) -> void:
	var start := Time.get_ticks_usec()
	var last := start
	var next_sample := start
	var frames := []
	var samples := []
	var ready_us := -1
	var expected: Dictionary = page.camera.frame(
		int(page._world_area.size.x), int(page._world_area.size.y), 0
	)
	var before := _snapshot()
	device.frames_reset(1000.0 / 60.0, DisplayServer.screen_get_refresh_rate())
	device.trace_begin("stream-" + label)
	while true:
		await process_frame
		await RenderingServer.frame_post_draw
		var now := Time.get_ticks_usec()
		frames.append((now - last) / 1000.0)
		last = now
		var current_frame := (
			is_equal_approx(float(page.state.get("density", 0)), float(expected.density))
			and int(page.state.get("epoch", -1)) == int(expected.epoch)
			and int(page.state.get("revision", -1)) == int(expected.revision)
		)
		var ready := current_frame and (not wait_for_bundles or _ready_bundles())
		if ready and ready_us < 0:
			ready_us = now - start
		if now >= next_sample:
			var sample := _snapshot()
			sample["elapsed_ms"] = (now - start) / 1000.0
			samples.append(sample)
			next_sample = now + 250000
		if ready and now - start >= seconds * 1000000:
			break
		if now - start > 20000000:
			_problem(label + " timed out waiting for complete bundles")
			break
	device.trace_end()
	frames.sort()
	var stage := {
		"label": label,
		"expected_density": expected.density,
		"expected_epoch": expected.epoch,
		"expected_revision": expected.revision,
		"elapsed_ms": (Time.get_ticks_usec() - start) / 1000.0,
		"complete_ms": ready_us / 1000.0 if ready_us >= 0 else -1,
		"frame_count": frames.size(),
		"frame_p50_ms": _percentile(frames, .5),
		"frame_p95_ms": _percentile(frames, .95),
		"frame_max_ms": frames.back() if not frames.is_empty() else 0,
		"frames": device.frames(),
		"before": before,
		"after": _snapshot(),
		"samples": samples
	}
	if (
		label.begins_with("warm")
		and int(stage.after.terrain.static_builds) != int(before.terrain.static_builds)
	):
		_problem(label + " rebuilt unchanged static masks")
	report.stages.append(stage)
	print(
		"STREAM BENCH ", label, " complete=", stage.complete_ms, "ms p95=", stage.frame_p95_ms, "ms"
	)
	await RenderingServer.frame_post_draw
	var image := root.get_texture().get_image()
	if image.get_size() != Vector2i(1080, 2400):
		_problem("capture resolution differs from1080x2400")
	var saved := image.save_png(prefix + "-" + mode + "-" + label + ".png")
	if saved != OK:
		_problem("could not save " + label + " capture")


func _save_world_a() -> void:
	page.world.save_now()
	report["world_a_digest"] = page.world.digest()
	page.frozen = false
	page._save_view()
	page.frozen = true
	report["view_cache"] = page._preferences_path()


func _close() -> void:
	page.free()
	page = null
	viewports.clear()
	for frame in 3:
		await process_frame


func run() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() != 2 or not args[1] in ["run", "reopen"]:
		printerr("usage: -- /tmp/<unique-prefix> run|reopen")
		quit(2)
		return
	prefix = args[0]
	mode = args[1]
	folder = "user://stream-bench/" + prefix.sha256_text().substr(0, 16)
	report["mode"] = mode
	report["renderer"] = RenderingServer.get_current_rendering_method()
	report["world_folder"] = folder
	report["engine"] = Engine.get_version_info()
	var repo := ProjectSettings.globalize_path("res://").trim_suffix("/").get_base_dir()
	var hashes := {}
	for source: String in ["view/src", "game/fixtures", "game/terrain", "game/ui"]:
		for name: String in DirAccess.get_files_at(repo.path_join(source)):
			if name.get_extension() in ["gd", "gdshader", "gdshaderinc", "cpp", "hpp", "json"]:
				var relative := source.path_join(name)
				hashes[relative] = FileAccess.get_sha256(repo.path_join(relative))
	for relative: String in [
		"tools/stream-bench.gd",
		"game/pages/examples.gd",
		"game/pages/fixtures.gd",
		"game/game_data.gd",
		"game/data/build.toml",
		"game/bin/libkindling.linux.x86_64.so",
		"game/project.godot"
	]:
		hashes[relative] = FileAccess.get_sha256(repo.path_join(relative))
	report["source_sha256"] = hashes
	report["catalogue_sources"] = GameData.build().get_value("catalogue", "sources", [])
	DisplayServer.window_set_size(Vector2i(1080, 2400))
	root.size = Vector2i(1080, 2400)
	await process_frame
	if mode == "reopen":
		var prior: Variant = JSON.parse_string(FileAccess.get_file_as_string(prefix + "-run.json"))
		if not prior is Dictionary or not prior.has("world_a_digest"):
			printerr("reopen requires a completed run report")
			quit(2)
			return
		var cache: String = prior.view_cache
		var expected := "user://view-state/" + folder.path_join("world-a").sha256_text() + ".cfg"
		if cache != expected:
			printerr("reopen cache does not belong to this disposable run")
			quit(2)
			return
		if FileAccess.file_exists(cache):
			var removed := DirAccess.remove_absolute(ProjectSettings.globalize_path(cache))
			if removed != OK:
				_problem("could not remove disposable view cache")
		await _open(folder.path_join("world-a"))
		report["world_a_digest"] = page.world.digest()
		if report.world_a_digest != prior.world_a_digest:
			_problem("authoritative world digest changed after disposable cache deletion")
		await _stage("cache-delete-reopen", 2.0)
	else:
		if DirAccess.dir_exists_absolute(ProjectSettings.globalize_path(folder)):
			printerr("use a new output prefix for a cold route; disposable world already exists")
			quit(2)
			return
		await _open(folder.path_join("world-a"))
		_zoom(8)
		await _stage("cold-middle", 2.0)
		await _stage("warm-middle", 2.0)
		_zoom(32)
		await _stage("near", 2.0)
		page.camera.focus(100, 100)
		page.set_light("Dusk")
		await _stage("pan-away-light-change", .5)
		page.camera.focus(-1, 2)
		await _stage("reentry", 1.0)
		_zoom(.5)
		await _stage("evict-wide", 1.0)
		_zoom(32)
		await _stage("return-near", 2.0)
		_save_world_a()
		_zoom(64)
		var start := Time.get_ticks_usec()
		var service: Node = page._stream
		while service._worker == null and Time.get_ticks_usec() - start < 3000000:
			await process_frame
		if service._worker == null:
			_problem("world-swap route did not observe preparation ownership")
		var old_token: int = service._job.get("token", 0)
		report["worldswap_before"] = service.ledger.status()
		# Do not drain here: the old worker must coexist with the new world's begin.
		page.free()
		await _open(folder.path_join("world-b"), old_token)
		await _stage("worldswap", 2.0)
	await _close()
	for relative: String in report.source_sha256:
		if FileAccess.get_sha256(repo.path_join(relative)) != report.source_sha256[relative]:
			_problem("source changed during benchmark: " + relative)
	var output := FileAccess.open(prefix + "-" + mode + ".json", FileAccess.WRITE)
	if output == null:
		printerr("could not write benchmark report")
		quit(2)
		return
	output.store_string(JSON.stringify(report, "\t"))
	output.close()
	quit(0 if report.problems.is_empty() else 1)
