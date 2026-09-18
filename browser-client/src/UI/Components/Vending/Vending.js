/**
 * UI/Components/Vending/Vending.js
 *
 * Vending / Buying store setup windows
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import VendingModelMessage from 'UI/Components/Vending/VendingModelMessage/VendingModelMessage.js';
import htmlText from './Vending.html?raw';
import cssText from './Vending.css?raw';
import Renderer from 'Renderer/Renderer.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import BasicInfo from 'UI/Components/BasicInfo/BasicInfo.js';

const Vending = new GUIComponent('Vending', cssText);

Vending.render = () => htmlText;

Vending.isOpen = false;
Vending.Type = {
	VENDING_STORE: 0,
	BUYING_STORE: 1
};

/**
 * @var {Preferences}
 */
const _preferences = Preferences.get(
	'Vending',
	{
		inputWindow: {
			x: 100,
			y: 100,
			height: 2
		},
		outputWindow: {
			x: 100 + 280 + 10,
			y: 100 + 7 * 32 - 2 * 32,
			height: 5
		},
		select_all: false
	},
	1.0
);

/**
 * @var {Array} item list
 */
const _input = [];

/**
 * @var {Array} output list
 */
const _output = [];
let _slots = 0;

/**
 * @var {number} type (buy/sell)
 */
let _type;

function escapeHtml(text) {
	const div = document.createElement('div');
	div.appendChild(document.createTextNode(text));
	return div.innerHTML;
}

function isItemStackable(item) {
	return (
		item.type !== ItemType.WEAPON &&
		item.type !== ItemType.ARMOR &&
		item.type !== ItemType.SHADOWGEAR &&
		item.type !== ItemType.PETEGG &&
		item.type !== ItemType.PETARMOR
	);
}

Vending.captureKeyEvents = true;

