/**
 * UI/Components/CartItems/CartItems.js
 *
 * Character CartItems Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import Session from 'Engine/SessionStorage.js';
import htmlText from './CartItems.html?raw';
import cssText from './CartItems.css?raw';
import Storage from 'UI/Components/Storage/Storage.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';

/**
 * Create Component
 */
const CartItems = new GUIComponent('CartItems', cssText);

CartItems.render = () => htmlText;

/**
 * Store inventory items
 */
CartItems.list = [];

/**
 * @var {number} used to remember the window height
 */
let _realSize = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'CartItems',
	{
		x: 200,
		y: 200,
		width: 7,
		height: 4,
		show: false,
		reduce: false
	},
	1.0
);

/**
 * Initialize UI
 */
CartItems.init = function Init() {
	const root = this.getRoot();

	// Bind buttons
	const baseBtn = root.querySelector('.titlebar .base');
	if (baseBtn) {
		baseBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
	}

	const miniBtn = root.querySelector('.titlebar .mini');
	if (miniBtn) {
		miniBtn.addEventListener('click', onToggleReduction);
	}

	const extendBtn = root.querySelector('.footer .extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', onResize);
	}

	const closeBtn = root.querySelector('.titlebar .close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => {
			CartItems._host.style.display = 'none';
		});
	}

	// on drop item
	this._host.addEventListener('drop', onDrop);
	this._host.addEventListener('dragover', e => e.stopImmediatePropagation());

	// Items event (delegation)
	const content = root.querySelector('.container .content');
	if (content) {
		content.addEventListener('wheel', onScroll);
		content.addEventListener('mouseover', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemOver.call(item, e);
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
			e.preventDefault();
			const item = e.target.closest('.item');
			if (item) {
				onItemInfo.call(item, e);
			}
		});
		content.addEventListener('dblclick', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemUsed.call(item, e);
			}
		});
	}

	this.draggable('.titlebar');
};

/**
 * Apply preferences once append to body
 */
