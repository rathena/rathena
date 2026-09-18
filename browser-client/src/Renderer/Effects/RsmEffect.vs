#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aVertexNormal;
in vec2 aTextureCoord;
in float aAlpha;

out vec2 vTextureCoord;
out float vLightWeighting;
out float vAlpha;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform vec3 uLightDirection;
uniform mat3 uNormalMat;

uniform vec3 uPosition;
uniform float uSize;

void main(void) {

	vec4 position  = vec4(uPosition.x + 0.5, -uPosition.z, uPosition.y + 0.5, 1.0);
	position += vec4(aPosition.x * uSize, aPosition.y * -uSize, aPosition.z * uSize, 0.0);

	gl_Position	 = uProjectionMat * uModelViewMat * position;

	vTextureCoord   = aTextureCoord;
	vAlpha		  = aAlpha;

	vec4 lDirection  = uModelViewMat * vec4( uLightDirection, 0.0);
	vec3 dirVector   = normalize(lDirection.xyz);
	float dotProduct = dot( uNormalMat * aVertexNormal, dirVector );
	vLightWeighting  = max( dotProduct, 0.5 );
}