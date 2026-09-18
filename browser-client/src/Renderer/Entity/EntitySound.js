/**
 * Renderer/EntityWalk.js
 *
 * Manage entity walking action
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import SoundManager from 'Audio/SoundManager.js';

/**
 * Sound class — spatial sound playback attached to entity actions
 *
 * @class Sound
 * @property {number} _lastActionId Last played action ID
 * @property {number} _lastAnimationId Last played animation frame ID
 * @property {string|null} _lastFileName Last played WAV sound filename
 * @property {number} _animCounter Sound animation counter
 * @property {string|null} attackFile Custom weapon attack sound filename
 * @property {Entity} entity Target entity reference
 */
class Sound {
	constructor() {
		this._lastActionId = -1;
		this._lastAnimationId = -1;
		this._lastFileName = null;
		this._animCounter = -1;

		this.attackFile = null;
	}

	/**
	 * Play a sound attached to an entity
	 *
	 * @param {string} sound name
	 * @param {number} action id
	 * @param {number} animation id
	 */
	play(fileName, action, animation) {
		// Pet does not produce sound
		if (this.entity.objecttype === this.entity.constructor.TYPE_PET) {
			return;
		}

		// Do not replay the sound if there is no updates
		if (this._lastActionId === action && this._lastAnimationId === animation && this._lastFileName === fileName) {
			return;
		}

		this._lastActionId = action;
		this._lastAnimationId = animation;
		this._lastFileName = fileName;

		// Find Audio filename
		if (fileName === 'atk') {
			if (!this.attackFile) {
				return;
			}

			fileName = this.attackFile;
		}

		SoundManager.playPosition(fileName, this.entity.position);
	}

	/**
	 * Reset action and animation
	 */
	free() {
		this._lastActionId = -1;
		this._lastAnimationId = -1;
		this._lastFileName = null;
		this._animCounter = -1;
	}

	/**
	 * Reset sound counter to allow repeating sounds
	 *
	 * @param {number} animation index
	 * @param {number} animation size
	 */
	freeOnAnimationEnd(anim, size) {
		if (anim < size) {
			return;
		}

		const count = Math.floor(anim / size);

		if (this._animCounter !== count) {
			this.free();
			this._animCounter = count;
		}
	}
}

/**
 * Initialize and export methods
 */
export default function init() {
	this.sound = new Sound();
	this.sound.entity = this;
}
