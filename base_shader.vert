#version 120

attribute vec4 vertexPosition;
attribute vec3 vertexNormal;

uniform mat4 mvpMatrix;  // Model-View-Projection
uniform mat4 modelMatrix;
uniform vec3 basePosition;

varying vec3 fragNormal;        // normal for fragment shader
varying vec3 fragPosition;      // fragment position in world coords

void main() {
    vec4 finalPosition = vec4(vertexPosition.xyz - basePosition, 1.0);

    fragPosition = vec4(modelMatrix * finalPosition).xyz;
    fragNormal = normalize(mat3(modelMatrix) * vertexNormal);

    gl_Position = mvpMatrix * finalPosition;
}
