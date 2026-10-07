## The look's light for the moment it shows (A4.3, PRE-30): the sun, the sky's fill and the globals
## the shared light function reads, from base/tuning/light.toml, so no number of the light is in
## code. Stand-ins for the view's maps and the fire grid, which the pages that draw the pilot's
## pieces need to hold still: open sky, no contact and no fires, until the view makes them (α2.3b).
## Implements PRE-30.
class_name Afternoon
extends RefCounted


## Puts the sun, named "Sun", and the sky's fill and background, named "Sky", under a node, and
## publishes the light's globals; "" or the problem.
static func apply(world: KdWorld, under: Node3D) -> String:
	var tuning := world.entry("tuning/light", "base:light")
	if tuning.is_empty():
		return "the catalogue has no tuning/light"
	var toward := toward_the_sun(int(tuning["sun_height"]), int(tuning["sun_turn"]))
	var sun := DirectionalLight3D.new()
	sun.name = "Sun"
	sun.transform = Transform3D().looking_at(-toward, Vector3.UP)
	sun.light_color = Color(str(tuning["sun_colour"]))
	sun.light_energy = _share(tuning["sun_energy"])
	sun.shadow_enabled = true
	under.add_child(sun)
	var environment := Environment.new()
	environment.background_mode = Environment.BG_COLOR
	environment.background_color = Color(str(tuning["sky_colour"]))
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color(str(tuning["ambient_colour"]))
	environment.ambient_light_energy = _share(tuning["ambient_energy"])
	environment.tonemap_mode = Environment.TONE_MAPPER_AGX
	var holder := WorldEnvironment.new()
	holder.name = "Sky"
	holder.environment = environment
	under.add_child(holder)
	_publish(tuning, toward)
	return ""


## The direction toward the sun in the world: x east, y up, z south, from its height in degrees
## above the horizon and its turn in degrees clockwise from north.
static func toward_the_sun(height: int, turn: int) -> Vector3:
	var up := deg_to_rad(float(height))
	var around := deg_to_rad(float(turn))
	return Vector3(sin(around) * cos(up), sin(up), -cos(around) * cos(up))


static func _share(parts_per_million: Variant) -> float:
	return float(parts_per_million) / 1.0e6


static func _linear(hex: Variant) -> Color:
	return Color(str(hex)).srgb_to_linear()


## The light's globals: the view's maps with the sky open and nothing between a point and the sun,
## a light grid with no fires, the sun's direction, the light bounced from the ground, the haze and
## the sky's colour.
static func _publish(tuning: Dictionary, toward: Vector3) -> void:
	var maps := Image.create(4, 4, false, Image.FORMAT_RGBA8)
	maps.fill(Color(1.0, 1.0, 0.0, 0.0))
	_global("kd_view_maps", ImageTexture.create_from_image(maps))
	_global("kd_maps_place", Vector4(-32.0, -32.0, 64.0, 20.0))
	var grid := Image.create(64, 64, false, Image.FORMAT_RGBA8)
	grid.fill(Color8(0, 0, 0, 0))
	_global("kd_fire_grid", ImageTexture.create_from_image(grid))
	var table := Image.create(4, 1, false, Image.FORMAT_RGBAF)
	table.fill(Color(0.0, 0.0, 0.0, 0.0))
	_global("kd_fire_table", ImageTexture.create_from_image(table))
	_global("kd_grid_place", Vector4(-128.0, -128.0, 4.0, 0.0))
	_global("kd_sun_toward", toward)
	var bounce := _linear(tuning["bounce_colour"])
	_global("kd_bounce", Vector3(bounce.r, bounce.g, bounce.b))
	var haze := _linear(tuning["haze_colour"])
	_global("kd_haze", Vector4(haze.r, haze.g, haze.b, _share(tuning["haze_density"])))
	var haze_sun := _linear(tuning["haze_sun_colour"])
	_global("kd_haze_sun", Vector3(haze_sun.r, haze_sun.g, haze_sun.b))
	# the sky's colour, which water mirrors by angle
	var sky := _linear(tuning["sky_colour"])
	_global("kd_sky", Vector3(sky.r, sky.g, sky.b))


static func _global(name: String, value: Variant) -> void:
	RenderingServer.global_shader_parameter_set(name, value)
