// P8's ground (A8.1, A8.5), the body its two shaders share: ground.gdshader, the full one; ground_light.gdshader, for
// the zoom's light mode (LIGHT), without the clouds' shadows, the forests' clumps or the sea's currents, and so without
// the weather's picture or any 3D texture. Include planet.gdshaderinc, rivers.gdshaderinc, forest.gdshaderinc and
// near.gdshaderinc first, and for the full one the clouds' includes and look_core.gdshaderinc. With SUN_SHADOW the colour is finished in
// light(), where the sun's shadow map says whether a tree or a thing shades the pixel, and close up the ground takes
// the art book's own look, lit in the look's steps as what stands on it is; without it the pixel is its colour in the
// sun.


instance uniform float spacing = 1.0;  // metres between the chunk's points
instance uniform vec2 morph = vec2(1.0e12, 2.0e12);  // metres from the camera where it starts to morph, and is done
// whether the chunk's trees are drawn as shapes (trees.gdshader): 0 not, so the ground draws the forests' crowns; 1 they
// are, fading out as it morphs while the ground's crowns fade in; 2 they are
instance uniform int trees_drawn = 0;

varying vec2 v_d;  // metres east and north of the focus
varying float v_up;  // metres above the sea, as drawn
varying vec3 v_normal;  // in the picture
varying vec3 v_up_dir;  // the ground's up here, in the picture
varying float v_slope;  // 0 flat to 1 sheer
varying float v_morph;  // how far the chunk has morphed into the grid half as fine here, 0 to 1
#ifdef SUN_SHADOW
// From fragment() to light(): the pixel's colour in the sun, and in a shadow cast on it; how much it takes the art
// book's look close up, and that look's palette row, step and sky light; and how much sun the clouds let through.
varying vec3 v_sunlit;
varying vec3 v_shaded;
varying float v_near;
varying float v_row;
varying float v_bias;
varying float v_sky;
varying float v_cloud;
varying float v_fire;
#endif

#ifndef LIGHT
// How much of the sun reaches a place on the ground `ground` metres above the sea through the clouds of the variant
// shown, 0 to 1, the pixel `fp` metres across; the sky's light still reaches the shade.
float cloud_shadow(vec2 w, float ground, mat3 frame, float fp) {
	vec3 sun = normalize(zoom_sun);
	vec3 local = vec3(dot(sun, frame[0]), dot(sun, frame[1]), dot(sun, frame[2]));
	if (local.y <= 0.03) {
		return 1.0;
	}
	vec2 toward = local.xz / local.y;
	if (int(zoom_style.z + 0.5) == 2) {
		vec2 at = w + toward * (CLOUD_DECK - ground);
		return 1.0 - 0.5 * cloud_puff(at, cloud_weather(at), fp);
	}
	vec4 weather = cloud_weather(w + toward * (3500.0 - ground));
	float top = cloud_top(weather.y);
	float od = 0.0;
	for (int k = 0; k < 3; k++) {
		float alt = mix(CLOUD_BASE, top, (float(k) + 0.5) / 3.0);
		vec2 at = w + toward * (alt - ground);
#ifdef CLOUD_NOISE
		od += cloud_density(at, alt, cloud_weather(at), fp);
#else
		od += cloud_far(alt, cloud_weather(at));
#endif
	}
	od *= CLOUD_SIGMA * (top - CLOUD_BASE) / 3.0 / local.y;
	return mix(0.3, 1.0, exp(-od));
}
#endif

void vertex() {
	vec3 corner = MODEL_MATRIX[3].xyz;
	vec2 local = vec2(VERTEX.x - corner.x, corner.z - VERTEX.z);
	vec2 d0 = vec2(VERTEX.x - zoom_focus_at.x, zoom_focus_at.z - VERTEX.z);
	vec3 p0 = planet_place(d0, VERTEX.y);
	float m = clamp((length(p0 - zoom_eye) - morph.x) / (morph.y - morph.x), 0.0, 1.0);
	// each odd point slides onto the even point before it, so the chunk becomes the grid half as fine
	vec2 g = floor(local / spacing + 0.5);
	vec2 d = d0 - mod(g, 2.0) * spacing * m;
	float up = mix(VERTEX.y, CUSTOM0.x, m);
	vec3 n = normalize(mix(NORMAL, CUSTOM1.xyz, m));
	vec3 p = planet_place(d, up);
	mat3 frame = planet_frame(p);
	v_normal = normalize(frame[0] * n.x + frame[1] * n.y - frame[2] * n.z);
	v_up_dir = frame[1];
	v_slope = 1.0 - n.y;
	v_d = d;
	v_up = up;
	v_morph = m;
	VERTEX = p;
	NORMAL = v_normal;
}

