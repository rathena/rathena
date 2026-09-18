/**
 * Renderer/GR2/GR2ModelRenderer.js
 *
 * GPU-skinned Granny3D (.gr2) model renderer for on-map 3dmob actors (guild flag,
 * Emperium, guardians). Ported from the sandbox robrowser render path
 * (re-ro-clients-asm/sandbox/src/subsystems/gr2/draw-gpu-robrowser.js) to roBrowser
 * conventions, consuming Loaders/GR2Loader.js.
 *
 * Architecture:
 *   - Per-TYPE resource cache (key = gr2 path): one VBO/IBO/VAO/textures/skeleton/parsed
 *     per type, refcounted. N instances carry only { world, actor, animIndex, t }.
 *   - 40 Hz pose cache: poseAt is recomputed on the rag tick, keyed
 *     (path, animIndex, quantize(t, 1/40)). Same-type / same-phase instances collapse to
 *     one poseAt; culled/absent instances never pose.
 *   - Bones: mat4 uBones[48] uniform + one draw per submesh per instance.
 *
 * Wired at the AnimatedModels seam of MapRenderer.onRender, wrapped in
 * SpriteRenderer.runWithDepth(true, true, true). Each instance frustum-culls on its ground anchor
 * before posing (see frustumCullClip in render()). Drive it from the /api.html console via
 * window.RO.load('Renderer/GR2/GR2ModelRenderer'): RO.spawn(path, {pos, dir, action}).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import Client from 'Core/Client.js';
import glMatrix from 'Utils/gl-matrix.js';
import WebGL from 'Utils/WebGL.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Session from 'Engine/SessionStorage.js';
import Altitude from 'Renderer/Map/Altitude.js';
import FlatColorTile from 'Renderer/Effects/FlatColorTile.js';
import GR2Loader from 'Loaders/GR2Loader.js';
import { poseAt, parseAnimated } from 'granny-ro-js/wasm';
import {
	packRobrowserInterleave,
	flattenPose,
	poseCacheKey,
	quantizePoseTime,
	gr2ActionFor,
	frustumCullClip,
	POSE_HZ
} from './gr2Pack.js';
import { buildWorld, computeGroundOffset } from './gr2World.js';
import { createActor, setAction, actorPose, ACTION } from './actorAction.js';
import { bankPathsFor } from './gr2Banks.js';
// The `?raw` shader imports and the `granny-ro-js/wasm` subpath (line 33) resolve in
// production too: the builder (applications/tools/builder-web.mjs) drives Vite's `build()`,
// so it runs the same Rollup pipeline as dev -- `?raw` is a first-class transform (already
// used pervasively for UI .css/.html) and the wasm entry is a plain ESM module with the
// binary base64-inlined (atob + WebAssembly.instantiate, no separate .wasm / fetch). Verified
// via `npm run build`: poseAt + WebAssembly.instantiate land in dist/Web/Online.js.
import _vertexShader from './GR2Model.vs?raw';
import _fragmentShader from './GR2Model.fs?raw';

const mat3 = glMatrix.mat3;
const mat4 = glMatrix.mat4;

/**
 * Phase-0 grayscale directional lighting (gr2-lighting-shadow-mars26.md): intensity
 * 128/255, ambient floor 127/255, no env tint, full directional opacity. No uShadow cell
 * scalar, no shadow blob.
 */
const ALPHA_REF = 207 / 255;
const _phaseDiffuse = new Float32Array([128 / 255, 128 / 255, 128 / 255]);
const _phaseAmbient = new Float32Array([127 / 255, 127 / 255, 127 / 255]);
const _phaseEnv = new Float32Array([1, 1, 1]);

/**
 * Byte-decided per-model lighting (gr2-lighting-shadow-mars26.md §6): the client shades each GR2
 * with a fixed grayscale diffuse/ambient keyed on the model, NOT the map light. Two constant sets
 * (dispatch is exhaustive — flag vs. everything else). Values ÷255 from the client bytes; the
 * Emperium set numerically equals the Phase-0 grays but encodes a distinct byte fact, so it is
 * named on its own. Keyed on the .gr2 basename — dev-spawned instances have no entity, so the
 * basename is the only universal key.
 */
const _gr2FlagDiffuse = new Float32Array([100 / 255, 100 / 255, 100 / 255]); // 0.392
const _gr2EmpDiffuse = new Float32Array([128 / 255, 128 / 255, 128 / 255]); // 0.502
const _gr2EmpAmbient = new Float32Array([127 / 255, 127 / 255, 127 / 255]); // 0.498
const _gr2FlagAmbient = new Float32Array(3); // scratch: per-frame grayscale of the map ambient
// ── Supported GR2 3D roster ──────────────────────────────────────────────────────────────────
// The complete set of .gr2 models the client renders in 3D, keyed by lowercase basename. This is
// the single source of truth: it drives BOTH isSupported() (below) and the per-model lighting
// dispatch in render(). A monster/NPC whose body resolves to a .gr2 NOT listed here (dragon_5,
// Hugeling90_6, or any future model) has no decode/lighting path, so the Entity layer substitutes
// a 2D Poring sprite instead of an invisible actor (see EntityView.UpdateBody -> isSupported).
//
// To add a model: add ONE line below with its lighting profile. That single edit enrols it in the
// 3D path (isSupported turns true) AND selects its shading (render() reads the profile value).
//   'flag' -> ambient runtime-tinted off the map light (guild flag only)
//   'emp'  -> fixed byte-decided grayscale diffuse/ambient (gr2-lighting-shadow-mars26.md §6)
//   'map'  -> plain map light (default profile; no model uses it yet)
const GR2_ROSTER = {
	guildflag90_1: 'flag', // Guild flag
	empelium90_0: 'emp', // Emperium
	kguardian90_7: 'emp', // Knight guardian
	aguardian90_8: 'emp', // Archer guardian
	sguardian90_9: 'emp', // Soldier guardian
	treasurebox_2: 'emp' // Treasure box
};

