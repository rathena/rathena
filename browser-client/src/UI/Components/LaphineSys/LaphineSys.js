/**
 * UI/Components/LaphineSys/LaphineSys.js
 *
 * Laphine Synthesis System UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Network from 'Network/NetworkManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Client from 'Core/Client.js';
import KEYS from 'Controls/KeyEventHandler.js';
import htmlText from './LaphineSys.html?raw';
import cssText from './LaphineSys.css?raw';
import PACKET from 'Network/PacketStructure.js';

/**
 * Create Component
 */
const LaphineSys = new GUIComponent('LaphineSys', cssText);

/**
 * @var {object} LaphineUIState
 */
const LaphineUIState = {
	itemId: null,
	needCount: null,
	needRefineMin: null,
	needRefineMax: null,
	sourceItems: [],
	needSourceString: null
};

// Initialize a list to keep track of submitted items
LaphineSys.submittedItems = [];

/**
 * Helper: query inside shadow root
 */
function _root() {
	return LaphineSys._shadow || LaphineSys._host;
}

/**
 * Render HTML
 */
LaphineSys.render = () => htmlText;

/**
 * Once append to the DOM
 */
LaphineSys.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		LaphineSys.remove();
	}
};

/**
 * Once append
 */
LaphineSys.onAppend = function onAppend() {
	const root = _root();

	root.querySelector('.some_notifs').style.display = 'none';
	root.querySelector('.make_enabled').style.display = 'none';
	root.querySelector('.make_disabled').style.display = 'block';
};

/**
 * Once removed from html
 */
LaphineSys.onRemove = function onRemove() {
	const root = _root();

	LaphineSys.submittedItems.length = 0;
	root.querySelector('.submit_button_enabled').style.display = 'block';
	root.querySelector('.submit_button_disabled').style.display = 'none';
	root.querySelector('.submitted_mat_list').innerHTML = '';
	root.querySelector('.mat_count_submitted').textContent = '0';
	root.querySelector('.some_notifs').style.display = 'none';
};

/**
 * Initialize UI
 */
