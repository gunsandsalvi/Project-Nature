## The phone benchmark (A18.1, PLT-04): one tap and about 19 minutes. It runs each scenario of the
## simulation's list in turn, on its own page with a world of its own kept apart from yours: the
## calendar alone, 10,000 markers at real speed and at top speed with the camera touring, the same
## pinned to the middle cores, a sweep through the zoom stops' speeds, saves with an export and a
## reopening, and a still camera. For each it measures the frames by the extension's own clock, the
## speed held, the heat, the battery, memory and the world thread's time, takes its world's digest
## at its set moment and compares it with the cloud's, and it ends with a short code to copy into
## the chat. Implements PLT-04 and RES-05.
extends VBoxContainer

const CrowdPage := preload("res://pages/crowd.gd")
const TimePage := preload("res://pages/time.gd")
## Where the benchmark keeps its worlds while it runs, apart from yours; emptied before and after.
const ROOT := "user://bench"
## Real seconds into a scenario before its frames count: its world made and its first frames drawn.
const SETTLE := 5.0
## Real seconds between readings of the phone: its heat, battery, memory and speed. Android gives
## the heat forecast at most once a second.
const EVERY := 2.0
## The saves scenario's export and reopening, in real seconds from its start.
const EXPORT_AT := 40.0
const REOPEN_AT := 55.0
## Each speed of the sweep lasts this long, in real seconds.
const SWEEP_STEP := 20.0
## A world that has not moved for this many real seconds while running on to its mark has stopped,
## and its digest is not taken.
const STUCK := 30.0
const TEXT := Palette.TEXT
const QUIET := Palette.QUIET
const GOOD := Palette.GOOD
const FAIL := Palette.FAIL

## Real time runs this many times faster in the tests; 1 on the phone.
var time_scale := 1.0
## The scenarios as the simulation lists them, the measures so far by the code's field names, the
## code once done, and every line the page shows, for the tests.
var scenarios: Array = []
var results := {}
var code := ""
var shown := PackedStringArray()

var _device := KdDevice.new()
var _index := -1
var _met := 0
var _clock := 0
var _elapsed := 0.0
var _total := 0.0
var _counted := false
var _next_read := 0.0
var _sweep := -1
var _exported := false
var _reopened := false
var _opening := 0
var _finishing := false
var _still := 0.0
var _last_frontier := -1
var _page: Control
var _digests := {}
var _cpu_at := {}
var _samples := {}
var _status: Label
var _run: Button
var _host: VBoxContainer
var _list: VBoxContainer


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 10)
	# worlds left by a run that was stopped part way
	Worlds.remove_tree(ROOT)
	scenarios = _device.bench_scenarios()
	_status = _label(16, TEXT)
	_status.text = (
		(
			"Unplug the phone, turn on flight mode and let it cool, then tap Run. It takes about"
			+ " %d minutes; leave the phone alone until the code shows."
		)
		% (roundi(_planned() / 60.0) + 2)
	)
	_run = Button.new()
	_run.text = "Run"
	_run.custom_minimum_size = Vector2(0, 56)
	_run.pressed.connect(start)
	add_child(_run)
	# the scenario's page above, three quarters of the height; each one's results below
	_host = VBoxContainer.new()
	_host.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_host.size_flags_stretch_ratio = 3.0
	add_child(_host)
	var scroll := ScrollContainer.new()
	scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	add_child(scroll)
	_list = VBoxContainer.new()
	_list.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	scroll.add_child(_list)
	set_process(false)
	# the cloud's picture of a finished run (tools/picture.sh game ... -- Bench quick): a run a
	# hundred times faster, the picture held back until it ends
	if "quick" in OS.get_cmdline_user_args():
		time_scale = 0.01
		add_to_group("busy")
		start()


func _exit_tree() -> void:
	# a run left part way, as another page opens: the screen may sleep again, and the scenario's
	# trace section ends
	if running():
		DisplayServer.screen_set_keep_on(false)
		_device.trace_end()


## Whether the benchmark is running.
func running() -> bool:
	return _index >= 0 and _index < scenarios.size()


