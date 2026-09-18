/**
 * UI/Components/PvPCount/PvPCount.js
 *
 * PvP count GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Client from 'Core/Client.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import Sound from 'Audio/SoundManager.js';
import htmlText from './PvPCount.html?raw';
import cssText from './PvPCount.css?raw';

const PvPCount = new GUIComponent('PvPCount', cssText);

PvPCount.render = () => htmlText;

PvPCount.mouseMode = GUIComponent.MouseMode.CROSS;

PvPCount.needFocus = false;

/* ================= CONFIG (OG values) ================= */

const RANK_W = 240,
	RANK_H = 96;
const RANK_Y = 52;

/* ================= CANVASES ================= */

let _rankCanvas, _rankCtx;

/* ================= ACT / SPR ================= */

let _rankfontAct, _rankfontSpr;

const _layerEntity = new Entity();

let ranking = 0;
let total = 0;

/**
 * Initialize UI
 */
PvPCount.init = function init() {
	Client.loadFiles(
		['data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/rankfont.act', 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/rankfont.spr'],
		(rAct, rSpr) => {
			_rankfontAct = rAct;
			_rankfontSpr = rSpr;
		}
	);

	const root = this.getRoot();
	_rankCanvas = root.querySelector('.pvp-rank-canvas');

	if (!_rankCanvas) {
		return;
	}

	_rankCanvas.width = RANK_W;
	_rankCanvas.height = RANK_H;

	_rankCtx = _rankCanvas.getContext('2d');
};

/**
 * Append UI
 */
PvPCount.onAppend = function onAppend() {};

/**
 * Remove UI
 */
PvPCount.onRemove = function onRemove() {
	ranking = 0;
	total = 0;
	clearRank();
};

/**
 * Set data
 * @param {Object} data
 */
PvPCount.setData = function setData(data) {
	if (data.ranking === ranking && data.total === total) {
		return;
	}

	renderRankText(`${data.ranking}/${data.total}`);

	if (data.total > total) {
		Sound.play('effect/number_change.wav');
	}

	ranking = data.ranking;
	total = data.total;
};

/**
 * Pick layers from act
 * @param {Object} act
 * @param {number} actionId
 * @returns {Object[]}
 */
function pickLayers(act, actionId) {
	const a = act.actions[actionId];
	if (!a || !a.animations || !a.animations.length) {
		return null;
	}
	return a.animations[(a.animations.length / 2) | 0].layers;
}

function drawActionToCanvas(ctx, act, spr, actionId, x, y) {
	const layers = pickLayers(act, actionId);
	if (!layers) {
		return;
	}

	SpriteRenderer.bind2DContext(ctx, x, y);

	for (let i = 0; i < layers.length; i++) {
		_layerEntity.renderLayer(layers[i], spr, spr, 1.0, [0, 0], false);
	}
}

function rankCharToAction(ch) {
	if (ch === '/') {
		return 10;
	}
	return parseInt(ch, 10);
}

function renderRankText(text) {
	if (!_rankCtx || !_rankfontAct) {
		return;
	}

	_rankCtx.clearRect(0, 0, RANK_W, RANK_H);

	const parts = text.split('/');
	const rankingStr = parts[0];
	const totalStr = parts[1];
	const step = 28;
	const slashStep = 28;

	const totalWidth = rankingStr.length * step + slashStep + totalStr.length * step;
	let x = (RANK_W - totalWidth) >> 1;

	let i, a;
	for (i = 0; i < rankingStr.length; i++) {
		a = rankCharToAction(rankingStr[i]);
		if (!isNaN(a)) {
			drawActionToCanvas(_rankCtx, _rankfontAct, _rankfontSpr, a, x, RANK_Y - 6);
			x += step;
		}
	}

	a = rankCharToAction('/');
	drawActionToCanvas(_rankCtx, _rankfontAct, _rankfontSpr, a, x, RANK_Y);
	x += slashStep;

	for (i = 0; i < totalStr.length; i++) {
		a = rankCharToAction(totalStr[i]);
		if (!isNaN(a)) {
			drawActionToCanvas(_rankCtx, _rankfontAct, _rankfontSpr, a, x, RANK_Y + 6);
			x += step;
		}
	}
}

function clearRank() {
	if (_rankCtx) {
		_rankCtx.clearRect(0, 0, RANK_W, RANK_H);
	}
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(PvPCount);
