/**
 * Renderer/Effects/Shaders/Cartoon.js
 * Implementation of a Cel-Shaded/Cartoon effect.
 * Performs edge detection and color posterization.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import commonVS from './GLSL/Common.vs?raw';
import cartoonFS from './GLSL/Cartoon.fs?raw';

let _program, _buffer;
class Cartoon {
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !Cartoon.isActive()) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		gl.uniform1f(_program.uniform.uPower, GraphicsSettings.cartoonPower || 1.5);
		gl.uniform1f(_program.uniform.uEdgeSlope, GraphicsSettings.cartoonEdgeSlope || 1.5);
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
			_program = WebGL.createShaderProgram(gl, commonVS, cartoonFS);
		} catch (e) {
			console.error('Error compiling Cartoon shader.', e);
			return;
		}
		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	static isActive() {
		return GraphicsSettings.cartoonEnabled;
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
export default Cartoon;
