#version 300 es 
precision mediump float;  

uniform sampler2D uTexture;  
uniform float uContrast;  
uniform float uSharpening;  
uniform vec2 uTexelSize;  

in vec2 vUv;  
out vec4 fragColor;  

void main() {  
    // Sampling 3x3  
    //  a b c  
    //  d(e)f  
    //  g h i  
      
    vec3 a = texture(uTexture, vUv + vec2(-uTexelSize.x, -uTexelSize.y)).rgb;  
    vec3 b = texture(uTexture, vUv + vec2(0.0, -uTexelSize.y)).rgb;  
    vec3 c = texture(uTexture, vUv + vec2(uTexelSize.x, -uTexelSize.y)).rgb;  
    vec3 d = texture(uTexture, vUv + vec2(-uTexelSize.x, 0.0)).rgb;  
    vec3 e = texture(uTexture, vUv).rgb;  
    vec3 f = texture(uTexture, vUv + vec2(uTexelSize.x, 0.0)).rgb;  
    vec3 g = texture(uTexture, vUv + vec2(-uTexelSize.x, uTexelSize.y)).rgb;  
    vec3 h = texture(uTexture, vUv + vec2(0.0, uTexelSize.y)).rgb;  
    vec3 i = texture(uTexture, vUv + vec2(uTexelSize.x, uTexelSize.y)).rgb;  

    // Soft min e max 
    vec3 mnRGB = min(min(min(d, e), min(f, b)), h);  
    vec3 mnRGB2 = min(mnRGB, min(min(a, c), min(g, i)));  
    mnRGB += mnRGB2;  

    vec3 mxRGB = max(max(max(d, e), max(f, b)), h);  
    vec3 mxRGB2 = max(mxRGB, max(max(a, c), max(g, i)));  
    mxRGB += mxRGB2;  

    vec3 rcpMRGB = 1.0 / mxRGB;  
    vec3 ampRGB = clamp(min(mnRGB, 2.0 - mxRGB) * rcpMRGB, 0.0, 1.0);  

    ampRGB = inversesqrt(ampRGB);  

    float peak = -3.0 * uContrast + 8.0;  
    vec3 wRGB = -1.0 / (ampRGB * peak);  
    vec3 rcpWeightRGB = 1.0 / (4.0 * wRGB + 1.0);  

    // Cross Filter 
    //  0 w 0  
    //  w 1 w  
    //  0 w 0  
    vec3 window = (b + d) + (f + h);  
    vec3 outColor = clamp((window * wRGB + e) * rcpWeightRGB, 0.0, 1.0);  

    // Blend 
    fragColor = vec4(mix(e, outColor, uSharpening), 1.0);  
}