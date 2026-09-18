/**
 * UI/Components/Rodex/WriteRodex.js
 *
 * Chararacter WriteRodex
 *
 * @author Alisonrag
 *
 */

import DB from 'DB/DBManager.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Session from 'Engine/SessionStorage.js';
import MonsterTable from 'DB/Monsters/MonsterTable.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Client from 'Core/Client.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './WriteRodex.html?raw';
import cssText from './WriteRodex.css?raw';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';

/**
 * Create Component
 */
const WriteRodex = new GUIComponent('WriteRodex', cssText);
WriteRodex.list = [];
WriteRodex.receiver = null;
WriteRodex.tax = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'WriteRodex',
	{
		show: false
	},
	1.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return WriteRodex._shadow || WriteRodex._host;
}

/**
 * Render HTML
 */
WriteRodex.render = () => htmlText;

/**
 * Initialize Component
 */
WriteRodex.onAppend = function onAppend() {
	const root = _root();

	// Bind buttons
	root.querySelector('.right .close').addEventListener('click', onClickClose);
	root.querySelector('.send').addEventListener('click', onClickSend);
	root.querySelector('.value').addEventListener('input', WriteRodex.updateTax);

	const rodexTop = Rodex._host ? parseInt(Rodex._host.style.top, 10) || 0 : 0;
	const rodexLeft = Rodex._host ? parseInt(Rodex._host.style.left, 10) || 0 : 0;

	this._host.style.top = `${Math.min(Math.max(0, rodexTop) - 20, Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, rodexLeft) + 330, Renderer.width - this._host.offsetWidth)}px`;

	this.draggable(root.querySelector('.titlebar'));
};

WriteRodex.initData = function initData(pkt) {
	const root = _root();

	WriteRodex.receiver = null;
	WriteRodex.CharID = 0;
	WriteRodex.list = [];
	WriteRodex.tax = 0;

	const nameInput = root.querySelector('.name');
	nameInput.type = 'text';
	nameInput.value = pkt.receiveName;

	const validateBtn = root.querySelector('.validate-name');
	validateBtn.style.display = '';

	const baloon = root.querySelector('.baloon');
	baloon.style.display = 'none';

	root.querySelector('.title-text').value = DB.getMessage(3575);
	root.querySelector('.content-text').value = '';
	root.querySelector('.character-zeny').textContent = `${prettifyZeny(Session.zeny)} Zeny`;

	validateBtn.addEventListener('click', onClickValidateName);

	root.querySelector('.weigth-text').textContent = '0  2000';
	root.querySelector('.tax-text').textContent = '0';

	const valueInput = root.querySelector('.value');
	valueInput.value = '';
	valueInput.max = Session.zeny;

	root.querySelector('.item-list').innerHTML = '';

	const itemsEl = root.querySelector('.items');
	itemsEl.addEventListener('drop', onDrop);
	itemsEl.addEventListener('dragover', stopPropagation);

	this._host.style.display = '';
	this.focus();
};

WriteRodex.characterInfo = function characterInfo(pkt) {
	const root = _root();
	const text = `Lv${pkt.level}<br>${MonsterTable[pkt.Job]}<br>${pkt.CharID}`;

	root.querySelector('.validate-name').style.display = 'none';

	const baloon = root.querySelector('.baloon');
	baloon.innerHTML = text;
	baloon.style.display = '';

	const nameInput = root.querySelector('.name');
	nameInput.type = 'none';

	WriteRodex.receiver = pkt.name !== undefined ? pkt.name : nameInput.value;
	WriteRodex.CharID = pkt.CharID;
};

function onClickClose(e) {
	e.stopImmediatePropagation();
	const root = _root();

	WriteRodex.receiver = null;
	root.querySelector('.name').value = '';
	root.querySelector('.title-text').value = '';
	root.querySelector('.content-text').value = '';
	root.querySelector('.value').value = '';
	root.querySelector('.item-list').innerHTML = '';
	root.querySelector('.weigth-text').textContent = '0  2000';
	root.querySelector('.tax-text').textContent = '0';
	WriteRodex.requestCancelWriteRodex();
	WriteRodex.list = [];
	WriteRodex.CharID = 0;
	WriteRodex.tax = 0;
	WriteRodex._host.style.display = 'none';
}

