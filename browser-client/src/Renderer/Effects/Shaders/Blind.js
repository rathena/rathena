/**
 * Renderer/Effects/Shaders/Blind.js
 * Implementation of Radial Blindness effect.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import WebGL from 'Utils/WebGL.js';
import Camera from 'Renderer/Camera.js';
import commonVS from './GLSL/Common.vs?raw';
import blindFS from './GLSL/Blind.fs?raw';
import PostProcess from 'Renderer/Effects/PostProcess.js';

let _program, _buffer;
let _active = false;

class Blind {
	/**
	 * Renders the Blind effect
	 * @param {WebGLRenderingContext} gl
	 * @param {WebGLTexture} inputTexture - Texture from previous pass
	 * @param {WebGLFramebuffer} outputFbo - Target buffer
	 */
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !Blind.isActive()) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		const baseRadius = 0.2;
		const baseFalloff = 0.5;
		const zoom = Camera.zoomFinal;

		const focusRadius = baseRadius + (63 - zoom) / 1000;
		const focusFalloff = baseFalloff + (63 - zoom) / 1000;

		gl.uniform1f(_program.uniform.uFocusRadius, focusRadius);
		gl.uniform1f(_program.uniform.uFocusFalloff, focusFalloff);
		gl.uniform2f(_program.uniform.uAspectRatio, gl.canvas.width / gl.canvas.height, 1.0);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		const posLoc = _program.attribute.aPosition;
		gl.enableVertexAttribArray(posLoc);
		gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 0, 0);

		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, inputTexture);
		gl.uniform1i(_program.uniform.uTexture, 0);

		gl.drawArrays(gl.TRIANGLES, 0, 6);

		PostProcess.afterRenderPass(gl);
	}

	static init(gl) {
		if (!gl) {
			return;
		}
		try {
			_program = WebGL.createShaderProgram(gl, commonVS, blindFS);
		} catch (e) {
			console.error('Error compiling Blind shader.', e);
			return;
		}
		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	static isActive() {
		return _active;
	}

	static setActive(bool) {
		_active = bool;
	}

	static program() {
		return _program;
	}

	// No internal FBO needed for this effect in this architecture
	static clean(gl) {
		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}
		_program = _buffer = null;
	}
}
export default Blind;
