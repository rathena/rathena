/**
 * Renderer/Effects/Shaders/GaussianBlur.js
 * Implementation of Radial Gaussian Blur.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import commonVS from './GLSL/Common.vs?raw';
import blurFS from './GLSL/GaussianBlur.fs?raw';

let _program, _buffer;

class GaussianBlur {
	/**
	 * Renders the Blur effect
	 * @param {WebGLRenderingContext} gl
	 * @param {WebGLTexture} inputTexture - Texture from previous pass
	 * @param {WebGLFramebuffer} outputFbo - Target buffer
	 */
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !GaussianBlur.isActive()) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		const focusRadius = GraphicsSettings.blurArea / 100;
		const focusFalloff = 0.5;

		gl.uniform1f(_program.uniform.uFocusRadius, focusRadius);
		gl.uniform1f(_program.uniform.uFocusFalloff, focusFalloff);

		const boxsampleFactor = GraphicsSettings.blurIntensity;

		gl.uniform2f(
			_program.uniform.uTexelSize,
			(1.0 / gl.canvas.width) * boxsampleFactor,
			(1.0 / gl.canvas.height) * boxsampleFactor
		);

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
			_program = WebGL.createShaderProgram(gl, commonVS, blurFS);
		} catch (e) {
			console.error('Error compiling Lens Blur shader.', e);
			return;
		}
		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	static isActive() {
		return GraphicsSettings.blur;
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
export default GaussianBlur;
