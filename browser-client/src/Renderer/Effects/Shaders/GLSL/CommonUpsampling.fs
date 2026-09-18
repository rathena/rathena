#version 300 es
precision mediump float;
uniform sampler2D uSceneTexture;

in vec2 vUv;
out vec4 fragColor;

void main() {
	vec3 original = texture(uSceneTexture, vUv).rgb; 
	fragColor = vec4(original, 1.0);
}