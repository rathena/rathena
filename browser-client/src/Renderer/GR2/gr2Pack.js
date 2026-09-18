/**
 * Renderer/GR2/gr2Pack.js
 *
 * Pure (no WebGL, no wasm) helpers for the GR2 GPU-skin renderer: the roBrowser
 * vertex interleave, pose flattening, and the 40 Hz pose-cache key. Kept free of
 * GL / granny-ro-js imports so the headless unit tests can exercise them directly.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

/**
 * Rag tick rate — poses are recomputed on the 40 Hz game tick, not per rAF, so
 * same-type / same-phase mobs collapse to a single poseAt per bucket.
 */
export const POSE_HZ = 40;

/**
 * Max bones a GR2 skeleton may carry (mat4 uBones[48] in GR2Model.vs). The verified
 * guildflag is 41; the client's 3dmob actors stay well under this.
 */
export const BONE_CAP = 48;

/**
 * packRobrowserInterleave(mesh) -> Float32Array(vcount * 16): roBrowser's model
 * vertex layout with the grafted GPU-skin attributes. Positions come from the
 * BIND pose (the vertex shader re-poses them from uBones).
 *
 *   aPosition@0(3) . aNormal@3(3) . aTextureCoord@6(2) . aBoneIndex@8(4) . aBoneWeight@12(4)
 *   = 16 floats/vert, stride 64 bytes.
 */
export function packRobrowserInterleave(mesh) {
	const n = mesh.vcount;
	const buf = new Float32Array(n * 16);
	for (let i = 0; i < n; i++) {
		const o = i * 16;
		buf[o] = mesh.bind[i * 3];
		buf[o + 1] = mesh.bind[i * 3 + 1];
		buf[o + 2] = mesh.bind[i * 3 + 2];
		buf[o + 3] = mesh.nrm[i * 3];
		buf[o + 4] = mesh.nrm[i * 3 + 1];
		buf[o + 5] = mesh.nrm[i * 3 + 2];
		buf[o + 6] = mesh.uv[i * 2];
		buf[o + 7] = mesh.uv[i * 2 + 1];
		buf[o + 8] = mesh.bidx[i * 4];
		buf[o + 9] = mesh.bidx[i * 4 + 1];
		buf[o + 10] = mesh.bidx[i * 4 + 2];
		buf[o + 11] = mesh.bidx[i * 4 + 3];
		buf[o + 12] = mesh.bw[i * 4];
		buf[o + 13] = mesh.bw[i * 4 + 1];
		buf[o + 14] = mesh.bw[i * 4 + 2];
		buf[o + 15] = mesh.bw[i * 4 + 3];
	}
	return buf;
}

/**
 * flattenPose(pose, boneCount) -> Float32Array(boneCount * 16): the granny skinning
 * matrices laid out contiguously for the uBones uniform array (column-major per bone).
 */
export function flattenPose(pose, boneCount) {
	const s = new Float32Array(boneCount * 16);
	for (let i = 0; i < boneCount; i++) {
		s.set(pose.skinningMatrices[i], i * 16);
	}
	return s;
}

/**
 * identityPose(boneCount) -> Float32Array of identity mat4s (the undeformed bind pose).
 */
export function identityPose(boneCount) {
	const s = new Float32Array(boneCount * 16);
	for (let i = 0; i < boneCount; i++) {
		const o = i * 16;
		s[o] = s[o + 5] = s[o + 10] = s[o + 15] = 1;
	}
	return s;
}

/**
 * quantizePoseTime(t, hz) -> integer bucket index at hz (default 40 Hz). Near-equal
 * clip times inside one 1/40 s bucket collapse to the same bucket, so the pose cache
 * computes one poseAt per (type, anim, bucket).
 */
export function quantizePoseTime(t, hz) {
	return Math.round(t * (hz || POSE_HZ));
}

/**
 * poseCacheKey(path, animIndex, quant) -> cache key for a posed skeleton. Distinct
 * gr2 path, anim index, or time bucket never collide.
 */
export function poseCacheKey(path, animIndex, quant) {
	return path + '|' + animIndex + '|' + quant;
}

/**
 * gr2ActionFor(entity) -> the GR2 action name for a mob entity's CURRENT action, read
 * SEMANTICALLY via the entity's own ACTION enum (correct for MOB and NPC). Idle / state-0 /
 * any unmapped action -> 'stand' (animated idx-0 standby; a missing bank falls to poseAt(-1)).
 */
export function gr2ActionFor(entity) {
	const A = entity.ACTION;
	const a = entity.action;
	if (a === A.DIE) {
		return 'dead';
	}
	if (a === A.HURT) {
		return 'damage';
	}
	if (a === A.ATTACK || a === A.ATTACK1 || a === A.ATTACK2 || a === A.ATTACK3) {
		return 'attack';
	}
	if (a === A.WALK) {
		return 'move';
	}
	return 'stand';
}

/**
 * frustumCullClip(x, y, w, margin) -> true when a clip-space point is off-screen (cull):
 * behind the camera (w <= 0) or beyond the padded [-1,1] NDC box on x/y.
 */
export function frustumCullClip(x, y, w, margin) {
	if (w <= 0) {
		return true;
	}
	const m = (1 + margin) * w;
	return x < -m || x > m || y < -m || y > m;
}
