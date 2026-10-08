## Implements PRE-03 PRE-22 PRE-42 PRE-43 PRE-46 PLT-04 (T2.9a.1/2).
## Validated authored families; this page presents four candidates, without unapproved actor art.
extends "res://fixtures/atlas.gd"

const Stream := preload("res://fixtures/sprite_stream.gd")
const ASSETS := {
	"tree": "birch_summer", "shelter": "tent", "boulder": "boulder", "ground": "meadow"
}
var _families: Dictionary = {}
var _descriptors: Dictionary = {}
var _requests: Dictionary = {}
var _identity: Dictionary = {}
var _limits: Dictionary = {}
var _service: Node
var _epoch := -1
var _revision := 0
var _generation := 1
var _build: ConfigFile


func configure(world: KdWorld, tree: SceneTree, world_id: String) -> void:
	_service = Stream.service(tree)
	_build = GameData.build()
	_descriptors = GameData.texture_descriptors(_build)
	_limits = world.stream_limits()
	for family: Dictionary in world.sprite_families():
		if family.part == "whole":
			_families[family.asset + "/" + family.family] = family
	_identity = {
		"world_id": world_id,
		"data_hash": _fingerprint(2),
		"look_hash": _fingerprint(4),
		"renderer": RenderingServer.get_current_rendering_method(),
		"format_version": 1
	}


func _fingerprint(column: int) -> String:
	var parts := PackedStringArray()
	for line: String in _build.get_value("catalogue", "sources", []):
		var fields := line.split(" ")
		if fields.size() == 5:
			parts.append(fields[0] + ":" + fields[1] + ":" + fields[column])
	return "\n".join(parts).sha256_text()


func read(path := "res://fixtures/manifest.json") -> bool:
	var parsed: Variant = JSON.parse_string(FileAccess.get_file_as_string(path))
	if not parsed is Array or parsed.size() != NAMES.size():
		return _fail("Fixture metadata is missing.")
	entries = parsed
	for entry: Dictionary in entries:
		if ASSETS.has(entry.name):
			entry.status = "Candidate art"
			entry.normal_levels = {"supplied": "streamed"}
			entry.material_levels = {"supplied": "streamed"}
	return _service != null


func update(frame: Dictionary) -> void:
	if _service == null or not frame.has("epoch"):
		return
	if _epoch != int(frame.epoch):
		_epoch = int(frame.epoch)
		_identity.epoch = _epoch
		_requests.clear()
		var begun: Dictionary = _service.begin(_identity, _limits)
		if not begun.ok:
			problem = begun.problem
			return
	_revision = int(frame.revision)
	var manifest: Dictionary = _service.ledger.manifest(frame.manifest)
	if not manifest.ok:
		problem = manifest.problem
		return
	var density := float(frame.density)
	if density < 2.0:
		return
	for asset: String in ASSETS.values():
		_request(asset, 4, {})
		var parent := _available(asset, 4)
		if not parent.is_empty() and density != 4:
			_request(asset, density, parent.key)
		if float(frame.target_density) >= 2.0 and not parent.is_empty():
			_request(asset, float(frame.target_density), parent.key)
	problem = _service.problem


func _slot(asset: String, density: float) -> String:
	var source := 64 if density >= 32 else 16 if density >= 8 else 4
	var level := 0 if density >= source else 1
	return "%s/%d/%d" % [asset, source, level]


func _request(asset: String, density: float, parent: Dictionary) -> void:
	var slot := _slot(asset, density)
	if _requests.has(slot):
		return
	var words := slot.split("/")
	var source := int(words[1])
	var level := int(words[2])
	var family_name := "near" if source == 64 else "middle" if source == 16 else "far"
	var family: Dictionary = _families.get(asset + "/" + family_name, {})
	if family.is_empty():
		problem = "Missing candidate family: " + asset + "/" + family_name
		return
	var files := {}
	var encoded_bytes := 0
	for channel: String in ["colour", "normal", "material"]:
		var path := GameData.texture_path(family[channel])
		if not _descriptors.has(path):
			problem = "Missing texture descriptor: " + path
			return
		files[channel] = _descriptors[path]
		encoded_bytes += int(files[channel].max_bytes)
	var input := {"family": family, "files": files, "level": level}
	var key := _identity.duplicate()
	key.erase("epoch")
	key.merge(
		{
			"asset": asset,
			"family": family_name,
			"form": "individual",
			"level": level,
			"tile_east": 0,
			"tile_north": 0,
			"surface_revision": 0,
			"caster_revision": 0,
			"appearance_revision": 0
		}
	)
	var pixels := int(family.page_width) * int(family.page_height) / (1 << (2 * level))
	var chain_bytes := ceili(float(family.page_width * family.page_height) * 16.0 / 3.0)
	var spec := {
		"key": key,
		"parent_key": parent,
		"generation": _generation,
		"revision": _revision,
		"dependencies": [],
		"input": input,
		"reserve":
		{
			"input": encoded_bytes + JSON.stringify(input).length() * 8 + 4096,
			"prepared": pixels * 4 * 6 + chain_bytes,
			"resident": pixels * 4 * 3,
			"category": "ground" if asset == "meadow" else "sprites"
		}
	}
	spec.reserve.input = maxi(spec.reserve.input, _service.ledger.input_bytes_required(spec))
	var answer: Dictionary = _service.request(spec)
	if answer.ok:
		_requests[slot] = {
			"token": answer.token,
			"key": key,
			"family": family,
			"level": level,
			"density": source >> level
		}
	else:
		problem = answer.problem


func _available(asset: String, density: float) -> Dictionary:
	var preferred: Dictionary = _requests.get(_slot(asset, density), {})
	if not preferred.is_empty() and not _service.bundle(preferred.token).is_empty():
		return preferred
	var parent: Dictionary = _requests.get(_slot(asset, 4), {})
	if not parent.is_empty() and not _service.bundle(parent.token).is_empty():
		return parent
	return {}


func sample(entry: Dictionary, density: float) -> Dictionary:
	if not ASSETS.has(entry.name) or _service == null or density < 2.0:
		return {}
	var record := _available(ASSETS[entry.name], density)
	if record.is_empty():
		return {}
	var bundle: Dictionary = _service.bundle(record.token)
	var cell: Dictionary = record.family.cells[0]
	var reduction: float = float(1 << int(record.level))
	return {
		"textures": bundle.textures,
		"image": bundle.images.colour,
		"pivot": Vector2(cell.pivot_x_256, cell.pivot_y_256) / (256.0 * reduction),
		"density": float(record.density),
		"tile_metres": float(record.family.page_width) / float(record.family.density)
	}


func texture(entry: Dictionary, density: float, _action: String, channel := "colour") -> Texture2D:
	var chosen := sample(entry, density)
	return chosen.textures.get(channel) if not chosen.is_empty() else null


func release() -> void:
	if _service == null:
		return
	for request_record: Dictionary in _requests.values():
		_service.release(request_record.token)
	_requests.clear()


func metrics() -> Dictionary:
	return _service.ledger.status() if _service != null else {}
