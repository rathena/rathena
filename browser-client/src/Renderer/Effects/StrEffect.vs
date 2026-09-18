#version 300 es
precision highp float;

in vec2 aPosition;
in vec2 aTextureCoord;

out vec2 vTextureCoord;

uniform mat4 uModelViewMat;
uniform mat4 uProjectionMat;

uniform mat4 uSpriteAngle;
uniform vec3 uSpritePosition;
uniform vec2 uSpriteOffset;
uniform float uVerticalBase;

const float pixelRatio = 1.0 / 35.0;

mat4 Project( mat4 mat, vec3 pos) {

	// xyz = x(-z)y + middle of cell (0.5)
	float x =  pos.x + 0.5;
	float y = -pos.z;
	float z =  pos.y + 0.5;

	// Matrix translation
	mat[3].x += mat[0].x * x + mat[1].x * y + mat[2].x * z;
	mat[3].y += mat[0].y * x + mat[1].y * y + mat[2].y * z;
	mat[3].z += mat[0].z * x + mat[1].z * y + mat[2].z * z;
	mat[3].w += mat[0].w * x + mat[1].w * y + mat[2].w * z;

	// Spherical billboard
	mat[0].xyz = vec3( 1.0, 0.0, 0.0 );
	mat[1].xyz = vec3( 0.0, 1.0, 0.0 );
	mat[2].xyz = vec3( 0.0, 0.0, 1.0 );

	return mat;
}

void main(void) {
	vTextureCoord = aTextureCoord;

	// Calculate position base on angle and sprite offset/size
	vec4 position = uSpriteAngle * vec4( aPosition.x * pixelRatio, -aPosition.y * pixelRatio, 0.0, 1.0 );
	position.x   += uSpriteOffset.x * pixelRatio;
	position.y   -= uSpriteOffset.y * pixelRatio + uVerticalBase;

	// Project to camera plane
	gl_Position    = uProjectionMat * Project(uModelViewMat, uSpritePosition) * position;
	gl_Position.z -= 0.1;
}