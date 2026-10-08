## The river's water for the pages that draw it (A4.5, PRE-26): the water's globals, which the
## ground's shader and the surface's read, from base/tuning/water.toml and the river's level, so no
## number of the water is in code; and the tick the marks and glints step on, ten times a second
## (A4.2). Implements PRE-26.
class_name Water
extends RefCounted

## The tick now, a whole number counted from the start, steps a second, and whether it is held.
var tick := 0
var rate := 10
var held := false

var _clock := 0.0
var _flow := Vector2.ZERO
var _level := 0.0


## Publishes the water's globals from the world's tuning, for a river at a level, in metres up from
## the meadow; "" or the problem.
func apply(world: KdWorld, level: float) -> String:
	var tuning := world.entry("tuning/water", "base:water")
	if tuning.is_empty():
		return "the catalogue has no tuning/water"
	_level = level
	rate = int(tuning["step_rate"])
	# the current runs east, and a tick is a step of it, in metres
	_flow = Vector2(float(tuning["flow"]) / 1000.0 / float(rate), 0.0)
	_publish_tick()
	var absorb := Vector4(
		1000.0 / float(tuning["fade_red"]),
		1000.0 / float(tuning["fade_green"]),
		1000.0 / float(tuning["fade_blue"]),
		1000.0 / float(tuning["murk"])
	)
	_global("kd_water_absorb", absorb)
	_global("kd_water_deep", _colour(tuning["deep_colour"], 0.0))
	_global(
		"kd_water_wet",
		Vector4(
			float(tuning["wet_margin"]) / 1000.0,
			_share(tuning["wet_darkening"]),
			_share(tuning["ragged"]),
			float(tuning["beach"]) / 1000.0
		)
	)
	_global(
		"kd_water_surface",
		Vector4(
			_share(tuning["sky_share"]),
			float(tuning["glint_life"]),
			_share(tuning["glint_share"]),
			float(tuning["shore_width"])
		)
	)
	_global(
		"kd_water_marks",
		Vector4(
			_share(tuning["calm_marks"]),
			float(tuning["calm_scale"]) / 1000.0,
			float(tuning["marks_depth"]) / 1000.0,
			_share(tuning["shore_line_share"])
		)
	)
	_global(
		"kd_water_edge",
		Vector4(
			float(tuning["shore_wander"]) / 1000.0,
			float(tuning["shore_scale"]) / 1000.0,
			_share(tuning["murk_curve"]),
			float(tuning["gravel_depth"]) / 1000.0
		)
	)
	_global("kd_water_glint", _colour(tuning["glint_colour"], 0.0))
	_global("kd_water_shore", _colour(tuning["shore_colour"], 0.0))
	return ""


## Moves the clock on and the tick with it, unless it is held.
func step(seconds: float) -> void:
	if held:
		return
	_clock += seconds
	var now := int(floor(_clock * float(rate)))
	if now != tick:
		tick = now
		_publish_tick()


## Holds the tick at a number, for the pictures that must be the same twice.
func hold_at(number: int) -> void:
	held = true
	tick = number
	_publish_tick()


## Lets the tick run again from where it is.
func release() -> void:
	held = false
	_clock = float(tick) / float(rate)


func _publish_tick() -> void:
	_global("kd_water", Vector4(_level, _flow.x, _flow.y, float(tick)))


static func _share(parts_per_million: Variant) -> float:
	return float(parts_per_million) / 1.0e6


static func _colour(hex: Variant, alpha: float) -> Vector4:
	var linear := Color(str(hex)).srgb_to_linear()
	return Vector4(linear.r, linear.g, linear.b, alpha)


static func _global(name: String, value: Variant) -> void:
	RenderingServer.global_shader_parameter_set(name, value)
