/**
 * UI/Components/LaphineUpg/LaphineUpg.js
 *
 * Laphine Upgrade UI
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
import htmlText from './LaphineUpg.html?raw';
import cssText from './LaphineUpg.css?raw';
import PACKET from 'Network/PacketStructure.js';

/**
 * Create Component
 */
const LaphineUpg = new GUIComponent('LaphineUpg', cssText);

/**
 * @var {object} LaphineUpgUIState
 */
const LaphineUpgUIState = {
	itemId: null,
	needRefineMin: null,
	needRefineMax: null,
	needoptionnummin: null,
	notsocketenchantitem: null,
	targetItems: [],
	needSourceString: null
};

// Initialize a list to keep track of submitted items
LaphineUpg.submittedIndex = 0;

/**
 * Helper: query inside shadow root
 */
function _root() {
	return LaphineUpg._shadow || LaphineUpg._host;
}

/**
 * Render HTML
 */
LaphineUpg.render = () => htmlText;

/**
 * Once append to the DOM
 */
LaphineUpg.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		LaphineUpg.remove();
	}
};

/**
 * Once append
 */
LaphineUpg.onAppend = function onAppend() {
	const root = _root();

	root.querySelector('.some_notifs').style.display = 'none';
	root.querySelector('.make_enabled').style.display = 'none';
	root.querySelector('.make_disabled').style.display = 'block';
};

/**
 * Once removed from html
 */
LaphineUpg.onRemove = function onRemove() {
	const root = _root();

	LaphineUpg.submittedIndex = 0;
	root.querySelector('.submit_button_enabled').style.display = 'block';
	root.querySelector('.submit_button_disabled').style.display = 'none';
	root.querySelector('.submitted_mat_list').innerHTML = '';
	root.querySelector('.some_notifs').style.display = 'none';
	clearLaphineUpgUIState();
};

/**
 * Clears the variables in the LaphineUpgUIState object.
 */
function clearLaphineUpgUIState() {
	Object.keys(LaphineUpgUIState).forEach(key => {
		if (Array.isArray(LaphineUpgUIState[key])) {
			LaphineUpgUIState[key] = [];
		} else {
			LaphineUpgUIState[key] = null;
		}
	});
}

/**
 * Initialize UI
 */
LaphineUpg.init = function init() {
	const root = _root();

	// UI initializations
	root.querySelector('.submit_button_disabled').style.display = 'none';
	root.querySelector('.some_notifs').style.display = 'none';
	this._host.style.top = '200px';
	this._host.style.left = '480px';
	this.draggable('.titlebar');

	// Close / Cancel
	root.querySelector('.close').addEventListener('click', onRequestLaphineUpgClose);
	root.querySelector('.cancel').addEventListener('click', onRequestLaphineUpgClose);

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
	root.querySelector('.make_enabled').addEventListener('click', onRequestLaphineUpg);

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
				'#LaphineUpg .available_mat_list::-webkit-scrollbar { width: 8px; height: 0px; }',
				'#LaphineUpg .available_mat_list::-webkit-scrollbar-corner { display: none; }',
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-button:vertical:increment { background: url(${bgBottom}) center center no-repeat, url(${bgMid}) top no-repeat; height: 15px; width: 7px; background-color: transparent; }`,
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-button:vertical:decrement { background: url(${bgTop}) center center no-repeat, url(${bgMid}) bottom no-repeat; height: 15px; width: 7px; background-color: transparent; }`,
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-track-piece:vertical { background: url(${bgTop}) top no-repeat, url(${bgMid}) center repeat-y, url(${bgBottom}) bottom no-repeat; }`,
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-thumb:vertical { background: url(${thumbOutTop}) top no-repeat, url(${thumbOutMid}) center repeat-y, url(${thumbOutBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-thumb:vertical:hover { background: url(${thumbOverTop}) top no-repeat, url(${thumbOverMid}) center repeat-y, url(${thumbOverBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				`#LaphineUpg .available_mat_list::-webkit-scrollbar-thumb:vertical:active { background: url(${thumbPressTop}) top no-repeat, url(${thumbPressMid}) center repeat-y, url(${thumbPressBottom}) bottom no-repeat !important; border-radius: 100px; }`,
				'#LaphineUpg .available_mat_list::-webkit-scrollbar-thumb:vertical{ -webkit-border-image: none; }'
			].join('\n');
			const shadow = LaphineUpg._shadow;
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
function onOpenLaphineUpgUI(pkt) {
	if (pkt) {
		clearLaphineUpgUIState();

		const laphineUpgInfo = DB.getLaphineUpgInfoById(pkt.itemId);

		if (laphineUpgInfo) {
			LaphineUpgUIState.itemId = laphineUpgInfo.ItemID;
			LaphineUpgUIState.needRefineMin = laphineUpgInfo.NeedRefineMin;
			LaphineUpgUIState.needRefineMax = laphineUpgInfo.NeedRefineMax;
			LaphineUpgUIState.needoptionnummin = laphineUpgInfo.NeedOptionNumMin;
			LaphineUpgUIState.notsocketenchantitem = laphineUpgInfo.NotSocketEnchantItem;
			LaphineUpgUIState.targetItems = laphineUpgInfo.TargetItems;
			LaphineUpgUIState.needSourceString = laphineUpgInfo.NeedSource_String;

			onUpdateLaphineUpgUI();
			populateAvailableUpgMatList();
			LaphineUpg.append();
		} else {
			console.warn('Item with ID', pkt.itemId, 'not found in laphine upgrade list.');
		}
	}
}

