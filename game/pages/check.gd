## The self-check (A2.3): what the phone is and what it computes, each line green when it holds,
## amber when it is worth a look, red when it fails, and every line copied with one tap for the
## chat. Implements PLT-01 and RES-05: the phone computes the same bits as the cloud, on one thread
## and four; and MAT-13: it reads the same catalogues.
extends VBoxContainer

const BUILD_DATA := "res://data/build.toml"
const COLOURS := {
	"ok": Palette.GOOD,
	"warn": Palette.WARN,
	"fail": Palette.FAIL,
	"info": Palette.QUIET,
}
const MARKS := {"ok": "✔", "warn": "!", "fail": "✘", "info": "·"}

## The lines, each {name, value, state}, state being "ok", "warn", "fail" or "info".
var lines: Array[Dictionary] = []

var _list: VBoxContainer
var _summary: Label
var _probes: Probes


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	_summary = Label.new()
	_summary.add_theme_font_size_override("font_size", 18)
	_summary.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	add_child(_summary)
	var copy := Button.new()
	copy.text = "Copy the details"
	copy.custom_minimum_size = Vector2(0, 48)
	copy.pressed.connect(func() -> void: DisplayServer.clipboard_set(details()))
	add_child(copy)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_list.add_theme_constant_override("separation", 6)
	scroll.add_child(_list)
	# the probes run once a build, a little drawing each, and the lines show them as they end
	_probes = Probes.new()
	add_child(_probes)
	_probes.finished.connect(_on_probes_done)
	build()
	_show()
	# a picture of the page in the cloud waits for the probes (tools/godot-picture.gd)
	add_to_group("busy")
	_probes.start()


func _on_probes_done() -> void:
	remove_from_group("busy")
	lines = lines.filter(
		func(line: Dictionary) -> bool: return not line["name"].begins_with("Probe: ")
	)
	_add_probes()
	_show()


## Runs every check and fills the lines.
func build() -> void:
	lines.clear()
	var device := KdDevice.new()
	var godot := Engine.get_version_info()
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	_add("App", "Kindling %s on Godot %s" % [version, godot["string"]], "info")
	_add("Simulation built with", device.built_with(), "info")
	_add_graphics()
	_add_screen()
	_add_cores(device)
	_add_heat(device)
	_add_graphics_readings(device)
	_add_probes()
	_add_threads(device)
	_add("Storage", _short_mount(device.storage()), "info")
	_add_same_bits(device)
	_add_catalogue()
	_add_saves()


## Every line as plain text, for the chat.
func details() -> String:
	var version: String = ProjectSettings.get_setting("application/config/version", "")
	var out := PackedStringArray(["Kindling %s self-check: %s" % [version, summary()]])
	for line in lines:
		out.append("%s %s: %s" % [MARKS[line["state"]], line["name"], line["value"]])
	return "\n".join(out)


## "All checks pass", or how many fail.
func summary() -> String:
	var failed := (
		lines.filter(func(line: Dictionary) -> bool: return line["state"] == "fail").size()
	)
	if failed == 0:
		return "all checks pass"
	return "%d %s" % [failed, "check fails" if failed == 1 else "checks fail"]


func _add(name: String, value: String, state: String) -> void:
	lines.append({"name": name, "value": value, "state": state})


func _add_graphics() -> void:
	var adapter := (
		"%s, %s, Vulkan %s"
		% [
			RenderingServer.get_video_adapter_name(),
			RenderingServer.get_video_adapter_vendor(),
			RenderingServer.get_video_adapter_api_version(),
		]
	)
	_add("Graphics", adapter, "info")
	var parts := PackedStringArray()
	for part in OS.get_video_adapter_driver_info():
		if part != "":
			parts.append(part)
	var rd := RenderingServer.get_rendering_device()
	if rd != null:
		var cache_id := rd.get_device_pipeline_cache_uuid()
		parts.append("pipeline cache %s%s" % [cache_id, _as_vulkan_version(cache_id)])
	_add("Driver", "; ".join(parts) if parts.size() > 0 else "not reported", "info")


## The first 32 bits of the pipeline cache's id read as Vulkan packs a version (A2.3): where the
## driver puts its version there, this shows it; some makers pack theirs differently, so the raw id
## stays beside it.
func _as_vulkan_version(cache_id: String) -> String:
	var hex := cache_id.replace("-", "")
	if hex.length() < 8 or not hex.substr(0, 8).is_valid_hex_number():
		return ""
	var v := hex.substr(0, 8).hex_to_int()
	return " (as Vulkan packs it: %d.%d.%d)" % [(v >> 22) & 0x7F, (v >> 12) & 0x3FF, v & 0xFFF]


func _add_screen() -> void:
	var rate := DisplayServer.screen_get_refresh_rate()
	var size := DisplayServer.screen_get_size()
	var value := "%d × %d, %s Hz, frame cap %d" % [size.x, size.y, _rate_text(rate), Engine.max_fps]
	var state := "info"
	if OS.get_name() == "Android":
		state = "ok" if not is_nan(rate) and absf(rate - 60.0) < 1.0 else "warn"
	_add("Screen", value, state)


