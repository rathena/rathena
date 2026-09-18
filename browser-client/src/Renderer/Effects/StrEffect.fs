#version 300 es
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform vec4 uSpriteColor;
uniform sampler2D uDiffuse;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

void main(void) {
	fragColor = texture( uDiffuse, vTextureCoord.st ) * uSpriteColor;
	if ( fragColor.a == 0.0 || (fragColor.r == 0.0 && fragColor.g == 0.0 && fragColor.b == 0.0) ) {
		discard;
	}

	if ( uFogUse ) {
		float depth     = gl_FragCoord.z / gl_FragCoord.w;
		float fogFactor = smoothstep( uFogNear, uFogFar, depth );
		fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
	}
}