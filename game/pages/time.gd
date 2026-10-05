## The calendar (TIM-14, TIM-01, TIM-10): the world's time on its own thread, the screen following
## it at the speed you choose, and the speed it really runs at. Until the world exists, a stand-in
## does a fixed amount of work for each game hour, so top speed shows what the phone can do.
## Implements TIM-01, TIM-10 and TIM-14.
extends VBoxContainer

## TIM-01's zoom stops, each a field of the tuning file base/tuning/time.toml, which gives the game
## time a real minute at that stop; top is no stop's but the phone's own.
const STOPS := ["person", "close_camp", "camp", "valley", "region"]
## Top asks for far more than any phone can give, so it runs as fast as this one can.
const TOP := 1.0e12
## The stand-in world's work for each game hour (MAT-16).
const WORK_PER_HOUR := 20000
## Units for the speed shown, largest first, in game seconds.
const UNITS := [
	["year", 5184000.0],
	["season", 1296000.0],
	["day", 86400.0],
	["hour", 3600.0],
	["minute", 60.0],
	["second", 1.0],
]
const TEXT := Color("#efe6d8")
const QUIET := Color("#a89f95")

var world: KdWorld
## The buttons' speeds, in game seconds a real second: the stops' from the tuning file, then top.
var speeds: Array[float] = []

var _date: Label
var _clock: Label
var _shown: Label
var _pause: Button
var _speeds: Array[Button] = []


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 14)
	_date = _label(28, TEXT)
	_clock = _label(56, TEXT)
	_shown = _label(18, QUIET)
	world = KdWorld.new()
	var loaded := GameData.load_into(world)
	var tuning := world.entry("tuning/time", "base:time")
	for stop: String in STOPS:
		if tuning.has(stop):
			speeds.append(int(tuning[stop]) / 60.0)
	if speeds.size() != STOPS.size():
		var problems: PackedStringArray = loaded.get("problems", PackedStringArray())
		_shown.text = "The speeds did not load: %s" % "; ".join(problems)
		speeds.clear()
		speeds.assign(STOPS.map(func(_stop: String) -> float: return 1.0))
	speeds.append(TOP)
	var grid := GridContainer.new()
	grid.columns = 2
	grid.add_theme_constant_override("h_separation", 10)
	grid.add_theme_constant_override("v_separation", 10)
	add_child(grid)
	for i in speeds.size():
		var button := Button.new()
		button.text = button_words(speeds[i])
		button.custom_minimum_size = Vector2(0, 56)
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		button.toggle_mode = true
		button.pressed.connect(choose_speed.bind(i))
		grid.add_child(button)
		_speeds.append(button)
	_pause = Button.new()
	_pause.custom_minimum_size = Vector2(0, 64)
	_pause.pressed.connect(toggle_pause)
	add_child(_pause)
	world.start_clockwork(WORK_PER_HOUR)
	choose_speed(0)
	_show()


func _process(_delta: float) -> void:
	world.frame()
	_show()


## Asks for one of the speeds, by its place in speeds, and plays if paused.
func choose_speed(index: int) -> void:
	world.set_speed(speeds[index])
	world.play()
	for i in _speeds.size():
		_speeds[i].button_pressed = i == index


func toggle_pause() -> void:
	if world.is_paused():
		world.play()
	else:
		world.pause()


## The date the page shows, "Year 1, spring, day 1".
func date_text() -> String:
	return _date.text


## A speed button's words: "Real", "1 hour a minute", "Top".
static func button_words(rate: float) -> String:
	if rate >= TOP:
		return "Top"
	if absf(rate - 1.0) < 0.02:
		return "Real"
	return speed_words(rate, false).replace("game ", "").replace("real ", "")


## The speed as you would say it: "1 game hour a real minute", "real speed", "paused".
static func speed_words(rate: float, paused: bool) -> String:
	if rate < 0.001:
		return "paused" if paused else "waiting for the world"
	if absf(rate - 1.0) < 0.02:
		return "real speed, a game second a second"
	var per_minute := rate * 60.0
	for unit: Array in UNITS:
		if per_minute >= unit[1] * 0.95:
			var n: float = per_minute / unit[1]
			var amount := "%.1f" % n if n < 9.95 else "%d" % roundi(n)
			amount = amount.trim_suffix(".0")
			var plural := "" if amount == "1" else "s"
			return "%s game %s%s a real minute" % [amount, unit[0], plural]
	return "%.2f game seconds a real minute" % per_minute


func _label(font_size: int, colour: Color) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", colour)
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label


func _show() -> void:
	_date.text = world.date_text()
	_clock.text = world.time_text()
	var paused := world.is_paused()
	_shown.text = "Running at: %s" % speed_words(world.speed_shown(), paused)
	_pause.text = "Play" if paused else "Pause"