func _rate_text(rate: float) -> String:
	return "unknown" if is_nan(rate) or rate <= 0.0 else "%.0f" % rate


func _add_cores(device: KdDevice) -> void:
	var by_clock := {}
	for core: Dictionary in device.cores():
		var mhz := int(core["max_khz"]) / 1000
		by_clock[mhz] = int(by_clock.get(mhz, 0)) + 1
	var parts := PackedStringArray()
	var clocks := by_clock.keys()
	clocks.sort()
	for mhz: int in clocks:
		var label := (
			"%d at %.2f GHz" % [by_clock[mhz], mhz / 1000.0] if mhz > 0 else "%d" % by_clock[mhz]
		)
		parts.append(label)
	var middle := device.middle_cores()
	var value := (
		"%s; middle cores %s"
		% [", ".join(parts), str(Array(middle)) if middle.size() > 0 else "none found"]
	)
	_add("Cores", value, "info")


func _add_heat(device: KdDevice) -> void:
	var heat := device.thermal()
	if not heat.get("available", false):
		_add("Heat", "not read on this device", "info")
		return
	var value := (
		"headroom %.2f now, %.2f in 10 s, status %d"
		% [heat["headroom"], heat["forecast_10s"], heat["status"]]
	)
	_add("Heat", value, "ok" if int(heat["status"]) == 0 else "warn")
	var levels := PackedStringArray()
	for level: String in ["light", "moderate", "severe"]:
		if heat.has(level):
			levels.append("%s %.2f" % [level, float(heat[level])])
	_add(
		"Heat thresholds",
		", ".join(levels) if levels.size() > 0 else "not given (Android 15 gives them)",
		"info"
	)
	if int(heat.get("listener_calls", 0)) > 0:
		_add(
			"Heat listener",
			(
				"%d calls, last %.2f now and %.2f in %d s"
				% [
					heat["listener_calls"],
					heat["listener_headroom"],
					heat["listener_forecast"],
					heat["listener_seconds"],
				]
			),
			"info"
		)


## What the look leans on from the graphics driver and the phone (A4.7): the shading rates the
## driver offers, the graphics chip's headroom, and the power rails. Implements VIS-14.
func _add_graphics_readings(device: KdDevice) -> void:
	var rates := device.shading_rates()
	var value := "not asked on this device"
	if rates.get("available", false):
		if not rates.get("extension", false):
			value = "none offered"
		else:
			value = (
				"for each draw %s, each triangle %s, from a picture %s; rates %s"
				% [
					_yes(rates.get("per_draw", false)),
					_yes(rates.get("per_primitive", false)),
					_yes(rates.get("from_picture", false)),
					", ".join(rates.get("rates", PackedStringArray())),
				]
			)
	_add("Shading rates", value, "info")
	var gpu := device.gpu_headroom()
	var no_headroom := {
		"unsupported": "not offered by this phone",
		"too soon": "not read: asked too soon after the last reading",
		"older Android": "not offered before Android 16",
		"off Android": "not asked off Android",
	}
	_add(
		"Graphics headroom",
		(
			"%.0f of 100" % float(gpu["headroom"])
			if gpu.get("available", false)
			else no_headroom.get(gpu.get("why", ""), "not read: the phone could not give it")
		),
		"info"
	)
	var power := Phone.power(device)
	var watts := "not read"
	if power.has("watts"):
		watts = (
			"%.2f W now, %s"
			% [power["watts"], "voltage measured" if power["measured_voltage"] else "at 3.85 V"]
		)
	_add("Power", watts, "info")
	_add(
		"Power rails",
		"not read: Android gives them only to Java callbacks, which the app has not yet",
		"info"
	)


func _yes(on: bool) -> String:
	return "yes" if on else "no"


## The probes of the graphics features the look relies on, each tried once a build (A4.7): passed,
## failed or closing the app. GPU particles are expected to fail.
func _add_probes() -> void:
	if _probes == null:
		return
	for name: String in Probes.ABOUT:
		var state: String = _probes.results.get(name, "")
		var expected_failure := name == "gpu_particles"
		var shown := "waiting" if state.is_empty() else state
		var mark := "info"
		if state == "passed":
			mark = "ok"
		elif state == "crashed" or state.begins_with("failed"):
			mark = "info" if expected_failure else "fail"
		_add("Probe: " + Probes.ABOUT[name], shown, mark)


func _add_threads(device: KdDevice) -> void:
	var t := device.thread_check()
	var value := (
		"main thread %s; a new thread finds %s and works in %s; stack %d MiB"
		% [t["main_thread"], t["inherited"], t["in_work"], t["stack_mib"]]
	)
	var good: bool = t["default_in_work"] and int(t["stack_mib"]) >= 8
	_add("Simulation threads", value, "ok" if good else "fail")


