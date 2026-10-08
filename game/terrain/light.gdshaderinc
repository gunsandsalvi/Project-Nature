// Implements PRE-20 PRE-21 PRE-30 (T2.8a.1/3): shared world east/north/up light, sun-only visibility.
uniform vec3 sun_direction = vec3(0.5, -0.4, 0.75);
uniform vec3 sunlight = vec3(0.78, 0.75, 0.66);
uniform vec3 sky_light = vec3(0.3, 0.35, 0.43);
uniform vec3 fire_position = vec3(3.0, 2.0, 0.6);
uniform bool fire_on = true;
uniform bool debug_albedo = false;
uniform bool debug_normals = false;
uniform bool debug_receivers = false;
uniform vec3 receiver_colour = vec3(0.5);
uniform vec4 material_table[10];

vec3 fixture_light(vec3 albedo, vec3 world_normal, vec3 point, vec4 visibility, float emission, int material_index) {
    if (debug_albedo) { return albedo; }
    if (debug_normals) { return world_normal * 0.5 + 0.5; }
    if (debug_receivers) { return receiver_colour; }
    vec4 tuning = material_table[clamp(material_index,0,9)];
    float response = max(dot(normalize(world_normal), normalize(sun_direction)), 0.0);
    vec3 colour = albedo * (sky_light * visibility.g + sunlight * response * tuning.x * visibility.r) * visibility.b;
    vec3 delta = point - fire_position;
    float distance_squared = dot(delta, delta);
    // Half at 2 metres, about a tenth at 3.5 metres. Local light bypasses sun visibility.
    float pool = 1.0 / (1.0 + distance_squared * distance_squared / 16.0);
    float facing = max(0.2, dot(normalize(world_normal), normalize(-delta + vec3(0.0, 0.0, 0.001))));
    colour += albedo * vec3(1.45, 0.58, 0.13) * pool * facing * visibility.a * float(fire_on);
    colour += sunlight * visibility.r * tuning.w * (1.0-tuning.y) * pow(response,12.0) * 0.06;
    colour += albedo * (emission + tuning.z);
    return colour / (1.0 + max(colour - 1.0, vec3(0.0)) * 0.25);
}
