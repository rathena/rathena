/**
 * @module Renderer/Effects/RsmEffect
 *
 * Rendering Rsm,Rsm2 File object with animation support
 */

import _vertexShader from './RsmEffect.vs?raw';
import _fragmentShader from './RsmEffect.fs?raw';
import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Model from 'Loaders/Model.js';

let _program = null;
const _normalMat = new Float32Array(3 * 3);
const mat4 = glMatrix.mat4;
const mat3 = glMatrix.mat3;
const quat = glMatrix.quat;
const vec3 = glMatrix.vec3;

const _light = {
	opacity: 1.0,
	ambient: new Float32Array([Math.PI, Math.PI, Math.PI]),
	diffuse: new Float32Array([0, 0, 0]),
	direction: new Float32Array([0, 1, 0])
};

/**
 * Interpolate between two quaternions using SLERP
 */
function slerpQuat(q1, q2, t) {
	const out = quat.create();
	quat.slerp(out, q1, q2, t);
	return out;
}

/**
 * Interpolate between two vec3 using linear interpolation
 */
function lerpVec3(v1, v2, t) {
	const out = vec3.create();
	vec3.lerp(out, v1, v2, t);
	return out;
}

/**
 * Get rotation at a specific frame from keyframes
 */
function getRotationAtFrame(rotKeyframes, frame, animLen) {
	if (!rotKeyframes || rotKeyframes.length === 0) {
		return null;
	}

	if (rotKeyframes.length === 1) {
		return rotKeyframes[0].q;
	}

	let prevIdx = 0;
	let nextIdx = 0;

	for (let i = 0; i < rotKeyframes.length; i++) {
		if (rotKeyframes[i].frame <= frame) {
			prevIdx = i;
		}
		if (rotKeyframes[i].frame >= frame) {
			nextIdx = i;
			break;
		}
	}

	if (nextIdx === 0 || rotKeyframes[nextIdx].frame < frame) {
		prevIdx = rotKeyframes.length - 1;
		nextIdx = 0;
	}

	if (prevIdx === nextIdx) {
		return rotKeyframes[prevIdx].q;
	}

	const prevFrame = rotKeyframes[prevIdx].frame;
	let nextFrame = rotKeyframes[nextIdx].frame;

	if (nextFrame < prevFrame) {
		nextFrame += animLen;
	}
	if (frame < prevFrame) {
		frame += animLen;
	}

	let t = nextFrame - prevFrame > 0 ? (frame - prevFrame) / (nextFrame - prevFrame) : 0;
	t = Math.max(0, Math.min(1, t));

	return slerpQuat(rotKeyframes[prevIdx].q, rotKeyframes[nextIdx].q, t);
}

/**
 * Get position at a specific frame from keyframes
 */
function getPositionAtFrame(posKeyframes, frame, animLen) {
	if (!posKeyframes || posKeyframes.length === 0) {
		return null;
	}

	if (posKeyframes.length === 1) {
		return [posKeyframes[0].px, posKeyframes[0].py, posKeyframes[0].pz];
	}

	let prevIdx = 0;
	let nextIdx = 0;

	for (let i = 0; i < posKeyframes.length; i++) {
		if (posKeyframes[i].frame <= frame) {
			prevIdx = i;
		}
		if (posKeyframes[i].frame >= frame) {
			nextIdx = i;
			break;
		}
	}

	if (nextIdx === 0 || posKeyframes[nextIdx].frame < frame) {
		prevIdx = posKeyframes.length - 1;
		nextIdx = 0;
	}

	if (prevIdx === nextIdx) {
		const kf = posKeyframes[prevIdx];
		return [kf.px, kf.py, kf.pz];
	}

	const prevFrame = posKeyframes[prevIdx].frame;
	let nextFrame = posKeyframes[nextIdx].frame;

	if (nextFrame < prevFrame) {
		nextFrame += animLen;
	}
	if (frame < prevFrame) {
		frame += animLen;
	}

	let t = nextFrame - prevFrame > 0 ? (frame - prevFrame) / (nextFrame - prevFrame) : 0;
	t = Math.max(0, Math.min(1, t));

	const p1 = posKeyframes[prevIdx];
	const p2 = posKeyframes[nextIdx];

	return lerpVec3([p1.px, p1.py, p1.pz], [p2.px, p2.py, p2.pz], t);
}

