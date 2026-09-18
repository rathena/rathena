#version 300 es
precision highp float;

in vec2 vTextureCoord;
in vec2 vLightmapCoord;
in vec2 vTileColorCoord;
in float vLightWeighting;
out vec4 fragColor;

uniform sampler2D uDiffuse;
uniform sampler2D uLightmap;
uniform sampler2D uTileColor;
uniform bool uLightMapUse;
uniform bool uPosterize;
uniform bool uGammaCorrection;

uniform bool  uFogUse;
uniform float uFogNear;
uniform float uFogFar;
uniform vec3  uFogColor;

uniform vec3  uLightAmbient;
uniform vec3  uLightDiffuse;
uniform float uLightOpacity;
uniform vec3  uLightDirection;
uniform vec3 uLightEnv;

vec3 posterize(vec3 c) {
    c *= 255.0;
    c = floor(c / 16.0) * 16.0;
    c /= 255.0;
    return c;
}

void main(void) {
    vec4 textureSample = texture(uDiffuse, vTextureCoord.st);
    if (textureSample.a < 0.1)
        discard;

    if (vTileColorCoord.st != vec2(0.0,0.0)) {
        textureSample    *= texture( uTileColor, vTileColorCoord.st);
    }

    vec3 color = (vLightWeighting * uLightDiffuse + uLightAmbient);
    textureSample.rgb *= clamp(color, 0.0, 1.0);
    textureSample.rgb *= clamp(uLightEnv, 0.0, 1.0);

    if (uLightMapUse) {
        vec4 lightmap = texture( uLightmap, vLightmapCoord.st);
        if(uPosterize) {
            lightmap.rgb = posterize(lightmap.rgb);
        } else if(uGammaCorrection){
            lightmap.rgb = pow(lightmap.rgb, vec3(1.1));
        }
        textureSample.rgb *= lightmap.a;
        textureSample.rgb += clamp(lightmap.rgb, 0.0, 1.0);
    }

    fragColor = textureSample;

    if (uFogUse) {
        float depth     = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = smoothstep( uFogNear, uFogFar, depth );
        fragColor    = mix( fragColor, vec4(uFogColor, fragColor.w), fogFactor );
    }

}