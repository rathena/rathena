#version 300 es
precision highp float;

in vec2 vTextureCoord;
in float vLightWeighting;
in float vAlpha;
in vec3 vWorldPos;
out vec4 fragColor;

uniform sampler2D uDiffuse;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

uniform vec3  uLightAmbient;
uniform vec3  uLightDiffuse;
uniform float uLightOpacity;
uniform bool uLightMapUse;
uniform vec3 uLightEnv;

// #include OccluderFade.glsl

void main(void) {
	vec4 textureSample  = texture( uDiffuse,  vTextureCoord.st );

	if (textureSample.a == 0.0) {
		discard;
	}

	if (!occluderFade(vWorldPos, textureSample.a)) {
		discard;
	}

	vec3 color = ((uLightMapUse ? vLightWeighting : 1.0) * uLightDiffuse + uLightAmbient);
	textureSample.rgb *= clamp(color, 0.0, 1.0);
	textureSample.rgb *= clamp(uLightEnv, 0.0, 1.0);
	textureSample.a *= vAlpha;

	fragColor = textureSample;
	
	if (uFogUse) {
		float depth     = gl_FragCoord.z / gl_FragCoord.w;
		float fogFactor = smoothstep( uFogNear, uFogFar, depth );
		fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
	}

}