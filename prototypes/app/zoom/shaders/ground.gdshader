// P8's ground, from the globe down to a person (A8.1, A8.5): every chunk of every level drawn by this one shader, so
// nothing changes but the detail as the zoom moves. The chunk morphs into the grid half as fine toward its far edge
// (CDLOD), then sits on the sphere. Each pixel's colour comes from the world's cells, read a little off by noise so no
// cell's edge shows, with detail at every scale the pixel can show, so the land always has texture a few pixels across:
// forests as clumps of crowns and stands, each lit on its sun side; grass and scrub more gently so; rock where it is
// steep, sand and stone in the deserts, snow by season and height; the land's relief shaded from its heights, deepened
// from far off as maps do. The sea and lakes by their depth, from deep navy to turquoise shallows, with currents and
// eddies drifting across them; the poles' ice; rivers as smooth curves through the cells, as wide as the land they
// drain, never thinner than about a pixel once they show, so they never vanish as the zoom closes in. Lit by the sun,
// warm, and the sky, blue, so shade leans blue as the art book's does, under the clouds' shadows. Variants: the land A
// the art book's colours in banded light, B vivid and lit, C vivid in banded light; the water A the art book's bands,
// B deep and shallow with currents and the sun's glint, C B with waves near the shore.
shader_type spatial;
render_mode unshaded, world_vertex_coords, cull_disabled, shadows_disabled, fog_disabled;

#define CLOUD_NOISE
#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/clouds.gdshaderinc"
#include "res://zoom/shaders/cloud_noise.gdshaderinc"
#include "res://zoom/shaders/ground.gdshaderinc"
