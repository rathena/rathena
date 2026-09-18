/**
 * @module Renderer/Map/OccluderFade
 *
 * Fades map geometry standing between the camera and the player, so tight
 * interiors and dense forests stay readable. Shared by the static and animated
 * model renderers. Skipped in first person (nothing can stand in front of the eye).
 *
 * The fade only engages while something actually covers the player: each frame
 * the models are re-drawn (color and depth writes off) inside a GPU occlusion
 * query with a shader mode that keeps only fragments in a thin line of sight
 * capsule between the eye and the player. Any sample passing means the view is
 * blocked; the fade strength then eases in, and eases out once the view clears.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import _shaderSource from 'Renderer/Effects/Shaders/GLSL/OccluderFade.glsl?raw';
import Camera from 'Renderer/Camera.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import glMatrix from 'Utils/gl-matrix.js';

const { mat4, vec3 } = glMatrix;

const SHADER_INCLUDE = '// #include OccluderFade.glsl';

/**
 * Shader-side modes (uOccluderFadeMode)
 */
const MODE = {
	OFF: 0,
	DITHER: 1,
	ALPHA_OPAQUE: 2,
	ALPHA_BLEND: 3,
	QUERY: 4
};

/**
 * Preference values
 */
const SETTING = {
	OFF: 'off',
	DITHER: 'dither',
	ALPHA: 'alpha'
};

/**
 * Occlusion query slots, one per model renderer
 */
const QUERY = {
	MODELS: 0,
	ANIMATED: 1
};

/**
 * Radius (cells) of the line of sight capsule used to detect occluders
 */
const QUERY_RADIUS = 0.7;

/**
 * Time (ms) for the fade to fully ease in / out
 */
const FADE_IN_MS = 150;
const FADE_OUT_MS = 300;

const _inverse = mat4.create();
const _eye = vec3.create();

const _queries = [null, null];
const _queryPending = [false, false];
const _queryHit = [false, false];

let _strength = 0;
let _lastTick = 0;

class OccluderFade {
	static MODE = MODE;
	static SETTING = SETTING;
	static QUERY = QUERY;

	/**
	 * Inline the shared GLSL into a fragment shader source
	 *
	 * @param {string} source fragment shader
	 * @return {string}
	 */
	static injectShader(source) {
		return source.replace(SHADER_INCLUDE, _shaderSource);
	}

	/**
	 * Whether the effect can run: enabled in the graphics options and not in
	 * first person camera.
	 *
	 * @return {boolean}
	 */
	static isActive() {
		return GraphicsSettings.occluderFade !== SETTING.OFF && Camera.state !== Camera.states.first_person;
	}

	/**
	 * Whether the alpha (two pass) variant is selected
	 *
	 * @return {boolean}
	 */
	static useAlpha() {
		return GraphicsSettings.occluderFade === SETTING.ALPHA;
	}

	/**
	 * Current fade strength, 0 (view clear) .. 1 (view blocked)
	 *
	 * @return {number}
	 */
	static getStrength() {
		return _strength;
	}

	/**
	 * Whether the fade is visible this frame
	 *
	 * @return {boolean}
	 */
	static isFading() {
		return OccluderFade.isActive() && _strength > 0.001;
	}

	/**
	 * Per frame update: collect last frame's occlusion query results, ease
	 * the fade strength and refresh the camera eye position.
	 *
	 * @param {WebGL2RenderingContext} gl
	 * @param {mat4} modelView
	 * @param {number} tick
	 */
	static beginFrame(gl, modelView, tick) {
		mat4.invert(_inverse, modelView);
		_eye[0] = _inverse[12];
		_eye[1] = _inverse[13];
		_eye[2] = _inverse[14];

		const dt = _lastTick ? Math.min(tick - _lastTick, 100) : 0;
		_lastTick = tick;

		if (!OccluderFade.isActive()) {
			_strength = 0;
			return;
		}

		for (let i = 0; i < _queries.length; ++i) {
			OccluderFade.pollQuery(gl, i);
		}

		const occluded = _queryHit[QUERY.MODELS] || _queryHit[QUERY.ANIMATED];
		if (occluded) {
			_strength = Math.min(1, _strength + dt / FADE_IN_MS);
		} else {
			_strength = Math.max(0, _strength - dt / FADE_OUT_MS);
		}
	}

