/**
 * Renderer/Effects/Level99Bubble.js
 *
 * Level 99 aura bubble sparkle (EF_LEVEL99_3)
 * Implements the Blue Level99_3 behavior with 4 columns of camera-facing quads.
 *
 * Reference behavior (flag1=1, texture=whitelight, radius=2.4, speed v=0.15):
 * - 4 emitter columns, each with 4 anchors (B_pre, B_now, T_pre, T_now)
 * - Each anchor has X,Z jitter: x += kx*sin(phaseA), z += kz*sin(phaseB)
 * - Anchors drift downward at constant speed v
 * - Reset when y < -30: x=z=0, y=rand[0,99], reseed phases
 * - Billboard unit square of radius 2.4 facing camera
 * - Alpha: clamp(250 + 25*(y + 20), 0, 250)
 * - Color: (80,80,255), additive blend
 */

import _vertexShader from './Level99Bubble.vs?raw';
import _fragmentShader from './Level99Bubble.fs?raw';
import WebGL from 'Utils/WebGL.js';
import Client from 'Core/Client.js';
import Camera from 'Renderer/Camera.js';
import Altitude from 'Renderer/Map/Altitude.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';

const DEG_TO_RAD = Math.PI / 180;
const GAME_TO_WORLD = 0.1 * 2.2;
const NUM_COLUMNS = 4; // 4 emitter columns
const ANCHORS_PER_COL = 4; // B_pre, B_now, T_pre, T_now
const BASE_LIFT = 0.05;
const DEBUG_UI_ID = 'lvl99bubble-debug';

const REF_RADIUS = 2.4; // Billboard radius
const REF_SPEED = 0.15; // Fall speed per frame
const REF_DRIFT_K = 0.15; // Jitter amplitude (±0.15)
const REF_SEED_MAX = 99; // Max spawn height
const REF_RESET_Y = -30; // Reset threshold
const REF_ALPHA_OFFSET = 20; // Alpha formula offset
const REF_ALPHA_GAIN = 30; // Alpha formula gain (default request)

// Sign patterns for the 4 anchors per column
// B_pre: (+kx, +kz), B_now: (-kx, -kz), T_pre: (+kx, -kz), T_now: (-kx, +kz)
const ANCHOR_SIGNS = [
	{ kx: 1, kz: 1 }, // B_pre
	{ kx: -1, kz: -1 }, // B_now
	{ kx: 1, kz: -1 }, // T_pre
	{ kx: -1, kz: 1 } // T_now
];

// Phase index pairs for each anchor (pa, pb from reference)
// B_pre: (0,2), B_now: (4,6), T_pre: (8,10), T_now: (12,14)
const ANCHOR_PHASE_OFFSETS = [
	{ pa: 0, pb: 2 },
	{ pa: 4, pb: 6 },
	{ pa: 8, pb: 10 },
	{ pa: 12, pb: 14 }
];

// Runtime-tunable config (temporary debug UI below)
const debugConfig = {
	scaleMult: 0.6,
	seedMax: REF_SEED_MAX,
	ghostSeedMax: 60,
	showRedBg: false,
	bgAlpha: 0.3,
	bgRadiusFactor: 2.0,
	fallSpeedMult: 1.0,
	respawnDepthMult: 1.0,
	alphaOffset: REF_ALPHA_OFFSET,
	alphaGain: REF_ALPHA_GAIN,
	floorLimit: Math.abs(REF_RESET_Y)
};

let _program;
let _buffer;

// Unit square corners for billboard (centered at origin)
// Order: bottom-left, bottom-right, top-right, top-left
const BILLBOARD_CORNERS = [
	{ x: -1, y: -1 },
	{ x: 1, y: -1 },
	{ x: 1, y: 1 },
	{ x: -1, y: 1 }
];

function randRange(min, max) {
	return min + Math.random() * (max - min);
}

function wrapDegrees(angle) {
	angle %= 360;
	if (angle < 0) {
		angle += 360;
	}
	return angle;
}

/**
 * Advance a phase angle toward a random target, reseed when reached.
 * Returns { angle, target }
 */
function advancePhase(current, target) {
	let diff = target - current;
	diff = ((diff + 540) % 360) - 180; // Wrap to [-180,180]
	const step = 2 + Math.random(); // 2–3 degrees/frame

	if (Math.abs(diff) <= step) {
		current = target;
		target = randRange(0, 360);
	} else {
		current = wrapDegrees(current + Math.sign(diff) * step);
	}

	return { angle: current, target: target };
}

