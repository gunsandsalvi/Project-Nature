// P8's sky in the zoom's light mode: the air alone, no clouds, no weather and no 3D texture. See sky.gdshaderinc.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://look/shaders/look_core.gdshaderinc"
#include "res://zoom/shaders/sky.gdshaderinc"
