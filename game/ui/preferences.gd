## Presentation preferences live outside camp saves and never enter a simulation command.
extends RefCounted

const PATH := "user://presentation.cfg"
static var _loaded := false
static var _values := {"mute": false, "large_text": false}


static func value(key: String) -> bool:
	if not _loaded:
		var file := ConfigFile.new()
		if file.load(PATH) == OK:
			for name: String in _values:
				_values[name] = bool(file.get_value("presentation", name, false))
		_loaded = true
	return bool(_values.get(key, false))


static func set_value(key: String, enabled: bool) -> void:
	value(key)
	_values[key] = enabled
	var file := ConfigFile.new()
	for name: String in _values:
		file.set_value("presentation", name, _values[name])
	file.save(PATH)
