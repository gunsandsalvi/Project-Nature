// The ground's vertices (T01b.4, A11.5): the mockup's terrainVS. Positions are metres from the area's corner, moved
// by the corner's offset from the floating origin (A11.2); the bytes after the normal are the surface, water
// distance, wear and cover (aB) and the flags.
in vec3 aPos;
in vec3 aNrm;
in vec4 aB;
in float aFlags;
uniform mat4 uVP;
uniform vec3 uAreaOff;
out vec3 vWorld;
out vec3 vNrm;
out vec4 vB;
out float vFlags;
void main() {
  vec3 p = aPos + uAreaOff;
  vWorld = p;
  vNrm = aNrm;
  vB = aB;
  vFlags = aFlags;
  gl_Position = uVP * vec4(p, 1.0);
}