LaphineSys.init = function init() {
	const root = _root();

	// UI initializations
	root.querySelector('.mat_count_submitted').textContent = '0';
	root.querySelector('.submit_button_disabled').style.display = 'none';
	root.querySelector('.some_notifs').style.display = 'none';
	this._host.style.top = '200px';
	this._host.style.left = '480px';
	this.draggable('.titlebar');

	// Close / Cancel
	root.querySelector('.close').addEventListener('click', onRequestLaphineClose);
	root.querySelector('.cancel').addEventListener('click', onRequestLaphineClose);

	// Drag and drop on available_mat_list and left_panel
	const availableMatList = root.querySelector('.available_mat_list');
	const leftPanel = root.querySelector('.left_panel');

	availableMatList.addEventListener('dragover', stopPropagation);
	leftPanel.addEventListener('dragover', stopPropagation);

	availableMatList.addEventListener('dragstart', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragStart.call(item, e);
		}
	});
	availableMatList.addEventListener('dragend', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragEnd.call(item, e);
		}
	});
	leftPanel.addEventListener('dragstart', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragStart.call(item, e);
		}
	});
	leftPanel.addEventListener('dragend', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemDragEnd.call(item, e);
		}
	});

	// Submit button
	root.querySelector('.submit_button_enabled').addEventListener('click', onSubmitItem);

	// Click on available items
	availableMatList.addEventListener('click', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemSelect.call(item, e);
		}
	});

	// Drop to remove from submitted
	availableMatList.addEventListener('drop', onRemoveSubmitDrop);

	// Drop to submit item
	leftPanel.addEventListener('drop', onSubmitItemDrop);

	// Make button
	root.querySelector('.make_enabled').addEventListener('click', onRequestSynthesis);

	// Hover / context on items
	availableMatList.addEventListener('mouseover', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOver.call(item, e);
		}
	});
	availableMatList.addEventListener('mouseout', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOut();
		}
	});
	availableMatList.addEventListener('contextmenu', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemInfo.call(item, e);
		}
	});

	const submittedMatList = root.querySelector('.submitted_mat_list');
	submittedMatList.addEventListener('mouseover', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOver.call(item, e);
		}
	});
	submittedMatList.addEventListener('mouseout', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemOut();
		}
	});
	submittedMatList.addEventListener('contextmenu', e => {
		const item = e.target.closest('.item');
		if (item) {
			onItemInfo.call(item, e);
		}
	});

	// Setting scrollbar styles inside shadow DOM
	Client.loadFiles(
		[
			DB.INTERFACE_PATH + 'lapine/scrollbar_bg_btm.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_bg_mid.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_bg_top.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_out_btm.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_out_mid.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_out_top.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_over_btm.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_over_mid.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_over_top.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_press_btm.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_press_mid.bmp',
			DB.INTERFACE_PATH + 'lapine/scrollbar_thumb_press_top.bmp'
		],
		function (
			bgBottom,
			bgMid,
			bgTop,
			thumbOutBottom,
			thumbOutMid,
			thumbOutTop,
			thumbOverBottom,
			thumbOverMid,
			thumbOverTop,
			thumbPressBottom,
			thumbPressMid,
			thumbPressTop
		) {
			const style = document.createElement('style');
			style.textContent = [
				'#LaphineSys .available_mat_list::-webkit-scrollbar { width: 8px; height: 0px; }',
				'#LaphineSys .available_mat_list::-webkit-scrollbar-corner { display: none; }',
				`#LaphineSys .available_mat_list::-webkit-scrollbar-button:vertical:increment { background: url(${bgBottom}) center center no-repeat, url(${bgMid}) top no-repeat; height: 15px; width: 7px; background-color: transparent; }`,
				`#LaphineSys .available_mat_list::-webkit-scrollbar-button:vertical:decrement { background: url(${bgTop}) center center no-repeat, url(${bgMid}) bottom no-repeat; height: 15px; width: 7px; background-color: transparent; }`,
				`#LaphineSys .available_mat_list::-webkit-scrollbar-track-piece:vertical { background: url(${bgTop}) top no-repeat, url(${bgMid}) center repeat-y, url(${bgBottom}) bottom no-repeat; }`,
				`#LaphineSys .available_mat_list::-webkit-scrollbar-thumb:vertical { background: url(${thumbOutTop}) top no-repeat, url(${thumbOutMid}) center repeat-y, url(${thumbOutBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				`#LaphineSys .available_mat_list::-webkit-scrollbar-thumb:vertical:hover { background: url(${thumbOverTop}) top no-repeat, url(${thumbOverMid}) center repeat-y, url(${thumbOverBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				`#LaphineSys .available_mat_list::-webkit-scrollbar-thumb:vertical:active { background: url(${thumbPressTop}) top no-repeat, url(${thumbPressMid}) center repeat-y, url(${thumbPressBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				'#LaphineSys .available_mat_list::-webkit-scrollbar-thumb:vertical{ -webkit-border-image: none; }'
			].join('\n');
			const shadow = LaphineSys._shadow;
			if (shadow) {
				shadow.appendChild(style);
			}
		}
	);
};

/**
 * Opens the Laphine UI and initializes its state based on the provided packet data.
 * @param {object} pkt - The packet containing item information.
 */
function onOpenLaphineUI(pkt) {
	if (pkt) {
		const laphineInfo = DB.getLaphineSysInfoById(pkt.itemId);

		if (laphineInfo) {
			LaphineUIState.itemId = laphineInfo.ItemID;
			LaphineUIState.needCount = laphineInfo.NeedCount;
			LaphineUIState.needRefineMin = laphineInfo.NeedRefineMin;
			LaphineUIState.needRefineMax = laphineInfo.NeedRefineMax;
			LaphineUIState.sourceItems = laphineInfo.SourceItems;
			LaphineUIState.needSourceString = laphineInfo.NeedSource_String;

			onUpdateLaphineUI();
			populateAvailableMatList();
			LaphineSys.append();
		} else {
			console.warn('Item with ID', pkt.itemId, 'not found in lapine_list.');
		}
	}
}

