#version 300 es
#pragma vscode_glsllint_stage : vert
precision highp float;

in vec2 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uTextureRotMat;
uniform mat4 uRotationMat;

uniform float uCameraZoom;

uniform vec3 uPosition;
uniform float uSize;
uniform float uZIndex;

void main(void) {

    vec4 position  = vec4(uPosition.x + 0.5, -uPosition.z - 2.0, uPosition.y + 0.5, 1.0);
    
    vec4 pos2      = vec4(aPosition.x * uSize, 0.0, aPosition.y * uSize, 0.0) * uTextureRotMat;
    pos2.x        += 1.0;
    position      += pos2 * uRotationMat;

    gl_Position    = uProjectionMat * uModelViewMat * position;
    gl_Position.z -= uZIndex / max(uCameraZoom, 1.0);

    vTextureCoord  = aTextureCoord;
}