#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform float uWaveHeight;
uniform float uWavePitch;
uniform float uWaterOffset;

const float PI = 3.14159265358979323846264;

void main(void) {
    float x       = mod( aPosition.x, 2.0);
    float y       = mod( aPosition.z, 2.0);
    float diff    = x < 1.0 ? y < 1.0 ? 1.0 : -1.0 : 0.0;
    float Height  = sin((PI / 180.0) * (uWaterOffset + 0.5 * uWavePitch * (aPosition.x + aPosition.z + diff))) * uWaveHeight;

    gl_Position   = uProjectionMat * uModelViewMat * vec4( aPosition.x, aPosition.y + Height, aPosition.z, 1.0);
    vTextureCoord = aTextureCoord;
}