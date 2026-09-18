/**
 * UI/Components/MakeItemSelection/ItemConvertSelection/ConvertItems.js
 *
 * ConvertItems windows
 *
 * @author Francisco Wallison
 */

import DB from 'DB/DBManager.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Preferences from 'Core/Preferences.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ItemListWindowSelection from 'UI/Components/MakeItemSelection/ItemListWindowSelection.js';
import MakeModelMessage from 'UI/Components/MakeItemSelection/ItemConvertSelection/MakeModelMessage/MakeModelMessage.js';
import 'UI/Elements/Elements.js';
import htmlText from './ConvertItems.html?raw';
import cssText from './ConvertItems.css?raw';

/**
 * @var {Preference} structure to save
 */
const _preferences = Preferences.get(
	'ConvertItems',
	{
		x: 200,
		y: 500,
		height: 8
	},
	1.0
);

/**
 * Create ConvertItems namespace
 */
const ConvertItems = new GUIComponent('ConvertItems', cssText);

ConvertItems.render = () => htmlText;

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
ConvertItems.material = [];

/**
 * Initialize UI
 */
ConvertItems.init = function init() {
	const root = ConvertItems.getRoot();

	this._host.style.top = `${(Renderer.height - 200) / 2}px`;
	this._host.style.left = `${(Renderer.width - 10) / 2}px`;

	this.material = [];

	this.draggable(root.querySelector('.head'));
	root.querySelector('.footer .extend').addEventListener('mousedown', onResize);
	root.querySelector('ui-button.trade').addEventListener('click', onMessageModel);
	root.querySelector('ui-button.cancel').addEventListener('click', onClose);

	resizeHeight(_preferences.height);

	const mainEl = root.querySelector('#ConvertItems');
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
};

/**
 * Apply preferences once append to body
 */
ConvertItems.onAppend = function OnAppend() {
	const root = ConvertItems.getRoot();
	this.material = [];
	root.querySelectorAll('.container .content .item').forEach(el => el.remove());
};

ConvertItems.addItem = function addItem(item) {
	const root = ConvertItems.getRoot();
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

	return true;
};

/**
 * Remove item from inventory
 *
 * @param {number} index in inventory
 * @param {number} count
 */
ConvertItems.updateItem = function UpdateItem(index, count) {
	const root = ConvertItems.getRoot();
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

	this.material.splice(this.material.indexOf(item), 1);
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
ConvertItems.getItemByIndex = function getItemByIndex(index) {
	const list = ConvertItems.material;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].index === index) {
			return list[i];
		}
	}

	return null;
};

ConvertItems.validItemSend = function validItemSend(valid_material) {
	ConvertItems.remove();
	ItemListWindowSelection.remove();
	MakeModelMessage.remove();

	const inforMaterialList = {
		Type: 0,
		Action: valid_material ? 1 : 0,
		MaterialList: valid_material ? this.material : []
	};

	ItemListWindowSelection.onItemListWindowSelected(inforMaterialList);
};

function onMessageModel(event) {
	event.stopImmediatePropagation();
	MakeModelMessage.append();
}

function onClose(event) {
	event.stopImmediatePropagation();
	ConvertItems.validItemSend(false);
}

/**
 * Extend ConvertItems window size
 */
function onResize() {
	const top = parseInt(ConvertItems._host.style.top, 10) || 0;
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
 * Extend ConvertItems window size
 */
function resizeHeight(height) {
	const root = ConvertItems.getRoot();
	height = Math.min(Math.max(height, 8), 17);

	const content = root.querySelector('.container .content');
	if (content) {
		content.style.height = `${height * 32}px`;
	}
	ConvertItems._host.style.height = `${31 + 19 + height * 32}px`;
}

/**
 * Free variables once removed from HTML
 */
ConvertItems.onRemove = function onRemove() {
	this.index = 0;
};

/**
 * Set new window name
 *
 * @param {string} title
 */
ConvertItems.setTitle = function setTitle(title) {
	const root = ConvertItems.getRoot();
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
ConvertItems.addMaterial = function AddMaterial(item) {
	const object = getItemIndexById(item.index);

	if (object < 0) {
		if (this.addItem(item)) {
			this.material.push(item);
		}
	} else {
		this.updateItem(item.index, item.count);
	}
};

/**
 * Remove item from Storage
 *
 * @param {number} index in Storage
 */
ConvertItems.removeItem = function removeItem(index, count) {
	const root = ConvertItems.getRoot();
	const i = getItemIndexById(index);

	if (i < 0) {
		return null;
	}

	if (ConvertItems.material[i].count) {
		ConvertItems.material[i].count -= count;

		if (ConvertItems.material[i].count > 0) {
			const countEl = root.querySelector(`.item[data-index="${index}"] .count`);
			if (countEl) {
				countEl.textContent = ConvertItems.material[i].count;
			}
			return ConvertItems.material[i];
		}
	}

	const item = ConvertItems.material[i];
	ConvertItems.material.splice(i, 1);
	const el = root.querySelector(`.item[data-index="${index}"]`);
	if (el) {
		el.remove();
	}

	return item;
};

/**
 * Mouse mouve out of an item, hide title description
 */
function onItemOut() {
	const root = ConvertItems.getRoot();
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
				from: 'ConvertItems',
				data: ConvertItems.material[i]
			})
		)
	);

	onItemOut();
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

	if (ItemInfo.uid === ConvertItems.material[i].ITID) {
		ItemInfo.remove();
	}

	ItemInfo.append();
	ItemInfo.uid = ConvertItems.material[i].ITID;
	ItemInfo.setItem(ConvertItems.material[i]);

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
	for (let i = 0, count = ConvertItems.material.length; i < count; ++i) {
		if (ConvertItems.material[i].index === index) {
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

	if (!data || data.type !== 'item' || data.from !== 'ItemListWindowSelection') {
		return false;
	}

	const item = data.data;
	const valid_select_all = !ItemListWindowSelection.getSelectAll();

	if (item.count > 1 && valid_select_all) {
		InputBox.append();
		InputBox.setType('number', false, item.count);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			InputBox.remove();

			ItemListWindowSelection.removeItem(item.index, parseInt(count, 10));
			item.count = parseInt(count, 10);
			ConvertItems.addMaterial(item);
		};
		return false;
	}

	ItemListWindowSelection.removeItem(item.index, item.count);
	ConvertItems.addMaterial(item);
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
	const root = ConvertItems.getRoot();
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const i = getItemIndexById(idx);

	if (i < 0) {
		return;
	}

	const item = ConvertItems.material[i];
	const rect = this.getBoundingClientRect();
	const hostRect = ConvertItems._host.getBoundingClientRect();
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

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(ConvertItems);
