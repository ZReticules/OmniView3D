#version 110

attribute vec4 vertexPosition;
attribute vec3 vertexColor;
uniform mat4 mvpMatrix;
varying vec3 fragColor;

void main() {
   gl_Position = mvpMatrix * vertexPosition;
   fragColor = vertexColor;
}
