## Calibration scene C6's figures, and its reads in the vertex stage (A18.1, α2.2b). Stand-in
## figures on a skeleton of 24 bones, walking in place, set out over the ground the camera sees,
## posed by the poser 10 times a second and drawn by Godot's own skeletons or by our bone palettes
## in one MultiMesh (KdFigures, A6.3); and points, one a vertex, each reading 0, 1, 3 or 12 texture
## pixels of a table laid out as a palette is. Built for the Calibrate page; the cloud's run counts
## what each draws and checks that the palettes bend every vertex where Godot's skeletons do.
## Implements PRE-27 and PLT-04.
class_name CalibrationFigures
extends RefCounted

## The figures' shader for each way.
const SHADERS := {
	"godot": preload("res://look/figure.gdshader"),
	"palette": preload("res://look/figure-palette.gdshader"),
}
## The points' shader for each count of texture pixels read.
const READS := {
	0: preload("res://look/reads-0.gdshader"),
	1: preload("res://look/reads-1.gdshader"),
	3: preload("res://look/reads-3.gdshader"),
	12: preload("res://look/reads-12.gdshader"),
}
## The table the points read: a palette's, 24 bones of three texels each, for 300 figures.
const TABLE := Vector2i(72, 300)


## `count` figures set out over the ground the camera sees, drawn one way, under a node; the node,
## whose metadata holds the figures (a KdFigures, which the page poses each frame), the copies, the
## draws as Godot counts them (an instance each) and the triangles a pass. screen: the screen's size
## as the camera's rays take it.
static func figures(
	under: Node3D, camera: Camera3D, screen: Vector2, count: int, way: String
) -> Node3D:
	var holder := Node3D.new()
	under.add_child(holder)
	var aspect := screen.x / screen.y if screen.x > 0.0 and screen.y > 0.0 else 1.0
	var across := maxi(1, ceili(sqrt(count * aspect)))
	var grid := CalibrationDrawing.cells(camera, screen, across, ceili(float(count) / across))
	var places := PackedVector3Array()
	for i in count:
		places.append(grid[i]["at"])
	var drawn := KdFigures.new()
	var shader: Shader = SHADERS[way]
	drawn.build(under.get_world_3d().scenario, way, shader.get_rid(), places, false)
	holder.set_meta("figures", drawn)
	holder.set_meta("copies", count)
	# a figure on Godot's skeleton is an instance of its own; the palettes' are one MultiMesh
	holder.set_meta("draws", count if way == "godot" else 1)
	holder.set_meta("triangles", count * drawn.triangles())
	return holder


## `thousands` thousand points over the screen, each reading `reads` texture pixels in the vertex
## stage, under a node; the node, whose metadata holds its draws. screen: the screen's size.
static func reads(under: Node3D, screen: Vector2, thousands: int, reads: int) -> Node3D:
	var count := thousands * 1000
	# all at the origin in the mesh: the shader sets each point out on the screen by its index
	var points := PackedVector3Array()
	points.resize(count)
	var arrays := []
	arrays.resize(Mesh.ARRAY_MAX)
	arrays[Mesh.ARRAY_VERTEX] = points
	var mesh := ArrayMesh.new()
	mesh.add_surface_from_arrays(Mesh.PRIMITIVE_POINTS, arrays)
	# bounds round the view's focus, so Godot never culls them
	mesh.custom_aabb = AABB(Vector3(-50.0, -1.0, -50.0), Vector3(100.0, 2.0, 100.0))
	var table := Image.create(TABLE.x, TABLE.y, false, Image.FORMAT_RGBAH)
	table.fill(Color(0.5, 0.25, 0.125, 1.0))
	var aspect := screen.x / screen.y if screen.x > 0.0 and screen.y > 0.0 else 1.0
	var across := maxi(1, ceili(sqrt(count * aspect)))
	var material := ShaderMaterial.new()
	material.shader = READS[reads]
	material.set_shader_parameter("kd_table", ImageTexture.create_from_image(table))
	material.set_shader_parameter("kd_grid", Vector2i(across, ceili(float(count) / across)))
	mesh.surface_set_material(0, material)
	var node := MeshInstance3D.new()
	node.mesh = mesh
	node.cast_shadow = GeometryInstance3D.SHADOW_CASTING_SETTING_OFF
	node.set_meta("draws", 1)
	under.add_child(node)
	return node