	/**
	 * Read back an occlusion query when its result is available
	 *
	 * @param {WebGL2RenderingContext} gl
	 * @param {number} slot one of QUERY
	 */
	static pollQuery(gl, slot) {
		if (!_queryPending[slot]) {
			return;
		}

		const query = _queries[slot];
		if (!gl.getQueryParameter(query, gl.QUERY_RESULT_AVAILABLE)) {
			return;
		}

		_queryHit[slot] = !!gl.getQueryParameter(query, gl.QUERY_RESULT);
		_queryPending[slot] = false;
	}

	/**
	 * Upload the fade uniforms for a program
	 *
	 * @param {WebGLRenderingContext} gl
	 * @param {object} uniform program uniform locations
	 * @param {number} mode one of MODE
	 */
	static setUniforms(gl, uniform, mode) {
		gl.uniform1i(uniform.uOccluderFadeMode, mode);

		if (mode === MODE.OFF) {
			return;
		}

		gl.uniform3fv(uniform.uOccluderFadeEye, _eye);
		gl.uniform3fv(uniform.uOccluderFadeFocus, Camera.focus);
		gl.uniform1f(
			uniform.uOccluderFadeRadius,
			mode === MODE.QUERY ? QUERY_RADIUS : GraphicsSettings.occluderFadeRadius
		);
		gl.uniform1f(uniform.uOccluderFadeOpacity, GraphicsSettings.occluderFadeOpacity);
		gl.uniform1f(uniform.uOccluderFadeStrength, _strength);
	}

	/**
	 * Whether a deferred translucent pass is required this frame
	 * (alpha variant selected and fade visible).
	 *
	 * @return {boolean}
	 */
	static needsBlendPass() {
		return OccluderFade.isFading() && OccluderFade.useAlpha();
	}

	/**
	 * Shader mode for the opaque geometry pass
	 *
	 * @return {number} one of MODE
	 */
	static opaqueMode() {
		if (!OccluderFade.isFading()) {
			return MODE.OFF;
		}
		return OccluderFade.useAlpha() ? MODE.ALPHA_OPAQUE : MODE.DITHER;
	}

	/**
	 * Opaque model pass: untouched, dithered, or with the fade capsule cut out.
	 *
	 * @param {WebGLRenderingContext} gl
	 * @param {object} uniform program uniform locations
	 * @param {function} draw issues the draw calls
	 */
	static renderOpaque(gl, uniform, draw) {
		OccluderFade.setUniforms(gl, uniform, OccluderFade.opaqueMode());
		SpriteRenderer.runWithDepth(true, true, true, draw);
	}

	/**
	 * Line of sight pass: re-draw the models without color/depth writes inside
	 * an occlusion query, keeping only fragments between the eye and the player.
	 * Skipped while the previous query of this slot is still in flight.
	 *
	 * @param {WebGL2RenderingContext} gl
	 * @param {object} uniform program uniform locations
	 * @param {function} draw issues the draw calls
	 * @param {number} slot one of QUERY
	 */
	static renderQuery(gl, uniform, draw, slot) {
		if (!OccluderFade.isActive() || _queryPending[slot]) {
			return;
		}

		if (!_queries[slot]) {
			_queries[slot] = gl.createQuery();
		}

		OccluderFade.setUniforms(gl, uniform, MODE.QUERY);
		gl.colorMask(false, false, false, false);
		gl.beginQuery(gl.ANY_SAMPLES_PASSED_CONSERVATIVE, _queries[slot]);
		SpriteRenderer.runWithDepth(false, false, true, draw);
		gl.endQuery(gl.ANY_SAMPLES_PASSED_CONSERVATIVE);
		gl.colorMask(true, true, true, true);
		_queryPending[slot] = true;
	}

	/**
	 * Translucent model pass (alpha variant): draws only the fade capsule,
	 * depth tested but not depth written. Runs after opaque scene elements
	 * (entities included) so they show through the faded geometry.
	 *
	 * @param {WebGLRenderingContext} gl
	 * @param {object} uniform program uniform locations
	 * @param {function} draw issues the draw calls
	 */
	static renderBlend(gl, uniform, draw) {
		OccluderFade.setUniforms(gl, uniform, MODE.ALPHA_BLEND);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		SpriteRenderer.runWithDepth(true, false, true, draw);
	}

	/**
	 * Release GPU queries (map change / context loss)
	 *
	 * @param {WebGL2RenderingContext} gl
	 */
	static free(gl) {
		for (let i = 0; i < _queries.length; ++i) {
			if (_queries[i]) {
				gl.deleteQuery(_queries[i]);
				_queries[i] = null;
			}
			_queryPending[i] = false;
			_queryHit[i] = false;
		}
		_strength = 0;
		_lastTick = 0;
	}
}

export default OccluderFade;
