#version 300 es
#pragma vscode_glsllint_stage : frag
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;
uniform sampler2D uDiffuse;
uniform float alpha;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

    void main(void) {
        vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );
        if (textureSample.r < 0.1 || textureSample.g < 0.1 || textureSample.b < 0.1) {
            discard;
        }
        textureSample.a = alpha;
        fragColor = textureSample;
    
    if (uFogUse) {
    float depth     = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = smoothstep( uFogNear, uFogFar, depth );
    fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}