// Lowercased .gr2 basename (no dir, no extension) — the per-model lighting key.
function gr2Basename(path) {
	const cut = Math.max(path.lastIndexOf('/'), path.lastIndexOf('\\'));
	const dot = path.lastIndexOf('.');
	return path.slice(cut + 1, dot > cut ? dot : path.length).toLowerCase();
}

// Collapse a map-light color to one grayscale scalar (Rec.601 luma), broadcast R=G=B. Stand-in for
// the flag's runtime scene-global ambient (0xef90f4, no static byte). VISUAL-UNVERIFIED — swap to a
// flat average if the in-engine A/B wants it flatter.
function grayBroadcast(src, out) {
	const g = 0.299 * src[0] + 0.587 * src[1] + 0.114 * src[2];
	out[0] = out[1] = out[2] = g;
	return out;
}

let _program = null;

// GL context cached from render() so detach() can free per-instance emblem textures off-frame.
// Safe to cache: roBrowser uses one persistent WebGL context, _gl is re-assigned every render()
// (never nulled), and if the context is ever lost the engine pauses rendering (Renderer.onContextLost)
// -- a deleteTexture on a lost context is a spec no-op and the GPU has already freed everything, so
// a "stale" _gl can neither crash nor leak. Not a dangling-context hazard.
let _gl = null;

// Per-type resource cache (key = gr2 path).
let _types = {};

// Negative cache: gr2 paths that 404'd on fetch (absent from the GRF). Populated in acquire's
// onError; consumed by isMissing() so the Entity path falls back to a Poring sprite instead of
// rendering nothing. Session-lifetime on purpose -- a missing model stays missing across maps,
// so free() deliberately does NOT clear it (no re-request, no re-attempt of the dead 3D path).
// Bounded, not a growing leak: acquire() only ever receives ROSTERED paths (UpdateBody gates on
// isSupported before this.gr2 is set), so this holds at most one key per roster entry.
const _missing = {};

// Live instances: { path, world, worldBuilt, actor, dir, pos, standbyIdx, animIndex, t,
// _lastAction, _lastTick, screenRect, emblemTex, _emblemImg } (+ entity, attached on bind).
let _instances = [];

// Pose cache — set each frame to only the poses used that frame (bounds memory, keeps the
// 40 Hz reuse across rAF frames when the quantized bucket is unchanged).
let _poseCache = {};

// Debug aid: highlight each instance's anchor CELL with a flat tile. The tile marks the cell
// CENTER (its shader adds +0.5) — the SAME cell centre the model's origin now lands on (worldCore
// adds RB_PLACEMENT_OFS = +0.5), so tile and pole should COINCIDE; a visible offset between them
// means a placement regression. Toggle from the dev console (`mod.debugCell = true`); off by
// default, no gameplay effect.
const _dbgCellTile = FlatColorTile('gr2_debug_cell', { r: 0.0, g: 1.0, b: 1.0, a: 0.45 });
const _dbgTileInst = new _dbgCellTile([0, 0, 0]);
let _dbgCellInited = false;
let _debugCell = false;

// GR2 pick-box geometry — TWO modes, switched at the call site in render() by commenting (no
// runtime flag). The client hit-tests a base bounding-SPHERE, NOT the model AABB (fcn.006852e0):
//   sphere centre (0,0,0) = model base, radius 10.0 client-u (fcn.006800d0.c:202-206).
//   10 client-u ÷ 5 (roBrowser /5 referential, gr2World.RB_UNIT_SCALE) = 2.0 world-u (cells).
//   Half-extent = √(r²·0.5) — the client's projected corner offset (fcn.006852e0.c:70-71).
// These constants feed computeBaseSphereRect (the "sphere" mode), which is the shipped DEFAULT.
// The alternative is the full model AABB ("tout le drapeau" — whole model hoverable, more generous
// than the client), switched in at the pick site by commenting.
// VISUAL-UNVERIFIED: the world-unit radius is a single-constant calibration point (visual gate).
const BASE_SPHERE_RADIUS = 2.0;
const BASE_SPHERE_HALF_EXTENT = Math.sqrt(BASE_SPHERE_RADIUS * BASE_SPHERE_RADIUS * 0.5);

// granny-ro-js WASM init (texture pixel decode); shared across all types.
let _readyPromise = null;

// Scratchpads to evade the GC.
const _mv = mat4.create();
const _mvp = mat4.create();
const _nmat = mat3.create();
const _lightView = new Float32Array(3);
const _clip = new Float32Array(4);

// Frustum-cull margin (NDC), padded so a model straddling the screen edge is not clipped
// by its ground anchor alone.
const CULL_MARGIN = 0.2;

// Clip-w floor: a projected point with w <= this is on/behind the camera plane, skip it.
const CLIP_W_EPS = 1e-6;

// RO direction is an 8-way index (0-7); the world yaw wants degrees (45 deg per step).
const DIR_STEP_DEG = 45;

// Removal-fade toggles — both OFF by default = FAITHFUL. For a genuine 3D (.gr2) actor the
// official mars26 client instantiates NEITHER the out-of-sight alpha fade NOR the death corpse
// (CCorpse) — both are 2D-sprite-only; a genuine 3D actor hard-POPS on removal (verified by RE
// on mars26; no alpha fade-in on spawn either). Our GR2 roster (guardians, Emperium, guild
// flag, treasure box) IS that 3D set, so popping is correct. Set a toggle true to A/B a
// roBrowser-parity fade over remove_delay instead (NON-authentic). Exported -> runtime-mutable:
// RO.load('Renderer/GR2/GR2ModelRenderer').FADE.death.
//   death  -> the DIE removal        vanish -> the out-of-sight removal
const FADE = { death: false, vanish: false };

/**
 * Initialize the shader program.
 */
