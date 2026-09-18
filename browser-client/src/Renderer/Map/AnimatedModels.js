/**
 * Renderer/Map/AnimatedModels.js
 *
 * Render animated RSM models on the map
 * These models have animation keyframes and need per-frame geometry updates
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import Client from 'Core/Client.js';
import glMatrix from 'Utils/gl-matrix.js';
import WebGL from 'Utils/WebGL.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import _vertexShader from './AnimatedModels.vs?raw';
import _fragmentShader from './AnimatedModels.fs?raw';
import OccluderFade from 'Renderer/Map/OccluderFade.js';

const mat3 = glMatrix.mat3;
const mat4 = glMatrix.mat4;
const vec3 = glMatrix.vec3;
const quat = glMatrix.quat;

/**
 * Scratchpads to evade GC (Garbage Collector)
 */
const _tempVec3 = vec3.create();
const _tempVec3Scale = vec3.create();
const _tempQuat = quat.create();
const _tempMat4 = mat4.create();

/**
 * Shader program
 */
let _program = null;

/**
 * List of animated models
 */
let _animatedModels = [];

/**
 * Initialize shader program
 */
function init(gl) {
	_program = WebGL.createShaderProgram(gl, _vertexShader, OccluderFade.injectShader(_fragmentShader));

	_program.uniform = {
		uModelViewMat: gl.getUniformLocation(_program, 'uModelViewMat'),
		uProjectionMat: gl.getUniformLocation(_program, 'uProjectionMat'),
		uNormalMat: gl.getUniformLocation(_program, 'uNormalMat'),
		uLightDirection: gl.getUniformLocation(_program, 'uLightDirection'),
		uLightOpacity: gl.getUniformLocation(_program, 'uLightOpacity'),
		uLightAmbient: gl.getUniformLocation(_program, 'uLightAmbient'),
		uLightDiffuse: gl.getUniformLocation(_program, 'uLightDiffuse'),
		uLightEnv: gl.getUniformLocation(_program, 'uLightEnv'),
		uFogUse: gl.getUniformLocation(_program, 'uFogUse'),
		uFogNear: gl.getUniformLocation(_program, 'uFogNear'),
		uFogFar: gl.getUniformLocation(_program, 'uFogFar'),
		uFogColor: gl.getUniformLocation(_program, 'uFogColor'),
		uDiffuse: gl.getUniformLocation(_program, 'uDiffuse'),
		uOccluderFadeMode: gl.getUniformLocation(_program, 'uOccluderFadeMode'),
		uOccluderFadeEye: gl.getUniformLocation(_program, 'uOccluderFadeEye'),
		uOccluderFadeFocus: gl.getUniformLocation(_program, 'uOccluderFadeFocus'),
		uOccluderFadeRadius: gl.getUniformLocation(_program, 'uOccluderFadeRadius'),
		uOccluderFadeOpacity: gl.getUniformLocation(_program, 'uOccluderFadeOpacity'),
		uOccluderFadeStrength: gl.getUniformLocation(_program, 'uOccluderFadeStrength')
	};

	_program.attribute = {
		aPosition: gl.getAttribLocation(_program, 'aPosition'),
		aNormal: gl.getAttribLocation(_program, 'aNormal'),
		aTextureCoord: gl.getAttribLocation(_program, 'aTextureCoord'),
		aAlpha: gl.getAttribLocation(_program, 'aAlpha')
	};
}

/**
 * Free resources
 */
function free(gl) {
	for (let i = 0; i < _animatedModels.length; i++) {
		const model = _animatedModels[i];
		if (model.vao) {
			gl.deleteVertexArray(model.vao);
		}
		if (model.buffer) {
			gl.deleteBuffer(model.buffer);
		}
		for (const tex in model.textureObjects) {
			if (model.textureObjects[tex]) {
				gl.deleteTexture(model.textureObjects[tex]);
			}
		}
	}
	_animatedModels = [];
}

function isNodeStatic(node) {
	return (
		(!node.rotKeyframes || node.rotKeyframes.length === 0) &&
		(!node.posKeyframes || node.posKeyframes.length === 0) &&
		(!node.scaleKeyFrames || node.scaleKeyFrames.length === 0)
	);
}

/**
 * Add an animated model
 */
