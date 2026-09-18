#version 300 es
precision highp float;

in vec2 vTextureCoord;
uniform sampler2D uDiffuse;
uniform vec4 uSpriteRendererColor;
uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;
out vec4 fragColor;

void main(void) {
    if (uSpriteRendererColor.a == 0.0) { discard; }
    
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
    
    if (textureSample.a == 0.0 ) { discard; }
    
    fragColor = textureSample * uSpriteRendererColor;
    
    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}