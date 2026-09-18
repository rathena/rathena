/**
 * UI/Components/ItemReform/ItemReform.js
 *
 * Item Reform UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import EffectConst from 'DB/Effects/EffectConst.js';
import ItemType from 'DB/Items/ItemType.js';
import Session from 'Engine/SessionStorage.js';
import Network from 'Network/NetworkManager.js';
import EffectManager from 'Renderer/EffectManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import NpcBox from 'UI/Components/NpcBox/NpcBox.js';
import Client from 'Core/Client.js';
import KEYS from 'Controls/KeyEventHandler.js';
import htmlText from './ItemReform.html?raw';
import cssText from './ItemReform.css?raw';
import PACKET from 'Network/PacketStructure.js';

/**
 * Create Component
 */
const ItemReform = new GUIComponent('ItemReform', cssText);

/**
 * Render HTML
 */
ItemReform.render = () => htmlText;

/**
 * Use capture phase for keydown so Escape is handled before EscapeWindow
 */
ItemReform.captureKeyEvents = true;

let ReformInfo = {};
let SelectedReformInfo = {};

const ReformUIState = {
	itemId: 0,
	index: 0,
	resultItem: null,
	timeout: 0
};

/**
 * Helper: query inside shadow root
 */
function _root() {
	return ItemReform._shadow || ItemReform._host;
}

/**
 * Key handler
 */
ItemReform.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		ItemReform.remove();
		event.preventDefault();
		event.stopImmediatePropagation();
	}
};

/**
 * Once append
 */
ItemReform.onAppend = function onAppend() {
	const root = _root();
	const details = root.querySelector('.information_details');
	const reformEnabled = root.querySelector('.reform_enabled');
	const reformDisabled = root.querySelector('.reform_disabled');

	if (details) {
		details.style.display = 'none';
	}
	if (reformEnabled) {
		reformEnabled.style.display = 'none';
	}
	if (reformDisabled) {
		reformDisabled.style.display = 'inline-block';
	}
};

/**
 * Once removed from html
 */
ItemReform.onRemove = function onRemove() {
	const root = _root();

	ReformInfo = {};
	SelectedReformInfo = {};

	const availableMatList = root.querySelector('.available_material_list');
	const materialList = root.querySelector('.material_list');
	const resultItemText = root.querySelector('.result_item_text');
	const baseItem = root.querySelector('.base_item');
	const resultItem = root.querySelector('.result_item');
	const infoMsg = root.querySelector('.info_msg');
	const details = root.querySelector('.information_details');
	const reformEnabled = root.querySelector('.reform_enabled');
	const reformDisabled = root.querySelector('.reform_disabled');
	const someNotifs = root.querySelector('.some_notifs');

	if (availableMatList) {
		availableMatList.innerHTML = '';
	}
	if (materialList) {
		materialList.innerHTML = '';
	}
	if (resultItemText) {
		resultItemText.innerHTML = '';
	}
	if (baseItem) {
		baseItem.innerHTML = '';
	}
	if (resultItem) {
		resultItem.innerHTML = '';
	}
	if (infoMsg) {
		infoMsg.innerHTML = '';
	}
	if (details) {
		details.style.display = 'none';
	}
	if (reformEnabled) {
		reformEnabled.style.display = 'none';
	}
	if (reformDisabled) {
		reformDisabled.style.display = 'inline-block';
	}
	if (someNotifs) {
		someNotifs.style.display = 'none';
	}

	if (_npcBoxMoveHandler) {
		document.removeEventListener('mousemove', _npcBoxMoveHandler);
		_npcBoxMoveHandler = null;
	}
	if (NpcBox.ui && NpcBox.ui.is(':visible')) {
		NpcBox.remove();
	}

	resetReformUIState();
};

function resetReformUIState() {
	ReformUIState.itemId = 0;
	ReformUIState.index = 0;
	ReformUIState.resultItem = null;
	ReformUIState.timeout = 0;
}

/**
 * Initialize UI
 */