// The sea's currents and eddies, at the scales a pixel `fp` metres across can show: noise warped by noise, drifting
// with the weather's clock. x the pattern, 0 to 1; y its ridges, thin streaks along the flow.
vec2 currents(vec2 d, float fp) {
	vec2 drift = vec2(0.35, 0.12) * zoom_clock.x;
	vec2 q = vec2(0.0);
	for (int o = 0; o < 3; o++) {
		vec3 a = octave_at(d + drift, 1 + o);
		vec3 b = octave_at(d - drift + 50000.0, 1 + o);
		q += vec2(value_noise(a.xy, a.z), value_noise(b.xy, b.z)) * exp2(-float(o));
	}
	q = q / 1.75 - 0.5;
	vec2 at_d = d + 70000.0 * q;
	int finest = octave_for(max(3.0 * fp, 250.0));
	float sum = 0.0;
	float total = 0.0;
	float amp = 1.0;
	for (int k = 2; k <= 10; k++) {
		if (k > finest) {
			break;
		}
		float keep = k == finest ? clamp(octave_cell(k) / max(3.0 * fp, 250.0) - 1.0, 0.0, 1.0) : 1.0;
		vec3 at = octave_at(at_d + drift * 0.5, k);
		sum += amp * keep * value_noise(at.xy, at.z);
		total += amp * keep;
		amp *= 0.6;
	}
	float f = sum / max(total, 1e-3);
	float ridge = 1.0 - abs(2.0 * f - 1.0);
	return vec2(f, pow(ridge, 8.0));
}

// The water's own colour `depth` metres deep at a place: the art book's map (variant A), the shelves light, the deep
// sea one colour, in bands; or (B, C) deep navy, the open sea's blue, the shelves' turquoise and the shallows' clear
// green over sand, the currents darker and lighter across it.
vec3 water_body(float depth, vec2 d, float fp) {
	if (int(zoom_style.y + 0.5) == 0) {
		vec3 shelf_band = vec3(0.333, 0.525, 0.545);
		vec3 deep_band = vec3(0.235, 0.408, 0.475);
		return mix(shelf_band, deep_band, depth < 40.0 ? 0.0 : (depth < 250.0 ? 0.45 : 1.0));
	}
	vec3 deep = vec3(0.012, 0.04, 0.17);
	vec3 open_sea = vec3(0.02, 0.10, 0.33);
	vec3 shelf = vec3(0.04, 0.30, 0.52);
	vec3 shallow = vec3(0.16, 0.58, 0.62);
	vec3 col = mix(open_sea, deep, smoothstep(700.0, 3500.0, depth));
	col = mix(col, shelf, smoothstep(260.0, 40.0, depth));
	col = mix(col, shallow, smoothstep(30.0, 3.0, depth));
#ifndef LIGHT
	if (depth > 15.0) {
		vec2 flow = currents(d, fp);
		float open = smoothstep(15.0, 120.0, depth);
		col *= mix(1.0, mix(0.7, 1.3, flow.x), open);
		// streaks only where the flow runs fast, in places
		vec3 at = octave_at(d + 21000.0, 3);
		float fast = smoothstep(0.55, 0.8, value_noise(at.xy, at.z));
		col += flow.y * fast * vec3(0.08, 0.17, 0.22) * open;
	}
#endif
	return col;
}

