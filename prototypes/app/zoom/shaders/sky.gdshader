// P8's sky (A8.4, A8.6), a pass over the whole picture after the ground: along each pixel's ray, the clouds and the
// air in front of the ground or, past the planet's edge, of space and its stars. The air scatters sunlight as Earth's
// does, by Rayleigh's and Mie's laws (single scattering, after Nishita 1993 and Hillaire 2020): blue overhead, a glow
// along the planet's rim, haze over distant ground, the sun reddening as it sinks. The clouds, from the climate's
// weather (weather.gdshader, planet.gdshaderinc): A raymarched through their 3D shapes and lit by the sun through
// themselves (Schneider 2015), so the camera passes through them as it descends; B the same, hard-edged and lit in the
// art book's bands; C the art book's flat clouds, one deck.
// Light: A the art book's, the air only a thin glow at the rim; B the air's full scattering.
shader_type spatial;
render_mode unshaded, depth_draw_never, depth_test_disabled, cull_disabled, shadows_disabled, fog_disabled;

#define FLAT
#include "res://zoom/shaders/planet.gdshaderinc"
#include "res://zoom/shaders/clouds.gdshaderinc"
#include "res://zoom/shaders/sky.gdshaderinc"
