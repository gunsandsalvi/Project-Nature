## Implements PRE-27, PRE-42, PRE-43, PRE-44, PRE-46 (T2.7a.2/4).
## Validates fixture atlases before drawing. Development metadata is not a physical world catalogue.
extends RefCounted

const DENSITIES := [64, 32, 16, 8, 4, 2]
const NAMES := ["tree", "shelter", "boulder", "ground", "person", "animal"]
var entries: Array = []
var textures: Dictionary = {}
var images: Dictionary = {}
var problem := ""


func read(path := "res://test/support/fixtures/manifest.json") -> bool:
	entries = []
	textures.clear()
	images.clear()
	var parsed: Variant = JSON.parse_string(FileAccess.get_file_as_string(path))
	if not parsed is Array or parsed.size() != NAMES.size():
		return _fail("The fixture manifest needs six named entries.")
	for index in parsed.size():
		if not parsed[index] is Dictionary:
			return _fail("A fixture entry must be a record.")
		var entry: Dictionary = parsed[index]
		if not _metadata(entry, index):
			return false
		if not _load_entry(entry):
			return false
	entries = parsed
	problem = ""
	return true


func _load_entry(entry: Dictionary) -> bool:
	for action: String in entry.actions:
		for density: int in DENSITIES:
			var resource: String = entry.actions[action][str(density)]
			if not _load_level(entry, density, resource):
				return false
	for channel: String in ["normal_levels", "material_levels"]:
		if entry.has(channel) and not entry[channel].is_empty():
			for density: int in DENSITIES:
				if not entry[channel].has(str(density)):
					return _fail("%s is missing %s at %d px/m." % [entry.name, channel, density])
				if not _load_level(entry, density, entry[channel][str(density)]):
					return false
	return true


func _metadata(entry: Dictionary, index: int) -> bool:
	for key: String in [
		"id",
		"name",
		"status",
		"frame_size",
		"pivot",
		"frames",
		"facings",
		"height_unit",
		"height_mm",
		"actions"
	]:
		if not entry.has(key):
			return _fail("Fixture entry %d is missing %s." % [index, key])
	if not _types(entry):
		return false
	if entry.name != NAMES[index] or int(entry.id) != index + 1 or entry.height_unit != "mm":
		return _fail("Fixture names, IDs or height units do not match the local fixture contract.")
	if not _dimensions(entry):
		return false
	return _actions(entry) and _pieces(entry)


func _types(entry: Dictionary) -> bool:
	for key: String in ["id", "frames", "facings", "height_mm"]:
		if not _number(entry[key]) or float(entry[key]) != floor(float(entry[key])):
			return _fail("%s must be a whole number." % key)
	for key: String in ["name", "status", "height_unit"]:
		if not entry[key] is String:
			return _fail("%s must be text." % key)
	for key: String in ["normal_levels", "material_levels"]:
		if entry.has(key) and not entry[key] is Dictionary:
			return _fail("%s must be a record of atlas paths." % key)
		if entry.has(key) and not _paths(entry[key]):
			return false
	return _channels(entry)


func _channels(entry: Dictionary) -> bool:
	if (
		not entry.get("normal_levels", {}).is_empty()
		and entry.get("normal_basis", "") != "world-east-north-up"
	):
		return _fail("Normal atlases must declare the world-east-north-up basis.")
	if (
		not entry.get("material_levels", {}).is_empty()
		and entry.get("material_format", "") != "index-r"
	):
		return _fail("Material atlases must declare index-r: red holds the material's byte ID.")
	return true


func _number(value: Variant) -> bool:
	return value is int or value is float


func _actions(entry: Dictionary) -> bool:
	if not entry.actions is Dictionary or not entry.actions.has("walk"):
		return _fail("%s needs a walk or static atlas." % entry.name)
	for action: String in entry.actions:
		if not entry.actions[action] is Dictionary:
			return _fail("%s has invalid action levels." % entry.name)
		if not _paths(entry.actions[action]):
			return false
		for density: int in DENSITIES:
			if not entry.actions[action].has(str(density)):
				return _fail("%s is missing an action level at %d px/m." % [entry.name, density])
	return true


func _paths(levels: Dictionary) -> bool:
	for key: Variant in levels:
		if not key is String or not levels[key] is String:
			return _fail("Atlas levels need text density keys and resource paths.")
	return true


func _pieces(entry: Dictionary) -> bool:
	if not entry.has("pieces"):
		return true
	if not entry.pieces is Dictionary:
		return _fail("Sprite pieces must be named frame rectangles.")
	for rect: Variant in entry.pieces.values():
		if not rect is Array or rect.size() != 4:
			return _fail("A sprite piece needs x, y, width and height.")
		for value: Variant in rect:
			if not _number(value) or value < 0:
				return _fail("Piece bounds must be positive pixel lengths.")
		if (
			rect[2] <= 0
			or rect[3] <= 0
			or rect[0] + rect[2] > entry.frame_size[0]
			or rect[1] + rect[3] > entry.frame_size[1]
		):
			return _fail("A sprite piece lies outside its frame.")
	return true


func _dimensions(entry: Dictionary) -> bool:
	if not entry.frame_size is Array or entry.frame_size.size() != 2:
		return _fail("%s needs a frame size." % entry.name)
	if not entry.pivot is Array or entry.pivot.size() != 2:
		return _fail("%s needs a foot pivot." % entry.name)
	for axis in 2:
		if (
			not _number(entry.frame_size[axis])
			or not _number(entry.pivot[axis])
			or entry.frame_size[axis] != floor(float(entry.frame_size[axis]))
			or entry.frame_size[axis] < 1
			or entry.pivot[axis] < 0
			or entry.pivot[axis] >= entry.frame_size[axis]
		):
			return _fail("%s has a pivot outside its frame." % entry.name)
	if int(entry.frames) < 1 or int(entry.facings) not in [1, 4, 8]:
		return _fail("%s needs positive frame counts and 1, 4 or 8 facings." % entry.name)
	return true


func _load_level(entry: Dictionary, density: int, resource: String) -> bool:
	if resource.contains("..") or resource.contains(":") or resource.begins_with("/"):
		return _fail("Fixture resource paths must stay inside res://test/support/fixtures.")
	var path := "res://test/support/fixtures/" + resource
	if not ResourceLoader.exists(path):
		return _fail("Missing fixture resource: %s" % path)
	var texture := load(path) as Texture2D
	if texture == null:
		return _fail("Fixture resource is not a texture: %s" % path)
	var width := maxi(1, roundi(float(entry.frame_size[0]) * density / 64.0))
	var height := maxi(1, roundi(float(entry.frame_size[1]) * density / 64.0))
	if (
		texture.get_width() != width * int(entry.frames)
		or texture.get_height() != height * int(entry.facings)
	):
		return _fail("%s does not have the declared frame grid at %d px/m." % [resource, density])
	textures[resource] = texture
	images[resource] = texture.get_image()
	return true


func texture(entry: Dictionary, density: float, action: String, channel := "colour") -> Texture2D:
	var levels: Dictionary = entry.actions.get(action, entry.actions.walk)
	if channel != "colour":
		levels = entry.get(channel + "_levels", {})
	return textures[levels[str(int(density))]] if levels.has(str(int(density))) else null


func _fail(message: String) -> bool:
	problem = message
	return false
