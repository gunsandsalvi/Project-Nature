## Implements PRC-10, see A17: compiles every GDScript of a Godot project outside its addons,
## so a script error stops tools/check.sh even where no test loads the script.
## Run as `godot --headless --path <project> -s <this file>`; exits 1 on any error.
extends SceneTree


func _init() -> void:
	var failed := 0
	var count := 0
	for path in _scripts("res://"):
		count += 1
		var script := (
			ResourceLoader.load(path, "GDScript", ResourceLoader.CACHE_MODE_IGNORE) as GDScript
		)
		if script == null or script.reload() != OK:
			printerr("Script error: %s" % path)
			failed += 1
	print("Scripts: %d compiled, %d with errors" % [count, failed])
	quit(1 if failed > 0 else 0)


func _scripts(dir: String) -> PackedStringArray:
	var out := PackedStringArray()
	for sub in DirAccess.get_directories_at(dir):
		if not sub.begins_with(".") and not (dir == "res://" and sub == "addons"):
			out.append_array(_scripts(dir.path_join(sub)))
	for file in DirAccess.get_files_at(dir):
		if file.get_extension() == "gd":
			out.append(dir.path_join(file))
	return out
