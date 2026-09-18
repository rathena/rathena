/**
 * UI/Components/FPS/FPS.js
 *
 * Manage Graphics details
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author IssID, AoShinHo
 */

/**
 * UI/Components/FPS/FPS.js
 *
 * Manage Graphics details
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author IssID
 */

import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './FPS.html?raw';
import cssText from './FPS.css?raw';

/**
 * Create Component
 */
const FPS = new GUIComponent('FPS', cssText);

FPS.render = () => htmlText;

/** @type {number} */
let _maxFPSRegistered = 0;

/** @type {function} */
let _tickFn = null;

/**
 * @var {Preferences} Graphics
 */
const _preferences = Preferences.get(
	'FPS',
	{
		show: false,
		x: 100,
		y: 100
	},
	1.1
);

/**
 * Initialize UI
 */
FPS.init = function init() {
	const root = this.getRoot();

	const baseBtn = root.querySelector('.base');
	if (baseBtn) {
		baseBtn.addEventListener('mousedown', function (event) {
			event.stopImmediatePropagation();
		});
	}

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', function () {
			FPS.remove();
		});
	}

	this.draggable('.titlebar');
};

/**
 * When appended to DOM
 */
FPS.onAppend = function onAppend() {
	// Apply preferences
	this._host.style.top = _preferences.y + 'px';
	this._host.style.left = _preferences.x + 'px';
	this._host.style.display = _preferences.show ? '' : 'none';

	const root = this.getRoot();
	const fpsEl = root.querySelector('#fpsCounter');
	const fpsRoot = root.querySelector('#FPS');

	let startTime = 0;
	let frame = 0;
	let lastValue = null;
	let lastClass = null;

	const FPS_COLORS = {
		'fps-good': '#006400',
		'fps-warn': '#ff9800',
		'fps-bad': '#f44336'
	};

	function getFPSClass(value, frameLimit) {
		const ratio = value / frameLimit;

		if (ratio >= 0.7) return 'fps-good'; // >= 70%
		if (ratio >= 0.4) return 'fps-warn'; // >= 40%
		return 'fps-bad'; // < 40%
	}

	function tick(time) {
		frame++;
		if (time - startTime < 1000) {
			return;
		}
		const value = +(frame / ((time - startTime) / 1000)).toFixed(1);

		// Update text only if changed
		if (value !== lastValue) {
			fpsEl.textContent = value;
			lastValue = value;
		}

		// Check if FPS increased
		if (_maxFPSRegistered < value) _maxFPSRegistered = value;
		const limit = Renderer.frameLimit > 0 ? Renderer.frameLimit : _maxFPSRegistered;
		const cls = getFPSClass(value, limit);

		// Update class only if changed
		if (cls !== lastClass) {
			fpsRoot.style.color = FPS_COLORS[cls] || FPS_COLORS['fps-good'];
			lastClass = cls;
		}

		startTime = time;
		frame = 0;
	}

	// Passive FPS listener (no render logic impact)
	if (_tickFn) Renderer.stop(_tickFn);
	_tickFn = tick;
	Renderer.render(tick);
};

/**
 * Once remove, save preferences
 */
FPS.onRemove = function onRemove() {
	if (_tickFn) {
		Renderer.stop(_tickFn);
		_tickFn = null;
	}
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.show = this._host.style.display !== 'none';
	_preferences.save();
};

/**
 * Show/Hide UI
 */
FPS.toggle = function toggle(isVisible) {
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);

	if (typeof isVisible === 'boolean') {
		// Explicit show/hide from GraphicsOption checkbox
		this._host.style.display = isVisible ? '' : 'none';
	} else {
		// Keyboard shortcut or other toggle — flip current state
		this._host.style.display = this._host.style.display === 'none' ? '' : 'none';
	}

	_preferences.show = this._host.style.display !== 'none';
	_preferences.save();

	if (this._host.style.display !== 'none') {
		this.focus();
	}
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(FPS);
