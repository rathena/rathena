#version 300 es
precision mediump float;
uniform sampler2D uTexture;
uniform float uBloomThreshold;
uniform float uBloomSoftKnee;
uniform vec2 uTexelSize;

in vec2 vUv;
out vec4 fragColor;

float luminance(vec3 c) {
	return dot(c, vec3(0.2126, 0.7152, 0.0722)); // Use BT.709 coefficients  
}

float threshold(float l, float threshold, float knee) {  
	float soft = l - threshold + knee;  
	soft = clamp(soft, 0.0, 2.0 * knee);  
	return max(soft * soft / (4.0 * knee + 1e-6), l - threshold) / max(l, 1e-6);  
}  

vec3 spdReduce4(vec3 c0, vec3 c1, vec3 c2, vec3 c3) {
	return (c0 + c1 + c2 + c3) * 0.25;
}

vec3 load_and_reduce(sampler2D tex, vec2 uv, vec2 texelSize) {  
	vec2 offset = texelSize * 0.5;  
	  
	vec3 c0 = texture(tex, uv + vec2(-offset.x, -offset.y)).rgb;  
	vec3 c1 = texture(tex, uv + vec2(offset.x, -offset.y)).rgb;  
	vec3 c2 = texture(tex, uv + vec2(-offset.x, offset.y)).rgb;  
	vec3 c3 = texture(tex, uv + vec2(offset.x, offset.y)).rgb;  
	  
	return spdReduce4(c0, c1, c2, c3);
}

void main() {
	vec3 color = load_and_reduce(uTexture, vUv, uTexelSize);
	float l = luminance(color);

	// Soft Threshold logic (Knee curve)
	float contribution = threshold(l, uBloomThreshold, uBloomThreshold * uBloomSoftKnee);

	fragColor = vec4(color * contribution, 1.0);
}