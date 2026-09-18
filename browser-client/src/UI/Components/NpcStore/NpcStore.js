/**
 * UI/Components/NpcStore/NpcStore.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import { InventoryItemTransferPriority } from 'UI/Components/Inventory/InventoryItemTransfer.js';
import htmlText from './NpcStore.html?raw';
import cssText from './NpcStore.css?raw';

/**
 * Create NPC Store component
 */
const NpcStore = new GUIComponent('NpcStore', cssText);

NpcStore.render = () => htmlText;

/**
 * @let {enum} Store type
 */
NpcStore.Type = {
	BUY: 0,
	SELL: 1,
	VENDING_STORE: 2,
	BUYING_STORE: 3,
	MARKETSHOP: 4,
	BARTER_MARKET: 5,
	BARTER_MARKET_EXTENDED: 6,
	CASH_SHOP: 7
};

/**
 * Freeze the mouse
 */
NpcStore.mouseMode = GUIComponent.MouseMode.FREEZE;

/**
 * @let {initialPreferences}
 */
const initialPreferences = {
	[NpcStore.Type.BARTER_MARKET_EXTENDED]: {
		inputWindow: { x: 100, y: 100, height: 7, width: 350 },
		outputWindow: { x: 100 + 350 + 10, y: 100, height: 7, width: 350 },
		AvailableItemsWindow: { x: 100 + 280 + 10, y: 100 + 4 * 32 - 2 * 32, height: 2 },
		PurchaseResult: { x: 100 + 280 + 10, y: 100 + 7 * 32 - 2 * 32, height: 2 }
	},
	DEFAULT: {
		inputWindow: { x: 100, y: 100, height: 7, width: 280 },
		outputWindow: { x: 100 + 280 + 10, y: 100 + 7 * 32 - 2 * 32, height: 2, width: 280 },
		AvailableItemsWindow: { x: 100 + 280 + 10, y: 100 + 4 * 32 - 2 * 32, height: 2 },
		PurchaseResult: { x: 100 + 280 + 10, y: 100 + 7 * 32 - 2 * 32, height: 2 }
	}
};

/**
 * @let {Preferences}
 */
const _preferences = Preferences.get('NpcStore', {}, 1.0);

/**
 * @let {Array} item list
 */
const _input = [];

/**
 * @let {Array} output list
 */
const _output = [];

/**
 * @let {number} type (buy/sell)
 */
let _type;

/**
 * @let {boolean} whether the close packet was already sent to the server
 */
let _closePacketSent = false;

/**
 * Make a sub-window element draggable by its handle
 */
function _makeDraggable(element, handle) {
	handle.addEventListener('mousedown', e => {
		if (e.which !== 1) {
			return;
		}

		const offsetX = element.offsetLeft - e.pageX;
		const offsetY = element.offsetTop - e.pageY;

		const onMove = ev => {
			element.style.left = `${ev.pageX + offsetX}px`;
			element.style.top = `${ev.pageY + offsetY}px`;
		};

		const onUp = ev => {
			if (ev.which === 1) {
				window.removeEventListener('mousemove', onMove);
				window.removeEventListener('mouseup', onUp);
			}
		};

		window.addEventListener('mousemove', onMove);
		window.addEventListener('mouseup', onUp);

		e.stopImmediatePropagation();
	});
}

/**
 * Helper: escape HTML
 */
function _escapeHTML(text) {
	const div = document.createElement('div');
	div.textContent = text;
	return div.innerHTML;
}

/**
 * Initialize component
 */
NpcStore.init = function init() {
	const root = NpcStore.getRoot();

	root.querySelector('.btn.cancel').addEventListener('click', () => this.remove());
	root.querySelector('.btn.buy').addEventListener('click', () => this.submit());
	root.querySelector('.btn.sell').addEventListener('click', () => this.submit());

	const selectall = root.querySelector('.selectall');
	if (selectall) {
		selectall.addEventListener('mousedown', function () {
			onToggleSelectAmount.call(this);
		});
	}

	// Resize handlers
	const InputWindow = root.querySelector('.InputWindow');
	const OutputWindow = root.querySelector('.OutputWindow');
	const AvailableItemsWindow = root.querySelector('.AvailableItemsWindow');
	const PurchaseResult = root.querySelector('.PurchaseResult');

	InputWindow.querySelector('.resize').addEventListener('mousedown', () => onResize(InputWindow));
	OutputWindow.querySelector('.resize').addEventListener('mousedown', () => onResize(OutputWindow));
	AvailableItemsWindow.querySelector('.resize').addEventListener('mousedown', () => onResize(AvailableItemsWindow));
	PurchaseResult.querySelector('.resize').addEventListener('mousedown', () => onResize(PurchaseResult));

	// Items options via event delegation on content areas
	root.querySelectorAll('.content').forEach(content => {
		content.addEventListener('wheel', function (event) {
			onScroll.call(this, event);
		});
		content.addEventListener('contextmenu', e => {
			const icon = e.target.closest('.icon');
			if (icon) {
				e.preventDefault();
				e.stopImmediatePropagation();
				onItemInfo.call(icon, e);
			}
		});
		content.addEventListener('dblclick', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemSelected.call(item);
			}
		});
		content.addEventListener('mousedown', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemFocus.call(item);
			}
		});
	});

	// Drag start on items
	root.addEventListener('dragstart', e => {
		const item = e.target.closest('.item');
		if (item) {
			onDragStart.call(item, e);
		}
	});
	root.addEventListener('dragend', e => {
		const item = e.target.closest('.item');
		if (item) {
			delete window._OBJ_DRAG_;
		}
	});

	// Drop targets
	[InputWindow, OutputWindow, AvailableItemsWindow].forEach(win => {
		win.addEventListener('drop', function (event) {
			onDrop.call(this, event);
		});
		win.addEventListener('dragover', e => {
			e.preventDefault();
			e.stopImmediatePropagation();
		});
		win.addEventListener('mousedown', () => {
			NpcStore.focus();
		});
	});

	// Draggable sub-windows
	_makeDraggable(InputWindow, InputWindow.querySelector('.titlebar'));
	_makeDraggable(OutputWindow, OutputWindow.querySelector('.titlebar'));
	_makeDraggable(AvailableItemsWindow, AvailableItemsWindow.querySelector('.titlebar'));
	_makeDraggable(PurchaseResult, PurchaseResult.querySelector('.titlebar'));

	// MarketShop close
	root.querySelector('.btn.ok').addEventListener('click', () => {
		NpcStore.closeStore();
	});
};