function init(gl) {
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
 * Free all type GL resources and drop every instance (MapRenderer calls this on map unload).
 */
function free(gl) {
	for (const path in _types) {
		const type = _types[path];
		const submeshes = type.submeshes;
		if (submeshes) {
			for (let s = 0; s < submeshes.length; s++) {
				const sm = submeshes[s];
				gl.deleteVertexArray(sm.vao);
				gl.deleteBuffer(sm.vbo);
				gl.deleteBuffer(sm.ibo);
			}
		}
		if (type.textures) {
			for (const file in type.textures.byFile) {
				gl.deleteTexture(type.textures.byFile[file]);
			}
		}
	}
	// Per-instance (per-guild) emblem textures live on the instance, not the type cache.
	for (let i = 0; i < _instances.length; i++) {
		if (_instances[i].emblemTex) {
			gl.deleteTexture(_instances[i].emblemTex);
		}
	}
	_types = {};
	_instances = [];
	_poseCache = {};
	// _program and _gl are intentionally kept (not deleted/nulled): roBrowser uses one persistent
	// WebGL context across map loads, so the compiled program stays valid and render() reuses it via
	// the `if (!_program)` guard -- re-init only on a genuinely fresh context. Same pattern as
	// AnimatedModels.js free(). If context loss/restore ever recreates the GL context, both would
	// need to reset _program here.
}

/**
 * keyGr2Magenta(px) — ASM-faithful gr2 texture transparency. The client keys MAGENTA
 * 0xFF00FF by zeroing the whole pixel to black + alpha 0 (mars26 fcn.0054e950 / ver12
 * fcn.00417550). Discarded before the RGB contributes (opaque + hard alpha-test at 207).
 * A 32-bit source with its own alpha is honoured verbatim.
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

// 1x1 fallback pixels. A granny texture that decoded no pixels -> a neutral grey placeholder;
// a submesh whose texFile matches no embedded texture -> the shared __grey stand-in.
const TEX_MISSING_PX = new Uint8Array([200, 200, 200, 255]);
const TEX_GREY_PX = new Uint8Array([180, 180, 185, 255]);

/**
 * makeTypeTextures(gl, parsed) -> { byFile:{ textureFile -> GLTexture } }. One texture per
 * embedded granny texture, keyed by its file name (the string a packed mesh carries in
 * texFile). LINEAR + CLAMP = the client's active sampler 0 (byte-cited).
 */
function makeTypeTextures(gl, parsed) {
	const byFile = {};
	const list = parsed.textures || [];
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

// A4R4G4B4 4-bit quantize — the client's on-load emblem format (byte-cited fcn.0054e1d0 class-1).
// Drop each channel to 4 bits then replicate the nibble across the byte (nibble ×0x11: 0..15 -> 0..255).
const A4_NIBBLE_EXPAND = 0x11;
function quantize4bit(px) {
	for (let i = 0; i < px.length; i++) {
		px[i] = (px[i] >> 4) * A4_NIBBLE_EXPAND;
	}
}

// Module-scope scratch canvas reused across emblem uploads (guild/version changes are frequent);
// created lazily and re-cleared per call by re-setting its size, so no per-change allocation.
let _emblemCanvas = null;

/**
 * buildEmblemTexture(gl, img, prevTex) — the live per-guild emblem, POT-padded the way the client
 * does it. The client requests the emblem 24x24 then rounds it up to the next power-of-two
 * (32x32, fcn.00529000) with the pixels in the TOP-LEFT corner and the rest transparent, so
 * UV 0..1 sample the whole 32x32 and the emblem reads at 75% top-left of the Plane01 quad
 * (~centered, upper third of the banner). The placement IS the POT padding — no geometry recentre.
 * The transparent [24..31] border is discarded by the shader's hard alpha-test (ALPHA_REF). The
 * source already passed Texture.load (magenta keyed) in Guild.js, so only the A4R4G4B4 quantize is
 * applied here. Reuses prevTex on a guild/version change so a re-bind never leaks. LINEAR + CLAMP
 * = the client's sampler (byte-cited).
 */
function buildEmblemTexture(gl, img, prevTex) {
	const POT = 32,
		SRC = 24;
	const cv = _emblemCanvas || (_emblemCanvas = document.createElement('canvas'));
	// Re-setting the size clears the canvas so a smaller/transparent emblem never leaks prior pixels.
	cv.width = POT;
	cv.height = POT;
	const ctx = cv.getContext('2d', { willReadFrequently: true });
	ctx.imageSmoothingEnabled = false;
	ctx.drawImage(img, 0, 0, SRC, SRC);
	const im = ctx.getImageData(0, 0, POT, POT);
	quantize4bit(im.data);

	const tex = prevTex || gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, im);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
	gl.bindTexture(gl.TEXTURE_2D, null);
	return tex;
}

/**
 * acquire(path) — refcount a type; on first acquire fetch the .gr2 (worker Thread ->
 * cyro source) and decode it on the CPU (GL upload is deferred to the first render that
 * has a gl context). Returns the type entry immediately (empty until decode completes).
 */
function acquire(path) {
	let type = _types[path];
	if (type) {
		type.refcount++;
		return type;
	}

	const declaredBanks = bankPathsFor(path); // [{ name, path }]
	type = {
		path: path,
		refcount: 1,
		cpuReady: false,
		glReady: false,
		parsed: null,
		meshes: null,
		ipRow: undefined,
		boneCount: 0,
		duration: 1,
		submeshes: null,
		textures: null,
		// External animation banks (data/model/3dmob_bone/<setId>_<suffix>.gr2) loaded
		// lazily, prioritised by the current action (see pumpBanks). The main .gr2 gives
		// the mesh + embedded standby; move/attack/dead/damage stream in on demand.
		declaredBanks: declaredBanks,
		bankQueue: declaredBanks.map(b => b.name),
		bankState: {}, // name -> 'loading' | 'loaded'
		bankLoading: false
	};
	_types[path] = type;

	if (!_readyPromise) {
		_readyPromise = GR2Loader.ready();
	}
	Client.getFile(
		path,
		function (buffer) {
			_readyPromise.then(function () {
				// The type may have been freed before the async decode returned.
				if (_types[path] !== type) {
					return;
				}
				try {
					const loader = new GR2Loader(buffer);
					type.parsed = loader.parsed;
					type.meshes = loader.meshes;
					// ipRow BEFORE groundOffset: grounding must run in the model's real draw frame
					// (v . ipRow . worldCore) — a non-identity IP (treasurebox 90deg rot, guardian Y
					// translation) else grounds in the wrong frame and the model floats/sinks/lies flat.
					type.ipRow = loader.ipRow;
					type.groundOffset = computeGroundOffset(type.meshes, type.ipRow); // drop the base onto the terrain
					type.aabb = computeLocalAABB(type.meshes); // local bounds for the screen-space pick box
					type.boneCount = loader.boneCount;
					type.duration = loader.duration;
					type.cpuReady = true;
				} catch (e) {
					// Decode threw (unexpected/corrupt buffer): same fate as a 404 -- flag missing so the
					// Entity path reverts to a Poring sprite instead of an invisible actor, and no re-attempt.
					// A defensive net only; the rostered models are expected to decode cleanly.
					_missing[path] = true;
					console.error('[GR2ModelRenderer] decode failed', path, e);
				}
			});
		},
		function () {
			// Absent from the GRF -> flag it so the Entity path substitutes a Poring sprite
			// (see EntityView.UpdateBody / EntityRender.render). No retry: the flag is permanent.
			_missing[path] = true;
			console.warn('[GR2ModelRenderer] failed to fetch', path);
		}
	);
	return type;
}

// Interleaved vertex layout — MUST mirror gr2Pack.js packRobrowserInterleave (16 floats/vertex,
// 64-byte stride): position(3), normal(3), uv(2), boneIndex(4), boneWeight(4). Byte offsets are the
// float offsets (0/3/6/8/12) x4.
const GR2_VERTEX_STRIDE = 64;
const GR2_VERTEX_LAYOUT = [
	{ attr: 'aPosition', size: 3, offset: 0 },
	{ attr: 'aNormal', size: 3, offset: 12 },
	{ attr: 'aTextureCoord', size: 2, offset: 24 },
	{ attr: 'aBoneIndex', size: 4, offset: 32 },
	{ attr: 'aBoneWeight', size: 4, offset: 48 }
];

/**
 * buildTypeGL(gl, type) — upload the type's VBO/IBO/VAO per submesh + textures, once the
 * CPU decode is done. One VAO per submesh, 16-float interleave (see GR2_VERTEX_LAYOUT).
 */
function buildTypeGL(gl, type) {
	type.textures = makeTypeTextures(gl, type.parsed);
	const attr = _program.attribute;
	type.submeshes = type.meshes.map(function (mesh) {
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
			tex: type.textures.byFile[mesh.texFile] || type.textures.byFile.__grey,
			emblem: mesh.emblem // the emblem submesh (Plane01) binds the live per-instance guild emblem
		};
	});
	// Whether this type carries an emblem submesh (guild flag) — gates the per-instance emblem
	// rebuild in render() so non-flag types skip it.
	type.hasEmblem = type.submeshes.some(function (sm) {
		return sm.emblem;
	});
	type.glReady = true;
}