## Starts the benchmark: the phone's own lines first, then each scenario in turn.
func start() -> void:
	if running():
		return
	_run.visible = false
	Worlds.remove_tree(ROOT)
	DisplayServer.screen_set_keep_on(true)
	results = _phone_lines()
	code = ""
	_met = 0
	_total = 0.0
	_clock = Time.get_ticks_usec()
	_open(0)
	set_process(true)


func _process(_delta: float) -> void:
	# real time by the steady clock, since Godot's delta is smoothed (A3.9)
	var now := Time.get_ticks_usec()
	var real := (now - _clock) / 1.0e6
	_clock = now
	_total += real
	var s: Dictionary = scenarios[_index]
	if _finishing:
		_finish(s, real)
		return
	_elapsed += real
	var t := _elapsed / time_scale
	if not _counted and t >= SETTLE:
		_counted = true
		_device.frames_reset(1000.0 / 60.0, DisplayServer.screen_get_refresh_rate())
		_cpu_at = _device.thread_times()
		if _crowd() != null:
			_crowd().draw_times_reset()
	if s["camera"] == "tour":
		_page.tour(t)
	if s["speed"] == "sweep":
		var step := mini(int(t / SWEEP_STEP), _page.bar.speeds.size() - 1)
		if step != _sweep:
			_sweep = step
			_page.bar.choose_speed(step)
	if s["saves"]:
		if not _exported and t >= EXPORT_AT:
			_export(s)
		if not _reopened and t >= REOPEN_AT:
			_reopen(s)
		_opened(s)
	if t >= _next_read:
		_next_read = t + EVERY
		_read(s, t)
	_status.text = (
		"%d of %d: %s\n%d s left in it"
		% [_index + 1, scenarios.size(), s["about"], maxi(0, int(s["seconds"]) - int(t))]
	)
	if t >= float(s["seconds"]):
		_measure(s)


## A scenario's page, made new on a world of its own, at its speed, pinned if it is, its digest
## marked, and its camp called home at its moment unless the world it reopens had already done so.
func _open(i: int) -> void:
	_index = i
	_elapsed = 0.0
	_counted = false
	_next_read = 0.0
	_sweep = -1
	_exported = false
	_reopened = false
	_opening = 0
	_finishing = false
	_digests = {}
	_samples = {
		"speed": [],
		"heat": [],
		"heat_at": [],
		"share": [],
		"current": [],
		"memory": [],
		"save_ms": [],
		"clock": [],
		"gpu_ms": [],
		"gpu_headroom": [],
		"power": [],
		"draws": [],
		"triangles": [],
		"video_mb": [],
	}
	var s: Dictionary = scenarios[i]
	_device.trace_begin("kd bench %s" % s["name"])
	_page = _make_page(s)


func _make_page(s: Dictionary) -> Control:
	var page: Control
	if s["ground"] == "calendar":
		page = TimePage.new()
	else:
		page = CrowdPage.new()
		page.root = ROOT
		page.folder = ROOT.path_join(s["name"])
	_host.add_child(page)
	var speed: int = page.bar.speeds.size() - 1
	if s["speed"] == "sweep":
		speed = 0
	elif s["speed"] != "top":
		speed = SpeedBar.STOPS.find(s["speed"])
	page.bar.choose_speed(speed)
	if s["pinned"]:
		page.world.set_pinned(_device.middle_cores())
	page.world.mark(s["mark"])
	var call_at: int = s["call_at"]
	if call_at > 0 and int(page.opened.get("was_at", 0)) < call_at:
		page.world.call_home_at(s["call_camp"], call_at)
	return page


## The page's world closed, as switching pages closes it, saved as it leaves; the digests it took
## are kept.
func _close_page() -> void:
	_host.remove_child(_page)
	_digests.merge(_page.world.marks())
	_page.free()
	_page = null


