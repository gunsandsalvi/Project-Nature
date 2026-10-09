## Retained renderer allocation regression (PLT-04 PRE-22): replacement reserves before retiring.
extends RefCounted

var _target_ticket := 0
var _target_size := Vector2i.ZERO
var _stream: Node


func _reserve_targets(target_size: Vector2i) -> bool:
	if target_size == _target_size:
		return true
	var allocation: Dictionary = _stream.reserve_allocation(
		{"category": "targets", "target": target_size.x * target_size.y * 4 * 3}
	)
	if not allocation.ok:
		return false
	_stream.retire_allocation(_target_ticket)
	_target_ticket = allocation.token
	_target_size = target_size
	return true
