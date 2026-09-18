#version 300 es
precision mediump float;  

uniform sampler2D uTexture;  
uniform float uSubpix;  
uniform float uEdgeThreshold;  
uniform float uEdgeThresholdMin;  
uniform vec2 uTexelSize;  

in vec2 vUv;  
out vec4 fragColor;  

float luminance(vec3 rgb) {  
    return dot(rgb, vec3(0.299, 0.587, 0.114));  
}  

void main() {  
    vec3 rgbM = texture(uTexture, vUv).rgb;  
      
    vec3 rgbNW = texture(uTexture, vUv + vec2(-uTexelSize.x, -uTexelSize.y)).rgb;  
    vec3 rgbNE = texture(uTexture, vUv + vec2(uTexelSize.x, -uTexelSize.y)).rgb;  
    vec3 rgbSW = texture(uTexture, vUv + vec2(-uTexelSize.x, uTexelSize.y)).rgb;  
    vec3 rgbSE = texture(uTexture, vUv + vec2(uTexelSize.x, uTexelSize.y)).rgb;  

    float lumaM = luminance(rgbM);  
    float lumaNW = luminance(rgbNW);  
    float lumaNE = luminance(rgbNE);  
    float lumaSW = luminance(rgbSW);  
    float lumaSE = luminance(rgbSE);  

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));  
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));  

    float lumaRange = lumaMax - lumaMin;  
    if(lumaRange < max(uEdgeThresholdMin, lumaMax * uEdgeThreshold)) {  
        fragColor = vec4(rgbM, 1.0);  
        return;  
    }  

    vec2 dir;  
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));  
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));  

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * 0.03125, 0.0078125);  
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);  
    dir = min(vec2(8.0), max(vec2(-8.0), dir * rcpDirMin)) * uTexelSize;  

    vec3 rgbA = 0.5 * (  
        texture(uTexture, vUv + dir * (1.0/3.0 - 0.5)).rgb +  
        texture(uTexture, vUv + dir * (2.0/3.0 - 0.5)).rgb);  
    vec3 rgbB = rgbA * 0.5 + 0.25 * (  
        texture(uTexture, vUv + dir * -0.5).rgb +  
        texture(uTexture, vUv + dir * 0.5).rgb);  

    float lumaB = luminance(rgbB);  
    if((lumaB < lumaMin) || (lumaB > lumaMax)) {  
        fragColor = vec4(rgbA, 1.0);  
    } else {  
        fragColor = vec4(rgbB, 1.0);  
    }  
}