function mat3MulVec3(m, v, out) {
	out[0] = m[0] * v[0] + m[4] * v[1] + m[8] * v[2];
	out[1] = m[1] * v[0] + m[5] * v[1] + m[9] * v[2];
	out[2] = m[2] * v[0] + m[6] * v[1] + m[10] * v[2];
}

/**
 * computeLocalAABB(meshes) -> the model's local bind-pose axis-aligned bounds { min, max }, or
 * null if empty. Used to project a screen-space pick box (see computeScreenRect) — the GR2 path
 * suppresses the 2D sprite body, which is what normally fills entity.boundingRect.
 */
function computeLocalAABB(meshes) {
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
		return null;
	}
	return { min: [minX, minY, minZ], max: [maxX, maxY, maxZ] };
}

/**
 * computeScreenRect(aabb, mvp, out) -> project the 8 AABB corners through mvp to a screen-space
 * rect (window pixels, y-down: x1/y1 = min, x2/y2 = max), written into `out`. Returns false when
 * every corner is behind the camera. Mirrors EntityRender.calculateBoundingRect's NDC->pixel map
 * so the projected box lines up with EntityManager's mouse-space pick test.
 *
 * Intentionally kept even though the shipped default calls computeBaseSphereRect: it is the
 * "tout le drapeau" mode (A) at the pick site, activated by commenting there. See render().
 */
// eslint-disable-next-line no-unused-vars
function computeScreenRect(aabb, mvp, out) {
	const min = aabb.min;
	const max = aabb.max;
	const halfW = window.innerWidth * 0.5;
	const halfH = window.innerHeight * 0.5;
	let minX = Infinity,
		minY = Infinity,
		maxX = -Infinity,
		maxY = -Infinity;
	let any = false;
	for (let i = 0; i < 8; i++) {
		const x = i & 1 ? max[0] : min[0];
		const y = i & 2 ? max[1] : min[1];
		const z = i & 4 ? max[2] : min[2];
		const cw = mvp[3] * x + mvp[7] * y + mvp[11] * z + mvp[15];
		if (cw <= CLIP_W_EPS) {
			continue;
		}
		const inv = 1 / cw;
		const sx = halfW * (1 + (mvp[0] * x + mvp[4] * y + mvp[8] * z + mvp[12]) * inv);
		const sy = halfH * (1 - (mvp[1] * x + mvp[5] * y + mvp[9] * z + mvp[13]) * inv);
		if (sx < minX) minX = sx;
		if (sx > maxX) maxX = sx;
		if (sy < minY) minY = sy;
		if (sy > maxY) maxY = sy;
		any = true;
	}
	if (!any) {
		return false;
	}
	out.x1 = minX;
	out.y1 = minY;
	out.x2 = maxX;
	out.y2 = maxY;
	return true;
}

