/**
 * Renderer/Effects/MagicRing.js
 *
 * Generate a Magic Ring under caster
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import _vertexShader from './MagicRing.vs?raw';
import _fragmentShader from './MagicRing.fs?raw';

/**
 * @var {WebGLProgram}
 */
let _program;

/**
 * @var {WebGLBuffer}
 */
let _buffer;

/**
 * @var {mat4}
 */
const mat4 = glMatrix.mat4;

/**
 * @var {mat4} rotation matrix
 */
const _matrix = mat4.create();

/**
 * @var {number}
 */
let _verticeCount = 0;

/**
 * Generate a generic MagicRing
 *
 * @returns {Float32Array} buffer array
 */
function generateMagicRing() {
	let i, a, b;
	const total = 20;
	const bottom = [];
	const top = [];
	const mesh = [];

	for (i = 0; i <= total; i++) {
		a = (i + 0.0) / total;
		b = (i + 0.5) / total;

		bottom[i] = [Math.sin(a * Math.PI * 2), Math.cos(a * Math.PI * 2), 0, a, 1];
		top[i] = [Math.sin(b * Math.PI * 2), Math.cos(b * Math.PI * 2), 1, b, 0];
	}

	for (i = 0; i <= total; i++) {
		mesh.push.apply(mesh, bottom[i + 0]);
		mesh.push.apply(mesh, top[i + 0]);
		mesh.push.apply(mesh, bottom[i + 1]);

		mesh.push.apply(mesh, top[i + 0]);
		mesh.push.apply(mesh, bottom[i + 1]);
		mesh.push.apply(mesh, top[i + 1]);
	}

	return new Float32Array(mesh);
}

/**
 * MagicRing constructor
 *
 * @param {Array} position
 * @param {number} top size of the MagicRing
 * @param {number} bottom size of the MagicRing
 * @param {number} height of the MagicRing
 * @param {string} texture name
 * @param {number} game tick
 */
class MagicRing {
	constructor(target, topSize, bottomSize, height, textureName, endTick) {
		this.target = target;
		this.topSize = topSize;
		this.bottomSize = bottomSize;
		this.textureName = textureName;
		this.height = height;
		this.endTick = endTick;
	}

	/**
	 * Preparing for render
	 *
	 * @param {object} webgl context
	 */
	init(gl) {
		Client.loadFile(`data/texture/effect/${this.textureName}.tga`, buffer => {
			WebGL.texture(gl, buffer, texture => {
				this.texture = texture;
				this.ready = true;
			});
		});
	}

	/**
	 * Destroying data
	 *
	 * @param {object} webgl context
	 */
	free(gl) {
		this.ready = false;
	}

	/**
	 * Rendering cast
	 *
	 * @param {object} wegl context
	 */
	render(gl, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		// Enable all attributes
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 4 * 5, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 5, 3 * 4);

		gl.uniform3fv(uniform.uPosition, this.target.position);
		gl.uniform1f(uniform.uBottomSize, this.bottomSize);
		gl.uniform1f(uniform.uTopSize, this.topSize);
		gl.uniform1f(uniform.uHeight, this.height);

		gl.drawArrays(gl.TRIANGLES, 0, _verticeCount);
		this.needCleanUp = this.endTick < tick;
		if (!this.target.cast.display) {
			this.needCleanUp = true;
		}
	}

	/**
	 * Initialize effect
	 *
	 * @param {object} webgl context
	 */
	static init(gl) {
		const vertices = generateMagicRing();
		_verticeCount = vertices.length / 5;

		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		_buffer = gl.createBuffer();
		this.ready = true;
		this.renderBeforeEntities = false;

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
	}

	/**
	 * Destroy objects
	 *
	 * @param {object} webgl context
	 */
	static free(gl) {
		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}

		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}

		this.ready = false;
	}

	/**
	 * Before render, set up program
	 *
	 * @param {object} webgl context
	 */
	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;

		mat4.identity(_matrix);
		mat4.rotateY(_matrix, _matrix, (tick / 4 / 180) * Math.PI);

		gl.useProgram(_program);

		// Bind matrix
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
		gl.uniformMatrix4fv(uniform.uRotationMat, false, _matrix);

		// Fog settings
		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		// Texture
		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);
	}

	/**
	 * After render, clean attributes
	 *
	 * @param {object} webgl context
	 */
	static afterRender(gl) {
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
	}
}
/**
 * Export
 */
export default MagicRing;
