#version 300 es
precision mediump float;
in vec3 vNrm;
uniform vec3 uBase;
uniform vec3 uLight;
out vec4 color;
void main() {
    color = vec4(uBase * (0.25 + 0.75 * max(dot(normalize(vNrm), uLight), 0.0)), 1.0);
}
