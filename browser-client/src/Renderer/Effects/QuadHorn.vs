#version 300 es
#pragma vscode_glsllint_stage : vert
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;
out vec2 vTextureCoord;

uniform vec3 uPosition;
uniform float uHeight;
uniform float uBottomSize;
uniform float uOffsetX;
uniform float uOffsetY;
uniform float uOffsetZ;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uXRotationMat;
uniform mat4 uYRotationMat;
uniform mat4 uZRotationMat;
void main(void) {
    vec4 position       = vec4(uPosition.x + uOffsetX, -uPosition.z - ((uHeight * 0.9) + uOffsetZ), uPosition.y + uOffsetY, 1.0);
    position            += vec4(aPosition.x * uBottomSize, aPosition.y * uHeight, aPosition.z * uBottomSize, 0.0) * uXRotationMat * uYRotationMat * uZRotationMat;
    gl_Position         = uProjectionMat * uModelViewMat * position;

    vTextureCoord = aTextureCoord;
}