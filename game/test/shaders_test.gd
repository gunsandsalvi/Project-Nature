## The look's shaders, scanned (A4.1, T2.3b tests): nothing in the main picture reads the screen or
## its depth, so Godot draws the whole picture without it leaving the chip (the shore line comes
## from height, never from depth). The scan reads every shader and include in res://look/ for what
## Godot needs to read the screen, its depth or its normals.
extends GdUnitTestSuite

const FORBIDDEN := [
	"hint_screen_texture",
	"hint_depth_texture",
	"hint_normal_roughness_texture",
	"SCREEN_TEXTURE",
	"DEPTH_TEXTURE",
	"NORMAL_ROUGHNESS_TEXTURE"
]


func _shaders() -> PackedStringArray:
	var out := PackedStringArray()
	for file in DirAccess.get_files_at("res://look"):
		if file.ends_with(".gdshader") or file.ends_with(".gdshaderinc"):
			out.append("res://look/" + file)
	return out


# checks: PRE-26 PRE-01
func test_no_shader_of_the_look_reads_the_screen_or_its_depth() -> void:
	var shaders := _shaders()
	assert_int(shaders.size()).is_greater(20)
	for path in shaders:
		var code := FileAccess.get_file_as_string(path)
		for word: String in FORBIDDEN:
			(
				assert_bool(code.contains(word))
				. override_failure_message("%s reads the screen or its depth: %s" % [path, word])
				. is_false()
			)


# checks: PRE-26
func test_the_water_s_shaders_are_among_those_scanned() -> void:
	var shaders := _shaders()
	for name in [
		"water.gdshader", "water.gdshaderinc", "land-river.gdshader", "ladder.gdshaderinc"
	]:
		assert_bool(shaders.has("res://look/" + name)).is_true()