## The phone read: its heat forecast, the working share the heat allows, its battery's current, the
## app's memory, the fastest core's clock, the slowest save's pause, and the speed shown in the
## scenario's last minute, after 3 minutes at top speed (PLT-04).
func _read(s: Dictionary, t: float) -> void:
	var c: Dictionary = _page.world.counters()
	var heat := -1.0
	if s["ground"] == "calendar":
		var thermal := _device.thermal()
		if thermal.get("available", false):
			heat = float(thermal["forecast_10s"])
	else:
		# the Crowd page asks the phone itself, as often as Android allows
		heat = _page.forecast
	if heat >= 0.0:
		_samples["heat"].append(heat)
		_samples["heat_at"].append(t)
	if c.has("share"):
		_samples["share"].append(float(c["share"]) * 100.0)
	if int(c.get("saves", 0)) > 0:
		_samples["save_ms"].append(float(c["save_ms"]))
	var battery := Phone.battery()
	if battery.has("current_ma"):
		_samples["current"].append(float(battery["current_ma"]))
	var memory := _device.memory()
	if memory.has("resident_mb"):
		_samples["memory"].append(float(memory["resident_mb"]))
	# the fastest core's clock now, where the system shows it: it falls as the phone throttles
	var clock := -1
	for khz: int in _device.clocks():
		clock = maxi(clock, khz)
	if clock > 0:
		_samples["clock"].append(clock / 1000.0)
	if s["speed"] != "sweep" and t >= float(s["seconds"]) - 60.0:
		_samples["speed"].append(float(c.get("speed_shown", 0.0)))
	_read_graphics()


## The graphics engine's readings (A18.1, PLT-04): the graphics chip's time for every viewport
## drawn, Godot's counts of draws, triangles and video memory, the chip's headroom where Android
## gives it, and the power drawn from the battery.
func _read_graphics() -> void:
	var gpu := Timing.gpu_ms(Timing.viewports(_page if _page != null else self))
	if gpu > 0.0:
		_samples["gpu_ms"].append(gpu)
	var draws := RenderingServer.RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME
	var triangles := RenderingServer.RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME
	var video := RenderingServer.RENDERING_INFO_VIDEO_MEM_USED
	_samples["draws"].append(float(RenderingServer.get_rendering_info(draws)))
	_samples["triangles"].append(RenderingServer.get_rendering_info(triangles) / 1000.0)
	_samples["video_mb"].append(RenderingServer.get_rendering_info(video) / 1048576.0)
	var headroom := _device.gpu_headroom()
	if headroom.get("available", false):
		_samples["gpu_headroom"].append(float(headroom["headroom"]))
	var power := Phone.power(_device)
	if power.has("watts"):
		_samples["power"].append(float(power["watts"]))


## The world saved and written out as one .kindling file, as the Worlds page exports one: the time
## the file took.
func _export(s: Dictionary) -> void:
	_exported = true
	var world: KdWorld = _page.world
	world.pause()
	world.save_now()
	var started := Time.get_ticks_usec()
	var worlds := Worlds.at(ROOT)
	var file := FileAccess.open(ROOT.path_join("export.kindling"), FileAccess.WRITE)
	if worlds.export_begin(s["name"]) >= 0 and file != null:
		var piece := worlds.export_next(1 << 20)
		while not piece.is_empty():
			file.store_buffer(piece)
			piece = worlds.export_next(1 << 20)
		file.close()
		results[s["name"] + ".export_ms"] = (Time.get_ticks_usec() - started) / 1000.0
	world.play()


## The world closed and opened again, timed until it shows where it was.
func _reopen(s: Dictionary) -> void:
	_reopened = true
	_close_page()
	_opening = Time.get_ticks_usec()
	_page = _make_page(s)
	_opened(s)


## The crowd's drawing on the scenario's page, or nothing on the calendar's.
func _crowd() -> KdCrowd:
	return _page.crowd if "crowd" in _page else null


func _opened(s: Dictionary) -> void:
	if _opening > 0 and not _page.world.catching_up():
		results[s["name"] + ".open_ms"] = (Time.get_ticks_usec() - _opening) / 1000.0
		_opening = 0


