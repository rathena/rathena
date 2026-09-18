#version 300 es
precision highp float;

in vec2 vTextureCoord;
in float vAlpha;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform vec3 uColor;
uniform bool uFogUse;
uniform float uFogNear;
uniform float uFogFar;

void main(void) {
	fragColor = texture(uTexture, vTextureCoord);
	fragColor.rgb *= uColor;
	fragColor.a *= vAlpha;
	if (uFogUse) {
		float fogFactor = smoothstep(uFogNear, uFogFar, gl_FragCoord.z / gl_FragCoord.w);
		fragColor.a *= 1.0 - fogFactor;
	}
	if (fragColor.a < 0.01) {
		discard;
	}
}
