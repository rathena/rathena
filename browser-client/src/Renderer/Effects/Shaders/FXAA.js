/**
 * Renderer/Effects/Shaders/FXAA.js
 * Implementation of Fast Approximate Anti-Aliasing (FXAA).
 * Smoothens jagged edges based on luma difference in a single pass.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import commonVS from './GLSL/Common.vs?raw';
import fxaaFS from './GLSL/FXAA.fs?raw';

let _program, _buffer;
class FXAA {
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !FXAA.isActive()) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		gl.uniform1f(_program.uniform.uSubpix, GraphicsSettings.fxaaSubpix || 0.25);
		gl.uniform1f(_program.uniform.uEdgeThreshold, GraphicsSettings.fxaaEdgeThreshold || 0.125);
		gl.uniform1f(_program.uniform.uEdgeThresholdMin, 0.0);
		gl.uniform2f(_program.uniform.uTexelSize, 1.0 / gl.canvas.width, 1.0 / gl.canvas.height);

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
			_program = WebGL.createShaderProgram(gl, commonVS, fxaaFS);
		} catch (e) {
			console.error('Error compiling FXAA shader.', e);
			return;
		}
		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	static isActive() {
		return GraphicsSettings.fxaaEnabled;
	}

	static program() {
		return _program;
	}

	static clean(gl) {
		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}
		_program = _buffer = null;
	}
}
export default FXAA;
