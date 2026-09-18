#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;

uniform vec4 uColor;
uniform sampler2D uDiffuse;
float tmp;
out vec4 fragColor;

void main(void) {
    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
    
    textureSample *= uColor;
    
    /*if (texture.r < 0.1 && texture.g < 0.1 && texture.b < 0.1) {
        discard;
    }*/
    
    fragColor = textureSample;

}