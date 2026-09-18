#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;
uniform vec4 uColor;
uniform sampler2D uDiffuse;
uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;
out vec4 fragColor;

void main(void) {
    
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
    
    if (textureSample.a == 0.0 ) { discard; }
    if (textureSample.r < 0.01 && textureSample.g < 0.01 && textureSample.b < 0.01) {
        discard;
        }
    
    fragColor = textureSample * uColor;

    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}