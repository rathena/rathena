/**
 * Renderer/Effects/Sky.js
 *
 * Rendering blue sky effects
 * TODO: Create a particle class to manage the process
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import WeatherTable from 'DB/Effects/WeatherEffect.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';

import { vec3 } from 'Utils/gl-matrix.js';

/**
 * @var {number} number of clouds to render
 */
const MAX_CLOUDS = 150;

/**
 * @var {Array} clouds particles
 */
const _clouds = new Array(MAX_CLOUDS);

/**
 * @var {Array} textures list
 */
const _textures = [];

/**
 * @var {vec4} RGBA color
 */
let _color = null;

/**
 * @var {boolean} display clouds ?
 */
let _display = true;

/**
 * Prepare cloud data
 *
 * @param {object} gl context
 * @param {string} mapname
 */
function init(gl, mapname) {
	let i;

	// Not found on weather, black sky, no cloud.
	if (!WeatherTable.sky[mapname]) {
		gl.clearColor(0.0, 0.0, 0.0, 1.0);
		_display = false;
		return;
	}

	// Save color
	_color = WeatherTable.sky[mapname].cloudColor;
	const color = WeatherTable.sky[mapname].skyColor;
	if (_color) {
		_display = true;
	} else {
		_display = false;
	}

	gl.clearColor(color[0], color[1], color[2], color[3]);

	// Add images to GPU
	if (!_textures.length && _display) {
		_textures.length = 8;

		for (i = 0; i < 7; i++) {
			loadCloudTexture(gl, i);
		}
	}
}

/**
 * Loading cloud texture index
 *
 * @param {object} gl context
 * @param {number} cloud texture index
 */
function loadCloudTexture(gl, i) {
	Client.loadFile('data/texture/effect/cloud' + (i + 1) + '.tga', function (buffer) {
		WebGL.texture(gl, buffer, function (texture) {
			_textures[i] = texture;
		});
	});
}

/**
 * Set up cloud data
 */
function setUpCloudData() {
	let i;

	// Add sprites to scene
	for (i = 0; i < MAX_CLOUDS; i++) {
		if (!_clouds[i]) {
			_clouds[i] = {
				position: vec3.create(),
				direction: vec3.create(),
				born_tick: 0,
				death_tick: 0
			};
		}
		cloudInit(_clouds[i]);
		_clouds[i].sprite = (Math.random() * (_textures.length - 1)) | 0;
		_clouds[i].death_tick = _clouds[i].born_tick + Math.random() * 8000;
		_clouds[i].born_tick -= 2000;
	}

	// Sort by textures
	_clouds.sort(function (a, b) {
		return a.sprite - b.sprite;
	});
}

/**
 * Initialize cloud element
 */
function cloudInit(cloud) {
	const pos = Session.Entity.position;

	cloud.position[0] = pos[0] + ((Math.random() * 35) | 0) * (Math.random() > 0.5 ? 1 : -1);
	cloud.position[1] = pos[1] + ((Math.random() * 35) | 0) * (Math.random() > 0.5 ? 1 : -1);
	cloud.position[2] = -10.0;

	cloud.direction[0] = Math.random() * 0.02 - 0.01;
	cloud.direction[1] = Math.random() * 0.02 - 0.01;
	cloud.direction[2] = Math.random() * 0.002 - 0.001;

	cloud.born_tick = cloud.death_tick ? cloud.death_tick + 2000 : Date.now();
	cloud.death_tick = cloud.born_tick + 6000;
}

/**
 * Rendering clouds on maps
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {object} fog structure
 * @param {number} tick - game tick
 */
function render(gl, modelView, projection, fog, tick) {
	if (!_display) {
		return;
	}

	let i, cloud, opacity;

	// Init program - interact with environment (use depth test + ray-plane)
	SpriteRenderer.bind3DContext(gl, modelView, projection, fog);

	// Base parameters
	SpriteRenderer.color.set(_color);
	SpriteRenderer.shadow = 1.0;
	SpriteRenderer.angle = 0;
	SpriteRenderer.size[0] = 500;
	SpriteRenderer.size[1] = 500;
	SpriteRenderer.offset[0] = 0;
	SpriteRenderer.offset[1] = 0;
	SpriteRenderer.image.palette = null;
	SpriteRenderer.depth = 0;

	for (i = 0; i < MAX_CLOUDS; i++) {
		cloud = _clouds[i];

		// Appear
		if (cloud.born_tick + 1000 > tick) {
			opacity = (tick - cloud.born_tick) / 1000;
		}

		// Remove
		else if (cloud.death_tick + 2000 < tick) {
			cloudInit(cloud);
			opacity = 0.0;
		}

		// Disapear
		else if (cloud.death_tick < tick) {
			opacity = 1.0 - (tick - cloud.death_tick) / 2000;
		}

		// Default
		else {
			opacity = 1.0;
		}

		SpriteRenderer.zIndex = 0;
		SpriteRenderer.color[3] = opacity;
		SpriteRenderer.image.texture = _textures[cloud.sprite];

		// Calculate position
		vec3.add(cloud.position, cloud.position, cloud.direction);
		SpriteRenderer.position.set(cloud.position);
		SpriteRenderer.runWithDepth(true, false, true, function () {
			SpriteRenderer.render();
		});
	}

	// Clean up
	SpriteRenderer.unbind(gl);
}

/**
 * Export
 */
export default {
	init: init,
	setUpCloudData: setUpCloudData,
	render: render
};
