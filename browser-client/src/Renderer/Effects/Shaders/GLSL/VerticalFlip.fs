#version 300 es
precision highp float;
in vec2 vTextureCoord;
out vec4 fragColor;
uniform sampler2D uTexture;
void main() {
	fragColor = texture(uTexture, vec2(vTextureCoord.x, 1.0 - vTextureCoord.y));
}