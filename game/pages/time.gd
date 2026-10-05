## The calendar (TIM-14, TIM-01, TIM-10): the world's time on its own thread, the screen following
## it at the speed you choose, and the speed it really runs at. Until the world exists, a stand-in
## does a fixed amount of work for each game hour, so top speed shows what the phone can do.
## Implements TIM-01, TIM-10 and TIM-14.
extends VBoxContainer

## TIM-01's speeds at the zoom stops, in game seconds a real second; top asks for far more than any
## phone can give, so it runs as fast as this one can.
const SPEEDS := [
	["Real", 1.0],
	["An hour a minute", 60.0],
	["A day in 3 minutes", 480.0],
	["A season a minute", 21600.0],
	["3 years a minute", 259200.0],
	["Top", 1.0e12],
]
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
	var grid := GridContainer.new()
	grid.columns = 2
	grid.add_theme_constant_override("h_separation", 10)
	grid.add_theme_constant_override("v_separation", 10)
	add_child(grid)
	for i in SPEEDS.size():
		var button := Button.new()
		button.text = SPEEDS[i][0]
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
	world = KdWorld.new()
	world.start_clockwork(WORK_PER_HOUR)
	choose_speed(0)
	_show()


func _process(_delta: float) -> void:
	world.frame()
	_show()


## Asks for one of the speeds, by its place in SPEEDS, and plays if paused.
func choose_speed(index: int) -> void:
	world.set_speed(SPEEDS[index][1])
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