/**
 * computeBaseSphereRect(mvp, projection, out) -> the client's GR2 pick box: the screen rect of a
 * base bounding-sphere (fcn.006852e0), written into `out` (same {x1,y1,x2,y2} shape and
 * window-pixel/y-down convention computeScreenRect produces, so EntityRender consumes it
 * unchanged). Returns false when the base is behind the camera.
 *
 * Centre = the model origin (0,0,0) projected through the FULL model→clip matrix = the mvp
 * translation column (mvp[12], mvp[13], w = mvp[15]). This is the model's base exactly as drawn —
 * it carries the cell-centre +0.5, the groundOffset and the ipRow — matching fcn.006852e0.c:50-57,
 * which transforms the stored centre (sub+0x394 = (0,0,0)) by the model's own matrices. Do NOT use
 * the raw cell anchor (worldAnchorToClip): it misses the +0.5/ground and lands half a cell off.
 *
 * Half-extent = √(r²·0.5) world-u (BASE_SPHERE_HALF_EXTENT), projected `·focal·(1/w)` symmetrically
 * around the centre (fcn.006852e0.c:70-71,75-81), where roBrowser's `focal` is `halfW·projection[0]`
 * / `halfH·projection[5]` (the viewport scales). The radius is a world/view extent, independent of
 * the model's own geometry scale — same as the client, which adds the raw radius in view space.
 */
function computeBaseSphereRect(mvp, projection, out) {
	const cw = mvp[15];
	if (cw <= CLIP_W_EPS) {
		return false;
	}
	const inv = 1 / cw;
	const halfW = window.innerWidth * 0.5;
	const halfH = window.innerHeight * 0.5;
	const cx = halfW * (1 + mvp[12] * inv);
	const cy = halfH * (1 - mvp[13] * inv);
	const ex = halfW * projection[0] * BASE_SPHERE_HALF_EXTENT * inv;
	const ey = halfH * projection[5] * BASE_SPHERE_HALF_EXTENT * inv;
	out.x1 = cx - ex;
	out.x2 = cx + ex;
	out.y1 = cy - ey;
	out.y2 = cy + ey;
	return true;
}

function normalize3(v) {
	const len = Math.hypot(v[0], v[1], v[2]) || 1;
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}

/**
 * Render every live GR2 instance. Signature mirrors AnimatedModels.render (the seam it
 * sits next to): (gl, modelView, projection, normalMat, fog, light, tick). modelView is
 * Camera.modelView (the view matrix); the per-instance world is composed on top.
 */
