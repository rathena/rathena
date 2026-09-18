/**
 * UI/Components/MakeItemSelection/ItemListWindowSelection.js
 *
 * ItemListWindowSelection windows
 *
 * @author Francisco Wallison
 */

import DB from 'DB/DBManager.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Preferences from 'Core/Preferences.js';
import ConvertItems from 'UI/Components/MakeItemSelection/ItemConvertSelection/ConvertItems.js';
import Mouse from 'Controls/MouseEventHandler.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import 'UI/Elements/Elements.js';
import htmlText from './ItemListWindowSelection.html?raw';
import cssText from './ItemListWindowSelection.css?raw';

/**
 * @var {Preference} structure to save
 */
const _preferences = Preferences.get(
	'ItemListWindowSelection',
	{
		x: 200,
		y: 500,
		height: 8,
		select_all: false
	},
	1.0
);

/**
 * Create ItemListWindowSelection namespace
 */
const ItemListWindowSelection = new GUIComponent('ItemListWindowSelection', cssText);

ItemListWindowSelection.render = () => htmlText;

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
 * Store Convert Items items
 */
ItemListWindowSelection.list = [];

/**
 * Initialize UI
 */
ItemListWindowSelection.init = function init() {
	const root = ItemListWindowSelection.getRoot();

	// Show at center.
	this._host.style.top = `${(Renderer.height - 200) / 2}px`;
	this._host.style.left = `${(Renderer.width - 655) / 2}px`;

	this.list = [];

	this.draggable(root.querySelector('.head'));
	root.querySelector('.footer .extend').addEventListener('mousedown', onResize);
	root.querySelector('.event_selectall').addEventListener('mousedown', onToggleSelectAmount);

	resizeHeight(_preferences.height);

	const mainEl = root.querySelector('#ItemListWindowSelection');
	mainEl.addEventListener('drop', onDrop);
	mainEl.addEventListener('dragover', stopPropagation);

	const content = root.querySelector('.container .content');
	content.addEventListener('wheel', onScroll);
	content.addEventListener('mouseover', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOver.call(item);
		}
	});
	content.addEventListener('mouseout', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOut();
		}
	});
	content.addEventListener('dragstart', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragStart.call(item, e);
		}
	});
	content.addEventListener('dragend', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragEnd();
		}
	});
	content.addEventListener('contextmenu', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemInfo.call(item, e);
		}
	});

	this.draggable(root.querySelector('.titlebar'));

	this.setList(Inventory.getUI().list);
};

/**
 * Apply preferences once append to body
 */
ItemListWindowSelection.onAppend = function OnAppend() {
	this.setList(Inventory.getUI().list);
	ConvertItems.append();
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
ItemListWindowSelection.setList = function setList(listItems) {
	const root = ItemListWindowSelection.getRoot();
	root.querySelector('.container .content').innerHTML = '';
	this.list = listItems;

	for (let i = 0, count = listItems.length; i < count; ++i) {
		this.addItem(listItems[i]);
	}
};

ItemListWindowSelection.addItem = function addItem(item) {
	const root = ItemListWindowSelection.getRoot();
	const it = DB.getItemInfo(item.ITID);
	const content = root.querySelector('.container .content');

	const div = document.createElement('div');
	div.className = 'item';
	div.setAttribute('data-index', item.index);
	div.setAttribute('draggable', 'true');
	div.innerHTML =
		'<div class="icon"></div>' +
		'<div class="amount">' +
		(item.count ? `<span class="count">${item.count}</span> ` : '') +
		'</div>' +
		`<span class="name">${_sanitizeHtml(DB.getItemName(item))}</span>`;
	content.appendChild(div);

	Client.loadFile(
		DB.INTERFACE_PATH +
			'item/' +
			(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
			'.bmp',
		data => {
			const icon = root.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);
};

/**
 * Add elements to the list
 *
 * @param {Array} list object to display
 */
ItemListWindowSelection.updateList = function UpdateList(item) {
	const root = ItemListWindowSelection.getRoot();
	this.list.push(item);
	root.querySelectorAll('.item').forEach(el => el.remove());

	for (let i = 0, count = this.list.length; i < count; ++i) {
		this.addItem(this.list[i]);
	}
};

/**
 * Extend ItemListWindowSelection window size
 */
function onResize() {
	const top = parseInt(ItemListWindowSelection._host.style.top, 10) || 0;
	let lastHeight = 0;

	function resizing() {
		const extraY = 31 + 19 - 30;
		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);
		h = Math.min(Math.max(h, 8), 17);

		if (h === lastHeight) {
			return;
		}

		resizeHeight(h);
		lastHeight = h;
	}

	const _Interval = setInterval(resizing, 30);

	function onMouseUp(event) {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	}
	window.addEventListener('mouseup', onMouseUp);
}

/**
 * Extend ItemListWindowSelection window size
 */
function resizeHeight(height) {
	const root = ItemListWindowSelection.getRoot();
	height = Math.min(Math.max(height, 8), 17);

	const content = root.querySelector('.container .content');
	if (content) {
		content.style.height = `${height * 32}px`;
	}
	ItemListWindowSelection._host.style.height = `${31 + 19 + height * 32}px`;
}

/**
 * Set new window name
 *
 * @param {string} title
 */
ItemListWindowSelection.setTitle = function setTitle(title) {
	const root = ItemListWindowSelection.getRoot();
	const text = root.querySelector('.head .text');
	if (text) {
		text.textContent = title;
	}
};

/**
 * Insert material to creation
 *
 * @param {object} Item
 */
ItemListWindowSelection.addReturnMaterial = function AddReturnMaterial(item) {
	const object = getItemIndexById(item.index);

	if (object < 0) {
		this.updateList(item);
	} else {
		this.updateItem(item.index, item.count);
	}
};

/**
 * Remove item from Storage
 *
 * @param {number} index in Storage
 */
ItemListWindowSelection.removeItem = function removeItem(index, count) {
	const root = ItemListWindowSelection.getRoot();
	const i = getItemIndexById(index);

	if (i < 0) {
		return null;
	}

	if (this.list[i].count) {
		this.list[i].count -= count;

		if (this.list[i].count > 0) {
			const countEl = root.querySelector(`.item[data-index="${index}"] .count`);
			if (countEl) {
				countEl.textContent = this.list[i].count;
			}
			return this.list[i];
		}
	}

	const item = this.list[i];
	this.list.splice(i, 1);
	const el = root.querySelector(`.item[data-index="${index}"]`);
	if (el) {
		el.remove();
	}

	return item;
};

/**
 * Remove item from inventory
 *
 * @param {number} index in inventory
 * @param {number} count
 */
ItemListWindowSelection.updateItem = function UpdateItem(index, count) {
	const root = ItemListWindowSelection.getRoot();
	const item = this.getItemByIndex(index);

	if (!item) {
		return;
	}

	item.count = item.count + count;

	if (item.count > 0) {
		const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
		if (countEl) {
			countEl.textContent = item.count;
		}
		return;
	}

	this.list.splice(this.list.indexOf(item), 1);
	const el = root.querySelector(`.item[data-index="${item.index}"]`);
	if (el) {
		el.remove();
	}
};

/**
 * Search in a list for an item by its index
 *
 * @param {number} index
 * @returns {Item}
 */
ItemListWindowSelection.getItemByIndex = function getItemByIndex(index) {
	const list = this.list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].index === index) {
			return list[i];
		}
	}

	return null;
};

