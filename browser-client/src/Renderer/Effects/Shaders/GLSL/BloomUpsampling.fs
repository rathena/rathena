#version 300 es
precision mediump float;
uniform sampler2D uSceneTexture;
uniform sampler2D uBloomTexture;
uniform float uBloomIntensity;

in vec2 vUv;
out vec4 fragColor;

void main() {
	vec3 original = texture(uSceneTexture, vUv).rgb; 
	// Hardware handles bilinear upsampling here
	vec3 bloom = texture(uBloomTexture, vUv).rgb;
	fragColor = vec4(original + (bloom * uBloomIntensity), 1.0);
}