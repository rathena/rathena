/**
 * UI/Components/ModelViewer/ModelViewer.js
 *
 * Model Viewer (rsm file)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import Configs from 'Core/Configs.js';
import Client from 'Core/Client.js';
import Model from 'Loaders/Model.js';
import Renderer from 'Renderer/Renderer.js';
import ModelRenderer from 'Renderer/Map/Models.js';
import Camera from 'Renderer/Camera.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './ModelViewer.html?raw';
import cssText from './ModelViewer.css?raw';

/**
 * Load dependencies
 */
const mat4 = glMatrix.mat4;
const mat3 = glMatrix.mat3;

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
 * @var {object} light structure
 */
const _light = {
	opacity: 1.0,
	ambient: new Float32Array([Math.PI, Math.PI, Math.PI]),
	diffuse: new Float32Array([0, 0, 0]),
	direction: new Float32Array([0, 1, 0]),
	env: new Float32Array([1, 1, 1])
};

/**
 * @var {object} model global parameters
 */
const _GlobalParameters = {
	position: new Float32Array(3),
	rotation: new Float32Array(3),
	scale: new Float32Array([-0.075, -0.075, 0.075]),
	filename: null
};

/**
 * @var {mat4} model view mat
 */
const _modelView = new Float32Array(4 * 4);

/**
 * @var {mat3} normal mat
 */
const _normalMat = new Float32Array(3 * 3);

/**
 * @var {Model} current model
 */
let _model = null;

/**
 * Create ModelViewer component
 */
const Viewer = new GUIComponent('GRFViewer', cssText);

Viewer.render = () => htmlText;

/**
 * Initialize Component
 */
Viewer.init = function init() {
	// Initialize WebGL
	Renderer.init({
		alpha: true,
		depth: true,
		stencil: false,
		antialias: true,
		premultipliedAlpha: false
	});

	Renderer.show();

	const root = Viewer.getRoot();

	if (!Configs.get('api')) {
		initDropDown(root.querySelector('select'));
	} else {
		const hash = decodeURIComponent(location.hash);
		location.hash = hash;
		if (hash.length > 1) {
			loadModel(hash.substr(1));
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
	Client.search(/data\\[^\0]+\.rsm/gi, list => {
		const hash = decodeURIComponent(location.hash);

		for (let i = 0, count = list.length; i < count; ++i) {
			list[i] = list[i].replace(/\\/g, '/');
			select.add(new Option(list[i], list[i]), null);
		}

		select.onchange = function () {
			loadModel((location.hash = this.value));
		};

		location.hash = hash;

		if (hash.indexOf('.rsm') !== -1) {
			loadModel(hash.substr(1));
			select.value = hash.substr(1);
		} else {
			loadModel(select.value);
		}

		const root = Viewer.getRoot();
		root.querySelector('.head').style.display = 'block';
		select.focus();
	});
}

/**
 * Stop to render
 */
function stop() {
	const gl = Renderer.getContext();

	Renderer.stop();
	ModelRenderer.free(gl);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
}

/**
 * Start loading a model
 *
 * @param {string} filename
 */
function loadModel(filename) {
	stop();

	Client.getFile(filename, buf => {
		_model = new Model(buf);

		let i, count, j, size, offset, length;
		const objects = [];
		const infos = [];
		let meshes;
		let index;
		let object;

		_GlobalParameters.filename = filename.replace('data/model/', '');
		_model.createInstance(_GlobalParameters, 0, 0);

		const data = _model.compile();

		count = data.meshes.length;
		let total = 0;
		for (i = 0; i < count; ++i) {
			meshes = data.meshes[i];
			index = Object.keys(meshes);

			for (j = 0, size = index.length; j < size; ++j) {
				objects.push({
					texture: data.textures[index[j]],
					alpha: _model.alpha,
					mesh: meshes[index[j]]
				});

				total += meshes[index[j]].length;
			}
		}

		const buffer = new Float32Array(total);
		count = objects.length;
		offset = 0;

		for (i = 0; i < count; ++i) {
			object = objects[i];
			length = object.mesh.length;

			infos[i] = {
				texture: `data/texture/${object.texture}`,
				vertOffset: offset / 9,
				vertCount: length / 9
			};

			buffer.set(object.mesh, offset);
			offset += length;
		}

		i = -1;
		function loadNextTexture() {
			if (++i === count) {
				ModelRenderer.init(Renderer.getContext(), {
					buffer: buffer,
					infos: infos
				});

				Renderer.render(render);
				return;
			}

			Client.loadFile(
				infos[i].texture,
				binaryData => {
					infos[i].texture = binaryData;
					loadNextTexture();
				},
				loadNextTexture
			);
		}

		loadNextTexture();
	});
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
	mat4.translate(_modelView, _modelView, [0, -_model.box.range[1] * 0.1, -_model.box.range[1] * 0.5 - 5]);
	mat4.rotateX(_modelView, _modelView, (15 / 180) * Math.PI);
	mat4.rotateY(_modelView, _modelView, (((tick / 1000) * 360) / 8 / 180) * Math.PI);

	// Calculate normal mat
	mat4.toInverseMat3(_modelView, _normalMat);
	mat3.transpose(_normalMat, _normalMat);

	// Clear screen, update camera
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	ModelRenderer.render(gl, _modelView, Camera.projection, _normalMat, _fog, _light);
}

/**
 * Export
 */
Viewer.loadModel = loadModel;
Viewer.stop = stop;

/**
 * Stored component and return it
 */
export default Viewer;