function add(gl, modelData) {
	if (!modelData || !modelData.nodes || modelData.nodes.length === 0) {
		return;
	}

	if (!_program) {
		init(gl);
	}

	// Deserialize instances
	const instances = [];
	for (let i = 0; i < modelData.instances.length; i++) {
		const instArray = modelData.instances[i];
		const matrix = mat4.create();
		for (let j = 0; j < 16; j++) {
			matrix[j] = instArray[j];
		}
		instances.push(matrix);
	}

	// Deserialize nodes - convert arrays to Float32Arrays
	const nodes = [];
	let totalAnimationLength = 0;
	let hasAnyAnimation = false;

	for (let n = 0; n < modelData.nodes.length; n++) {
		const srcNode = modelData.nodes[n];
		if (!isNodeStatic(srcNode)) {
			hasAnyAnimation = true;
		}
		const node = {
			name: srcNode.name,
			parentname: srcNode.parentname,
			is_only: srcNode.is_only,
			textures: srcNode.textures,
			vertices: srcNode.vertices,
			tvertices: new Float32Array(srcNode.tvertices),
			faces: srcNode.faces,
			pos: new Float32Array(srcNode.pos),
			rotangle: srcNode.rotangle,
			rotaxis: new Float32Array(srcNode.rotaxis),
			scale: new Float32Array(srcNode.scale),
			offset: new Float32Array(srcNode.offset),
			mat3: new Float32Array(srcNode.mat3),
			rotKeyframes: srcNode.rotKeyframes || [],
			posKeyframes: srcNode.posKeyframes || [],
			scaleKeyFrames: srcNode.scaleKeyFrames || [],
			_isStatic: !hasAnyAnimation,
			_index: n,
			_cache: {
				local: mat4.create(),
				final: mat4.create(),
				// Cache for instances final matrices
				instances: new Array(instances.length)
			}
		};
		// Pre-calculate local matrix for static nodes
		if (node._isStatic) {
			const local = mat4.create();
			mat4.identity(local);
			mat4.translate(local, local, node.pos);
			mat4.rotate(local, local, node.rotangle, node.rotaxis);
			mat4.scale(local, local, node.scale);
			node._staticLocalMatrix = local;
		}
		// Initialize instance cache
		for (let k = 0; k < instances.length; k++) {
			node._cache.instances[k] = mat4.create();
		}

		if (node.rotKeyframes) {
			for (let rk = 0; rk < node.rotKeyframes.length; rk++) {
				const kf = node.rotKeyframes[rk];
				totalAnimationLength = Math.max(totalAnimationLength, kf.frame || 0);

				if (kf.q) {
					kf._quat = quat.fromValues(kf.q[0], kf.q[1], kf.q[2], kf.q[3]);
				}
			}
		}

		if (node.posKeyframes) {
			for (let pk = 0; pk < node.posKeyframes.length; pk++) {
				const kf = node.posKeyframes[pk];
				totalAnimationLength = Math.max(totalAnimationLength, kf.frame || 0);
				kf._vec = vec3.fromValues(kf.px, kf.py, kf.pz);
			}
		}

		if (node.scaleKeyFrames) {
			for (let sk = 0; sk < node.scaleKeyFrames.length; sk++) {
				const kf = node.scaleKeyFrames[sk];

				totalAnimationLength = Math.max(totalAnimationLength, kf.Frame || 0);
				kf._vec = vec3.fromValues(kf.Scale[0], kf.Scale[1], kf.Scale[2]);
			}
		}

		nodes.push(node);
	}

	// Calculate animLen from keyframes if not set
	let animLen = modelData.animLen || 0;

	// Build Mesh Layout Plan (Group by Texture -> Node -> Instance)
	// This prevents rebuilding the array structure every frame.
	const textureGroups = {};
	let totalFloats = 0;

	if (animLen === 0) {
		// Find max frame from keyframes
		for (let n = 0; n < nodes.length; n++) {
			const node = nodes[n];
			if (node.rotKeyframes) {
				for (let rk = 0; rk < node.rotKeyframes.length; rk++) {
					animLen = Math.max(animLen, node.rotKeyframes[rk].frame || 0);
				}
			}
			if (node.posKeyframes) {
				for (let pk = 0; pk < node.posKeyframes.length; pk++) {
					animLen = Math.max(animLen, node.posKeyframes[pk].frame || 0);
				}
			}
			if (node.scaleKeyFrames) {
				for (let sk = 0; sk < node.scaleKeyFrames.length; sk++) {
					animLen = Math.max(animLen, node.scaleKeyFrames[sk].Frame || 0);
				}
			}
		}
		// Add some frames to ensure last keyframe is reached
		animLen = animLen + 1;
	}
	// Calculate size per texture
	// Iterate all nodes to find which textures they use and how many faces
	for (let n = 0; n < nodes.length; n++) {
		const node = nodes[n];
		// Count faces per texture for this node
		const facesPerTex = {};
		for (let f = 0; f < node.faces.length; f++) {
			const tid = node.textures[node.faces[f].texid];
			if (!facesPerTex[tid]) {
				facesPerTex[tid] = 0;
			}
			facesPerTex[tid]++;
		}

		// Add to global texture groups
		for (const tid in facesPerTex) {
			if (!textureGroups[tid]) {
				textureGroups[tid] = { count: 0, writePlan: [] };
			}

			// For every instance of this node
			for (let inst = 0; inst < instances.length; inst++) {
				const vertCount = facesPerTex[tid] * 3; // 3 verts per face
				const floatCount = vertCount * 9; // 9 floats per vert (3 pos, 3 norm, 2 uv, 1 alpha)

				// Create a write instruction
				textureGroups[tid].writePlan.push({
					nodeIndex: n,
					instanceIndex: inst,
					faceCount: facesPerTex[tid],
					startFaceIndex: 0, // Need to find where faces for this tex start in node.faces? No, faces are mixed.
					// We will need to iterate faces in update and pick matches
					targetOffset: 0 // Will be calculated below
				});

				textureGroups[tid].count += vertCount;
				totalFloats += floatCount;
			}
		}
	}

	let currentOffset = 0;
	const meshInfos = [];

	for (const tid in textureGroups) {
		const group = textureGroups[tid];
		const startOffset = currentOffset;

		// Record where each sub-mesh writes
		for (let wp = 0; wp < group.writePlan.length; wp++) {
			const plan = group.writePlan[wp];
			plan.targetOffset = currentOffset;
			currentOffset += plan.faceCount * 3 * 9;
		}

		meshInfos.push({
			textureIdx: parseInt(tid),
			vertOffset: startOffset / 9,
			vertCount: (currentOffset - startOffset) / 9
		});
	}

	// Create Model Object
	const animModel = {
		filename: modelData.filename,
		animLen: animLen,
		fps: modelData.frameRatePerSecond || 30,
		shadeType: modelData.shadeType,
		alpha: modelData.alpha,
		textures: modelData.textures,
		instances: instances,
		nodes: nodes,
		box: {
			center: new Float32Array(modelData.box.center),
			max: new Float32Array(modelData.box.max),
			min: new Float32Array(modelData.box.min),
			offset: new Float32Array(modelData.box.offset),
			range: new Float32Array(modelData.box.range)
		},
		textureObjects: {},

		// WebGL & Caching
		buffer: gl.createBuffer(),
		_gpuBuffer: new Float32Array(totalFloats), // Allocate once
		meshInfos: meshInfos,
		writePlans: textureGroups, // Instructions for update

		// State
		lastFrame: -1,
		lastAnimFrame: -1,
		staticModel: !hasAnyAnimation,
		_nodeMap: {},
		_globalMatrices: new Array(nodes.length)
	};

	// Cache Node Map
	for (let n = 0; n < nodes.length; n++) {
		animModel._nodeMap[nodes[n].name] = nodes[n];
		animModel._globalMatrices[n] = mat4.create();
	}

	// Allocate Buffer on GPU immediately (Empty but sized)
	gl.bindBuffer(gl.ARRAY_BUFFER, animModel.buffer);
	gl.bufferData(gl.ARRAY_BUFFER, animModel._gpuBuffer.byteLength, gl.DYNAMIC_DRAW);

	// Load textures
	for (let t = 0; t < modelData.textures.length; t++) {
		const texturePath = 'data\\texture\\' + modelData.textures[t];
		loadTexture(gl, animModel, texturePath, t);
	}

	animModel.vao = gl.createVertexArray();
	gl.bindVertexArray(animModel.vao);

	gl.bindBuffer(gl.ARRAY_BUFFER, animModel.buffer);

	const attribute = _program.attribute;
	const stride = 36; // 9 floats * 4 bytes

	gl.enableVertexAttribArray(attribute.aPosition);
	gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, stride, 0);

	gl.enableVertexAttribArray(attribute.aNormal);
	gl.vertexAttribPointer(attribute.aNormal, 3, gl.FLOAT, false, stride, 12);

	gl.enableVertexAttribArray(attribute.aTextureCoord);
	gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, stride, 24);

	gl.enableVertexAttribArray(attribute.aAlpha);
	gl.vertexAttribPointer(attribute.aAlpha, 1, gl.FLOAT, false, stride, 32);

	gl.bindVertexArray(null);

	_animatedModels.push(animModel);

	// Force first update to populate buffer
	updateModelBuffer(gl, animModel, 0, true);
}

