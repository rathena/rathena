#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

void main(void) {
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );

    if (textureSample.a == 0.0) {
        discard;
    }

    if (textureSample.r < 0.5 || textureSample.g < 0.5 || textureSample.b < 0.5) {
        discard;
    }
    textureSample.a = 0.7;
    fragColor = textureSample;

    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}