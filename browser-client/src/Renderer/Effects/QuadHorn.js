/**
 * Renderer/Effects/QuadHorn.js
 *
 * Generate Ice Wall effect like.
 *
 * @author MrUnzO
 */

import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import _vertexShader from './QuadHorn.vs?raw';
import _fragmentShader from './QuadHorn.fs?raw';

let _program;
const mat4 = glMatrix.mat4;

const blendMode = {};

const vertices = [
	0.0, 1.0, 0.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0,

	0.0, 1.0, 0.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0,

	0.0, 1.0, 0.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0,

	0.0, 1.0, 0.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0
];

const texCoords = [
	0, 0, 1, 0, 1, 1,

	0, 0, 1, 1, 0, 1,

	1, 0, 1, 1, 0, 1,

	1, 0, 0, 1, 0, 0
];

const rand = (min, max) => parseFloat(Math.min(min + Math.random() * (max - min), max).toFixed(3));

class QuadHorn {
	constructor(effect, EF_Inst_Par, EF_Init_Par) {
		this._zRotationMatrix = mat4.create();
		this._yRotationMatrix = mat4.create();
		this._xRotationMatrix = mat4.create();

		this.position = EF_Inst_Par.position;
		this.blendMode = effect.blendMode || 1;
		this.height =
			(effect.height && effect.height instanceof Array
				? rand(effect.height[0], effect.height[1])
				: effect.height) || 0.0;
		this.rotateX =
			(effect.rotateX && effect.rotateX instanceof Array
				? rand(effect.rotateX[0], effect.rotateX[1])
				: effect.rotateX) || 0.0;
		this.rotateY =
			(effect.rotateY && effect.rotateY instanceof Array
				? rand(effect.rotateY[0], effect.rotateY[1])
				: effect.rotateY) || 0.0;
		this.rotateZ =
			(effect.rotateZ && effect.rotateZ instanceof Array
				? rand(effect.rotateZ[0], effect.rotateZ[1])
				: effect.rotateZ) || 0.0;
		this.offsetX =
			(effect.offsetX && effect.offsetX instanceof Array
				? rand(effect.offsetX[0], effect.offsetX[1])
				: effect.offsetX) || 0.5;
		this.offsetY =
			(effect.offsetY && effect.offsetY instanceof Array
				? rand(effect.offsetY[0], effect.offsetY[1])
				: effect.offsetY) || 0.5;
		this.offsetZ =
			(effect.offsetZ && effect.offsetZ instanceof Array
				? rand(effect.offsetZ[0], effect.offsetZ[1])
				: effect.offsetZ) || 0.5;
		this.bottomSize =
			(effect.bottomSize && effect.bottomSize instanceof Array
				? rand(effect.bottomSize[0], effect.bottomSize[1])
				: effect.bottomSize) || 0.0;
		this.color = effect.color || [1.0, 1.0, 1.0, 1.0];
		this.animation = effect.animation || 0;
		this.animationSpeed = effect.animationSpeed || 100;
		this.animationOut = effect.animationOut || false;
		this.textureFile = effect.textureFile;
		this.startTick = EF_Inst_Par.startTick;
		this.endTick = EF_Inst_Par.endTick;
		this._endAnimation = effect.animation > 0 ? false : true;
	}