Vending.init = function init() {
	const root = Vending.getRoot();

	const sellBtn = root.querySelector('.btn.sell');
	if (sellBtn) {
		sellBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		sellBtn.addEventListener('click', e => {
			e.stopImmediatePropagation();
			Vending.onSubmit();
		});
	}

	const cancelBtn = root.querySelector('.btn.cancel');
	if (cancelBtn) {
		cancelBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		cancelBtn.addEventListener('click', e => {
			e.stopImmediatePropagation();
			let pkt;
			if (_type === Vending.Type.VENDING_STORE) {
				pkt = new PACKET.CZ.REQ_OPENSTORE2();
			} else {
				pkt = new PACKET.CZ.REQ_OPEN_BUYING_STORE();
			}
			submitNetworkPacket(pkt);
			Vending.onRemove();
		});
	}

	const extendBtn = root.querySelector('.InputWindow .footer .extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', onResizeInput);
	}

	// Delegated event handlers on content areas
	const contents = root.querySelectorAll('.content');
	contents.forEach(content => {
		content.addEventListener('contextmenu', e => {
			const icon = e.target.closest('.icon');
			if (icon) {
				onItemInfo.call(icon, e);
			}
		});

		content.addEventListener('wheel', e => {
			onScroll.call(content, e);
		});

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

		content.addEventListener('dragstart', e => {
			const item = e.target.closest('.item');
			if (item) {
				onDragStart.call(item, e);
			}
		});

		content.addEventListener('dragend', e => {
			const item = e.target.closest('.item');
			if (item) {
				delete window._OBJ_DRAG_;
			}
		});
	});

	// Drop items on InputWindow and OutputWindow
	const inputWin = root.querySelector('.InputWindow');
	const outputWin = root.querySelector('.OutputWindow');

	[inputWin, outputWin].forEach(win => {
		if (!win) {
			return;
		}

		win.addEventListener('drop', e => {
			onDrop.call(win, e);
		});

		win.addEventListener('dragover', e => {
			e.stopImmediatePropagation();
			e.preventDefault();
		});

		win.addEventListener('mousedown', () => {
			Vending.focus();
		});
	});

	// Make sub-windows independently draggable
	if (inputWin) {
		this.draggable.call(
			{ _host: inputWin, _shadow: null, _container: inputWin, magnet: {}, needFocus: false, manager: null },
			inputWin.querySelector('.titlebar')
		);
	}
	if (outputWin) {
		this.draggable.call(
			{ _host: outputWin, _shadow: null, _container: outputWin, magnet: {}, needFocus: false, manager: null },
			outputWin.querySelector('.titlebar')
		);
	}

	Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/itemwin_mid.bmp`, data => {
		Vending.itemBg = data;
	});
};

Vending.onAppend = function onAppend() {
	const root = Vending.getRoot();
	const inputWin = root.querySelector('.InputWindow');
	const outputWin = root.querySelector('.OutputWindow');
	const inputContent = inputWin.querySelector('.content');
	const outputContent = outputWin.querySelector('.content');

	inputWin.style.top = `${Math.min(Math.max(0, _preferences.inputWindow.y), Renderer.height - inputContent.offsetHeight)}px`;
	inputWin.style.left = `${Math.min(Math.max(0, _preferences.inputWindow.x), Renderer.width - inputContent.offsetWidth)}px`;

	outputWin.style.top = `${Math.min(Math.max(0, _preferences.outputWindow.y), Renderer.height - outputContent.offsetHeight)}px`;
	outputWin.style.left = `${Math.min(Math.max(0, _preferences.outputWindow.x), Renderer.width - outputContent.offsetWidth)}px`;

	resize(inputContent, _preferences.inputWindow.height);
	resize(outputContent, _preferences.outputWindow.height);

	this._host.style.display = 'none';
};

Vending.setType = function setType(type) {
	const root = Vending.getRoot();

	const winBuyEls = root.querySelectorAll('.WinBuy');
	const winSellEls = root.querySelectorAll('.WinSell');

	switch (type) {
		case Vending.Type.VENDING_STORE:
			winBuyEls.forEach(el => {
				el.style.display = 'none';
			});
			winSellEls.forEach(el => {
				el.style.display = '';
			});
			break;

		case Vending.Type.BUYING_STORE:
			winSellEls.forEach(el => {
				el.style.display = 'none';
			});
			winBuyEls.forEach(el => {
				el.style.display = '';
			});
			root.querySelector('.zenySpan').textContent = prettyZeny(Session.zeny);
			root.querySelector('.weightSpan').textContent =
				`${BasicInfo.getUI().weight}/${BasicInfo.getUI().weight_max}`;
			root.querySelector('.limitZeny').value = '0';
			break;
	}

	_type = type;
};

function resize(content, height) {
	height = Math.min(Math.max(height, 2), 9);
	content.style.height = `${height * 32}px`;
}

Vending.onRemove = function onRemove() {
	VendingModelMessage.onRemove();

	const root = Vending.getRoot();
	const inputWin = root.querySelector('.InputWindow');
	const outputWin = root.querySelector('.OutputWindow');

	_input.length = 0;
	_output.length = 0;

	_preferences.inputWindow.x = parseInt(inputWin.style.left, 10);
	_preferences.inputWindow.y = parseInt(inputWin.style.top, 10);
	_preferences.inputWindow.height = (inputWin.querySelector('.content').offsetHeight / 32) | 0;

	_preferences.outputWindow.x = parseInt(outputWin.style.left, 10);
	_preferences.outputWindow.y = parseInt(outputWin.style.top, 10);
	_preferences.outputWindow.height = (outputWin.querySelector('.content').offsetHeight / 32) | 0;

	_preferences.save();

	root.querySelectorAll('.content').forEach(el => {
		el.innerHTML = '';
	});

	this._host.style.display = 'none';

	Vending.isOpen = false;
};

Vending.onKeyDown = function onKeyDown(event) {
	if (this.isEditableFocused()) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			this.remove();
			event.stopImmediatePropagation();
			return false;
		}
		event.stopImmediatePropagation();
		return true;
	}

	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
	}

	return true;
};

Vending.setList = function setList(items) {
	const root = Vending.getRoot();

	root.querySelectorAll('.content').forEach(el => {
		el.innerHTML = '';
	});

	_input.length = 0;
	_output.length = 0;

	const content = root.querySelector('.InputWindow .content');

	for (let i = 0, count = items.length; i < count; ++i) {
		if (!('index' in items[i])) {
			items[i].index = i;
		}

		items[i].IsStackable = isItemStackable(items[i]);

		if (!Object.prototype.hasOwnProperty.call(items[i], 'count')) {
			items[i].count = 1;
		}

		items[i].total = items[i].count;

		const out = Object.assign({}, items[i]);
		out.count = 0;

		addItem(content, items[i], true);

		_input[items[i].index] = items[i];
		_output[items[i].index] = out;
	}
};

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

function addItem(content, item, isinput) {
	const it = DB.getItemInfo(item.ITID);
	const element = content.querySelector(`.item[data-index="${item.index}"]`);
	const textPrice = DB.getMessage(1721);

	if (item.count === 0) {
		if (element) {
			const parent = element.closest('.item-container');
			if (parent) {
				parent.remove();
			} else {
				element.remove();
			}
		}
		return;
	}

	if (element) {
		const amountEl = element.querySelector('.amount');
		if (amountEl) {
			amountEl.textContent = item.IsStackable ? item.count : '';
		}
		return;
	}

	let itemObj;

	if (isinput) {
		itemObj = document.createElement('div');
		itemObj.className = 'item input';
		itemObj.draggable = true;
		itemObj.dataset.index = item.index;
		itemObj.innerHTML =
			'<div class="icon"></div>' + `<div class="amount">${item.IsStackable ? item.count : ''}</div>`;
	} else {
		const price = prettyZeny(item.price, true);
		const container = document.createElement('div');
		container.className = 'item-container';

		const amountText = _type === Vending.Type.BUYING_STORE ? item.total : item.IsStackable ? item.count : '';
		const eaHtml = _type === Vending.Type.BUYING_STORE ? `<div class="amount_">${item.count} ea</div>` : '';

		container.innerHTML =
			`<div class="item output" draggable="true" data-index="${item.index}">` +
			'<div class="icon"></div>' +
			`<div class="amount">${amountText}</div>` +
			eaHtml +
			`<div class="name">${escapeHtml(DB.getItemName(item))}</div>` +
			`<div class="price">${textPrice} ${price}</div>` +
			'</div>';

		itemObj = container;

		if (_type === Vending.Type.BUYING_STORE) {
			const root = Vending.getRoot();
			const limitInput = root.querySelector('.limitZeny');
			let limit = parseInt(limitInput.value, 10);
			limit += item.count * item.price;
			limitInput.value = limit;
		}
	}

	const actualItem = itemObj.classList.contains('item') ? itemObj : itemObj.querySelector('.item');

	if (item.IsDamaged) {
		actualItem.style.backgroundImage = `url("${Vending.itemBg}")`;
		actualItem.classList.add('damaged');
	}

	content.appendChild(itemObj);

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = content.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);
}

const transferItem = (() => {
	let tmpItem = {};

	return (fromContent, toContent, isAdding, index, count) => {
		if (isAdding) {
			if (!_input[index].IsIdentified) {
				VendingModelMessage.setInit(603);
				return;
			}

			_output[index].count =
				_type === Vending.Type.BUYING_STORE
					? count
					: Math.min(_output[index].count + count, _input[index].count);
			tmpItem = Object.assign({}, _input[index]);

			tmpItem.count = _type === Vending.Type.BUYING_STORE ? 0 : _input[index].count - _output[index].count;

			addItem(fromContent, tmpItem, true);
			addItem(toContent, _output[index], false);
		} else {
			count = Math.min(count, _output[index].count);
			if (!count) {
				return;
			}

			_output[index].count = _type === Vending.Type.BUYING_STORE ? 0 : _output[index].count - count;

			tmpItem = Object.assign({}, _input[index]);
			tmpItem.count =
				_type === Vending.Type.BUYING_STORE ? _input[index].total : _input[index].count + _output[index].count;

			addItem(fromContent, _output[index], false);
			addItem(toContent, tmpItem, true);
		}
	};
})();

function requestMoveItem(index, fromContent, toContent, isAdding) {
	let count;
	const item_price = 0;

	const item = isAdding ? _input[index] : _output[index];

	if (isAdding) {
		if (!(countSlotsUsed() < _slots)) {
			return false;
		}
		count = _input[index].count;
	} else {
		count = _output[index].count;
	}

	if ((item.count === 1 || !item.IsStackable) && _type === Vending.Type.VENDING_STORE) {
		if (isAdding) {
			InputBox.append();
			InputBox.setType('price', false, item_price);
			InputBox.onSubmitRequest = function (_item_price) {
				InputBox.remove();
				_output[index].price = _item_price;
				if (_item_price > 0) {
					transferItem(fromContent, toContent, isAdding, index, item.count);
				}
			};
		} else {
			transferItem(fromContent, toContent, isAdding, index, item.count);
		}
		return false;
	}

	InputBox.append();
	InputBox.setType('number', false, count);
	InputBox.onSubmitRequest = function (_count) {
		InputBox.remove();
		if (_count > 0) {
			if (_count >= 9999 && _type === Vending.Type.BUYING_STORE) {
				VendingModelMessage.setInit(1742);
				return;
			}

			if (item.count + _count > 9999 && _type === Vending.Type.BUYING_STORE) {
				VendingModelMessage.setInit(1728);
				return;
			}

			if (isAdding) {
				InputBox.append();
				InputBox.setType('price', false, item_price);
				InputBox.onSubmitRequest = function (_item_price) {
					InputBox.remove();
					_output[index].price = _item_price;
					if (_item_price > 0) {
						transferItem(fromContent, toContent, isAdding, index, _count);
					}
				};
			} else {
				transferItem(fromContent, toContent, isAdding, index, _count);
			}
		}
	};
}

function onDrop(event) {
	let data;

	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		return;
	}

	if (data.type !== 'item' || data.from !== 'Vending' || data.container === this.className) {
		return;
	}

	const root = Vending.getRoot();
	const fromContent = root.querySelector(`.${data.container} .content`);
	const toContent = this.querySelector('.content');

	requestMoveItem(data.index, fromContent, toContent, this.className === 'OutputWindow');
}

function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const index = parseInt(this.parentNode.getAttribute('data-index'), 10);
	const item = _input[index];

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

function onItemSelected() {
	if (_type === Vending.Type.BUY || _type === Vending.Type.VENDING_STORE) {
		return;
	}

	const root = Vending.getRoot();
	const inputWin = root.querySelector('.InputWindow');

	let from, to;
	if (inputWin.contains(this)) {
		from = inputWin;
		to = root.querySelector('.OutputWindow');
	} else {
		from = root.querySelector('.OutputWindow');
		to = inputWin;
	}

	requestMoveItem(
		parseInt(this.getAttribute('data-index'), 10),
		from.querySelector('.content'),
		to.querySelector('.content'),
		from === inputWin
	);
}

function onItemFocus() {
	const root = Vending.getRoot();
	root.querySelectorAll('.item.selected').forEach(el => el.classList.remove('selected'));
	this.classList.add('selected');
}

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
	event.preventDefault();
}

function onDragStart(event) {
	const root = Vending.getRoot();
	const inputWin = root.querySelector('.InputWindow');
	const outputWin = root.querySelector('.OutputWindow');

	const container = (inputWin.contains(this) ? inputWin : outputWin).className;
	const img = new Image();
	const url = this.firstChild.style.backgroundImage.match(/\(([^)]+)/)[1].replace(/"/g, '');
	img.decoding = 'async';
	img.src = url;

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'Vending',
				container: container,
				index: this.getAttribute('data-index')
			})
		)
	);
}

function onResizeInput() {
	const root = Vending.getRoot();
	const inputWin = root.querySelector('.InputWindow');
	const content = inputWin.querySelector('.container .content');
	const top = inputWin.offsetTop;
	let lastHeight = 0;

	function resizing() {
		const extraY = 31 + 19 - 30;

		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);

		h = Math.min(Math.max(h, 2), 6);

		if (h === lastHeight) {
			return;
		}

		resize(content, h);
		lastHeight = h;
	}

	const _Interval = setInterval(resizing, 30);

	const onMouseUp = event => {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	};
	window.addEventListener('mouseup', onMouseUp);
}

Vending.onVendingSkill = function onVendingSkill(pkt) {
	if (Vending.isOpen) {
		return;
	}

	_slots = pkt.itemcount;
	this.setList(CartItems.list);

	const root = Vending.getRoot();
	root.querySelector('.add_shop').style.height = `${32 * _slots}px`;
	root.querySelector('.shopname').value = '';
	this._host.style.display = '';
	this._fixPositionOverflow();

	Vending.isOpen = true;
};

Vending.onBuyingSkill = function onBuyingSkill(pkt) {
	if (Vending.isOpen) {
		return;
	}

	_slots = pkt.itemcount;
	const buyable = [];
	for (const key in Inventory.getUI().list) {
		const item = Inventory.getUI().list[key];
		if (isItemStackable(item) && DB.isBuyable(item.ITID)) {
			buyable.push(item);
		}
	}
	this.setList(buyable);

	const root = Vending.getRoot();
	root.querySelector('.add_shop').style.height = `${32 * _slots}px`;
	root.querySelector('.shopname').value = '';
	this._host.style.display = '';
	this._fixPositionOverflow();

	Vending.isOpen = true;
};

Vending.onClose = function onClose() {
	this._host.style.display = 'none';
	Vending.isOpen = false;
};

Vending.onSubmit = function onSubmit() {
	const output = [];
	const count = _output.length;

	const root = Vending.getRoot();
	const shopname = root.querySelector('.shopname').value;

	let limitZeny;
	let ctr = 0;

	for (let i = 0; i < count; ++i) {
		if (_output[i] && _output[i].count) {
			output.push(_output[i]);
			ctr++;
		}
	}

	if (ctr < 1) {
		VendingModelMessage.setInit(2494);
		return;
	}

	let pkt;
	if (_type === Vending.Type.VENDING_STORE) {
		pkt = new PACKET.CZ.REQ_OPENSTORE2();
	} else {
		pkt = new PACKET.CZ.REQ_OPEN_BUYING_STORE();
		limitZeny = parseInt(root.querySelector('.limitZeny').value, 10);
		if (limitZeny > Session.zeny) {
			VendingModelMessage.setInit(3683);
			return;
		}
		if (limitZeny <= 0) {
			VendingModelMessage.setInit(1730);
			return;
		}
		pkt.LimitZeny = limitZeny;
	}

	pkt.storeName = shopname;
	pkt.result = 1;
	pkt.storeList = output;

	if (!shopname) {
		VendingModelMessage.setInit(225);
		return;
	} else {
		this._shopname = shopname;
		submitNetworkPacket(pkt);
	}

	this.onRemove();
};

function countSlotsUsed() {
	let count = 0;
	_output.forEach(item => {
		if (item.count > 0) {
			count++;
		}
	});
	return count;
}

function submitNetworkPacket(pkt) {
	Network.sendPacket(pkt);
}

function onItemOver() {
	if (!this.classList.contains('input')) {
		return;
	}

	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item =
		_type === Vending.Type.VENDING_STORE ? CartItems.getItemByIndex(idx) : Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = Vending.getRoot();
	const overlay = root.querySelector('.overlay');

	overlay.style.display = '';
	overlay.style.top = `${this.offsetTop - 20}px`;
	overlay.style.left = `${this.offsetLeft - 10}px`;
	overlay.textContent = `${DB.getItemName(item)} ${item.count || 1} ea`;
}

function onItemOut() {
	const root = Vending.getRoot();
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

Vending.mouseMode = GUIComponent.MouseMode.STOP;

export default UIManager.addComponent(Vending);