// The water's surface seen along `view` (toward the eye): how much of the sky it reflects, x, and the sun's glint, y.
// Its waves are rough from far off, so the glint spreads, and fine and moving close up.
vec2 water_surface(vec3 view, vec3 up_dir, float fp, vec2 d) {
	float t = zoom_clock.x;
	vec2 wave = vec2(0.0);
	float rough = clamp(fp / 40.0, 0.08, 1.0);
	if (fp < 400.0) {
		wave.x = ground_noise(d + vec2(t * 1.3, t * 0.7), 6.0 + fp * 2.0) - 0.5;
		wave.y = ground_noise(d + vec2(-t * 0.9, t * 1.1) + 17.0, 6.0 + fp * 2.0) - 0.5;
	}
	vec3 east = normalize(cross(up_dir, vec3(0.0, 0.0, 1.0)) + vec3(1e-4, 0.0, 0.0));
	vec3 wn = normalize(up_dir + (east * wave.x + cross(east, up_dir) * wave.y) * 0.35);
	float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(view, wn), 0.0), 5.0);
	vec3 half_way = normalize(view + normalize(zoom_sun));
	float shine = pow(max(dot(wn, half_way), 0.0), mix(700.0, 60.0, rough)) * mix(2.5, 0.12, rough);
	return vec2(fresnel, shine);
}

// Water `depth` metres deep, of its own colour `body`, its surface as water_surface gives it, under the sun (`sun_c`,
// `direct`) and the sky (`sky_c`, `ambient`); the sun's glint where the sun reaches it (`sun_seen`, 0 or 1).
vec3 water_lit(vec3 body, vec2 surface, float depth, float fp, vec2 d, vec3 sun_c, vec3 sky_c, float direct, float ambient,
		float sun_seen) {
	int style = int(zoom_style.y + 0.5);
	if (style == 0) {
		return body * (sun_c * direct + sky_c * ambient);
	}
	vec3 colour = body * (sun_c * (0.35 + 0.65 * direct) + sky_c * ambient) + sky_c * surface.x * 0.5
			+ sun_c * surface.y * sun_seen;
	if (style == 2 && fp < 6.0 && depth < 4.0) {
		// waves breaking near the shore, moving in
		float surf = sin(depth * 2.6 - zoom_clock.x * 1.7 + 6.0 * ground_noise(d, 9.0));
		colour = mix(colour, vec3(0.92, 0.95, 0.95), smoothstep(0.82, 0.95, surf) * (1.0 - depth / 4.0));
	}
	return colour;
}

// The land of colour `base` under the sun's light `direct` and the sky's `ambient`: in four steps of light meeting in
// clean edges (variants A and C, the sharp look chosen in P1, A4.1), the shade blue; or smoothly (B).
vec3 land_lit(vec3 base, float direct, float ambient, vec3 sun_c, vec3 sky_c, int land_style) {
	if (land_style == 0 || land_style == 2) {
		float level = clamp(direct * 1.1 + ambient * 0.3, 0.0, 1.3);
		float step_level = floor(level * 4.0 + 0.5) / 4.0;
		return base * mix(sky_c * 1.3 + 0.12, sun_c * 1.15, clamp(step_level, 0.0, 1.0)) * (0.5 + 0.5 * step_level);
	}
	return base * (sun_c * direct * 1.25 + sky_c * ambient * 0.55);
}

// A colour made more vivid: its saturation times `k`.
vec3 vivid(vec3 c, float k) {
	float l = dot(c, vec3(0.3, 0.59, 0.11));
	return clamp(mix(vec3(l), c, k), 0.0, 1.0);
}