/**
 * Load a texture for a model
 */
function loadTexture(gl, model, path, index) {
	Client.loadFile(
		path,
		function (data) {
			WebGL.texture(gl, data, function (texture) {
				model.textureObjects[index] = texture;
			});
		},
		function () {
			// Texture not found, use a placeholder
			model.textureObjects[index] = null;
		}
	);
}

function getPositionAtFrame(keyframes, frame, out) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}
	if (keyframes.length === 1) {
		return keyframes[0]._vec;
	}

	let prev = keyframes[0],
		next = null;
	for (let i = 0; i < keyframes.length; i++) {
		if (keyframes[i].frame > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return prev._vec;
	}
	const t = (frame - prev.frame) / (next.frame - prev.frame);
	return vec3.lerp(out, prev._vec, next._vec, t);
}

function getRotationAtFrame(keyframes, frame, out) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}
	if (keyframes.length === 1) {
		return keyframes[0]._quat;
	}

	let prev = keyframes[0],
		next = null;
	for (let i = 0; i < keyframes.length; i++) {
		if (keyframes[i].frame > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return prev._quat;
	}
	const t = (frame - prev.frame) / (next.frame - prev.frame);
	return quat.slerp(out, prev._quat, next._quat, t);
}

function getScaleAtFrame(keyframes, frame, out) {
	if (!keyframes || keyframes.length === 0) {
		return null;
	}
	if (keyframes.length === 1) {
		return keyframes[0]._vec;
	}

	let prev = keyframes[0],
		next = null;
	for (let i = 0; i < keyframes.length; i++) {
		const f = typeof keyframes[i].Frame !== 'undefined' ? keyframes[i].Frame : keyframes[i].frame;
		if (f > frame) {
			next = keyframes[i];
			break;
		}
		prev = keyframes[i];
	}

	if (!next) {
		return prev._vec;
	}

	const fPrev = typeof prev.Frame !== 'undefined' ? prev.Frame : prev.frame;
	const fNext = typeof next.Frame !== 'undefined' ? next.Frame : next.frame;

	const t = (frame - fPrev) / (fNext - fPrev);
	return vec3.lerp(out, prev._vec, next._vec, t);
}

