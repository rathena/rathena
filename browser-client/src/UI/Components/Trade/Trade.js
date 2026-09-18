/**
 * UI/Components/Trade/Trade.js
 *
 * Manage Trade UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import 'UI/Elements/Elements.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import { InventoryItemTransferPriority } from 'UI/Components/Inventory/InventoryItemTransfer.js';
import htmlText from './Trade.html?raw';
import cssText from './Trade.css?raw';

/**
 * Create Component
 */
const Trade = new GUIComponent('Trade', cssText);

/**
 * HTML returned by render()
 */
Trade.render = () => htmlText;

/**
 * @var {Object} queue, item waiting from server an answer
 */
let _tmpCount = {};

/**
 * @var {Array} list of items to send
 */
const _send = [];

/**
 * @var {object} list of items to received
 */
const _recv = [];

/**
 * @var {string} trade title
 */
Trade.title = '';

/**
 * Capture key events so the zeny input field works inside Shadow DOM
 */
Trade.captureKeyEvents = true;

/**
 * Escape HTML special characters
 *
 * @param {string} text
 * @returns {string}
 */
function escapeHtml(text) {
	const div = document.createElement('div');
	div.appendChild(document.createTextNode(text));
	return div.innerHTML;
}

/**
 * Initialize UI
 */
Trade.init = function init() {
	const root = this.getRoot();

	// Bind buttons
	const okBtn = root.querySelector('.ok.enabled');
	if (okBtn) {
		okBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		okBtn.addEventListener('click', () => onConclude());
	}

	const tradeBtn = root.querySelector('.trade.enabled');
	if (tradeBtn) {
		tradeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		tradeBtn.addEventListener('click', () => onTrade());
	}

	const cancelBtn = root.querySelector('.cancel');
	if (cancelBtn) {
		cancelBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		cancelBtn.addEventListener('click', () => onCancel());
	}

	// Block propagation on disabled elements
	root.addEventListener('mousedown', e => {
		if (e.target.closest && e.target.closest('.disabled')) {
			e.stopImmediatePropagation();
		}
	});

	// Drag and drop support
	this._host.addEventListener('drop', e => onDrop(e));
	this._host.addEventListener('dragover', e => {
		e.preventDefault();
		e.stopImmediatePropagation();
	});

	// Select zeny input on mousedown
	const zenyInput = root.querySelector('.zeny.send');
	if (zenyInput) {
		zenyInput.addEventListener('mousedown', function () {
			this.select();
		});
	}

	// Item hover/info events on boxes
	const boxes = root.querySelectorAll('.box');
	boxes.forEach(box => {
		box.addEventListener('mouseover', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemOver(itemEl);
			}
		});
		box.addEventListener('mouseout', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemOut();
			}
		});
		box.addEventListener('contextmenu', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				onItemInfo(e, itemEl);
			}
		});
	});

	this.draggable('.titlebar');
};

/**
 * Guard keyboard input for the zeny <input> inside Shadow DOM
 */
Trade.onKeyDown = function onKeyDown(event) {
	if (this.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}

	return true;
};

/**
 * Initialize UI on append
 */
Trade.onAppend = function onAppend() {
	// Clean up (interface)
	resetUI.call(this);

	const root = this.getRoot();
	const titleEl = root.querySelector('.titlebar .title');
	if (titleEl) {
		titleEl.textContent = this.title;
	}

	const width = this._host.getBoundingClientRect().width;
	const height = this._host.getBoundingClientRect().height;
	this._host.style.top = `${(Renderer.height - height) / 2}px`;
	this._host.style.left = `${(Renderer.width - width) / 2}px`;
};

/**
 * Clean UP UI
 */
Trade.onRemove = function onRemove() {
	resetUI.call(this);
};

/**
 * Reset the UI to its initial state
 */
function resetUI() {
	_tmpCount = {};
	_recv.length = 0;
	_send.length = 0;

	const root = Trade.getRoot();

	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}

	// Reset ok/trade buttons visibility
	const okDisabled = root.querySelector('.ok.disabled');
	const tradeEnabled = root.querySelector('.trade.enabled');
	if (okDisabled) {
		okDisabled.style.display = 'none';
	}
	if (tradeEnabled) {
		tradeEnabled.style.display = 'none';
	}

	const okEnabled = root.querySelector('.ok.enabled');
	const tradeDisabled = root.querySelector('.trade.disabled');
	if (okEnabled) {
		okEnabled.style.display = '';
	}
	if (tradeDisabled) {
		tradeDisabled.style.display = '';
	}

	// Clear boxes
	const boxes = root.querySelectorAll('.box');
	boxes.forEach(box => {
		box.classList.remove('disabled');
		box.innerHTML = '';
	});

	// Reset zeny
	const zenySend = root.querySelector('.zeny.send');
	if (zenySend) {
		zenySend.value = '0';
		zenySend.classList.remove('disabled');
		zenySend.disabled = false;
	}

	const zenyRecv = root.querySelector('.zeny.recv');
	if (zenyRecv) {
		zenyRecv.textContent = '0';
	}
}

