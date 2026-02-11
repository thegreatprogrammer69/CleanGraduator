#version 120

varying vec2 vTex;

void main() {
    vTex = gl_MultiTexCoord0.st;
    gl_Position = gl_Vertex;
}
