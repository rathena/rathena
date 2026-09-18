/**
 * UI/Components/RefineWeaponSelection/RefineWeaponSelection.js
 *
 * RefineWeaponSelection windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './RefineWeaponSelection.html?raw';
import cssText from './RefineWeaponSelection.css?raw';

/**
 * Create RefineWeaponSelection namespace
 */
const RefineWeaponSelection = new GUIComponent('RefineWeaponSelection', cssText);

RefineWeaponSelection.render = () => htmlText;

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
RefineWeaponSelection.init = function init() {
	const root = RefineWeaponSelection.getRoot();

	this.list = root.querySelector('.list');
	this.index = 0;

	this.draggable(root.querySelector('.head'));

	// Click Events
	root.querySelector('ui-button.ok').addEventListener('click', () => {
		RefineWeaponSelection.selectIndex();
	});
	root.querySelector('ui-button.cancel').addEventListener('click', () => {
		RefineWeaponSelection.index = -1;
		RefineWeaponSelection.selectIndex();
	});

	// Bind events
	root.querySelector('#RefineWeaponSelection').addEventListener('dblclick', e => {
		const item = e.target.closest('.item');
		if (item) {
			RefineWeaponSelection.selectIndex();
		}
	});
	root.querySelector('#RefineWeaponSelection').addEventListener('mousedown', e => {
		const item = e.target.closest('.item');
		if (item) {
			RefineWeaponSelection.setIndex(Math.floor(item.getAttribute('data-index')));
		}
	});
};

/**
 * Once append to body
 */
RefineWeaponSelection.onAppend = function onAppend() {
	this._host.style.top = `${(Renderer.height - 200) / 2}px`;
	this._host.style.left = `${(Renderer.width - 200) / 2}px`;
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
RefineWeaponSelection.setList = function setList(list) {
	const root = RefineWeaponSelection.getRoot();
	const listEl = root.querySelector('.list');
	listEl.innerHTML = '';
	RefineWeaponSelection.ItemList = [];

	for (let i = 0, count = list.length; i < count; ++i) {
		RefineWeaponSelection.ItemList[i] = list[i];

		const it = DB.getItemInfo(list[i].ITID);
		const file = it.identifiedResourceName;
		const refine = list[i].RefiningLevel;
		const name = refine > 0 ? `+${refine} ${it.identifiedDisplayName}` : it.identifiedDisplayName;

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
	const root = RefineWeaponSelection.getRoot();
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
RefineWeaponSelection.setIndex = function setIndex(id) {
	const root = RefineWeaponSelection.getRoot();
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
RefineWeaponSelection.selectIndex = function selectIndex() {
	this.onIndexSelected(this.index);
	this.remove();
};

/**
 * Select a server, callback
 */
RefineWeaponSelection.getItemByIndex = function getItemByIndex(index) {
	const list = RefineWeaponSelection.ItemList;

	for (let i = 0; i < list.length; i++) {
		if (RefineWeaponSelection.ItemList[i].index == index) {
			return RefineWeaponSelection.ItemList[i];
		}
	}
};

/**
 * Free variables once removed from HTML
 */
RefineWeaponSelection.onRemove = function onRemove() {
	this.index = 0;
};

/**
 * Set new window name
 *
 * @param {string} title
 */
RefineWeaponSelection.setTitle = function setTitle(title) {
	const root = RefineWeaponSelection.getRoot();
	const text = root.querySelector('.head .text');
	if (text) {
		text.textContent = title;
	}
};

/**
 * Functions to define
 */
RefineWeaponSelection.onIndexSelected = function onIndexSelected() {};

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(RefineWeaponSelection);