/**
 * Add Item to the trade window from our inventory
 *
 * @param {number} item index in inventory
 * @param {boolean} success ?
 */
Trade.addItemFromInventory = function addItemFromInventory(index, success) {
	// Reset value
	if (!success) {
		delete _tmpCount[index];
		return;
	}

	const root = Trade.getRoot();

	// ZENY
	if (index === 0) {
		const zenySend = root.querySelector('.zeny.send');
		if (zenySend) {
			zenySend.value = prettifyZeny(_tmpCount[index]);
		}
		return;
	}

	const inventoryItem = Inventory.getUI().removeItem(index, _tmpCount[index]);
	const item = Object.assign({}, inventoryItem);
	const it = DB.getItemInfo(item.ITID);
	const idx = _send.push(item) - 1;
	const box = root.querySelector('.box.send');
	item.count = _tmpCount[index];

	const itemDiv = document.createElement('div');
	itemDiv.className = 'item';
	itemDiv.setAttribute('data-index', idx);
	itemDiv.innerHTML =
		'<div class="icon"></div>' +
		`<div class="amount"><span class="count">${_tmpCount[index] || 1}</span></div>` +
		`<span class="name">${escapeHtml(DB.getItemName(item))}</span>`;
	box.appendChild(itemDiv);

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = root.querySelector(`.item[data-index="${idx}"] .icon`);
			if (icon && icon.closest('.box.send')) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);
};

/**
 * Add item to the trade UI
 *
 * @param {object} item
 */
Trade.addItem = function addItem(item) {
	const root = Trade.getRoot();

	// ZENY
	if (item.ITID === 0) {
		const zenyRecv = root.querySelector('.zeny.recv');
		if (zenyRecv) {
			zenyRecv.textContent = prettifyZeny(item.count);
		}
		return;
	}

	const it = DB.getItemInfo(item.ITID);
	const idx = _recv.push(item) - 1;
	const box = root.querySelector('.box.recv');

	const itemDiv = document.createElement('div');
	itemDiv.className = 'item';
	itemDiv.setAttribute('data-index', idx);
	itemDiv.innerHTML =
		'<div class="icon"></div>' +
		`<div class="amount">${item.count}</div>` +
		`<span class="name">${escapeHtml(DB.getItemName(item))}</span>`;
	box.appendChild(itemDiv);

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = root.querySelector(`.item[data-index="${idx}"] .icon`);
			if (icon && icon.closest('.box.recv')) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);
};

/**
 * Prettify number (15000 -> 15,000)
 *
 * @param {number} value
 * @return {string}
 */
function prettifyZeny(value) {
	return Number(value).toLocaleString('en-US');
}

/**
 * Request to add an item to the trade UI
 *
 * @param {number} index - item index in inventory
 * @param {number} count - item count
 */
