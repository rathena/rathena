/**
 * @module Renderer/ScreenEffectManager
 *
 * Manage ScreenEffects
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import EffectConst from 'DB/Effects/EffectConst.js';
import Renderer from 'Renderer/Renderer.js';
import WeatherTable from 'DB/Effects/WeatherEffect.js';
import SnowWeather from 'Renderer/Effects/SnowWeather.js';
import RainWeather from 'Renderer/Effects/RainWeather.js';
import SakuraWeatherEffect from 'Renderer/Effects/SakuraWeatherEffect.js';
import PokJukWeatherEffect from 'Renderer/Effects/PokJukWeatherEffect.js';
import CloudWeatherEffect from 'Renderer/Effects/CloudWeatherEffect.js';
import Poison from 'Renderer/Effects/PoisonEffect.js';
import Blind from 'Renderer/Effects/Shaders/Blind.js';
import VerticalFlip from 'Renderer/Effects/Shaders/VerticalFlip.js';
import EFST from 'DB/Status/StatusConst.js';
import Session from 'Engine/SessionStorage.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import MapRenderer from 'Renderer/MapRenderer.js';

let isMapflagEffect = false;

function beforeRender(gl, modelView, projection, fog, tick) {
	SpriteRenderer.bind3DContext(gl, modelView, projection, fog);
}

function afterRender(gl, modelView, projection, fog, tick) {
	SpriteRenderer.unbind(gl);
}

/**
 * ScreenEffectManager Namespace
 */
class ScreenEffectManager {
	/**
	 * @var {number} _nightInterval
	 */
	static _nightInterval = null;
	/**
	 * @param {object} gl context
	 * @param {string} mapname
	 */
	static init(gl, mapname) {
		// weather effects
		if (WeatherTable.effects && WeatherTable.effects[mapname]) {
			isMapflagEffect = true;
		}
	}

	static hasAnyActiveEffect() {
		// Weather effects
		if (SnowWeather.isActive()) {
			return true;
		}
		if (RainWeather.isActive()) {
			return true;
		}
		if (SakuraWeatherEffect.isActive()) {
			return true;
		}
		if (PokJukWeatherEffect.isActive()) {
			return true;
		}
		if (CloudWeatherEffect.isActive()) {
			return true;
		}

		// Status effects
		if (Session.Entity && Poison.isActive()) {
			return true;
		}
		if (Session.Entity && Blind.isActive()) {
			return true;
		}
		if (Session.Entity && VerticalFlip.isActive()) {
			return true;
		}

		return false;
	}

	static startMapflagEffect(mapname) {
		if (!isMapflagEffect) {
			return;
		}

		isMapflagEffect = false;
		const Params = {
			Inst: {
				startTick: Renderer.tick,
				endTick: -1
			},
			Init: {
				ownerAID: -1,
				position: null
			}
		};

		const weather = WeatherTable.effects[mapname].weather;
		if (weather === 'snow') {
			Params.Inst.effectId = EffectConst.EF_SNOW;
			SnowWeather.startOrRestart(Params);
		} else if (weather === 'rain') {
			Params.Inst.effectId = EffectConst.EF_RAIN;
			RainWeather.startOrRestart(Params);
		} else if (weather === 'fireworks') {
			Params.Inst.effectId = EffectConst.EF_POKJUK;
			PokJukWeatherEffect.startOrRestart(Params);
		} else if (weather === 'leaves') {
			Params.Inst.effectId = EffectConst.EF_MAPLE;
			SakuraWeatherEffect.startOrRestart(Params);
		} else if (weather === 'sakura') {
			Params.Inst.effectId = EffectConst.EF_SAKURA;
			SakuraWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud') {
			Params.Inst.effectId = EffectConst.EF_CLOUD;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud2') {
			Params.Inst.effectId = EffectConst.EF_CLOUD2;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud3') {
			Params.Inst.effectId = EffectConst.EF_CLOUD3;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud4') {
			Params.Inst.effectId = EffectConst.EF_CLOUD4;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud5') {
			Params.Inst.effectId = EffectConst.EF_CLOUD5;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud6') {
			Params.Inst.effectId = EffectConst.EF_CLOUD6;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud7') {
			Params.Inst.effectId = EffectConst.EF_CLOUD7;
			CloudWeatherEffect.startOrRestart(Params);
		} else if (weather === 'cloud8') {
			Params.Inst.effectId = EffectConst.EF_CLOUD8;
			CloudWeatherEffect.startOrRestart(Params);
		}
	}