/**
 * Updates the Laphine UI with the current state information.
 */
function onUpdateLaphineUpgUI() {
	const item = Inventory.getUI().getItemById(LaphineUpgUIState.itemId);

	if (!item) {
		return false;
	}

	const root = _root();
	root.querySelector('.item_text').textContent = DB.getItemName(item);
	root.querySelector('.mat_info_list').textContent = LaphineUpgUIState.needSourceString;
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
function populateAvailableUpgMatList() {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');
	availableMatList.innerHTML = '';

	LaphineUpgUIState.targetItems.forEach(targetItem => {
		const matchingItems = GetInventoryItemsById(targetItem.id);
		matchingItems.forEach(inventoryItem => {
			let isValid = true;

			if (
				(inventoryItem.type === ItemType.WEAPON || inventoryItem.type === ItemType.ARMOR) &&
				(inventoryItem.RefiningLevel < LaphineUpgUIState.needRefineMin ||
					inventoryItem.RefiningLevel > LaphineUpgUIState.needRefineMax)
			) {
				isValid = false;
			}

			if (LaphineUpgUIState.needoptionnummin) {
				isValid = false;
				if (inventoryItem.Options) {
					const numOfOptions = inventoryItem.Options.filter(Option => Option.index !== 0).length;
					if (numOfOptions && numOfOptions >= LaphineUpgUIState.needoptionnummin) {
						isValid = true;
					}
				}
			}

			if (isValid) {
				const it = DB.getItemInfo(targetItem.id);

				if (!it) {
					return;
				}

				const icon_name = it.identifiedResourceName;
				onAddMaterialItem(inventoryItem, icon_name);
			}
		});
	});
}

/**
 * Handles the addition of an item in the UI from the available materials list.
 */
function onAddMaterialItem(item, target_iconname) {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');

	const newItemHTML =
		`<div class="item" data-index="${item.index}" draggable="true">` +
		`<div class="icon"></div>` +
		`<div class="name">${DB.getItemName(item, { showItemGrade: false, showItemSlots: false })}</div>` +
		`</div>`;

	availableMatList.insertAdjacentHTML('beforeend', newItemHTML);

	const newItemEl = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
	if (newItemEl) {
		newItemEl.addEventListener('dblclick', onSubmitItem);
	}

	Client.loadFile(DB.INTERFACE_PATH + 'item/' + target_iconname + '.bmp', data => {
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
 * Handles the result of Laphine Upgrade.
 */
function onLaphineUpgResult(pkt) {
	if (pkt) {
		switch (pkt.result) {
			case 0:
				onRequestLaphineUpgClose();
				break;
			case 1:
			default:
				break;
		}
	}
}

/**
 * Handles the close request of the Laphine UI and sends the appropriate packet.
 */
function onRequestLaphineUpgClose() {
	LaphineUpg.remove();

	const pkt = new PACKET.CZ.RANDOM_UPGRADE_ITEM_UI_CLOSE();
	Network.sendPacket(pkt);
}

/**
 * Handles the selection of an item from the available materials list.
 */
function onItemSelect(e) {
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

	if (LaphineUpg.submittedIndex !== 0) {
		return;
	}

	if (root.querySelector('.submitted_mat_list .item')) {
		return;
	}

	const targetItem = LaphineUpgUIState.targetItems.find(si => si.id === item.ITID);
	if (!targetItem) {
		return;
	}

	onUpdateSubmitList(item);
}

/**
 * Updates the submit list with the given item.
 */
function onUpdateSubmitList(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = _root();
	const submittedMatList = root.querySelector('.submitted_mat_list');

	const newItemHTML =
		`<div class="item" data-index="${item.index}" draggable="true">` + `<div class="icon"></div>` + `</div>`;

	submittedMatList.insertAdjacentHTML('beforeend', newItemHTML);

	const newItemEl = submittedMatList.querySelector(`.item[data-index="${item.index}"]`);
	if (newItemEl) {
		newItemEl.addEventListener('dblclick', onItemRemove);
	}

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

	LaphineUpg.submittedIndex = item.index;

	root.querySelector('.submit_button_enabled').style.display = 'block';
	root.querySelector('.submit_button_disabled').style.display = 'none';
	root.querySelector('.make_disabled').style.display = 'none';
	root.querySelector('.make_enabled').style.display = 'inline-block';

	onCheckSubmittedItem(item);

	updateAvailableMatList(item.ITID, item.index, true);
}

/**
 * Checks if the submitted item has any occupied slots and shows a message if it does.
 */
function onCheckSubmittedItem(item) {
	if (LaphineUpgUIState.notsocketenchantitem) {
		const occupiedSlots = Object.values(item.slot).filter(slot => slot !== 0).length;
		if (occupiedSlots) {
			showMessage(DB.getMessage(3646));
		}
	}
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

/**
 * Removes an item from the submitted items list and updates the UI accordingly.
 */
function onRemoveItemSubmitList(item, element = null) {
	const root = _root();
	const itemIndex = LaphineUpg.submittedIndex;
	if (itemIndex !== -1) {
		LaphineUpg.submittedIndex = 0;

		if (!element) {
			element = root.querySelector(`.submitted_mat_list .item[data-index="${item.index}"]`);
		}

		if (element) {
			element.remove();
		}

		root.querySelector('.submit_button_enabled').style.display = 'block';
		root.querySelector('.submit_button_disabled').style.display = 'none';
		root.querySelector('.make_disabled').style.display = 'block';
		root.querySelector('.make_enabled').style.display = 'none';

		updateAvailableMatList(item.ITID, item.index, false);
	}
}

/**
 * Handles the changes in UI when submitting and removing item from submitted list.
 */
function updateAvailableMatList(itemId, itemIndex, remove) {
	const root = _root();
	const availableMatList = root.querySelector('.available_mat_list');
	let itemExists = false;

	availableMatList.querySelectorAll('.item').forEach(el => {
		const idx = parseInt(el.getAttribute('data-index'), 10);
		const item = Inventory.getUI().getItemByIndex(idx);

		if (item.ITID === itemId && idx === itemIndex) {
			itemExists = true;
			if (remove) {
				el.remove();
			}
		}
	});

	if (!remove && !itemExists) {
		const item = Inventory.getUI().getItemByIndex(itemIndex);
		if (item) {
			const targetItem = LaphineUpgUIState.targetItems.find(ti => ti.id === itemId);
			if (targetItem) {
				const it = DB.getItemInfo(targetItem.id);

				if (!it) {
					return;
				}

				const icon_name = it.identifiedResourceName;
				onAddMaterialItem(item, icon_name);
			}
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
 * Handles the upgrade request by preparing and sending the packet.
 */
function onRequestLaphineUpg() {
	const item = Inventory.getUI().getItemByIndex(LaphineUpg.submittedIndex);

	if (!item) {
		return;
	}

	const pkt = new PACKET.CZ.REQ_RANDOM_UPGRADE_ITEM();
	pkt.itemId = LaphineUpgUIState.itemId;
	pkt.item_index = LaphineUpg.submittedIndex;
	Network.sendPacket(pkt);

	const root = _root();
	root.querySelector('.make_enabled').style.display = 'none';
	root.querySelector('.make_disabled').style.display = 'block';
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
	overlay.textContent = DB.getItemName(item, { showItemGrade: false, showItemSlots: false });

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
				from: 'LaphineUpg',
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

	if (!item) {
		return;
	}

	const root = _root();

	if (LaphineUpg.submittedIndex !== 0) {
		return;
	}

	if (root.querySelector('.submitted_mat_list .item')) {
		return;
	}

	const targetItem = LaphineUpgUIState.targetItems.find(si => si.id === item.ITID);
	if (!targetItem) {
		return;
	}

	onUpdateSubmitList(item);
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
Network.hookPacket(PACKET.ZC.RANDOM_UPGRADE_ITEM_UI_OPEN, onOpenLaphineUpgUI);
Network.hookPacket(PACKET.ZC.ACK_RANDOM_UPGRADE_ITEM, onLaphineUpgResult);

/**
 * Create component and export it
 */
export default UIManager.addComponent(LaphineUpg);