/**
 * Get scale at a specific frame from keyframes
 */
function getScaleAtFrame(scaleKeyFrames, frame, animLen) {
	if (!scaleKeyFrames || scaleKeyFrames.length === 0) {
		return null;
	}

	if (scaleKeyFrames.length === 1) {
		return scaleKeyFrames[0].Scale;
	}

	let prevIdx = 0;
	let nextIdx = 0;

	for (let i = 0; i < scaleKeyFrames.length; i++) {
		if (scaleKeyFrames[i].Frame <= frame) {
			prevIdx = i;
		}
		if (scaleKeyFrames[i].Frame >= frame) {
			nextIdx = i;
			break;
		}
	}

	if (nextIdx === 0 || scaleKeyFrames[nextIdx].Frame < frame) {
		prevIdx = scaleKeyFrames.length - 1;
		nextIdx = 0;
	}

	if (prevIdx === nextIdx) {
		return scaleKeyFrames[prevIdx].Scale;
	}

	const prevFrame = scaleKeyFrames[prevIdx].Frame;
	let nextFrame = scaleKeyFrames[nextIdx].Frame;

	if (nextFrame < prevFrame) {
		nextFrame += animLen;
	}
	if (frame < prevFrame) {
		frame += animLen;
	}

	let t = nextFrame - prevFrame > 0 ? (frame - prevFrame) / (nextFrame - prevFrame) : 0;
	t = Math.max(0, Math.min(1, t));

	return lerpVec3(scaleKeyFrames[prevIdx].Scale, scaleKeyFrames[nextIdx].Scale, t);
}

/**
 * Calculate normals (NONE type)
 */
function calcNormal_NONE(out) {
	for (let i = 1, count = out.length; i < count; i += 3) {
		out[i] = -1;
	}
}

/**
 * Calculate normals (FLAT type)
 */
function calcNormal_FLAT(node, out, normalMat, groupUsed) {
	let i, j, count;
	let face;
	const temp_vec = vec3.create();
	const faces = node.faces;
	const vertices = node.vertices;

	for (i = 0, j = 0, count = faces.length; i < count; ++i, j += 3) {
		face = faces[i];

		vec3.calcNormal(vertices[face.vertidx[0]], vertices[face.vertidx[1]], vertices[face.vertidx[2]], temp_vec);

		out[j] = normalMat[0] * temp_vec[0] + normalMat[4] * temp_vec[1] + normalMat[8] * temp_vec[2] + normalMat[12];
		out[j + 1] =
			normalMat[1] * temp_vec[0] + normalMat[5] * temp_vec[1] + normalMat[9] * temp_vec[2] + normalMat[13];
		out[j + 2] =
			normalMat[2] * temp_vec[0] + normalMat[6] * temp_vec[1] + normalMat[10] * temp_vec[2] + normalMat[14];

		if (face.smoothGroup !== undefined) {
			groupUsed[face.smoothGroup] = true;
		}
	}
}

/**
 * Calculate normals (SMOOTH type)
 */
function calcNormal_SMOOTH(node, normal, groupUsed, group) {
	let i, j, k, l, v, x, y, z, len;
	const size = node.vertices.length;
	const faces = node.faces;
	let face, norm;
	const count = faces.length;

	for (j = 0; j < 32; ++j) {
		if (!groupUsed[j]) {
			continue;
		}

		group[j] = new Float32Array(size * 3);
		norm = group[j];

		for (v = 0, l = 0; v < size; ++v, l += 3) {
			x = 0;
			y = 0;
			z = 0;

			for (i = 0, k = 0; i < count; ++i, k += 3) {
				face = faces[i];
				if (
					face.smoothGroup === j &&
					(face.vertidx[0] === v || face.vertidx[1] === v || face.vertidx[2] === v)
				) {
					x += normal[k];
					y += normal[k + 1];
					z += normal[k + 2];
				}
			}

			len = 1 / Math.sqrt(x * x + y * y + z * z);
			if (!isFinite(len)) {
				len = 1;
			}
			norm[l] = x * len;
			norm[l + 1] = y * len;
			norm[l + 2] = z * len;
		}
	}
}

/**
 * Generate mesh (FLAT normals)
 */
