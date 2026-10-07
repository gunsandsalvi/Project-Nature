// The one shared light function (A4.3, A4.4, CLAUDE.md rule 4), its first version (α2.2a): every material
// hands it its texture pixel's colour and the facts of its surface, and it lights them. The sun is Godot's
// light, with the small casters' soft shadow from Godot's sun map; the sky's fill is Godot's ambient light,
// scaled by openness; this adds light bounced from the ground, fire from the light grid, the big casters'
// shadow, openness and contact from the view's maps, and haze by distance, warmer toward the sun. The colour
// table grades the picture in the final step, in the environment. Its numbers are stand-ins until the light
// is tuned against the targets (α2.3b); its cost is what calibration scene C1 measures. Implements PRE-30,
// PRE-21 and PRE-24.

// The view's maps (A4.6), seen from above round the focus: r openness to the sky; g contact darkening, 1 for
// none; b the steepest angle up to anything between the point and the sun, as a share of a right angle; a
// how far away that is, as a share of kd_maps_place.w metres.
global uniform sampler2D kd_view_maps;
// The heights the maps were made from, a float a texel in metres: the top of what stands at each texel, 0 for bare ground.
// A surface that faces up and lies well below its texel's top is under a roof, and is dark and out of the sun.
global uniform sampler2D kd_view_tops;
// The maps' south-west corner in the rig's metres east and north (xy), their width in metres (z), and the
// farthest caster's distance they hold in metres (w).
global uniform vec4 kd_maps_place;
// The light grid (A4.5): squares of kd_grid_place.z metres from its south-west corner at xy, each texel up to
// four fires, each its row of kd_fire_table plus one, 0 for none, in bytes; a row a fire, its place in the
// rig's metres east, up and north, and its heat from 0 to 1.
global uniform sampler2D kd_fire_grid;
global uniform sampler2D kd_fire_table;
global uniform vec4 kd_grid_place;
#if defined(KD_FIRE_WALK) || defined(KD_FIRE_MAP) || defined(KD_FIRE_SCREEN)
#include "res://look/fire.gdshaderinc"
#endif

// Toward the sun, in the world; light bounced up from the sunlit cover below; the haze's colour (rgb) and
// its density a metre (a); and its colour toward the sun.
global uniform vec3 kd_sun_toward;
global uniform vec3 kd_bounce;
global uniform vec4 kd_haze;
global uniform vec3 kd_haze_sun;

// A small hearth's light halves this far away, in metres, and falls to a tenth at three times it (A4.3).
const float KD_FIRE_HALF = 1.25;

// Only a shader that draws ground defines KD_UNDER_ROOFS: a part of a thing lies below the poles or the crown that
// stand over it, and is not under a roof. How far below the top at its texel an upward-facing surface lies to be
// under a roof, in metres: more than a
// curved stone's own height across a texel, less than any cover over the ground.
const float KD_UNDER_ROOF = 0.15;
// The least a thing's top may be to count as one, in metres, and the openness left under a roof, so a cavity is dark
// and never black (A4.8).
const float KD_LEAST_TOP = 0.03;
const float KD_CAVITY = 0.25;

