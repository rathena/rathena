/**
 * Renderer/Effects/SwirlingAura.js
 *
 * Swirling aura band effect
 * Based on original inspiration game Level99()
 *
 * From original inspiration game:
 * - Three bands with different parameters:
 *   GI[0]: RotStart=0°, max_height=15, distance=3.9, rise_angle=55°
 *   GI[1]: RotStart=90°, max_height=13, distance=4.1, rise_angle=50°
 *   GI[2]: RotStart=180°, max_height=11, distance=4.3, rise_angle=45°
 * - Spin speed: RotStart += (ec + 3) per frame (3°, 4°, 5° per band)
 * - 21 divisions, 315° arc
 * - Height profile: height[i] = max_height * sin(SinLimit) * sin(process°)
 *   where SinLimit = 90° + (i - 10) * 9°
 * - Render: base ring at distance, top offset by rotated height
 */

import _vertexShader from './SwirlingAura.vs?raw';
import _fragmentShader from './SwirlingAura.fs?raw';
import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Altitude from 'Renderer/Map/Altitude.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';

const mat4 = glMatrix.mat4;

/**
 * @var {WebGLProgram}
 */
let _program;

/**
 * @var {mat4}
 */
const _modelMatrix = mat4.create();

/**
 * Constants from original inspiration game
 */
const E_DIVISION = 21; // Number of divisions (0-20)
const FULL_DISPLAY_ANGLE = 315; // 315° arc
const DEG_TO_RAD = Math.PI / 180;
const STRIDE = 5; // x, y, z, u, v
const VERTICES_PER_BAND = E_DIVISION * 2;

/**
 * SwirlingAura constructor
 *
 * Creates THREE bands as per original inspiration game:
 * - GI[0]: RotStart=0°, max_height=15, distance=3.9, rise_angle=55°
 * - GI[1]: RotStart=90°, max_height=13, distance=4.1, rise_angle=50°
 * - GI[2]: RotStart=180°, max_height=11, distance=4.3, rise_angle=45°
 */
class SwirlingAura {
	constructor(position, textureName, tick, sizeType) {
		this.position = position;
		this.textureName = textureName;
		this.tick = tick;
		this.sizeType = sizeType || 4; // 4 = blue, 7 = green

		// Scale factor: original inspiration game units to world units
		const GAME_TO_WORLD = 0.1 * 2.2; // Adjusted for visual match

		// Color based on m_size (4 = blue, 7 = green)
		if (this.sizeType === 7) {
			this.color = { r: 100 / 255, g: 255 / 255, b: 100 / 255 };
		} else {
			this.color = { r: 100 / 255, g: 100 / 255, b: 255 / 255 };
		}

		// Alpha from original inspiration game: alphaB = 120
		this.alphaB = 120 / 255;

		// Create THREE bands with different parameters
		const INNER_CIRCLE_SCALE = 0.6;
		this.bands = [];
		for (let ec = 0; ec < 3; ec++) {
			this.bands.push({
				life: 1,
				process: 0,
				rotStart: ec * 90, // 0, 90, 180 degrees
				maxHeight: (15 - 2 * ec) * GAME_TO_WORLD, // 15, 13, 11 (unchanged)
				distance: (3.9 + 0.2 * ec) * GAME_TO_WORLD * INNER_CIRCLE_SCALE, // 20% smaller
				riseAngle: (55 - 5 * ec) * DEG_TO_RAD, // 55°, 50°, 45°
				spinSpeed: ec + 3, // 3, 4, 5 degrees per frame
				height: new Float32Array(E_DIVISION), // Height profile
				flag1: new Uint8Array(E_DIVISION) // Reached max flag
			});
		}

		// Angle step between divisions
		this.basicAngle = FULL_DISPLAY_ANGLE / (E_DIVISION - 1); // 315/20 = 15.75°

		// Vertex data for a single band (GC evasion uses dynamic vertex data)
		this.vertices = new Float32Array(VERTICES_PER_BAND * STRIDE);

		// Vertex buffers for each band
		this.buffers = null;
		this.indexBuffer = null;
		this.indexCount = 0;
		this.ready = false;
	}

	/**
	 * Update height profile for a band
	 */
	updateHeightProfile(band) {
		const middle = 10;
		const step = 9; // 90 / 10 = 9 degrees

		for (let i = 0; i < E_DIVISION; i++) {
			if (band.flag1[i] === 0) {
				// SinLimit = 90° + (i - middle) * step
				const sinLimit = (90 + (i - middle) * step) * DEG_TO_RAD;
				const sinLimitValue = Math.sin(sinLimit);
				const maxPossible = band.maxHeight * sinLimitValue;

				if (band.process <= 90) {
					// Build up phase: height = max_height * sin(SinLimit) * sin(process°)
					const sinProcess = Math.sin(band.process * DEG_TO_RAD);
					band.height[i] = band.maxHeight * sinLimitValue * sinProcess;
				}

				// Clamp to [0, maxPossible]
				band.height[i] = Math.max(0, Math.min(band.height[i], maxPossible));

				// Mark as reached max
				if (band.height[i] >= maxPossible * 0.99) {
					band.flag1[i] = 1;
				}
			}
		}
	}

