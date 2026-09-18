/**
 * Renderer/EntityLife.js
 *
 * Manage Entity Life GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import DB from 'DB/DBManager.js';
import EntityOverlay from 'Renderer/Entity/EntityOverlay.js';

/**
 * Global methods
 */
const vec4 = glMatrix.vec4;
const _pos = new Float32Array(4);
const _size = new Float32Array(2);

/**
 * Life class — HP / SP / AP health bar rendering above entity
 *
 * @class Life
 * @property {number} hp Current health points
 * @property {number} sp Current spell points
 * @property {number} ap Current action points
 * @property {number} hp_max Maximum health points
 * @property {number} sp_max Maximum spell points
 * @property {number} ap_max Maximum action points
 * @property {number} hunger Current companion hunger
 * @property {number} hunger_max Max companion hunger
 * @property {boolean} display Whether health bar is visible
 * @property {HTMLCanvasElement} canvas Health bar canvas element
 * @property {CanvasRenderingContext2D} ctx 2d context for health bar
 * @property {Entity|null} entity Attached entity reference
 */
class Life {
	constructor() {
		this.hp = -1;
		this.sp = -1;
		this.ap = -1;
		this.hp_max = -1;
		this.sp_max = -1;
		this.ap_max = -1;
		this.display = false;
		this.canvas = document.createElement('canvas');
		this.canvas.className = 'entity-life';
		this.ctx = this.canvas.getContext('2d');
		this.canvas.style.position = 'absolute';
		this.canvas.style.zIndex = 1;
		this.entity = null;
		this.hunger = -1;
		this.hunger_max = -1;
	}

	/**
	 * Remove Life GUI
	 */
	remove() {
		this.display = false;
		this.canvas.remove();
	}

	/**
	 * Clean Up Life
	 */
	clean() {
		this.remove();
	}

	/**
	 * Update life
	 */
	update() {
		const width = 60;
		let height = 5;
		const Entity = this.entity.constructor;

		// Don't display it, if negatives values.
		if (this.hp < 0 || this.hp_max < 0) {
			this.remove();
			return;
		}

		// Init variables
		this.display = true;
		const ctx = this.ctx;
		const hp_per = this.hp / this.hp_max;
		const sp = this.sp > -1 && this.sp_max > -1;
		const sp_per = this.sp / this.sp_max;
		const ap = this.ap > -1 && this.ap_max > -1;
		const ap_per = this.ap / this.ap_max;

		const hunger = this.hunger > -1 && this.hunger_max > -1;
		const hunger_per = this.hunger / this.hunger_max;

		if (sp) {
			height += 4;
		}

		// AP should only show to 4th Job Class
		if (DB.getJobClass(this.entity._job) === 'Fourth_Class' && ap) {
			height += 4;
		}

		if (hunger) {
			height += 4;
		}

		// Draw life
		ctx.canvas.width = width;
		ctx.canvas.height = height;

		// border
		ctx.fillStyle = '#10189c';
		ctx.fillRect(0, 0, width, height);

		// empty
		ctx.fillStyle = '#424242';
		ctx.fillRect(1, 1, width - 2, height - 2);

		//zIndex
		this.canvas.style.zIndex = this.entity.objecttype === Entity.TYPE_PC ? 2 : 1;

		// Display HP
		if (
			this.entity.objecttype === Entity.TYPE_MOB ||
			this.entity.objecttype === Entity.TYPE_NPC_ABR ||
			this.entity.objecttype === Entity.TYPE_NPC_BIONIC
		) {
			ctx.fillStyle = hp_per < 0.25 ? '#FFFF00' : '#FF00E7';
		} else if (this.entity.objecttype === Entity.TYPE_PET) {
			ctx.fillStyle = hp_per < 0.25 ? '#FFFF00' : '#FFE7E7';
		} else {
			ctx.fillStyle = hp_per < 0.25 ? '#FF0000' : '#10ef21';
		}

		ctx.fillRect(1, 1, Math.round((width - 2) * hp_per), 3);

		// Display SP
		if (sp) {
			ctx.fillStyle = '#10189c';
			ctx.fillRect(0, 4, width, 1);
			ctx.fillStyle = '#1863de';
			ctx.fillRect(1, 5, Math.round((width - 2) * sp_per), 3);
		}

		if (ap) {
			ctx.fillStyle = '#424242';
			ctx.fillRect(1, 9, width, 1);
			ctx.fillStyle = '#ffc663';
			ctx.fillRect(1, 9, Math.round((width - 2) * ap_per), 3);
		}

		if (hunger) {
			ctx.fillStyle = '#424242';
			ctx.fillRect(1, 9, width, 1);
			ctx.fillStyle = hunger_per < 0.25 ? '#FFFF00' : '#FFE7E7';
			ctx.fillRect(1, 9, Math.round((width - 2) * hunger_per), 3);
		}
	}

	/**
	 * Rendering life
	 *
	 * @param {mat4} matrix
	 */
	render(matrix) {
		const canvas = this.canvas;

		// Cast position
		_pos[0] = 0.0;
		_pos[1] = -0.5;
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

		// Check if the Vertical Flip (Illusion effect) is active
		// If true, invert the Y coordinate relative to the renderer height
		// Using a dynamic check or window access to avoid cycle if possible,
		// but usually Shaders don't import Renderer.
		if (window.VerticalFlip && window.VerticalFlip.isActive()) {
			_pos[1] = window.innerHeight - _pos[1];
		}

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
	this.life = new Life();
	this.life.entity = this;
}
