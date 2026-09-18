/**
 * Renderer/GR2/GR2ViewerRenderer.js
 *
 * Standalone GPU-skinned Granny3D (.gr2) renderer for the GrannyModelViewer app.
 *
 * The on-map GR2ModelRenderer.js is tightly coupled to the map/entity system (Altitude,
 * SessionStorage, animation banks, per-guild emblems, frustum culling). The asset viewer
 * only needs to draw ONE model with an orbit camera and its embedded standby animation, so
 * this is a thin, self-contained draw path over the SAME decode (Loaders/GR2Loader.js) and
 * the SAME shaders (GR2Model.vs/fs) + vertex layout (gr2Pack.packRobrowserInterleave).
 *
 * NOTE (design / TODO): the RSM ModelViewer needs no such extra renderer because it reuses the
 * shared Renderer/Map/Models.js (a clean init/render/free API used both in-game and by the
 * viewer). The GR2 analogue, GR2ModelRenderer.js, is not yet reusable that way — its render loop
 * is map/entity-driven. For NOW we ship this separate viewer-only renderer to keep the fix
 * decoupled and risk-free; in the LONG RUN GR2ModelRenderer should be refactored to split its
 * map-coupled bits from a reusable core, and this file should then be retired in favour of that
 * single shared GR2 renderer (the "option 2" path).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import glMatrix from 'Utils/gl-matrix.js';
import WebGL from 'Utils/WebGL.js';
import { poseAt } from 'granny-ro-js/wasm';
import { packRobrowserInterleave, flattenPose, identityPose } from './gr2Pack.js';
import { mul, IDENTITY_ROW } from './gr2Math.js';
import _vertexShader from './GR2Model.vs?raw';
import _fragmentShader from './GR2Model.fs?raw';

const mat3 = glMatrix.mat3;
const mat4 = glMatrix.mat4;

/**
 * Row-vector reflection + pitch that carries a granny right-handed model into a plain Y-up
 * orbit camera. The on-map path (gr2World.js) uses a clean Rx(+pi/2) because roBrowser's game
 * camera already inverts world Y (that inversion IS the RH->LH bridge); the standalone viewer
 * builds its OWN Y-up camera with no such inversion, so it carries the reflection in-model as
 * flipX . Rx(-pi/2) instead (the two are equivalent — see gr2World.js HANDEDNESS). Rx(-pi/2)
 * also turns the model's Z-up authoring axis into the camera's Y-up.
 */
