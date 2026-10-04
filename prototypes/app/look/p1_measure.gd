## P1's Measure (PLT-04): each outline method without and with the mirrored water at 60 frames a
## second, then none, C, and C with the mirror at 120, where a frame has only 8.3 ms, so the chip
## can't hide its cost by slowing its clock; each for RUN_TIME seconds from the painter's view: a
## second to settle, then two each of panning, turning and zooming by script. Each run keeps the
## graphics time of every pass and the share of frames on time at its frame rate; at the end, one
## line for the chat (runs.gd). Pre-production code (research 00).
extends RefCounted

const Runs := preload("res://look/runs.gd")
## Each run: [outline method, mirror, frame rate].
const RUNS := [
	[0, false, 60],
	[1, false, 60],
	[2, false, 60],
	[3, false, 60],
	[4, false, 60],
	[0, true, 60],
	[1, true, 60],
	[2, true, 60],
	[3, true, 60],
	[4, true, 60],
	[0, false, 120],
	[3, false, 120],
	[3, true, 120]
]
const RUN_TIME := 7.0

## The run under way, or -1.
var index := -1
var _screen: Control
var _clock := 0.0
var _samples := PackedFloat32Array()
var _late := 0
var _results := PackedStringArray()
var _before := []


func _init(screen: Control) -> void:
	_screen = screen


func start() -> void:
	var s := _screen
	_before = [s.target, s.yaw, s.mpp, s.outline, s.reflect, s.crawl]
	s.crawl = 0
	_results = PackedStringArray()
	_begin(0)


func _begin(i: int) -> void:
	index = i
	var run: Array = RUNS[i]
	_screen._set_outline(run[0])
	_screen._set_reflect(run[1])
	Engine.max_fps = run[2]
	_clock = 0.0
	_samples = PackedFloat32Array()
	_late = 0
	_screen._readout.text = (
		"Measuring %d of %d: outline %s, mirror %s, %d frames a second"
		% [i + 1, RUNS.size(), _screen.OUTLINES[run[0]], "on" if run[1] else "off", run[2]]
	)


func step(delta: float) -> void:
	var s := _screen
	_clock += delta
	var t := _clock
	var c: Dictionary = s.painter.camera
	var home := Vector3(c.target[0], c.target[1], c.target[2])
	var right := Vector3(cos(deg_to_rad(c.yaw)), 0.0, -sin(deg_to_rad(c.yaw)))
	s.target = home
	s.yaw = c.yaw
	s.mpp = c.mpp
	if t >= 1.0 and t < 3.0:
		s.target = home + right * 4.0 * sin((t - 1.0) * PI)
	elif t >= 3.0 and t < 5.0:
		s.yaw = c.yaw + 25.0 * sin((t - 3.0) * PI)
	elif t >= 5.0:
		s.mpp = c.mpp * (1.0 + 0.4 * sin((t - 5.0) * PI * 0.5))
	s._apply_camera()
	var run: Array = RUNS[index]
	if t >= 1.0:
		_samples.append(s.graphics_time())
		if delta > 1.15 / float(run[2]):
			_late += 1
	if t < RUN_TIME:
		return
	var label := (
		"%s%s%s"
		% [s.OUTLINES[run[0]], "+m" if run[1] else "", "@%d" % run[2] if run[2] != 60 else ""]
	)
	_results.append(Runs.summary(label, _samples, _late))
	if index + 1 < RUNS.size():
		_begin(index + 1)
		return
	index = -1
	Engine.max_fps = 0
	s.target = _before[0]
	s.yaw = _before[1]
	s.mpp = _before[2]
	s.crawl = _before[5]
	s._set_outline(_before[3])
	s._set_reflect(_before[4])
	s._apply_camera()
	Runs.finish(s, "P1", _results, [])
