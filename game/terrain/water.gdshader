// Implements PRE-26 PRE-30 (T2.8a.3): prepared bed only, shared heights, bounded reflection and stable flow.
shader_type canvas_item;
render_mode unshaded;
#include "res://terrain/light.gdshaderinc"
uniform sampler2D bed_background : filter_nearest, repeat_disable;
uniform sampler2D reflection_layer : filter_nearest, repeat_disable;
uniform vec2 viewport_size;
uniform vec3 corner0;
uniform vec3 corner1;
uniform vec3 corner2;
uniform vec3 corner3;
uniform vec4 bed_heights;
uniform sampler2D visibility_mask : filter_linear, repeat_disable;
uniform float fixture_second = 0.0;
varying vec3 world_point;
void vertex() { world_point = mix(mix(corner0,corner1,UV.x),mix(corner3,corner2,UV.x),UV.y); }
void fragment() {
    float bed = mix(mix(bed_heights.x,bed_heights.y,UV.x),mix(bed_heights.w,bed_heights.z,UV.x),UV.y);
    float depth = max(0.0,world_point.z-bed);
    vec2 flow = vec2(sin(world_point.y*3.0+fixture_second*1.3),cos(world_point.x*2.0+fixture_second*0.8));
    vec2 uv = SCREEN_UV;
    vec3 bottom = texture(bed_background,uv+flow/viewport_size*0.7).rgb;
    vec4 reflection = texture(reflection_layer,uv);
    vec3 tint = mix(vec3(0.30,0.47,0.43),vec3(0.08,0.19,0.24),clamp(depth/2.0,0.0,1.0));
    vec3 water_colour = fixture_light(tint,vec3(0.0,0.0,1.0),world_point,texture(visibility_mask,UV),0.0,8);
    vec3 colour = mix(bottom,water_colour,clamp(depth*0.45,0.12,0.65));
    colour = mix(colour,reflection.rgb,reflection.a*0.18);
    colour += sky_light*0.09;
    float shore = 1.0-smoothstep(0.0,0.025,min(min(UV.x,1.0-UV.x),min(UV.y,1.0-UV.y)));
    float current = smoothstep(0.97,1.0,sin(world_point.y*6.0+world_point.x*1.1+fixture_second*0.9));
    colour += vec3(0.13,0.15,0.13)*(shore+current*0.22);
    COLOR = vec4(colour,1.0);
}
