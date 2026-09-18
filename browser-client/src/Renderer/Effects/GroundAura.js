/**
 * Renderer/Effects/GroundAura.js
 *
 * Ground-aligned aura effect (flat on the ground, not a billboard)
 * This is a direct port of Aura.js but renders as horizontal quads
 * to avoid camera-tilt clipping issues.
 *
 */

import _vertexShader from './GroundAura.vs?raw';
import _fragmentShader from './GroundAura.fs?raw';
import WebGL from 'Utils/WebGL.js';
import Client from 'Core/Client.js';
import Altitude from 'Renderer/Map/Altitude.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';

/**
 * @var {WebGLProgram}
 */
let _program;

/**
 * @var {WebGLBuffer}
 */
let _buffer;

/**
 * Generate a flat quad on the XZ plane (Y=0, horizontal)
 * Unit size (-0.5 to 0.5), will be scaled by shader
 */
function generateGroundQuad() {
	return new Float32Array([
		// Triangle 1
		-0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0, -0.5, 1.0, 0.0, -0.5, 0.0, 0.5, 0.0, 1.0,
		// Triangle 2
		0.5, 0.0, -0.5, 1.0, 0.0, 0.5, 0.0, 0.5, 1.0, 1.0, -0.5, 0.0, 0.5, 0.0, 1.0
	]);
}

function calculateSize(self, aura, auraAngle, i) {
	const sinRiseAngle = self.sinCache[aura[i].riseAngle]
		? self.sinCache[aura[i].riseAngle]
		: (self.sinCache[aura[i].riseAngle] = Math.sin(aura[i].riseAngle));

	const riseFactor = aura[i].distance * 0.1 * (sinRiseAngle + 1);

	let cos = self.cosCache[auraAngle] ? self.cosCache[auraAngle] : (self.cosCache[auraAngle] = Math.cos(auraAngle));
	const startX = cos * (aura[i].distance * 0.8 + riseFactor);

	auraAngle += 90;
	if (auraAngle >= 360) {
		auraAngle -= 360;
	}
	cos = self.cosCache[auraAngle] ? self.cosCache[auraAngle] : (self.cosCache[auraAngle] = Math.cos(auraAngle));
	const endX = cos * (aura[i].distance * 0.8 + riseFactor);

	auraAngle += 90;
	if (auraAngle >= 360) {
		auraAngle -= 360;
	}
	let sin = self.sinCache[auraAngle] ? self.sinCache[auraAngle] : (self.sinCache[auraAngle] = Math.sin(auraAngle));
	const startY = sin * (aura[i].distance * 0.8 + riseFactor);

	auraAngle += 90;
	if (auraAngle >= 360) {
		auraAngle -= 360;
	}
	sin = self.sinCache[auraAngle] ? self.sinCache[auraAngle] : (self.sinCache[auraAngle] = Math.sin(auraAngle));
	const endY = sin * (aura[i].distance * 0.8 + riseFactor);

	const width = Math.abs(endX - startX);
	const height = Math.abs(endY - startY);

	return [width, height];
}

/**
 * GroundAura constructor
 */
class GroundAura {
	constructor(position, size, distance, textureName, tick) {
		this.position = position;
		this.textureName = textureName;
		this.tick = tick;
		this.distance = distance;
		this.size = size;
		this.minSize = size + distance;
		this.maxSize = size + distance * 2;

		this.aura = [{}, {}];
		this.aura[0].life = 1;
		this.aura[1].life = 1;

		this.aura[0].distance = 15.0;
		this.aura[1].distance = 15.0;

		this.aura[0].riseAngle = 0;
		this.aura[1].riseAngle = 0;

		this.aura[0].height = -1.0;
		this.aura[1].height = -1.0;

		this.aura[0].size = [this.minSize, this.minSize];
		this.aura[0].initialSize = [this.minSize, this.minSize];
		this.aura[1].size = [this.maxSize, this.maxSize];
		this.aura[1].initialSize = [this.maxSize, this.maxSize];

		this.aura[0].direction = 1;
		this.aura[1].direction = -1;

		this.cosCache = {};
		this.sinCache = {};
	}

	/**
	 * Initialize instance
	 */
	init(gl) {
		Client.loadFile(`data/texture/effect/${this.textureName}`, buffer => {
			WebGL.texture(gl, buffer, texture => {
				this.texture = texture;
				this.ready = true;
			});
		});
	}

	/**
	 * Free instance resources
	 */
	free(gl) {
		this.ready = false;
	}

	render(gl, tick) {
		const uniform = _program.uniform;

		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		for (let i = 0; i < this.aura.length; i++) {
			this.aura[i].riseAngle += 3;
			if (this.aura[i].riseAngle && !(this.aura[i].riseAngle % 180)) {
				this.aura[i].direction *= -1;
				// Avoid aura getting bigger and bigger or smaller and smaller over time
				if (
					(this.aura[i].direction < 0 && this.aura[i].size[0] < this.aura[i].initialSize[0]) ||
					(this.aura[i].direction > 0 && this.aura[i].size[0] > this.aura[i].initialSize[0])
				) {
					this.aura[i].size[0] = this.aura[i].initialSize[0];
					this.aura[i].size[1] = this.aura[i].initialSize[1];
				}
			}
			if (this.aura[i].riseAngle >= 360) {
				this.aura[i].riseAngle -= 360;
			}
		}

		// Get ground height
		const groundZ = Altitude.getCellHeight(this.position[0], this.position[1]);

		// World position with small lift to avoid z-fighting
		const worldPos = [this.position[0], this.position[1], groundZ + 0.05];

		gl.uniform3fv(uniform.uWorldPosition, worldPos);
		const self = this;
		SpriteRenderer.runWithDepth(true, false, false, function () {
			for (let i = 0; i < self.aura.length; i++) {
				if (!self.aura[i].life) {
					continue;
				}

				const auraAngle = i * 23;

				const sizeModifier = calculateSize(self, self.aura, auraAngle, i);

				self.aura[i].size[0] += (sizeModifier[0] * self.aura[i].direction) / (self.size / 2);
				self.aura[i].size[1] += (sizeModifier[1] * self.aura[i].direction) / (self.size / 2);

				// Set uniforms - size in SpriteRenderer units (shader converts to world units)
				gl.uniform2f(uniform.uSize, self.aura[i].size[0], self.aura[i].size[1]);
				gl.uniform1f(uniform.uAngle, (auraAngle * Math.PI) / 180);
				gl.uniform4f(uniform.uColor, 1.0, 1.0, 1.0, 0.8);
				gl.uniform1f(uniform.uZIndex, 1 + i);

				gl.drawArrays(gl.TRIANGLES, 0, 6);
			}
		});
	}

	/**
	 * Initialize static resources
	 */
	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

		const vertices = generateGroundQuad();
		_buffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

		this.ready = true;
		this.renderBeforeEntities = true;
	}

	/**
	 * Free static resources
	 */
	static free(gl) {
		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}
		if (_buffer) {
			gl.deleteBuffer(_buffer);
			_buffer = null;
		}
		this.ready = false;
	}

	/**
	 * Before render setup
	 */
	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE); // Additive blend

		gl.useProgram(_program);

		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

		// Fog
		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);

		// Bind vertex buffer and set attributes
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 5 * 4, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 5 * 4, 3 * 4);
	}

	/**
	 * After render cleanup
	 */
	static afterRender(gl) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
	}
}
export default GroundAura;
