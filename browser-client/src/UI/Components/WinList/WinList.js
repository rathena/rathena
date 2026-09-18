/**
 * UI/Components/WinList/WinList.js
 *
 * WinList windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './WinList.html?raw';
import cssText from './WinList.css?raw';

/**
 * Create WinList namespace
 */
const WinList = new GUIComponent('WinList', cssText);

WinList.render = () => htmlText;

/**
 * Initialize UI
 */
WinList.init = function init() {
	this._host.style.top = `${(Renderer.height - 280) / 1.5}px`;
	this._host.style.left = `${(Renderer.width - 280) / 2}px`;
	this.draggable();

	const root = this.getRoot();
	this._listEl = root.querySelector('.list');
	this.list = null;
	this.index = 0;

	const okBtn = root.querySelector('.ok');
	const cancelBtn = root.querySelector('.cancel');
	if (okBtn) {
		okBtn.addEventListener('click', () => WinList.selectIndex());
	}
	if (cancelBtn) {
		cancelBtn.addEventListener('click', () => WinList.exit());
	}
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
WinList.setList = function setList(list) {
	this.list = list;
	this._listEl.innerHTML = '';

	for (let i = 0, count = list.length; i < count; ++i) {
		const node = document.createElement('div');
		node.classList.add('menu_node');
		node.textContent = list[i];
		node.dataset.id = i;
		node.addEventListener('mousedown', event => {
			WinList.setIndex(parseInt(node.dataset.id, 10));
			event.stopImmediatePropagation();
		});
		node.addEventListener('dblclick', () => WinList.selectIndex());
		this._listEl.appendChild(node);
	}

	this.setIndex(0);
};

/**
 *  Cancel window
 */
WinList.exit = function exit() {
	WinList.onExitRequest();
};

/**
 * Callback to use
 */
WinList.onExitRequest = function onExitRequest() {};
WinList.onIndexSelected = function onIndexSelected() {};

/**
 * Change selection
 *
 * @param {number} id in list
 */
WinList.setIndex = function setIndex(id) {
	if (id > -1 && id < this.list.length) {
		const nodes = this._listEl.querySelectorAll('.menu_node');
		if (nodes[this.index]) {
			nodes[this.index].style.backgroundColor = 'transparent';
		}
		if (nodes[id]) {
			nodes[id].style.backgroundColor = '#cde0ff';
		}
		this.index = id;
	}
};

/**
 * Select a server, callback
 */
WinList.selectIndex = function selectIndex() {
	this.onIndexSelected(this.index);
};

/**
 * Key Management
 *
 * @param {object} event
 */
WinList.onKeyDown = function onKeyDown(event) {
	if (this._host.style.display === 'none') {
		return true;
	}
	switch (event.which) {
		default:
			return;
		case KEYS.ENTER:
			this.selectIndex();
			break;
		case KEYS.ESCAPE:
			this.exit();
			break;
		case KEYS.UP:
			this.setIndex(this.index - 1);
			break;
		case KEYS.DOWN:
			this.setIndex(this.index + 1);
			break;
	}
	event.stopImmediatePropagation();
};

/**
 * Free variables once removed from HTML
 */
WinList.onRemove = function onRemove() {
	this._listEl.innerHTML = '';
	this.list = null;
	this.index = 0;
};

WinList.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(WinList);
