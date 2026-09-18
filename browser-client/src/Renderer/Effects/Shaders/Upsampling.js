/**
 * Renderer/Effects/Shaders/Upsampling.js
 * Implementation of the Upsampling post-processing effect.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import commonVS from './GLSL/Common.vs?raw';
import compositeFS from './GLSL/CommonUpsampling.fs?raw';

let _program;
let _buffer;

/**
 * @constructor Upsampling
 */
class Upsampling {
	/**
	 * Renders the Upsampling effect
	 * @param {WebGLRenderingContext} gl - WebGL Context
	 * @param {WebGLTexture} inputTexture - Low resolution scene texture
	 * @param {WebGLFramebuffer} outputFramebuffer - Destination (Screen or next effect)
	 */
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !Upsampling.isActive()) {
			return;
		}
		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		const posLoc = _program.attribute.aPosition;
		gl.enableVertexAttribArray(posLoc);
		gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 0, 0);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, inputTexture);

		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);

		gl.uniform1i(_program.uniform.uSceneTexture, 0);

		gl.drawArrays(gl.TRIANGLES, 0, 6);

		PostProcess.afterRenderPass(gl);
	}

	/**
	 * Initializes shaders and buffers
	 */
	static init(gl) {
		if (!gl) {
			return;
		}

		try {
			_program = WebGL.createShaderProgram(gl, commonVS, compositeFS);
		} catch (e) {
			console.error('Error compiling Upsampling shader.', e);
			return;
		}

		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);

		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	/** @returns {boolean} always false here */
	static isActive() {
		return GraphicsSettings.performanceMode;
	}

	/** @returns {WebGLProgram} The loaded shader program (returning one for validation check) */
	static program() {
		return _program;
	}

	/** Clears memory references */
	static clean(gl) {
		_program = null;
		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}
		_buffer = null;
	}
}
export default Upsampling;