/**
 * Transform a specific node's geometry for a specific texture and instance
 * Writes directly to the monolithic buffer
 */
function transformAndWrite(node, finalMatrix, textureId, offset, buffer, alpha) {
	// Extract Rotation for normals
	// Optimization: Don't use mat4.extractRotation/invert/transpose every face.
	// Just use the upper 3x3 of finalMatrix if uniform scaling.
	// For non-uniform, we strictly need normal matrix, but RSMs usually handle normals simpler.
	// Let's stick to standard logic: Extract rotation from finalMatrix.

	const normalMat = node._cache.normalMat || mat4.create();
	node._cache.normalMat = normalMat;
	mat4.extractRotation(normalMat, finalMatrix);

	const m = finalMatrix;
	const n = normalMat;

	// Cache Matrix access
	const m0 = m[0],
		m1 = m[1],
		m2 = m[2],
		m4 = m[4],
		m5 = m[5],
		m6 = m[6],
		m8 = m[8],
		m9 = m[9],
		m10 = m[10],
		m12 = m[12],
		m13 = m[13],
		m14 = m[14];
	const n0 = n[0],
		n1 = n[1],
		n2 = n[2],
		n4 = n[4],
		n5 = n[5],
		n6 = n[6],
		n8 = n[8],
		n9 = n[9],
		n10 = n[10];

	const faces = node.faces;
	const vertices = node.vertices;
	const tvertices = node.tvertices;
	const textures = node.textures;
	const output = buffer;
	let o = offset;

	// Iterate faces, process only those matching textureId
	for (let f = 0; f < faces.length; f++) {
		const face = faces[f];
		if (textures[face.texid] !== textureId) {
			continue;
		}

		// Compute Face Normal (using Original Vertices)
		const v0 = vertices[face.vertidx[0]];
		const v1 = vertices[face.vertidx[1]];
		const v2 = vertices[face.vertidx[2]];

		const ax = v1[0] - v0[0],
			ay = v1[1] - v0[1],
			az = v1[2] - v0[2];
		const bx = v2[0] - v0[0],
			by = v2[1] - v0[1],
			bz = v2[2] - v0[2];

		let nx = ay * bz - az * by;
		let ny = az * bx - ax * bz;
		let nz = ax * by - ay * bx;

		// Normalize
		let len = nx * nx + ny * ny + nz * nz;
		if (len > 0) {
			len = 1.0 / Math.sqrt(len);
			nx *= len;
			ny *= len;
			nz *= len;
		}

		// Transform Normal
		const tnx = n0 * nx + n4 * ny + n8 * nz;
		const tny = n1 * nx + n5 * ny + n9 * nz;
		const tnz = n2 * nx + n6 * ny + n10 * nz;

		// Process 3 Vertices
		for (let vi = 0; vi < 3; vi++) {
			const vIdx = face.vertidx[vi];
			const tIdx = face.tvertidx[vi];

			const vx = vertices[vIdx][0];
			const vy = vertices[vIdx][1];
			const vz = vertices[vIdx][2];

			// Position (Transformed)
			output[o++] = m0 * vx + m4 * vy + m8 * vz + m12;
			output[o++] = m1 * vx + m5 * vy + m9 * vz + m13;
			output[o++] = m2 * vx + m6 * vy + m10 * vz + m14;

			// Normal (Transformed)
			output[o++] = tnx;
			output[o++] = tny;
			output[o++] = tnz;

			// UV
			output[o++] = tvertices[tIdx * 6 + 4];
			output[o++] = tvertices[tIdx * 6 + 5];

			// Alpha
			output[o++] = alpha;
		}
	}
}

