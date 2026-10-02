// The golden cube (T01a.6, A11.12): world position and normal for the scene shader; the camera looks down -z.
in vec3 aPos;
in vec3 aNrm;
uniform mat4 uMVP;
uniform mat4 uModel;
out vec3 vWorld;
out vec3 vNrm;
void main() {
  vWorld = (uModel * vec4(aPos, 1.0)).xyz;
  vNrm = mat3(uModel) * aNrm;
  gl_Position = uMVP * vec4(aPos, 1.0);
}