function onClickSend(e) {
	e.stopImmediatePropagation();
	const root = _root();

	if (
		WriteRodex.receiver == null ||
		(typeof WriteRodex.receiver === 'string' && WriteRodex.receiver.trim().length === 0)
	) {
		ChatBox.addText(DB.getMessage(2611), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}
	const receiver = WriteRodex.receiver;
	const sender = Session.Entity.display.name;
	let zeny = parseInt(root.querySelector('.value').value, 10);
	zeny = isNaN(zeny) ? 0 : zeny;
	zeny = zeny < 0 ? 0 : zeny;

	if (WriteRodex.tax + zeny > Session.zeny) {
		ChatBox.addText(DB.getMessage(2643), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	const title =
		root
			.querySelector('.title-text')
			.value.replace(/^(\$|\%)/, '')
			.replace(/\t/g, '')
			.substring(0, 23) + String.fromCharCode(0);
	const body =
		root
			.querySelector('.content-text')
			.value.replace(/^(\$|\%)/, '')
			.replace(/\t/g, '')
			.substring(0, 499) + String.fromCharCode(0);
	const Titlelength = title.length;
	const Bodylength = body.length;
	const CharID = WriteRodex.CharID;
	WriteRodex.requestSendRodex(receiver, sender, zeny, Titlelength, Bodylength, CharID, title, body);
	WriteRodex.requestCancelWriteRodex();
}

/**
 * Search in a list for an item by its index
 *
 * @param {number} index
 * @returns {Item}
 */
WriteRodex.getItemByIndex = function getItemByIndex(index) {
	const list = WriteRodex.list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].index === index) {
			return list[i];
		}
	}

	return null;
};

WriteRodex.addItem = function addItem(item) {
	const root = _root();
	let object = WriteRodex.getItemByIndex(item.index);

	if (object) {
		object.count += item.count;
		const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
		if (countEl) {
			countEl.textContent = object.count;
		}
		return;
	}

	object = Object.assign({}, item);
	WriteRodex.list.push(object);

	const it = DB.getItemInfo(item.ITID);
	const content = root.querySelector('.items .item-list');

	content.insertAdjacentHTML(
		'beforeend',
		`<div class="item" data-index="${item.index}" draggable="true">` +
			'<div class="icon"></div>' +
			`<div class="amount"><span class="count">${item.count || 1}</span></div>` +
			'</div>'
	);

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = root.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);

	const itemDiv = root.querySelector(`.item[data-index="${item.index}"]`);
	if (itemDiv) {
		itemDiv.addEventListener('mouseover', onItemOver);
		itemDiv.addEventListener('mouseout', onItemOut);
		itemDiv.addEventListener('dragstart', onItemDragStart);
		itemDiv.addEventListener('dragend', onItemDragEnd);
		itemDiv.addEventListener('contextmenu', onItemInfo);
	}

	WriteRodex.updateWeight(item.weight);
	WriteRodex.updateTax();
};

/**
 * Remove item from WriteRodex
 *
 * @param {number} index in WriteRodex
 * @param {number} count
 */
WriteRodex.removeItem = function RemoveItem(index, count, weight) {
	const root = _root();
	const item = WriteRodex.getItemByIndex(index);

	if (!item || count <= 0) {
		return null;
	}

	if (item.count) {
		item.count -= count;

		if (item.count > 0) {
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = item.count;
			}
			return;
		}
	}

	WriteRodex.list.splice(WriteRodex.list.indexOf(item), 1);
	const itemEl = root.querySelector(`.item[data-index="${item.index}"]`);
	if (itemEl) {
		itemEl.remove();
	}
	WriteRodex.updateWeight(weight);
	WriteRodex.updateTax();
};