/**
 * Updates the Laphine UI with the current state information.
 */
function onUpdateLaphineUI() {
	const item = Inventory.getUI().getItemById(LaphineUIState.itemId);

	if (!item) {
		return false;
	}

	const root = _root();
	root.querySelector('.item_text').textContent = DB.getItemName(item);
	root.querySelector('.mat_info_list').textContent = LaphineUIState.needSourceString;
	root.querySelector('.mat_count_needed').textContent = LaphineUIState.needCount;
}

/**
 * Get item objects by id
 *
 * @param {number} id
 * @returns {Array<Item>}
 */
function GetInventoryItemsById(id) {
	const items = [];
	const list = Inventory.getUI().list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].ITID === id) {
			items.push(list[i]);
		}
	}

	return items;
}

/**
 * Populates the list of available materials.
 */
function populateAvailableMatList() {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');
	availableMatList.innerHTML = '';

	LaphineUIState.sourceItems.forEach(sourceItem => {
		const matchingItems = GetInventoryItemsById(sourceItem.id);
		matchingItems.forEach(inventoryItem => {
			const count =
				inventoryItem.type === ItemType.WEAPON || inventoryItem.type === ItemType.ARMOR
					? 1
					: inventoryItem.count;
			onAddMaterialItem(inventoryItem, count, sourceItem.count, sourceItem.name);
		});
	});
}

/**
 * Handles the addition of an item in the UI from the available materials list.
 */
