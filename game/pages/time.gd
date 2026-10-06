## The calendar (TIM-14, TIM-01, TIM-10): the world's time on its own thread, the screen following
## it at the speed you choose, and the speed it really runs at. Until the world exists, a stand-in
## does a fixed amount of work for each game hour, so top speed shows what the phone can do.
## Implements TIM-01, TIM-10 and TIM-14.
extends VBoxContainer

const TEXT := Palette.TEXT

var world: KdWorld
var bar: SpeedBar

var _date: Label
var _clock: Label


func _ready() -> void:
	size_flags_vertical = Control.SIZE_EXPAND_FILL
	add_theme_constant_override("separation", 14)
	_date = _label(28)
	_clock = _label(56)
	world = KdWorld.new()
	var loaded := GameData.load_into(world)
	bar = SpeedBar.new()
	add_child(bar)
	bar.setup(world, loaded.get("problems", PackedStringArray()))
	world.start_clockwork()
	bar.choose_speed(0)
	_show()


func _process(_delta: float) -> void:
	world.frame()
	_show()


## The date the page shows, "Year 1, spring, day 1".
func date_text() -> String:
	return _date.text


func _label(font_size: int) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	label.add_theme_color_override("font_color", TEXT)
	label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	add_child(label)
	return label


func _show() -> void:
	_date.text = world.date_text()
	_clock.text = world.time_text()
	bar.show_speed()
