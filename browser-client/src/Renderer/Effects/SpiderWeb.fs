#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;

void main(void) {
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
    if (textureSample.r < 0.5 || textureSample.g < 0.5 || textureSample.b < 0.5) {
        discard;
    }
    textureSample.a = 0.7;
    fragColor = textureSample;
}