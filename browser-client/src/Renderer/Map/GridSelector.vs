#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTextCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

void main(void) {
    vec3 position = aPosition;
    position.y    -= 0.1;
    
    gl_Position    = uProjectionMat * uModelViewMat * vec4( position.xyz, 1.0) ;
    vTextureCoord  = aTextCoord;
}