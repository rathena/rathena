#version 300 es
precision highp float;

in vec2 vTextureCoord;
in float vLightWeighting;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform float uAlphaRef;      // client HARD alpha-test (207/255), not roBrowser's a == 0.0
uniform float uAlpha;         // per-instance entity alpha (removal fade); 1.0 = opaque

uniform bool uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3 uFogColor;

uniform vec3 uLightAmbient;
uniform vec3 uLightDiffuse;
uniform vec3 uLightEnv;

void main(void) {
	vec4 textureSample = texture(uDiffuse, vTextureCoord.st);

	// Client hard alpha-test (fcn.005384e0.c:32-34, ALPHAREF 0xCF): discard alpha < 207/255,
	// keep the rest OPAQUE. Clips the linear alpha ramp to a tight 1-texel edge.
	if (textureSample.a < uAlphaRef) {
		discard;
	}

	// Phase-0 grayscale directional light: uLightDiffuse / uLightAmbient are fed grayscale
	// (R=G=B) so the shade is the RE directional gradient MODULATE'd with the texel.
	vec3 color = (vLightWeighting * uLightDiffuse + uLightAmbient);
	textureSample.rgb *= clamp(color, 0.0, 1.0);
	textureSample.rgb *= clamp(uLightEnv, 0.0, 1.0);
	// Kept fragments are opaque except during a removal fade, where uAlpha ramps the FINAL alpha
	// down (the discard above already cut the texture edge). uAlpha = 1.0 -> unchanged.
	textureSample.a = uAlpha;

	fragColor = textureSample;

	if (uFogUse) {
		float depth = gl_FragCoord.z / gl_FragCoord.w;
		float fogFactor = smoothstep(uFogNear, uFogFar, depth);
		fragColor = mix(fragColor, vec4(uFogColor, fragColor.w), fogFactor);
	}
}
