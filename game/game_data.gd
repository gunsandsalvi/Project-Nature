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


## The calibration scenes the build lists (A18.1), by their paths under res://data/, such as
## "scenes/look/c4.toml".
static func calibration_paths(build_file: ConfigFile) -> PackedStringArray:
	var out := PackedStringArray()
	for item: String in build_file.get_value("calibration", "files", []):
		out.append(item.get_slice(" ", 0))
	return out


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