/**
 * Player should not be able to move when the store is opened
 */
NpcStore.onAppend = function onAppend() {
	_closePacketSent = false;

	Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (_preferences.select_all ? 1 : 0) + '.bmp', function (data) {
		const root = NpcStore.getRoot();
		const selectall = root.querySelector('.selectall');
		if (selectall) {
			selectall.style.backgroundImage = `url(${data})`;
		}
	});
};

/**
 * Released movement and save preferences
 */
NpcStore.onRemove = function onRemove() {
	const root = NpcStore.getRoot();
	const InputWindow = root.querySelector('.InputWindow');
	const OutputWindow = root.querySelector('.OutputWindow');
	const AvailableItemsWindow = root.querySelector('.AvailableItemsWindow');
	const PurchaseResult = root.querySelector('.PurchaseResult');

	_input.length = 0;
	_output.length = 0;

	const currentPref = getCurrentPref();

	currentPref.inputWindow.x = parseInt(InputWindow.style.left, 10);
	currentPref.inputWindow.y = parseInt(InputWindow.style.top, 10);
	currentPref.inputWindow.height = (InputWindow.querySelector('.content').offsetHeight / 32) | 0;

	currentPref.outputWindow.x = parseInt(OutputWindow.style.left, 10);
	currentPref.outputWindow.y = parseInt(OutputWindow.style.top, 10);
	currentPref.outputWindow.height = (OutputWindow.querySelector('.content').offsetHeight / 32) | 0;

	currentPref.AvailableItemsWindow.x = parseInt(AvailableItemsWindow.style.left, 10);
	currentPref.AvailableItemsWindow.y = parseInt(AvailableItemsWindow.style.top, 10);
	currentPref.AvailableItemsWindow.height = (AvailableItemsWindow.querySelector('.content').offsetHeight / 32) | 0;

	currentPref.PurchaseResult.x = parseInt(PurchaseResult.style.left, 10);
	currentPref.PurchaseResult.y = parseInt(PurchaseResult.style.top, 10);
	currentPref.PurchaseResult.height = (PurchaseResult.querySelector('.content').offsetHeight / 32) | 0;

	_preferences.save();

	root.querySelectorAll('.content').forEach(c => {
		c.innerHTML = '';
	});
	root.querySelectorAll('.total .result').forEach(r => {
		r.textContent = '0';
	});
	root.querySelectorAll('.totalP .resultP').forEach(r => {
		r.textContent = '0';
	});

	if (!_closePacketSent) {
		NpcStore.StoreClosePacket(_type);
	}
};

/**
 * Key Listener
 */
NpcStore.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
	}
};

/**
 * Helper to show/hide elements by selector
 */
function _hideAll(root, selector) {
	root.querySelectorAll(selector).forEach(el => {
		el.style.display = 'none';
	});
}

function _showAll(root, selector) {
	root.querySelectorAll(selector).forEach(el => {
		el.style.display = '';
	});
}

/**
 * Specify the type of the shop
 *
 * @param {number} type (see NpcStore.Type.*)
 */
