#version 300 es
// The lit cube of α00 (A11.1 grows from here). Shared by the phone and the web unchanged (A2.6).
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
uniform mat4 uMVP;
uniform mat3 uNrm;
out vec3 vNrm;
void main() {
    vNrm = uNrm * aNrm;
    gl_Position = uMVP * vec4(aPos, 1.0);
}