## A scenario's timed part ends: its frames, speed, heat, battery, memory and thread time. Then its
## world runs on to its mark, if it has not passed it, the screen still.
func _measure(s: Dictionary) -> void:
	var n: String = s["name"]
	var frames := _device.frames()
	if int(frames["frames"]) > 0:
		results[n + ".on_time"] = float(frames["on_time"]) / float(frames["frames"])
		results[n + ".slowest"] = float(frames["slowest_ms"])
		results[n + ".stalls"] = float(frames["stalls"])
	for key: String in ["speed", "current", "clock", "gpu_ms", "power"]:
		if not _samples[key].is_empty():
			results["%s.%s" % [n, key]] = _mean(_samples[key])
	for key: String in ["heat", "memory", "save_ms", "draws", "triangles", "video_mb"]:
		if not _samples[key].is_empty():
			results["%s.%s" % [n, key]] = _samples[key].max()
	for key: String in ["share", "gpu_headroom"]:
		if not _samples[key].is_empty():
			results["%s.%s" % [n, key]] = _samples[key].min()
	var to_light := minutes_to_light(
		_samples["heat_at"], _samples["heat"], results.get("light", -1.0)
	)
	if to_light >= 0.0:
		results[n + ".to_light"] = to_light
	if _crowd() != null and int(_crowd().draw_times()["draws"]) > 0:
		results[n + ".draw_ms"] = float(_crowd().draw_times()["mean_ms"])
	var cpu := _device.thread_times()
	var thread := "kd-world" if s["ground"] == "calendar" else "kd-crowd"
	var counted := _elapsed - SETTLE * time_scale
	if cpu.has(thread) and _cpu_at.has(thread) and counted > 0.0:
		results[n + ".cpu"] = (float(cpu[thread]) - float(_cpu_at[thread])) / counted * 100.0
	_finishing = true
	_still = 0.0
	_last_frontier = -1
	_page.set_process(false)
	_page.world.reach(s["mark"])
	_finish(s, 0.0)


## Once a frame while the world runs on: once it has passed its mark, its digest is compared with
## the cloud's and the next scenario opens.
func _finish(s: Dictionary, real: float) -> void:
	var mark: int = s["mark"]
	var world: KdWorld = _page.world
	_digests.merge(world.marks())
	var frontier := world.frontier()
	_still = 0.0 if frontier != _last_frontier else _still + real
	_last_frontier = frontier
	if not _digests.has(mark) and _still < STUCK:
		_status.text = (
			"%d of %d: %s\nrunning its world on to %s, to compare it with the cloud's"
			% [_index + 1, scenarios.size(), s["about"], KdWorld.moment_text(mark)]
		)
		return
	_finishing = false
	var n: String = s["name"]
	if _digests.has(mark):
		var digest: String = _digests[mark]
		var cloud: String = GameData.build().get_value("bench", n, "")
		results[n + ".digest"] = 1 if digest == cloud else 2
		results[n + ".digest_bits"] = digest.left(5).hex_to_int()
	_close_page()
	_device.trace_end()
	var summary := _summary(s)
	if summary["met"]:
		_met += 1
	_line(summary["text"], 15, TEXT if summary["met"] else FAIL)
	if _index + 1 < scenarios.size():
		_open(_index + 1)
	else:
		_end()


func _end() -> void:
	_index = scenarios.size()
	set_process(false)
	results["seconds"] = _total / time_scale
	code = _device.bench_code(results)
	DisplayServer.screen_set_keep_on(false)
	Worlds.remove_tree(ROOT)
	_host.visible = false
	remove_from_group("busy")
	_status.text = (
		"Done: %d of %d scenarios met every line. Copy the code into the chat:"
		% [_met, scenarios.size()]
	)
	var shown_code := Label.new()
	# spaces rather than dashes between its groups, so the lines break between them
	shown_code.text = code.replace("-", " ")
	shown_code.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	shown_code.add_theme_font_size_override("font_size", 20)
	# green only when every scenario met every line
	shown_code.add_theme_color_override(
		"font_color", GOOD if _met == scenarios.size() else Palette.WARN
	)
	add_child(shown_code)
	move_child(shown_code, 1)
	shown.append(code)
	var copy := Button.new()
	copy.text = "Copy the code"
	copy.custom_minimum_size = Vector2(0, 56)
	copy.pressed.connect(func() -> void: DisplayServer.clipboard_set(code))
	add_child(copy)
	move_child(copy, 2)


