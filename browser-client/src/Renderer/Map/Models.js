/**
 * @module Renderer/Map/Models
 *
 * Rendering Models
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import _vertexShader from 'Renderer/Effects/Shaders/GLSL/Models.vs?raw';
import _fragmentShader from 'Renderer/Effects/Shaders/GLSL/Models.fs?raw';
import WebGL from 'Utils/WebGL.js';
import Preferences from 'Preferences/Map.js';
import OccluderFade from 'Renderer/Map/OccluderFade.js';

/**
 * @let {WebGLProgram}
 */
let _program = null;

/**
 * @let {WebGLBuffer}
 */
let _buffer = null;

/**
 * @let {Array} list of meshes
 */
const _objects = [];

/**
 * @let {Array} batched draw calls (grouped by texture)
 */
const _batches = [];

/**
 * @let {boolean} whether all textures are loaded and batches are built
 */
let _batchesReady = false;

/**
 * @let {number} count of textures still loading
 */
let _pendingTextures = 0;

/**
 * Build batched draw calls by merging consecutive objects with the same texture.
 * Objects sharing the same texture with contiguous vertex ranges are merged
 * into a single draw call, reducing GPU state changes.
 */
function buildBatches() {
	_batches.length = 0;
	let current = null;

	for (let i = 0, count = _objects.length; i < count; ++i) {
		if (!_objects[i].complete) {
			continue;
		}

		// Can merge if same texture and contiguous vertex range
		if (
			current &&
			current.texture === _objects[i].texture &&
			current.vertOffset + current.vertCount === _objects[i].vertOffset
		) {
			current.vertCount += _objects[i].vertCount;
		} else {
			current = {
				texture: _objects[i].texture,
				vertOffset: _objects[i].vertOffset,
				vertCount: _objects[i].vertCount
			};
			_batches.push(current);
		}
	}

	_batchesReady = true;
}

/**
 * Initialize models
 *
 * @param {object} gl context
 * @param {object} data ( models )
 */
function init(gl, data) {
	const objects = data.infos;
	const count = objects.length;
	_objects.length = count;
	_batchesReady = false;
	_pendingTextures = count;

	// Bind buffer
	if (!_buffer) {
		_buffer = gl.createBuffer();
	}

	if (!_program) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, OccluderFade.injectShader(_fragmentShader));
	}

	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
	gl.bufferData(gl.ARRAY_BUFFER, data.buffer, gl.STATIC_DRAW);

	function onTextureLoaded(texture, index) {
		_objects[index].texture = texture;
		_objects[index].complete = true;
		_pendingTextures--;

		// Rebuild batches when all textures are loaded
		if (_pendingTextures <= 0) {
			buildBatches();
		}
	}

	// Fetch all images, and draw them in a mega-texture
	for (let i = 0; i < count; ++i) {
		if (!_objects[i]) {
			_objects[i] = {};
		}

		_objects[i].vertCount = data.infos[i].vertCount;
		_objects[i].vertOffset = data.infos[i].vertOffset;
		_objects[i].complete = false;

		WebGL.texture(gl, data.infos[i].texture, onTextureLoaded, i);
	}
}

/**
 * Issue the draw calls for every loaded mesh
 *
 * @param {object} gl context
 */
function drawMeshes(gl) {
	let i, count;

	if (_batchesReady) {
		// Optimized path: use pre-built batches with conditional texture binding
		let lastTexture = null;
		for (i = 0, count = _batches.length; i < count; ++i) {
			if (_batches[i].texture !== lastTexture) {
				gl.bindTexture(gl.TEXTURE_2D, _batches[i].texture);
				lastTexture = _batches[i].texture;
			}
			gl.drawArrays(gl.TRIANGLES, _batches[i].vertOffset, _batches[i].vertCount);
		}
	} else {
		// Fallback: render individually while textures are still loading
		for (i = 0, count = _objects.length; i < count; ++i) {
			if (_objects[i].complete) {
				gl.bindTexture(gl.TEXTURE_2D, _objects[i].texture);
				gl.drawArrays(gl.TRIANGLES, _objects[i].vertOffset, _objects[i].vertCount);
			}
		}
	}
}

/**
 * Bind program, uniforms and vertex layout shared by both model passes
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {object} fog structure
 * @param {object} light structure
 */
function bind(gl, modelView, projection, fog, light) {
	const uniform = _program.uniform;
	const attribute = _program.attribute;

	gl.useProgram(_program);

	// Bind matrix
	gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

	// Bind light
	gl.uniform3fv(uniform.uLightDirection, light.direction);
	gl.uniform1f(uniform.uLightOpacity, light.opacity);
	gl.uniform3fv(uniform.uLightAmbient, light.ambient);
	gl.uniform3fv(uniform.uLightDiffuse, light.diffuse);
	gl.uniform3fv(uniform.uLightEnv, light.env);

	// Use shadows
	gl.uniform1i(uniform.uLightMapUse, Preferences.lightmap);

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

	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

	// Link attribute
	gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 9 * 4, 0);
	gl.vertexAttribPointer(attribute.aVertexNormal, 3, gl.FLOAT, false, 9 * 4, 3 * 4);
	gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 9 * 4, 6 * 4);
	gl.vertexAttribPointer(attribute.aAlpha, 1, gl.FLOAT, false, 9 * 4, 8 * 4);

	// Textures
	gl.activeTexture(gl.TEXTURE0);
	gl.uniform1i(uniform.uDiffuse, 0);
}

/**
 * Release the vertex layout
 *
 * @param {object} gl context
 */
function unbind(gl) {
	const attribute = _program.attribute;

	// Is it needed ?
	gl.disableVertexAttribArray(attribute.aPosition);
	gl.disableVertexAttribArray(attribute.aVertexNormal);
	gl.disableVertexAttribArray(attribute.aTextureCoord);
	gl.disableVertexAttribArray(attribute.aAlpha);
}

/**
 * Render models (opaque pass)
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {mat3} normalMat
 * @param {object} fog structure
 * @param {object} light structure
 */
function render(gl, modelView, projection, normalMat, fog, light) {
	bind(gl, modelView, projection, fog, light);
	OccluderFade.renderOpaque(gl, _program.uniform, () => drawMeshes(gl));
	OccluderFade.renderQuery(gl, _program.uniform, () => drawMeshes(gl), OccluderFade.QUERY.MODELS);
	unbind(gl);
}

/**
 * Render the faded (see-through) part of the models, translucent.
 * Call after opaque scene elements so they remain visible behind it.
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {mat3} normalMat
 * @param {object} fog structure
 * @param {object} light structure
 */
function renderFaded(gl, modelView, projection, normalMat, fog, light) {
	if (!OccluderFade.needsBlendPass()) {
		return;
	}

	bind(gl, modelView, projection, fog, light);
	OccluderFade.renderBlend(gl, _program.uniform, () => drawMeshes(gl));
	unbind(gl);
}

/**
 * Clean textures/buffer from memory
 *
 * @param {object} gl context
 */
function free(gl) {
	let i, count;

	if (_buffer) {
		gl.deleteBuffer(_buffer);
		_buffer = null;
	}

	if (_program) {
		gl.deleteProgram(_program);
		_program = null;
	}

	for (i = 0, count = _objects.length; i < count; ++i) {
		gl.deleteTexture(_objects[i].texture);
	}

	_objects.length = 0;
	_batches.length = 0;
	_batchesReady = false;
}

/**
 * Export
 */
export default {
	init: init,
	render: render,
	renderFaded: renderFaded,
	free: free
};
