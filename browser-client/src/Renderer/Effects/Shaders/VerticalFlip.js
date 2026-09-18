/**
 * Renderer/Effects/Shaders/VerticalFlip.js
 *
 * Screen vertical inversion effect (Illusion/Hallucination effect).
 * Inverts UV coordinates in the fragment shader.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import vs from './GLSL/VerticalFlip.vs?raw';
import fs from './GLSL/VerticalFlip.fs?raw';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';

let _program, _buffer;
let _active = false;

class VerticalFlip {
	static init(gl) {
		if (_program) {
			return;
		}
		try {
			_program = WebGL.createShaderProgram(gl, vs, fs);
		} catch (e) {
			console.error('Error when compiling shader VerticalFlip.', e);
			return;
		}

		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		// Format: X, Y, U, V (Draws a TRIANGLE_STRIP quad)
		gl.bufferData(
			gl.ARRAY_BUFFER,
			new Float32Array([-1, -1, 0, 0, 1, -1, 1, 0, -1, 1, 0, 1, 1, 1, 1, 1]),
			gl.STATIC_DRAW
		);
	}

	/**
	 * Executes the inverted drawing
	 * @param {WebGLRenderingContext} gl
	 * @param {WebGLTexture} inputTexture - Texture to be inverted
	 * @param {WebGLFramebuffer} outputFbo - Target
	 */
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !_active) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		// Position Attribute (X, Y)
		let posLoc = _program.attribute.aPosition;
		gl.enableVertexAttribArray(posLoc);
		gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 16, 0);

		// Texture Coordinate Attribute (U, V)
		posLoc = _program.attribute.aTextureCoord;
		gl.enableVertexAttribArray(posLoc);
		gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 16, 8);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, inputTexture);
		gl.uniform1i(_program.uniform.uTexture, 0);

		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

		PostProcess.afterRenderPass(gl);
	}

	/**
	 * @returns {WebGLProgram} Shader program
	 */
	static program() {
		return _program;
	}

	/** Resets effect state */
	static clean(gl) {
		_active = false;
		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}
		_program = _buffer = null;
	}

	/** @returns {boolean} Whether the effect is active */
	static isActive() {
		return _active;
	}

	/** @param {boolean} bool - Enables/disables the effect */
	static setActive(bool) {
		_active = bool;
	}
}

export default VerticalFlip;
