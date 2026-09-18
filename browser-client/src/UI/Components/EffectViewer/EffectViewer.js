/**
 * UI/Components/EffectViewer/EffectViewer.js
 *
 * Effect Viewer
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import EffectManager from 'Renderer/EffectManager.js';
import EffectTable from 'DB/Effects/EffectTable.js';
import EC from 'DB/Effects/EffectConst.js';
import EntityManager from 'Renderer/EntityManager.js';
import Entity from 'Renderer/Entity/Entity.js';
import Session from 'Engine/SessionStorage.js';
import Camera from 'Renderer/Camera.js';
import MapControl from 'Controls/MapControl.js';
import Mouse from 'Controls/MouseEventHandler.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './EffectViewer.html?raw';
import cssText from './EffectViewer.css?raw';

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

let _selectedEffect = null;

/**
 * Create EffectViewer component
 */
const Viewer = new GUIComponent('EffectViewer', cssText);

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

	Mouse.intersect = true;
	MapControl.init();
	Renderer.show();
	EffectManager.init(Renderer.getContext());
	SpriteRenderer.init(Renderer.getContext());
	Client.init([]);

	// We need an entity in order to apply effect
	Session.Entity = new Entity({ PosDir: [0, 0, 0], position: [0, 0, 0], GID: 0 });
	EntityManager.add(Session.Entity);
	Camera.setTarget(Session.Entity);
	Camera.init();

	// Initialize the dropdown
	const root = this.getRoot();
	initDropDown(root.querySelector('select'), root.querySelector('button'));
};

/**
 * Once append to body, set body styles
 */
Viewer.onAppend = function onAppend() {
	document.body.style.backgroundColor = '#45484d';
	document.body.style.fontFamily = 'Arial';
	document.body.style.margin = '0';
	document.body.style.overflow = 'hidden';
};

/**
 * Initialise Drop Down list
 *
 * @param {HTMLElement} select dropdown
 * @param {HTMLElement} button play button
 */
function initDropDown(select, button) {
	let hash = decodeURIComponent(location.hash);
	if (hash) {
		hash = hash.substring(1);
		_selectedEffect = hash;
	}

	function getKeyByValue(object, value) {
		if (isNaN(value)) {
			return undefined;
		} else {
			return Object.keys(object).find(key => object[key] === Number(value));
		}
	}

	for (const effectId of Object.keys(EffectTable)) {
		const ef_name = getKeyByValue(EC, effectId);
		select.add(
			new Option(
				ef_name !== undefined ? `${effectId} (${ef_name})` : effectId,
				effectId,
				null,
				_selectedEffect === effectId
			),
			null
		);
	}
	select.onchange = function () {
		_selectedEffect = this.value;
		saveEffectId(_selectedEffect);
		loadEffect(this.value);
	};

	button.onclick = () => {
		loadEffect(_selectedEffect);
	};

	loadEffect(0);

	const root = Viewer.getRoot();
	root.querySelector('.head').style.display = 'block';
	select.focus();
}

function saveEffectId(effectId) {
	location.hash = effectId;
}

/**
 * Start loading an effect
 *
 * @param {string} effectId
 */
function loadEffect(effectId) {
	stop();
	Session.Entity.renderEntity();
	EffectManager.spam({
		effectId: effectId,
		ownerAID: Session.Entity.GID,
		otherAID: Session.Entity.GID,
		position: Session.Entity.position
	});
	Renderer.render(render);
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
	// Clear screen, update camera
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	Camera.update(tick);
	const modelView = Camera.modelView;
	EffectManager.render(gl, modelView, Camera.projection, _fog, tick, false);
	EffectManager.render(gl, modelView, Camera.projection, _fog, tick, true);
}

/**
 * Exports methods
 */
Viewer.loadEffect = loadEffect;
Viewer.stop = stop;

Camera.init = function Init() {
	this.lastTick = Date.now();

	this.angle[0] = this.range % 360.0;
	this.angle[1] = this.rotationFrom % 360.0;
	this.angleFinal[0] = this.range % 360.0;
	this.angleFinal[1] = this.rotationFrom % 360.0;

	this.position[0] = -this.target.position[0];
	this.position[1] = -this.target.position[1];
	this.position[2] = this.target.position[2];

	this.altitudeRange = this.altitudeTo - this.altitudeFrom;

	this.zoomFinal = 125;
};
Camera.rotate = function Rotate(active) {
	const action = this.action;
	const tick = Date.now();

	if (!active) {
		action.active = false;
		return;
	}

	this.angleFinal[1] = 0.0;
	action.x = Mouse.screen.x;
	action.y = Mouse.screen.y;
	action.tick = tick;
	action.active = true;
};

/**
 * Stored component and return it
 */
export default Viewer;
