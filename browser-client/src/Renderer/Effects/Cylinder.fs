#version 300 es

precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform vec4 uSpriteRendererColor;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

void main(void) {

    if (uSpriteRendererColor.a ==  0.0) {
        discard;
    }

    vec4 textureSample = texture( uDiffuse,  vTextureCoord.st );

    if (textureSample.a == 0.0) {
        discard;
    }

    if (textureSample.r < 0.01 && textureSample.g < 0.01 && textureSample.b < 0.01) {
        discard;
    }

    fragColor   = textureSample * uSpriteRendererColor;

    if (uFogUse) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}