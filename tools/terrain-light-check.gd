## checks: PRE-20 PRE-21 PRE-30 (T2.8a.1/3): rendered normals, materials, fire and emission.
extends SceneTree

const SPRITE := preload("res://terrain/sprite.gdshader")


func _init() -> void:
	var image := Image.create(32, 32, false, Image.FORMAT_RGBA8)
	image.fill(Color(0.4, 0.4, 0.4, 1))
	var albedo := ImageTexture.create_from_image(image)
	var tuning: Dictionary = JSON.parse_string(
		FileAccess.get_file_as_string("res://terrain/materials.json")
	)
	var table := PackedVector4Array()
	for values: Array in tuning.values:
		table.append(Vector4(values[0], values[1], values[2], values[3]))
	var basis: float = KdCanvas.new().frame(1080, 2400).height_basis
	var names := [
		"east", "north", "up", "shade", "emit", "emit_shade", "mask_emit", "fire", "fire_blocked"
	]
	for index in names.size():
		var sprite := Sprite2D.new()
		sprite.centered = false
		sprite.position = Vector2(4 + 36 * index, 4)
		sprite.texture = albedo
		var material := ShaderMaterial.new()
		material.shader = SPRITE
		material.set_shader_parameter("height_basis", basis)
		material.set_shader_parameter("density", 32.0)
		material.set_shader_parameter("pivot", Vector2(16, 32))
		material.set_shader_parameter("sun_direction", Vector3(1, 0, 0))
		material.set_shader_parameter("sunlight", Vector3(0.6, 0.6, 0.6))
		material.set_shader_parameter("sky_light", Vector3(0.1, 0.1, 0.1))
		material.set_shader_parameter("material_table", table)
		material.set_shader_parameter("material_kind", 9 if index in [4, 5] else 0)
		material.set_shader_parameter("sun_bands", Vector3.ZERO if index in [3, 5] else Vector3.ONE)
		material.set_shader_parameter("fire_on", index >= 7)
		material.set_shader_parameter("fire_position", Vector3(0, 0, 2))
		material.set_shader_parameter("fire_bands", Vector3.ZERO if index == 8 else Vector3.ONE)
		var normal := Image.create(1, 1, false, Image.FORMAT_RGBAF)
		normal.fill(
			(
				Color(1, 0.5, 0.5)
				if index == 0
				else Color(0.5, 1, 0.5) if index == 1 else Color(0.5, 0.5, 1)
			)
		)
		material.set_shader_parameter("normal_atlas", ImageTexture.create_from_image(normal))
		material.set_shader_parameter("has_normal", true)
		if index == 6:
			var mask := Image.create(1, 1, false, Image.FORMAT_RGBA8)
			mask.fill(Color(9.0 / 255.0, 0, 0))
			material.set_shader_parameter("material_atlas", ImageTexture.create_from_image(mask))
			material.set_shader_parameter("has_material", true)
		sprite.material = material
		root.add_child(sprite)
	for i in 8:
		await process_frame
	await RenderingServer.frame_post_draw
	var pixels := root.get_texture().get_image()
	var values := {}
	for index in names.size():
		values[names[index]] = pixels.get_pixel(20 + 36 * index, 20).r
	print("Lighting pixel checks: ", JSON.stringify(values))
	var passed: bool = values.east > values.north + 0.08 and absf(values.up - values.north) < 0.02
	passed = passed and values.emit > 0.5 and absf(values.emit - values.emit_shade) < 0.005
	passed = passed and absf(values.mask_emit - values.emit) < 0.005
	passed = passed and values.fire > values.fire_blocked + 0.08
	var args := OS.get_cmdline_user_args()
	if not args.is_empty():
		pixels.save_png(args[0] + "-light-check.png")
		var evidence := FileAccess.open(args[0] + "-light-check.json", FileAccess.WRITE)
		evidence.store_string(JSON.stringify({"pixels": values, "passed": passed}, "\t") + "\n")
	if not passed:
		printerr(
			"Lighting contract failed: normal basis, albedo, material packing, emission or local light."
		)
		quit(1)
		return
	print("Lighting contracts passed on the actual renderer.")
	quit(0)
