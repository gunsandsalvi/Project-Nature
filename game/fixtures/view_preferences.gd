## Implements PRE-03 WLD-13 PLT-07 (T2.9a.3): disposable view state outside world archives.
extends RefCounted


static func read(path: String) -> Dictionary:
	var file := ConfigFile.new()
	if file.load(path) != OK or file.get_value("view", "version", 0) != 2:
		return {}
	var state := {}
	for key: String in ["east", "north", "density", "manual_rate"]:
		var value: Variant = file.get_value("view", key, 0.0)
		if value is String:
			if not value.begins_with("f64:") or value.length() != 20:
				return {}
			var encoded: String = value.substr(4)
			if not encoded.is_valid_hex_number():
				return {}
			value = encoded.hex_decode().decode_double(0)
		if not (value is float or value is int) or not is_finite(float(value)):
			return {}
		state[key] = float(value)
	if absf(state.east) > 100000000 or absf(state.north) > 100000000:
		return {}
	if state.density < pow(2.0, -12) or state.density > 64 or state.manual_rate < 0:
		return {}
	state["scene"] = file.get_value("view", "scene", "Camp")
	state["light"] = file.get_value("view", "light", "Noon")
	state["selected"] = file.get_value("view", "selected", 0)
	state["locked"] = file.get_value("view", "locked", false)
	if not state.scene in ["Camp", "River", "Shelter"] or not state.light in ["Noon", "Dusk"]:
		return {}
	if not state.selected is int or not state.locked is bool:
		return {}
	for key: String in [
		"origin_east_cm", "origin_north_cm", "raster_origin_east_cm", "raster_origin_north_cm"
	]:
		var value: Variant = file.get_value("view", key, 0)
		if not value is int:
			return {}
		state[key] = value
	return state


static func write(path: String, state: Dictionary) -> Error:
	var file := ConfigFile.new()
	file.set_value("view", "version", 2)
	for key: String in state:
		var value: Variant = state[key]
		if value is float:
			# ConfigFile's decimal formatting loses centimetres at wide positions and rounds zoom stops.
			var encoded := PackedByteArray()
			encoded.resize(8)
			encoded.encode_double(0, value)
			value = "f64:" + encoded.hex_encode()
		file.set_value("view", key, value)
	var made := DirAccess.make_dir_recursive_absolute(path.get_base_dir())
	if made != OK:
		return made
	var written := file.save(path + ".tmp")
	if written != OK:
		return written
	return DirAccess.rename_absolute(path + ".tmp", path)