NpcStore.setType = function setType(type) {
	const root = NpcStore.getRoot();

	switch (type) {
		case NpcStore.Type.BUY:
		case NpcStore.Type.MARKETSHOP:
			_hideAll(
				root,
				'.WinSell, .WinVendingStore, .WinCash, .WinBuyingStore, .AvailableItemsWindow, .PurchaseResult'
			);
			_showAll(root, '.WinBuy');
			break;

		case NpcStore.Type.SELL:
			_hideAll(
				root,
				'.WinBuy, .WinVendingStore, .WinCash, .WinBuyingStore, .AvailableItemsWindow, .PurchaseResult'
			);
			_showAll(root, '.WinSell');
			break;

		case NpcStore.Type.VENDING_STORE:
			_hideAll(root, '.WinBuy, .WinSell, .WinCash, .WinBuyingStore, .AvailableItemsWindow, .PurchaseResult');
			_showAll(root, '.WinVendingStore');
			break;

		case NpcStore.Type.BUYING_STORE:
			_hideAll(root, '.WinBuy, .WinSell, .WinCash, .WinVendingStore, .PurchaseResult');
			_showAll(root, '.WinBuyingStore, .AvailableItemsWindow');
			root.querySelectorAll('.content').forEach(c => {
				c.style.height = '160px';
			});
			root.querySelectorAll('.contentAvailable').forEach(c => {
				c.style.height = '65px';
			});
			break;

		case NpcStore.Type.BARTER_MARKET:
		case NpcStore.Type.BARTER_MARKET_EXTENDED:
			_hideAll(
				root,
				'.WinSell, .WinVendingStore, .WinCash, .WinBuyingStore, .AvailableItemsWindow, .PurchaseResult'
			);
			_showAll(root, '.WinBuy');
			_hideAll(root, '.total');
			break;

		case NpcStore.Type.CASH_SHOP:
			_hideAll(
				root,
				'.WinSell, .WinVendingStore, .WinBuyingStore, .AvailableItemsWindow, .PurchaseResult, .total'
			);
			_showAll(root, '.WinBuy');
			break;
	}

	_type = type;

	const currentPref = getCurrentPref();

	const InputWindow = root.querySelector('.InputWindow');
	const OutputWindow = root.querySelector('.OutputWindow');
	const AvailableItemsWindow = root.querySelector('.AvailableItemsWindow');
	const PurchaseResult = root.querySelector('.PurchaseResult');

	InputWindow.style.top = `${currentPref.inputWindow.y}px`;
	InputWindow.style.left = `${currentPref.inputWindow.x}px`;
	OutputWindow.style.top = `${currentPref.outputWindow.y}px`;
	OutputWindow.style.left = `${currentPref.outputWindow.x}px`;
	AvailableItemsWindow.style.top = `${currentPref.AvailableItemsWindow.y}px`;
	AvailableItemsWindow.style.left = `${currentPref.AvailableItemsWindow.x}px`;
	PurchaseResult.style.top = `${currentPref.PurchaseResult.y}px`;
	PurchaseResult.style.left = `${currentPref.PurchaseResult.x}px`;

	resize(InputWindow.querySelector('.content'), currentPref.inputWindow.height);
	resize(OutputWindow.querySelector('.content'), currentPref.outputWindow.height);
	resize(AvailableItemsWindow.querySelector('.content'), currentPref.AvailableItemsWindow.height);
	resize(PurchaseResult.querySelector('.content'), currentPref.PurchaseResult.height);

	InputWindow.style.width = `${currentPref.inputWindow.width}px`;
	OutputWindow.style.width = `${currentPref.outputWindow.width}px`;
};

/**
 * Add items to list
 *
 * @param {Array} item list
 */
NpcStore.setList = function setList(items) {
	let i, count;
	let it, item, out;

	const root = NpcStore.getRoot();
	root.querySelectorAll('.content').forEach(c => {
		c.innerHTML = '';
	});
	root.querySelectorAll('.total .result').forEach(r => {
		r.textContent = '0';
	});
	root.querySelectorAll('.totalP .resultP').forEach(r => {
		r.textContent = '0';
	});

	_input.length = 0;
	_output.length = 0;
	const content = root.querySelector('.InputWindow .content');
	const availableContent = root.querySelector('.AvailableItemsWindow .content');
	switch (_type) {
		case NpcStore.Type.BUY:
		case NpcStore.Type.VENDING_STORE:
		case NpcStore.Type.MARKETSHOP:
		case NpcStore.Type.CASH_SHOP:
			for (i = 0, count = items.length; i < count; ++i) {
				if (!('index' in items[i])) {
					items[i].index = i;
				}
				items[i].count = items[i].count || Infinity;
				items[i].IsIdentified = true;
				out = Object.assign({}, items[i]);
				out.count = 0;

				addItem(content, items[i]);

				_input[items[i].index] = items[i];
				_output[items[i].index] = out;
			}
			break;
		case NpcStore.Type.BUYING_STORE:
			for (i = 0, count = items.length; i < count; ++i) {
				if (!('index' in items[i])) {
					items[i].index = i;
				}
				items[i].count = items[i].count || Infinity;
				items[i].IsIdentified = true;
				out = Object.assign({}, items[i]);
				out.count = 0;

				addItem(content, items[i]);
				it = Inventory.getUI().getItemById(items[i].ITID);

				if (it) {
					item = Object.assign({}, it);
					item.ITID = it.ITID;
					item.price = items[i].price;
					item.count = 'count' in item ? item.count : 1;
					item.maxCount = isFinite(items[i].count) ? items[i].count : 0;

					out = Object.assign({}, item);
					out.count = 0;

					addItem(availableContent, item);

					_input[item.index] = item;
					_output[item.index] = out;
				}
			}
			break;

		case NpcStore.Type.BARTER_MARKET:
		case NpcStore.Type.BARTER_MARKET_EXTENDED:
			for (i = 0, count = items.length; i < count; ++i) {
				if (!('index' in items[i])) {
					items[i].index = i;
				}
				items[i].count = items[i].count || Infinity;
				items[i].IsIdentified = true;
				out = Object.assign({}, items[i]);
				out.count = 0;

				addItem(content, items[i]);

				_input[items[i].index] = items[i];
				_output[items[i].index] = out;
			}
			break;

		case NpcStore.Type.SELL: {
			const InventoryVersion = UIManager.getComponent('Inventory').name;
			for (i = 0, count = items.length; i < count; ++i) {
				it = Inventory.getUI().getItemByIndex(items[i].index);

				const condition =
					InventoryVersion !== 'InventoryV0'
						? it && (!Inventory.getUI().npcsalelock || it.PlaceETCTab < 1)
						: it;

				if (condition) {
					item = Object.assign({}, it);
					item.price = items[i].price;
					item.overchargeprice = items[i].overchargeprice;
					item.count = 'count' in item ? item.count : 1;

					out = Object.assign({}, item);
					out.count = 0;

					addItem(content, item);

					_input[item.index] = item;
					_output[item.index] = out;
				}
			}
			break;
		}
	}
};