const _FLIP_X = [-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
const _ROT_X_NEG90 = [1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1];

/**
 * Client hard alpha-test (207/255) — matches the on-map GR2 path (GR2Model.fs).
 */
const ALPHA_REF = 207 / 255;

/**
 * Interleaved vertex layout — MUST mirror gr2Pack.packRobrowserInterleave (16 floats/vertex,
 * 64-byte stride): position(3), normal(3), uv(2), boneIndex(4), boneWeight(4).
 */
const GR2_VERTEX_STRIDE = 64;
const GR2_VERTEX_LAYOUT = [
	{ attr: 'aPosition', size: 3, offset: 0 },
	{ attr: 'aNormal', size: 3, offset: 12 },
	{ attr: 'aTextureCoord', size: 2, offset: 24 },
	{ attr: 'aBoneIndex', size: 4, offset: 32 },
	{ attr: 'aBoneWeight', size: 4, offset: 48 }
];

/**
 * 1x1 fallback pixels: a decoded-nothing granny texture -> neutral grey; a submesh whose
 * texFile matches no embedded texture -> the shared __grey stand-in.
 */
const TEX_MISSING_PX = new Uint8Array([200, 200, 200, 255]);
const TEX_GREY_PX = new Uint8Array([180, 180, 185, 255]);

let _program = null;

let _submeshes = null;
let _textures = null;
let _parsed = null;
let _boneCount = 0;
let _animIndex = -1;
let _duration = 1;

// Column-major world matrix (row-vector chain reinterpreted as-is for gl-matrix — see
// gr2World.js MATRIX CONVENTION): the flipX . Rx(-pi/2) reflection with the model's own
// granny InitialPlacement (ipRow) applied innermost.
let _world = null;

// World-space bounds { center:[x,y,z], radius } used by the viewer to frame the camera.
let _bounds = null;

// Scratchpads.
const _mv = mat4.create();
const _nmat = mat3.create();
const _lightView = new Float32Array(3);

/**
 * keyGr2Magenta(px) — gr2 texture transparency: key MAGENTA 0xFF00FF by zeroing the whole
 * pixel to black + alpha 0 (the on-map GR2 path does the same). A 32-bit source with its own
 * alpha is honoured verbatim.
 */
function keyGr2Magenta(px) {
	for (let i = 3; i < px.length; i += 4) {
		if (px[i] < 255) {
			return px;
		}
	}
	const out = new Uint8Array(px);
	for (let i = 0; i < out.length; i += 4) {
		if (out[i] >= 248 && out[i + 1] < 8 && out[i + 2] >= 248) {
			out[i] = out[i + 1] = out[i + 2] = out[i + 3] = 0;
		}
	}
	return out;
}

/**
 * Compile the shader program (once) and resolve its uniform/attribute locations.
 */
function initProgram(gl) {
	if (_program) {
		return;
	}

	_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

	_program.uniform = {
		uModelViewMat: gl.getUniformLocation(_program, 'uModelViewMat'),
		uProjectionMat: gl.getUniformLocation(_program, 'uProjectionMat'),
		uNormalMat: gl.getUniformLocation(_program, 'uNormalMat'),
		uBones: gl.getUniformLocation(_program, 'uBones[0]'),
		uLightDirection: gl.getUniformLocation(_program, 'uLightDirection'),
		uLightOpacity: gl.getUniformLocation(_program, 'uLightOpacity'),
		uLightAmbient: gl.getUniformLocation(_program, 'uLightAmbient'),
		uLightDiffuse: gl.getUniformLocation(_program, 'uLightDiffuse'),
		uLightEnv: gl.getUniformLocation(_program, 'uLightEnv'),
		uAlphaRef: gl.getUniformLocation(_program, 'uAlphaRef'),
		uAlpha: gl.getUniformLocation(_program, 'uAlpha'),
		uFogUse: gl.getUniformLocation(_program, 'uFogUse'),
		uFogNear: gl.getUniformLocation(_program, 'uFogNear'),
		uFogFar: gl.getUniformLocation(_program, 'uFogFar'),
		uFogColor: gl.getUniformLocation(_program, 'uFogColor'),
		uDiffuse: gl.getUniformLocation(_program, 'uDiffuse')
	};

	_program.attribute = {
		aPosition: gl.getAttribLocation(_program, 'aPosition'),
		aNormal: gl.getAttribLocation(_program, 'aNormal'),
		aTextureCoord: gl.getAttribLocation(_program, 'aTextureCoord'),
		aBoneIndex: gl.getAttribLocation(_program, 'aBoneIndex'),
		aBoneWeight: gl.getAttribLocation(_program, 'aBoneWeight')
	};
}

/**
 * Upload one texture per embedded granny texture, keyed by name. LINEAR + CLAMP sampler.
 */
function makeTextures(gl, parsed) {
	const byFile = {};
	const list = (parsed && parsed.textures) || [];
	for (let i = 0; i < list.length; i++) {
		const t = list[i];
		const tex = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, tex);
		if (t && t.pixels) {
			gl.texImage2D(
				gl.TEXTURE_2D,
				0,
				gl.RGBA,
				t.width,
				t.height,
				0,
				gl.RGBA,
				gl.UNSIGNED_BYTE,
				keyGr2Magenta(t.pixels)
			);
		} else {
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, TEX_MISSING_PX);
		}
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		byFile[t.name] = tex;
	}

	const grey = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, grey);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, TEX_GREY_PX);
	byFile.__grey = grey;
	gl.bindTexture(gl.TEXTURE_2D, null);
	return { byFile: byFile };
}

/**
 * transformRow(m, p) -> row-vector point transform p . m (p is a position, w = 1). Used to carry
 * the bind-pose bounds center into world space (radius is reflection/rotation-invariant).
 */
