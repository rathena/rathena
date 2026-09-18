#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aVertexNormal;
in vec2 aTextureCoord;
in float aAlpha;

out vec2 vTextureCoord;
out float vLightWeighting;
out float vAlpha;
out vec3 vWorldPos;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform vec3 uLightDirection;

void main(void) {
	gl_Position     = uProjectionMat * uModelViewMat * vec4( aPosition, 1.0);

	vTextureCoord   = aTextureCoord;
	vAlpha          = aAlpha;
	vWorldPos       = aPosition;

	float dotProduct = dot(aVertexNormal, uLightDirection );
	vLightWeighting = max(dotProduct, 0.0);
}