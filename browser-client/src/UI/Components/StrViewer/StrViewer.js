/**
 * UI/Components/StrViewer/StrViewer.js
 *
 * STR Effect Viewer
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Configs from 'Core/Configs.js';
import Renderer from 'Renderer/Renderer.js';
import EffectManager from 'Renderer/EffectManager.js';
import StrEffect from 'Renderer/Effects/StrEffect.js';
import Camera from 'Renderer/Camera.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './StrViewer.html?raw';
import cssText from './StrViewer.css?raw';

/**
 * Load dependencies
 */
const mat4 = glMatrix.mat4;

/**
 * @var {object} fog structure
 */
const _fog = {
	use: false,
	exist: true,
	far: 30,
	near: 180,
	factor: 1.0,
	color: new Float32Array([1, 1, 1])
};

/**
 * @var {mat4} model view mat
 */
const _modelView = mat4.create();

/**
 * @var {StrEffect} current str effect
 */
let _strObject = null;

/**
 * Create StrViewer component
 */
const Viewer = new GUIComponent('StrViewer', cssText);

Viewer.render = () => htmlText;

/**
 * Initialize Component
 */
Viewer.init = function init() {
	// Initialize WebGL
	Renderer.init({
		alpha: false,
		depth: true,
		stencil: false,
		antialias: true,
		premultipliedAlpha: false
	});

	Renderer.show();
	EffectManager.init(Renderer.getContext());
	Client.init([]);

	const root = Viewer.getRoot();

	if (!Configs.get('api')) {
		initDropDown(root.querySelector('select'));
	} else {
		const hash = decodeURIComponent(location.hash);
		location.hash = hash;
		if (hash.length > 1) {
			loadEffect(hash.substr(1));
		}
	}
};

/**
 * Once append to body, set body styles
 */
Viewer.onAppend = function onAppend() {
	document.body.style.backgroundColor = '#45484d';
	document.body.style.fontFamily = 'Arial';
	document.body.style.fontSize = '12px';
	document.body.style.margin = '0';
	document.body.style.overflow = 'hidden';
};

/**
 * Initialise Drop Down list
 *
 * @param {HTMLElement} select dropdown
 */
function initDropDown(select) {
	Client.search(/data\\[^\0]+\.str/gi, list => {
		list.sort();

		for (let i = 0, count = list.length; i < count; ++i) {
			list[i] = list[i].replace(/\\/g, '/');
			select.add(new Option(list[i], list[i]), null);
		}

		select.onchange = function () {
			loadEffect((location.hash = this.value));
		};

		const hash = decodeURIComponent(location.hash);
		location.hash = hash;

		if (hash.indexOf('.str') !== -1) {
			loadEffect(hash.substr(1));
			select.value = hash.substr(1);
		} else {
			loadEffect(select.value);
		}

		const root = Viewer.getRoot();
		root.querySelector('.head').style.display = 'block';
		select.focus();
	});
}

/**
 * Start loading an effect
 *
 * @param {string} filename
 */
function loadEffect(filename) {
	stop();

	Client.loadFile(filename, () => {
		if (_strObject) {
			_strObject._needCleanUp = true;
		}

		_strObject = new StrEffect(filename, [0, 0, 0], Renderer.tick);
		_strObject._needCleanUp = false;

		Object.defineProperty(_strObject, 'needCleanUp', {
			get: function () {
				return this._needCleanUp;
			},
			set: function (val) {
				if (val) {
					this.startTick = Renderer.tick;
				}
			}
		});

		EffectManager.add(_strObject, 0);
		Renderer.render(render);
	});
}

/**
 * Stop to render
 */
function stop() {
	const gl = Renderer.getContext();

	Renderer.stop();
	EffectManager.free(gl);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}

/**
 * Rendering scene
 *
 * @param {number} tick
 * @param {object} webgl context
 */
function render(tick, gl) {
	// Updating camera position
	mat4.identity(_modelView);
	mat4.translate(_modelView, _modelView, [0, -3, -50]);
	mat4.rotateX(_modelView, _modelView, (50 / 180) * Math.PI);

	// Clear screen, update camera
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	EffectManager.render(gl, _modelView, Camera.projection, _fog, tick, false);
}

/**
 * Export
 */
Viewer.loadEffect = loadEffect;
Viewer.stop = stop;

/**
 * Stored component and return it
 */
export default Viewer;
