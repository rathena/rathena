#version 300 es
precision highp float;

out vec4 vColor;

in vec2 aPosition;

uniform vec4 uColor;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform vec3 uPosition;
uniform float uSize;

void main(void) {
    vec4 position  = vec4(uPosition.x + 0.5, -uPosition.z, uPosition.y + 0.5, 1.0);
    position      += vec4(aPosition.x * uSize, 0.0, aPosition.y * uSize, 0.0);

    gl_Position    = uProjectionMat * uModelViewMat * position;
    gl_Position.z -= 0.01;
    vColor = uColor;
}