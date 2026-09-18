#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aVertexNormal;
in vec2 aTextureCoord;
in vec2 aLightmapCoord;
in vec2 aTileColorCoord;

out vec2 vTextureCoord;
out vec2 vLightmapCoord;
out vec2 vTileColorCoord;
out float vLightWeighting;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform vec3 uLightDirection;
uniform vec3 uLightEnv;

void main(void) {
    gl_Position     = uProjectionMat * uModelViewMat * vec4( aPosition, 1.0);

    vTextureCoord   = aTextureCoord;
    vLightmapCoord  = aLightmapCoord;
    vTileColorCoord = aTileColorCoord;

    float dotProduct = dot(aVertexNormal, uLightDirection );
    vLightWeighting = max(dotProduct, 0.0);
}