import { describe, it, expect } from 'vitest';
import {
	packRobrowserInterleave,
	flattenPose,
	identityPose,
	quantizePoseTime,
	poseCacheKey,
	POSE_HZ
} from 'Renderer/GR2/gr2Pack.js';

/**
 * Synthetic 2-vertex packed mesh (the shape GR2Loader.packModel emits).
 */
function makeMesh() {
	return {
		vcount: 2,
		bind: new Float32Array([1, 2, 3, 4, 5, 6]),
		nrm: new Float32Array([0.1, 0.2, 0.3, 0.4, 0.5, 0.6]),
		uv: new Float32Array([0.7, 0.8, 0.9, 1.0]),
		bidx: new Float32Array([10, 11, 12, 13, 20, 21, 22, 23]),
		bw: new Float32Array([0.25, 0.25, 0.25, 0.25, 0.4, 0.3, 0.2, 0.1])
	};
}

describe('packRobrowserInterleave', () => {
	it('lays out 16 floats/vert at pos@0 normal@3 uv@6 bidx@8 bw@12', () => {
		const mesh = makeMesh();
		const buf = packRobrowserInterleave(mesh);

		expect(buf).toBeInstanceOf(Float32Array);
		expect(buf.length).toBe(mesh.vcount * 16);

		for (let i = 0; i < mesh.vcount; i++) {
			const o = i * 16;
			// Position sources the BIND pose (the vertex shader re-poses it), not a runtime position.
			expect([buf[o], buf[o + 1], buf[o + 2]]).toEqual([mesh.bind[i * 3], mesh.bind[i * 3 + 1], mesh.bind[i * 3 + 2]]);
			expect([buf[o + 3], buf[o + 4], buf[o + 5]]).toEqual([mesh.nrm[i * 3], mesh.nrm[i * 3 + 1], mesh.nrm[i * 3 + 2]]);
			expect([buf[o + 6], buf[o + 7]]).toEqual([mesh.uv[i * 2], mesh.uv[i * 2 + 1]]);
			expect([buf[o + 8], buf[o + 9], buf[o + 10], buf[o + 11]]).toEqual([
				mesh.bidx[i * 4],
				mesh.bidx[i * 4 + 1],
				mesh.bidx[i * 4 + 2],
				mesh.bidx[i * 4 + 3]
			]);
			expect([buf[o + 12], buf[o + 13], buf[o + 14], buf[o + 15]]).toEqual([
				mesh.bw[i * 4],
				mesh.bw[i * 4 + 1],
				mesh.bw[i * 4 + 2],
				mesh.bw[i * 4 + 3]
			]);
		}
	});
});

describe('flattenPose / identityPose', () => {
	it('flattens skinning matrices contiguously', () => {
		const pose = { skinningMatrices: [new Float32Array(16).fill(2), new Float32Array(16).fill(3)] };
		const flat = flattenPose(pose, 2);
		expect(flat.length).toBe(32);
		expect(flat[0]).toBe(2);
		expect(flat[16]).toBe(3);
	});

	it('identityPose emits identity mat4s', () => {
		const id = identityPose(2);
		expect(id.length).toBe(32);
		// Diagonal set, off-diagonal zero.
		expect([id[0], id[5], id[10], id[15]]).toEqual([1, 1, 1, 1]);
		expect([id[16], id[21], id[26], id[31]]).toEqual([1, 1, 1, 1]);
		expect(id[1]).toBe(0);
	});
});

describe('pose cache key / quantization', () => {
	it('collapses near-equal clip times inside one 1/40 s bucket', () => {
		expect(POSE_HZ).toBe(40);
		// 0.011 s and 0.012 s both round to bucket 0 at 40 Hz.
		expect(quantizePoseTime(0.011)).toBe(quantizePoseTime(0.012));
		expect(poseCacheKey('a.gr2', 0, quantizePoseTime(0.011))).toBe(poseCacheKey('a.gr2', 0, quantizePoseTime(0.012)));
	});

	it('separates distinct time buckets, anim indices, and paths', () => {
		expect(quantizePoseTime(0.011)).not.toBe(quantizePoseTime(0.05));
		const base = poseCacheKey('a.gr2', 0, 0);
		expect(poseCacheKey('a.gr2', 0, 1)).not.toBe(base); // bucket
		expect(poseCacheKey('a.gr2', 1, 0)).not.toBe(base); // anim
		expect(poseCacheKey('b.gr2', 0, 0)).not.toBe(base); // path
	});
});
