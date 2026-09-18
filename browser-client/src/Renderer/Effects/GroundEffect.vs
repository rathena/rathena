#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;
out vec2 vTextureCoord;
uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uRotationMat;
void main(void) {
    gl_Position    = uProjectionMat * uModelViewMat * vec4( aPosition.x, aPosition.y - 0.5, aPosition.z, 1.0);
    gl_Position.z -= 0.01;
    vTextureCoord  = (uRotationMat * vec4( aTextureCoord - 0.5, 1.0, 1.0)).xy + 0.5;
}