NpcStore.setPriceLimit = function setPriceLimit(price) {
	const prettyPrice = prettyZeny(price);
	const text = DB.getMessage(1735);
	const result = text.replace('%s', prettyPrice);
	const root = NpcStore.getRoot();
	const priceLimit = root.querySelector('.priceLimit');
	if (priceLimit) {
		priceLimit.textContent = result;
	}
};

/**
 * Submit data to send items
 */
NpcStore.submit = function submit() {
	const output = [];
	const count = _output.length;

	for (let i = 0; i < count; ++i) {
		if (_output[i] && _output[i].count) {
			output.push(_output[i]);
		}
	}

	NpcStore.onSubmit(output);

	const root = NpcStore.getRoot();
	const outputContent = root.querySelector('.OutputWindow .content');
	if (outputContent) {
		outputContent.innerHTML = '';
	}
	const resultP = root.querySelector('.totalP .resultP');
	if (resultP) {
		resultP.textContent = '0';
	}

	for (let i = 0; i < count; ++i) {
		if (_output[i] && _output[i].count) {
			_output[i].count = 0;
		}
	}
};

/**
 * Calculate the cost of all items in the output box
 *
 * @return {number}
 */
NpcStore.calculateCost = function calculateCost() {
	let i, total;

	total = 0;
	const count = _output.length;

	for (i = 0; i < count; ++i) {
		if (_output[i]) {
			total += (_output[i].discountprice || _output[i].overchargeprice || _output[i].price) * _output[i].count;
		}
	}

	const root = NpcStore.getRoot();
	root.querySelectorAll('.total .result').forEach(r => {
		r.textContent = prettyZeny(total);
	});
	root.querySelectorAll('.totalP .resultP').forEach(r => {
		r.textContent = prettyZeny(total);
	});

	if (_type === NpcStore.Type.BARTER_MARKET) {
		_hideAll(root, '.total');
	}

	return total;
};

/**
 * Calculate the total weight of all items in the output box
 *
 * @return {number}
 */
NpcStore.calculateWeight = function calculateWeight() {
	let totalWeight = 0;

	_output.forEach(item => {
		if (item && item.count > 0 && item.total_weight) {
			totalWeight += item.total_weight;
		}
	});

	return totalWeight;
};

/**
 * Prettify zeny : 1000000 -> 1,000,000
 *
 * @param {number} zeny
 * @param {boolean} use color
 * @return {string}
 */
