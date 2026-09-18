/**
 * Renderer/Entity/EntityAura.js
 *
 * Helper to manage entity's aura
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Gulfaraz Rahman
 *
 * @typedef {Object} TMapPreferencesAura
 * @prop {number} aura - 0: no aura, 1: only aura, 2: aura and aura2
 *
 * @typedef {Object} TAuraSettings - default settings for aura (can be overridden by server config)
 * @prop {number} defaultLv - default aura level
 * @prop {number} babyLv - Baby class aura level
 * @prop {number} secondLv - Second class aura level
 * @prop {number} thirdLv - Third class aura level
 * @prop {number} homunLv - Homun class aura level
 * @prop {number} bossLv - Boss class aura level
 */

import EffectConst from 'DB/Effects/EffectConst.js';
import /** @type {TMapPreferencesAura} */ MapPreferences from 'Preferences/Map.js';
import Configs from 'Core/Configs.js';

/** @type {TAuraSettings} */
const _auraSettings = {
	defaultLv: 99
	// babyLv: 99, // TODO implement other aura levels
	// secondLv: 99,
	// thirdLv: 175,
	// homunLv: 99,
	// bossLv: 99,
};

const normalEffects = [EffectConst.EF_LEVEL99, EffectConst.EF_LEVEL99_2, EffectConst.EF_LEVEL99_3];

const simpleEffects = [EffectConst.EF_LEVEL99_3];

/**
 * Aura class — handles level 99/max level character aura visual effects
 *
 * @class Aura
 * @property {boolean} isLoaded Whether aura effect is currently loaded
 * @property {Entity} entity Attached entity reference
 * @property {number} lastAuraState Saved preference state to track /aura toggle changes
 */
class Aura {
	constructor(entity) {
		this.isLoaded = false; // to avoid duplicate aura effects
		this.entity = entity; // reference to attached entity
		this.lastAuraState = 0; // save last aura state to track changes on aura/aura2 command
	}

	/**
	 * Show aura
	 */
	load(effectManager) {
		const server = Configs.getServer(); // find aura from servers config

		/** @type {TAuraSettings} - merge server aura config with default settings */
		const settings =
			server != null ? Object.assign({}, _auraSettings, server.aura) : Object.assign({}, _auraSettings);

		// check if qualifies for aura and /aura2 preference
		if (MapPreferences.aura > 0 && this.entity.clevel >= settings.defaultLv) {
			// check if entity is visible
			if (this.entity.isVisible()) {
				// check if aura state has changed
				if (this.lastAuraState !== MapPreferences.aura && this.isLoaded) {
					this.remove(effectManager);
				}
				if (!this.isLoaded) {
					// aura is already loaded
					// select effects based on /aura preference
					const effects = MapPreferences.aura < 2 ? simpleEffects : normalEffects;
					// add aura effects
					for (let effectIndex = 0; effectIndex < effects.length; effectIndex++) {
						effectManager.spam({
							ownerAID: this.entity.GID,
							position: this.entity.position,
							effectId: effects[effectIndex]
						});
					}
					// set flag to avoid duplicate aura effects
					this.isLoaded = true;
					// save current aura state
					this.lastAuraState = MapPreferences.aura;
				}
			} else {
				// remove aura if entity is invisible
				this.remove(effectManager);
			}
		} else if (this.isLoaded) {
			// remove aura if entity does not qualify
			this.remove(effectManager);
			// save current aura state
			this.lastAuraState = MapPreferences.aura;
		}
	}

	/**
	 * Hide aura
	 */
	remove(effectManager) {
		// remove aura effects
		effectManager.remove(null, this.entity.GID, normalEffects);
		// free aura - needs to be separate to avoid circular dependency
		this.free();
	}

	/**
	 * Hide aura
	 */
	free() {
		// reset flag to allow aura to be loaded
		this.isLoaded = false;
	}
}
/**
 * Export
 */
export default function init() {
	this.aura = new Aura(this);
}
