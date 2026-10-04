## Measure for a screen with runs (PLT-04): each run sets the view, holds a frame rate for its
## seconds while the camera turns, and keeps the graphics time and the share of frames on time; the
## phone's forecast of its heat is read every READ_EVERY seconds. At the end, one line for the chat,
## as P1's Measure ends too. Used by P2 and P3. Pre-production code (research 00).
extends RefCounted

## Where the phone's heat is read: the forecast this many seconds ahead, every READ_EVERY seconds.
const FORECAST := 30
const READ_EVERY := 10.0

## The run under way, or -1.
var index := -1
var _screen: Control
var _name: String
var _runs: Array
var _clock := 0.0
var _samples := PackedFloat32Array()
var _late := 0
var _results := PackedStringArray()
var _heat := []
var _read_clock := 0.0
var _crawl := 0


## runs: each [label, frames a second, seconds, setup to call first].
func _init(screen: Control, name: String, runs: Array) -> void:
	_screen = screen
	_name = name
	_runs = runs


func start() -> void:
	# free turns while Measure turns the view: "ease" would pull it back to a whole step each frame
	_crawl = _screen.crawl
	_screen.crawl = 0
	_results = PackedStringArray()
	_heat = []
	_read_clock = READ_EVERY
	_begin(0)


func _begin(i: int) -> void:
	index = i
	var run: Array = _runs[i]
	if (run[3] as Callable).is_valid():
		(run[3] as Callable).call()
	Engine.max_fps = run[1]
	_clock = 0.0
	_samples = PackedFloat32Array()
	_late = 0
	_screen._readout.text = (
		"Measuring %d of %d: %s, %d frames a second" % [i + 1, _runs.size(), run[0], run[1]]
	)


func step(delta: float) -> void:
	var run: Array = _runs[index]
	_clock += delta
	_screen.yaw = wrapf(_screen.yaw + 12.0 * delta, -180.0, 180.0)
	_screen._apply_camera()
	_read_clock += delta
	if _read_clock >= READ_EVERY:
		_read_clock = 0.0
		_heat.append(thermal())
	if _clock >= 1.0:
		_samples.append(_screen.graphics_time())
		if delta > 1.15 / float(run[1]):
			_late += 1
	if _clock < float(run[2]):
		return
	_results.append(summary(run[0], _samples, _late))
	if index + 1 < _runs.size():
		_begin(index + 1)
		return
	index = -1
	Engine.max_fps = 0
	_screen.crawl = _crawl
	_screen._rest_yaw = _screen.yaw
	finish(_screen, _name, _results, _heat)


## The end of a Measure: its line for the chat, copied, printed and shown until the next touch; the
## cloud's "measure" run quits there.
static func finish(screen: Control, name: String, runs: PackedStringArray, heat: Array) -> void:
	var main: GDScript = load("res://main.gd")
	var art: Vector2i = screen._art.size
	# the picture's size in art pixels and the screen's rate, which the results depend on
	var head := (
		"%s %s %s %d×%d %s Hz"
		% [
			name,
			ProjectSettings.get_setting("application/config/version", ""),
			main.facts().phone,
			art.x - 2,
			art.y - 2,
			main.refresh_rate()
		]
	)
	var line := code(head, runs, heat)
	DisplayServer.clipboard_set(line)
	print(line)
	if "measure" in OS.get_cmdline_user_args():
		screen.get_tree().quit()
	screen._readout.text = (
		"Copied for the chat (graphics ms, average/slowest 5%%, frames on time):\n%s" % line
	)


## One run's result: the graphics time's average and slowest 5%, and the share of frames on time.
static func summary(label: String, samples: PackedFloat32Array, late: int) -> String:
	var sorted := samples.duplicate()
	sorted.sort()
	var n := maxi(sorted.size(), 1)
	var total := 0.0
	for g in sorted:
		total += g
	var slow: float = sorted[mini(int(n * 0.95), n - 1)] if sorted.size() > 0 else 0.0
	return "%s %.1f/%.1f %d%%" % [label, total / n, slow, roundi(100.0 * (n - late) / n)]


## The line for the chat: the runs, then the heat as the forecast headroom at the first and last
## readings (1 is where the phone starts to slow itself) and the worst thermal status (0 none,
## 1 light, 2 moderate, 3 severe), or "?" where the phone gives none; nothing where it wasn't read.
static func code(head: String, runs: PackedStringArray, heat: Array) -> String:
	if heat.is_empty():
		return "%s | %s" % [head, " | ".join(runs)]
	var known := heat.filter(func(h: Vector2) -> bool: return h.x >= 0.0)
	var text := "heat ?"
	if not known.is_empty():
		var worst := 0
		for h: Vector2 in known:
			worst = maxi(worst, int(h.y))
		text = "heat %.2f→%.2f s%d" % [known[0].x, known[-1].x, worst]
	return "%s | %s | %s" % [head, " | ".join(runs), text]


## The phone's forecast of its heat FORECAST seconds ahead, and its thermal status, from
## Android's PowerManager through Godot's Android runtime; (-1, -1) where there is none.
static func thermal() -> Vector2:
	if OS.get_name() != "Android" or not Engine.has_singleton("AndroidRuntime"):
		return Vector2(-1.0, -1.0)
	var runtime: Object = Engine.get_singleton("AndroidRuntime")
	var context: Object = runtime.call("getApplicationContext")
	if context == null:
		return Vector2(-1.0, -1.0)
	var power: Object = context.call("getSystemService", "power")
	if power == null:
		return Vector2(-1.0, -1.0)
	var headroom: Variant = power.call("getThermalHeadroom", FORECAST)
	var status: Variant = power.call("getCurrentThermalStatus")
	var h: float = headroom if typeof(headroom) == TYPE_FLOAT and not is_nan(headroom) else -1.0
	return Vector2(h, status if typeof(status) == TYPE_INT else -1)
