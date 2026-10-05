// P8's ground in the zoom's light mode: the full ground's colours and light without the clouds' shadows, the forests'
// clumps, the sea's currents or the shadows things cast, so it needs neither the weather's picture nor any 3D texture.
// See ground.gdshaderinc.
shader_type spatial;
render_mode unshaded, world_vertex_coords, cull_disabled, shadows_disabled, fog_disabled;

#define LIGHT
#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/rivers.gdshaderinc"
#include "res://zoom/shaders/forest.gdshaderinc"
#include "res://zoom/shaders/near.gdshaderinc"
#include "res://zoom/shaders/ground.gdshaderinc"