function generate_mesh_FLAT(node, vert, norm, mesh) {
	let a, b, o, i, j, k, t, count;
	const faces = node.faces;
	const textures = node.textures;
	const tver = node.tvertices;
	const alpha = node.main.alpha;
	const offset = [];
	let face, idx, tidx, out;

	for (i = 0, count = textures.length; i < count; ++i) {
		offset[textures[i]] = 0;
	}

	for (i = 0, o = 0, k = 0, count = faces.length; i < count; ++i, k += 3) {
		face = faces[i];
		idx = face.vertidx;
		tidx = face.tvertidx;
		t = textures[face.texid];
		out = mesh[t];
		o = offset[t];

		for (j = 0; j < 3; j++, o += 9) {
			a = idx[j] * 3;
			b = tidx[j] * 6;
			out[o + 0] = vert[a + 0];
			out[o + 1] = vert[a + 1];
			out[o + 2] = vert[a + 2];
			out[o + 3] = norm[k + 0];
			out[o + 4] = norm[k + 1];
			out[o + 5] = norm[k + 2];
			out[o + 6] = tver[b + 4];
			out[o + 7] = tver[b + 5];
			out[o + 8] = alpha;
		}

		offset[t] = o;
	}
}

/**
 * Generate mesh (SMOOTH normals)
 */
function generate_mesh_SMOOTH(node, vert, shadeGroup, mesh) {
	let a, b, o, i, j, t, count;
	const faces = node.faces;
	const textures = node.textures;
	const tver = node.tvertices;
	const alpha = node.main.alpha;
	const offset = [];
	let norm, face, idx, tidx, out;

	for (i = 0, count = textures.length; i < count; ++i) {
		offset[textures[i]] = 0;
	}

	for (i = 0, o = 0, count = faces.length; i < count; ++i) {
		face = faces[i];
		norm = shadeGroup[face.smoothGroup];
		idx = face.vertidx;
		tidx = face.tvertidx;

		t = textures[face.texid];
		out = mesh[t];
		o = offset[t];

		for (j = 0; j < 3; j++, o += 9) {
			a = idx[j] * 3;
			b = tidx[j] * 6;
			out[o + 0] = vert[a + 0];
			out[o + 1] = vert[a + 1];
			out[o + 2] = vert[a + 2];
			out[o + 3] = norm[a + 0];
			out[o + 4] = norm[a + 1];
			out[o + 5] = norm[a + 2];
			out[o + 6] = tver[b + 4];
			out[o + 7] = tver[b + 5];
			out[o + 8] = alpha;
		}

		offset[t] = o;
	}
}

/**
 * Compile a node at a specific animation frame
 */