## Each proof suite on one thread and four, against the digest the cloud wrote into the build, with
## the time each run took.
func _add_same_bits(device: KdDevice) -> void:
	var expected := ConfigFile.new()
	var have_build := expected.load(BUILD_DATA) == OK
	for suite: String in device.proof_suites():
		var started := Time.get_ticks_usec()
		var one := device.proof(suite, 1)
		var one_ms := (Time.get_ticks_usec() - started) / 1000.0
		started = Time.get_ticks_usec()
		var four := device.proof(suite, 4)
		var four_ms := (Time.get_ticks_usec() - started) / 1000.0
		var cloud: String = expected.get_value("proof", suite, "") if have_build else ""
		var times := "%.0f ms on 1 thread, %.0f on 4" % [one_ms, four_ms]
		if one == four and one == cloud:
			_add("Same bits: %s" % suite, "the same as the cloud (%s); %s" % [one, times], "ok")
			continue
		var value := (
			"%s on 1 thread, %s on 4, the cloud's %s; %s"
			% [one, four, cloud if cloud != "" else "missing", times]
		)
		_add("Same bits: %s" % suite, value, "fail")


## The catalogue the phone reads against the one the cloud checked (A2.3, A3.6): the same files,
## byte for byte, and the same digests from the simulation's own reading of them.
func _add_catalogue() -> void:
	var build_file := GameData.build()
	var listed := GameData.files(build_file)
	if listed.is_empty():
		_add("Catalogues", "the build lists no catalogue files", "fail")
		return
	var differ := PackedStringArray()
	for file: Dictionary in listed:
		if FileAccess.get_sha256("res://data/" + str(file["path"])) != file["sha256"]:
			differ.append(file["path"])
	var loaded := KdWorld.new().load_catalogue(GameData.paths(build_file))
	var problems: PackedStringArray = loaded["problems"]
	var mine := GameData.source_lines(loaded)
	var theirs := PackedStringArray(build_file.get_value("catalogue", "sources", []))
	var counts := (
		"%d sources, %d kinds, %d entries; %d files read in %.1f ms"
		% [
			mine.size(),
			(loaded["kinds"] as Array).size(),
			GameData.entry_count(loaded),
			loaded["files"],
			int(loaded["microseconds"]) / 1000.0,
		]
	)
	if differ.is_empty() and problems.is_empty() and mine == theirs:
		_add("Catalogues", "the same as the build: " + counts, "ok")
		return
	var why := PackedStringArray()
	if not differ.is_empty():
		why.append("files unlike the build's: " + ", ".join(differ))
	if not problems.is_empty():
		why.append("problems: " + "; ".join(problems))
	if mine != theirs:
		why.append("digests %s here, %s in the build" % [" | ".join(mine), " | ".join(theirs)])
	_add("Catalogues", "; ".join(why), "fail")


func _short_mount(line: String) -> String:
	var fields := line.split(" ")
	if fields.size() < 4:
		return line if line != "" else "not read"
	return "%s on %s (%s)" % [fields[2], fields[1], fields[3]]


func _show() -> void:
	var failed := lines.any(func(line: Dictionary) -> bool: return line["state"] == "fail")
	var said := summary()
	_summary.text = said.substr(0, 1).to_upper() + said.substr(1)
	_summary.add_theme_color_override("font_color", COLOURS["fail" if failed else "ok"])
	for row in _list.get_children():
		row.queue_free()
	for line in lines:
		var row := HBoxContainer.new()
		row.add_theme_constant_override("separation", 8)
		var mark := Label.new()
		mark.text = MARKS[line["state"]]
		mark.custom_minimum_size = Vector2(20, 0)
		mark.add_theme_color_override("font_color", COLOURS[line["state"]])
		row.add_child(mark)
		var text := Label.new()
		text.text = "%s: %s" % [line["name"], line["value"]]
		text.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
		text.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		text.add_theme_font_size_override("font_size", 14)
		row.add_child(text)
		_list.add_child(row)


## The world the Crowd page opens, as last saved (PLT-07): its name, how long ago, at what moment
## of its time, and how large it is; and the free space, amber when the phone is nearly full
## (PLT-10).
func _add_saves() -> void:
	var worlds := Worlds.at()
	var id := Worlds.current_id(worlds)
	var listed: Dictionary = {}
	for w: Dictionary in worlds.list():
		if w["id"] == id:
			listed = w
	if listed.is_empty() or int(listed["moment"]) < 0:
		_add("Saves", "no world saved yet: open Crowd", "info")
	else:
		var ago := maxi(0, int(Time.get_unix_time_from_system()) - int(listed["saved"]))
		_add(
			"Saves",
			(
				"%s, %s ago, at %s, %s"
				% [
					Worlds.name_of(listed),
					Worlds.ago_words(ago),
					listed["moment_text"],
					Worlds.size_words(listed["size"]),
				]
			),
			"ok"
		)
	var free := worlds.free_space()
	var free_mb := free / 1048576 if free >= 0 else -1
	var tuning := KdWorld.new()
	GameData.load_into(tuning)
	var warn_below := int(tuning.entry("tuning/saves", "base:saves").get("warn_below", 1024))
	var warning := Worlds.space_warning(free_mb, warn_below)
	_add(
		"Free space",
		warning if warning != "" else Worlds.size_words(free),
		"warn" if warning != "" else "info"
	)
