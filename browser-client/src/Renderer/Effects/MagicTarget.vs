#version 300 es
#pragma vscode_glsllint_stage : vert
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform float uCameraLatitude;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uRotationMat;

void main(void) {
    gl_Position    = uProjectionMat * uModelViewMat * vec4( aPosition, 1.0);
    gl_Position.z -= 0.02 / uCameraLatitude;

    vTextureCoord  = (uRotationMat * vec4( aTextureCoord - 0.5, 1.0, 1.0)).xy + 0.5;
}