function pickColor(flag1) {
	if (flag1 === 1) {
		// Exact blue from reference (80,80,255)
		return { r: 80 / 255, g: 80 / 255, b: 255 / 255 };
	}
	if (flag1 === 11 || flag1 === 3) {
		// Ghost gray
		return { r: 0.85, g: 0.85, b: 0.85 };
	}
	return { r: 1.0, g: 1.0, b: 1.0 };
}

/**
 * Create an anchor with initial state
 */
function createAnchor(isGhost, seedMaxUnits) {
	return {
		x: 0,
		y: randRange(0, seedMaxUnits),
		z: 0
	};
}

/**
 * Level99Bubble constructor
 *
 * @param {vec3} position - world grid position (entity position reference)
 * @param {string} textureName - texture filename
 * @param {number} tick - start tick (unused, kept for parity)
 * @param {number} flag1 - tint selector (0 white, 1 blue, 11 ghost)
 */
class Level99Bubble {
	constructor(position, textureName, tick, flag1) {
		this.position = position;
		this.textureName = textureName || 'whitelight.tga';
		this.tick = tick || 0;
		// Default to flag1 = 1 (blue variant) unless explicitly overridden
		this.flag1 = flag1 === 0 || flag1 ? flag1 : 1;

		const isGhost = this.flag1 === 11 || this.flag1 === 3;

		// Reference values in robrowser units
		this.baseRadius = this.flag1 === 1 ? REF_RADIUS : isGhost ? 3.2 : 0.8;
		this.fallSpeed = isGhost ? 0.6 : REF_SPEED; // v per frame in robrowser units
		this.seedMax = isGhost ? debugConfig.ghostSeedMax : debugConfig.seedMax;
		this.resetY = isGhost ? -debugConfig.ghostSeedMax : REF_RESET_Y;
		this.driftK = REF_DRIFT_K; // Jitter amplitude
		this.color = pickColor(this.flag1);
		this.isGhost = isGhost;
		this.passCount = this.flag1 === 1 ? 2 : 1;

		// Initialize columns (emitters)
		this.columns = [];
		const numCols = isGhost ? 1 : NUM_COLUMNS; // Ghost uses only 1 column
		for (let ec = 0; ec < numCols; ec++) {
			const column = {
				life: true,
				anchors: [],
				// 16 phases per column: pairs for each anchor's X and Z
				phases: new Float32Array(16),
				phaseTargets: new Float32Array(16)
			};

			// Initialize phases with random values [0, 360)
			for (let p = 0; p < 16; p++) {
				column.phases[p] = randRange(0, 360);
				column.phaseTargets[p] = randRange(0, 360);
			}

			// Create 4 anchors per column
			for (let a = 0; a < ANCHORS_PER_COL; a++) {
				const anchor = createAnchor(isGhost, this.seedMax);
				column.anchors.push(anchor);
			}

			this.columns.push(column);
		}

		this.quadData = new Float32Array(30); // 6 vertices * (3 pos + 2 uv)
		this.tmpPoints = [
			[0, 0, 0],
			[0, 0, 0],
			[0, 0, 0],
			[0, 0, 0]
		];
	}

	init(gl) {
		Client.loadFile(`data/texture/effect/${this.textureName}`, buffer => {
			WebGL.texture(gl, buffer, texture => {
				this.texture = texture;

				this.ready = true;
			});
		});
	}

	free(gl) {
		this.ready = false;
	}

	/**
	 * Compute alpha based on height using reference formula:
	 * alpha(y) = clamp(250 + gain*(y + offset), 0, 250)
	 * Full above y=-offset, fades out by y=-(offset + 250/gain)
	 */
	computeAlpha(localY) {
		const offset = debugConfig.alphaOffset;
		const gain = debugConfig.alphaGain;
		let alpha255 = 250 + gain * (localY + offset);
		alpha255 = Math.max(0, Math.min(250, alpha255));
		return alpha255 / 255;
	}

	/**
	 * Update all phases in a column (advance toward random targets)
	 */
	updatePhases(column) {
		for (let i = 0; i < 16; i++) {
			const result = advancePhase(column.phases[i], column.phaseTargets[i]);
			column.phases[i] = result.angle;
			column.phaseTargets[i] = result.target;
		}
	}

