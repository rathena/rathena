#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform float uZIndex;

void main(void) {
	gl_Position = uProjectionMat * uModelViewMat * vec4(aPosition, 1.0);
	gl_Position.z -= uZIndex;
	vTextureCoord = aTextureCoord;
}