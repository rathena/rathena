/**
 * UI/Components/VendingShop/VendingShop.js
 *
 * Character VendingShop Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import 'UI/Elements/Elements.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Vending from 'UI/Components/Vending/Vending.js';
import htmlText from './VendingShop.html?raw';
import cssText from './VendingShop.css?raw';
import VendingReport from 'UI/Components/VendingReport/VendingReport.js';

/**
 * Create Component
 */
const VendingShop = new GUIComponent('VendingShop', cssText);

VendingShop.render = () => htmlText;

/**
 * @var {enum} Store type
 */
VendingShop.Type = {
	VENDING_LIST: 0,
	BUYING_LIST: 1
};

/**
 * Store inventory items
 */
VendingShop.list = [];

/**
 * @var {number} used to remember the window height
 */
let _realSize = 0;

/**
 * @var {number} type (buy/sell)
 */
let _type;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'VendingShop',
	{
		x: 200,
		y: 200,
		width: 8,
		height: 2,
		reduce: false
	},
	1.0
);

/**
 * Initialize UI
 */
VendingShop.init = function init() {
	const root = this.getRoot();

	const closeBtn = root.querySelector('.btn.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => VendingShop.onSubmit());
	}

	this._host.addEventListener('drop', onDrop);
	this._host.addEventListener('dragover', e => {
		e.stopImmediatePropagation();
		e.preventDefault();
	});

	const content = root.querySelector('.container .content');
	if (content) {
		content.addEventListener('wheel', onScroll);
		content.addEventListener('mouseover', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemOver(itemEl, root);
			}
		});
		content.addEventListener('mouseout', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemOut(root);
			}
		});
		content.addEventListener('dragstart', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemDragStart(e);
			}
		});
		content.addEventListener('dragend', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemDragEnd();
			}
		});
		content.addEventListener('contextmenu', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemInfo(e, itemEl);
			}
		});
		content.addEventListener('dblclick', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemUsed(e, itemEl);
			}
		});
	}

	this.draggable('.titlebar');
	this._host.style.display = 'none';
};

/**
 * Apply preferences once append to body
 */