function transformRow(m, p) {
	return [
		p[0] * m[0] + p[1] * m[4] + p[2] * m[8] + m[12],
		p[0] * m[1] + p[1] * m[5] + p[2] * m[9] + m[13],
		p[0] * m[2] + p[1] * m[6] + p[2] * m[10] + m[14]
	];
}

/**
 * computeBounds(meshes) -> { center:[x,y,z], radius } over the bind-pose vertices; the viewer
 * frames its orbit camera off this. radius is the bounding-sphere radius (half the diagonal),
 * clamped to a small floor so a degenerate model never yields a zero camera distance.
 */
function computeBounds(meshes) {
	let minX = Infinity,
		minY = Infinity,
		minZ = Infinity;
	let maxX = -Infinity,
		maxY = -Infinity,
		maxZ = -Infinity;

	for (let k = 0; k < meshes.length; k++) {
		const bind = meshes[k].bind;
		const vc = meshes[k].vcount;
		for (let i = 0; i < vc; i++) {
			const x = bind[i * 3];
			const y = bind[i * 3 + 1];
			const z = bind[i * 3 + 2];
			if (x < minX) minX = x;
			if (x > maxX) maxX = x;
			if (y < minY) minY = y;
			if (y > maxY) maxY = y;
			if (z < minZ) minZ = z;
			if (z > maxZ) maxZ = z;
		}
	}

	if (minX === Infinity) {
		return { center: [0, 0, 0], radius: 1 };
	}

	const dx = maxX - minX;
	const dy = maxY - minY;
	const dz = maxZ - minZ;
	return {
		center: [(minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2],
		radius: Math.max(0.5 * Math.hypot(dx, dy, dz), 0.001)
	};
}

/**
 * init(gl, loader) — build the GL resources for a decoded GR2Loader model: one VAO/VBO/IBO
 * per textured submesh + one texture per embedded granny texture. Replaces any previous model.
 */
function init(gl, loader) {
	initProgram(gl);
	free(gl);

	_parsed = loader.parsed;
	_boneCount = loader.boneCount;

	// World orientation: v . ipRow . flipX . Rx(-pi/2) (row-vector), handed to gl-matrix as-is.
	const ipRow = loader.ipRow || IDENTITY_ROW;
	_world = new Float32Array(mul(ipRow, mul(_FLIP_X, _ROT_X_NEG90)));

	const bindBounds = computeBounds(loader.meshes);
	_bounds = { center: transformRow(_world, bindBounds.center), radius: bindBounds.radius };

	// Prefer the embedded standby animation (index 0); fall back to the static bind pose.
	const anims = (loader.parsed && loader.parsed.animations) || [];
	_animIndex = anims.length ? 0 : -1;
	_duration = anims.length && anims[0].duration ? anims[0].duration : 1;

	_textures = makeTextures(gl, loader.parsed);

	const attr = _program.attribute;
	_submeshes = loader.meshes.map(function (mesh) {
		const vao = gl.createVertexArray();
		gl.bindVertexArray(vao);

		const vbo = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
		gl.bufferData(gl.ARRAY_BUFFER, packRobrowserInterleave(mesh), gl.STATIC_DRAW);

		for (let a = 0; a < GR2_VERTEX_LAYOUT.length; a++) {
			const layout = GR2_VERTEX_LAYOUT[a];
			const loc = attr[layout.attr];
			gl.enableVertexAttribArray(loc);
			gl.vertexAttribPointer(loc, layout.size, gl.FLOAT, false, GR2_VERTEX_STRIDE, layout.offset);
		}

		const ibo = gl.createBuffer();
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ibo);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, mesh.indices, gl.STATIC_DRAW);

		gl.bindVertexArray(null);
		return {
			vao: vao,
			vbo: vbo,
			ibo: ibo,
			count: mesh.indices.length,
			tex: _textures.byFile[mesh.texFile] || _textures.byFile.__grey
		};
	});
}

/**
 * Free the current model's GL resources (VAOs/VBOs/IBOs/textures). The compiled program is
 * kept — roBrowser reuses one persistent WebGL context across model loads.
 */
