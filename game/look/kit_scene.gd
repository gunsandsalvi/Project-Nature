## The model kit for the pages that draw it (A6.1, CLAUDE.md rule 4): the build's families read, the
## world's recipes given to the kit, the shader every part is drawn with, and the texture every role
## of a recipe may wear made, so the Kit and Pilot pages load it the same way. Implements PRE-46.
class_name KitScene
extends RefCounted


## Loads the build's kit into a KdKit once the world's catalogue is loaded: {"families": the names
## of the families that loaded, "problems": every fault found, in words, none when all is well}.
static func load_into(kit: KdKit, world: KdWorld, shader: RID) -> Dictionary:
	var families := PackedStringArray()
	var problems := PackedStringArray()
	var build := GameData.build()
	var files := GameData.models(build)
	for family: String in files:
		var failed := kit.load_family(family, files[family])
		if failed.is_empty():
			families.append(family)
		else:
			problems.append(failed)
	kit.use_world(world)
	kit.set_shader(shader)
	for name: String in kit.texture_names():
		var record := world.entry("textures", name)
		if record.is_empty():
			problems.append("the recipes name the texture %s, which the catalogue lacks" % name)
			continue
		var failed := kit.set_texture(
			name, GameData.texture_path(name), GameData.tile_metres(record)
		)
		if not failed.is_empty():
			problems.append(failed)
	return {"families": families, "problems": problems}
