#version 300 es
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform vec4 uColor;
uniform bool uSolidBg;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

void main(void) {
	vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
	textureSample *= uColor;
	fragColor = textureSample;
	if (uFogUse) {
		float depth = gl_FragCoord.z / gl_FragCoord.w;
		float fogFactor = smoothstep(uFogNear, uFogFar, depth);
		fragColor = mix(fragColor, vec4(uFogColor, fragColor.w), fogFactor);
	}
}