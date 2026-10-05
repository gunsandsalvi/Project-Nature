## The gesture reader (IMPLEMENTATION α0.7a, PRE-33, A15): every touch read by one rule set, on
## raw touches, so a scripted test can tell each gesture from every other. One finger: a still
## press let go before it is long is a tap, which selects; a long press opens your powers, and a
## move after it draws; a drag moves; and a second touch soon after a tap, dragged, zooms. On the
## views' handle, a tap or a swipe up opens the views. Two fingers: a pinch zooms and a twist
## turns, whichever shows first. Distances are in dp and times in milliseconds; positions are in
## whatever pixels the touches come in, `dp` of them a dp. Pre-production code (research 00).
extends RefCounted

const DOUBLE_MS := 300  # a touch this soon after a tap may begin a double tap
const LONG_MS := 500  # held still this long, a press opens your powers
const SLOP := 10.0  # dp a finger may wander and still be still
const NEAR := 40.0  # dp from a tap a double tap's second touch may land
const PINCH := 0.12  # the share the fingers' distance must change to read a pinch
const TWIST := 15.0  # degrees the fingers must turn to read a twist
const ZOOM_DP := 160.0  # dp a double tap's drag goes for each doubling of the zoom, down to zoom in

## Pixels a dp, and the views' handle, in the touches' pixels.
var dp := 3.0
var handle := Rect2()

var _touches := {}  # index: {"at": Vector2, "from": Vector2, "ms": int}
## "" none; "press", "handle", "double": a still finger; "drag", "long", "draw", "zoomdrag": one
## moving; "two": two not yet read; "pinch", "twist"; "done": until every finger is up
var _mode := ""
var _tap := {}  # a tap held back while a second touch could make it a double: {"at", "ms"}
var _last := Vector2()
var _two := {}  # the two fingers when the gesture was read: distance, angle


## A finger down or up at a position, at a time: the gestures it ends or begins.
func touch(index: int, at: Vector2, pressed: bool, ms: int) -> Array:
	var out := tick(ms)
	if pressed:
		_touches[index] = {"at": at, "from": at, "ms": ms}
		if _touches.size() == 1:
			out += _first_down(at, ms)
		elif _touches.size() == 2:
			out += _second_down()
		return out
	if not _touches.has(index):
		return out
	var t: Dictionary = _touches[index]
	_touches.erase(index)
	match _mode:
		"press":
			_tap = {"at": t.from, "ms": ms}
		"handle":
			out.append({"kind": "views"})
		"double":
			# a second quick tap with no drag: both taps, as two
			out.append({"kind": "tap", "at": _tap.at})
			out.append({"kind": "tap", "at": t.from})
			_tap = {}
		"drag":
			out.append({"kind": "drag_end"})
		"zoomdrag":
			out.append({"kind": "zoom_end"})
		"pinch":
			out.append({"kind": "zoom_end"})
		"twist":
			out.append({"kind": "turn_end"})
	_mode = "" if _touches.is_empty() else "done"
	return out


## A finger moved, at a time: the gestures it reads or moves on.
func move(index: int, at: Vector2, ms: int) -> Array:
	var out := tick(ms)
	if not _touches.has(index):
		return out
	_touches[index].at = at
	if _touches.size() >= 2:
		return out + _two_moved()
	var t: Dictionary = _touches[index]
	var moved: Vector2 = at - t.from
	var far := moved.length() > SLOP * dp
	match _mode:
		"press":
			if far:
				_mode = "drag"
				_last = t.from
				out += _dragged(at)
		"handle":
			if far:
				if -moved.y > absf(moved.x):
					out.append({"kind": "views"})
					_mode = "done"
				else:
					_mode = "drag"
					_last = t.from
					out += _dragged(at)
		"double":
			if far:
				_tap = {}
				_mode = "zoomdrag"
				_last = t.from
				out += _zoom_dragged(at)
		"drag":
			out += _dragged(at)
		"zoomdrag":
			out += _zoom_dragged(at)
		"long":
			if far:
				_mode = "draw"
				out.append({"kind": "draw", "at": at})
		"draw":
			out.append({"kind": "draw", "at": at})
	return out


## Time passing with no touch: a tap no second touch followed, or a finger held still long enough.
func tick(ms: int) -> Array:
	var out := []
	if not _tap.is_empty() and _mode != "double" and ms - int(_tap.ms) > DOUBLE_MS:
		out.append({"kind": "tap", "at": _tap.at})
		_tap = {}
	if _mode in ["press", "double"] and _touches.size() == 1:
		var t: Dictionary = _touches.values()[0]
		if ms - int(t.ms) >= LONG_MS:
			if _mode == "double":
				out.append({"kind": "tap", "at": _tap.at})
				_tap = {}
			_mode = "long"
			out.append({"kind": "long", "at": t.from})
	return out


func _first_down(at: Vector2, ms: int) -> Array:
	var out := []
	if handle.has_point(at):
		_mode = "handle"
		return out
	if not _tap.is_empty():
		if ms - int(_tap.ms) <= DOUBLE_MS and at.distance_to(_tap.at) <= NEAR * dp:
			_mode = "double"
			return out
		out.append({"kind": "tap", "at": _tap.at})
		_tap = {}
	_mode = "press"
	return out


## A second finger down: whatever one finger was doing ends, and the two are read from here.
func _second_down() -> Array:
	var out := []
	match _mode:
		"drag":
			out.append({"kind": "drag_end"})
		"zoomdrag":
			out.append({"kind": "zoom_end"})
	if _mode == "double":
		out.append({"kind": "tap", "at": _tap.at})
	_tap = {}
	var pair := _pair()
	_two = {"distance": pair[0], "angle": pair[1]}
	_mode = "two"
	return out


func _two_moved() -> Array:
	var out := []
	var pair := _pair()
	var distance: float = pair[0]
	var angle: float = pair[1]
	var middle: Vector2 = pair[2]
	match _mode:
		"two":
			var scale := distance / maxf(float(_two.distance), 1.0)
			var turned := wrapf(angle - float(_two.angle), -180.0, 180.0)
			if absf(scale - 1.0) > PINCH:
				_mode = "pinch"
				out.append({"kind": "zoom", "by": scale, "at": middle})
			elif absf(turned) > TWIST:
				_mode = "twist"
				out.append({"kind": "turn", "by": turned, "at": middle})
			else:
				return out
		"pinch":
			out.append(
				{"kind": "zoom", "by": distance / maxf(float(_two.distance), 1.0), "at": middle}
			)
		"twist":
			out.append(
				{
					"kind": "turn",
					"by": wrapf(angle - float(_two.angle), -180.0, 180.0),
					"at": middle
				}
			)
		_:
			return out
	_two = {"distance": distance, "angle": angle}
	return out


## The first two fingers' distance, angle in degrees and middle.
func _pair() -> Array:
	var ats := []
	for t: Dictionary in _touches.values():
		ats.append(t.at)
		if ats.size() == 2:
			break
	var a: Vector2 = ats[0]
	var b: Vector2 = ats[1]
	return [a.distance_to(b), rad_to_deg((b - a).angle()), (a + b) / 2.0]


func _dragged(at: Vector2) -> Array:
	var by := at - _last
	_last = at
	return [{"kind": "drag", "at": at, "by": by}]


func _zoom_dragged(at: Vector2) -> Array:
	var by := pow(2.0, (at.y - _last.y) / (ZOOM_DP * dp))
	_last = at
	return [{"kind": "zoom", "by": by, "at": at}]
