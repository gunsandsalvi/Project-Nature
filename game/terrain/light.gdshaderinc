// Implements PRE-20 PRE-21 PRE-30 (T2.8a.1/3): shared world east/north/up light, sun-only visibility.
// Compatibility SDR canvas samples and writes encoded RGB; light arithmetic is linear.
uniform bool sdr_canvas = true;
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

vec3 fixture_decode(vec3 colour) {
    if (!sdr_canvas) { return colour; }
    return mix(pow((max(colour,vec3(0.0))+0.055)/1.055,vec3(2.4)),colour/12.92,lessThanEqual(colour,vec3(0.04045)));
}
vec3 fixture_encode(vec3 colour) {
    if (!sdr_canvas) { return colour; }
    return mix(1.055*pow(max(colour,vec3(0.0)),vec3(1.0/2.4))-0.055,12.92*colour,lessThanEqual(colour,vec3(0.0031308)));
}
vec3 fixture_light_linear(vec3 albedo, vec3 world_normal, vec3 point, vec4 visibility, float emission, int material_index) {
    if (debug_albedo) { return fixture_decode(albedo); }
    if (debug_normals) { return fixture_decode(world_normal * 0.5 + 0.5); }
    if (debug_receivers) { return fixture_decode(receiver_colour); }
    albedo = fixture_decode(albedo);
    vec4 tuning = material_table[clamp(material_index,0,9)];
    float response = max(dot(normalize(world_normal), normalize(sun_direction)), 0.0);
    vec3 colour = albedo * (sky_light * visibility.g * visibility.b + sunlight * response * tuning.x * visibility.r);
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

vec3 fixture_light(vec3 albedo, vec3 world_normal, vec3 point, vec4 visibility, float emission, int material_index) {
    return fixture_encode(fixture_light_linear(albedo,world_normal,point,visibility,emission,material_index));
}