	/**
	 * Update a single anchor within a column
	 * Reference behavior:
	 * - X,Z jitter: x += kx * sin(phaseA), z += kz * sin(phaseB) when y < 0
	 * - Y drift: y -= v each frame
	 * - Reset when y < resetY: x=z=0, y=rand[0,seedMax], reseed phases
	 */
	updateAnchor(column, anchorIndex) {
		const anchor = column.anchors[anchorIndex];
		const signs = ANCHOR_SIGNS[anchorIndex];
		const phaseOffsets = ANCHOR_PHASE_OFFSETS[anchorIndex];

		// Apply jitter only when below ground (y < 0)
		if (anchor.y < 0) {
			const phaseA = column.phases[phaseOffsets.pa] * DEG_TO_RAD;
			const phaseB = column.phases[phaseOffsets.pb] * DEG_TO_RAD;
			anchor.x += signs.kx * this.driftK * Math.sin(phaseA);
			anchor.z += signs.kz * this.driftK * Math.sin(phaseB);
		}

		// Drift downward
		anchor.y -= this.fallSpeed * debugConfig.fallSpeedMult;

		// Reset when below threshold
		const resetLimit = this.resetY * debugConfig.respawnDepthMult;
		if (anchor.y < resetLimit) {
			anchor.x = 0;
			anchor.z = 0;
			anchor.y = randRange(0, this.seedMax);

			// Reseed the phase pair for this anchor
			column.phases[phaseOffsets.pa] = randRange(0, 360);
			column.phaseTargets[phaseOffsets.pa] = randRange(0, 360);
			column.phases[phaseOffsets.pb] = randRange(0, 360);
			column.phaseTargets[phaseOffsets.pb] = randRange(0, 360);
		}
	}

	fillQuad(points, quadData) {
		// Two triangles: p0-p1-p2 and p2-p3-p0
		quadData[0] = points[0][0];
		quadData[1] = points[0][1];
		quadData[2] = points[0][2];
		quadData[3] = 0.0;
		quadData[4] = 0.0;
		quadData[5] = points[1][0];
		quadData[6] = points[1][1];
		quadData[7] = points[1][2];
		quadData[8] = 1.0;
		quadData[9] = 0.0;
		quadData[10] = points[2][0];
		quadData[11] = points[2][1];
		quadData[12] = points[2][2];
		quadData[13] = 1.0;
		quadData[14] = 1.0;

		quadData[15] = points[2][0];
		quadData[16] = points[2][1];
		quadData[17] = points[2][2];
		quadData[18] = 1.0;
		quadData[19] = 1.0;
		quadData[20] = points[3][0];
		quadData[21] = points[3][1];
		quadData[22] = points[3][2];
		quadData[23] = 0.0;
		quadData[24] = 1.0;
		quadData[25] = points[0][0];
		quadData[26] = points[0][1];
		quadData[27] = points[0][2];
		quadData[28] = 0.0;
		quadData[29] = 0.0;
	}

