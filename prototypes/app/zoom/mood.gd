## P8's light for what stands on the ground (look.gdshaderinc, PRE-30): the look's globals from
## P1's close camp, the colours of the hour from the sun's height, and the camp's fire.
## Pre-production code (research 00).
extends RefCounted


## The look's light for what stands on the ground: the painter's palette, atlas and look from
## P1's close camp at `scene`, and nothing of its camp: no sky map, fires, fires' shadows, outline
## pass or haze of its own, the air's pass giving the haze (A8.6). Returns the painter, with its
## rows.
static func look_globals(scene: String) -> Dictionary:
	var state := (load(scene) as PackedScene).get_state()
	var meta := {}
	for i in state.get_node_property_count(0):
		var key := String(state.get_node_property_name(0, i))
		if key.begins_with("metadata/"):
			meta[key.substr(9)] = state.get_node_property_value(0, i)
	var painter: Dictionary = meta.painter
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_palette", meta.palette)
	set_global.call("look_atlas", meta.atlas)
	set_global.call("look_atlas_grid", float(painter.grid))
	var look: Dictionary = painter.look
	set_global.call("look_contrast", float(look.contrast))
	set_global.call(
		"look_edges", Vector4(look.outline, look.outlineNature, look.lit, painter.edgeK)
	)
	set_global.call("look_sky_cover", float(painter.skyCover))
	var blank := ImageTexture.create_from_image(Image.create_empty(1, 1, false, Image.FORMAT_RGBA8))
	for key in ["look_sky_map", "look_occ_map", "look_occ_floor", "look_gbuf", "look_mirror"]:
		set_global.call(key, blank)
	# the sky map's square far away, so every point sees the open sky
	set_global.call("look_sky_box", Vector4(1.0e9, 1.0e9, 1.0, 0.0))
	set_global.call("look_occ_box", Vector4.ZERO)
	set_global.call("look_fire_powers", Vector4.ZERO)
	set_global.call("look_outline", 0)
	set_global.call("look_haze_k", 0.0)
	return painter


## The look's colours of the hour for the sun `to_sun`, `height` degrees over the focus: the
## painter's noon by day, its dusk near the horizon, its night from 6° below, and mixed between.
## By day the sun's share is scaled to its height, as the painter's own moods are set, so flat
## sunlit ground stays on the step noon puts it on and only slopes show where the sun stands.
static func set_mood(painter: Dictionary, to_sun: Vector3, height: float) -> void:
	var moods: Dictionary = painter.get("moods", {})
	if moods.is_empty():
		return
	var noon: Dictionary = moods.noon.duplicate()
	var noon_height := float(noon.get("el", 52.0))
	var flat := sin(deg_to_rad(noon_height)) / maxf(sin(deg_to_rad(maxf(height, 1.0))), 0.01)
	noon.sunK = float(noon.sunK) * clampf(flat, 1.0, 1.6)
	var a: Dictionary = noon
	var b: Dictionary = noon
	var t := 0.0
	if height < -6.0:
		a = moods.night
		b = moods.night
	elif height < 5.0:
		a = moods.night
		b = moods.dusk
		t = (height + 6.0) / 11.0
	elif height < 25.0:
		a = moods.dusk
		t = smoothstep(5.0, 25.0, height)
	var set_global := RenderingServer.global_shader_parameter_set
	set_global.call("look_sun_dir", to_sun)
	for key: String in ["sunK", "skyK", "shift", "desat"]:
		var name_of: String = {
			"sunK": "look_sun_k", "skyK": "look_sky_k", "shift": "look_shift", "desat": "look_desat"
		}[key]
		set_global.call(name_of, lerpf(float(a[key]), float(b[key]), t))
	set_global.call(
		"look_sun_pow", lerpf(float(a.get("sunPow", 1.0)), float(b.get("sunPow", 1.0)), t)
	)
	for key: String in ["sun", "shade", "fire", "haze", "sky"]:
		var name_of: String = {
			"sun": "look_sun_tint",
			"shade": "look_shade_tint",
			"fire": "look_fire_tint",
			"haze": "look_haze_col",
			"sky": "look_sky_col"
		}[key]
		var c := Color(a[key]).lerp(Color(b[key]), t)
		set_global.call(name_of, Vector3(c.r, c.g, c.b))
	# a fire's light shows on sunlit ground only once the sun is low or gone
	set_global.call("look_fire_in_sun", clampf(1.0 - (height + 6.0) / 30.0, 0.0, 1.0))


## The camp's fire (MAT-18): low by day, high from dusk, its light on what stands round it and on
## the ground (look_core.gdshaderinc), flickering 10 times a second near its flames; `frame` counts
## the frames drawn.
static func light_fire(camp: RefCounted, to_sun: Vector3, frame: int) -> void:
	var height := rad_to_deg(asin(clampf(to_sun.y, -1.0, 1.0)))
	var burn := clampf(1.0 - (height - 10.0) / 30.0, 0.35, 1.0)
	camp.set_fire(burn)
	var at: Vector3 = camp.root.position + camp.flames()
	var fires := Projection()
	fires.x = Vector4(at.x, at.y, at.z, 13.0)
	var rs := RenderingServer
	rs.global_shader_parameter_set("look_fires", fires)
	rs.global_shader_parameter_set("look_fire_powers", Vector4(burn, 0.0, 0.0, 0.0))
	if frame % 6 == 0:
		var flicker := 0.82 + 0.18 * float((frame / 6 * 7919) % 101) / 100.0
		rs.global_shader_parameter_set("look_fire_flicker", Vector4(flicker, 1.0, 1.0, 1.0))
