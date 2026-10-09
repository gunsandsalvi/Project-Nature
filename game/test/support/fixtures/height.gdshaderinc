// Implements PRE-26 PRE-31: colour and integer coverage share the projected height and water footprint.
vec3 fixture_sprite_point(vec3 foot, vec2 pivot, vec2 vertex, float density, float height_basis) {
    return foot + vec3((vertex.x-pivot.x)/density, 0.0, (pivot.y-vertex.y)/(density*height_basis));
}
bool fixture_inside_water(vec3 point, vec4 bounds) {
    return point.x >= bounds.x && point.x <= bounds.z && point.y >= bounds.y && point.y <= bounds.w;
}
