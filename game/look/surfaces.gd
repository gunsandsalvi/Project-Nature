## A big surface's tiles as the catalogue names them (A5.3, A5.4): art:meadow is its near tile,
## art:meadow/v2 a version of it, art:meadow/middle and art:meadow/far the other two tiles, each
## with its own versions after it (art:meadow/middle/v2). Gathers a surface's files in the order
## the ladder wants them, the near tile's versions first, then the middle tile's and the far's,
## with how many each has and the band each starts at, for the area to make its texture array from
## (game/look/ladder.gdshaderinc); the Lab page lists a material's tiles by the same names.
## Implements PRE-20 and PRE-22.
class_name Surfaces
extends RefCounted

## The tiles of a surface, nearest first, as their names call them.
const TILES := ["near", "middle", "far"]
## The band a tile starts at when a surface has none of it: past the last band any surface serves.
const NONE := 99


## Every texture the catalogue holds, by its entry's name, in the order it lists them, from what
## KdWorld.load_catalogue gave.
static func texture_names(loaded: Dictionary) -> Array[String]:
	var names: Array[String] = []
	for kind: Dictionary in loaded.get("kinds", []):
		if kind["folder"] == "textures":
			for entry: Dictionary in kind["entries"]:
				names.append(str(entry["name"]))
	return names


## The material a texture belongs to: "meadow" for "art:meadow/middle/v2".
static func material_of(name: String) -> String:
	return name.trim_prefix("art:").get_slice("/", 0)


## Texture names in the order a material's sheet lists them: by material, then its near tile, its
## middle and its far, each tile's versions in order.
static func sorted(names: Array[String]) -> Array[String]:
	var listed := names.duplicate()
	listed.sort_custom(func(a: String, b: String) -> bool: return _rank(a) < _rank(b))
	return listed


## Which tile of a material a texture is: "near", "middle" or "far", or "" for one of another
## material. A name after the material that is not a tile, such as a version, is the near tile's.
static func tile_of(material: String, name: String) -> String:
	var base := "art:" + material
	if name != base and not name.begins_with(base + "/"):
		return ""
	var word := name.trim_prefix(base).trim_prefix("/").get_slice("/", 0)
	return word if word == "middle" or word == "far" else "near"


## Which version of its tile a texture is: 1 for the tile as it is, 2 for v2, and so on.
static func version_of(name: String) -> int:
	var last := name.get_slice("/", name.get_slice_count("/") - 1)
	if last.length() > 1 and last.begins_with("v") and last.substr(1).is_valid_int():
		return int(last.substr(1))
	return 1


## A material's tiles from the catalogue's texture names, for KdArea.set_surface: {"paths",
## "count", "first"}, or {"problem"}. The first band of each tile is its record's own; for a tile
## the material has none of, NONE.
static func ladder(world: KdWorld, names: Array[String], material: String) -> Dictionary:
	var tiles := {"near": [], "middle": [], "far": []}
	for name: String in names:
		var tile := tile_of(material, name)
		if tile != "":
			(tiles[tile] as Array).append(name)
	if (tiles["near"] as Array).is_empty():
		return {"problem": "the catalogue has no texture art:%s" % material}
	var paths := PackedStringArray()
	var count := PackedInt32Array()
	var first := PackedInt32Array()
	for tile: String in TILES:
		var list: Array = tiles[tile]
		list.sort_custom(func(a: String, b: String) -> bool: return version_of(a) < version_of(b))
		count.append(list.size())
		first.append(
			NONE if list.is_empty() else int(world.entry("textures", list[0]).get("first_band", 0))
		)
		for name: String in list:
			paths.append(GameData.texture_path(name))
	return {"paths": paths, "count": count, "first": first}


## What sorts a name into its place in a material's sheet: the material, the tile's place, the
## version's.
static func _rank(name: String) -> String:
	var material := material_of(name)
	return "%s %d %03d %s" % [material, TILES.find(tile_of(material, name)), version_of(name), name]
