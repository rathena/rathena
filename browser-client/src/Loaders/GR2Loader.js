/**
 * Loaders/GR2Loader.js
 *
 * Decodes a Granny3D .gr2 buffer into roBrowser model structures via granny-ro-js
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * The actual decode is delegated to the granny-ro-js package (Oodle0 + type-tree + skeleton +
 * skinning + animation, verified vs granny2.dll). This loader is the thin adapter: parse the
 * bytes, then pack the meshes into per-vertex attribute arrays and read the model
 * InitialPlacement transform. (Superseded the legacy Loaders/GrannyModel.js hand-rolled parser,
 * whose load() bailed before decompression; it has been removed.)
 */

import { parseAll, ready } from 'granny-ro-js/wasm';
import { mul, trans, IDENTITY_ROW } from 'Renderer/GR2/gr2Math.js';

/**
 * granny TRANSFORM_FLAGS bits gating which InitialPlacement components are live.
 */
const HAS_POSITION = 1;
const HAS_ORIENTATION = 2;
const HAS_SCALESHEAR = 4;

// Quaternion (x,y,z,w) to a row-vector rotation matrix (transpose of the column-vector form).
function _quatToRow(x, y, z, w) {
	const xx = x * x;
	const yy = y * y;
	const zz = z * z;
	const xy = x * y;
	const xz = x * z;
	const yz = y * z;
	const wx = w * x;
	const wy = w * y;
	const wz = w * z;
	return [
		1 - 2 * (yy + zz),
		2 * (xy + wz),
		2 * (xz - wy),
		0,
		2 * (xy - wz),
		1 - 2 * (xx + zz),
		2 * (yz + wx),
		0,
		2 * (xz + wy),
		2 * (yz - wx),
		1 - 2 * (xx + yy),
		0,
		0,
		0,
		0,
		1
	];
}

const _isEmblemMesh = name => /Plane/i.test(name);

/**
 * GR2Loader — decode a .gr2 buffer. Parse-in-constructor like Loaders/Model.js:
 *   const model = new GR2Loader(buffer);
 *   model.meshes / model.boneCount / model.parsed / model.ipRow
 */
class GR2Loader {
	constructor(buffer) {
		if (buffer) {
			this.load(buffer);
		}
	}

	/**
	 * Parse the .gr2 bytes into { parsed, meshes, boneCount, animIndex, duration, ipRow }.
	 * A single parseAll decompresses the buffer once and runs every extractor on that one
	 * graph: parsed carries the textured meshes/skeletons, the animations, and models[] —
	 * models[0].initialPlacement is the InitialPlacement transform that grounds the actor.
	 */
	load(buffer) {
		const u8 = new Uint8Array(buffer);
		const parsed = parseAll(u8);
		this.parsed = parsed;

		const packed = GR2Loader.packModel(parsed);
		this.meshes = packed.meshes;
		this.boneCount = packed.boneCount;

		this.animIndex = 0;
		const anim = parsed.animations[0];
		this.duration = anim ? anim.duration : 1;

		const model = parsed.models[0];
		this.ipRow = model ? GR2Loader.ipRowFromTransform(model.initialPlacement) : undefined;
	}

