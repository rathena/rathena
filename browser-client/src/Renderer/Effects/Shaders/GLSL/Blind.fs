#version 300 es
precision mediump float;

uniform sampler2D uTexture;
uniform float uFocusRadius;
uniform float uFocusFalloff;
uniform vec2 uAspectRatio;

in vec2 vUv;
out vec4 fragColor;

void main() {
	vec2 uv = (vUv - 0.5) * uAspectRatio;  
	float dist = length(uv); 
	vec3 original = texture(uTexture, vUv).rgb;
	float effectMask = smoothstep(uFocusRadius, uFocusRadius + uFocusFalloff, dist);

	if (effectMask <= 0.01) {
		fragColor = texture(uTexture, vUv);
		return;
	}
	vec3 finalColor = mix(original, vec3(0.0), effectMask);  
	fragColor = vec4(finalColor, 1.0);
}