function render(gl, modelView, projection, normalMat, fog, light, tick) {
	_gl = gl;
	if (_instances.length === 0 || !light) {
		return;
	}
	if (!_program) {
		init(gl);
	}

	// Build GL resources for any type whose CPU decode has finished.
	for (const path in _types) {
		const type = _types[path];
		if (type.cpuReady && !type.glReady) {
			buildTypeGL(gl, type);
		}
	}

	const uniform = _program.uniform;
	gl.useProgram(_program);

	// Per-frame constant uniforms.
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
	// Opacity and env tint come from the map's own light (parity with Models.js / AnimatedModels.js
	// — the GR2 .fs math is byte-identical). uLightDiffuse / uLightAmbient are byte-decided per model
	// and set inside the instance loop below. Falls back to the Phase-0 baked grays if a map omits a
	// field (dev spawn always has light; the render early-returns without it).
	gl.uniform1f(uniform.uLightOpacity, light.opacity != null ? light.opacity : 1.0);
	gl.uniform3fv(uniform.uLightEnv, light.env || _phaseEnv);
	gl.uniform1f(uniform.uAlphaRef, ALPHA_REF);
	gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
	gl.uniform1f(uniform.uFogNear, fog.near);
	gl.uniform1f(uniform.uFogFar, fog.far);
	gl.uniform3fv(uniform.uFogColor, fog.color);
	gl.activeTexture(gl.TEXTURE0);
	gl.uniform1i(uniform.uDiffuse, 0);

	// Opaque device state — no blend (client draws the GR2 opaque with a hard alpha-test). Saved
	// and restored so the later transparent passes (entity shadow, water, effects) keep their blend.
	const blendWasEnabled = gl.isEnabled(gl.BLEND);
	gl.disable(gl.BLEND);

	// Scene light direction (world) -> view space, so N.L shares the normal's frame.
	mat3MulVec3(modelView, light.direction, _lightView);
	normalize3(_lightView);
	gl.uniform3fv(uniform.uLightDirection, _lightView);

	const seen = {};
	SpriteRenderer.runWithDepth(true, true, true, function () {
		for (let i = 0; i < _instances.length; i++) {
			const inst = _instances[i];
			const type = _types[inst.path];
			if (!type || !type.glReady) {
				continue;
			}

			// Entity-backed instance: pull pos/dir/action from the live entity (1-frame lag —
			// this pass runs before EntityManager.render) and prioritise its bank load.
			if (inst.entity) {
				syncFromEntity(inst, type, tick);

				// Live guild emblem: bind the entity's fetched emblem image (Guild.requestGuildEmblem,
				// triggered on spawn for any GUID) as a per-instance POT texture. Per-instance, not
				// per-type — two flags of different guilds must show different emblems. Rebuild only when
				// the image reference changes (new guild / GEmblemVer → new image object), so the identity
				// compare is the whole per-frame cost.
				if (type.hasEmblem) {
					const emblemImg = inst.entity.emblem && inst.entity.emblem.emblem;
					if (emblemImg !== inst._emblemImg) {
						inst._emblemImg = emblemImg;
						if (emblemImg) {
							inst.emblemTex = buildEmblemTexture(gl, emblemImg, inst.emblemTex);
						} else if (inst.emblemTex) {
							gl.deleteTexture(inst.emblemTex);
							inst.emblemTex = null;
						}
					}
				}
			}

			// Frustum cull BEFORE posing — a culled instance neither poses nor draws. The anchor
			// negates the height to match buildWorld's world-Y (-position[2]).
			worldAnchorToClip(inst.pos, modelView, projection, _clip);
			if (frustumCullClip(_clip[0], _clip[1], _clip[3], CULL_MARGIN)) {
				// Off-screen: drop the stale pick box so the hover test can't match it (the entity
				// falls back to EntityRender's default box, which is off-screen anyway).
				inst.screenRect = null;
				continue;
			}

			// Build the instance world matrix once the type (its ipRow) is known. Entity
			// positions are [cellX, cellY, position[2]] where position[2] = getCellHeight (a
			// NEGATED terrain height). roBrowser's world-Y (up) is -position[2] (see the 2D sprite
			// Project(): `y = -pos.z`), so negate p[2] into buildWorld's up axis — else the
			// placement error is proportional to the terrain height (flat OK, hills sink/vanish).
			if (!inst.worldBuilt) {
				const p = inst.pos;
				inst.world = buildWorld(inst.dir * DIR_STEP_DEG, [p[0], -p[2], p[1]], type.ipRow, {
					heightOffset: type.groundOffset
				});
				inst.worldBuilt = true;
			}

			// Advance the action clock, then resolve/cache the pose at 40 Hz.
			const pose = actorPose(inst.actor, type, tick, inst.standbyIdx, null);
			const animIndex = pose.animIndex;
			const t = pose.t;
			inst.animIndex = animIndex;
			inst.t = t;

			const key = poseCacheKey(inst.path, animIndex, quantizePoseTime(t));
			let bones = seen[key] || _poseCache[key];
			if (!bones) {
				const idx = animIndex < 0 ? -1 : animIndex;
				const sampleT = animIndex < 0 ? 0 : t;
				bones = flattenPose(poseAt(type.parsed, idx, sampleT), type.boneCount);
			}
			seen[key] = bones;

			// Per-instance matrices: modelView = view * world, normalMat = inv-transpose of its 3x3.
			mat4.multiply(_mv, modelView, inst.world);
			mat4.toInverseMat3(_mv, _nmat);
			mat3.transpose(_nmat, _nmat);
			gl.uniformMatrix4fv(uniform.uModelViewMat, false, _mv);
			gl.uniformMatrix3fv(uniform.uNormalMat, false, _nmat);
			gl.uniformMatrix4fv(uniform.uBones, false, bones);

			// Per-model lighting constants (gr2-lighting-shadow-mars26.md §6): the client shades each
			// GR2 by a byte-decided grayscale diffuse/ambient, not the map light. Re-issued every
			// iteration because GL uniforms are stateful — a fixed-constant model must not leak its
			// diffuse/ambient onto the next instance.
			const profile = GR2_ROSTER[gr2Basename(inst.path)];
			if (profile === 'flag') {
				gl.uniform3fv(uniform.uLightDiffuse, _gr2FlagDiffuse);
				gl.uniform3fv(uniform.uLightAmbient, grayBroadcast(light.ambient || _phaseAmbient, _gr2FlagAmbient));
			} else if (profile === 'emp') {
				gl.uniform3fv(uniform.uLightDiffuse, _gr2EmpDiffuse);
				gl.uniform3fv(uniform.uLightAmbient, _gr2EmpAmbient);
			} else {
				gl.uniform3fv(uniform.uLightDiffuse, light.diffuse || _phaseDiffuse);
				gl.uniform3fv(uniform.uLightAmbient, light.ambient || _phaseAmbient);
			}

			// Project a screen-space pick box for EntityManager's hover/click test. Stashed on the
			// instance (not entity.boundingRect) because Entity.render() resets that rect after this
			// map pass; EntityRender.calculateBoundingRect copies it.
			//
			// TWO faithful-to-different-things modes — switch by commenting (no runtime flag):
			//  (A) "tout le drapeau": the full model AABB projected. The WHOLE model is hoverable —
			//      more generous (better UX) than the client, but a divergence.
			//  (B) DEFAULT — "sphere", client-faithful (fcn.006852e0): a coarse square at the model
			//      base only (computeBaseSphereRect). On the tall flag just the lower ~46% is
			//      clickable. To use (A) instead, comment (B) and uncomment (A).
			if (inst.entity && type.aabb) {
				const box = inst.screenRect || (inst.screenRect = { x1: 0, y1: 0, x2: 0, y2: 0 });
				mat4.multiply(_mvp, projection, _mv);
				// const ok = computeScreenRect(type.aabb, _mvp, box); // (A) full model AABB
				const ok = computeBaseSphereRect(_mvp, projection, box); // (B) client base sphere
				if (!ok) {
					inst.screenRect = null;
				}
			}

			// Removal fade: honour the entity's current alpha — effectColor[3] scaled by the
			// remove_tick ramp, the SAME fade a 2D body applies (EntityRender.renderLayer) — so a
			// GR2 mob/NPC fades out on death/vanish instead of hard-popping. Opaque instances
			// (alpha 1) stay on the no-blend fast path; only a fading instance flips blend on and
			// back off. The outer save/restore guards the BLEND enable bit; the blend func it may
			// switch is reset to the resting default at pass end (see below).
			let alpha = 1.0;
			const e = inst.entity;
			if (e) {
				alpha = e.effectColor[3];
				if (e.remove_tick && e.remove_delay) {
					const isDeath = e.action === e.ACTION.DIE;
					if (isDeath ? FADE.death : FADE.vanish) {
						alpha *= 1 - (Date.now() - e.remove_tick) / e.remove_delay;
					}
				}
				alpha = alpha < 0 ? 0 : alpha > 1 ? 1 : alpha;
			}
			gl.uniform1f(uniform.uAlpha, alpha);
			const fading = alpha < 1;
			if (fading) {
				gl.enable(gl.BLEND);
				gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
			}

			const submeshes = type.submeshes;
			for (let s = 0; s < submeshes.length; s++) {
				const sm = submeshes[s];
				// The emblem submesh (Plane01) stays empty until the live guild emblem loads
				// (Guild.requestGuildEmblem): skip it when there is no per-instance emblem texture —
				// a dev spawn with no entity, or a flag whose emblem hasn't arrived. Every other
				// submesh uses its baked texture.
				if (sm.emblem) {
					if (!inst.emblemTex) {
						continue;
					}
					gl.bindTexture(gl.TEXTURE_2D, inst.emblemTex);
				} else {
					gl.bindTexture(gl.TEXTURE_2D, sm.tex);
				}
				gl.bindVertexArray(sm.vao);
				gl.drawElements(gl.TRIANGLES, sm.count, gl.UNSIGNED_SHORT, 0);
			}

			if (fading) {
				gl.disable(gl.BLEND);
			}
		}
	});
	gl.bindVertexArray(null);
	if (blendWasEnabled) {
		gl.enable(gl.BLEND);
	}
	// House convention: leave the blend func at roBrowser's resting default (every effect
	// restores this on exit) — a fading instance above may have switched it.
	gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	_poseCache = seen;

	// Debug: paint the anchor cell of every instance (see _debugCell above).
	if (_debugCell && _instances.length) {
		if (!_dbgCellInited) {
			_dbgCellTile.init(gl);
			_dbgCellInited = true;
		}
		const dbgBlendWasEnabled = gl.isEnabled(gl.BLEND);
		gl.enable(gl.BLEND);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		_dbgCellTile.beforeRender(gl, modelView, projection, fog, tick);
		for (let i = 0; i < _instances.length; i++) {
			const inst = _instances[i];
			const type = _types[inst.path];
			if (!type || !type.glReady) {
				continue;
			}
			_dbgTileInst.position = inst.pos;
			_dbgTileInst.render(gl, tick);
		}
		_dbgCellTile.afterRender(gl);
		if (!dbgBlendWasEnabled) {
			gl.disable(gl.BLEND);
		}
	}
}

