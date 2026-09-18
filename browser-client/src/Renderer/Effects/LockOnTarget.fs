#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;

uniform sampler2D uDiffuse;
uniform float uColor;
out vec4 fragColor;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

void main(void) {
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );

    if (textureSample.a == 0.0) {
        discard;
    }

    fragColor     = textureSample;
    fragColor.gb *= uColor;

    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}