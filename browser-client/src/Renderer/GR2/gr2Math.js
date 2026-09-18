/**
 * Renderer/GR2/gr2Math.js
 *
 * D3D row-vector 4x4 matrix helpers (convention v' = v.M) shared by the GR2 world-matrix
 * builder (gr2World.js) and the loader's InitialPlacement decode (GR2Loader.js). granny-ro-js
 * hands geometry back row-major, so the whole GR2 chain is built row-vector and handed to
 * GL / gl-matrix as-is (see gr2World.js MATRIX CONVENTION).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

export const IDENTITY_ROW = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];

export function mul(a, b) {
	const o = new Array(16);
	for (let r = 0; r < 4; r++) {
		for (let c = 0; c < 4; c++) {
			let s = 0;
			for (let k = 0; k < 4; k++) {
				s += a[r * 4 + k] * b[k * 4 + c];
			}
			o[r * 4 + c] = s;
		}
	}
	return o;
}

export const trans = (x, y, z) => [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, x, y, z, 1];
