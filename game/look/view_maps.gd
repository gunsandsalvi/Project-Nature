## The view's maps round the things that stand on the ground (A4.4, A4.3), for the pages that put
## things on it: made from the triangles of the kit's things by the view's own class (KdMaps), with
## the numbers of tuning/light and the sun it names, and published as the globals the shared light
## function reads: openness, contact, the sun's angle and distance, and the heights of the tops. The
## maps lie in the world's own metres; where the rig's origin lies moves them in its (A8.2), so the
## page tells the maps whenever the origin moves. Until a page makes them, Afternoon's stand-ins
## hold the globals: open sky, no contact, nothing in the sun's way. Implements PRE-21, PRE-24 and
## PRE-30.
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


## Puts the maps in the rig's metres, for where its origin lies in centimetres east and north.
func follow(origin_east: int, origin_north: int) -> void:
	if view == null:
		return
	var place := Vector4(
		west - float(origin_east) / 100.0, south - float(origin_north) / 100.0, width, reach
	)
	RenderingServer.global_shader_parameter_set("kd_maps_place", place)


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
