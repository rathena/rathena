/**
 * Renderer/Map/Effects.js
 *
 * Display 3D effects
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import EffectManager from 'Renderer/EffectManager.js';

/**
 * Sound renderer namespace
 */
const vec3 = glMatrix.vec3;
const _list = [];

/**
 * Add 3D sound to the list
 */
function add(mapEffect) {
	_list.push(mapEffect);
}

/**
 * Remove data from memory
 */
function free() {
	_list.length = 0;
}

/**
 * Get effect from list
 */
function get(GID) {
	return _list.find(mapEffect => mapEffect.name == GID) || null;
}

/**
 * Remove effect from list
 */
function remove(GID) {
	const index = _list.findIndex(mapEffect => mapEffect.name == GID);
	if (index !== -1) {
		_list.splice(index, 1);
	}
}

/**
 * Add effects to scene
 *
 * @param {vec3} position
 */
function spam(position, tick) {
	_list.forEach(mapEffect => {
		// distance need to be less than 25 cells (seems like it's
		// how the official client handle it).
		if (!mapEffect.isVisible && vec3.dist(mapEffect.pos, position) < 25) {
			const EF_Init_Par = {
				effectId: mapEffect.id,
				ownerAID: mapEffect.name,
				position: mapEffect.pos,
				startTick: tick,
				persistent: true
			};

			EffectManager.spam(EF_Init_Par);
			mapEffect.isVisible = true;
		} else if (mapEffect.isVisible && vec3.dist(mapEffect.pos, position) >= 25) {
			EffectManager.remove(null, mapEffect.name);
			mapEffect.isVisible = false;
		}
	});
}

/**
 * Export
 */
export default {
	add: add,
	free: free,
	get: get,
	remove: remove,
	spam: spam
};
