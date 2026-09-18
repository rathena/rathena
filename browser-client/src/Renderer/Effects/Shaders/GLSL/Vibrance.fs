#version 300 es
precision mediump float;  

uniform sampler2D uTexture;  
uniform float uVibrance;  
uniform vec3 uVibranceRGBBalance;  

in vec2 vUv;  
out vec4 fragColor;  

void main() {  
    vec3 color = texture(uTexture, vUv).rgb;  
      
    // (Rec. 709)  
    vec3 coefLuma = vec3(0.212656, 0.715158, 0.072186);  
    float luma = dot(coefLuma, color);  

    float max_color = max(color.r, max(color.g, color.b));  
    float min_color = min(color.r, min(color.g, color.b));  
    float color_saturation = max_color - min_color;  

    vec3 coeffVibrance = uVibranceRGBBalance * uVibrance;  

    float strength = 1.0 + (coeffVibrance.r * (1.0 - (sign(coeffVibrance.r) * color_saturation)));  
    color.r = mix(luma, color.r, strength);  
      
    strength = 1.0 + (coeffVibrance.g * (1.0 - (sign(coeffVibrance.g) * color_saturation)));  
    color.g = mix(luma, color.g, strength);  
      
    strength = 1.0 + (coeffVibrance.b * (1.0 - (sign(coeffVibrance.b) * color_saturation)));  
    color.b = mix(luma, color.b, strength);  

    fragColor = vec4(color, 1.0);  
} 