VendingShop.onAppend = function onAppend() {
	this.resize(_preferences.width, _preferences.height);

	const hostRect = this._host.getBoundingClientRect();
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostRect.height)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostRect.width)}px`;

	_realSize = _preferences.reduce ? 0 : hostRect.height;
	const messageText = DB.getMessage(226);
	const titleShop = Vending._shopname.length > 25 ? `${Vending._shopname.substring(0, 25)}...` : Vending._shopname;
	const root = this.getRoot();
	const shopnameEl = root.querySelector('.text.shopname');
	if (shopnameEl) {
		shopnameEl.textContent = `${messageText} : ${titleShop}`;
	}
};

/**
 * Specify the type of the shop
 *
 * @param {number} type (see NpcStore.Type.*)
 */
VendingShop.setType = function setType(type) {
	_type = type;
};

/**
 * Remove Inventory from window (and so clean up items)
 */
VendingShop.onRemove = function onRemove() {
	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.innerHTML = '';
	}
	this.list.length = 0;

	const itemInfoEl = document.querySelector('.ItemInfo');
	if (itemInfoEl) {
		itemInfoEl.remove();
	}

	_preferences.reduce = !!_realSize;
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.width = Math.floor((this._host.getBoundingClientRect().width - (23 + 16 + 16 - 30)) / 32);
	_preferences.height = Math.floor((this._host.getBoundingClientRect().height - (31 + 19 - 30)) / 32);
	_preferences.save();

	this._host.style.display = 'none';
};

/**
 * Extend inventory window size
 *
 * @param {number} width
 * @param {number} height
 */
VendingShop.resize = function resize(width, height) {
	width = Math.min(Math.max(width, 6), 9);
	height = Math.min(Math.max(height, 2), 6);

	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.style.width = `${width * 32 + 13}px`;
		content.style.height = `${height * 32}px`;
	}

	const innerW = 16 + 16 + width * 32;
	const innerH = 31 + 19 + height * 32;
	this._host.style.width = `${innerW}px`;
	this._host.style.height = `${innerH}px`;

	const inner = root.querySelector('#vendingshop');
	if (inner) {
		inner.style.width = `${innerW}px`;
		inner.style.height = `${innerH}px`;
	}
};

/**
 * Get item object
 *
 * @param {number} id
 * @returns {Item}
 */
VendingShop.getItemById = function getItemById(id) {
	const list = VendingShop.list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].ITID === id) {
			return list[i];
		}
	}

	return null;
};

/**
 * Search in a list for an item by its index
 *
 * @param {number} index
 * @returns {Item}
 */
VendingShop.getItemByIndex = function getItemByIndex(index) {
	const list = VendingShop.list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].index === index) {
			return list[i];
		}
	}

	return null;
};

/**
 * Add items to the list
 * if the item index is exist you should clear it;[skybook888]
 */
VendingShop.setItems = function setItems(items) {
	for (let i = 0, count = items.length; i < count; ++i) {
		if (this.addItemSub(items[i])) {
			this.list.push(items[i]);
		}
	}

	this._host.style.display = '';
};

/**
 * Insert Item to inventory
 *
 * @param {object} Item
 */
VendingShop.addItem = function addItem(item) {
	let object = this.getItemByIndex(item.index);

	if (object) {
		object.count += item.count;
		const root = this.getRoot();
		const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
		if (countEl) {
			countEl.textContent = object.count;
		}
		return;
	}

	object = Object.assign({}, item);
	if (this.addItemSub(object)) {
		this.list.push(object);
	}
};

/**
 * Add item to inventory
 *
 * @param {object} Item
 */
VendingShop.addItemSub = function addItemSub(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (!content) {
		return false;
	}

	const itemDiv = document.createElement('div');
	itemDiv.className = 'item';
	itemDiv.setAttribute('data-index', item.index);
	itemDiv.setAttribute('draggable', 'true');
	itemDiv.innerHTML =
		'<div class="icon"></div>' + `<div class="amount"><span class="count">${item.count || 1}</span></div>`;
	content.appendChild(itemDiv);

	const hideEl = root.querySelector('.hide');
	if (hideEl) {
		if (content.clientHeight < content.scrollHeight) {
			hideEl.style.display = 'none';
		} else {
			hideEl.style.display = '';
		}
	}

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const iconEl = content.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (iconEl) {
				iconEl.style.backgroundImage = `url(${data})`;
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
VendingShop.removeItem = function removeItem(index, count) {
	const item = this.getItemByIndex(index);

	if (!item || count <= 0) {
		return null;
	}

	const msg = DB.getMessage(231).replace('%s', DB.getItemName(item)).replace('%d', count);
	ChatBox.addText(msg, ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);

	const root = this.getRoot();

	if (item.count) {
		item.count -= count;

		if (item.count > 0) {
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = item.count;
			}
			return item;
		}
	}

	this.list.splice(this.list.indexOf(item), 1);
	const itemEl = root.querySelector(`.item[data-index="${item.index}"]`);
	if (itemEl) {
		itemEl.remove();
	}

	let ctr = 0;
	for (let i = 0; i < this.list.length; i++) {
		if (this.list[i].count > 0) {
			ctr++;
		}
	}

	if (ctr === 0) {
		this.onSubmit();
	}

	return item;
};

/**
 * Update item in inventory
 *
 * @param {number} index in inventory
 * @param {number} count
 */
VendingShop.updateItem = function updateItem(index, count) {
	const item = this.getItemByIndex(index);

	if (!item) {
		return;
	}

	item.count = count;

	const root = this.getRoot();

	if (item.count > 0) {
		const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
		if (countEl) {
			countEl.textContent = item.count;
		}
		return;
	}

	this.list.splice(this.list.indexOf(item), 1);
	const itemEl = root.querySelector(`.item[data-index="${item.index}"]`);
	if (itemEl) {
		itemEl.remove();
	}

	const content = root.querySelector('.container .content');
	const hideEl = root.querySelector('.hide');
	if (content && hideEl) {
		if (content.clientHeight >= content.scrollHeight) {
			hideEl.style.display = '';
		}
	}
};

/**
 * Prettify zeny : 1000000 -> 1,000,000
 *
 * @param {number} zeny
 * @param {boolean} useStyle
 * @return {string}
 */
function prettyZeny(val, useStyle) {
	const list = val.toString().split('');
	const count = list.length;
	let str = '';

	for (let i = 0; i < count; i++) {
		str = list[count - i - 1] + (i && i % 3 === 0 ? ',' : '') + str;
	}

	if (useStyle) {
		const style = [
			'color:#000000; text-shadow:1px 0px #00ffff;',
			'color:#0000ff; text-shadow:1px 0px #ce00ce;',
			'color:#0000ff; text-shadow:1px 0px #00ffff;',
			'color:#ff0000; text-shadow:1px 0px #ffff00;',
			'color:#ff18ff;',
			'color:#0000ff;',
			'color:#000000; text-shadow:1px 0px #00ff00;',
			'color:#ff0000;',
			'color:#000000; text-shadow:1px 0px #cece63;',
			'color:#ff0000; text-shadow:1px 0px #ff007b;'
		];
		str = `<span style="${style[count - 1]}">${str}</span>`;
	}

	return str;
}

/**
 * Hide/show inventory's content
 * @preserved currently unused, might be needed in the future
 */
function _onToggleReduction() {
	const host = VendingShop._host;
	const root = VendingShop.getRoot();

	if (_realSize) {
		const panel = root.querySelector('.panel');
		if (panel) {
			panel.style.display = '';
		}
		host.style.height = `${_realSize}px`;
		_realSize = 0;
	} else {
		_realSize = host.getBoundingClientRect().height;
		host.style.height = '17px';
		const panel = root.querySelector('.panel');
		if (panel) {
			panel.style.display = 'none';
		}
	}
}

/**
 * Update tab, reset inventory content
 * @preserved currently unused, might be needed in the future
 */
function _requestFilter() {
	const root = VendingShop.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.innerHTML = '';
	}

	const list = VendingShop.list;
	for (let i = 0, count = list.length; i < count; ++i) {
		VendingShop.addItemSub(list[i]);
	}
}

/**
 * Drop an item from storage to inventory
 *
 * @param {Event} event
 */
function onDrop(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Block the scroll to move 32px at each move
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
	} else if (event.deltaY) {
		delta = -event.deltaY / 100;
	}

	event.currentTarget.scrollTop = Math.floor(event.currentTarget.scrollTop / 32) * 32 - delta * 32;
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Show item name when mouse is over
 */
function onItemOver(itemEl, root) {
	const idx = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = VendingShop.getItemByIndex(idx);

	if (!item) {
		return;
	}

	const overlay = root.querySelector('.overlay');
	if (!overlay) {
		return;
	}

	overlay.style.display = '';
	overlay.style.top = `${itemEl.offsetTop}px`;
	overlay.style.left = `${itemEl.offsetLeft + 35}px`;
	overlay.textContent = `${DB.getItemName(item)} ${prettyZeny(item.price, false)} ${DB.getMessage(2328)}`;

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	} else if (_type === VendingShop.Type.VENDING_LIST) {
		overlay.classList.add('grey');
	}
}

/**
 * Hide the item name
 */
function onItemOut(root) {
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

/**
 * Start dragging an item
 */
function onItemDragStart(_event) {
	return;
}

/**
 * Stop dragging an item
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event, itemEl) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const index = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = VendingShop.getItemByIndex(index);

	if (!item) {
		return;
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return;
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
}

/**
 * Ask to use an item
 */
function onItemUsed(event, itemEl) {
	const index = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = VendingShop.getItemByIndex(index);

	if (item) {
		VendingShop.useItem(item);
		onItemOut(VendingShop.getRoot());
	}

	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Submit data to send items
 */
VendingShop.onSubmit = function onSubmit() {
	let pkt;
	if (_type === VendingShop.Type.VENDING_LIST) {
		pkt = new PACKET.CZ.REQ_CLOSESTORE();
	} else {
		pkt = new PACKET.CZ.REQ_CLOSE_BUYING_STORE();
	}
	Network.sendPacket(pkt);
	this.onRemove();

	if (_type === VendingShop.Type.VENDING_LIST && PACKETVER.value >= 20141016) {
		VendingReport.append();
	}
};

VendingShop.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component and export it
 */
export default UIManager.addComponent(VendingShop);
