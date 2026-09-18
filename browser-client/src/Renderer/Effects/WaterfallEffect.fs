#version 300 es
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uTexture;
uniform bool uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3 uFogColor;
uniform float uOpacity;

void main(void) {
	fragColor = texture(uTexture, vTextureCoord);
	fragColor.a *= uOpacity;
	if (fragColor.a < 0.01) {
		discard;
	}
	if (uFogUse) {
		float fogFactor = smoothstep(uFogNear, uFogFar, gl_FragCoord.z / gl_FragCoord.w);
		fragColor = mix(fragColor, vec4(uFogColor, fragColor.a), fogFactor);
	}
}
