/**
 * Renderer/EntityCast.js
 *
 * Display the progressbar when an Entity cast a skill
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
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
 * Cast class — progressbar rendering when an Entity casts a skill
 *
 * @class Cast
 * @property {number} tick Start tick of skill cast
 * @property {number} delay Total cast delay in ms
 * @property {number} percent Current progress percentage (0..1)
 * @property {boolean} display Whether cast bar is active/visible
 * @property {string} color Progressbar color hex string
 * @property {function|null} onComplete Cast completion callback
 * @property {HTMLCanvasElement} canvas Progressbar canvas element
 * @property {CanvasRenderingContext2D} ctx Progressbar 2d rendering context
 */
class Cast {
	constructor() {
		this.tick = 0;
		this.delay = 0;
		this.percent = -1;
		this.display = false;
		this.color = '#00FF00';
		this.onComplete = null;

		this.canvas = document.createElement('canvas');
		this.canvas.className = 'entity-cast';
		this.ctx = this.canvas.getContext('2d');
		this.canvas.style.position = 'absolute';
		this.canvas.style.zIndex = 1;
		this.canvas.width = 60;
		this.canvas.height = 6;
	}

	/**
	 * Set a progressbar
	 *
	 * @param {number} delay
	 */
	set(delay, color) {
		// Init cast
		this.display = true;
		this.tick = Date.now() + 0;
		this.delay = delay;
		this.color = color || '#00FF00';
	}

	/**
	 * Remove GUI from html
	 */
	remove() {
		this.percent = -1;
		this.display = false;
		this.canvas.remove();
	}

	/**
	 * Clean up memory
	 */
	clean() {
		this.remove();
		if (this.onComplete) {
			this.onComplete();
		}
		this.onComplete = null;
	}

	/**
	 * Update progress bar
	 *
	 * @param {number} perc
	 */
	update(perc) {
		const width = 60,
			height = 6;
		const ctx = this.ctx;

		// Border
		ctx.fillStyle = '#10189c';
		ctx.fillRect(0, 0, width, height);

		// Background
		ctx.fillStyle = '#424242';
		ctx.fillRect(1, 1, width - 2, 4);

		// Percent
		ctx.fillStyle = this.color;
		ctx.fillRect(1, 1, Math.round((width - 2) * perc), 4);
	}

	/**
	 * Rendering cast
	 *
	 * @param {mat4} matrix
	 */
	render(matrix) {
		const canvas = this.canvas;
		const percent = +((Date.now() - this.tick) / this.delay).toFixed(2);

		// Cast complete remove it
		if (percent >= 1.0) {
			this.remove();

			if (this.onComplete) {
				this.onComplete();
				this.onComplete = null;
			}
			return;
		}

		// Update
		if (percent !== this.percent) {
			this.update(percent);
			this.percent = percent;
		}

		// Cast position
		_pos[0] = 0.0;
		_pos[1] = 90 / 35;
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
	this.cast = new Cast();
}
