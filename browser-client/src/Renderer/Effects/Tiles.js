import WebGL from 'Utils/WebGL.js';
import Texture from 'Utils/Texture.js';
import Client from 'Core/Client.js';
import Configs from 'Core/Configs.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import _vertexShader from './Tiles.vs?raw';
import _fragmentShader from './Tiles.fs?raw';

export function loadTexture(gl, texture, cb) {
	const _texture = gl.createTexture();

	Client.loadFile(texture.filename, function (buffer) {
		Texture.load(buffer, function (canvas) {
			const enableMipmap = Configs.get('enableMipmap');
			const size = texture.size;
			canvas = document.createElement('canvas');
			canvas.width = canvas.height = size;
			const ctx = canvas.getContext('2d');
			ctx.save();
			ctx.translate(canvas.width / 2, canvas.height / 2);
			ctx.rotate(Math.PI);
			ctx.translate(-canvas.width / 2, -canvas.height / 2);
			ctx.drawImage(this, 0, 0, size, size);
			ctx.restore();
			gl.bindTexture(gl.TEXTURE_2D, _texture);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			if (enableMipmap) {
				gl.generateMipmap(gl.TEXTURE_2D);
			}
			cb(_texture);
		});
	});
}

export const FlatTexture = (textureFilename, size = 64) =>
	class {
		static get renderBeforeEntities() {
			return true;
		}

		static createShaderProgram(gl) {
			return WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		}

		static init(gl) {
			this._program = this.createShaderProgram(gl);
			this._buffer = gl.createBuffer();
			this._texture = null;
			gl.bindBuffer(gl.ARRAY_BUFFER, this._buffer);
			gl.bufferData(
				gl.ARRAY_BUFFER,
				new Float32Array([
					-1.0, -1.0, 0.0, 0.0, +1.0, -1.0, 1.0, 0.0, +1.0, +1.0, 1.0, 1.0, +1.0, +1.0, 1.0, 1.0, -1.0, +1.0,
					0.0, 1.0, -1.0, -1.0, 0.0, 0.0
				]),
				gl.STATIC_DRAW
			);
			loadTexture(
				gl,
				{
					filename: textureFilename,
					size
				},
				texture => {
					this._texture = texture;
					this.ready = true;
				}
			);
		}

		static free(gl) {
			if (this._texture) {
				gl.deleteTexture(this._texture);
				this._texture = null;
			}

			if (this._program) {
				gl.deleteProgram(this._program);
				this._program = null;
			}

			if (this._buffer) {
				gl.deleteBuffer(this._buffer);
				this._buffer = null;
			}

			this.ready = false;
		}

		static beforeRender(gl, modelView, projection, fog, tick) {
			const uniform = this._program.uniform;
			const attribute = this._program.attribute;
			gl.useProgram(this._program); // Bind matrix

			gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
			gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection); // Texture

			gl.activeTexture(gl.TEXTURE0);
			gl.bindTexture(gl.TEXTURE_2D, this._texture);
			gl.uniform1i(uniform.uDiffuse, 0); // Enable all attributes

			gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
			gl.uniform1f(uniform.uFogNear, fog.near);
			gl.uniform1f(uniform.uFogFar, fog.far);
			gl.uniform3fv(uniform.uFogColor, fog.color);

			gl.enableVertexAttribArray(attribute.aPosition);
			gl.enableVertexAttribArray(attribute.aTextureCoord);
			gl.bindBuffer(gl.ARRAY_BUFFER, this._buffer);
			gl.vertexAttribPointer(attribute.aPosition, 2, gl.FLOAT, false, 4 * 4, 0);
			gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 4, 2 * 4);
		}

		static afterRender(gl) {
			gl.disableVertexAttribArray(this._program.attribute.aPosition);
			gl.disableVertexAttribArray(this._program.attribute.aTextureCoord);
		}

		constructor(pos, startTick) {
			this.position = pos;
		}

		init() {
			this.ready = true;
		}

		free() {
			this.ready = false;
		}

		render(gl, tick) {
			gl.uniform3fv(this.constructor._program.uniform.uPosition, this.position);
			gl.bindBuffer(gl.ARRAY_BUFFER, this.constructor._buffer);

			SpriteRenderer.runWithDepth(false, false, false, function () {
				gl.drawArrays(gl.TRIANGLES, 0, 6);
			});
		}
	};

let _hoveringNum = 0;

export const HoveringTexture = (textureFilename, effectSize = 1, alpha = 1) =>
	class extends FlatTexture(textureFilename) {
		constructor() {
			super(...arguments);
			this.ix = ++_hoveringNum;
			this.effectSize = effectSize;
			this.alpha = alpha;
		}

		render(gl, tick) {
			const oddEven = this.ix % 2 === 0 ? Math.PI : 0;
			const heightMult = Math.sin(oddEven + tick / (540 * Math.PI));
			const position = [this.position[0], this.position[1], this.position[2] + 0.4 - 0.2 * heightMult];
			gl.uniform3fv(this.constructor._program.uniform.uPosition, position);
			gl.uniform1f(this.constructor._program.uniform.uSize, this.effectSize);
			gl.uniform1f(this.constructor._program.uniform.alpha, this.alpha);
			gl.bindBuffer(gl.ARRAY_BUFFER, this.constructor._buffer);
			gl.drawArrays(gl.TRIANGLES, 0, 6);
		}
	};