function prettyZeny(val, useStyle) {
	const list = val.toString().split('');
	let i;
	const count = list.length;
	let str = '';

	for (i = 0; i < count; i++) {
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
 * Add item to the list
 *
 * @param {Element} content element
 * @param {Item} item info
 */
function addItem(content, item) {
	const it = DB.getItemInfo(item.ITID);
	const currencyit = DB.getItemInfo(item.currencyITID);
	const element = content.querySelector(`.item[data-index="${item.index}"]`);
	let price;
	let amountText;

	let currency_item;
	if (_type === NpcStore.Type.BARTER_MARKET) {
		currency_item = { ...item };
		currency_item.ITID = item.currencyITID;
	}

	if (item.count === 0) {
		if (element) {
			element.remove();
		}
		return;
	}

	if (element) {
		amountText =
			_type === NpcStore.Type.BUYING_STORE && !content.classList.contains('contentAvailable') ? ' ea.' : '';
		const amountEl = element.querySelector('.amount');
		if (amountEl) {
			amountEl.textContent = isFinite(item.count) ? item.count + amountText : '';
		}
		return;
	}

	if (
		!content.classList.contains('contentAvailable') &&
		_type !== NpcStore.Type.BARTER_MARKET &&
		_type !== NpcStore.Type.BARTER_MARKET_EXTENDED
	) {
		price = prettyZeny(item.price, _type === NpcStore.Type.VENDING_STORE || _type === NpcStore.Type.BUYING_STORE);

		if ('discountprice' in item && item.price !== item.discountprice) {
			price += ' -> ' + prettyZeny(item.discountprice);
		} else if ('overchargeprice' in item && item.price !== item.overchargeprice) {
			price += ' -> ' + prettyZeny(item.overchargeprice);
		}

		const buyingClass = _type === NpcStore.Type.BUYING_STORE ? ' amountBuying' : '';
		amountText = _type === NpcStore.Type.BUYING_STORE ? ' ea.' : '';
		const html =
			`<div class="item" draggable="true" data-index="${item.index}">` +
			`<div class="icon"></div>` +
			`<div class="amount${buyingClass}">` +
			(isFinite(item.count) ? item.count : _type === NpcStore.Type.BUYING_STORE ? 0 : '') +
			amountText +
			`</div>` +
			`<div class="name">${_escapeHTML(DB.getItemName(item))}</div>` +
			`<div class="price">${price}</div>` +
			`<div class="unity">Z</div>` +
			`</div>`;
		content.insertAdjacentHTML('beforeend', html);
	} else if (_type === NpcStore.Type.BARTER_MARKET) {
		const html =
			`<div class="item" draggable="true" data-index="${item.index}"` +
			` data-weight="${item.weight}"` +
			` data-location="${item.location}"` +
			` data-viewSprite="${item.viewSprite}">` +
			`<div class="icon"></div>` +
			`<div class="amount">${isFinite(item.count) ? item.count : ''}</div>` +
			`<div class="name">${_escapeHTML(DB.getItemName(item))}</div>` +
			`<div class="currency_icon" data-item="${item.currencyITID}"></div>` +
			`<div class="currency_amount">${item.currencyamount}</div>` +
			`<div class="currency_nameOverlay">${_escapeHTML(DB.getItemName(currency_item))} ${item.currencyamount} ea</div>` +
			`</div>`;
		content.insertAdjacentHTML('beforeend', html);
	} else if (_type === NpcStore.Type.BARTER_MARKET_EXTENDED) {
		let currencySlotsHTML = '';
		let currencyOverlay = '';
		if (item.currencyList && item.currencyList.length > 0) {
			for (let i = 0; i < item.currencyList.length; i++) {
				const currency = item.currencyList[i];
				const currencyItem = DB.getItemInfo(currency.ITID);

				currencySlotsHTML +=
					`<div class="currency_slot" data-item="${currency.ITID}">` +
					`<div class="expanded_currency_holder">` +
					`<div class="expanded_currency_icon"></div>` +
					`</div>` +
					`<div class="expanded_currency_amount">${currency.amount}</div>` +
					(currency.refine_level > 0
						? `<div class="expanded_currency_refinelvl">+${currency.refine_level}</div>`
						: '') +
					`</div>`;

				currencyOverlay += `${_escapeHTML(currencyItem.identifiedDisplayName)} ${currency.amount} ea<br>`;
			}
		}
		const html =
			`<div class="item expanded-barter" draggable="true" data-index="${item.index}"` +
			` data-weight="${item.weight}"` +
			` data-location="${item.location}"` +
			` data-viewSprite="${item.viewSprite}">` +
			`<div class="expanded_currency_holder">` +
			`<div class="icon"></div>` +
			`</div>` +
			`<div class="amount">${isFinite(item.count) ? item.count : ''}</div>` +
			`<div class="name">${_escapeHTML(DB.getItemName(item))}</div>` +
			`<div class="currency_section">${currencySlotsHTML}</div>` +
			`<div class="expanded_price">${item.price}z</div>` +
			`<div class="expanded_currency_nameOverlay">${currencyOverlay}</div>` +
			`</div>`;
		content.insertAdjacentHTML('beforeend', html);

		if (item.currencyList && item.currencyList.length > 0) {
			for (let i = 0; i < item.currencyList.length; i++) {
				const currency = item.currencyList[i];
				const currencyItem = DB.getItemInfo(currency.ITID);

				Client.loadFile(
					DB.INTERFACE_PATH + 'item/' + currencyItem.identifiedResourceName + '.bmp',
					function (data) {
						const icons = content.querySelectorAll(
							`.currency_slot[data-item="${currency.ITID}"] .expanded_currency_icon`
						);
						icons.forEach(icon => {
							icon.style.backgroundImage = `url(${data})`;
						});
					}
				);
			}
		}
	} else {
		const html =
			`<div class="item itemAvailable" draggable="true" data-index="${item.index}">` +
			`<div class="icon"></div>` +
			`<div class="amount">${isFinite(item.count) ? item.count : ''}</div>` +
			`<div class="nameOverlay">${_escapeHTML(DB.getItemName(item))}</div>` +
			`</div>`;
		content.insertAdjacentHTML('beforeend', html);
	}

	Client.loadFile(
		DB.INTERFACE_PATH +
			'item/' +
			(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
			'.bmp',
		function (data) {
			const icons = content.querySelectorAll(`.item[data-index="${item.index}"] .icon`);
			icons.forEach(icon => {
				icon.style.backgroundImage = `url(${data})`;
			});
		}
	);

	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/itemwin_mid.bmp', function (data) {
		content.querySelectorAll('.expanded_currency_holder').forEach(holder => {
			holder.style.backgroundImage = `url(${data})`;
		});
	});

	Client.loadFile(
		DB.INTERFACE_PATH +
			'item/' +
			(item.IsIdentified ? currencyit.identifiedResourceName : currencyit.unidentifiedResourceName) +
			'.bmp',
		function (data) {
			const icons = content.querySelectorAll(`.item[data-index="${item.index}"] .currency_icon`);
			icons.forEach(icon => {
				icon.style.backgroundImage = `url(${data})`;
			});
		}
	);
}

/**
 * Resize the content
 *
 * @param {Element} content
 * @param {number} height
 */
function resize(content, height) {
	height = Math.min(Math.max(height, 2), 9);
	content.style.height = `${height * 32}px`;
}

/**
 * Resizing window
 *
 * @param {Element} ui element
 */
function onResize(ui) {
	const top = ui.offsetTop;
	const content = ui.querySelector('.content');
	let lastHeight = 0;

	function resizing() {
		const extraY = 31 + 19 - 30;
		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);

		h = Math.min(Math.max(h, 2), 9);
		if (h === lastHeight) {
			return;
		}

		resize(content, h);
		lastHeight = h;
	}

	const interval = setInterval(resizing, 30);

	function onMouseUp(event) {
		if (event.which === 1) {
			clearInterval(interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	}

	window.addEventListener('mouseup', onMouseUp);
}

/**
 * Transfer item from input to output (or the inverse)
 */
const transferItem = (function () {
	const tmpItem = {
		ITID: 0,
		count: 0,
		price: 0,
		index: 0
	};

	const updateTmpItem = (inputItem, outputItem) => {
		tmpItem.ITID = inputItem.ITID;
		tmpItem.count = inputItem.count - outputItem.count;
		tmpItem.price = inputItem.price;
		tmpItem.index = inputItem.index;
	};

	return function (fromContent, toContent, isAdding, index, count) {
		const inputItem = _input[index];
		const outputItem = _output[index];
		const root = NpcStore.getRoot();

		if (isAdding) {
			if (
				(_type === NpcStore.Type.BUY ||
					_type === NpcStore.Type.VENDING_STORE ||
					_type === NpcStore.Type.MARKETSHOP) &&
				NpcStore.calculateCost() + (inputItem.discountprice || inputItem.price) * count > Session.zeny
			) {
				ChatBox.addText(DB.getMessage(55), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
				return;
			}

			const originalCount = outputItem.count;
			outputItem.count = Math.min(outputItem.count + count, inputItem.count);

			if (_type === NpcStore.Type.BARTER_MARKET) {
				const inputCurrency = root.querySelector(`.InputWindow .item[data-index="${index}"]`);
				const inputCurrencyDiv = root.querySelector(
					`.InputWindow .item[data-index="${index}"] .currency_amount`
				);
				const currencyItemWeight = parseInt(inputCurrency.getAttribute('data-weight'), 10);
				const currencyAmount = parseInt(inputCurrencyDiv.textContent, 10);
				const additionalWeight = currencyItemWeight * (outputItem.count - originalCount);
				const expectedWeight = Session.Entity.weight + NpcStore.calculateWeight() + additionalWeight;

				if (expectedWeight > Session.Entity.max_weight) {
					ChatBox.addText(DB.getMessage(56), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
					outputItem.count -= count;
					return;
				}

				outputItem.total_weight = currencyItemWeight * outputItem.count;

				updateTmpItem(inputItem, outputItem);
				addItem(fromContent, tmpItem);
				addItem(toContent, outputItem);

				const outputCurrencyDiv = root.querySelector(
					`.OutputWindow .item[data-index="${index}"] .currency_amount`
				);
				const currencyItemDiv = root.querySelector(`.OutputWindow .item[data-index="${index}"] .currency_icon`);
				const currencyItem = parseInt(currencyItemDiv.getAttribute('data-item'), 10);
				const currencyTotal = currencyAmount * outputItem.count;

				outputCurrencyDiv.textContent = currencyTotal;
				outputItem.shopIndex = index;
				outputItem.matcurrency = currencyItem;
				outputItem.matcurrencyamount = currencyTotal;
			} else {
				updateTmpItem(inputItem, outputItem);
				addItem(fromContent, tmpItem);
				addItem(toContent, outputItem);
			}

			if (typeof outputItem.maxCount !== 'undefined' && outputItem.count > outputItem.maxCount) {
				const text = DB.getMessage(1739).replace('%d', outputItem.maxCount);
				ChatBox.addText(text, ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			}
		} else {
			count = Math.min(count, outputItem.count);
			if (!count) {
				return;
			}

			outputItem.count -= count;

			if (_type === NpcStore.Type.BARTER_MARKET) {
				const inputCurrency = root.querySelector(`.InputWindow .item[data-index="${index}"]`);
				const currencyItemWeight = parseInt(inputCurrency.getAttribute('data-weight'), 10);
				outputItem.total_weight = currencyItemWeight * outputItem.count;

				const inputCurrencyDiv = root.querySelector(
					`.InputWindow .item[data-index="${index}"] .currency_amount`
				);
				const outputCurrencyDiv = root.querySelector(
					`.OutputWindow .item[data-index="${index}"] .currency_amount`
				);
				const currencyAmount = parseInt(inputCurrencyDiv.textContent, 10);
				const currencyTotal = currencyAmount * outputItem.count;

				outputCurrencyDiv.textContent = currencyTotal;
				outputItem.matcurrencyamount = currencyTotal;
			}

			updateTmpItem(inputItem, outputItem);
			addItem(fromContent, outputItem);
			addItem(toContent, tmpItem);
		}

		NpcStore.calculateCost();
		NpcStore.calculateWeight();
	};
})();

/**
 * Request move item from box to another
 */
function requestMoveItem(index, fromContent, toContent, isAdding, transferAll = false) {
	let count;
	const item = isAdding ? _input[index] : _output[index];
	const isStackable =
		item.type !== ItemType.WEAPON &&
		item.type !== ItemType.EQUIP &&
		item.type !== ItemType.PETEGG &&
		item.type !== ItemType.PETEQUIP;

	if (isAdding) {
		count = isFinite(_input[index].count) ? _input[index].count : 1;
	} else {
		count = _output[index].count;
	}

	if ((_type === NpcStore.Type.BUY || _type === NpcStore.Type.VENDING_STORE) && !isStackable && isAdding) {
		if (toContent.querySelector(`.item[data-index="${item.index}"]`)) {
			return false;
		}
	}

	if (transferAll || item.count === 1 || (_type === NpcStore.Type.SELL && _preferences.select_all) || !isStackable) {
		transferItem(fromContent, toContent, isAdding, index, isFinite(item.count) ? item.count : 1);
		return false;
	}

	InputBox.append();
	InputBox.setType('number', false, count);
	InputBox.onSubmitRequest = function (_count) {
		InputBox.remove();
		if (_count > 0) {
			transferItem(fromContent, toContent, isAdding, index, _count);
		}
	};
}

function transferSellItemStack(index) {
	if (_type !== NpcStore.Type.SELL || !_input[index]) {
		return false;
	}

	const root = NpcStore.getRoot();
	requestMoveItem(
		index,
		root.querySelector('.InputWindow .content'),
		root.querySelector('.OutputWindow .content'),
		true,
		true
	);
	return true;
}

NpcStore.inventoryTransferPriority = InventoryItemTransferPriority.NPC_STORE;
NpcStore.receiveInventoryItemStack = function receiveInventoryItemStack(item) {
	return item ? transferSellItemStack(item.index) : false;
};

/**
 * Drop an input in the InputWindow or OutputWindow
 */
function onDrop(event) {
	let data;

	event.preventDefault();
	event.stopImmediatePropagation();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		return false;
	}

	if (data.type !== 'item' || data.from !== 'NpcStore' || data.container === this.className) {
		return false;
	}

	const root = NpcStore.getRoot();

	requestMoveItem(
		data.index,
		root.querySelector('.' + data.container + ' .content'),
		this.querySelector('.content'),
		this.className.indexOf('OutputWindow') !== -1
	);

	return false;
}

/**
 * Get informations about an item
 */
function onItemInfo(event) {
	const index = parseInt(this.parentNode.getAttribute('data-index'), 10);
	const item = _input[index];

	event.stopImmediatePropagation();

	if (!item) {
		return false;
	}

	const inputWindow = NpcStore.getRoot().querySelector('.InputWindow');
	if (_type === NpcStore.Type.SELL && event.altKey && event.which === 3 && inputWindow.contains(this)) {
		transferSellItemStack(index);
		return false;
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return false;
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
	return false;
}

/**
 * Select an item, put it on the other box
 */
function onItemSelected() {
	let from, to;

	if ((_type === NpcStore.Type.BUY || _type === NpcStore.Type.VENDING_STORE) && !Session.isTouchDevice) {
		return;
	}

	const root = NpcStore.getRoot();
	const input = root.querySelector('.InputWindow');

	if (input.contains(this)) {
		from = input;
		to = root.querySelector('.OutputWindow');
	} else {
		from = root.querySelector('.OutputWindow');
		to = input;
	}

	requestMoveItem(
		parseInt(this.getAttribute('data-index'), 10),
		from.querySelector('.content'),
		to.querySelector('.content'),
		from === input
	);
}

/**
 * Focus an item
 */
function onItemFocus() {
	const root = NpcStore.getRoot();
	root.querySelectorAll('.item.selected').forEach(el => el.classList.remove('selected'));
	this.classList.add('selected');
}

/**
 * Update scroll by block (32px)
 */
function onScroll(event) {
	let delta;

	if (event.deltaY) {
		delta = event.deltaY > 0 ? -1 : 1;
	} else if (event.wheelDelta) {
		delta = event.wheelDelta / 120;
	} else if (event.detail) {
		delta = -event.detail;
	}

	this.scrollTop = Math.floor(this.scrollTop / 32) * 32 - delta * 32;
	event.preventDefault();
}

/**
 * Start dragging an item
 */
function onDragStart(event) {
	const root = NpcStore.getRoot();
	const InputWindow = root.querySelector('.InputWindow');
	const OutputWindow = root.querySelector('.OutputWindow');
	const AvailableItemsWindow = root.querySelector('.AvailableItemsWindow');

	let container;
	if (InputWindow.contains(this)) {
		container = InputWindow.className;
	} else if (AvailableItemsWindow.contains(this)) {
		container = AvailableItemsWindow.className;
	} else {
		container = OutputWindow.className;
	}

	const img = new Image();
	const bgImage = this.firstChild.style.backgroundImage;
	const match = bgImage.match(/url\(["']?([^"')]+)["']?\)/);
	if (match) {
		img.decoding = 'async';
		img.src = match[1];
		event.dataTransfer.setDragImage(img, 12, 12);
	}

	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'NpcStore',
				container: container,
				index: this.getAttribute('data-index')
			})
		)
	);
}

/**
 * Option to automatically buy/sell all items instead of specify the amount
 */
function onToggleSelectAmount() {
	_preferences.select_all = !_preferences.select_all;

	Client.loadFile(
		DB.INTERFACE_PATH + 'checkbox_' + (_preferences.select_all ? 1 : 0) + '.bmp',
		function (data) {
			this.style.backgroundImage = `url(${data})`;
		}.bind(this)
	);
}

/**
 * Handles the packet to send to the server when closing stores
 */
NpcStore.closeStore = function () {
	NpcStore.remove();
	const root = NpcStore.getRoot();
	root.querySelectorAll('.total').forEach(el => {
		el.style.display = '';
	});
};

/**
 * Handles packet to close store based on the store type
 */
NpcStore.StoreClosePacket = function (type) {
	const root = NpcStore.getRoot();
	const inputWindow = root.querySelector('.InputWindow');
	const outputWindow = root.querySelector('.OutputWindow');

	InputBox.remove();

	let pkt;

	if (PACKETVER.value < 20131223) {
		if (type === NpcStore.Type.SELL) {
			pkt = new PACKET.CZ.PC_SELL_ITEMLIST();
		} else {
			pkt = new PACKET.CZ.PC_PURCHASE_ITEMLIST();
		}
	} else {
		switch (type) {
			case NpcStore.Type.MARKETSHOP:
				pkt = new PACKET.CZ.NPC_MARKET_CLOSE();
				inputWindow.style.display = '';
				outputWindow.style.display = '';
				break;
			case NpcStore.Type.BARTER_MARKET:
				pkt = new PACKET.CZ.NPC_BARTER_MARKET_CLOSE();
				break;
			case NpcStore.Type.BARTER_MARKET_EXTENDED:
				pkt = new PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_CLOSE();
				break;
			default:
				pkt = new PACKET.CZ.NPC_TRADE_QUIT();
				break;
			case NpcStore.Type.VENDING_STORE:
			case NpcStore.Type.BUYING_STORE:
				_closePacketSent = true;
				return;
		}
	}
	_closePacketSent = true;
	Network.sendPacket(pkt);
};

/**
 * Returns Npc Store Type
 */
NpcStore.getCurrentType = function () {
	return _type;
};

/**
 * Returns the current preference for NPCStore Type
 */
function getCurrentPref() {
	if (!_preferences[_type]) {
		_preferences[_type] = JSON.parse(JSON.stringify(initialPreferences[_type] || initialPreferences.DEFAULT));
	}
	return _preferences[_type];
}

/**
 * Update Marketshop Result UI
 */
NpcStore.onMarketShopResultUI = function (itemList) {
	const root = NpcStore.getRoot();
	const InputWindow = root.querySelector('.InputWindow');
	const OutputWindow = root.querySelector('.OutputWindow');
	const OutputWindowContent = OutputWindow.querySelector('.content');
	const resultUI = root.querySelector('.PurchaseResult');
	const resultUIContent = resultUI.querySelector('.content');

	InputWindow.style.display = 'none';
	OutputWindow.style.display = 'none';
	resultUI.style.display = 'block';
	resultUIContent.innerHTML = '';

	if (!itemList || itemList.length === 0) {
		return;
	}

	resultUIContent.innerHTML = OutputWindowContent.innerHTML;

	resultUI.querySelector('.resize').addEventListener('mousedown', () => {
		onResize(resultUI);
	});
};

/**
 * Exports
 */
NpcStore.onSubmit = function onSubmit(/* itemList */) {};

NpcStore.setClosePacketSent = function (bool) {
	_closePacketSent = bool;
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(NpcStore);