WriteRodex.updateWeight = function updateWeight(weight) {
	const root = _root();
	const el = root.querySelector('.weigth-text');
	if (el) {
		el.textContent = `${weight}  2000`;
	}
};

WriteRodex.updateTax = function updateTax() {
	const root = _root();
	const total_items = WriteRodex.list.length;
	let tax = total_items * 2500;
	let zeny = parseInt(root.querySelector('.value').value, 10);
	zeny = isNaN(zeny) ? 0 : zeny;
	zeny = zeny < 0 ? 0 : zeny;
	tax += parseInt(zeny * 0.02);
	const taxEl = root.querySelector('.tax-text');
	if (taxEl) {
		taxEl.textContent = tax;
	}
	WriteRodex.tax = tax;

	const valueEl = root.querySelector('.value');
	if (tax + zeny > Session.zeny) {
		taxEl.classList.add('red');
		valueEl.classList.add('red');
	} else {
		taxEl.classList.remove('red');
		valueEl.classList.remove('red');
	}
};

WriteRodex.close = function close() {
	WriteRodex._host.style.display = 'none';
};

function prettifyZeny(value) {
	const num = String(value);
	let i = 0;
	const len = num.length;
	let out = '';

	while (i < len) {
		out = num[len - i - 1] + out;
		if ((i + 1) % 3 === 0 && i + 1 !== len) {
			out = ',' + out;
		}
		++i;
	}

	return out;
}

function onClickValidateName(e) {
	e.stopImmediatePropagation();
	const root = _root();
	const name = root.querySelector('.name').value;
	WriteRodex.validateName(name.replace(/^(\$|\%)/, '').replace(/\t/g, ''));
}

/**
 * Drop an item from inventory to writerodex
 *
 * @param {event}
 */
function onDrop(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	// Just allow item from inventory
	if (data.type !== 'item' || data.from !== 'Inventory') {
		return;
	}

	// Have to specify how much
	if (item.count > 1) {
		InputBox.append();
		InputBox.setType('item', false, item.count, item.ITID);

		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			InputBox.remove();
			switch (data.from) {
				case 'Inventory':
					Inventory.reqMoveItemToWriteRodex(item.index, parseInt(count, 10));
					break;
				default:
				//cant do this action
			}
		};
		return;
	}

	switch (data.from) {
		case 'Inventory':
			Inventory.reqMoveItemToWriteRodex(item.index, 1);
			break;
		default:
		//cant do this action
	}
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Show item name when mouse is over
 */
function onItemOver(event) {
	const el = event.currentTarget;
	const idx = parseInt(el.getAttribute('data-index'), 10);
	const item = WriteRodex.getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = _root();
	const pos = { top: el.offsetTop, left: el.offsetLeft };
	const overlay = root.querySelector('.overlay');

	overlay.style.display = '';
	overlay.style.top = `${pos.top}px`;
	overlay.style.left = `${pos.left + 35}px`;
	overlay.textContent = `${DB.getItemName(item)} ${item.count || 1} ea`;

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	} else {
		overlay.classList.add('grey');
	}
}

/**
 * Hide the item name
 */
function onItemOut() {
	const root = _root();
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	const el = event.currentTarget;
	const index = parseInt(el.getAttribute('data-index'), 10);
	const item = WriteRodex.getItemByIndex(index);

	if (!item) {
		return;
	}

	// Set image to the drag drop element
	const img = new Image();
	const url = el.firstChild.style.backgroundImage.match(/\(([^)]+)/)[1];
	img.decoding = 'async';
	img.src = url.replace(/^"/, '').replace(/"$/, '');

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'WriteRodex',
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
	event.preventDefault();

	const el = event.currentTarget;
	const index = parseInt(el.getAttribute('data-index'), 10);
	const item = WriteRodex.getItemByIndex(index);

	if (!item) {
		return;
	}

	// Don't add the same UI twice, remove it
	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return;
	}

	// Add ui to window
	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(WriteRodex);
