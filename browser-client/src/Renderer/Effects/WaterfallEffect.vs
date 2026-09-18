#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;
uniform mat4 uModelMat;
out vec2 vTextureCoord;

void main(void) {
	vec4 worldPosition = uModelMat * vec4(aPosition.x, -aPosition.y, aPosition.z, 1.0);
	gl_Position = uProjectionMat * uModelViewMat * worldPosition;
	vTextureCoord = aTextureCoord;
}
