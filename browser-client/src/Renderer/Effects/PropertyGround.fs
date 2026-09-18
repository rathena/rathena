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
    vec4 texture = texture( uDiffuse,  vTextureCoord.st );

    if (texture.a == 0.0) {
        discard;
    }

    if (texture.r < 0.5 || texture.g < 0.5 || texture.b < 0.5) {
        discard;
    }
    texture.a = 0.42;
    fragColor = texture;

    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}