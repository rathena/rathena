/**
 * Renderer/EntityEmblem.js
 *
 * Manage Emblem GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alison Serafim
 */

import glMatrix from 'Utils/gl-matrix.js';
import EntityOverlay from 'Renderer/Entity/EntityOverlay.js';

/**
 * Global methods
 */
const vec4 = glMatrix.vec4;
const _pos = new Float32Array(4);
const _size = new Float32Array(2);

/**
 * Emblem class — standalone guild / siege emblem rendering
 *
 * @class Emblem
 * @property {Image|null} emblem Guild emblem image element
 * @property {boolean} display Whether emblem GUI is visible
 * @property {HTMLCanvasElement} canvas Emblem canvas element
 * @property {CanvasRenderingContext2D} ctx 2d context for emblem
 * @property {Entity|null} entity Target entity
 */
class Emblem {
	constructor() {
		this.emblem = null;
		this.display = false;
		this.canvas = document.createElement('canvas');
		this.canvas.className = 'entity-emblem';
		this.ctx = this.canvas.getContext('2d');
		this.canvas.style.position = 'absolute';
		this.canvas.style.zIndex = 1;
		this.entity = null;
	}

	/**
	 * Remove Emblem GUI
	 */
	remove() {
		this.display = false;
		this.canvas.remove();
	}

	/**
	 * Clean Up Emblem
	 */
	clean() {
		this.remove();
		//this.ctx    = null;
		//this.canvas = null;
	}

	/**
	 * Update Emblem
	 */
	update() {
		const width = 24,
			height = 24;

		// Init variables
		const ctx = this.ctx;

		// Set size
		ctx.canvas.width = width;
		ctx.canvas.height = height;

		// Draw Emblem
		if (this.emblem !== null) {
			ctx.drawImage(this.emblem, 0, 0);
		}
	}

	/**
	 * Rendering Emblem
	 *
	 * @param {mat4} matrix
	 */
	render(matrix) {
		const canvas = this.canvas;

		// Cast position
		_pos[0] = 0.0;
		_pos[1] = 140 / 35;
		_pos[2] = 0.0;
		_pos[3] = 1.0;

		// Set the viewport
		_size[0] = window.innerWidth / 2;
		_size[1] = window.innerHeight / 2;

		// Project point to scene
		vec4.transformMat4(_pos, _pos, matrix);

		// Calculate position
		const z = _pos[3] === 0.0 ? 1.0 : 1.0 / _pos[3];
		_pos[0] = _size[0] + Math.round(_size[0] * (_pos[0] * z));
		_pos[1] = _size[1] - Math.round(_size[1] * (_pos[1] * z));

		canvas.style.top = (_pos[1] | 0) + 'px';
		canvas.style.left = ((_pos[0] - canvas.width / 2) | 0) + 'px';

		// Append to the clipped overlay layer
		EntityOverlay.append(canvas);
	}
}
/**
 * Export
 */
export default function Init() {
	this.emblem = new Emblem();
	this.emblem.entity = this;
}