## The phone's own lines: its build, cores, screen, Android, battery and heat forecast.
func _phone_lines() -> Dictionary:
	var out := {}
	out["build"] = float(GameData.build().get_value("build", "code", 0))
	var cores := _device.cores()
	out["cores"] = float(cores.size())
	var fastest := 0
	for core: Dictionary in cores:
		fastest = maxi(fastest, int(core["max_khz"]))
	if fastest > 0:
		out["big_mhz"] = fastest / 1000.0
	var refresh := DisplayServer.screen_get_refresh_rate()
	if refresh > 0.0:
		out["refresh_hz"] = refresh
	if Phone.android_version() > 0:
		out["android"] = float(Phone.android_version())
	var battery := Phone.battery()
	if battery.has("percent"):
		out["battery"] = float(battery["percent"])
	if battery.has("plugged"):
		out["plugged"] = 2.0 if battery["plugged"] else 1.0
	var thermal := _device.thermal()
	out["thermal"] = 1.0 if thermal.get("available", false) else 2.0
	for level: String in ["light", "moderate"]:
		if thermal.has(level):
			out[level] = float(thermal[level])
	out["gpu_offered"] = 1.0 if _device.gpu_headroom().get("available", false) else 2.0
	return out


## Minutes until the heat forecast reaches the light throttling level at the rate it rose over the
## readings, by a straight line through them; 500 when it did not rise, and -1 without the level or
## two readings (PLT-04).
static func minutes_to_light(seconds: Array, heat: Array, light: float) -> float:
	if light <= 0.0 or heat.size() < 2:
		return -1.0
	var n := float(heat.size())
	var mean_t := 0.0
	var mean_h := 0.0
	for i in heat.size():
		mean_t += float(seconds[i]) / n
		mean_h += float(heat[i]) / n
	var spread := 0.0
	var together := 0.0
	for i in heat.size():
		spread += (float(seconds[i]) - mean_t) * (float(seconds[i]) - mean_t)
		together += (float(seconds[i]) - mean_t) * (float(heat[i]) - mean_h)
	if spread <= 0.0 or together <= 0.0:
		return 500.0
	var per_second := together / spread
	var last := float(heat[heat.size() - 1])
	if last >= light:
		return 0.0
	return minf(499.0, (light - last) / per_second / 60.0)


## A scenario's results in a line, each measure that has a pass line held to it as the cloud's
## decoder holds it (A18.1, RES-09), and whether it met every line, its digest included.
func _summary(s: Dictionary) -> Dictionary:
	var n: String = s["name"]
	var words := PackedStringArray()
	var met := int(results.get(n + ".digest", 0)) == 1
	if results.has(n + ".on_time"):
		var thousandths := roundi(results[n + ".on_time"] * 1000.0)
		var text := "%.1f%% of frames on time" % (thousandths / 10.0)
		if int(s["on_time"]) > 0:
			var held := thousandths >= int(s["on_time"])
			met = met and held
			text += " (%s%% needed%s)" % [int(s["on_time"]) / 10, "" if held else ", MISSED"]
		words.append(text)
	if results.has(n + ".slowest"):
		var slowest := roundi(results[n + ".slowest"])
		var text := "slowest %d ms" % slowest
		if int(s["slowest"]) > 0:
			var held := slowest <= int(s["slowest"])
			met = met and held
			text += " (%d at most%s)" % [int(s["slowest"]), "" if held else ", MISSED"]
		words.append(text)
	if results.has(n + ".speed"):
		words.append(SpeedBar.speed_words(results[n + ".speed"], true))
	if results.has(n + ".open_ms"):
		var open := roundi(results[n + ".open_ms"])
		var held := open <= int(s["open"])
		met = met and held
		words.append(
			"reopened in %d ms (%d at most%s)" % [open, int(s["open"]), "" if held else ", MISSED"]
		)
	match int(results.get(n + ".digest", 0)):
		1:
			words.append("its world ended as the cloud's")
		2:
			words.append("its world did NOT end as the cloud's")
		_:
			words.append("its world stopped before its digest")
	return {"text": "%s: %s" % [n, ", ".join(words)], "met": met}


func _planned() -> float:
	var seconds := 0.0
	for s: Dictionary in scenarios:
		seconds += float(s["seconds"])
	return seconds


static func _mean(values: Array) -> float:
	var sum := 0.0
	for v: float in values:
		sum += v
	return sum / values.size()


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label


func _line(text: String, font_size: int, colour: Color) -> void:
	var label := Label.new()
	label.text = text
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	_list.add_child(label)
	shown.append(text)
