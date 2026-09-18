#version 300 es
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform vec4 uColor;

void main(void) {
	vec4 texColor = texture(uDiffuse, vTextureCoord);

	if (texColor.a < 0.01) {
		discard;
	}

	fragColor = texColor * uColor;
}