	/**
	 * packModel(parsed) -> { meshes:[{ name, vcount, indices, bind, bidx, bw, uv, nrm, texFile, emblem }], boneCount }.
	 * Per-vertex attribute arrays: bind pos(3), GLOBAL bone indices(4), normalized top-4 weights(4),
	 * uv(2), bind normals(3). Matless proxy submeshes are dropped (the client binds no shader for them).
	 */
	static packModel(parsed) {
		const skel = parsed.skeletons[0];
		if (!skel) {
			throw new Error('GR2Loader: model has no skeleton');
		}

		const bones = skel.bones;
		const byName = {};
		for (let i = 0; i < bones.length; i++) {
			byName[bones[i].name] = i;
		}
		const boneCount = bones.length;

		const textured = parsed.meshes.filter(m => m.materials && m.materials[0] && m.materials[0].textureFile);
		const meshes = textured.map(m => {
			const l2g = m.boneBindings.map(b => (byName[b.name] != null ? byName[b.name] : 0));
			const N = m.vertexCount;
			const bind = new Float32Array(N * 3);
			const bidx = new Float32Array(N * 4);
			const bw = new Float32Array(N * 4);
			const uv = new Float32Array(N * 2);
			const nrm = new Float32Array(N * 3);

			for (let i = 0; i < N; i++) {
				const p = m.positions[i];
				bind[i * 3] = p[0];
				bind[i * 3 + 1] = p[1];
				bind[i * 3 + 2] = p[2];

				const nn = (m.normals && m.normals[i]) || [0, 0, 1];
				nrm[i * 3] = nn[0];
				nrm[i * 3 + 1] = nn[1];
				nrm[i * 3 + 2] = nn[2];

				const t = m.uvs[i] || [0, 0];
				uv[i * 2] = t[0];
				uv[i * 2 + 1] = t[1];

				const vw = m.vertexWeights[i] || [];
				let s = 0;
				for (let j = 0; j < 4; j++) {
					const e = vw[j];
					if (e) {
						bidx[i * 4 + j] = l2g[e.boneIndex] | 0;
						bw[i * 4 + j] = e.weight;
						s += e.weight;
					}
				}
				// Rigid vertex (no weights) binds fully to the first bound bone.
				if (s < 1e-6) {
					bidx[i * 4] = l2g.length ? l2g[0] | 0 : 0;
					bw[i * 4] = 1;
					s = 1;
				}
				for (let j = 0; j < 4; j++) {
					bw[i * 4 + j] /= s;
				}
			}

			const mat0 = m.materials && m.materials[0];
			const texFile = (mat0 && mat0.textureFile) || '';
			return {
				name: m.name,
				vcount: N,
				indices: new Uint16Array(m.indices),
				bind: bind,
				bidx: bidx,
				bw: bw,
				uv: uv,
				nrm: nrm,
				texFile: texFile,
				emblem: _isEmblemMesh(m.name)
			};
		});

		return { meshes: meshes, boneCount: boneCount };
	}

	/**
	 * ipRowFromTransform(t) -> row-vector mat4 for a granny Transform { flags, position,
	 * orientation, scaleShear }, applying ONLY the flag-marked components (S.R.T order).
	 * Identity when the transform is absent or flags is 0.
	 *
	 * POSITION is applied INVERTED (`-p`). The granny InitialPlacement records where the
	 * model sat in its authoring scene; rendering at the actor's cell origin must UNDO that
	 * placement, not re-apply it. A model authored off-origin (e.g. Kguardian90_7, whose mesh
	 * lives entirely forward of origin with an IP `[0.045,-7.253,0]`) otherwise draws ~1.45
	 * cells too far forward — verified against mars-26/data.grf: `-p` re-centers its world-Z
	 * onto its identity-IP siblings. Orientation/scale stay as-authored (translation-free IPs
	 * like the treasurebox Rx+90 are unaffected).
	 */
	static ipRowFromTransform(t) {
		if (!t || !t.flags) {
			return IDENTITY_ROW.slice();
		}
		let M = IDENTITY_ROW.slice();
		if (t.flags & HAS_SCALESHEAR) {
			const s = t.scaleShear;
			M = mul(M, [s[0], s[1], s[2], 0, s[3], s[4], s[5], 0, s[6], s[7], s[8], 0, 0, 0, 0, 1]);
		}
		if (t.flags & HAS_ORIENTATION) {
			const q = t.orientation;
			M = mul(M, _quatToRow(q[0], q[1], q[2], q[3]));
		}
		if (t.flags & HAS_POSITION) {
			const p = t.position;
			M = mul(M, trans(-p[0], -p[1], -p[2]));
		}
		return M;
	}

	/**
	 * Initialize the granny-ro-js WASM (texture pixel decode only). The geometry / skeleton /
	 * animation decode is WASM-independent; call this before decoding textures.
	 */
	static ready() {
		return ready();
	}
}

export default GR2Loader;
