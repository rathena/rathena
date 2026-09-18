#version 300 es
precision highp float;

in vec3 aPosition;
in vec3 aNormal;
in vec2 aTextureCoord;
in vec4 aBoneIndex;
in vec4 aBoneWeight;

out vec2 vTextureCoord;
out float vLightWeighting;

uniform mat4 uModelViewMat;   // Camera view * instance world
uniform mat4 uProjectionMat;
uniform mat3 uNormalMat;      // normal-matrix of (view * world) -> VIEW space

uniform mat4 uBones[48];      // GPU skin. MUST equal gr2Pack.js BONE_CAP (48) — GLSL can't import
                              // it, so this literal is the hand-synced mirror; change both together.

uniform vec3 uLightDirection; // scene light direction, VIEW space
uniform float uLightOpacity;

void main(void) {
	mat4 skin = aBoneWeight.x * uBones[int(aBoneIndex.x)]
	          + aBoneWeight.y * uBones[int(aBoneIndex.y)]
	          + aBoneWeight.z * uBones[int(aBoneIndex.z)]
	          + aBoneWeight.w * uBones[int(aBoneIndex.w)];

	vec4 skinned = skin * vec4(aPosition, 1.0);
	gl_Position = uProjectionMat * uModelViewMat * skinned;

	vTextureCoord = aTextureCoord;

	// Skin the normal, then carry it to view space (uNormalMat = normal-matrix of view*world);
	// uLightDirection is handed in already in view space, so N.L shares a frame and stays
	// world-fixed as the orbit camera moves.
	vec3 normal = normalize(uNormalMat * (mat3(skin) * aNormal));
	float lightWeight = max(dot(normal, uLightDirection), 0.0);
	vLightWeighting = (1.0 - uLightOpacity) + lightWeight * uLightOpacity;
}
