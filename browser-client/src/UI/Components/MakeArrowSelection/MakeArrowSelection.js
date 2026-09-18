/**
 * UI/Components/MakeArrowSelection/MakeArrowSelection.js
 *
 * MakeArrowSelection windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './MakeArrowSelection.html?raw';
import cssText from './MakeArrowSelection.css?raw';

/**
 * Create MakeArrowSelection namespace
 */
const MakeArrowSelection = new GUIComponent('MakeArrowSelection', cssText);

MakeArrowSelection.render = () => htmlText;

/**
 * Sanitize HTML, allowing only whitelisted tags (font, i, b)
 */
function _sanitizeHtml(str) {
	const whitelist = ['font', 'i', 'b'];
	const div = document.createElement('div');
	div.innerHTML = str;
	div.querySelectorAll('*').forEach(el => {
		if (!whitelist.includes(el.tagName.toLowerCase())) {
			el.replaceWith(...el.childNodes);
		}
	});
	return div.innerHTML;
}

/**
 * Initialize UI
 */
MakeArrowSelection.init = function init() {
	const root = MakeArrowSelection.getRoot();

	this.list = root.querySelector('.list');
	this.index = 0;

	this.draggable(root.querySelector('.head'));

	// Click Events
	root.querySelector('ui-button.ok').addEventListener('click', () => {
		MakeArrowSelection.selectIndex();
	});
	root.querySelector('ui-button.cancel').addEventListener('click', () => {
		MakeArrowSelection.index = -1;
		MakeArrowSelection.selectIndex();
	});

	// Bind events
	root.querySelector('#MakeArrowSelection').addEventListener('dblclick', e => {
		const item = e.target.closest('.item');
		if (item) {
			MakeArrowSelection.selectIndex();
		}
	});
	root.querySelector('#MakeArrowSelection').addEventListener('mousedown', e => {
		const item = e.target.closest('.item');
		if (item) {
			MakeArrowSelection.setIndex(Math.floor(item.getAttribute('data-index')));
		}
	});
};

/**
 * Once append to body
 */
MakeArrowSelection.onAppend = function onAppend() {
	this._host.style.top = `${(Renderer.height - 200) / 2}px`;
	this._host.style.left = `${(Renderer.width - 200) / 2}px`;
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
MakeArrowSelection.setList = function setList(list) {
	const root = MakeArrowSelection.getRoot();
	const listEl = root.querySelector('.list');
	listEl.innerHTML = '';

	for (let i = 0, count = list.length; i < count; ++i) {
		const item = list[i];
		const it = DB.getItemInfo(item.index);
		const file = it.identifiedResourceName;
		const name = it.identifiedDisplayName;

		addElement(DB.INTERFACE_PATH + 'item/' + file + '.bmp', list[i].index, name);
	}

	this.setIndex(list[0].index);
};

/**
 * Add an element to the list
 *
 * @param {string} image url
 * @param {number} index in list
 * @param {string} element name
 */
function addElement(url, index, name) {
	const root = MakeArrowSelection.getRoot();
	const listEl = root.querySelector('.list');

	const div = document.createElement('div');
	div.className = 'item';
	div.setAttribute('data-index', index);
	div.innerHTML = '<div class="icon"></div>' + `<span class="name">${_sanitizeHtml(name)}</span>`;
	listEl.appendChild(div);

	Client.loadFile(url, data => {
		const icon = root.querySelector(`div[data-index="${index}"] .icon`);
		if (icon) {
			icon.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Change selection
 *
 * @param {number} id in list
 */
MakeArrowSelection.setIndex = function setIndex(id) {
	const root = MakeArrowSelection.getRoot();
	const prev = root.querySelector(`div[data-index="${this.index}"]`);
	if (prev) {
		prev.style.backgroundColor = 'transparent';
	}
	const next = root.querySelector(`div[data-index="${id}"]`);
	if (next) {
		next.style.backgroundColor = '#cde0ff';
	}
	this.index = id;
};

/**
 * Select a server, callback
 */
MakeArrowSelection.selectIndex = function selectIndex() {
	this.onIndexSelected(this.index);
	this.remove();
};

/**
 * Free variables once removed from HTML
 */
MakeArrowSelection.onRemove = function onRemove() {
	this.index = 0;
};

/**
 * Set new window name
 *
 * @param {string} title
 */
MakeArrowSelection.setTitle = function setTitle(title) {
	const root = MakeArrowSelection.getRoot();
	const text = root.querySelector('.head .text');
	if (text) {
		text.textContent = title;
	}
};

/**
 * Functions to define
 */
MakeArrowSelection.onIndexSelected = function onIndexSelected() {};

MakeArrowSelection.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		this.remove();
	}
};

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(MakeArrowSelection);