function compileNodeAtFrame(node, instanceMatrix, frame, animLen) {
	const modelViewMat = mat4.create();
	const normalMat = mat4.create();

	const textures = node.textures;
	const faces = node.faces;
	const vertices = node.vertices;

	const mesh = {};
	const mesh_size = [];

	const shadeGroup = new Array(32);
	const shadeGroupUsed = new Array(32);
	let i, x, y, z, count;

	// Calculate animated matrix
	const matrix = mat4.create();
	mat4.identity(matrix);
	mat4.translate(matrix, matrix, [-node.main.box.center[0], -node.main.box.max[1], -node.main.box.center[2]]);

	// Apply node transformations with animation
	const nodeMatrix = mat4.create();
	mat4.identity(nodeMatrix);

	// Position animation
	const animPos = getPositionAtFrame(node.posKeyframes, frame, animLen);
	if (animPos) {
		mat4.translate(nodeMatrix, nodeMatrix, animPos);
	} else {
		mat4.translate(nodeMatrix, nodeMatrix, node.pos);
	}

	// Rotation animation
	const animRot = getRotationAtFrame(node.rotKeyframes, frame, animLen);
	if (animRot) {
		mat4.rotateQuat(nodeMatrix, nodeMatrix, animRot);
	} else if (node.rotKeyframes && node.rotKeyframes.length > 0) {
		mat4.rotateQuat(nodeMatrix, nodeMatrix, node.rotKeyframes[0].q);
	} else {
		mat4.rotate(nodeMatrix, nodeMatrix, node.rotangle, node.rotaxis);
	}

	// Scale animation
	const animScale = getScaleAtFrame(node.scaleKeyFrames, frame, animLen);
	if (animScale) {
		mat4.scale(nodeMatrix, nodeMatrix, animScale);
	} else {
		mat4.scale(nodeMatrix, nodeMatrix, node.scale);
	}

	mat4.multiply(matrix, matrix, nodeMatrix);

	if (!node.is_only) {
		mat4.translate(matrix, matrix, node.offset);
	}

	mat4.multiply(matrix, matrix, mat3.toMat4(node.mat3));

	// Multiply with instance matrix
	mat4.multiply(modelViewMat, instanceMatrix, matrix);
	mat4.extractRotation(normalMat, modelViewMat);

	// Generate new vertices
	count = vertices.length;
	const vert = new Float32Array(count * 3);
	for (i = 0; i < count; ++i) {
		x = vertices[i][0];
		y = vertices[i][1];
		z = vertices[i][2];

		vert[i * 3 + 0] = modelViewMat[0] * x + modelViewMat[4] * y + modelViewMat[8] * z + modelViewMat[12];
		vert[i * 3 + 1] = modelViewMat[1] * x + modelViewMat[5] * y + modelViewMat[9] * z + modelViewMat[13];
		vert[i * 3 + 2] = modelViewMat[2] * x + modelViewMat[6] * y + modelViewMat[10] * z + modelViewMat[14];
	}

	// Generate face normals
	const face_normal = new Float32Array(faces.length * 3);

	// Setup mesh slot array
	for (i = 0, count = textures.length; i < count; ++i) {
		mesh_size[textures[i]] = 0;
	}

	// Find mesh max face
	for (i = 0, count = faces.length; i < count; ++i) {
		mesh_size[textures[faces[i].texid]]++;
	}

	// Initialize buffer
	for (i = 0, count = textures.length; i < count; ++i) {
		mesh[textures[i]] = new Float32Array(mesh_size[textures[i]] * 9 * 3);
	}

	// Calculate normals based on shading type
	switch (node.main.shadeType) {
		default:
		case 0: // NONE
			calcNormal_NONE(face_normal);
			generate_mesh_FLAT(node, vert, face_normal, mesh);
			break;

		case 1: // FLAT
			calcNormal_FLAT(node, face_normal, normalMat, shadeGroupUsed);
			generate_mesh_FLAT(node, vert, face_normal, mesh);
			break;

		case 2: // SMOOTH
			calcNormal_FLAT(node, face_normal, normalMat, shadeGroupUsed);
			calcNormal_SMOOTH(node, face_normal, shadeGroupUsed, shadeGroup);
			generate_mesh_SMOOTH(node, vert, shadeGroup, mesh);
			break;
	}

	return mesh;
}

/**
 * Rebuild mesh buffer at current frame
 */
function rebuildMeshAtFrame(self, gl, frame) {
	const model = self.model;
	const nodes = model.nodes;
	const instances = model.instances;
	const objects = [];
	const infos = [];
	let total = 0;

	// Compile all nodes at current frame
	for (let ni = 0; ni < nodes.length; ni++) {
		for (let ii = 0; ii < instances.length; ii++) {
			const mesh = compileNodeAtFrame(nodes[ni], instances[ii], frame, model.animLen);
			const textureKeys = Object.keys(mesh);

			for (let ki = 0; ki < textureKeys.length; ki++) {
				const texIdx = textureKeys[ki];
				objects.push({
					texture: model.textures[texIdx],
					alpha: model.alpha,
					mesh: mesh[texIdx]
				});
				total += mesh[texIdx].length;
			}
		}
	}

	// Create buffer
	const buffer = new Float32Array(total);
	let offset = 0;
	let i;
	for (i = 0; i < objects.length; i++) {
		const obj = objects[i];
		const length = obj.mesh.length;

		infos[i] = {
			texture: 'data/texture/' + obj.texture,
			vertOffset: offset / 9,
			vertCount: length / 9
		};

		buffer.set(obj.mesh, offset);
		offset += length;
	}

	// Update GPU buffer
	if (!self.buffer) {
		self.buffer = gl.createBuffer();
	}

	gl.bindBuffer(gl.ARRAY_BUFFER, self.buffer);

	// If buffer size is the same, just update data. Otherwise, reallocate buffer storage
	if (self._bufferSize === buffer.byteLength) {
		gl.bufferSubData(gl.ARRAY_BUFFER, 0, buffer);
	} else {
		gl.bufferData(gl.ARRAY_BUFFER, buffer, gl.DYNAMIC_DRAW);
		self._bufferSize = buffer.byteLength;
	}

	// Update objects info
	if (self.objects.length !== infos.length) {
		// Need to reload textures
		for (i = 0; i < self.objects.length; i++) {
			if (self.objects[i] && self.objects[i].texture) {
				gl.deleteTexture(self.objects[i].texture);
			}
		}
		self.objects = new Array(infos.length);
		for (i = 0; i < infos.length; i++) {
			self.objects[i] = {
				vertCount: infos[i].vertCount,
				vertOffset: infos[i].vertOffset,
				complete: false,
				texture: null
			};

			(function (idx, texturePath) {
				Client.loadFile(
					texturePath,
					function (data) {
						WebGL.texture(gl, data, function (texture) {
							self.objects[idx].texture = texture;
							self.objects[idx].complete = true;
						});
					},
					function () {
						// Texture load failed, mark as complete anyway
						self.objects[idx].complete = true;
					}
				);
			})(i, infos[i].texture);
		}
	} else {
		// Just update vertex offsets and counts
		for (i = 0; i < infos.length; i++) {
			self.objects[i].vertCount = infos[i].vertCount;
			self.objects[i].vertOffset = infos[i].vertOffset;
		}
	}
}

