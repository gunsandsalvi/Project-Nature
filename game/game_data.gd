## The build's copy of the catalogue (A2.3, A3.6): the files tools/gamedata.py copied into
## res://data/ and listed in build.toml, with what the cloud computed from them, for the self-check
## to compare and the pages to load. Implements MAT-13.
class_name GameData
extends RefCounted

const BUILD := "res://data/build.toml"


## build.toml, or an empty file when the build wrote none.
static func build() -> ConfigFile:
	var file := ConfigFile.new()
	file.load(BUILD)
	return file


## Each file the build lists, {"path", "sha256"}, in order.
static func files(build_file: ConfigFile) -> Array[Dictionary]:
	var out: Array[Dictionary] = []
	for item: String in build_file.get_value("catalogue", "files", []):
		var words := item.split(" ")
		if words.size() == 2:
			out.append({"path": words[0], "sha256": words[1]})
	return out


## The paths of the files the build lists.
static func paths(build_file: ConfigFile) -> PackedStringArray:
	var out := PackedStringArray()
	for file: Dictionary in files(build_file):
		out.append(file["path"])
	return out


## The reference sheets the build ships for the pilot's pieces (T2.3a.5), each by its piece's id
## with its file: {"club": "res://data/sheets/club.kdsheet"}.
static func sheets(build_file: ConfigFile) -> Dictionary:
	var out := {}
	for item: String in build_file.get_value("sheets", "files", []):
		var file := item.get_slice(" ", 0)
		out[file.get_basename()] = "res://data/sheets/" + file
	return out


## A sheet's picture as a texture: the file is a WebP kept under a name Godot's import leaves alone,
## so it ships as it is; null if it cannot be read.
static func sheet_texture(path: String) -> ImageTexture:
	var bytes := FileAccess.get_file_as_bytes(path)
	var image := Image.new()
	if bytes.is_empty() or image.load_webp_from_buffer(bytes) != OK:
		return null
	return ImageTexture.create_from_image(image)


## The .kdtex file of a texture's catalogue entry (A5.4): "art:meadow/middle" is
## textures/art/meadow/middle.kdtex.
static func texture_path(name: String) -> String:
	return "res://data/textures/art/%s.kdtex" % name.trim_prefix("art:")


## A texture tile's width in metres, from its record: its texture pixels over how many make a metre.
static func tile_metres(record: Dictionary) -> float:
	return float(record["tile_texels"]) / float(record["texels_a_metre"])


## Loads the build's catalogue into a world: what loading found, as KdWorld.load_catalogue says.
static func load_into(world: KdWorld) -> Dictionary:
	return world.load_catalogue(paths(build()))


## Each source as one line, "<id> <version> <rules> <world> <look>", the way build.toml lists them.
static func source_lines(loaded: Dictionary) -> PackedStringArray:
	var out := PackedStringArray()
	for s: Dictionary in loaded.get("sources", []):
		out.append("%s %d %s %s %s" % [s["id"], s["version"], s["rules"], s["world"], s["look"]])
	return out


## How many entries loaded, in every kind.
static func entry_count(loaded: Dictionary) -> int:
	var n := 0
	for kind: Dictionary in loaded.get("kinds", []):
		n += (kind["entries"] as Array).size()
	return n


## Implements PLT-04 PRE-43: immutable encoded sizes/hashes, avoiding file reads on camera frames.
static func texture_descriptors(build_file: ConfigFile) -> Dictionary:
	var sizes := {}
	for item: String in build_file.get_value("textures", "sizes", []):
		var words := item.split(" ")
		if words.size() == 2 and words[1].is_valid_int():
			sizes[words[0]] = int(words[1])
	var out := {}
	for item: String in build_file.get_value("textures", "files", []):
		var words := item.split(" ")
		if words.size() == 2 and sizes.has(words[0]):
			out["res://data/textures/" + words[0]] = {
				"path": "res://data/textures/" + words[0],
				"sha256": words[1],
				"max_bytes": sizes[words[0]]
			}
	return out
