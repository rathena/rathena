/**
 * UI/Components/Escape/Escape.js
 *
 * Game Escape window, manage options
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import KEYS from 'Controls/KeyEventHandler.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import SoundOption from 'UI/Components/SoundOption/SoundOption.js';
import GraphicsOption from 'UI/Components/GraphicsOption/GraphicsOption.js';
import ShortCutOption from 'UI/Components/ShortCutOption/ShortCutOption.js';
import htmlText from './Escape.html?raw';
import cssText from './Escape.css?raw';

/**
 * Create Escape window component
 */
const Escape = new GUIComponent('Escape', cssText);

/**
 * Render HTML
 */
Escape.render = () => htmlText;

/**
 * Initialize UI
 */
Escape.init = function init() {
	const root = this.getRoot();
	const rect = this._host.getBoundingClientRect();
	this._host.style.top = (Renderer.height - rect.height) * 0.75 + 'px';
	this._host.style.left = (Renderer.width - rect.width) * 0.5 + 'px';
	this.draggable();

	const nodeBtn = root.querySelector('.node');
	if (nodeBtn) {
		nodeBtn.addEventListener('mousedown', function (event) {
			event.stopImmediatePropagation();
			return false;
		});
	}

	// Only used in specific case
	root.querySelectorAll('button').forEach(function (el) {
		el.style.display = '';
	});
	root.querySelectorAll('.resurection, .savepoint').forEach(function (el) {
		el.style.display = 'none';
	});

	root.querySelector('.sound').addEventListener('click', onToggleSoundUI);
	root.querySelector('.graphics').addEventListener('click', onToggleGraphicUI);
	root.querySelector('.resurection').addEventListener('click', function () {
		Escape.onResurectionRequest();
	});
	root.querySelector('.savepoint').addEventListener('click', function () {
		Escape.onReturnSavePointRequest();
	});
	root.querySelector('.charselect').addEventListener('click', function () {
		Escape.onCharSelectionRequest();
	});
	root.querySelector('.hotkey').addEventListener('click', onToggleShortcutUI);
	root.querySelector('.exit').addEventListener('click', function () {
		Escape.onExitRequest();
	});
	root.querySelector('.cancel').addEventListener('click', function () {
		Escape._host.style.display = 'none';
	});

	// Start hidden
	this._host.style.display = 'none';
};

/**
 * Window must not be visible once append
 * but need to be here to manage key event
 */
Escape.onAppend = function onAppend() {
	this._host.style.display = 'none';
};

/**
 * Reset buttons once UI is removed
 */
Escape.onRemove = function onRemove() {
	this._host.style.display = 'none';
	const root = this.getRoot();
	root.querySelectorAll('.resurection, .savepoint').forEach(function (el) {
		el.style.display = 'none';
	});
	root.querySelectorAll('.graphics, .sound, .hotkey').forEach(function (el) {
		el.style.display = '';
	});
};

/**
 * Key Listener
 *
 * @param {object} event
 * @return {boolean}
 */
Escape.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		if (this._host.style.display === 'none') {
			this._host.style.display = '';
			this.focus();
		} else {
			this._host.style.display = 'none';
		}
	}
};

/**
 * Click on Sound button, toggle the UI
 */
function onToggleSoundUI() {
	if (!SoundOption._host || !SoundOption._host.parentNode) {
		SoundOption.append();
	} else {
		SoundOption.remove();
	}
}

/**
 * Click on Graphic button, toggle the UI
 */
function onToggleGraphicUI() {
	if (!GraphicsOption._host || !GraphicsOption._host.parentNode) {
		GraphicsOption.append();
	} else {
		GraphicsOption.remove();
	}
}

/**
 * Click on Shortcut button, toggle the UI
 */
function onToggleShortcutUI() {
	if (!ShortCutOption._host || !ShortCutOption._host.parentNode) {
		ShortCutOption.append();
	} else {
		ShortCutOption.remove();
	}
}

/**
 * Show death menu (called when player dies)
 */
Escape.showDeathMenu = function showDeathMenu(hasSiegfried) {
	const root = this.getRoot();
	this._host.style.display = '';
	root.querySelector('.savepoint').style.display = '';
	if (hasSiegfried) {
		root.querySelector('.resurection').style.display = '';
	}
	root.querySelectorAll('.graphics, .sound, .hotkey').forEach(function (el) {
		el.style.display = 'none';
	});
};

/**
 * Reset to normal menu (called when player resurrects)
 */
Escape.resetMenu = function resetMenu() {
	this._host.style.display = 'none';
	const root = this.getRoot();
	root.querySelectorAll('.resurection, .savepoint').forEach(function (el) {
		el.style.display = 'none';
	});
	root.querySelectorAll('.graphics, .sound, .hotkey').forEach(function (el) {
		el.style.display = '';
	});
};

/**
 * @var {function} callback when player want to resurect using Token of Siegfried
 */
Escape.onResurectionRequest = function onResurectionRequest() {};

/**
 * @var {function} callback to define to disconnect from game
 */
Escape.onExitRequest = function onExitRequest() {};

/**
 * @var {function} callback when player want to resurect using Token of Siegfried
 */
Escape.onReturnSavePointRequest = function onReturnSavePointRequest() {};

/**
 * @var {function} callback when player want to return to char selection
 */
Escape.onCharSelectionRequest = function onCharSelectionRequest() {};

Escape.mouseMode = GUIComponent.MouseMode.STOP;
Escape.needFocus = true;

/**
 * Create component and export it
 */
export default UIManager.addComponent(Escape);