function initModel(gl, data) {
	const self = this;
	const count = data.infos.length;
	this.objects.length = count;

	// Create a buffer if it doesn't exist
	if (!this.buffer) {
		this.buffer = gl.createBuffer();
	}

	gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);

	gl.bufferData(gl.ARRAY_BUFFER, data.buffer, this.isAnimated ? gl.DYNAMIC_DRAW : gl.STATIC_DRAW);
	this._bufferSize = data.buffer.byteLength;

	function onTextureLoaded(texture, i) {
		self.objects[i].texture = texture;
		self.objects[i].complete = true;
	}

	// Fetch all images, and draw them in a mega-texture
	for (let i = 0; i < count; ++i) {
		if (!this.objects[i]) {
			this.objects[i] = {};
		}

		this.objects[i].vertCount = data.infos[i].vertCount;
		this.objects[i].vertOffset = data.infos[i].vertOffset;
		this.objects[i].complete = false;

		WebGL.texture(gl, data.infos[i].texture, onTextureLoaded, i);
	}
}

class RsmEffect {
	constructor(params) {
		this.position = params.Inst.position;
		this.size = params.effect.size || 1;
		this.filename = 'data\\model\\' + params.effect.file + '.rsm';
		this.objects = [];
		this.buffer = null;
		this.model = null;
		this.startTick = params.Inst.startTick || 0;
		this.lastFrame = -1;
		this.isAnimated = false;
		this.animLen = 0;
		this.fps = 30;
		this.globalParameters = {
			position: new Float32Array(3),
			rotation: new Float32Array(3),
			scale: new Float32Array([-0.075, -0.075, 0.075]),
			filename: null
		};
		this._Params = params;
	}

	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

