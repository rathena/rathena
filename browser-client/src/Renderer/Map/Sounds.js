/**
 * Renderer/Map/Sounds.js
 *
 * Play 3D sounds
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import SoundManager from 'Audio/SoundManager.js';

/**
 * Sound renderer namespace
 */
const vec2 = glMatrix.vec2;
const _list = [];

/**
 * Add 3D sound to the list
 */
function add(sound) {
	_list.push(sound);
}

/**
 * Remove data from memory
 */
function free() {
	_list.length = 0;
}

/**
 * Rendering sounds
 *
 * @param {vec2} position
 */
function render(position, tick) {
	_list.forEach(sound => {
		const dist = Math.floor(vec2.dist(sound.pos, position));
		if (sound.tick < tick && dist <= sound.range) {
			SoundManager.playPosition(sound.file, sound.pos);
			sound.tick = tick + sound.cycle * 1000;
		}
	});
}

/**
 * Export
 */
export default {
	add: add,
	free: free,
	render: render
};
