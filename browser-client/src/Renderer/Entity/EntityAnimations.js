/**
 * Renderer/EntityAnimations.js
 *
 * Manage entity special animations
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Renderer from 'Renderer/Renderer.js';

/**
 * Manage entity special animations
 *
 * @class Animations
 * @property {Entity} entity Target entity
 * @property {Array<{tick: number, callback: function(number): boolean}>} list List of active animation callbacks
 */
class Animations {
	constructor(entity) {
		this.entity = entity;
		this.list = [];
	}

	/**
	 * Add an animation to the list
	 *
	 * @param {function} callback
	 */
	add(callback) {
		this.list.push({
			tick: Renderer.tick,
			callback: callback
		});
	}

	/**
	 * Process events
	 */
	process() {
		let i, count;

		for (i = 0, count = this.list.length; i < count; ++i) {
			if (this.list[i].callback(Renderer.tick - this.list[i].tick)) {
				this.list.splice(i, 1);
				i--;
				count--;
			}
		}
	}

	/**
	 * Clean up events
	 */
	free() {
		this.list.length = 0;
	}
}
export default function init() {
	this.animations = new Animations(this);
}
