#version 300 es
precision mediump float;  

uniform sampler2D uTexture;  
uniform float uPower;  
uniform float uEdgeSlope;  
uniform vec2 uTexelSize;  

in vec2 vUv;  
out vec4 fragColor;  

void main() {  
    vec3 color = texture(uTexture, vUv).rgb;  

    const vec3 coefLuma = vec3(0.2126, 0.7152, 0.0722);  

    float diff1 = dot(coefLuma, texture(uTexture, vUv + uTexelSize).rgb);  
    diff1 = dot(vec4(coefLuma, -1.0), vec4(texture(uTexture, vUv - uTexelSize).rgb, diff1));  
      
    float diff2 = dot(coefLuma, texture(uTexture, vUv + uTexelSize * vec2(1, -1)).rgb);  
    diff2 = dot(vec4(coefLuma, -1.0), vec4(texture(uTexture, vUv + uTexelSize * vec2(-1, 1)).rgb, diff2));  

    float edge = dot(vec2(diff1, diff2), vec2(diff1, diff2));  

    vec3 result = clamp(pow(abs(edge), uEdgeSlope) * -uPower + color, 0.0, 1.0);  
    fragColor = vec4(result, 1.0);  
}