void fragment() {
	vec2 d = v_d;
	vec2 w = zoom_focus + d;
	vec2 uv = w / zoom_world;
	float fp = max(length(fwidth(d)), 0.005);
	vec3 n_mesh = normalize(v_normal);
	vec3 up_dir = normalize(v_up_dir);
	vec3 view = normalize((INV_VIEW_MATRIX * vec4(VIEW, 0.0)).xyz);
	vec3 sun = normalize(zoom_sun);
	int land_style = int(zoom_style.x + 0.5);
	mat3 frame = mat3(cross(normalize(vec3(0.0, sin(focus_latitude()), -cos(focus_latitude()))), up_dir), up_dir, vec3(0.0));
	frame[0] = length(frame[0]) > 1e-4 ? normalize(frame[0]) : vec3(1.0, 0.0, 0.0);
	frame[2] = cross(up_dir, frame[0]);

	// the light: the sun, warm, reddening as it sinks, dark behind the planet; the sky's, blue; the clouds' shadows
	float sun_up = dot(up_dir, sun);
	float day = smoothstep(-0.1, 0.06, sun_up);
	vec3 sun_c = mix(vec3(1.0, 0.45, 0.2), vec3(1.0, 0.9, 0.74), smoothstep(0.0, 0.35, sun_up)) * day;
	vec3 sky_c = mix(vec3(0.02, 0.025, 0.06), vec3(0.28, 0.40, 0.72), day);
#ifdef LIGHT
	float shade = 1.0;
#else
	float shade = cloud_shadow(w, max(v_up, 0.0), frame, fp);
#endif
	float sun_seen = step(0.0, sun_up);
	float water_direct = max(sun_up, 0.0) * shade;

	// the cells' height, smooth, coastlines a little worn so they are not the cells' straight edges
	vec2 shore = shore_uv(d);
	float h = textureLod(zoom_height, shore, log2(max(fp / 977.0, 1.0))).r;
	// the cells' cover and climate read a little off where the pixel is, by noise, so their edges wander as the land's
	// do rather than follow the cells
	vec2 uv_cell = cell_uv(d);
	vec4 clim = texture(zoom_climate, uv_cell);
	float lat = PI * (uv.y - 0.5);
	float warmth = season_warmth(lat);
	float season_temp = mix(clim.y, clim.z, warmth);

	// the poles' ice, round each pole whatever the cells hold there, its edge wandering by noise read round the pole,
	// so nothing of the cells squeezed into it shows; and sea frozen this season, and land frozen all year (WLD-26)
	float lon = TAU * uv.x;
	vec2 round_pole = vec2(cos(lon), sin(lon)) * (0.5 * PI - abs(lat)) * planet_radius() + sign(lat) * 3.0e6;
	float ragged = value_noise(round_pole / 90000.0, 1.0e7) * 0.6 + value_noise(round_pole / 30000.0, 1.0e7) * 0.3
			+ value_noise(round_pole / 10000.0, 1.0e7) * 0.1;
	float ragged_fine = value_noise(round_pole / 4000.0, 1.0e7);
	float cap = radians(73.0) + radians(3.0) * (warmth - 0.5) + radians(12.0) * (ragged - 0.5)
			+ radians(1.5) * (ragged_fine - 0.5);
	float ice = smoothstep(cap, cap + radians(0.3), abs(lat));
	// the ice's own texture, floes and ridges, read round the pole too
	vec4 floes = worley(round_pole / 9000.0, 1.0e7);
	float ice_tone = mix(0.86, 1.04, floes.w) - 0.12 * smoothstep(0.35, 0.05, floes.x)
			+ 0.06 * (value_noise(round_pole / 2500.0, 1.0e7) - 0.5);
	if (h < 0.0) {
		ice = max(ice, smoothstep(-1.0, -3.0, season_temp + 2.0 * (ragged - 0.5)));
	} else {
		ice = max(ice, smoothstep(0.5, -1.5, clim.z));
	}

	// the surface's slope: the land's relief from its heights, deepened from far off; and the clumps of its cover
	float exaggerate = mix(1.0, 5.0, smoothstep(30.0, 1500.0, fp)) * smoothstep(40.0, 200.0, fp);
	vec2 slope = h >= 0.0 ? relief(w, fp) * exaggerate : vec2(0.0);
	vec4 cover = texture(zoom_cover, uv_cell);
	float trees = cover.a;
	float dry = dryness(clim.a, trees);
#ifdef LIGHT
	vec4 clump = vec4(0.0, 0.0, 0.5, 0.5);
#else
	vec4 clump = clumps(d, fp, 1.5);
#endif
	// the forests' stands, where the trees' own rule puts them (forest.gdshaderinc); their crowns drawn by the ground
	// where the trees are not drawn as shapes, fading in on the coarsest chunks that draw them as their trees fade out;
	// under the trees' shapes, the forest's floor
	float forest = h >= 0.0 ? forest_stand(d, trees, fp) : 0.0;
	float canopy = trees_drawn == 0 ? 1.0 : (trees_drawn == 1 ? v_morph : 0.0);
	float bumpy = 0.0;
	if (h >= 0.0 && land_style != 0) {
		// open land's clumps only from a few metres up: closer, grass is drawn by its own tufts, not domes
		float open_land = mix(0.22, 0.12, dry) * smoothstep(0.25, 2.5, fp);
		bumpy = mix(open_land, 1.0, forest * canopy) * (1.0 - ice);
		slope += clump.xy * bumpy;
	}
	// the mesh's own slope deepened too, as the zoom leaves the ground, so hills show from above as on a map
	float deepen = mix(1.0, 3.0, smoothstep(5.0, 300.0, fp));
	vec3 n_flat = n_mesh - up_dir * dot(n_mesh, up_dir);
	vec3 n = normalize(up_dir * dot(n_mesh, up_dir) + n_flat * deepen - frame[0] * slope.x - frame[2] * slope.y);
	float direct = max(dot(n, sun), 0.0) * shade;
	float ambient = (0.55 + 0.45 * dot(n, up_dir)) * mix(1.0, mix(0.55, 1.0, clump.z), bumpy)
			* mix(1.0, 0.7, forest * (1.0 - canopy));

	vec3 colour_sun;
	vec3 colour_shadow;
#ifdef SUN_SHADOW
	v_near = 0.0;
	v_row = 0.0;
	v_bias = 0.0;
	v_sky = 1.0;
	v_cloud = shade;
	v_fire = 0.0;
#endif
	if (h < 0.0) {
		vec3 body = water_body(-h, d, fp);
		vec2 surface = water_surface(view, up_dir, fp, d);
		colour_sun = water_lit(body, surface, -h, fp, d, sun_c, sky_c, water_direct, 0.55, sun_seen);
		colour_shadow = water_lit(body, surface, -h, fp, d, sun_c, sky_c, 0.0, 0.55, 0.0);
		if (ice > 0.0) {
			vec3 floe = vec3(0.88, 0.93, 0.98) * ice_tone;
			colour_sun = mix(colour_sun, floe * (sun_c * (0.45 + 0.7 * water_direct) + sky_c * 0.8), ice);
			colour_shadow = mix(colour_shadow, floe * (sun_c * 0.45 + sky_c * 0.8), ice);
		}
	} else {
		vec3 base = cover.rgb;
		if (land_style != 0) {
			// the cover's colour, vivid; where trees stand, a forest's green, the open land's own between them
			vec3 open_land = vivid(mix(cover.rgb, vec3(0.62, 0.66, 0.30), trees * 0.6), 1.5);
			vec3 woods = vivid(cover.rgb * vec3(0.82, 1.0, 0.86), 1.7) * mix(0.8, 1.0, trees);
			// each clump its own shade, the crowns' tops lighter and yellower, stands lighter and darker
			woods *= mix(0.8, 1.15, clump.w);
			woods = mix(woods, woods * vec3(1.2, 1.24, 0.76), clump.z * 0.5);
			vec3 floor_colour = open_land * vec3(0.66, 0.72, 0.62);
#ifdef SUN_SHADOW
			// as the zoom comes down to the camp, these colours give way to the art book's own, the middle steps of the
			// meadow's or the dry grass's ramp, the leaves' and the woods' floor's, so its look close up arrives with
			// no change of colour
			float art = smoothstep(16.0, 4.0, fp);
			if (art > 0.0) {
				float dry_row = smoothstep(0.3, 0.55, dry);
				vec3 meadow_art = mix(mix(pal(zoom_rows_a.x, 4.0), pal(zoom_rows_a.x, 5.0), 0.45),
						mix(pal(zoom_rows_a.y, 4.0), pal(zoom_rows_a.y, 5.0), 0.45), dry_row);
				open_land = mix(open_land, to_linear(meadow_art), art);
				vec3 leaf_art = mix(pal(zoom_rows_c.y, 3.0), pal(zoom_rows_c.y, 4.0), 0.6);
				woods = mix(woods, to_linear(leaf_art) * mix(0.8, 1.15, clump.w), art);
				floor_colour = mix(floor_colour, to_linear(pal(zoom_rows_a.z, 3.0)), art);
			}
#endif
			base = mix(open_land, mix(floor_colour, woods, canopy), forest);
			base *= 1.0 + 0.4 * ground_detail(d, 3000.0, 2.0 * fp, 9);
			// the deserts' sand and stone
			base = mix(base, base * vec3(0.78, 0.62, 0.5), dry * smoothstep(0.55, 0.75, clump.z + 0.3 * ground_detail(d, 6000.0, fp, 5)));
		} else {
			base *= 1.0 + 0.3 * ground_detail(d, 8000.0, max(fp, 300.0), 3);
		}
		// rock where it is steep, beaches by the sea
		float steep = max(v_slope, length(slope) * 0.5 * (1.0 - bumpy));
		float rock = smoothstep(0.25, 0.45, steep + 0.08 * ground_detail(d, 400.0, fp, 4));
		base = mix(base, vec3(0.50, 0.44, 0.42), rock);
		float beach = beach_of(h, rock);
		base = mix(base, vec3(0.90, 0.80, 0.56), beach * 0.85);
		// snow where this season is cold enough at this height, and ice
		float snow = snow_of(season_temp, max(v_up - max(h, 0.0), 0.0), d, fp);
		base = mix(base, vec3(0.93, 0.95, 0.98), max(snow, ice));
		colour_sun = land_lit(base, direct, ambient, sun_c, sky_c, land_style);
		colour_shadow = land_lit(base, 0.0, ambient, sun_c, sky_c, land_style);
		// lakes, their shores worn as the coasts' are
		float lake = textureLod(zoom_water_soft, shore, 0.0).b * 255.0;
		if (lake > 1.0) {
			vec3 body = water_body(lake, d, fp);
			vec2 surface = water_surface(view, up_dir, fp, d);
			colour_sun = water_lit(body, surface, lake, fp, d, sun_c, sky_c, water_direct, 0.55, sun_seen);
			colour_shadow = water_lit(body, surface, lake, fp, d, sun_c, sky_c, 0.0, 0.55, 0.0);
		}
		// rivers over land and lakes alike
		vec3 river = river_near(d, fp);
		if (river.x > 0.0) {
			// as deep as the land it drains is wide, so larger rivers show bluer
			float depth = 4.0 + 2.5 * log2(max(river.y, 1.0));
			vec3 body = water_body(depth, d, fp);
			vec2 surface = water_surface(view, up_dir, fp, d);
			vec3 flowing_sun = water_lit(body, surface, depth, fp, d, sun_c, sky_c, water_direct, 0.55, sun_seen);
			vec3 flowing_shadow = water_lit(body, surface, depth, fp, d, sun_c, sky_c, 0.0, 0.55, 0.0);
			colour_sun = mix(colour_sun, flowing_sun, river.x);
			colour_shadow = mix(colour_shadow, flowing_shadow, river.x);
		}
#ifdef SUN_SHADOW
		// close up, the art book's ground (A4.2): each pixel one layer of the look's palette (near.gdshaderinc), in
		// patches a step darker, meeting in clean edges, lit in the look's steps as what stands on it is
		v_near = smoothstep(4.0, 1.6, fp);
		if (v_near > 0.0) {
			vec4 layer = near_layer(d, fp, dry, forest * (1.0 - canopy), river, lake, beach, rock, max(snow, ice));
			v_row = layer.x;
			v_bias = layer.y;
			float open = layer.z;
			vec3 shading_n = layer.w == LAYER_WATER ? up_dir : n;
			v_sky = sky_light(vec3(0.0, dot(shading_n, up_dir), 0.0), open);
			// the camp's fire on the ground round it (look_core.gdshaderinc)
			vec3 at = (INV_VIEW_MATRIX * vec4(VERTEX, 1.0)).xyz;
			for (int i = 0; i < 4; i++) {
				if (look_fire_powers[i] > 0.0) {
					v_fire += fire_reach(i, at, shading_n, 0.35, ground_noise(d + float(i) * 70.0, 1.2));
				}
			}
			NORMAL = (VIEW_MATRIX * vec4(shading_n, 0.0)).xyz;
		}
#endif
	}
#ifdef SUN_SHADOW
	v_sunlit = colour_sun;
	v_shaded = colour_shadow;
	ALBEDO = vec3(1.0);
#else
	ALBEDO = colour_sun;
#endif
}

#ifdef SUN_SHADOW
// The sun's shadow map: a tree or a thing between the pixel and the sun takes the sun's own light away; close up the
// look's light gives the art book's step, the clouds' shadows as hard as a tree's.
void light() {
	if (LIGHT_IS_DIRECTIONAL) {
		vec3 far = mix(v_shaded, v_sunlit, ATTENUATION);
		vec3 near = far;
		if (v_near > 0.0) {
			float sun = ATTENUATION * step(0.65, v_cloud);
			near = to_linear(shade_pixel(dot(NORMAL, LIGHT), sun, 0.0, v_row, v_bias, 0.0, v_sky, v_fire, 0.0));
		}
		DIFFUSE_LIGHT = mix(far, near, v_near);
	}
}
#endif