// worldAnchorToClip(pos, modelView, projection, out4): project the instance ground anchor to
// clip space for the frustum cull. Mirrors buildWorld's frame: world-X = cellX, world-Y (up) =
// -position[2] (the negated terrain height), world-Z = cellY.
function worldAnchorToClip(pos, modelView, projection, out) {
	const x = pos[0];
	const y = -pos[2];
	const z = pos[1];
	const vx = modelView[0] * x + modelView[4] * y + modelView[8] * z + modelView[12];
	const vy = modelView[1] * x + modelView[5] * y + modelView[9] * z + modelView[13];
	const vz = modelView[2] * x + modelView[6] * y + modelView[10] * z + modelView[14];
	const vw = modelView[3] * x + modelView[7] * y + modelView[11] * z + modelView[15];
	out[0] = projection[0] * vx + projection[4] * vy + projection[8] * vz + projection[12] * vw;
	out[1] = projection[1] * vx + projection[5] * vy + projection[9] * vz + projection[13] * vw;
	out[2] = projection[2] * vx + projection[6] * vy + projection[10] * vz + projection[14] * vw;
	out[3] = projection[3] * vx + projection[7] * vy + projection[11] * vz + projection[15] * vw;
}

/**
 * pumpBanks(type, priorityName): per-type sequential loader for the external animation
 * banks. The current action's bank (priorityName) jumps to the front of the queue; one bank
 * is fetched at a time and grafted into the shared type.parsed.animations at its action slot
 * (loaded once for every instance of that type). A 404 drops the name (stays bind pose).
 */
function pumpBanks(type, priorityName) {
	const queue = type.bankQueue;

	// Re-prioritise: the current action's bank jumps the queue ("change la priorité au besoin").
	if (priorityName) {
		const at = queue.indexOf(priorityName);
		if (at > 0) {
			queue.splice(at, 1);
			queue.unshift(priorityName);
		}
	}

	if (type.bankLoading || queue.length === 0) {
		return;
	}

	const name = queue.shift();
	const entry = type.declaredBanks.find(b => b.name === name);
	if (!entry) {
		return;
	}
	type.bankLoading = true;
	type.bankState[name] = 'loading';
	Client.getFile(
		entry.path,
		function (buffer) {
			type.bankLoading = false;
			// The type may have been freed during the async fetch.
			if (_types[type.path] === type && type.parsed) {
				const bank = parseAnimated(new Uint8Array(buffer));
				type.parsed.animations[ACTION[name].idx] = bank.animations[0];
				type.bankState[name] = 'loaded';
			}
			pumpBanks(type);
		},
		function () {
			type.bankLoading = false;
			delete type.bankState[name];
			pumpBanks(type);
		}
	);
}

// syncFromEntity(inst, type, tick): pull the live entity's pos/dir/action onto the instance
// each frame. NPC instances (standbyIdx < 0) are never action-driven (static bind pose); MOB
// instances map their action enum to a GR2 action and priority-load that bank.
function syncFromEntity(inst, type, tick) {
	const e = inst.entity;
	const p = e.position;
	const dir = e.direction;
	// Rebuild the world matrix on any pose change. p[2] IS the terrain cell height
	// (position[2] = Altitude.getCellHeight, see attach), so a terrain-height change is
	// already covered here -- it's part of the position tuple, not a missing trigger.
	if (inst.pos[0] !== p[0] || inst.pos[1] !== p[1] || inst.pos[2] !== p[2] || inst.dir !== dir) {
		inst.pos[0] = p[0];
		inst.pos[1] = p[1];
		inst.pos[2] = p[2];
		inst.dir = dir;
		inst.worldBuilt = false;
	}

	if (inst.standbyIdx < 0) {
		return;
	}

	const name = gr2ActionFor(e);
	pumpBanks(type, name);
	// Re-cut on a name change OR a re-issued action with the same name. The client restarts the
	// clip on every ZC_NOTIFY_ACT (SetAction = hard clock reset, no same-action dedup); a name-only
	// guard latched one-shot actions — an immobile plant re-hit stays entity.action=HURT, so the
	// hurt clip plays once and never replays. The entity bumps animation.tick on every setAction()
	// (EntityAction), so a changed tick is the faithful "re-issued this frame" signal.
	const animTick = e.animation.tick;
	if (name !== inst._lastAction || animTick !== inst._lastTick) {
		setAction(inst.actor, name, tick);
		inst.actor.loop = name === 'move';
		inst._lastAction = name;
		inst._lastTick = animTick;
	}
}

/**
 * attach(entity): create a GR2 instance bound to a mob/NPC entity. standbyIdx is classified
 * by objecttype — NPC -> -1 (static poseAt(-1); SetAction never fires) vs MOB -> 0 (animated
 * idx-0 standby). The renderer pulls pos/dir/action from the entity each frame (syncFromEntity).
 */