function free(gl) {
	if (_submeshes) {
		for (let s = 0; s < _submeshes.length; s++) {
			const sm = _submeshes[s];
			gl.deleteVertexArray(sm.vao);
			gl.deleteBuffer(sm.vbo);
			gl.deleteBuffer(sm.ibo);
		}
	}
	if (_textures) {
		for (const file in _textures.byFile) {
			gl.deleteTexture(_textures.byFile[file]);
		}
	}
	_submeshes = null;
	_textures = null;
	_parsed = null;
	_boneCount = 0;
	_world = null;
	_bounds = null;
	_animIndex = -1;
	_duration = 1;
}

function mat3MulVec3(m, v, out) {
	out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
	out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
	out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
	const len = Math.hypot(out[0], out[1], out[2]) || 1;
	out[0] /= len;
	out[1] /= len;
	out[2] /= len;
}

/**
 * getBounds() -> the current model's bind-pose { center, radius } (or null when no model).
 */
function getBounds() {
	return _bounds;
}

/**
 * render(gl, view, projection, fog, light, tick) — draw the current model. `view` is the viewer's
 * orbit view matrix; the model world matrix is composed on top (modelView = view * world) and the
 * pose skin runs on the GPU. The pose is sampled from the embedded standby animation at `tick`,
 * looping over its duration (static bind pose when the model has no embedded animation).
 */
function render(gl, view, projection, fog, light, tick) {
	if (!_submeshes || !_program) {
		return;
	}

	const uniform = _program.uniform;
	gl.useProgram(_program);

	// Bone skin: sample the embedded standby (looping) or the static bind pose.
	let bones;
	if (_animIndex < 0) {
		bones = identityPose(_boneCount);
	} else {
		const t = (tick / 1000) % _duration;
		bones = flattenPose(poseAt(_parsed, _animIndex, t), _boneCount);
	}

	// Per-frame matrices: modelView = view * world, normalMat = inv-transpose of its 3x3.
	mat4.multiply(_mv, view, _world);
	mat4.toInverseMat3(_mv, _nmat);
	mat3.transpose(_nmat, _nmat);

	gl.uniformMatrix4fv(uniform.uModelViewMat, false, _mv);
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
	gl.uniformMatrix3fv(uniform.uNormalMat, false, _nmat);
	gl.uniformMatrix4fv(uniform.uBones, false, bones);

	// Scene light direction (world) -> view space so N.L shares the normal's frame.
	mat3MulVec3(_mv, light.direction, _lightView);
	gl.uniform3fv(uniform.uLightDirection, _lightView);
	gl.uniform1f(uniform.uLightOpacity, light.opacity != null ? light.opacity : 1.0);
	gl.uniform3fv(uniform.uLightAmbient, light.ambient);
	gl.uniform3fv(uniform.uLightDiffuse, light.diffuse);
	gl.uniform3fv(uniform.uLightEnv, light.env);

	gl.uniform1f(uniform.uAlphaRef, ALPHA_REF);
	gl.uniform1f(uniform.uAlpha, 1.0);

	gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
	gl.uniform1f(uniform.uFogNear, fog.near);
	gl.uniform1f(uniform.uFogFar, fog.far);
	gl.uniform3fv(uniform.uFogColor, fog.color);

	gl.activeTexture(gl.TEXTURE0);
	gl.uniform1i(uniform.uDiffuse, 0);

	// Client draws the GR2 opaque with a hard alpha-test — no blend.
	const blendWasEnabled = gl.isEnabled(gl.BLEND);
	gl.disable(gl.BLEND);

	for (let s = 0; s < _submeshes.length; s++) {
		const sm = _submeshes[s];
		gl.bindTexture(gl.TEXTURE_2D, sm.tex);
		gl.bindVertexArray(sm.vao);
		gl.drawElements(gl.TRIANGLES, sm.count, gl.UNSIGNED_SHORT, 0);
	}

	gl.bindVertexArray(null);
	if (blendWasEnabled) {
		gl.enable(gl.BLEND);
	}
}

export default {
	init: init,
	render: render,
	free: free,
	getBounds: getBounds
};