ItemListWindowSelection.getSelectAll = function getSelectAll() {
	return _preferences.select_all;
};

/**
 * Mouse mouve out of an item, hide title description
 */
function onItemOut() {
	const root = ItemListWindowSelection.getRoot();
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const i = getItemIndexById(index);

	if (i === -1) {
		return;
	}

	const img = new Image();
	let url = this.firstChild.style.backgroundImage.match(/\(([^)]+)/)[1];
	url = url.replace(/^"/, '').replace(/"$/, '');
	img.decoding = 'async';
	img.src = url;

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'ItemListWindowSelection',
				data: ItemListWindowSelection.list[i]
			})
		)
	);

	onItemOut();
}

/**
 * Option to automatically buy/sell alls items instead of specify the amount
 */
function onToggleSelectAmount() {
	const root = ItemListWindowSelection.getRoot();
	_preferences.select_all = !_preferences.select_all;

	Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (_preferences.select_all ? 1 : 0) + '.bmp', data => {
		const btn = root.querySelector('.selectall');
		if (btn) {
			btn.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Display item description
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();

	const index = parseInt(this.getAttribute('data-index'), 10);
	const i = getItemIndexById(index);

	if (i === -1) {
		return false;
	}

	if (ItemInfo.uid === ItemListWindowSelection.list[i].ITID) {
		ItemInfo.remove();
	}

	ItemInfo.append();
	ItemInfo.uid = ItemListWindowSelection.list[i].ITID;
	ItemInfo.setItem(ItemListWindowSelection.list[i]);

	return false;
}

/**
 * Stop the drag/drop on an item
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

/**
 * Get item index in list base on it's ID
 *
 * @param {number} item id
 */
function getItemIndexById(index) {
	for (let i = 0, count = ItemListWindowSelection.list.length; i < count; ++i) {
		if (ItemListWindowSelection.list[i].index === index) {
			return i;
		}
	}

	return -1;
}

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
		// Ignore
	}

	if (!data || data.type !== 'item' || data.from !== 'ConvertItems') {
		return false;
	}

	const item = data.data;
	const valid_select_all = !ItemListWindowSelection.getSelectAll();

	if (item.count > 1 && valid_select_all) {
		InputBox.append();
		InputBox.setType('number', false, item.count);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			InputBox.remove();

			ConvertItems.removeItem(item.index, parseInt(count, 10));

			item.count = parseInt(count, 10);
			ItemListWindowSelection.addReturnMaterial(item);
		};
		return false;
	}

	ConvertItems.removeItem(item.index, item.count);
	ItemListWindowSelection.addReturnMaterial(item);
	return false;
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.preventDefault();
	event.stopImmediatePropagation();
}

/**
 * Update scroll by block (32px)
 */
function onScroll(event) {
	let delta;

	if (event.wheelDelta) {
		delta = event.wheelDelta / 120;
		if (window.opera) {
			delta = -delta;
		}
	} else if (event.detail) {
		delta = -event.detail;
	}

	this.scrollTop = Math.floor(this.scrollTop / 32) * 32 - delta * 32;
	return false;
}

/**
 * Mouse over item, display name and informations
 */
function onItemOver() {
	const root = ItemListWindowSelection.getRoot();
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const i = getItemIndexById(idx);

	if (i < 0) {
		return;
	}

	const item = ItemListWindowSelection.list[i];
	const rect = this.getBoundingClientRect();
	const hostRect = ItemListWindowSelection._host.getBoundingClientRect();
	const overlay = root.querySelector('.overlay');

	overlay.style.display = 'block';
	overlay.style.top = `${rect.top - hostRect.top - 10}px`;
	overlay.style.left = `${rect.left - hostRect.left + 35}px`;
	overlay.textContent = `${DB.getItemName(item)} ${item.count || 1} ea`;

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	} else {
		overlay.classList.add('grey');
	}
}

ItemListWindowSelection.onItemListWindowSelected = function onItemListWindowSelected() {};

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(ItemListWindowSelection);