ItemReform.init = function init() {
	const root = _root();

	Object.assign(this._host.style, { top: '200px', left: '480px' });
	this.draggable('.titlebar');

	const closeBtn = root.querySelector('.close');
	const cancelBtn = root.querySelector('.cancel');
	if (closeBtn) {
		closeBtn.addEventListener('click', onRequestReformClose);
	}
	if (cancelBtn) {
		cancelBtn.addEventListener('click', onRequestReformClose);
	}

	const availableMatList = root.querySelector('.available_material_list');
	if (availableMatList) {
		availableMatList.addEventListener('click', e => {
			const item = e.target.closest('.item');
			if (item) {
				onMaterialSelect(item);
			}
		});
		availableMatList.addEventListener('mouseover', e => {
			const item = e.target.closest('.item');
			if (item) {
				onHoverContainer(item);
			}
		});
		availableMatList.addEventListener('mouseout', e => {
			const item = e.target.closest('.item');
			if (item && (!e.relatedTarget || !item.contains(e.relatedTarget))) {
				onHoverOutContainer(item);
			}
		});
	}

	const infoDetails = root.querySelector('.information_details');
	if (infoDetails) {
		infoDetails.addEventListener('mouseover', onHoverDetails);
		infoDetails.addEventListener('mouseout', onHoverOutDetails);
	}

	const reformEnabled = root.querySelector('.reform_enabled');
	if (reformEnabled) {
		reformEnabled.addEventListener('click', onRequestItemReform);
	}

	const panel = root.querySelector('.panel');
	if (panel) {
		panel.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) {
				e.preventDefault();
				e.stopImmediatePropagation();
				onItemInfo(item);
			}
		});
	}

	const leftPanel = root.querySelector('.panel .left_panel');
	if (leftPanel) {
		leftPanel.addEventListener('mouseover', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemOver(e, item);
			}
		});
		leftPanel.addEventListener('mouseout', e => {
			const item = e.target.closest('.item');
			if (item && (!e.relatedTarget || !item.contains(e.relatedTarget))) {
				onItemOut();
			}
		});
	}
};

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
 * Opens the Reform UI and initializes its state based on the provided packet data.
 * @param {object} pkt - The packet containing item information.
 */
function onOpenReformUI(pkt) {
	if (pkt) {
		ReformInfo = {};
		SelectedReformInfo = {};

		const reformids = DB.findReformListByItemID(pkt.ITID);

		const item = Inventory.getUI().getItemById(pkt.ITID);

		if (!item) {
			return false;
		}

		ReformUIState.itemId = pkt.ITID;

		if (reformids) {
			ReformInfo = DB.getAllReformInfos(reformids);
			checkReformCriteria();
			ItemReform.append();

			const root = _root();
			const itemText = root.querySelector('.item_text');
			if (itemText) {
				itemText.textContent = DB.getItemName(item);
			}
		} else {
			console.warn('Item with ID', pkt.itemId, 'not found in Reform List.');
		}
	}
}

/**
 * Checks inventory items against reform criteria.
 */
function checkReformCriteria() {
	const root = _root();
	const availableMatList = root.querySelector('.available_material_list');
	if (!availableMatList) {
		return;
	}
	availableMatList.innerHTML = '';

	let availableMats = 0;
	for (const reform of ReformInfo) {
		const baseItemId = reform.BaseItemId;

		const items = GetInventoryItemsById(baseItemId);

		for (const item of items) {
			if (item.RefiningLevel < reform.NeedRefineMin || item.RefiningLevel > reform.NeedRefineMax) {
				continue;
			}

			const optionCount = item.Options.filter(option => option.index !== 0).length;
			if (optionCount < reform.NeedOptionNumMin) {
				continue;
			}

			const cardCount = Object.values(item.slot).filter(slot => slot !== 0).length;
			if (reform.IsEmptySocket && cardCount > 0) {
				continue;
			}

			availableMats++;
			onAddMaterialItem(item);
		}
	}

	if (!availableMats) {
		showMessage(DB.getMessage(3856));
	}
}

/**
 * Handles the addition of an item in the UI from the available materials list.
 */
