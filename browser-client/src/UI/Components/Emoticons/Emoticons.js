/**
 * UI/Components/Emoticons/Emoticons.js
 *
 * Emoticons Interface
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import EmoticonsDB from 'DB/Emotions.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import htmlText from './Emoticons.html?raw';
import cssText from './Emoticons.css?raw';
import ShortCuts from 'UI/Components/ShortCuts/ShortCuts.js';

/**
 * Create Component
 */
const Emoticons = new GUIComponent('Emoticons', cssText);

/**
 * Render HTML
 */
Emoticons.render = () => htmlText;

/**
 * @var {number} page index
 */
let _page = 0;

/**
 * @var {number} emoticons per page
 */
const EMOTICONS_PER_PAGE = 30;

/**
 * @var {number} total pages
 */
let TOTAL_PAGES = 0;

/**
 * @var {number} emoticons count
 */
const EMOTICONS_COUNT = Object.keys(EmoticonsDB.order).length;

/**
 * @var {object} emoticons action file
 */
let _action;

/**
 * @var {object} emoticons sprite
 */
let _sprite;

/**
 * @var {Entity} Helper to render sprites
 */
const _entity = new Entity();

/**
 * @var {Preference} structure to save
 */
const _preferences = Preferences.get(
	'Emoticons',
	{
		x: 600,
		y: 200,
		show: false
	},
	1.0
);

/**
 * Initialize UI
 */
Emoticons.init = function init() {
	const root = this.getRoot();
	const contentEl = root.querySelector('.content');

	Client.loadFiles(
		['data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/emotion.act', 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/emotion.spr'],
		(act, spr) => {
			_action = act;
			_sprite = spr;
			TOTAL_PAGES = Math.max(0, Math.ceil(EMOTICONS_COUNT / EMOTICONS_PER_PAGE) - 1);

			const totalEl = root.querySelector('.total');
			if (totalEl) {
				totalEl.textContent = TOTAL_PAGES + 1;
			}
			this.movePage(0);
		}
	);

	// Delegated dblclick on canvas → play emoticon
	contentEl.addEventListener('dblclick', event => {
		const canvas = event.target.closest('canvas');
		if (!canvas) return;
		onPlayEmoticon(canvas);
	});

	// Delegated mousedown on canvas → select emoticon
	contentEl.addEventListener('mousedown', event => {
		const canvas = event.target.closest('canvas');
		if (!canvas) return;
		onSelectEmoticon(canvas);
	});

	root.querySelector('.prev').classList.add('disabled');
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
	}
	const baseButtons = root.querySelectorAll('.base');
	baseButtons.forEach(btn => {
		if (btn !== closeBtn) {
			btn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		}
	});
	root.querySelector('.close').addEventListener('click', () => {
		this._host.style.display = 'none';
	});
	root.querySelector('.prev').addEventListener('click', getMovePageHandler(-1));
	root.querySelector('.next').addEventListener('click', getMovePageHandler(+1));

	this.draggable('.titlebar');
};

/**
 * Appending to html
 */
Emoticons.onAppend = function onAppend() {
	if (!_preferences.show) {
		this._host.style.display = 'none';
	}

	this._host.style.top = Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight) + 'px';
	this._host.style.left = Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth) + 'px';
};

/**
 * Once removed from DOM, save preferences
 */
Emoticons.onRemove = function onRemove() {
	_preferences.show = this._host.style.display !== 'none';
	_preferences.y = parseInt(this._host.style.top, 10) || 0;
	_preferences.x = parseInt(this._host.style.left, 10) || 0;
	_preferences.save();
};

/**
 * Update page
 *
 * @param {number} direction
 */
Emoticons.movePage = function movePage(direction) {
	const root = this.getRoot();
	const prevBtn = root.querySelector('.prev');
	const nextBtn = root.querySelector('.next');

	prevBtn.classList.remove('disabled');
	nextBtn.classList.remove('disabled');

	_page += direction;

	if (_page <= 0) {
		prevBtn.classList.add('disabled');
		_page = 0;
	}

	if (_page >= TOTAL_PAGES) {
		nextBtn.classList.add('disabled');
		_page = TOTAL_PAGES;
	}

	const currentEl = root.querySelector('.current');
	if (currentEl) {
		currentEl.textContent = _page + 1;
	}

	refreshList(root.querySelector('.content'));
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
Emoticons.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			if (this._host.style.display === 'none') {
				this.ui.show();
				this.focus();
			} else {
				this._host.style.display = 'none';
			}
			break;
	}
};

/**
 * Generic move page function
 *
 * @param {number} index
 * @return {function}
 */
function getMovePageHandler(index) {
	return function movePageClosure() {
		if (!this.classList.contains('disabled')) {
			Emoticons.movePage(index);
		}
	};
}

/**
 * Select an emoticon
 * Display the command shortcut in the ShortCuts
 *
 * @param {HTMLCanvasElement} canvas
 */
function onSelectEmoticon(canvas) {
	const idx = canvas.getAttribute('data-index');
	const cmd = EmoticonsDB.names[idx];
	if (cmd && ShortCuts.ui.is(':visible')) {
		if (ShortCuts.ui.find('.input_macro_focus').length) {
			ShortCuts.ui
				.find('.input_macro_focus')
				.val('/' + cmd)
				.select();
			return;
		}
	}

	if (cmd) {
		ChatBox.ui
			.find('.input .message')
			.html('/' + cmd)
			.focus();
	}
}

/**
 * Do an emoticon
 *
 * @param {HTMLCanvasElement} canvas
 */
function onPlayEmoticon(canvas) {
	const idx = canvas.getAttribute('data-index');
	const cmd = EmoticonsDB.names[idx];

	ChatBox.ui.find('.input .message').html('/' + cmd);
	ChatBox.submit();
}

/**
 * Refresh emoticons list
 *
 * @param {HTMLElement} contentEl
 */
function refreshList(contentEl) {
	let index = EMOTICONS_PER_PAGE * _page;
	const end = Math.min(EMOTICONS_COUNT, index + EMOTICONS_PER_PAGE);
	const pos = [0, 0];

	contentEl.innerHTML = '';

	while (index < end) {
		const canvas = document.createElement('canvas');
		const ctx = canvas.getContext('2d');
		canvas.width = 40;
		canvas.height = 40;
		const emo = EmoticonsDB.order[index];

		canvas.setAttribute('data-index', emo);
		canvas.title = '/' + EmoticonsDB.names[emo];
		const animations = _action.actions[emo].animations;

		// Do not ask why, but we don't know how Gravity find
		// the animation to render:
		const animation = animations[Math.floor(animations.length / 5)];
		const layers = animation.layers;
		const count = layers.length;

		SpriteRenderer.bind2DContext(ctx, 20 - layers[0].pos[0], 40 - layers[0].pos[1]);

		for (let i = 0; i < count; ++i) {
			_entity.renderLayer(layers[i], _sprite, _sprite, 1.0, pos, false);
		}

		contentEl.appendChild(canvas);
		index++;
	}
}

Emoticons.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Export
 */
export default UIManager.addComponent(Emoticons);
