/**
 * UI/Components/MakeItemSelection/MakeItemSelection.js
 *
 * MakeItemSelection windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import 'UI/Elements/Elements.js';
import htmlText from './MakeItemSelection.html?raw';
import cssText from './MakeItemSelection.css?raw';

/**
 * Create MakeItemSelection namespace
 */
const MakeItemSelection = new GUIComponent('MakeItemSelection', cssText);

MakeItemSelection.render = () => htmlText;

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

const validMultipleMaterials = [
	1000 //star crumb
];

const validSingleMaterials = [
	997, //great nature
	996, //rough wind
	995, //mystic frozen
	994 //flame heart
];

/**
 * Track current ok/dblclick handlers so we can replace them
 */
let _okHandler = null;
let _dblClickHandler = null;

/**
 * Initialize UI
 */
MakeItemSelection.init = function init() {
	const root = MakeItemSelection.getRoot();

	this.list = root.querySelector('.list');
	this.index = 0;
	this.mkType = 0;
	this.material = [];

	this.draggable(root.querySelector('.head'));

	// Click Events
	root.querySelector('ui-button.cancel').addEventListener('click', () => {
		MakeItemSelection.index = -1;
		MakeItemSelection.selectIndex();
	});

	// Bind mousedown on items
	root.querySelector('#MakeItemSelection').addEventListener('mousedown', e => {
		const item = e.target.closest('.item');
		if (item) {
			MakeItemSelection.setIndex(Math.floor(item.getAttribute('data-index')));
		}
	});

	// on drop item
	const materials = root.querySelector('.materials');
	materials.addEventListener('drop', onDrop);
	materials.addEventListener('dragover', stopPropagation);

	root.querySelectorAll('.materials .item').forEach(el => el.remove());
	materials.style.display = 'none';
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
MakeItemSelection.setList = function setList(list) {
	const root = MakeItemSelection.getRoot();
	const listEl = root.querySelector('.list');
	listEl.innerHTML = '';
	listEl.style.backgroundColor = '#f7f7f7';

	const materials = root.querySelector('.materials');
	materials.style.display = 'none';
	root.querySelectorAll('.materials .item').forEach(el => el.remove());

	let showMaterials = true;
	this.mkType = 0;
	this.material = [];

	for (let i = 0, count = list.length; i < count; ++i) {
		const item = list[i];
		const it = DB.getItemInfo(item.ITID);
		const file = it.identifiedResourceName;
		const name = it.identifiedDisplayName;

		if (it.processitemlist === '') {
			showMaterials = false;
		}

		addElement(DB.INTERFACE_PATH + 'item/' + file + '.bmp', list[i].ITID, name);
	}

	this.setIndex(list[0].ITID);
	bindSelectEvents(showMaterials);
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
MakeItemSelection.setCookingList = function setCookingList(list, mkType) {
	const root = MakeItemSelection.getRoot();
	const listEl = root.querySelector('.list');
	listEl.innerHTML = '';
	listEl.style.backgroundColor = '#f7f7f7';

	const materials = root.querySelector('.materials');
	materials.style.display = 'none';
	root.querySelectorAll('.materials .item').forEach(el => el.remove());

	this.mkType = mkType;

	for (let i = 0, count = list.length; i < count; ++i) {
		const item = list[i];
		const it = DB.getItemInfo(item);
		const file = it.identifiedResourceName;
		const name = it.identifiedDisplayName;

		addElement(DB.INTERFACE_PATH + 'item/' + file + '.bmp', list[i], name);
	}

	this.setIndex(list[0].ITID);
	bindSelectEvents(false);
};

/**
 * Add an element to the list
 *
 * @param {string} image url
 * @param {number} index in list
 * @param {string} element name
 */
function addElement(url, index, name) {
	const root = MakeItemSelection.getRoot();
	const listEl = root.querySelector('.list');

	const div = document.createElement('div');
	div.className = 'item';
	div.setAttribute('data-index', index);
	div.innerHTML = '<div class="icon"></div>' + `<span class="name">${_sanitizeHtml(name)}</span>`;
	listEl.appendChild(div);

	Client.loadFile(url, data => {
		const icon = root.querySelector(`.list div[data-index="${index}"] .icon`);
		if (icon) {
			icon.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Advances to the next screen of the item creation
 *
 * @param {number} index in list
 */
MakeItemSelection.advance = function advance() {
	const root = MakeItemSelection.getRoot();
	const listEl = root.querySelector('.list');
	listEl.innerHTML = '';

	const it = DB.getItemInfo(this.index);
	const title = `${it.identifiedDisplayName} ${DB.getMessage(426)}`;
	const metal = it.processitemlist;
	MakeItemSelection.setTitle(title);

	const okBtn = root.querySelector('ui-button.ok');
	if (_okHandler) {
		okBtn.removeEventListener('click', _okHandler);
	}
	_okHandler = () => MakeItemSelection.selectIndex();
	okBtn.addEventListener('click', _okHandler);

	listEl.style.backgroundColor = '#ffffff';
	listEl.innerHTML = `<pre>${_sanitizeHtml(it.identifiedDisplayName)} - ${DB.getMessage(427)}\n${_sanitizeHtml(metal)}</pre>`;

	root.querySelector('.materials').style.display = 'block';
};

/**
 * Change selection
 *
 * @param {number} id in list
 */
MakeItemSelection.setIndex = function setIndex(id) {
	const root = MakeItemSelection.getRoot();
	id = id === 0 ? this.index : id;
	const prev = root.querySelector(`.list div[data-index="${this.index}"]`);
	if (prev) {
		prev.classList.remove('select');
	}
	const next = root.querySelector(`.list div[data-index="${id}"]`);
	if (next) {
		next.classList.add('select');
	}
	this.index = id;
};

/**
 * Select a server, callback
 */
MakeItemSelection.selectIndex = function selectIndex() {
	this.onIndexSelected(this.index, this.material, this.mkType);
	if (this.index == -1) {
		this.material.forEach(item => Inventory.getUI().addItem(item));
	}
	this.remove();
};

/**
 * Free variables once removed from HTML
 */
MakeItemSelection.onRemove = function onRemove() {
	this.index = 0;
};

MakeItemSelection.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		this.remove();
	}
};

/**
 * Set new window name
 *
 * @param {string} title
 */
MakeItemSelection.setTitle = function setTitle(title) {
	const root = MakeItemSelection.getRoot();
	const text = root.querySelector('.head .text');
	if (text) {
		text.textContent = title;
	}
};

/**
 * Functions to define
 */
MakeItemSelection.onIndexSelected = function onIndexSelected() {};

/**
 * Insert material to creation
 *
 * @param {object} Item
 */
MakeItemSelection.addMaterial = function AddMaterial(item, from) {
	let singleMatUsed = false;
	this.material.forEach(it => {
		if (validSingleMaterials.includes(it.ITID)) {
			singleMatUsed = true;
		}
	});

	if (
		this.material.length < 3 &&
		(validMultipleMaterials.includes(item.ITID) || (validSingleMaterials.includes(item.ITID) && !singleMatUsed))
	) {
		if (this.addItemSub(item)) {
			switch (from) {
				case 'Inventory':
					Inventory.getUI().removeItem(item.index, 1);
					break;
			}
			this.material.push(item);
		}
	}
};

/**
 * Add item to inventory
 *
 * @param {object} Item
 */
MakeItemSelection.addItemSub = function AddItemSub(item) {
	const root = MakeItemSelection.getRoot();
	const it = DB.getItemInfo(item.ITID);
	const content = root.querySelector('.materials');

	const div = document.createElement('div');
	div.className = 'item';
	div.setAttribute('data-index', item.index);
	div.setAttribute('draggable', 'false');
	div.innerHTML = '<div class="icon"></div>';
	content.appendChild(div);

	Client.loadFile(
		DB.INTERFACE_PATH +
			'item/' +
			(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
			'.bmp',
		data => {
			const icon = root.querySelector(`.materials .item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);

	return true;
};

/**
 * Drop an item from storage to inventory
 *
 * @param {event}
 */
function onDrop(event) {
	event.preventDefault();
	event.stopImmediatePropagation();

	let data;

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		// Ignore invalid JSON data
	}

	if (!data || data.type !== 'item' || data.from !== 'Inventory') {
		return false;
	}

	const item = data.data;
	item.count = 1;

	MakeItemSelection.addMaterial(item, data.from);
	return false;
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.preventDefault();
	event.stopImmediatePropagation();
}

function bindSelectEvents(showMaterials) {
	const root = MakeItemSelection.getRoot();
	const okBtn = root.querySelector('ui-button.ok');
	const mainEl = root.querySelector('#MakeItemSelection');

	// Remove old handlers
	if (_okHandler) {
		okBtn.removeEventListener('click', _okHandler);
	}
	if (_dblClickHandler) {
		mainEl.removeEventListener('dblclick', _dblClickHandler);
	}

	if (showMaterials) {
		_okHandler = () => MakeItemSelection.advance();
		_dblClickHandler = e => {
			const item = e.target.closest('.item');
			if (item) {
				MakeItemSelection.advance();
			}
		};
	} else {
		_okHandler = () => MakeItemSelection.selectIndex();
		_dblClickHandler = e => {
			const item = e.target.closest('.item');
			if (item) {
				MakeItemSelection.selectIndex();
			}
		};
	}

	okBtn.addEventListener('click', _okHandler);
	mainEl.addEventListener('dblclick', _dblClickHandler);
}

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(MakeItemSelection);
