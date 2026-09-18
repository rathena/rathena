/**
 * Renderer/Effects/Shaders/Vibrance.js
 * Implementation of the Vibrance post-processing effect.
 * Adjusts color saturation based on existing saturation levels.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';
import WebGL from 'Utils/WebGL.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import commonVS from './GLSL/Common.vs?raw';
import vibranceFS from './GLSL/Vibrance.fs?raw';

let _program, _buffer;
class Vibrance {
	static render(gl, inputTexture, outputFbo) {
		if (!_buffer || !_program || !Vibrance.isActive()) {
			return;
		}

		PostProcess.beforeRenderPass(gl, outputFbo);

		gl.useProgram(_program);

		gl.uniform1f(_program.uniform.uVibrance, GraphicsSettings.vibrance || 0.15);
		gl.uniform3f(_program.uniform.uVibranceRGBBalance, 1.0, 1.0, 1.0);

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
			_program = WebGL.createShaderProgram(gl, commonVS, vibranceFS);
		} catch (e) {
			console.error('Error compiling Vibrance shader.', e);
			return;
		}
		const quadVertices = new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]);
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, quadVertices, gl.STATIC_DRAW);
	}

	static isActive() {
		return GraphicsSettings.vibranceEnabled;
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
export default Vibrance;
