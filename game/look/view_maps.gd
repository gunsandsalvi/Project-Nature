## The view's maps round the things that stand on the ground (A4.4, A4.3), for the pages that put
## things on it: made from the triangles of the kit's things by the view's own class (KdMaps), with
## the numbers of tuning/light and the sun it names, and published as the globals the shared light
## function reads: openness, contact, the sun's angle and distance, and the heights of the tops. The
## maps lie in the world's own metres; where the rig's origin lies moves them in its (A8.2), so the
## page tells the maps whenever the origin moves. Until a page makes them, Afternoon's stand-ins
## hold the globals: open sky, no contact, nothing in the sun's way. The area's patch picture is
## made and followed here too. Implements PRE-20, PRE-21, PRE-24 and PRE-30.
class_name ViewMaps
extends RefCounted

## The square the maps cover: its west and south edges in metres east and north of the world's
## centre, its side, the farthest caster's distance they hold, and the texels under things.
var west := 0.0
var south := 0.0
var width := 0.0
var reach := 14.0
var footprint := 0
## The pictures, once made: the four channels and the tops.
var view: ImageTexture
var tops: ImageTexture
## The area's patch picture (A4.6), once made, and the square it lies in: west and south edges
## in metres east and north of the world's centre, and its side.
var patches: ImageTexture
var patches_west := 0.0
var patches_south := 0.0
var patches_width := 0.0
## What the ground's shader was last given for the picture: its square in the rig's metres, and
## the earth's colour (linear) with the swing of the ground's colour with growth.
var patches_place := Vector4.ZERO
var patches_look := Vector4.ZERO


## Makes the maps round the things of a kit (their ids) and publishes them; "" or the problem.
func build(kit: KdKit, ids: Array, world: KdWorld) -> String:
	var tuning := world.entry("tuning/light", "base:light")
	if tuning.is_empty():
		return "the catalogue has no tuning/light"
	var triangles := PackedVector3Array()
	for id: int in ids:
		triangles.append_array(kit.thing_triangles(id))
	var toward := Afternoon.toward_the_sun(int(tuning["sun_height"]), int(tuning["sun_turn"]))
	var made := KdMaps.new().make(triangles, toward, params_of(tuning))
	if str(made["problem"]) != "":
		return str(made["problem"])
	if int(made["size"]) == 0:
		return ""
	west = float(made["west"])
	south = float(made["south"])
	width = float(made["width"])
	reach = float(made["shadow_reach"])
	footprint = int(made["footprint"])
	view = ImageTexture.create_from_image(made["view"])
	tops = ImageTexture.create_from_image(made["tops"])
	RenderingServer.global_shader_parameter_set("kd_view_maps", view)
	RenderingServer.global_shader_parameter_set("kd_view_tops", tops)
	return ""


## Makes the area's patch picture round a camp (its place in metres east and north) from the
## numbers of tuning/area, and publishes it with the look the ground reads it by; "" or the
## problem. Implements PRE-20 and WLD-31 (see A4.6 and A5.3).
func build_patches(world: KdWorld, camp_east: float, camp_north: float) -> String:
	var tuning := world.entry("tuning/area", "base:area")
	if tuning.is_empty():
		return "the catalogue has no tuning/area"
	var made := KdMaps.new().make_patches(camp_east, camp_north, patches_of(tuning))
	if str(made["problem"]) != "":
		return str(made["problem"])
	patches_west = float(made["west"])
	patches_south = float(made["south"])
	patches_width = float(made["width"])
	patches = ImageTexture.create_from_image(made["picture"])
	var earth := Color(str(tuning["earth_colour"])).srgb_to_linear()
	var swing := float(tuning["growth_swing"]) / 1.0e6
	patches_look = Vector4(earth.r, earth.g, earth.b, swing)
	RenderingServer.global_shader_parameter_set("kd_patches", patches)
	RenderingServer.global_shader_parameter_set("kd_patch_look", patches_look)
	return ""


## Puts the maps in the rig's metres, for where its origin lies in centimetres east and north.
func follow(origin_east: int, origin_north: int) -> void:
	if view != null:
		var place := Vector4(
			west - float(origin_east) / 100.0, south - float(origin_north) / 100.0, width, reach
		)
		RenderingServer.global_shader_parameter_set("kd_maps_place", place)
	if patches != null:
		patches_place = Vector4(
			patches_west - float(origin_east) / 100.0,
			patches_south - float(origin_north) / 100.0,
			patches_width,
			0.0
		)
		RenderingServer.global_shader_parameter_set("kd_patches_place", patches_place)


## The maps' numbers from the light's tuning: metres for its lengths, shares for its ratios.
static func params_of(tuning: Dictionary) -> Dictionary:
	return {
		"texel": float(tuning["maps_texel"]) / 1000.0,
		"open_reach": float(tuning["open_reach"]) / 1000.0,
		"open_strength": float(tuning["open_strength"]) / 1.0e6,
		"contact_width": float(tuning["contact_width"]) / 1000.0,
		"contact_strength": float(tuning["contact_strength"]) / 1.0e6,
		"shadow_reach": float(tuning["shadow_reach"]) / 1000.0,
	}


## The patch picture's numbers from the area's tuning: metres for its lengths, shares for its
## ratios.
static func patches_of(tuning: Dictionary) -> Dictionary:
	return {
		"seed": float(tuning["patch_seed"]),
		"growth_scale": float(tuning["growth_scale"]) / 1000.0,
		"bare_below": float(tuning["bare_below"]) / 1.0e6,
		"clearing": float(tuning["clearing"]) / 1000.0,
		"clearing_fade": float(tuning["clearing_fade"]) / 1000.0,
	}
