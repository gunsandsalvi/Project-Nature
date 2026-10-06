## The look's scene for the pages that draw it (A4.1, A5.5): the sun, the sky's light and the
## stand-in ground, built once for the Look page and the Compare page (CLAUDE.md rule 4).
## Implements PRE-01.
class_name LookScene
extends RefCounted

const GROUND_SHADER := preload("res://look/ground.gdshader")
const TEXTURES := "res://data/textures/"
const LAYERS := ["standin-meadow", "standin-pattern"]


## Puts the sun, named "Sun", and the sky's light under a node, and the stand-in ground into its
## world, drawn by the look's class; "" or the problem that kept the ground from loading.
static func build(look: KdLook, under: Node3D) -> String:
	var sun := DirectionalLight3D.new()
	sun.name = "Sun"
	sun.rotation_degrees = Vector3(-50.0, 30.0, 0.0)
	sun.light_color = Color("#fff4e0")
	sun.shadow_enabled = true
	under.add_child(sun)
	var environment := Environment.new()
	environment.background_mode = Environment.BG_COLOR
	environment.background_color = Color("#a9c4d8")
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color("#a9b8c8")
	environment.ambient_light_energy = 0.45
	environment.tonemap_mode = Environment.TONE_MAPPER_AGX
	var holder := WorldEnvironment.new()
	holder.environment = environment
	under.add_child(holder)
	var paths := PackedStringArray()
	for layer: String in LAYERS:
		paths.append(TEXTURES + layer + ".kdtex")
	var problem := look.load_layers(paths)
	if problem.is_empty():
		look.build(under.get_world_3d().scenario, GROUND_SHADER.get_rid())
	return problem