// The surface's own light, in its fragment(). albedo: its texture pixel's colour in linear light; world: its
// point; normal: its facing, in the world; eye: the camera's place; crease: darkening painted or baked into
// it, 1 for none; fire_lit: how much of each of the first fires its square lists reaches it, as a picture
// at half resolution gives it (KD_FIRE_SCREEN), and 1 for each where none does. Gives Godot's ALBEDO
// (colour), AO (openness, which scales the sky's fill), EMISSION (glow: bounce and fire) and FOG (haze),
// and the big casters' shadow on it for the sun in light(). Fire shadows come from the walk at every pixel
// with KD_FIRE_WALK defined, from a map for each fire with KD_FIRE_MAP, and from fire_lit.
void kd_light_surface(vec3 albedo, vec3 world, vec3 normal, vec3 eye, float crease, vec4 fire_lit,
		out vec3 colour, out float openness, out vec3 glow, out vec4 haze, out float big_shadow) {
	vec2 at = vec2(world.x, -world.z);
	vec2 on_maps = (at - kd_maps_place.xy) / kd_maps_place.z;
	// beyond the square the maps lie in there is nothing: open sky, no contact, the sun clear
	bool on = all(greaterThanEqual(on_maps, vec2(0.0))) && all(lessThan(on_maps, vec2(1.0)));
	vec4 maps = on ? textureLod(kd_view_maps, on_maps, 0.0) : vec4(1.0, 1.0, 0.0, 0.0);
	// under a roof: facing up, below the top that stands at this texel, which only a thing's top does not
	float covered = 0.0;
#ifdef KD_UNDER_ROOFS
	if (on) {
		float top = texelFetch(kd_view_tops, ivec2(on_maps * vec2(textureSize(kd_view_tops, 0))), 0).r;
		covered = (top > KD_LEAST_TOP && normal.y > 0.5 && world.y < top - KD_UNDER_ROOF) ? 1.0 : 0.0;
	}
#endif
	colour = albedo * crease * maps.g;
	openness = mix(maps.r, KD_CAVITY, covered);
	// the big casters' shadow: lit where the sun stands above the steepest angle toward it, faded over a band
	// that widens with the caster's distance, so long shadows go soft and shadows at the foot stay sharp
	float sun_height = asin(clamp(kd_sun_toward.y, -1.0, 1.0)) / (0.5 * PI);
	float band = 0.005 + 0.1 * maps.a;
	big_shadow = smoothstep(-band, band, sun_height - maps.b) * (1.0 - covered);
	// light bounced up from the ground, on faces turned from the sky
	glow = colour * kd_bounce * (0.5 - 0.5 * normal.y) * openness;
	// fire from the light grid: the fires listed in the point's square
	ivec2 square = ivec2(floor((at - kd_grid_place.xy) / kd_grid_place.z));
	ivec2 squares = textureSize(kd_fire_grid, 0);
	if (all(greaterThanEqual(square, ivec2(0))) && all(lessThan(square, squares))) {
		vec4 rows = texelFetch(kd_fire_grid, square, 0) * 255.0;
		for (int i = 0; i < 4; i++) {
			int row = int(rows[i] + 0.5) - 1;
			if (row < 0) {
				break;
			}
			vec4 fire = texelFetch(kd_fire_table, ivec2(row, 0), 0);
			vec3 to = vec3(fire.x, fire.y, -fire.z) - world;
			float near = dot(to, to);
			float heat = fire.w * (0.9 + 0.1 * sin(TIME * 7.0 + float(row) * 1.7));
			vec3 flame = mix(vec3(1.0, 0.42, 0.12), vec3(1.0, 0.72, 0.4), heat);
			float facing = max(dot(normal, to * inversesqrt(max(near, 1e-4))), 0.0);
			// what stands between the point and the fire
			float lit = fire_lit[i];
#if defined(KD_FIRE_WALK)
			lit *= kd_fire_walk(world, kd_flame(fire));
#elif defined(KD_FIRE_MAP)
			lit *= kd_fire_map(world, row, fire);
#endif
			glow += colour * flame * heat * facing * lit / (1.0 + near / (KD_FIRE_HALF * KD_FIRE_HALF));
		}
	}
	// haze by distance, warmer toward the sun
	vec3 look = normalize(world - eye);
	vec3 tint = mix(kd_haze.rgb, kd_haze_sun, pow(max(dot(look, kd_sun_toward), 0.0), 4.0));
	haze = vec4(tint, 1.0 - exp(-kd_haze.a * length(world - eye)));
}

// The sun in a surface's light(), all in Godot's view space: Godot's light with the small casters' soft
// shadow (attenuation) and the big casters' (big_shadow), and light through thin things lit from behind
// (backlit, 0 for solid ground and stone).
vec3 kd_light_sun(vec3 normal, vec3 light, vec3 light_colour, float attenuation, float big_shadow,
		float backlit) {
	float facing = max(dot(normal, light), 0.0) + backlit * max(-dot(normal, light), 0.0);
	return light_colour * attenuation * big_shadow * facing / PI;
}