function onAddMaterialItem(item, inventory_count, source_needcount, source_iconname) {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');

	let isSelectable;
	if (item.type === ItemType.WEAPON || item.type === ItemType.ARMOR) {
		isSelectable =
			item.RefiningLevel >= LaphineUIState.needRefineMin && item.RefiningLevel <= LaphineUIState.needRefineMax;
	} else {
		isSelectable = inventory_count >= source_needcount;
	}

	const draggableAttr = !isSelectable ? '' : ' draggable="true"';
	const itemClass = isSelectable ? '' : ' unselectable';

	const newItemHTML =
		`<div class="item${itemClass}" data-index="${item.index}"${draggableAttr}>` +
		`<div class="icon"></div>` +
		`<div class="amount">${inventory_count}</div>` +
		`<div class="name">${DB.getItemName(item)}</div>` +
		`</div>`;

	availableMatList.insertAdjacentHTML('beforeend', newItemHTML);

	const newItemEl = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
	if (newItemEl) {
		newItemEl.addEventListener('dblclick', onSubmitItem);
	}

	Client.loadFile(DB.INTERFACE_PATH + 'item/' + source_iconname + '.bmp', data => {
		const icon = availableMatList.querySelector(`.item[data-index="${item.index}"] .icon`);
		if (icon) {
			icon.style.backgroundImage = `url(${data})`;
		}
	});

	Client.loadFile(DB.INTERFACE_PATH + 'lapine/list_selected_item.bmp', data => {
		const name = availableMatList.querySelector(`.item[data-index="${item.index}"] .name`);
		if (name) {
			name.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Handles the result of Laphine Synthesis.
 */
function onLaphineSysResult(pkt) {
	if (pkt) {
		switch (pkt.result) {
			case 0:
				onRequestLaphineClose();
				break;
			case 5: // LAPHINE_SYNTHESIS_AMOUNT = 5,
			case 7: // LAPHINE_SYNTHESIS_ITEM = 7
				break;
			default:
				break;
		}
	}
}

/**
 * Handles the close request of the Laphine UI and sends the appropriate packet.
 */
function onRequestLaphineClose() {
	LaphineSys.remove();

	const pkt = new PACKET.CZ.RANDOM_COMBINE_ITEM_UI_CLOSE();
	Network.sendPacket(pkt);
}

/**
 * Handles the selection of an item from the available materials list.
 */
function onItemSelect(e) {
	if (this.classList.contains('unselectable')) {
		e.preventDefault();
		e.stopImmediatePropagation();
		return;
	}

	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		e.preventDefault();
		e.stopImmediatePropagation();
		return;
	}

	const root = _root();
	root.querySelectorAll('.name').forEach(el => el.classList.remove('selected'));
	const nameEl = root.querySelector(`.item[data-index="${item.index}"] .name`);
	if (nameEl) {
		nameEl.classList.add('selected');
	}
}

/**
 * Submits a selected item and updates the UI accordingly.
 */
function onSubmitItem() {
	const root = _root();
	const submittedCount = parseInt(root.querySelector('.mat_count_submitted').textContent, 10);
	const neededCount = parseInt(root.querySelector('.mat_count_needed').textContent, 10);

	if (submittedCount >= neededCount) {
		return;
	}

	const selectedItem = root.querySelector('.item .name.selected');
	let idx;

	if (selectedItem) {
		idx = parseInt(selectedItem.closest('.item').getAttribute('data-index'), 10);
	} else {
		idx = parseInt(this.getAttribute('data-index'), 10);
	}

	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	if (LaphineSys.submittedItems.some(submittedItem => submittedItem.index === item.index)) {
		return;
	}

	const sourceItem = LaphineUIState.sourceItems.find(si => si.id === item.ITID);
	if (!sourceItem) {
		return;
	}

	if (this.classList && this.classList.contains('unselectable')) {
		let message;
		if (item.type === ItemType.WEAPON || item.type === ItemType.ARMOR) {
			if (item.RefiningLevel > LaphineUIState.needRefineMax) {
				message = DB.getMessage(3644);
				showMessage(message);
			} else {
				message = DB.getMessage(2899);
				showMessage(message);
			}
		} else {
			message = DB.getMessage(2898).replace('%d', sourceItem.count);
			showMessage(message);
		}
		return;
	}

	onUpdateSubmitList(item);
}

function onUpdateSubmitList(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = _root();
	const submittedMatList = root.querySelector('.submitted_mat_list');

	const sourceItem = LaphineUIState.sourceItems.find(si => si.id === item.ITID);

	const itemCount = item.type === ItemType.WEAPON || item.type === ItemType.ARMOR ? 1 : sourceItem.count;

	const newItemHTML =
		`<div class="item" data-index="${item.index}" draggable="true">` +
		`<div class="shadow"></div>` +
		`<div class="icon"></div>` +
		`<div class="amount">${sourceItem.count}</div>` +
		`</div>`;

	submittedMatList.insertAdjacentHTML('beforeend', newItemHTML);

	const newItemEl = submittedMatList.querySelector(`.item[data-index="${item.index}"]`);
	if (newItemEl) {
		newItemEl.addEventListener('dblclick', onItemRemove);
	}

	Client.loadFile(DB.INTERFACE_PATH + 'lapine/shadow_item_off.bmp', data => {
		const shadow = submittedMatList.querySelector(`.item[data-index="${item.index}"] .shadow`);
		if (shadow) {
			shadow.style.backgroundImage = `url(${data})`;
		}
	});

	Client.loadFile(
		DB.INTERFACE_PATH +
			'item/' +
			(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
			'.bmp',
		data => {
			const icon = submittedMatList.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);

	LaphineSys.submittedItems.push({
		index: item.index,
		count: itemCount
	});

	const submittedCount = parseInt(root.querySelector('.mat_count_submitted').textContent, 10) + 1;
	root.querySelector('.mat_count_submitted').textContent = submittedCount;

	const neededCount = parseInt(root.querySelector('.mat_count_needed').textContent, 10);
	if (submittedCount >= neededCount) {
		root.querySelector('.submit_button_enabled').style.display = 'none';
		root.querySelector('.submit_button_disabled').style.display = 'block';
		root.querySelector('.make_disabled').style.display = 'none';
		root.querySelector('.make_enabled').style.display = 'inline-block';
	}

	adjustSubmittedMatList();

	updateAvailableMatList(item.ITID, item.index, sourceItem.count, false);
}

/**
 * Adjusts the layout of the submitted materials list.
 */
function adjustSubmittedMatList() {
	const root = _root();
	const submittedMatList = root.querySelector('.submitted_mat_list');
	const items = submittedMatList.querySelectorAll('.item');
	const numItems = items.length;
	const numRows = Math.ceil(numItems / 5);

	submittedMatList.style.top = `${145 - (numRows - 1) * 28}px`;
}

/**
 * Handles the removal of a submitted item and updates the UI accordingly.
 */
function onItemRemove() {
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (item) {
		onRemoveItemSubmitList(item, this);
	}
}

function onRemoveItemSubmitList(item, element = null) {
	const root = _root();
	const itemIndex = LaphineSys.submittedItems.findIndex(submittedItem => submittedItem.index === item.index);
	if (itemIndex !== -1) {
		LaphineSys.submittedItems.splice(itemIndex, 1);

		if (!element) {
			element = root.querySelector(`.submitted_mat_list .item[data-index="${item.index}"]`);
		}

		if (element) {
			element.remove();
		}

		const submittedCount = parseInt(root.querySelector('.mat_count_submitted').textContent, 10) - 1;
		root.querySelector('.mat_count_submitted').textContent = submittedCount;

		const neededCount = parseInt(root.querySelector('.mat_count_needed').textContent, 10);
		if (submittedCount < neededCount) {
			root.querySelector('.submit_button_enabled').style.display = 'block';
			root.querySelector('.submit_button_disabled').style.display = 'none';
		}

		adjustSubmittedMatList();

		updateAvailableMatList(item.ITID, item.index, item.count, true);
	}
}

/**
 * Handles the changes in UI when submitting and removing item from submitted list.
 */
function updateAvailableMatList(itemId, itemIndex, countChange, increase) {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');
	let itemExists = false;

	availableMatList.querySelectorAll('.item').forEach(el => {
		const idx = parseInt(el.getAttribute('data-index'), 10);
		const item = Inventory.getUI().getItemByIndex(idx);

		if (item.ITID === itemId && idx === itemIndex) {
			itemExists = true;
			let tempCount = item.type === ItemType.WEAPON || item.type === ItemType.ARMOR ? 1 : item.count;
			if (increase) {
				el.classList.remove('unselectable');
				const amountEl = el.querySelector('.amount');
				if (amountEl) {
					amountEl.textContent = tempCount;
				}
				el.setAttribute('draggable', 'true');
			} else {
				tempCount -= countChange;
				if (tempCount <= 0) {
					el.remove();
				} else {
					el.classList.add('unselectable');
					const nameEl = el.querySelector('.name');
					if (nameEl) {
						nameEl.classList.remove('selected');
					}
					const amountEl = el.querySelector('.amount');
					if (amountEl) {
						amountEl.textContent = tempCount;
					}
					el.setAttribute('draggable', 'false');
				}
			}
		}
	});

	if (increase && !itemExists) {
		const item = Inventory.getUI().getItemByIndex(itemIndex);
		if (item) {
			const inventory_count = item.type === ItemType.WEAPON || item.type === ItemType.ARMOR ? 1 : item.count;
			const sourceItem = LaphineUIState.sourceItems.find(si => si.id === itemId);
			const source_needcount = sourceItem.count;
			const source_iconname = sourceItem.name;

			onAddMaterialItem(item, inventory_count, source_needcount, source_iconname);
		}
	}
}

/**
 * Handles showing of message for notifications.
 */
function showMessage(message) {
	const root = _root();
	const infoMsg = root.querySelector('.info_msg');
	if (infoMsg) {
		infoMsg.textContent = '';
		infoMsg.textContent = message;
	}
	const notifs = root.querySelector('.some_notifs');
	if (notifs) {
		notifs.style.display = 'block';
	}
}

/**
 * Handles the synthesis request by preparing and sending the packet.
 */
function onRequestSynthesis() {
	const pkt = new PACKET.CZ.REQ_RANDOM_COMBINE_ITEM();
	pkt.itemId = LaphineUIState.itemId;
	pkt.items = [];

	if (LaphineSys.submittedItems.length === LaphineUIState.needCount) {
		for (let i = 0; i < LaphineSys.submittedItems.length; i++) {
			const submittedItem = LaphineSys.submittedItems[i];
			pkt.items.push({
				index: submittedItem.index,
				count: submittedItem.count
			});
		}
	} else {
		console.warn('[Laphine Synthesis] The number of submitted items does not match the needed count.');
		return;
	}

	Network.sendPacket(pkt);
}

/**
 * Show item name when mouse is over
 */
function onItemOver() {
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = _root();
	const overlay = root.querySelector('.overlay');

	const parentContainer = this.closest('.available_mat_list') || this.closest('.submitted_mat_list');
	if (!parentContainer) {
		return;
	}

	const itemRect = this.getBoundingClientRect();
	const containerRect = parentContainer.getBoundingClientRect();
	const panelRect = root.querySelector('.panel').getBoundingClientRect();
	const overlayHeight = overlay.offsetHeight || 23;

	const top = itemRect.top - containerRect.top + (containerRect.top - panelRect.top) - overlayHeight + 25;
	const left = itemRect.left - panelRect.left;

	overlay.style.display = 'block';
	overlay.style.top = `${top}px`;
	overlay.style.left = `${left}px`;
	overlay.textContent = DB.getItemName(item);

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
	root.querySelector('.overlay').style.display = 'none';
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(index);

	if (!item) {
		return;
	}

	const img = new Image();
	const iconEl = this.querySelector('.icon');
	if (!iconEl) {
		return;
	}
	const match = iconEl.style.backgroundImage.match(/\((.*?)\)/);
	if (!match) {
		return;
	}
	const url = match[1].replace(/('|")/g, '');
	img.decoding = 'async';
	img.src = url.replace(/^"/, '').replace(/"$/, '');

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'LaphineSys',
				data: item
			})
		)
	);

	onItemOut();
}

/**
 * Stop dragging an item
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

/**
 * Drop an item from available_mat_list into submitted_mat_list
 */
function onSubmitItemDrop(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	if (data.type !== 'item') {
		return;
	}

	const root = _root();
	const submittedCount = parseInt(root.querySelector('.mat_count_submitted').textContent, 10);
	const neededCount = parseInt(root.querySelector('.mat_count_needed').textContent, 10);

	if (submittedCount >= neededCount) {
		return;
	}

	if (LaphineSys.submittedItems.some(submittedItem => submittedItem.index === item.index)) {
		return;
	}

	const sourceItem = LaphineUIState.sourceItems.find(si => si.id === item.ITID);
	if (!sourceItem) {
		return;
	}

	if (item) {
		onUpdateSubmitList(item);
	}
}

/**
 * Drop an item from submitted_mat_list into available_mat_list
 */
function onRemoveSubmitDrop(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	if (data.type !== 'item') {
		return;
	}

	if (!item) {
		return;
	}

	const root = _root();
	const element = root.querySelector(`.submitted_mat_list .item[data-index="${item.index}"]`);

	onRemoveItemSubmitList(item, element);
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	if (ItemCompare.ui) {
		ItemCompare.remove();
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		if (ItemCompare.ui) {
			ItemCompare.remove();
		}
		return;
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);

	const compareItem = Equipment.getUI().isInEquipList(item.location);

	if (compareItem && Inventory.getUI().itemcomp) {
		ItemCompare.prepare();
		ItemCompare.append();
		ItemCompare.uid = compareItem.ITID;
		ItemCompare.setItem(compareItem);
	}
}

/**
 * Packet Hooks to functions
 */
Network.hookPacket(PACKET.ZC.RANDOM_COMBINE_ITEM_UI_OPEN, onOpenLaphineUI);
Network.hookPacket(PACKET.ZC.ACK_RANDOM_COMBINE_ITEM, onLaphineSysResult);

/**
 * Create component and export it
 */
export default UIManager.addComponent(LaphineSys);
