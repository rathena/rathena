/**
 * Renderer/Effects/Cylinder.js
 *
 * Generate cone/cylinder/circle
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Camera from 'Renderer/Camera.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import _vertexShader from './Cylinder.vs?raw';
import _fragmentShader from './Cylinder.fs?raw';

/**
 * @var {WebGLProgram}
 */
let _program;

const blendMode = {};

/**
 * @var {mat4}
 */
const mat4 = glMatrix.mat4;

/**
 * @var {mat4} rotation matrix
 */
const _matrix = mat4.create();

/*
 * Sets how many sides the base "circle" of the cylinder has.
 * @var {number} Base circle side count
 */

/**
 * Generate a generic cylinder
 *
 * @returns {Float32Array} buffer array
 */
function generateCylinder(totalCircleSides, circleSides, repeatTextureX) {
	let i, a, b;
	const bottom = [];
	const top = [];
	const mesh = [];

	for (i = 0; i <= circleSides; i++) {
		a = (i + 0.0) / totalCircleSides;
		b = (i + 0.0) / totalCircleSides;

		bottom[i] = [
			Math.sin(a * Math.PI * 2),
			Math.cos(a * Math.PI * 2),
			0,
			a * (totalCircleSides / circleSides) * repeatTextureX,
			1
		];
		top[i] = [
			Math.sin(b * Math.PI * 2),
			Math.cos(b * Math.PI * 2),
			1,
			b * (totalCircleSides / circleSides) * repeatTextureX,
			0
		];
	}

	for (i = 0; i <= circleSides; i++) {
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
 * Cylinder constructor
 *
 * @param {Array} position
 * @param {Object} effect attributes
 * @param {number} Start tick
 * @param {number} End tick
 */
class Cylinder {
	constructor(effect, EF_Inst_Par, EF_Init_Par) {
		const position = EF_Inst_Par.position;
		const otherPosition = EF_Inst_Par.otherPosition;
		const direction = EF_Inst_Par.direction;
		const startTick = EF_Inst_Par.startTick;
		const endTick = EF_Inst_Par.endTick;

		this.semiCircle = effect.semiCircle ? false : true;

		this.totalCircleSides = !isNaN(effect.totalCircleSides) ? effect.totalCircleSides : 20;
		this.circleSides = !isNaN(effect.circleSides) ? effect.circleSides : this.totalCircleSides;

		this.color = new Float32Array([1.0, 1.0, 1.0, 1.0]);
		if (!isNaN(effect.red)) {
			this.color[0] = effect.red;
		}
		if (!isNaN(effect.green)) {
			this.color[1] = effect.green;
		}
		if (!isNaN(effect.blue)) {
			this.color[2] = effect.blue;
		}

		//copy position instead of reference
		this.position = position;
		this.otherPosition = otherPosition;

		this.posX = !isNaN(effect.posX) ? effect.posX : 0;
		this.posY = !isNaN(effect.posY) ? effect.posY : 0;
		this.posZ = !isNaN(effect.posZ) ? effect.posZ : 0;

		this.topSize = effect.topSize;
		this.bottomSize = effect.bottomSize;
		this.textureName = effect.textureName;
		this.height = effect.height;

		this.animation = effect.animation;
		this.fade = effect.fade;
		this.rotate = effect.rotate;

		if (effect.alphaMax > 0) {
			this.alphaMax = effect.alphaMax;
		} else {
			this.alphaMax = 1.0;
		}

		this.angleX = !isNaN(effect.angleX) ? effect.angleX : 0;
		this.angleY = !isNaN(effect.angleY) ? effect.angleY : 0;
		this.angleZ = !isNaN(effect.angleZ) ? effect.angleZ : 0;

		this.angleX += !isNaN(effect.angleXRandom) ? Math.floor(Math.random() * effect.angleXRandom) : 0;
		this.angleY += !isNaN(effect.angleYRandom) ? Math.floor(Math.random() * effect.angleYRandom) : 0;
		this.angleZ += !isNaN(effect.angleZRandom) ? Math.floor(Math.random() * effect.angleZRandom) : 0;

		this.repeatTextureX = !isNaN(effect.repeatTextureX) ? effect.repeatTextureX : 1;

		if (effect.rotateToTarget) {
			this.rotateToTarget = true;
			const x = this.otherPosition[0] - this.position[0];
			const y = this.otherPosition[1] - this.position[1];
			this.angleY += 90 - Math.atan2(y, x) * (180 / Math.PI);
		}

		if (effect.rotateWithSource) {
			this.rotateWithSource = true;
			this.angleY += 180 + direction * -45;
		}

		this.rotateWithCamera = effect.rotateWithCamera ? true : false;
		this.fixedPerspective = effect.fixedPerspective ? true : false;

		this.zIndex = !isNaN(effect.zIndex) ? effect.zIndex : 0;

		this.blendMode = effect.blendMode;
		this.startTick = startTick;
		this.endTick = endTick;

		this.repeat = effect.repeat;
	}

	/**
	 * Preparing for render
	 *
	 * @param {object} webgl context
	 */
	init(gl) {
		this.vertices = generateCylinder(this.totalCircleSides, this.circleSides, this.repeatTextureX);
		this.verticeCount = this.vertices.length / 5;

		this.buffer = gl.createBuffer();

		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.bufferData(gl.ARRAY_BUFFER, this.vertices, gl.STATIC_DRAW);

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
		const renderCount = tick - this.startTick;
		const duration = this.endTick - this.startTick;
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		if (this.repeatTextureX > 1) {
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
		}

		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);

		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 4 * 5, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 5, 3 * 4);

		gl.uniform1f(uniform.uBottomSize, this.bottomSize);
		gl.uniform1f(uniform.uZindex, this.zIndex);

		if (this.animation == 1) {
			if (duration > 1000) {
				if (renderCount <= 1000) {
					gl.uniform1f(uniform.uHeight, (renderCount / 1000) * this.height);
				} else {
					gl.uniform1f(uniform.uHeight, this.height);
				}
			} else {
				gl.uniform1f(uniform.uHeight, (renderCount / duration) * this.height);
			}
			gl.uniform1f(uniform.uTopSize, this.topSize);
		} else if (this.animation == 2) {
			gl.uniform1f(uniform.uHeight, this.height);
			if (duration > 1000) {
				if (renderCount <= 1000) {
					gl.uniform1f(uniform.uTopSize, (renderCount / 1000) * this.topSize);
				} else {
					gl.uniform1f(uniform.uTopSize, this.topSize);
				}
			} else {
				gl.uniform1f(uniform.uTopSize, (renderCount / duration) * this.topSize);
			}
		} else if (this.animation == 3) {
			gl.uniform1f(uniform.uHeight, this.height);
			gl.uniform1f(uniform.uBottomSize, (1 - renderCount / duration) * this.bottomSize);
			gl.uniform1f(uniform.uTopSize, (1 - renderCount / duration) * this.topSize);
			if (renderCount < duration / 2) {
				gl.uniform1f(uniform.uHeight, (renderCount * this.height) / (duration / 2));
			} else if (renderCount > duration / 2) {
				gl.uniform1f(uniform.uHeight, ((duration - renderCount) * this.height) / (duration / 2));
			}
		} else if (this.animation == 4) {
			gl.uniform1f(uniform.uHeight, this.height);
			let bottomSize = (renderCount / duration) * this.bottomSize;
			if (bottomSize < 0) {
				bottomSize = 0;
			}
			let topSize = (renderCount / duration) * this.topSize;
			if (topSize < 0) {
				topSize = 0;
			}
			gl.uniform1f(uniform.uBottomSize, bottomSize);
			gl.uniform1f(uniform.uTopSize, topSize);
		} else if (this.animation == 5) {
			if (renderCount < duration / 2) {
				gl.uniform1f(uniform.uHeight, ((renderCount * 2) / duration) * this.height);
			} else {
				gl.uniform1f(uniform.uHeight, ((duration - renderCount) * this.height) / (duration / 2));
			}
			gl.uniform1f(uniform.uTopSize, this.topSize);
		} else {
			gl.uniform1f(uniform.uHeight, this.height);
			gl.uniform1f(uniform.uTopSize, this.topSize);
		}

		this.color[3] = this.alphaMax;

		if (this.fade) {
			if (renderCount < duration / 4) {
				this.color[3] = (renderCount * this.alphaMax) / (duration / 4);
			} else if (renderCount > duration / 2 + duration / 4) {
				this.color[3] = ((duration - renderCount) * this.alphaMax) / (duration / 4);
			}
		}

		gl.uniform4fv(uniform.uSpriteRendererColor, this.color);

		if (this.blendMode > 0 && this.blendMode < 16) {
			gl.blendFunc(gl.SRC_ALPHA, blendMode[this.blendMode]);
		} else {
			gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		}

		const currentPosition = [this.position[0], this.position[1], this.position[2]];

		if (
			this.rotate ||
			this.angleX ||
			this.angleY ||
			this.angleZ ||
			this.rotateWithCamera ||
			this.fixedPerspective
		) {
			mat4.identity(_matrix);

			if (this.rotate) {
				mat4.rotateY(_matrix, _matrix, (tick / 4 / 180) * Math.PI);
			}

			if (this.angleX) {
				mat4.rotateX(_matrix, _matrix, (this.angleX / 180) * Math.PI);
			}
			if (this.angleY) {
				mat4.rotateY(_matrix, _matrix, (this.angleY / 180) * Math.PI);
			}
			if (this.angleZ) {
				mat4.rotateZ(_matrix, _matrix, (this.angleZ / 180) * Math.PI);
			}

			if (this.rotateWithCamera || this.fixedPerspective) {
				let magic = this.posY;

				if (this.fixedPerspective) {
					const vcRad = ((Camera.angle[0] - 180) * Math.PI) / 180;
					if (this.posZ) {
						currentPosition[2] += this.posZ * Math.cos(vcRad) - this.posY * Math.sin(vcRad);
						magic = this.posY * Math.sin(vcRad) + this.posZ * Math.sin(vcRad);
					}
					mat4.rotateX(_matrix, _matrix, vcRad);
				}

				const hcRad = (Camera.angle[1] * Math.PI) / 180;
				if (this.posX || this.posY) {
					currentPosition[0] += this.posX * Math.cos(hcRad) - magic * Math.sin(hcRad);
					currentPosition[1] += magic * Math.cos(hcRad) + this.posX * Math.sin(hcRad);
				}
				mat4.rotateY(_matrix, _matrix, hcRad);
			} else {
				currentPosition[0] += this.posX;
				currentPosition[1] += this.posY;
				currentPosition[2] += this.posZ;
			}

			gl.uniform1i(uniform.uRotate, true);
			gl.uniformMatrix4fv(uniform.uRotationMat, false, _matrix);
		} else {
			currentPosition[0] += this.posX;
			currentPosition[1] += this.posY;
			currentPosition[2] += this.posZ;
			gl.uniform1i(uniform.uRotate, false);
		}

		gl.uniform3fv(uniform.uPosition, currentPosition);

		const self = this;
		SpriteRenderer.runWithDepth(true, false, true, () => {
			gl.drawArrays(gl.TRIANGLES, 0, self.verticeCount);
		});

		this.needCleanUp = this.endTick < tick;
	}

	/**
	 * Initialize effect
	 *
	 * @param {object} webgl context
	 */
	static init(gl) {
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

		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

		this.ready = true;
		this.renderBeforeEntities = false;
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

		if (this.buffer) {
			gl.deleteBuffer(this.buffer);
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

		gl.useProgram(_program);

		// Bind matrix
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

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
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}
}

/**
 * @var {WebGLBuffer}
 */
Cylinder.buffer = null;

/**
 * @var {number}
 */
Cylinder.verticeCount = 0;

/**
 * Export
 */
export default Cylinder;