	render(gl, tick) {
		if (!this.ready || !this.texture) {
			return;
		}

		const uniform = _program.uniform;

		// Get ground position (world coordinates)
		const groundZ = Altitude.getCellHeight(this.position[0], this.position[1]);
		const basePos = [this.position[0] + 0.5, -groundZ - BASE_LIFT, this.position[1] + 0.5];

		// Camera orientation for billboarding
		const viewPitch = Camera.angle[0];
		const viewYaw = Camera.angle[1];
		const beta = ((360 - viewPitch + 90) % 360) * DEG_TO_RAD;
		const alpha = ((360 - viewYaw) % 360) * DEG_TO_RAD;

		const sinBeta = Math.sin(beta);
		const cosBeta = Math.cos(beta);
		const sinAlpha = Math.sin(alpha);
		const cosAlpha = Math.cos(alpha);

		gl.bindTexture(gl.TEXTURE_2D, this.texture);

		// Debug: show spawn volume
		if (debugConfig.showRedBg) {
			this.renderBackground(gl, basePos);
		}

		// Current billboard radius (in world units)
		const radius = this.baseRadius * GAME_TO_WORLD * debugConfig.scaleMult;

		// Process each column
		for (let ec = 0; ec < this.columns.length; ec++) {
			const column = this.columns[ec];
			if (!column.life) {
				continue;
			}

			// Update phases for this column
			this.updatePhases(column);

			// Process each anchor in the column
			for (let ai = 0; ai < column.anchors.length; ai++) {
				// Update anchor position (jitter + drift + reset)
				this.updateAnchor(column, ai);

				const anchor = column.anchors[ai];

				// Convert anchor position to world units
				const anchorWorldX = anchor.x * GAME_TO_WORLD;
				const anchorWorldY = anchor.y * GAME_TO_WORLD;
				const anchorWorldZ = anchor.z * GAME_TO_WORLD;

				// Build billboard quad corners around anchor
				for (let k = 0; k < BILLBOARD_CORNERS.length; k++) {
					const corner = BILLBOARD_CORNERS[k];

					// Local billboard coordinates (unit square scaled by radius)
					const localX = corner.x * radius;
					const localZ = corner.y * radius;

					// Apply R_x(beta) - pitch rotation
					const ry = -localZ * sinBeta;
					let rz = localZ * cosBeta;

					// Apply R_y(alpha) - yaw rotation
					const rx = localX * cosAlpha + rz * sinAlpha;
					rz = -localX * sinAlpha + rz * cosAlpha;

					// Final world position = base + anchor offset + billboard corner
					const finalX = basePos[0] + anchorWorldX + rx;
					const finalY = basePos[1] + anchorWorldY + ry;
					const finalZ = basePos[2] + anchorWorldZ + rz;

					this.tmpPoints[k][0] = finalX;
					this.tmpPoints[k][1] = finalY;
					this.tmpPoints[k][2] = finalZ;
				}

				// Use anchor Y (in game units) for alpha calculation
				// Skip quads that are still above ground (positive Y = underground in this system)
				if (anchor.y > 0) {
					continue;
				}

				// Compute alpha based on anchor height
				const alphaValue = this.computeAlpha(anchor.y);
				if (alphaValue <= 0) {
					continue;
				}

				this.fillQuad(this.tmpPoints, this.quadData);

				// Update GPU buffer with quad vertex data (6 vertices * (3 pos + 2 uv) = 30 floats)
				gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.quadData);

				const self = this;
				SpriteRenderer.runWithDepth(true, false, false, function () {
					for (let pass = 0; pass < self.passCount; pass++) {
						gl.uniform4f(uniform.uColor, self.color.r, self.color.g, self.color.b, alphaValue);
						gl.uniform1f(uniform.uZIndex, 0.01 + ec * 0.002 + ai * 0.0001 + pass * 0.00005);
						gl.drawArrays(gl.TRIANGLES, 0, 6);
					}
				});
			}
		}
	}

	renderBackground(gl, basePos) {
		const uniform = _program.uniform;
		const radius = this.baseRadius * GAME_TO_WORLD * debugConfig.bgRadiusFactor;
		const height = this.seedMax * GAME_TO_WORLD;

		function drawQuad(v0, v1, v2, v3) {
			// Reuse tmpPoints
			// v0..v3: [x,y,z]
			for (let i = 0; i < 3; i++) {
				this.tmpPoints[0][i] = v0[i];
				this.tmpPoints[1][i] = v1[i];
				this.tmpPoints[2][i] = v2[i];
				this.tmpPoints[3][i] = v3[i];
			}
			this.fillQuad(this.tmpPoints, this.quadData);
			// Update GPU buffer with quad vertex data
			gl.bufferSubData(gl.ARRAY_BUFFER, 0, this.quadData);
			gl.uniform1i(uniform.uSolidBg, 1);
			gl.uniform4f(uniform.uColor, 1.0, 0.0, 0.0, debugConfig.bgAlpha);
			gl.uniform1f(uniform.uZIndex, 0.0005);

			SpriteRenderer.runWithDepth(true, true, false, function () {
				gl.drawArrays(gl.TRIANGLES, 0, 6);
			});
		}

		const x0 = basePos[0] - radius;
		const x1 = basePos[0] + radius;
		const z0 = basePos[2] - radius;
		const z1 = basePos[2] + radius;
		const y0 = basePos[1];
		const y1 = basePos[1] + height;

		// Bottom
		drawQuad.call(this, [x0, y0, z0], [x1, y0, z0], [x1, y0, z1], [x0, y0, z1]);

		// Top
		drawQuad.call(this, [x0, y1, z0], [x1, y1, z0], [x1, y1, z1], [x0, y1, z1]);

		// Sides
		drawQuad.call(this, [x0, y0, z0], [x1, y0, z0], [x1, y1, z0], [x0, y1, z0]);

		drawQuad.call(this, [x1, y0, z0], [x1, y0, z1], [x1, y1, z1], [x1, y1, z0]);

		drawQuad.call(this, [x1, y0, z1], [x0, y0, z1], [x0, y1, z1], [x1, y1, z1]);

		drawQuad.call(this, [x0, y0, z1], [x0, y0, z0], [x0, y1, z0], [x0, y1, z1]);

		gl.uniform1i(uniform.uSolidBg, 0);
	}

	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		_buffer = gl.createBuffer();

		// Initialize empty buffer for dynamic quad data (6 vertices * (3 pos + 2 uv) = 30 floats)
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		// Allocate buffer with null data for dynamic updates (120 bytes = 30 floats * 4 bytes/float)
		gl.bufferData(gl.ARRAY_BUFFER, 120, gl.DYNAMIC_DRAW);

		this.ready = true;
		this.renderBeforeEntities = true;
	}

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

	static beforeRender(gl, modelView, projection, fog) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.blendFunc(gl.SRC_ALPHA, gl.ONE); // Additive blend

		gl.useProgram(_program);

		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);
		gl.uniform1i(uniform.uSolidBg, 0);

		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 5 * 4, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 5 * 4, 3 * 4);
	}

	static afterRender(gl) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
	}
}

