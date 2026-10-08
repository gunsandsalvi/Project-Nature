## checks: PRE-21 PRE-30: actual Compatibility pixels, linear albedo and indirect-only contact.
extends SceneTree


func _initialize() -> void:
	call_deferred("run")


func run() -> void:
	DisplayServer.window_set_size(Vector2i(256, 80))
	root.size = Vector2i(256, 80)
	var source := FileAccess.get_file_as_string("res://terrain/light.gdshaderinc")
	var shader := Shader.new()
	shader.code = (
		"shader_type canvas_item; render_mode unshaded;\n"
		+ source
		+ "\nuniform vec4 test_visibility; void fragment() { COLOR=vec4(fixture_light("
		+ "texture(TEXTURE,UV).rgb,vec3(0,0,1),vec3(0),test_visibility,0.0,0),1.0); }"
	)
	var white := Image.create(1, 1, false, Image.FORMAT_RGBA8)
	white.fill(Color8(229, 229, 229))
	var table := PackedVector4Array()
	for i in 10:
		table.append(Vector4(1, 1, 0, 0))
	for i in 4:
		var patch := TextureRect.new()
		patch.texture = ImageTexture.create_from_image(white)
		patch.position = Vector2(i * 64, 0)
		patch.size = Vector2(64, 80)
		patch.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
		var material := ShaderMaterial.new()
		material.shader = shader
		material.set_shader_parameter("sun_direction", Vector3(0, 0, 1))
		material.set_shader_parameter("sunlight", Vector3.ONE * (0.355 if i < 2 else 0.0))
		material.set_shader_parameter("sky_light", Vector3.ONE * (0.0 if i < 2 else 0.355))
		material.set_shader_parameter("fire_on", false)
		material.set_shader_parameter("material_table", table)
		material.set_shader_parameter("test_visibility", Vector4(1, 1, 1 if i % 2 == 0 else 0.1, 1))
		patch.material = material
		root.add_child(patch)
	for i in 8:
		await process_frame
	await RenderingServer.frame_post_draw
	var pixels := root.get_texture().get_image()
	var values := PackedInt32Array()
	for i in 4:
		values.append(pixels.get_pixel(i * 64 + 32, 40).r8)
	var passed: bool = (
		abs(values[0] - 144) <= 1
		and values[1] == values[0]
		and values[2] == values[0]
		and abs(values[3] - 47) <= 2
	)
	print("LIGHT EQUATION ", values, " passed=", passed)
	quit(0 if passed else 1)
