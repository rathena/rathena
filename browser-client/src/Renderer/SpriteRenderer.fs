#version 300 es
precision highp float;

in vec2 vTextureCoord;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform sampler2D uPalette;

uniform bool uUsePal;
uniform vec4 uSpriteRendererColor;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

uniform float uShadow;
uniform vec2 uTextSize;
uniform bool uIsRGBA;

// With palette we don't have a good result because of the gl.NEAREST, so smooth it.
vec4 bilinearSample(vec2 uv, sampler2D indexT, sampler2D LUT) {
    vec2 TextInterval = 1.0 / uTextSize;

    float tlLUT = texture(indexT, uv ).x;
    float trLUT = texture(indexT, uv + vec2(TextInterval.x, 0.0)).x;
    float blLUT = texture(indexT, uv + vec2(0.0, TextInterval.y)).x;
    float brLUT = texture(indexT, uv + TextInterval).x;

    vec4 transparent = vec4( 0.0, 0.0, 0.0, 0.0);

    vec4 tl = tlLUT == 0.0 ? transparent : vec4( texture(LUT, vec2(tlLUT,1.0)).rgb, 1.0);
    vec4 tr = trLUT == 0.0 ? transparent : vec4( texture(LUT, vec2(trLUT,1.0)).rgb, 1.0);
    vec4 bl = blLUT == 0.0 ? transparent : vec4( texture(LUT, vec2(blLUT,1.0)).rgb, 1.0);
    vec4 br = brLUT == 0.0 ? transparent : vec4( texture(LUT, vec2(brLUT,1.0)).rgb, 1.0);

    vec2 f  = fract( uv.xy * uTextSize );
    vec4 tA = mix( tl, tr, f.x );
    vec4 tB = mix( bl, br, f.x );

    return mix( tA, tB, f.y );
}


void main(void) {

    // Don't render if it's not shown.
    if (uSpriteRendererColor.a == 0.0) {
        discard;
    }

    // Calculate texture
    vec4 textureSample;
    if (uUsePal) {
        textureSample = bilinearSample( vTextureCoord, uDiffuse, uPalette );
    }
    else {
        textureSample = texture( uDiffuse, vTextureCoord.st );
    }

    // No alpha, skip.
    if ( textureSample.a == 0.0 )
        discard;

    // Apply shadow, apply color
    textureSample.rgb   *= uShadow;
    fragColor   = textureSample * uSpriteRendererColor;

    // Fog feature
    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4( uFogColor, fragColor.w ), fogFactor );
    }
}