function _ensureDebugUI() {
	if (typeof document === 'undefined') {
		return;
	}
	if (document.getElementById(DEBUG_UI_ID)) {
		return;
	}

	const container = document.createElement('div');
	container.id = DEBUG_UI_ID;
	container.style.position = 'fixed';
	container.style.bottom = '12px';
	container.style.right = '12px';
	container.style.zIndex = 9999;
	container.style.background = 'rgba(20,20,20,0.8)';
	container.style.color = '#fff';
	container.style.padding = '8px';
	container.style.borderRadius = '4px';
	container.style.font = '12px sans-serif';
	container.style.pointerEvents = 'auto';

	const title = document.createElement('div');
	title.textContent = 'L99 Bubble Debug';
	title.style.marginBottom = '6px';
	title.style.fontWeight = 'bold';
	container.appendChild(title);

	function addSlider(labelText, min, max, step, value, onChange) {
		const row = document.createElement('div');
		row.style.display = 'flex';
		row.style.alignItems = 'center';
		row.style.gap = '6px';
		row.style.marginBottom = '4px';

		const label = document.createElement('span');
		label.textContent = labelText;
		label.style.whiteSpace = 'nowrap';
		row.appendChild(label);

		const input = document.createElement('input');
		input.type = 'range';
		input.min = min;
		input.max = max;
		input.step = step;
		input.value = value;
		input.style.flex = '1';
		input.addEventListener('input', function () {
			onChange(parseFloat(input.value));
		});
		row.appendChild(input);

		const valLabel = document.createElement('span');
		valLabel.textContent = value;
		input.addEventListener('input', function () {
			valLabel.textContent = input.value;
		});
		row.appendChild(valLabel);

		container.appendChild(row);
	}

	function addCheckbox(labelText, checked, onChange) {
		const row = document.createElement('div');
		row.style.display = 'flex';
		row.style.alignItems = 'center';
		row.style.gap = '6px';
		row.style.marginBottom = '4px';

		const input = document.createElement('input');
		input.type = 'checkbox';
		input.checked = checked;
		input.addEventListener('change', function () {
			onChange(!!input.checked);
		});
		row.appendChild(input);

		const label = document.createElement('span');
		label.textContent = labelText;
		row.appendChild(label);

		container.appendChild(row);
	}

	addSlider('Billboard size', 0.3, 3.0, 0.05, debugConfig.scaleMult, function (v) {
		debugConfig.scaleMult = v;
	});

	addSlider('Spawn height (seedMax)', 10, 150, 1, debugConfig.seedMax, function (v) {
		debugConfig.seedMax = v;
	});

	addSlider('Fall speed mult', 0.1, 5.0, 0.05, debugConfig.fallSpeedMult, function (v) {
		debugConfig.fallSpeedMult = v;
	});

	addSlider('Respawn depth mult', 0.1, 5.0, 0.05, debugConfig.respawnDepthMult, function (v) {
		debugConfig.respawnDepthMult = v;
	});

	addSlider('Alpha offset', -50, 50, 1, debugConfig.alphaOffset, function (v) {
		debugConfig.alphaOffset = v;
	});

	addSlider('Alpha gain', 1, 80, 1, debugConfig.alphaGain, function (v) {
		debugConfig.alphaGain = v;
	});

	addSlider('Floor limit', 10, 60, 1, debugConfig.floorLimit, function (v) {
		debugConfig.floorLimit = v;
	});

	addCheckbox('Show spawn volume', debugConfig.showRedBg, function (v) {
		debugConfig.showRedBg = v;
	});

	document.body.appendChild(container);
}

// Initialize temporary debug UI once
// Temporarily disabled; will be re-enabled via a future interface
// _ensureDebugUI();
export default Level99Bubble;