function onRequestAddItem(index, count) {
	// You cannot overlap items on a window
	if (index in _tmpCount) {
		ChatBox.addText(DB.getMessage(51), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	// You cannot trade more than 10 types of items per trade.
	if (_send.length >= 10) {
		ChatBox.addText(DB.getMessage(297), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	_tmpCount[index] = count;
	Trade.reqAddItem(index, count);
}

Trade.inventoryTransferPriority = InventoryItemTransferPriority.TRADE;
Trade.receiveInventoryItemStack = function receiveInventoryItemStack(item) {
	const sendBox = Trade.getRoot().querySelector('.box.send');
	if (!item || !sendBox || sendBox.classList.contains('disabled')) {
		return false;
	}

	onRequestAddItem(item.index, item.count || 1);
	return true;
};

/**
 * Conclude a part of the trade
 *
 * @param {string} element - 'send' or 'recv'
 */
Trade.conclude = function conclude(element) {
	const root = Trade.getRoot();

	const box = root.querySelector(`.box.${element}`);
	if (box) {
		box.classList.add('disabled');
	}

	if (element === 'send') {
		const okDisabled = root.querySelector('.ok.disabled');
		const okEnabled = root.querySelector('.ok.enabled');
		if (okDisabled) {
			okDisabled.style.display = '';
		}
		if (okEnabled) {
			okEnabled.style.display = 'none';
		}

		const zenySend = root.querySelector('.zeny.send');
		if (zenySend) {
			zenySend.classList.add('disabled');
			zenySend.disabled = true;
		}
	}

	// Can conclude
	const recvDisabled = root.querySelector('.box.recv.disabled');
	const sendDisabled = root.querySelector('.box.send.disabled');
	if (
		recvDisabled &&
		recvDisabled.style.display !== 'none' &&
		sendDisabled &&
		sendDisabled.style.display !== 'none'
	) {
		const tradeEnabled = root.querySelector('.trade.enabled');
		const tradeDisabledBtn = root.querySelector('.trade.disabled');
		if (tradeEnabled) {
			tradeEnabled.style.display = '';
		}
		if (tradeDisabledBtn) {
			tradeDisabledBtn.style.display = 'none';
		}
	}
};

/**
 * Cancel the deal
 */
function onCancel() {
	Trade.remove();
	Trade.onCancel();
}

/**
 * Conclude our part
 */
function onConclude() {
	const root = Trade.getRoot();
	const zenySend = root.querySelector('.zeny.send');
	let zeny = parseInt(zenySend ? zenySend.value : '0', 10) || 0;
	zeny = Math.min(Math.max(0, zeny), Session.zeny);

	onRequestAddItem(0, zeny);
	Trade.onConclude();
}

/**
 * Let's finish the trade
 */
function onTrade() {
	Trade.onTradeSubmit();
	const root = Trade.getRoot();
	const tradeEnabled = root.querySelector('.trade.enabled');
	const tradeDisabled = root.querySelector('.trade.disabled');
	if (tradeEnabled) {
		tradeEnabled.style.display = 'none';
	}
	if (tradeDisabled) {
		tradeDisabled.style.display = '';
	}
}

/**
 * Drop from inventory to trade
 */
function onDrop(event) {
	let data;

	try {
		data = JSON.parse(
			event.dataTransfer ? event.dataTransfer.getData('Text') : event.originalEvent.dataTransfer.getData('Text')
		);
	} catch (_e) {
		// Ignore parsing error
	}

	event.stopImmediatePropagation();
	event.preventDefault();

	// Just support items for now ?
	if (!data || data.type !== 'item' || data.from !== 'Inventory') {
		return false;
	}

	const item = data.data;

	// Have to specify how much
	if (item.count > 1) {
		InputBox.append();
		InputBox.setType('number', false, item.count);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			let value = parseInt(count, 10) || 0;
			value = Math.min(Math.max(value, 0), item.count); // cap

			InputBox.remove();

			if (value) {
				onRequestAddItem(item.index, value);
			}
		};

		return false;
	}

	onRequestAddItem(item.index, 1);
	return false;
}

/**
 * When mouse is over an item, show title
 *
 * @param {HTMLElement} itemEl
 */
function onItemOver(itemEl) {
	const idx = parseInt(itemEl.getAttribute('data-index'), 10);
	const isSend = itemEl.parentNode.className.match(/send/i);
	const item = isSend ? _send[idx] : _recv[idx];

	if (!item) {
		return;
	}

	const root = Trade.getRoot();
	const overlay = root.querySelector('.overlay');
	if (!overlay) {
		return;
	}

	const itemRect = itemEl.getBoundingClientRect();
	const hostRect = Trade._host.getBoundingClientRect();

	const posLeft = itemRect.left - hostRect.left;
	const posTop = itemRect.top - hostRect.top;

	overlay.style.display = '';
	overlay.style.top = `${posTop + 5}px`;
	overlay.style.left = `${posLeft + 30}px`;
	overlay.textContent = DB.getItemName(item);

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	} else {
		overlay.classList.add('grey');
	}
}

/**
 * Hide the item title when mouse is not over anymore
 */
function onItemOut() {
	const root = Trade.getRoot();
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

/**
 * Display ItemInfo UI
 *
 * @param {Event} event
 * @param {HTMLElement} itemEl
 */
function onItemInfo(event, itemEl) {
	const idx = parseInt(itemEl.getAttribute('data-index'), 10);
	const isSend = itemEl.parentNode.className.match(/send/i);
	const item = isSend ? _send[idx] : _recv[idx];

	if (!item) {
		event.stopImmediatePropagation();
		event.preventDefault();
		return;
	}

	// Don't add the same UI twice, remove it
	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
	}

	// Add ui to window
	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);

	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Callbacks
 */
Trade.onConclude = function onConclude() {}; // eslint-disable-line no-shadow
Trade.onTradeSubmit = function onTradeSubmit() {};
Trade.reqAddItem = function reqAddItem() {};
Trade.onCancel = function onCancel() {}; // eslint-disable-line no-shadow

/**
 * Set mouse mode
 */
Trade.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component and export it
 */
export default UIManager.addComponent(Trade);
