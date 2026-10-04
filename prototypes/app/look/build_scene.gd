## P1's scene builder (IMPLEMENTATION α0.2a): turns make_scene.py's pack into close_camp.scn, one
## compressed file of meshes, each surface's attributes in Godot's own formats, with the painter's
## settings, palette and atlas. Run headless:
## `godot --headless --path <app> -s look/build_scene.gd -- <pack folder> <out .scn>`.
## Pre-production code (research 00): thrown away with the prototypes.
extends SceneTree

## Each kind's columns after the position, in make_scene.py's order: the bytes a vertex takes,
## where they go, and their format there (bytes, or half floats).
const COLUMNS := {
	"ground": [[4, "normal", ""], [4, "custom0", "rgba8"]],
	"solid":
	[
		[4, "normal", ""],
		[4, "custom0", "rgba8"],
		[8, "custom1", "rgba_half"],
		[4, "custom2", "rgba8"],
		[4, "custom3", "rgba8"]
	],
	"cards":
	[
		[4, "normal", ""],
		[8, "custom0", "rgba_half"],
		[4, "custom1", "rgba8"],
		[4, "custom2", "rg_half"],
		[4, "custom3", "rgba8"]
	],
	"water": [[8, "custom0", "rgba_half"]],
	"puffs": [[16, "custom0", "two_rgba_half"]],
}
const FORMATS := {
	"rgba8": Mesh.ARRAY_CUSTOM_RGBA8_UNORM,
	"rgba_half": Mesh.ARRAY_CUSTOM_RGBA_HALF,
	"rg_half": Mesh.ARRAY_CUSTOM_RG_HALF,
}


func _init() -> void:
	var args := OS.get_cmdline_user_args()
	var meta: Dictionary = JSON.parse_string(
		FileAccess.get_file_as_string(args[0].path_join("pack.json"))
	)
	var data := FileAccess.get_file_as_bytes(args[0].path_join("pack.bin"))
	var root := Node3D.new()
	root.name = "CloseCamp"
	var count := 0
	for piece: Dictionary in meta.meshes:
		var node := MeshInstance3D.new()
		node.name = "%s_%d" % [piece.kind, count]
		node.mesh = _mesh(piece, data)
		node.set_meta("kind", piece.kind)
		for key in ["layers", "layerPat", "row", "shift"]:
			if piece.has(key):
				node.set_meta(key, piece[key])
		root.add_child(node)
		node.owner = root
		count += 1
	var atlas: Dictionary = meta.atlas
	var size: int = atlas.size
	var pixels := data.slice(int(atlas.offset), int(atlas.offset) + size * size * 4)
	root.set_meta(
		"atlas",
		ImageTexture.create_from_image(
			Image.create_from_data(size, size, false, Image.FORMAT_RGBA8, pixels)
		)
	)
	root.set_meta("palette", ImageTexture.create_from_image(_palette(meta.palette)))
	for key in ["atlas", "meshes", "palette"]:
		meta.erase(key)
	root.set_meta("painter", meta)
	var scene := PackedScene.new()
	scene.pack(root)
	var error := ResourceSaver.save(scene, args[1], ResourceSaver.FLAG_COMPRESS)
	print(
		(
			"Scene: %s, %d meshes, %s"
			% [args[1], count, "saved" if error == OK else "error %d" % error]
		)
	)
	root.free()
	quit(0 if error == OK else 1)


## One surface from a piece: positions as floats, normals from bytes, custom channels as they are,
## and 16-bit indices.
func _mesh(piece: Dictionary, data: PackedByteArray) -> ArrayMesh:
	var n: int = piece.vertices
	var offsets: Array = piece.columns
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	var floats := data.slice(int(offsets[0]), int(offsets[0]) + n * 12).to_float32_array()
	var points := PackedVector3Array()
	points.resize(n)
	for i in n:
		points[i] = Vector3(floats[i * 3], floats[i * 3 + 1], floats[i * 3 + 2])
	arrays[Mesh.ARRAY_VERTEX] = points
	var flags := 0
	var columns: Array = COLUMNS[piece.kind]
	for c in columns.size():
		var width: int = columns[c][0]
		var where: String = columns[c][1]
		var format: String = columns[c][2]
		var at := int(offsets[c + 1])
		var bytes := data.slice(at, at + n * width)
		if where == "normal":
			var normals := PackedVector3Array()
			normals.resize(n)
			for i in n:
				normals[i] = (
					Vector3(
						bytes.decode_s8(i * 4),
						bytes.decode_s8(i * 4 + 1),
						bytes.decode_s8(i * 4 + 2)
					)
					. normalized()
				)
			arrays[Mesh.ARRAY_NORMAL] = normals
		elif format == "two_rgba_half":
			# two half-float channels' worth: split into custom0 and custom1
			var first := PackedByteArray()
			var second := PackedByteArray()
			first.resize(n * 8)
			second.resize(n * 8)
			for i in n:
				for k in 8:
					first[i * 8 + k] = bytes[i * 16 + k]
					second[i * 8 + k] = bytes[i * 16 + 8 + k]
			arrays[Mesh.ARRAY_CUSTOM0] = first
			arrays[Mesh.ARRAY_CUSTOM1] = second
			flags |= Mesh.ARRAY_CUSTOM_RGBA_HALF << Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT
			flags |= Mesh.ARRAY_CUSTOM_RGBA_HALF << Mesh.ARRAY_FORMAT_CUSTOM1_SHIFT
		else:
			var channel := int(where.substr(6))
			arrays[Mesh.ARRAY_CUSTOM0 + channel] = bytes
			flags |= (
				FORMATS[format]
				<< (Mesh.ARRAY_FORMAT_CUSTOM0_SHIFT + channel * Mesh.ARRAY_FORMAT_CUSTOM_BITS)
			)
	var tris: int = piece.triangles
	var raw := data.slice(int(piece.index), int(piece.index) + tris * 6)
	# three.js's front faces wind counter-clockwise and Godot's clockwise: each triangle is reversed
	var index := PackedInt32Array()
	index.resize(tris * 3)
	for t in tris:
		index[t * 3] = raw.decode_u16(t * 6)
		index[t * 3 + 1] = raw.decode_u16(t * 6 + 4)
		index[t * 3 + 2] = raw.decode_u16(t * 6 + 2)
	arrays[Mesh.ARRAY_INDEX] = index
	var mesh := ArrayMesh.new()
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays, [], {}, flags)
	return mesh


## Every material's ladder of 7 shades as one row of an 8 × N picture, in the painter's sRGB.
func _palette(rows: Array) -> Image:
	var image := Image.create_empty(8, rows.size(), false, Image.FORMAT_RGBA8)
	for y in rows.size():
		var ramp: Array = rows[y]
		for x in ramp.size():
			image.set_pixel(x, y, Color(ramp[x]))
	return image
