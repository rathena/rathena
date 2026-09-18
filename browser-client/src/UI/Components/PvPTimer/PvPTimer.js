/**
 * UI/Components/PvPTimer/PvPTimer.js
 *
 * PvP timer GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Client from 'Core/Client.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import htmlText from './PvPTimer.html?raw';
import cssText from './PvPTimer.css?raw';

const PvPTimer = new GUIComponent('PvPTimer', cssText);

PvPTimer.render = () => htmlText;

PvPTimer.mouseMode = GUIComponent.MouseMode.CROSS;

PvPTimer.needFocus = false;

/* ================= CONFIG (OG values) ================= */

const TIMER_W = 300,
	TIMER_H = 110;
const TA_W = 360,
	TA_H = 128;

const TIMER_Y = 60;
const TA_Y = 80;

/* ================= CANVASES ================= */

let _timerCanvas, _timerCtx;
let _taCanvas, _taCtx;

/* ================= ACT / SPR ================= */

let _timefontAct, _timefontSpr;
let _timeAtkAct, _timeAtkSpr;

/* ================= TIMER ================= */

let _timerInterval = null;
let _startTs = 0;

const _layerEntity = new Entity();

let _taHideTimer = null;

let isFirstTime = true;

/**
 * Initialize UI
 */
PvPTimer.init = function init() {
	Client.loadFiles(
		[
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/timefont.act',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/timefont.spr',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/timeattack.act',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/timeattack.spr'
		],
		(tAct, tSpr, aAct, aSpr) => {
			_timefontAct = tAct;
			_timefontSpr = tSpr;
			_timeAtkAct = aAct;
			_timeAtkSpr = aSpr;
		}
	);

	const root = this.getRoot();
	_timerCanvas = root.querySelector('.pvp-timer-canvas');
	_taCanvas = root.querySelector('.pvp-timeattack-canvas');

	if (!_timerCanvas || !_taCanvas) {
		return;
	}

	_timerCanvas.width = TIMER_W;
	_timerCanvas.height = TIMER_H;
	_taCanvas.width = TA_W;
	_taCanvas.height = TA_H;

	_timerCtx = _timerCanvas.getContext('2d');
	_taCtx = _taCanvas.getContext('2d');
};

/**
 * Append UI
 */
PvPTimer.onAppend = function onAppend() {
	this._host.style.display = 'none';
};

/**
 * Remove UI
 */
PvPTimer.onRemove = function onRemove() {
	isFirstTime = true;
	stopTimer();
};

/**
 * Set data
 * @param {Object} data
 */
PvPTimer.setData = function setData(_data) {};

PvPTimer.hide = function hide() {
	this._host.style.display = 'none';
	stopTimer();
};

PvPTimer.show = function show() {
	this._host.style.display = '';
	startTimer();
	if (isFirstTime === true) {
		playTimeAttackBanner();
		isFirstTime = false;
	}
};

/**
 * Pick layers from act
 * @param {Object} act
 * @param {number} actionId
 * @param {number} frameId
 * @returns {Object[]}
 */
function pickLayers(act, actionId, frameId) {
	const a = act.actions[actionId];
	if (!a || !a.animations || !a.animations.length) {
		return null;
	}

	const idx = frameId !== undefined ? frameId : (a.animations.length / 2) | 0;
	if (idx >= a.animations.length) {
		return null;
	}

	return a.animations[idx].layers;
}

function drawActionToCanvas(ctx, act, spr, actionId, x, y, frameId) {
	const layers = pickLayers(act, actionId, frameId);
	if (!layers) {
		return;
	}

	SpriteRenderer.bind2DContext(ctx, x, y);

	for (let i = 0; i < layers.length; i++) {
		_layerEntity.renderLayer(layers[i], spr, spr, 1.0, [0, 0], false);
	}
}

function timerCharToAction(ch) {
	if (ch === ':') {
		return 10;
	}
	return parseInt(ch, 10);
}

function renderTimer(seconds) {
	if (!_timerCtx || !_timefontAct) {
		return;
	}

	_timerCtx.clearRect(0, 0, TIMER_W, TIMER_H);

	const m = Math.floor(seconds / 60);
	const s = seconds % 60;

	const text = m === 0 ? String(s).padStart(2, '0') : `${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;

	const digitWidth = 50;
	const totalWidth = m === 0 ? 2 * digitWidth : 5 * digitWidth;

	let x = (TIMER_W - totalWidth) >> 1;

	for (let i = 0; i < text.length; i++) {
		const a = timerCharToAction(text[i]);

		if (!isNaN(a)) {
			drawActionToCanvas(_timerCtx, _timefontAct, _timefontSpr, a, x, TIMER_Y);
			x += digitWidth;
		}
	}
}

function startTimer() {
	if (_timerInterval) {
		return;
	}
	_startTs = (Date.now() / 1000) | 0;
	renderTimer(0);
	_timerInterval = setInterval(() => {
		renderTimer(((Date.now() / 1000) | 0) - _startTs);
	}, 1000);
}

function stopTimer() {
	if (_timerInterval) {
		clearInterval(_timerInterval);
	}
	_timerInterval = null;
	if (_timerCtx) {
		_timerCtx.clearRect(0, 0, TIMER_W, TIMER_H);
	}
}

/* ================= TIME ATTACK ================= */

function playTimeAttackBanner() {
	if (!_taCtx || !_timeAtkAct) {
		return;
	}

	if (_taHideTimer) {
		clearTimeout(_taHideTimer);
		_taHideTimer = null;
	}

	_taCtx.clearRect(0, 0, TA_W, TA_H);

	const action = _timeAtkAct.actions[0];
	if (!action || !action.animations) {
		return;
	}

	let frame = 0;
	const count = action.animations.length;

	function run() {
		_taCtx.clearRect(0, 0, TA_W, TA_H);
		drawActionToCanvas(_taCtx, _timeAtkAct, _timeAtkSpr, 0, (TA_W >> 1) - 100, TA_Y, frame);

		frame++;

		if (frame < count) {
			_taHideTimer = setTimeout(run, 100);
		} else {
			_taHideTimer = setTimeout(() => {
				_taCtx.clearRect(0, 0, TA_W, TA_H);
			}, 300);
		}
	}

	run();
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(PvPTimer);