function onAddMaterialItem(item) {
	const root = _root();
	const availableMatList = root.querySelector('.available_material_list');
	if (!availableMatList) {
		return;
	}

	const it = DB.getItemInfo(item.ITID);

	const newItem = document.createElement('div');
	newItem.className = 'item';
	newItem.setAttribute('data-index', item.index);
	newItem.innerHTML =
		`<div class="item_container" data-index="${item.index}">` +
		`<div class="icon"></div>` +
		`<div class="name">${DB.getItemName(item, { showItemGrade: false, showItemOptions: false })}</div>` +
		`</div>`;

	availableMatList.appendChild(newItem);

	Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', function (data) {
		const icon = availableMatList.querySelector(`.item[data-index="${item.index}"] .icon`);
		if (icon) {
			icon.style.backgroundImage = `url(${data})`;
		}
	});

	Client.loadFile(DB.INTERFACE_PATH + 'itemreform/btn_reform_item.bmp', function (data) {
		const el = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Handles the selection of a material in the UI.
 */
function onMaterialSelect(element) {
	const idx = parseInt(element.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	ReformUIState.index = item.index;

	SelectedReformInfo = ReformInfo.find(info => info.BaseItemId === item.ITID);

	if (!SelectedReformInfo) {
		return;
	}

	UpdatePossibleReformUI(item, SelectedReformInfo);

	const root = _root();
	const availableMatList = root.querySelector('.available_material_list');
	if (!availableMatList) {
		return;
	}

	availableMatList.querySelectorAll('.item').forEach(el => {
		el.classList.remove('selected');
		const resetIdx = parseInt(el.getAttribute('data-index'), 10);
		const resetItem = Inventory.getUI().getItemByIndex(resetIdx);
		if (resetItem) {
			Client.loadFile(DB.INTERFACE_PATH + 'itemreform/btn_reform_item.bmp', function (data) {
				const target = availableMatList.querySelector(`.item[data-index="${resetItem.index}"]`);
				if (target) {
					target.style.backgroundImage = `url(${data})`;
				}
			});
		}
	});

	element.classList.add('selected');
	Client.loadFile(DB.INTERFACE_PATH + 'itemreform/btn_reform_item_press.bmp', function (data) {
		const target = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
		if (target) {
			target.style.backgroundImage = `url(${data})`;
		}
	});

	showMessage(DB.getMessage(3855));
}

/**
 * Handles the hover event on a container element.
 */
function onHoverContainer(element) {
	if (element.classList.contains('selected')) {
		return;
	}

	const idx = parseInt(element.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = _root();
	const availableMatList = root.querySelector('.available_material_list');
	if (!availableMatList) {
		return;
	}

	Client.loadFile(DB.INTERFACE_PATH + 'itemreform/btn_reform_item_over.bmp', function (data) {
		const target = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
		if (target) {
			target.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Handles the hover out event on a container element.
 */
function onHoverOutContainer(element) {
	if (element.classList.contains('selected')) {
		return;
	}

	const idx = parseInt(element.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = _root();
	const availableMatList = root.querySelector('.available_material_list');
	if (!availableMatList) {
		return;
	}

	Client.loadFile(DB.INTERFACE_PATH + 'itemreform/btn_reform_item.bmp', function (data) {
		const target = availableMatList.querySelector(`.item[data-index="${item.index}"]`);
		if (target) {
			target.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Updates the UI with the possible reform details for a given item and reform info.
 *
 * @param {Object} item - The item object.
 * @param {Object} info - The reform info object.
 */
function UpdatePossibleReformUI(item, info) {
	const root = _root();

	const details = root.querySelector('.information_details');
	if (details) {
		details.style.display = '';
	}

	const resultItem = { ...item };
	resultItem.ITID = info.ResultItemId;
	resultItem.RefiningLevel = item.RefiningLevel + info.ChangeRefineValue;
	if (!info.PreserveGrade) {
		resultItem.enchantgrade = 0;
	}
	if (!info.PreserveSocketItem) {
		resultItem.slot = { card1: 0, card2: 0, card3: 0, card4: 0 };
	}

	ReformUIState.resultItem = resultItem;

	// Result Item
	const result_it = DB.getItemInfo(resultItem.ITID);
	const resultItemDiv = root.querySelector('.result_item');

	if (resultItemDiv) {
		resultItemDiv.innerHTML =
			`<div class="item resultitem" data-index="${resultItem.ITID}">` + `<div class="icon"></div>` + `</div>`;

		Client.loadFile(DB.INTERFACE_PATH + 'item/' + result_it.identifiedResourceName + '.bmp', function (data) {
			const icon = resultItemDiv.querySelector(`.item[data-index="${resultItem.ITID}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		});
	}

	// Base Item
	const it = DB.getItemInfo(item.ITID);
	const baseItemDiv = root.querySelector('.base_item');

	if (baseItemDiv) {
		baseItemDiv.innerHTML = `<div class="item" data-index="${item.index}">` + `<div class="icon"></div>` + `</div>`;

		Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', function (data) {
			const icon = baseItemDiv.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		});
	}

	const resultItemText = root.querySelector('.result_item_text');
	if (resultItemText) {
		resultItemText.textContent = DB.getItemName(resultItem, { showItemOptions: false });
	}

	// Populate Material List
	const materialDiv = root.querySelector('.material_list');
	if (!materialDiv) {
		return;
	}

	materialDiv.innerHTML = '';

	const sortedMaterials = info.Materials.slice().sort((a, b) => a.MaterialItemID - b.MaterialItemID);
	const limitedMaterials = sortedMaterials.slice(0, 6);

	let withenoughMat = 0;
	limitedMaterials.forEach(material => {
		const mat_it = DB.getItemInfo(material.MaterialItemID);
		const mat_item = Inventory.getUI().getItemById(material.MaterialItemID);
		let inventory_mat_count;

		if (!mat_item) {
			inventory_mat_count = 0;
		} else {
			inventory_mat_count =
				mat_item.type === ItemType.WEAPON || mat_item.type === ItemType.ARMOR ? 1 : mat_item.count;
		}

		if (inventory_mat_count >= material.Amount) {
			withenoughMat++;
		}

		const itemClass = inventory_mat_count >= material.Amount ? '' : 'red';

		const newMat = document.createElement('div');
		newMat.className = 'item dummy';
		newMat.setAttribute('data-index', material.MaterialItemID);
		newMat.innerHTML =
			`<div class="icon"></div>` +
			`<div class="count ${itemClass}">${inventory_mat_count} / ${material.Amount}</div>`;

		materialDiv.appendChild(newMat);

		Client.loadFile(DB.INTERFACE_PATH + 'item/' + mat_it.identifiedResourceName + '.bmp', function (data) {
			const icon = materialDiv.querySelector(`.item[data-index="${material.MaterialItemID}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		});
	});

	if (withenoughMat >= limitedMaterials.length) {
		const reformDisabled = root.querySelector('.reform_disabled');
		const reformEnabled = root.querySelector('.reform_enabled');
		if (reformDisabled) {
			reformDisabled.style.display = 'none';
		}
		if (reformEnabled) {
			reformEnabled.style.display = 'inline-block';
		}
	}
}

/**
 * Handles the hover event for displaying item details in the NpcBox.
 */
let _npcBoxMoveHandler = null;

function onHoverDetails() {
	if (SelectedReformInfo) {
		const infoText = SelectedReformInfo.InformationString.join('\n');

		NpcBox.append();
		NpcBox.setText(infoText, 0);

		NpcBox.ui.css('height', '150px');
		NpcBox.ui.find('.border').css('height', '139px');

		function updateNpcBoxPosition(e) {
			NpcBox.ui.css({
				top: e.pageY + 10,
				left: e.pageX
			});
		}

		document.body.appendChild(NpcBox._host || NpcBox.ui[0]);

		_npcBoxMoveHandler = updateNpcBoxPosition;
		document.addEventListener('mousemove', _npcBoxMoveHandler);
	}
}

/**
 * Handles the hover out event for the details.
 */
function onHoverOutDetails() {
	if (SelectedReformInfo) {
		if (_npcBoxMoveHandler) {
			document.removeEventListener('mousemove', _npcBoxMoveHandler);
			_npcBoxMoveHandler = null;
		}
		if (NpcBox.ui.is(':visible')) {
			NpcBox.remove();
		}
	}
}

/**
 * Handles the result of Item Reform
 */
function onItemReformResult(pkt) {
	if (pkt) {
		switch (pkt.result) {
			case 0: {
				const item = Inventory.getUI().getItemByIndex(pkt.index);

				const EF_Init_Par = {
					effectId: EffectConst.EF_NEW_SUCCESS,
					ownerAID: Session.AID
				};

				function handleEffectAndPreview() {
					EffectManager.spam(EF_Init_Par);

					if (ReformUIState.timeout) {
						clearTimeout(ReformUIState.timeout);
					}

					ReformUIState.timeout = setTimeout(() => {
						showItemPreview(item);
					}, 3000);
				}

				handleEffectAndPreview();

				onRequestReformClose();
				break;
			}
			default:
				break;
		}
	}
}

/**
 * Handles the close request of the Reform UI and sends the appropriate packet.
 */
function onRequestReformClose() {
	ItemReform.remove();

	const pkt = new PACKET.CZ.CLOSE_REFORM_UI();
	Network.sendPacket(pkt);
}

/**
 * Handles showing of message for notifications.
 */
function showMessage(message) {
	const root = _root();
	const infoMsg = root.querySelector('.info_msg');
	const someNotifs = root.querySelector('.some_notifs');

	if (infoMsg) {
		infoMsg.textContent = message;
	}
	if (someNotifs) {
		someNotifs.style.display = '';
	}
}

/**
 * Handles the item reform request by preparing and sending the packet.
 */
function onRequestItemReform() {
	const pkt = new PACKET.CZ.ITEM_REFORM();
	pkt.ITID = ReformUIState.itemId;
	pkt.index = ReformUIState.index;

	Network.sendPacket(pkt);
}

/**
 * Show item name when mouse is over
 */
function onItemOver(event, element) {
	const idx = parseInt(element.getAttribute('data-index'), 10);
	let item;

	if (element.classList.contains('dummy')) {
		item = { ITID: idx, IsIdentified: 1 };
	} else if (element.classList.contains('resultitem')) {
		item = ReformUIState.resultItem;
	} else {
		item = Inventory.getUI().getItemByIndex(idx);
	}

	const root = _root();
	const overlay = root.querySelector('.overlay');

	if (!overlay) {
		return;
	}

	overlay.style.display = 'block';
	overlay.textContent = DB.getItemName(item, { showItemOptions: false });

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	} else {
		overlay.classList.add('grey');
	}

	const hostRect = ItemReform._host.getBoundingClientRect();

	function updateOverlayPosition(e) {
		Object.assign(overlay.style, {
			top: `${e.pageY - hostRect.top + 10}px`,
			left: `${e.pageX - hostRect.left + 10}px`
		});
	}

	updateOverlayPosition(event);

	function moveHandler(e) {
		updateOverlayPosition(e);
	}

	function outHandler(e) {
		if (element.contains(e.relatedTarget)) {
			return;
		}
		document.removeEventListener('mousemove', moveHandler);
		element.removeEventListener('mouseout', outHandler);
		onItemOut();
	}

	document.addEventListener('mousemove', moveHandler);
	element.addEventListener('mouseout', outHandler);
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
 * Get item info (open description window)
 */
function onItemInfo(element) {
	const idx = parseInt(element.getAttribute('data-index'), 10);
	let item;

	if (element.classList.contains('dummy')) {
		item = { ITID: idx, IsIdentified: 1 };
	} else if (element.classList.contains('resultitem')) {
		item = ReformUIState.resultItem;
	} else {
		item = Inventory.getUI().getItemByIndex(idx);
	}

	if (!item) {
		return;
	}

	showItemPreview(item);
}

/**
 * Displays a preview of an item and its comparison item, if available.
 *
 * @param {Object} item - The item to display.
 */
function showItemPreview(item) {
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
Network.hookPacket(PACKET.ZC.OPEN_REFORM_UI, onOpenReformUI);
Network.hookPacket(PACKET.ZC.ITEM_REFORM_ACK, onItemReformResult);

/**
 * Create component and export it
 */
export default UIManager.addComponent(ItemReform);
