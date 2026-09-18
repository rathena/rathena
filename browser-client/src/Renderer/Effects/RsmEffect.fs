#version 300 es
precision highp float;

in vec2 vTextureCoord;
in float vLightWeighting;
in float vAlpha;
out vec4 fragColor;

uniform sampler2D uDiffuse;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

uniform vec3  uLightAmbient;
uniform vec3  uLightDiffuse;
uniform float uLightOpacity;

void main(void) {
	vec4 textureSample  = texture( uDiffuse,  vTextureCoord.st );

	if (textureSample.a == 0.0) {
		discard;
	}

	vec3 Ambient	= uLightAmbient * uLightOpacity;
	vec3 Diffuse	= uLightDiffuse * vLightWeighting;
	vec4 LightColor = vec4( Ambient + Diffuse, 1.0);

	fragColor	= textureSample * clamp(LightColor, 0.0, 1.0);
	fragColor.a *= vAlpha;

	if (uFogUse) {
		float depth	 = gl_FragCoord.z / gl_FragCoord.w;
		float fogFactor = smoothstep( uFogNear, uFogFar, depth );
		fragColor	= mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
	}
}