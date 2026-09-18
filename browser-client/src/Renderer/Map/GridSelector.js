/**
 * Renderer/Map/GridSelectior.js
 *
 * Rendering Grid Selector
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Altitude from 'Renderer/Map/Altitude.js';
import Client from 'Core/Client.js';
import WebGL from 'Utils/WebGL.js';
import Texture from 'Utils/Texture.js';
import Configs from 'Core/Configs.js';
import _vertexShader from './GridSelector.vs?raw';
import _fragmentShader from './GridSelector.fs?raw';

/**
 * @var {WebGLProgram}
 */
let _program = null;

/**
 * param {WebGLBuffer}
 */
let _buffer = null;

/**
 * @var {WebGLTexture} texture of the grid
 */
let _texture = null;

/**
 * @var {string} last position rendered
 */
let _xy = null;

/**
 * WebGL buffer array
 * x, y, z, u, v
 */
const _buffer_data = new Float32Array([
	0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0
]);

/**
 * Initialize Grid
 *
 * @param {object} gl context
 */
function init(gl) {
	Client.loadFile('data/texture/grid.tga', function (buffer) {
		Texture.load(buffer, function (success) {
			if (!success) {
				return;
			}

			const canvas = document.createElement('canvas');
			const ctx = canvas.getContext('2d');
			const enableMipmap = Configs.get('enableMipmap');

			canvas.width = WebGL.toPowerOfTwo(this.width);
			canvas.height = WebGL.toPowerOfTwo(this.height);

			ctx.globalAlpha = 0.6;
			ctx.drawImage(this, 0, 0, canvas.width, canvas.height);
			ctx.fillStyle = 'rgb( 50, 240, 160)';
			ctx.globalCompositeOperation = 'source-atop';
			ctx.fillRect(0, 0, canvas.width, canvas.height);

			_texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, _texture);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			if (enableMipmap) {
				gl.generateMipmap(gl.TEXTURE_2D);
			}
		});
	});

	_buffer = gl.createBuffer();
	_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

	// Allocate buffer memory, data will be sent later
	gl.bufferData(gl.ARRAY_BUFFER, _buffer_data.byteLength, gl.DYNAMIC_DRAW);
}

/**
 * Render Grid Selection
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {object} fog structure
 * @param {number} x
 * @param {number} y
 */
function render(gl, modelView, projection, fog, x, y) {
	// Texture not loaded yet
	if (!_texture) {
		return;
	}

	const uniform = _program.uniform;
	const attribute = _program.attribute;
	let z;

	gl.useProgram(_program);

	// Bind matrix
	gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

	// Fog settings
	gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
	gl.uniform1f(uniform.uFogNear, fog.near);
	gl.uniform1f(uniform.uFogFar, fog.far);
	gl.uniform3fv(uniform.uFogColor, fog.color);

	gl.enableVertexAttribArray(attribute.aPosition);
	gl.enableVertexAttribArray(attribute.aTextCoord);
	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

	// Link attributes
	gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 5 * 4, 0);
	gl.vertexAttribPointer(attribute.aTextCoord, 2, gl.FLOAT, false, 5 * 4, 3 * 4);

	// Textures
	gl.activeTexture(gl.TEXTURE0);
	gl.uniform1i(uniform.uDiffuse, 0);

	// Update buffer only if there is a change
	if (_xy !== x + '' + y) {
		_xy = x + '' + y;
		z = Altitude.getCell(x, y);

		_buffer_data[0] = _buffer_data[10] = x + 0;
		_buffer_data[2] = _buffer_data[7] = y + 0;
		_buffer_data[5] = _buffer_data[15] = x + 1;
		_buffer_data[12] = _buffer_data[17] = y + 1;
		_buffer_data[1] = z[0];
		_buffer_data[6] = z[1];
		_buffer_data[11] = z[2];
		_buffer_data[16] = z[3];

		// Send data to buffer
		gl.bufferSubData(gl.ARRAY_BUFFER, 0, _buffer_data);
	}

	// Send mesh
	gl.bindTexture(gl.TEXTURE_2D, _texture);
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

	// Is it needed ?
	gl.disableVertexAttribArray(attribute.aPosition);
	gl.disableVertexAttribArray(attribute.aTextCoord);
}

/**
 * Clean texture/buffer from memory
 *
 * @param {object} gl context
 */
function free(gl) {
	if (_buffer) {
		gl.deleteBuffer(_buffer);
		_buffer = null;
	}

	if (_texture) {
		gl.deleteTexture(_texture);
		_texture = null;
	}

	if (_program) {
		gl.deleteProgram(_program);
		_program = null;
	}
}

/**
 * Export
 */
export default {
	init: init,
	free: free,
	render: render
};