/**
 * Update model buffer at frame
 */
function updateModelBuffer(gl, model, frame, force) {
	const frameDuration = 1000 / (model.fps / (GraphicsSettings.performanceMode ? 2 : 1.14));
	const currentAnimFrame = Math.floor(frame / frameDuration);

	if (!force && model.lastAnimFrame === currentAnimFrame && !model.staticModel) {
		return;
	}

	if (model.staticModel && model.lastAnimFrame !== -1 && !force) {
		return;
	}

	model.lastAnimFrame = currentAnimFrame;
	model.lastFrame = frame;

	const box = model.box;
	const nodeMap = model._nodeMap;
	const globalMatrices = model._globalMatrices;

	// 1. Update Matrix Hierarchy
	for (let n = 0; n < model.nodes.length; n++) {
		const node = model.nodes[n];
		const globalMatrix = globalMatrices[n];

		// Parent Transform
		if (node.parentname && nodeMap[node.parentname] && node.parentname !== node.name) {
			const parentIdx = nodeMap[node.parentname]._index;
			mat4.copy(globalMatrix, globalMatrices[parentIdx]);
		} else {
			mat4.identity(globalMatrix);
		}

		// Local Transform
		if (node._isStatic) {
			// Optimization: Use precalculated local matrix
			mat4.multiply(globalMatrix, globalMatrix, node._staticLocalMatrix);
		} else {
			const animPos = getPositionAtFrame(node.posKeyframes, frame, _tempVec3);
			mat4.translate(globalMatrix, globalMatrix, animPos || node.pos);

			const animRot = getRotationAtFrame(node.rotKeyframes, frame, _tempQuat);
			if (animRot) {
				mat4.fromQuat(_tempMat4, animRot);
				mat4.multiply(globalMatrix, globalMatrix, _tempMat4);
			} else {
				mat4.rotate(globalMatrix, globalMatrix, node.rotangle, node.rotaxis);
			}

			const animScale = getScaleAtFrame(node.scaleKeyFrames, frame, _tempVec3Scale);
			mat4.scale(globalMatrix, globalMatrix, animScale || node.scale);
		}

		// Final Node Matrix (Center correction + Offset)
		const finalNodeMatrix = node._cache.final;
		mat4.identity(finalNodeMatrix);
		mat4.translate(finalNodeMatrix, finalNodeMatrix, [-box.center[0], -box.max[1], -box.center[2]]);
		mat4.multiply(finalNodeMatrix, finalNodeMatrix, globalMatrix); // Apply hierarchy

		if (!node.is_only) {
			mat4.translate(finalNodeMatrix, finalNodeMatrix, node.offset);
		}
		mat4.multiply(finalNodeMatrix, finalNodeMatrix, mat3.toMat4(node.mat3));

		// Cache for step 2
		node.finalMatrix = finalNodeMatrix;
	}

	// 2. Pre-calculate Instance Matrices
	// We do this to avoid recalculating Node * Instance for every texture group
	for (let n = 0; n < model.nodes.length; n++) {
		const node = model.nodes[n];
		for (let i = 0; i < model.instances.length; i++) {
			// Instance Final = InstanceWorld * NodeFinal
			mat4.multiply(node._cache.instances[i], model.instances[i], node.finalMatrix);
		}
	}

	// 3. Write Geometry to Buffer (Using WritePlan)
	// This ensures the buffer is ordered by Texture, maximizing batching
	const buffer = model._gpuBuffer;
	const writePlans = model.writePlans;

	for (const tid in writePlans) {
		const group = writePlans[tid];
		const plans = group.writePlan;
		const textureId = parseInt(tid);

		for (let p = 0; p < plans.length; p++) {
			const plan = plans[p];
			const node = model.nodes[plan.nodeIndex];
			const finalInstanceMatrix = node._cache.instances[plan.instanceIndex];

			transformAndWrite(node, finalInstanceMatrix, textureId, plan.targetOffset, buffer, model.alpha);
		}
	}

	// 4. Upload to GPU
	// Use bufferSubData to avoid reallocation
	gl.bindBuffer(gl.ARRAY_BUFFER, model.buffer);
	gl.bufferSubData(gl.ARRAY_BUFFER, 0, buffer);
}