CartItems.onAppend = function OnAppend() {
	if (Session.Entity.hasCart === false) {
		this._host.style.display = 'none';
	}

	// Apply preferences
	if (!_preferences.show) {
		this._host.style.display = 'none';
	}

	this.resize(_preferences.width, _preferences.height);

	const hostRect = this._host.getBoundingClientRect();
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostRect.height)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostRect.width)}px`;

	_realSize = _preferences.reduce ? 0 : hostRect.height;
	const miniBtn = this.getRoot().querySelector('.titlebar .mini');
	if (miniBtn) {
		miniBtn.dispatchEvent(new Event('mousedown'));
	}
};

/**
 * Remove Inventory from window (and so clean up items)
 */
CartItems.onRemove = function OnRemove() {
	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.innerHTML = '';
	}
	this.list.length = 0;

	// Remove any ItemInfo instances from the document
	document.querySelectorAll('.ItemInfo').forEach(el => el.remove());

	// Save preferences
	_preferences.show = this._host.style.display !== 'none';
	_preferences.reduce = !!_realSize;
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	const hostRect = this._host.getBoundingClientRect();
	_preferences.width = Math.floor((hostRect.width - (23 + 16 + 16 - 30)) / 32);
	_preferences.height = Math.floor((hostRect.height - (31 + 19 - 30)) / 32);
	_preferences.save();
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
CartItems.onShortCut = function onShurtCut(key) {
	if (Session.Entity.hasCart === false) {
		return;
	}

	switch (key.cmd) {
		case 'TOGGLE':
			if (this._host.style.display === 'none') {
				this._host.style.display = '';
				this.focus();
			} else {
				this._host.style.display = 'none';
				this._host.dispatchEvent(new Event('mouseleave'));
			}
			break;
	}
};

CartItems.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this._host.style.display = this._host.style.display === 'none' ? '' : 'none';
	}
};

/**
 * Extend inventory window size
 *
 * @param {number} width
 * @param {number} height
 */
CartItems.resize = function Resize(width, height) {
	width = Math.min(Math.max(width, 6), 9);
	height = Math.min(Math.max(height, 2), 6);

	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.style.width = `${width * 32 + 13}px`;
		content.style.height = `${height * 32}px`;
	}

	this._host.style.width = `${16 + 39 + width * 32}px`;
	this._host.style.height = `${31 + 19 + height * 32}px`;
};

/**
 * Get item object
 *
 * @param {number} id
 * @returns {Item}
 */
CartItems.getItemById = function GetItemById(id) {
	const list = CartItems.list;
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
CartItems.getItemByIndex = function getItemByIndex(index) {
	const list = CartItems.list;
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
CartItems.setItems = function SetItems(items) {
	for (let i = 0, count = items.length; i < count; ++i) {
		const object = this.getItemByIndex(items[i].index);
		if (object) {
			this.removeItem(object.index, object.count);
		}
		if (this.addItemSub(items[i])) {
			this.list.push(items[i]);
		}
	}
};

CartItems.setCartInfo = function SetCartInfo(curCount, maxCount, curWeight, maxWeight) {
	const root = this.getRoot();
	const ncnt = root.querySelector('.ncnt');
	const mcnt = root.querySelector('.mcnt');
	const nwt = root.querySelector('.nwt');
	const mwt = root.querySelector('.mwt');
	if (ncnt) {
		ncnt.textContent = curCount;
	}
	if (mcnt) {
		mcnt.textContent = maxCount;
	}
	if (nwt) {
		nwt.textContent = curWeight / 10;
	}
	if (mwt) {
		mwt.textContent = maxWeight / 10;
	}
};

/**
 * Insert Item to inventory
 *
 * @param {object} Item
 */
CartItems.addItem = function AddItem(item) {
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
CartItems.addItemSub = function AddItemSub(item) {
	// Equip item (if not arrow)
	if (item.WearState && item.type !== ItemType.AMMO && item.type !== ItemType.CARD) {
		return false;
	}

	const it = DB.getItemInfo(item.ITID);
	const root = this.getRoot();
	const content = root.querySelector('.container .content');
	if (!content) {
		return true;
	}

	content.insertAdjacentHTML(
		'beforeend',
		`<div class="item" data-index="${item.index}" draggable="true">` +
			'<div class="icon"></div>' +
			'<div class="grade"></div>' +
			`<div class="amount"><span class="count">${item.count || 1}</span></div>` +
			'</div>'
	);

	const hideEl = root.querySelector('.hide');
	if (hideEl) {
		if (content.offsetHeight < content.scrollHeight) {
			hideEl.style.display = 'none';
		} else {
			hideEl.style.display = 'block';
		}
	}

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

	/* Grade System */
	if (item.enchantgrade) {
		Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/grade_icon' + item.enchantgrade + '.bmp', data => {
			const grade = root.querySelector(`.item[data-index="${item.index}"] .grade`);
			if (grade) {
				grade.style.backgroundImage = `url(${data})`;
			}
		});
	}

	return true;
};

/**
 * Remove item from inventory
 *
 * @param {number} index in inventory
 * @param {number} count
 */
CartItems.removeItem = function RemoveItem(index, count) {
	const item = this.getItemByIndex(index);

	if (!item || count <= 0) {
		return null;
	}

	if (item.count) {
		item.count -= count;

		if (item.count > 0) {
			const root = this.getRoot();
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = item.count;
			}
			return item;
		}
	}

	this.list.splice(this.list.indexOf(item), 1);
	const root = this.getRoot();
	const el = root.querySelector(`.item[data-index="${item.index}"]`);
	if (el) {
		el.remove();
	}

	const content = root.querySelector('.container .content');
	const hideEl = root.querySelector('.hide');
	if (content && hideEl) {
		if (content.offsetHeight === content.scrollHeight) {
			hideEl.style.display = 'block';
		}
	}

	return item;
};

/**
 * Remove item from inventory
 *
 * @param {number} index in inventory
 * @param {number} count
 */
CartItems.updateItem = function UpdateItem(index, count) {
	const item = this.getItemByIndex(index);

	if (!item) {
		return;
	}

	item.count = count;
	const root = this.getRoot();

	// Update quantity
	if (item.count > 0) {
		const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
		if (countEl) {
			countEl.textContent = item.count;
		}
		return;
	}

	// no quantity, remove
	this.list.splice(this.list.indexOf(item), 1);
	const el = root.querySelector(`.item[data-index="${item.index}"]`);
	if (el) {
		el.remove();
	}

	const content = root.querySelector('.container .content');
	const hideEl = root.querySelector('.hide');
	if (content && hideEl) {
		if (content.offsetHeight === content.scrollHeight) {
			hideEl.style.display = 'block';
		}
	}
};

/**
 * Extend inventory window size
 */
function onResize() {
	const content = CartItems.getRoot().querySelector('.container .content');
	const hideEl = CartItems.getRoot().querySelector('.hide');
	const top = CartItems._host.offsetTop;
	const left = CartItems._host.offsetLeft;
	let lastWidth = 0;
	let lastHeight = 0;

	function resizing() {
		const extraX = 23 + 16 + 16 - 30;
		const extraY = 31 + 19 - 30;

		let w = Math.floor((Mouse.screen.x - left - extraX) / 32);
		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);

		// Maximum and minimum window size
		w = Math.min(Math.max(w, 6), 9);
		h = Math.min(Math.max(h, 2), 6);

		if (w === lastWidth && h === lastHeight) {
			return;
		}

		CartItems.resize(w, h);
		lastWidth = w;
		lastHeight = h;

		//Show or hide scrollbar
		if (content && hideEl) {
			if (content.offsetHeight === content.scrollHeight) {
				hideEl.style.display = 'block';
			} else {
				hideEl.style.display = 'none';
			}
		}
	}

	// Start resizing
	const _Interval = setInterval(resizing, 30);

	// Stop resizing on left click
	const onMouseUp = event => {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	};
	window.addEventListener('mouseup', onMouseUp);
}

/**
 * Hide/show inventory's content
 */
function onToggleReduction() {
	const root = CartItems.getRoot();
	const panel = root.querySelector('.panel');

	if (_realSize) {
		if (panel) {
			panel.style.display = 'block';
		}
		CartItems._host.style.height = `${_realSize}px`;
		_realSize = 0;
	} else {
		_realSize = CartItems._host.getBoundingClientRect().height;
		CartItems._host.style.height = '17px';
		if (panel) {
			panel.style.display = 'none';
		}
	}
}

/**
 * Drop an item from storage to inventory
 *
 * @param {event}
 */
function onDrop(event) {
	let item, data;
	event.stopImmediatePropagation();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return false;
	}

	// Just allow item from storage
	if (data.type !== 'item' || (data.from !== 'Storage' && data.from !== 'Inventory')) {
		return false;
	}

	// Have to specify how much
	if (item.count > 1) {
		InputBox.append();
		InputBox.setType('number', false, item.count);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			InputBox.remove();

			switch (data.from) {
				case 'Storage':
					Storage.reqMoveItemToCart(item.index, parseInt(count, 10));
					break;

				case 'Inventory':
					Inventory.getUI().reqMoveItemToCart(item.index, parseInt(count, 10));
					break;
			}
		};
		return false;
	}

	switch (data.from) {
		case 'Storage':
			Storage.reqMoveItemToCart(item.index, 1);
			break;

		case 'Inventory':
			Inventory.getUI().reqMoveItemToCart(item.index, 1);
			break;
	}

	return false;
}

/**
 * Block the scroll to move 32px at each move
 */
function onScroll(event) {
	const delta = event.deltaY > 0 ? -1 : 1;
	const el = event.currentTarget;

	el.scrollTop = Math.floor(el.scrollTop / 32) * 32 - delta * 32;

	if (el._roScrollbarRestart) {
		el._roScrollbarRestart();
	}

	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Show item name when mouse is over
 */
function onItemOver(_e) {
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = CartItems.getItemByIndex(idx);

	if (!item) {
		return;
	}

	let quantity = ' ea';
	if (
		(item.type === ItemType.WEAPON || item.type === ItemType.ARMOR) &&
		item.Options &&
		item.Options.filter(Option => Option.index !== 0).length > 0
	) {
		quantity = ' Quantity';
	}

	const root = CartItems.getRoot();
	const overlay = root.querySelector('.overlay');
	const rootEl = root.querySelector('#cartitems') || root;
	const itemRect = this.getBoundingClientRect();
	const rootRect = rootEl.getBoundingClientRect();

	// Display box
	if (overlay) {
		overlay.style.display = 'block';
		overlay.style.top = `${itemRect.top - rootRect.top}px`;
		overlay.style.left = `${itemRect.left - rootRect.left + 35}px`;
		overlay.textContent = `${DB.getItemName(item)}: ${item.count || 1}${quantity}`;

		if (item.IsIdentified) {
			overlay.classList.remove('grey');
		} else {
			overlay.classList.add('grey');
		}
	}
}

/**
 * Hide the item name
 */
function onItemOut() {
	const root = CartItems.getRoot();
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
	const item = CartItems.getItemByIndex(index);

	if (!item) {
		return;
	}

	// Set image to the drag drop element
	const img = new Image();
	const iconEl = this.querySelector('.icon');
	const url = iconEl ? iconEl.style.backgroundImage.match(/\(([^)]+)/)?.[1] : '';
	img.decoding = 'async';
	img.src = url ? url.replace(/^["']/, '').replace(/["']$/, '') : '';

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'CartItems',
				data: item
			})
		)
	);

	onItemOut();
}

/**
 * Stop dragging an item
 *
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();

	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = CartItems.getItemByIndex(index);

	if (!item) {
		return false;
	}

	// If right click w/ alt (Request Transfer Item)
	if (event.altKey && event.which === 3) {
		event.stopImmediatePropagation();
		transferItemToOtherUI(item);
		return false;
	}

	// Don't add the same UI twice, remove it
	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		if (ItemCompare.ui) {
			ItemCompare.remove();
		}
		return false;
	}

	// Remove existing compare UI if it's currently displayed
	if (ItemCompare.ui) {
		ItemCompare.remove();
	}

	// Add ui to window
	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);

	// Check if there is an equipped item in the same location
	const compareItem = Equipment.getUI().isInEquipList(item.location);

	// If a comparison item is found, display comparison
	if (compareItem && Inventory.getUI().itemcomp) {
		ItemCompare.prepare();
		ItemCompare.append();
		ItemCompare.uid = compareItem.ITID;
		ItemCompare.setItem(compareItem);
	}

	return false;
}

/**
 * Alt Right Click Request Transfer
 */
function transferItemToOtherUI(item) {
	const storageUI = Storage.getUI();
	const inventoryUI = Inventory.getUI();
	const isStorageOpen = storageUI._host ? storageUI._host.style.display !== 'none' : false;
	const isInventoryOpen = inventoryUI._host ? inventoryUI._host.style.display !== 'none' : false;

	if (!item) {
		return false;
	}

	const count = item.count || 1;

	if (isStorageOpen) {
		Storage.reqAddItemFromCart(item.index, count);
	} else if (isInventoryOpen) {
		CartItems.reqRemoveItem(item.index, count);
	}

	return true;
}

/**
 * Ask to use an item
 */
function onItemUsed(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = CartItems.getItemByIndex(index);

	if (item) {
		CartItems.useItem(item);
		onItemOut();
	}

	event.stopImmediatePropagation();
	event.preventDefault();
}

CartItems.reqRemoveItem = function reqRemoveItem() {};

CartItems.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component and export it
 */
export default UIManager.addComponent(CartItems);
