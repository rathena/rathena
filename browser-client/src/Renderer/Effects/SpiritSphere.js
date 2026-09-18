import WebGL from 'Utils/WebGL.js';
import Texture from 'Utils/Texture.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Camera from 'Renderer/Camera.js';
import Configs from 'Core/Configs.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import _vertexShader from './SpiritSphere.vs?raw';
import _fragmentShader from './SpiritSphere.fs?raw';

// Load dependencies
/**
 * @var {WebGLTexture}
 */
let _texture;

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

const _rotationMatrices = (function () {
	const matrices = [];
	for (let i = 0; i < 5; i++) {
		matrices.push({
			posMat: mat4.create(),
			texMat: mat4.create()
		});
	}
	return matrices;
})();

const _textureMatrix = mat4.create();

class SpiritSphere {
	constructor(entity, num, isCoin) {
		this.position = entity.position;
		this.num = num;
		this.isCoin = isCoin;

		this.initialAlpha = 0;
	}

	init(gl) {
		this.ready = true;
	}

	free(gl) {
		this.ready = false;
	}

	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		_buffer = gl.createBuffer();

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(
			gl.ARRAY_BUFFER,
			new Float32Array([
				-1.0, -1.0, 0.0, 0.0, +1.0, -1.0, 1.0, 0.0, +1.0, +1.0, 1.0, 1.0, +1.0, +1.0, 1.0, 1.0, -1.0, +1.0, 0.0,
				1.0, -1.0, -1.0, 0.0, 0.0
			]),
			gl.STATIC_DRAW
		);

		Client.loadFile('data/texture/effect/thunder_center.bmp', buffer => {
			Texture.load(buffer, function () {
				const enableMipmap = Configs.get('enableMipmap');
				const ctx = this.getContext('2d');
				ctx.save();
				ctx.translate(this.width / 2, this.height / 2);
				// ctx.rotate( 45 / 180 * Math.PI);
				ctx.translate(-this.width / 2, -this.height / 2);
				ctx.drawImage(this, 0, 0);
				ctx.restore();

				_texture = gl.createTexture();
				gl.bindTexture(gl.TEXTURE_2D, _texture);
				gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this);
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
				if (enableMipmap) {
					gl.generateMipmap(gl.TEXTURE_2D);
				}

				SpiritSphere.ready = true;
			});
		});
	}

	static free(gl) {
		if (_texture) {
			gl.deleteTexture(_texture);
			_texture = null;
		}

		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}

		if (_buffer) {
			gl.deleteBuffer(_buffer);
		}

		this.ready = false;
	}

	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;
		gl.useProgram(_program);

		let _matrix, offset;
		for (let i = 0, _len = _rotationMatrices.length; i < _len; i++) {
			const vcRad = ((Camera.angle[0] - 90) * Math.PI) / 180;
			const hcRad = (Camera.angle[1] * Math.PI) / 180;
			offset = (i * 2 * Math.PI) / _rotationMatrices.length;
			const rotRad = offset - (tick / 64 / 180) * Math.PI;

			//_matrix = _rotationMatrices[i].texMat;
			//mat4.identity(_matrix);
			const textureMatrix = mat4.create();
			mat4.rotateX(_rotationMatrices[i].texMat, textureMatrix, vcRad);
			mat4.rotateY(_rotationMatrices[i].texMat, _rotationMatrices[i].texMat, hcRad - rotRad);

			_matrix = _rotationMatrices[i].posMat;
			mat4.identity(_matrix);
			mat4.rotateY(_matrix, _matrix, rotRad);
		}

		// Bind matrix
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

		// Texture
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, _texture);
		gl.uniform1i(uniform.uDiffuse, 0);

		// Enable all attributes
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		gl.vertexAttribPointer(attribute.aPosition, 2, gl.FLOAT, false, 4 * 4, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 4, 2 * 4);
	}

	render(gl, tick) {
		const uniform = _program.uniform;

		gl.uniform3fv(uniform.uPosition, this.position);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		gl.blendFunc(gl.SRC_ALPHA, gl.ONE);

		gl.uniform1f(uniform.uCameraZoom, Camera.zoom);

		SpriteRenderer.runWithDepth(true, false, false, () => {
			let _matrix;
			for (let i = this.num; i > 0; i--) {
				_matrix = _rotationMatrices[i % _rotationMatrices.length];

				gl.uniformMatrix4fv(uniform.uTextureRotMat, false, _matrix.texMat);
				gl.uniformMatrix4fv(uniform.uRotationMat, false, _matrix.posMat);

				if (i > 10) {
					if (this.isCoin) {
						gl.uniform1f(uniform.uSize, 0.3);
						gl.uniform4fv(uniform.uColor, [1.0, 0.9, 0.4, 0.2 * this.initialAlpha]);
					} else {
						gl.uniform1f(uniform.uSize, 0.55);
						gl.uniform4fv(uniform.uColor, [0.0, 0.0, 1.0, 0.2 * this.initialAlpha]);
					}

					gl.uniform1f(uniform.uZIndex, 0.0);
					gl.drawArrays(gl.TRIANGLES, 0, 6);
				} else if (i > 5) {
					if (this.isCoin) {
						gl.uniform1f(uniform.uSize, 0.2);
						gl.uniform4fv(uniform.uColor, [1.0, 0.9, 0.4, 0.4 * this.initialAlpha]);
					} else {
						gl.uniform1f(uniform.uSize, 0.35);
						gl.uniform4fv(uniform.uColor, [0.0, 0.0, 1.0, 0.6 * this.initialAlpha]);
					}

					gl.uniform1f(uniform.uZIndex, 0.01);
					gl.drawArrays(gl.TRIANGLES, 0, 6);
				} else {
					if (this.isCoin) {
						gl.uniform1f(uniform.uSize, 0.1);
						gl.uniform4fv(uniform.uColor, [1.0, 0.9, 0.4, 0.6 * this.initialAlpha]);
					} else {
						gl.uniform1f(uniform.uSize, 0.25);
						gl.uniform4fv(uniform.uColor, [0.0, 0.0, 1.0, 1.0 * this.initialAlpha]);
					}
					gl.uniform1f(uniform.uZIndex, 0.02);
					gl.drawArrays(gl.TRIANGLES, 0, 6);

					if (this.isCoin) {
						gl.uniform1f(uniform.uSize, 0.05);
						gl.uniform4fv(uniform.uColor, [1.0, 1.0, 0.7, 1.0 * this.initialAlpha]);
					} else {
						gl.uniform1f(uniform.uSize, 0.15);
						gl.uniform4fv(uniform.uColor, [0.8, 0.8, 1.0, 1.0 * this.initialAlpha]);
					}
					gl.uniform1f(uniform.uZIndex, 0.03);
					gl.drawArrays(gl.TRIANGLES, 0, 6);
				}
			}
		});

		if (this.initialAlpha < 1) {
			this.initialAlpha = Math.min(this.initialAlpha + 0.005, 1);
		}
	}

	static afterRender(gl) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
	}
}

SpiritSphere.renderBeforeEntities = false;

/**
 * Export
 */
export default SpiritSphere;
