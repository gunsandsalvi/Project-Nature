## Implements PRE-42 PRE-43 PRE-46 PLT-04 (T2.9a.2/4): exclusively owned CPU preparation.
## Called on one worker; no scene access, shared resources or GPU texture creation.
extends RefCounted

const CHANNELS := ["colour", "normal", "material"]
const MATERIAL_IDS := [0, 1, 4, 1, 7, 7, 3, 2, 2]


static func converted(image: Image, channel: String, basis: String, mapping: String) -> Image:
	var bytes := image.get_data()
	for offset in range(0, bytes.size(), 4):
		if bytes[offset + 3] == 0:
			continue
		if channel == "normal" and basis == "world-east-south-up":
			bytes[offset + 1] = 255 - bytes[offset + 1]
		elif channel == "material" and mapping == "fixture27-v1":
			var source := int(bytes[offset])
			if source <= 0 or source >= MATERIAL_IDS.size():
				return null
			bytes[offset] = MATERIAL_IDS[source]
			bytes[offset + 1] = 0
			bytes[offset + 2] = 0
	return Image.create_from_data(
		image.get_width(), image.get_height(), false, Image.FORMAT_RGBA8, bytes
	)


static func prepare(job: Dictionary) -> Dictionary:
	var started := Time.get_ticks_usec()
	var result := {
		"epoch": job.epoch,
		"generation": job.generation,
		"revision": job.revision,
		"key": job.key,
		"images": {},
		"cpu_bytes": 0,
		"problem": "",
		"normal_basis": "world-east-north-up"
	}
	var input: Dictionary = job.input
	var source: Dictionary = input.family
	if source.normal_basis not in ["world-east-south-up", "world-east-north-up"]:
		return _failed(result, "Unsupported normal basis.")
	if source.material_map not in ["fixture27-v1", "terrain-v1"]:
		return _failed(result, "Unsupported material mapping.")
	var decoder := KdLook.new()
	for channel: String in CHANNELS:
		var descriptor: Dictionary = input.files[channel]
		var bytes := _read(descriptor)
		if bytes.is_empty():
			return _failed(result, "Missing, oversized or changed sprite channel: " + channel)
		var decoded: Dictionary = decoder.call("texture_levels_bytes", bytes)
		bytes.clear()
		if not decoded.problem.is_empty():
			return _failed(result, decoded.problem)
		var levels: Array = decoded.levels
		var level: int = input.level
		if level < 0 or level >= levels.size():
			return _failed(result, "Sprite reduction is absent.")
		var image := converted(levels[level], channel, source.normal_basis, source.material_map)
		levels.clear()
		decoded.clear()
		if image == null:
			return _failed(result, "Unknown semantic material ID.")
		result.images[channel] = image
		result.cpu_bytes += image.get_width() * image.get_height() * 4
	result.preparation_us = Time.get_ticks_usec() - started
	return result


static func _read(descriptor: Dictionary) -> PackedByteArray:
	var path: String = descriptor.path
	if not path.begins_with("res://test/support/textures/") or path.contains(".."):
		return PackedByteArray()
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null or file.get_length() < 1 or file.get_length() > int(descriptor.max_bytes):
		return PackedByteArray()
	var bytes := file.get_buffer(file.get_length())
	var hash := HashingContext.new()
	hash.start(HashingContext.HASH_SHA256)
	hash.update(bytes)
	if hash.finish().hex_encode() != descriptor.sha256:
		return PackedByteArray()
	return bytes


static func _failed(result: Dictionary, message: String) -> Dictionary:
	result.images.clear()
	result.cpu_bytes = 0
	result.problem = message
	return result