	static renderStatusEffects(gl, modelView, projection, fog) {
		if (!Session.Entity) {
			return;
		}

		if (Poison.isActive()) {
			Poison.render(gl, modelView, projection, fog);
		}
	}

	static parseStatus(efstConst) {
		if (!Session.Entity) {
			return;
		}

		if (efstConst == EFST.HEALTHSTATE_POISON) {
			Poison.setActive(true);
		}
		if (efstConst == EFST.HEALTHSTATE_BLIND) {
			Blind.setActive(true);
		}
		if (efstConst == EFST.ILLUSION) {
			VerticalFlip.setActive(true);
		}
	}

	static cleanStatusEffect(efstConst) {
		if (!Session.Entity) {
			return;
		}

		if (efstConst == EFST.HEALTHSTATE_POISON) {
			Poison.setActive(false);
		}
		if (efstConst == EFST.HEALTHSTATE_BLIND) {
			Blind.setActive(false);
		}
		if (efstConst == EFST.ILLUSION) {
			VerticalFlip.setActive(false);
		}
	}

	static clean() {
		Poison.setActive(false);
		Blind.setActive(false);
		VerticalFlip.setActive(false);
	}

	static renderBeforeEntities(gl, modelView, projection, fog, tick) {
		RainWeather.renderPuddles(gl, modelView, projection, fog, tick);
	}

	/**
	 * Rendering self screen effects
	 *
	 * @param {object} gl context
	 * @param {mat4} modelView
	 * @param {mat4} projection
	 * @param {object} fog structure
	 * @param {number} tick - game tick
	 * @param {boolean} beforeEntities - render before entities
	 */
	static render(gl, modelView, projection, fog, tick, beforeEntities = false) {
		if (!ScreenEffectManager.hasAnyActiveEffect()) {
			return;
		}

		if (beforeEntities) {
			ScreenEffectManager.renderBeforeEntities(gl, modelView, projection, fog, tick);
			return;
		}

		beforeRender(gl, modelView, projection, fog, tick);

		// Weather Effects
		SnowWeather.renderAll(gl, modelView, projection, fog, tick);
		RainWeather.renderAll(gl, modelView, projection, fog, tick);
		SakuraWeatherEffect.renderAll(gl, modelView, projection, fog, tick);
		PokJukWeatherEffect.renderAll(gl, modelView, projection, fog, tick);
		CloudWeatherEffect.renderAll(gl, modelView, projection, fog, tick);

		// Screen Efst status based
		ScreenEffectManager.renderStatusEffects(gl, modelView, projection, fog);

		afterRender(gl, modelView, projection, fog, tick);
	}

	/**
	 * Set night mode by changing the diffuse color
	 * This effect is the same as SC_SKE
	 * @param {boolean} night - true for night mode, false for day mode
	 */
	static setNight(night) {
		if (ScreenEffectManager._nightInterval) {
			clearInterval(ScreenEffectManager._nightInterval);
			ScreenEffectManager._nightInterval = null;
		}
		ScreenEffectManager._nightInterval = setInterval(function () {
			const diffuse = MapRenderer.diffuse;
			const light = MapRenderer.light;
			const step = 0.005;

			if (night) {
				if (diffuse[0] > 0.5) {
					diffuse[0] -= step;
				}
				if (diffuse[1] > 0.5) {
					diffuse[1] -= step;
				}
			} else {
				if (diffuse[0] < light.diffuse[0]) {
					diffuse[0] += step;
				}
				if (diffuse[1] < light.diffuse[1]) {
					diffuse[1] += step;
				}
			}

			light.env[0] = 1 - (1 - diffuse[0]) * (1 - light.ambient[0]);
			light.env[1] = 1 - (1 - diffuse[1]) * (1 - light.ambient[1]);
			light.env[2] = 1 - (1 - diffuse[2]) * (1 - light.ambient[2]);

			const done = night
				? diffuse[0] <= 0.5 && diffuse[1] <= 0.5
				: diffuse[0] >= light.diffuse[0] && diffuse[1] >= light.diffuse[1];

			if (done) {
				clearInterval(ScreenEffectManager._nightInterval);
				ScreenEffectManager._nightInterval = null;
			}
		}, 8);
	}
}

/**
 * Export
 */
export default ScreenEffectManager;
