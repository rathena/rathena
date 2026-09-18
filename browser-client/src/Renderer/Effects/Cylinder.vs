#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uRotationMat;
uniform bool uRotate;
uniform vec3 uPosition;
uniform float uTopSize;
uniform float uBottomSize;
uniform float uHeight;

uniform float uZindex;

void main(void) {
    float size, height;

    if (aPosition.z == 1.0) {
        size   = uTopSize;
        height = uHeight;
    } else {
        size   = uBottomSize;
        height = 0.0;
    }

    vec4 position  = vec4(uPosition.x + 0.5, -uPosition.z, uPosition.y + 0.5, 1.0);
    if(uRotate) {
        position += vec4(aPosition.x * size, -height, aPosition.y * size, 0.0) * uRotationMat;
    } else {
        position  += vec4(aPosition.x * size, -height, aPosition.y * size, 0.0);
    }

    gl_Position    = uProjectionMat * uModelViewMat * position;
    gl_Position.z -= uZindex;

    vTextureCoord  = aTextureCoord;
}