	init(gl) {
		this.buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

		this.texCoordBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texCoords), gl.STATIC_DRAW);

		const self = this;
		Client.loadFile('data/texture/' + this.textureFile, function (buffer) {
			WebGL.texture(gl, buffer, function (texture) {
				self.texture = texture;
				self.ready = true;
			});
		});
	}

	free(gl) {
		gl.deleteBuffer(this.buffer);
		this.ready = false;
	}

	render(gl, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;
		const deltaStart = (tick - this.startTick) / 1000.0;
		const deltaEnd = (tick - this.endTick) / 1000.0;

		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		gl.enableVertexAttribArray(attribute.aColor);

		if (this.blendMode > 0 && this.blendMode < 16) {
			gl.blendFunc(gl.SRC_ALPHA, blendMode[this.blendMode]);
		} else {
			gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		}

		if (this.animation === 1 && !this._endAnimation) {
			//Grow up
			const lerpHeight = deltaStart / (this.animationSpeed / 1000);
			if (lerpHeight > this.height) {
				this._endAnimation = true;
				gl.uniform1f(uniform.uHeight, this.height);
			} else {
				gl.uniform1f(uniform.uHeight, lerpHeight);
			}
			gl.uniform1f(uniform.uOffsetZ, this.offsetZ);
		} else if (this.animation === 2 && !this._endAnimation) {
			//Move up
			const lerpZOffset = deltaStart / (this.animationSpeed / 1000);
			if (lerpZOffset > this.offsetZ) {
				this._endAnimation = true;
				gl.uniform1f(uniform.uOffsetZ, this.offsetZ);
			} else {
				gl.uniform1f(uniform.uOffsetZ, lerpZOffset);
			}
			gl.uniform1f(uniform.uHeight, this.height);
		} else if (this.animation === 3 && !this._endAnimation) {
			//Move up and over the ground a bit to make it feel more attack power
			const lerpZOffset = deltaStart / (this.animationSpeed / 1000);
			if (lerpZOffset > this.height / 2) {
				this._endAnimation = true;
				gl.uniform1f(uniform.uOffsetZ, this.offsetZ);
			} else {
				gl.uniform1f(uniform.uOffsetZ, lerpZOffset);
			}
			gl.uniform1f(uniform.uHeight, this.height);
		} else {
			gl.uniform1f(uniform.uHeight, this.height);
			gl.uniform1f(uniform.uOffsetZ, this.offsetZ);
		}

		if (this.endTick > 0 && this.endTick < tick) {
			if (this.animationOut && this._endAnimation) {
				const lerpZOffset = -((deltaEnd / (this.animationSpeed / 1000)) * this.height);
				if (lerpZOffset < -(this.height + this.offsetZ)) {
					this.needCleanUp = this.endTick < tick;
				}
				gl.uniform1f(uniform.uOffsetZ, lerpZOffset);
			} else {
				this.needCleanUp = this.endTick < tick;
			}
		}

		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 0, 0);
		gl.bindBuffer(gl.ARRAY_BUFFER, null);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 0, 0);
		gl.bindBuffer(gl.ARRAY_BUFFER, null);

		gl.uniform3fv(uniform.uPosition, this.position);

		gl.uniform1f(uniform.uBottomSize, this.bottomSize);
		gl.uniform1f(uniform.uOffsetX, this.offsetX);
		gl.uniform1f(uniform.uOffsetY, this.offsetY);

		gl.uniform4fv(uniform.uColor, this.color);

		mat4.identity(this._xRotationMatrix);
		mat4.rotate(this._xRotationMatrix, this._xRotationMatrix, (this.rotateX * Math.PI) / 180, [1, 0, 0]);
		gl.uniformMatrix4fv(uniform.uXRotationMat, false, this._xRotationMatrix);

		mat4.identity(this._yRotationMatrix);
		mat4.rotate(this._yRotationMatrix, this._yRotationMatrix, (this.rotateY * Math.PI) / 180, [0, 1, 0]);
		gl.uniformMatrix4fv(uniform.uYRotationMat, false, this._yRotationMatrix);

		mat4.identity(this._zRotationMatrix);
		mat4.rotate(this._zRotationMatrix, this._zRotationMatrix, ((180 + this.rotateZ) * Math.PI) / 180, [0, 0, 1]);
		gl.uniformMatrix4fv(uniform.uZRotationMat, false, this._zRotationMatrix);

		gl.drawArrays(gl.TRIANGLES, 0, 12);
		gl.flush();
	}

	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

		blendMode[1] = gl.ZERO;
		blendMode[2] = gl.ONE;
		blendMode[3] = gl.SRC_COLOR;
		blendMode[4] = gl.ONE_MINUS_SRC_COLOR;
		blendMode[5] = gl.DST_COLOR;
		blendMode[6] = gl.ONE_MINUS_DST_COLOR;
		blendMode[7] = gl.SRC_ALPHA;
		blendMode[8] = gl.ONE_MINUS_SRC_ALPHA;
		blendMode[9] = gl.DST_ALPHA;
		blendMode[10] = gl.ONE_MINUS_DST_ALPHA;
		blendMode[11] = gl.CONSTANT_COLOR;
		blendMode[12] = gl.ONE_MINUS_CONSTANT_COLOR;
		blendMode[13] = gl.CONSTANT_ALPHA;
		blendMode[14] = gl.ONE_MINUS_CONSTANT_ALPHA;
		blendMode[15] = gl.SRC_ALPHA_SATURATE;

		this.ready = true;
		this.renderBeforeEntities = true;
	}

	static free(gl) {
		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}
		if (this.buffer) {
			gl.deleteBuffer(this.buffer);
		}

		this.ready = false;
	}

	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;

		gl.useProgram(_program);

		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);
	}

	static afterRender(gl) {
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
		gl.disableVertexAttribArray(_program.attribute.aColor);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}
}

export default QuadHorn;
