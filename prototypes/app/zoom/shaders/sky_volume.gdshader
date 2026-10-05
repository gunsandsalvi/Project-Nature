// P8's sky with the volumetric clouds (variants A and B, A8.6): its own shader, so it is made only when they are
// chosen. See sky.gdshaderinc.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

#define VOLUME
#define CLOUD_NOISE
#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/clouds.gdshaderinc"
#include "res://zoom/shaders/cloud_noise.gdshaderinc"
#include "res://zoom/shaders/sky.gdshaderinc"