	/**
	 * fill mesh for a single band
	 */
	fillBandMesh(band) {
		const verts = this.vertices;
		const cosRise = Math.cos(band.riseAngle);
		const sinRise = Math.sin(band.riseAngle);
		let offset = 0;

		for (let k = 0; k < E_DIVISION; k++) {
			// Calculate angle for this division
			const angle = (band.rotStart + k * this.basicAngle) * DEG_TO_RAD;
			const cosAngle = Math.cos(angle);
			const sinAngle = Math.sin(angle);

			// Calculate base and top vertices
			const baseX = band.distance * cosAngle;
			const baseZ = band.distance * sinAngle;
			const h = band.height[k];

			// Calculate top vertex offset based on rise angle
			const Rx = cosRise * h;
			const Ry = sinRise * h;

			// Top vertex position
			const topX = baseX + Rx * cosAngle;
			const topY = -Ry;
			const topZ = baseZ + Rx * sinAngle;

			// Texture coordinate u based on division index
			const u = k / (E_DIVISION - 1);

			// Vértice Base (Inner)
			verts[offset++] = baseX;
			verts[offset++] = 0;
			verts[offset++] = baseZ;
			verts[offset++] = u;
			verts[offset++] = 1.0;

			// Vértice Top (Outer)
			verts[offset++] = topX;
			verts[offset++] = topY;
			verts[offset++] = topZ;
			verts[offset++] = u;
			verts[offset++] = 0.0;
		}
	}

	/**
	 * Generate index buffer
	 */
	generateIndices() {
		const indices = [];
		for (let k = 0; k < E_DIVISION - 1; k++) {
			const i0 = k * 2; // Base of current
			const i1 = k * 2 + 1; // Top of current
			const i2 = k * 2 + 2; // Base of next
			const i3 = k * 2 + 3; // Top of next
			// Two triangles: (i0, i1, i2) and (i1, i3, i2)
			indices.push(i0, i1, i2, i1, i3, i2);
		}
		return new Uint16Array(indices);
	}

	/**
	 * Initialize instance
	 */
	init(gl) {
		// Create vertex buffers for each band
		this.buffers = [];
		for (let i = 0; i < this.bands.length; i++) {
			const buf = gl.createBuffer();
			gl.bindBuffer(gl.ARRAY_BUFFER, buf);
			// Allocate buffer with initial size (will be updated each frame)
			gl.bufferData(gl.ARRAY_BUFFER, this.vertices.byteLength, gl.DYNAMIC_DRAW);
			this.buffers.push(buf);
		}

		// Create shared index buffer
		const indices = this.generateIndices();
		this.indexBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indexBuffer);
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);
		this.indexCount = indices.length;

		// Load texture
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
		if (this.buffers) {
			for (let i = 0; i < this.buffers.length; i++) {
				gl.deleteBuffer(this.buffers[i]);
			}
			this.buffers = null;
		}
		if (this.indexBuffer) {
			gl.deleteBuffer(this.indexBuffer);
			this.indexBuffer = null;
		}
		this.ready = false;
	}

	/**
	 * Render all three bands
	 */
	render(gl, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		// Get ground height
		const groundZ = Altitude.getCellHeight(this.position[0], this.position[1]);

		// Build model matrix
		mat4.identity(_modelMatrix);
		mat4.translate(_modelMatrix, _modelMatrix, [this.position[0] + 0.5, -groundZ, this.position[1] + 0.5]);

		gl.uniformMatrix4fv(uniform.uModelMat, false, _modelMatrix);
		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		const self = this;
		SpriteRenderer.runWithDepth(true, false, false, function () {
			// Render each band
			for (let ec = 0; ec < self.bands.length; ec++) {
				const band = self.bands[ec];
				if (!band.life) {
					continue;
				}

				// Update animation (Prim3DCasting)
				band.process++;
				band.rotStart = (band.rotStart + band.spinSpeed) % 360;

				// Update height profile
				self.updateHeightProfile(band);

				// fill mesh for this band
				self.fillBandMesh(band);

				// Upload vertex data
				gl.bindBuffer(gl.ARRAY_BUFFER, self.buffers[ec]);
				gl.bufferSubData(gl.ARRAY_BUFFER, 0, self.vertices);

				gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, STRIDE * 4, 0);
				gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, STRIDE * 4, 3 * 4);

				// Set color and alpha
				gl.uniform4f(uniform.uColor, self.color.r, self.color.g, self.color.b, self.alphaB);
				gl.uniform1f(uniform.uZIndex, 0.01 + ec * 0.001);

				// Draw
				gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, self.indexBuffer);
				gl.drawElements(gl.TRIANGLES, self.indexCount, gl.UNSIGNED_SHORT, 0);
			}
		});
	}

	/**
	 * Initialize static resources
	 */
	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);

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
		this.ready = false;
	}

	/**
	 * Before render setup
	 */
	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;
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
export default SwirlingAura;
