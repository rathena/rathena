#version 300 es
precision mediump float;

uniform sampler2D uTexture;
uniform float uFocusRadius;
uniform float uFocusFalloff;
uniform vec2 uTexelSize;

in vec2 vUv;
out vec4 fragColor;

vec3 spdReduce4(vec3 c0, vec3 c1, vec3 c2, vec3 c3) {
	return (c0 + c1 + c2 + c3) * 0.25;
}

vec3 load_and_reduce(sampler2D tex, vec2 uv, vec2 texelSize, float intensity) {  
	vec2 offset = texelSize * (intensity * 0.5); 
	  
	vec3 c0 = texture(tex, uv + vec2(-offset.x, -offset.y)).rgb;  
	vec3 c1 = texture(tex, uv + vec2(offset.x, -offset.y)).rgb;  
	vec3 c2 = texture(tex, uv + vec2(-offset.x, offset.y)).rgb;  
	vec3 c3 = texture(tex, uv + vec2(offset.x, offset.y)).rgb;  
	  
	return spdReduce4(c0, c1, c2, c3);
}  

void main() {
	float dist = distance(vUv, vec2(0.5));
	vec3 original = texture(uTexture, vUv).rgb;
	float effectMask = smoothstep(uFocusRadius, uFocusRadius + uFocusFalloff, dist);

	if (effectMask <= 0.01) {
		fragColor = texture(uTexture, vUv);
		return;
	}

	vec3 blurred = load_and_reduce(uTexture, vUv, uTexelSize, effectMask);
	fragColor = vec4(mix(original, blurred, effectMask), 1.0);
}