/**
 * Bind program and per-frame uniforms shared by both model passes
 */
function bind(gl, modelView, projection, normalMat, fog, light) {
	const uniform = _program.uniform;

	gl.useProgram(_program);

	// Bind matrices
	gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
	gl.uniformMatrix3fv(uniform.uNormalMat, false, normalMat);

	// Bind light
	gl.uniform3fv(uniform.uLightDirection, light.direction);
	gl.uniform1f(uniform.uLightOpacity, light.opacity);
	gl.uniform3fv(uniform.uLightAmbient, light.ambient);
	gl.uniform3fv(uniform.uLightDiffuse, light.diffuse);
	gl.uniform3fv(uniform.uLightEnv, light.env);

	// Fog settings
	gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
	gl.uniform1f(uniform.uFogNear, fog.near);
	gl.uniform1f(uniform.uFogFar, fog.far);
	gl.uniform3fv(uniform.uFogColor, fog.color);

	gl.activeTexture(gl.TEXTURE0);
	gl.uniform1i(uniform.uDiffuse, 0);
}

/**
 * Issue the draw calls for every animated model
 */
function drawModels(gl) {
	for (let m = 0; m < _animatedModels.length; m++) {
		const model = _animatedModels[m];

		if (!model.buffer || model.meshInfos.length === 0) {
			continue;
		}

		gl.bindVertexArray(model.vao);

		for (let i = 0; i < model.meshInfos.length; i++) {
			const info = model.meshInfos[i];
			const texture = model.textureObjects[info.textureIdx];

			if (texture) {
				gl.bindTexture(gl.TEXTURE_2D, texture);
				gl.drawArrays(gl.TRIANGLES, info.vertOffset, info.vertCount);
			}
		}
	}
}

/**
 * Render animated models (opaque pass)
 */
function render(gl, modelView, projection, normalMat, fog, light, tick) {
	if (_animatedModels.length === 0) {
		return;
	}

	if (!_program) {
		init(gl);
	}

	bind(gl, modelView, projection, normalMat, fog, light);

	for (let m = 0; m < _animatedModels.length; m++) {
		const model = _animatedModels[m];
		updateModelBuffer(gl, model, tick % (model.animLen || 1), false);
	}

	OccluderFade.renderOpaque(gl, _program.uniform, () => drawModels(gl));
	OccluderFade.renderQuery(gl, _program.uniform, () => drawModels(gl), OccluderFade.QUERY.ANIMATED);
	// Disable attributes
	gl.bindVertexArray(null);
}

/**
 * Render the faded (see-through) part of the animated models, translucent.
 * Reuses the vertex data uploaded by render() this frame.
 */
function renderFaded(gl, modelView, projection, normalMat, fog, light) {
	if (_animatedModels.length === 0 || !_program || !OccluderFade.needsBlendPass()) {
		return;
	}

	bind(gl, modelView, projection, normalMat, fog, light);
	OccluderFade.renderBlend(gl, _program.uniform, () => drawModels(gl));
	gl.bindVertexArray(null);
}

/**
 * Check if there are animated models
 */
function hasAnimatedModels() {
	return _animatedModels.length > 0;
}

/**
 * Export
 */
export default {
	init: init,
	free: free,
	add: add,
	render: render,
	renderFaded: renderFaded,
	hasAnimatedModels: hasAnimatedModels
};