		this.ready = true;
	}

	init(gl, tick) {
		const self = this;
		let i, count, j, size, total, offset, length; /*, pos -UNUSED*/

		Client.getFile(this.filename, function (buf) {
			self.model = new Model(buf);

			// Check if model has animation
			self.isAnimated = false;
			self.animLen = self.model.animLen || 0;
			self.fps = self.model.frameRatePerSecond || 30;

			for (let n = 0; n < self.model.nodes.length; n++) {
				const node = self.model.nodes[n];
				if (
					(node.rotKeyframes && node.rotKeyframes.length > 0) ||
					(node.posKeyframes && node.posKeyframes.length > 0) ||
					(node.scaleKeyFrames && node.scaleKeyFrames.length > 0)
				) {
					self.isAnimated = true;
					break;
				}
			}

			const objects = [],
				infos = [];
			let meshes, index, object;

			// Create model in world
			self.globalParameters.filename = self.filename.replace('data/model/', '') + Math.floor(Math.random() * 15);
			self.model.createInstance(self.globalParameters, 0, 0);

			// Compile model
			const data = self.model.compile();
			count = data.meshes.length;
			total = 0;

			// Extract meshes
			for (i = 0, count = data.meshes.length; i < count; ++i) {
				meshes = data.meshes[i];
				index = Object.keys(meshes);

				for (j = 0, size = index.length; j < size; ++j) {
					objects.push({
						texture: data.textures[index[j]],
						alpha: self.model.alpha,
						mesh: meshes[index[j]]
					});

					total += meshes[index[j]].length;
				}
			}

			const buffer = new Float32Array(total);
			count = objects.length;
			//pos = 0; // UNUSED
			offset = 0;

			// Merge meshes to buffer
			for (i = 0; i < count; ++i) {
				object = objects[i];
				length = object.mesh.length;

				infos[i] = {
					texture: 'data/texture/' + object.texture,
					vertOffset: offset / 9,
					vertCount: length / 9
				};

				// Add to buffer
				buffer.set(object.mesh, offset);
				offset += length;
			}

			// Load textures
			i = -1;

			function loadNextTexture() {
				// Loading complete, rendering...
				if (++i === count) {
					// Initialize renderer
					initModel.call(self, gl, {
						buffer: buffer,
						infos: infos
					});
					self.ready = true;
					return;
				}

				Client.loadFile(
					infos[i].texture,
					_data => {
						infos[i].texture = _data;
						loadNextTexture();
					},
					loadNextTexture
				);
			}

			// Start loading textures
			loadNextTexture();
		});

		this.needInit = false;
	}

	free(gl) {
		for (let i = 0, count = this.objects.length; i < count; ++i) {
			if (this.objects[i] && this.objects[i].texture) {
				gl.deleteTexture(this.objects[i].texture);
			}
		}

		if (this.buffer) {
			gl.deleteBuffer(this.buffer);
			this.buffer = null;
		}

		this.objects.length = 0;
		this.ready = false;
	}

	static free(gl) {
		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}

		this.ready = false;
	}
	static beforeRender(gl, modelView, projection, fog, tick) {
		// Calculate normal mat
		mat4.toInverseMat3(modelView, _normalMat);
		mat3.transpose(_normalMat, _normalMat);

		// -- render
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.useProgram(_program);

		// Bind matrix
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
		gl.uniformMatrix3fv(uniform.uNormalMat, false, _normalMat);

		// Bind light
		gl.uniform3fv(uniform.uLightDirection, _light.direction);
		gl.uniform1f(uniform.uLightOpacity, _light.opacity);
		gl.uniform3fv(uniform.uLightAmbient, _light.ambient);
		gl.uniform3fv(uniform.uLightDiffuse, _light.diffuse);

		// Fog settings
		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		// Enable all attributes
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aVertexNormal);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		gl.enableVertexAttribArray(attribute.aAlpha);

		// Textures
		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);
	}

	render(gl, tick) {
		const uniform = _program.uniform;

		// Handle animation
		if (this.isAnimated && this.model && this.animLen > 0) {
			const elapsed = tick - this.startTick;
			const frame = Math.floor(((elapsed * this.fps) / 1000) % this.animLen);

			if (frame !== this.lastFrame) {
				rebuildMeshAtFrame(this, gl, frame);
				this.lastFrame = frame;
			}
		}

		gl.uniform3fv(uniform.uPosition, this.position);
		gl.uniform1f(uniform.uSize, this.size);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);

		// Resetting attributes because buffer has changed
		const attribute = _program.attribute;
		// Link attribute
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 9 * 4, 0);
		gl.vertexAttribPointer(attribute.aVertexNormal, 3, gl.FLOAT, false, 9 * 4, 3 * 4);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 9 * 4, 6 * 4);
		gl.vertexAttribPointer(attribute.aAlpha, 1, gl.FLOAT, false, 9 * 4, 8 * 4);

		for (let i = 0, count = this.objects.length; i < count; ++i) {
			if (this.objects[i] && this.objects[i].complete) {
				gl.bindTexture(gl.TEXTURE_2D, this.objects[i].texture);
				gl.drawArrays(gl.TRIANGLES, this.objects[i].vertOffset, this.objects[i].vertCount);
			}
		}
	}

	static afterRender(gl) {
		const attribute = _program.attribute;
		gl.disableVertexAttribArray(attribute.aPosition);
		gl.disableVertexAttribArray(attribute.aVertexNormal);
		gl.disableVertexAttribArray(attribute.aTextureCoord);
		gl.disableVertexAttribArray(attribute.aAlpha);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}
}
export default RsmEffect;