function attach(entity) {
	acquire(entity.gr2);
	const p = entity.position;
	const isNpc = entity.objecttype === entity.constructor.TYPE_NPC;
	const inst = {
		entity: entity,
		path: entity.gr2,
		world: null,
		worldBuilt: false,
		actor: createActor(),
		dir: entity.direction,
		pos: [p[0], p[1], p[2]],
		standbyIdx: isNpc ? -1 : 0,
		animIndex: 0,
		t: 0,
		_lastAction: 'stand',
		_lastTick: 0,
		screenRect: null,
		// Per-instance (per-guild) emblem texture + the image it was built from, for change detection.
		emblemTex: null,
		_emblemImg: null
	};
	_instances.push(inst);
	return inst;
}

/**
 * detach(inst): drop an entity's GR2 instance (called from EntityManager on removal). The type
 * GL resources stay cached until map unload (free); the type set is bounded, so no leak.
 */
function detach(inst) {
	const idx = _instances.indexOf(inst);
	if (idx !== -1) {
		_instances.splice(idx, 1);
	}
	const type = _types[inst.path];
	if (type && type.refcount > 0) {
		type.refcount--;
	}
	// Free this instance's per-guild emblem texture (the per-type cache is freed at map unload).
	// Invariant (no leak on early detach): emblemTex is assigned ONLY inside render(), after _gl is
	// set, and _gl is never nulled -- so emblemTex != null implies _gl != null. A spawn-then-detach
	// before the first render leaves emblemTex null (nothing allocated), so the guard frees exactly
	// what was created. The `&& _gl` is defensive, never the reason a real texture is skipped.
	if (inst.emblemTex && _gl) {
		_gl.deleteTexture(inst.emblemTex);
		inst.emblemTex = null;
	}
	inst.entity = null;
}

/**
 * spawn(path, opts) — dev harness: place a GR2 instance. opts = { pos, dir, action,
 * standbyIdx }; pos/dir default to the player's position/facing. Returns the instance.
 */
function spawn(path, opts) {
	const o = opts || {};
	const ent = Session.Entity;
	const pos = o.pos || (ent ? [ent.position[0], ent.position[1], ent.position[2]] : [0, 0, 0]);
	const dir = o.dir != null ? o.dir : ent ? ent.direction : 0;
	// Ground the base on the REAL terrain height at the target cell (what a real entity carries in
	// position[2]) — so an offset dev spawn sits on the local ground, not the player's height.
	pos[2] = Altitude.getCellHeight(pos[0], pos[1]);

	acquire(path);

	const actor = createActor();
	if (o.action && o.action !== 'stand') {
		setAction(actor, o.action, 0);
		if (o.action === 'move') {
			actor.loop = true;
		}
	}

	const inst = {
		path: path,
		world: null,
		worldBuilt: false,
		actor: actor,
		dir: dir,
		pos: pos,
		standbyIdx: o.standbyIdx != null ? o.standbyIdx : 0,
		animIndex: 0,
		t: 0
	};
	_instances.push(inst);
	return inst;
}

/**
 * spawnMany(path, n, opts) — dev stress path: spawn n instances of one type in a sunflower
 * (phyllotaxis) spiral radiating out around the player, to prove same-type mobs collapse to one
 * geometry upload + one poseAt per phase. opts.spacing scales the disk radius.
 */
function spawnMany(path, n, opts) {
	const o = opts || {};
	const ent = Session.Entity;
	const base = o.pos || (ent ? [ent.position[0], ent.position[1], ent.position[2]] : [0, 0, 0]);
	const spacing = o.spacing || 1.2;
	const golden = Math.PI * (3 - Math.sqrt(5)); // ~137.5 deg — even phyllotaxis fill
	const out = [];
	for (let i = 0; i < n; i++) {
		const r = spacing * Math.sqrt(i + 0.5); // radius grows outward, dense near the centre
		const a = i * golden;
		const inst = spawn(path, {
			pos: [base[0] + r * Math.cos(a), base[1] + r * Math.sin(a), base[2]],
			dir: o.dir,
			action: o.action,
			standbyIdx: o.standbyIdx
		});
		// decorrelate: phase-shift each instance by one 40 Hz tick so they land in distinct pose
		// buckets (realistic non-synchronized load — poseAt count then caps at duration*40, not N).
		if (o.decorrelate) {
			inst.actor.startT = -i * (1000 / POSE_HZ);
		}
		out.push(inst);
	}
	return out;
}

/**
 * clear() — dev: drop every instance (GL type resources stay cached for re-spawn; the map
 * unload frees them via free(gl)).
 */
function clear() {
	// Route every instance through detach() so per-guild emblem textures are freed and the type
	// refcount is decremented (a bare `_instances = []` orphaned both). detach() mutates _instances,
	// so iterate over a copy. Type GL resources stay cached for re-spawn (freed at map unload).
	const insts = _instances.slice();
	for (let i = 0; i < insts.length; i++) {
		detach(insts[i]);
	}
	_poseCache = {};
}

export default {
	init: init,
	free: free,
	render: render,
	attach: attach,
	detach: detach,
	// True once a gr2 path has 404'd on fetch or thrown on decode; the Entity path falls back to Poring.
	isMissing: function (path) {
		return _missing[path] === true;
	},
	// True only for a model in GR2_ROSTER (has a working 3D decode + lighting path). Unsupported .gr2
	// (dragon_5, Hugeling90_6, future models) return false so the Entity path substitutes a Poring
	// sprite synchronously -- no I/O, no invisible-actor frame.
	isSupported: function (path) {
		return GR2_ROSTER[gr2Basename(path)] !== undefined;
	},
	spawn: spawn,
	spawnMany: spawnMany,
	clear: clear,
	FADE: FADE,
	// Debug: highlight each instance's anchor cell (see _debugCell). Toggle from the dev console.
	get debugCell() {
		return _debugCell;
	},
	set debugCell(v) {
		_debugCell = !!v;
	}
};
