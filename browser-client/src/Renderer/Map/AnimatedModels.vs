#version 300 es  
precision highp float;  
  
in vec3 aPosition;  
in vec3 aNormal;  
in vec2 aTextureCoord;  
in float aAlpha;  
  
out vec2 vTextureCoord;  
out float vLightWeighting;  
out float vAlpha;  
out float vFogFactor;  
out vec3 vWorldPos;
  
uniform mat4 uModelViewMat;  
uniform mat4 uProjectionMat;  
uniform mat3 uNormalMat;  
  
uniform vec3 uLightDirection;  
uniform float uLightOpacity;  
uniform vec3 uLightAmbient;  
uniform vec3 uLightDiffuse;  
  
uniform bool uFogUse;  
uniform float uFogNear;  
uniform float uFogFar;  
  
void main(void) {  
    vec4 position = uModelViewMat * vec4(aPosition, 1.0);  
    gl_Position = uProjectionMat * position;  
  
    vTextureCoord = aTextureCoord;  
    vAlpha = aAlpha;  
    vWorldPos = aPosition;
  
    vec3 normal = normalize(aNormal);  
    float lightWeight = max(dot(normal, uLightDirection), 0.0);  
    vLightWeighting = (1.0 - uLightOpacity) + lightWeight * uLightOpacity;  
  
    if (uFogUse) {  
        float depth = length(position.xyz);  
        vFogFactor = clamp((uFogFar - depth) / (uFogFar - uFogNear), 0.0, 1.0);  
    } else {  
        vFogFactor = 1.0;  
    }  
}