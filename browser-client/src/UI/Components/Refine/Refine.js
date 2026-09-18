/**
 * UI/Components/Refine/Refine.js
 *
 * Refine Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Announce from 'UI/Components/Announce/Announce.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import 'UI/Elements/Elements.js';
import htmlText from './Refine.html?raw';
import cssText from './Refine.css?raw';

/**
 * Create Component
 */
const Refine = new GUIComponent('Refine', cssText);

/**
 * Blacksmtith's Blessing ItemID
 */
const BSB_ITID = 6635;

/**
 * Variables for current Refine Session
 */
let refiningMaterials = [];
let blacksmithBlessing = 0;
let refine_item_index = 0;
let refine_item_mat = 0;
let refine_fee = 0;
let refine_bsb = 0;
let refine_result = 0;
let refine_result_div = '';
let refine_can_cont = 0;
let refine_no_mats = 0;
let refine_no_zeny = 0;
let refine_no_bsb = 0;
let refine_item_broken = 0;
let refine_new_mats = 0;
let refine_ongoing = 0;
let refine_current_chance = 0;
let refine_current_zeny = 0;
let initialsuccess;
let currentLoopHandle;
Refine.imageLoopTimeout = 0;
Refine.messageTimeOut = 0;
Refine.hammer = 0;

/**
 * Mapping for messageID and ItemID for Refine Info
 */
const itemMessageMapping = {
	2988: [1000336, 1000355, 1000368, 1000369, 1000370, 1000371],
	2989: [6225, 6226, 1000331, 1000333],
	2990: [6223, 6624]
};

/**
 * Variables for images to play in between phases
 */
const images = {
	waiting: [
		'bg_refining_wait_00.bmp',
		'bg_refining_wait_01.bmp',
		'bg_refining_wait_02.bmp',
		'bg_refining_wait_03.bmp'
	],
	readya: ['bg_refininga_ready_00.bmp', 'bg_refininga_ready_01.bmp', 'bg_refininga_ready_02.bmp'],
	readyb: ['bg_refiningb_ready_00.bmp', 'bg_refiningb_ready_01.bmp', 'bg_refiningb_ready_02.bmp'],
	processa: [
		'bg_refininga_process_00.bmp',
		'bg_refininga_process_01.bmp',
		'bg_refininga_process_02.bmp',
		'bg_refininga_process_03.bmp',
		'bg_refininga_process_04.bmp',
		'bg_refininga_process_05.bmp',
		'bg_refininga_process_06.bmp',
		'bg_refininga_process_07.bmp',
		'bg_refininga_process_08.bmp',
		'bg_refining_process_09.bmp',
		'bg_refining_process_10.bmp',
		'bg_refining_process_11.bmp',
		'bg_refining_process_12.bmp',
		'bg_refining_process_13.bmp'
	],
	processb: [
		'bg_refiningb_process_00.bmp',
		'bg_refiningb_process_01.bmp',
		'bg_refiningb_process_02.bmp',
		'bg_refiningb_process_03.bmp',
		'bg_refiningb_process_04.bmp',
		'bg_refiningb_process_05.bmp',
		'bg_refiningb_process_06.bmp',
		'bg_refiningb_process_07.bmp',
		'bg_refiningb_process_08.bmp',
		'bg_refining_process_09.bmp',
		'bg_refining_process_10.bmp',
		'bg_refining_process_11.bmp',
		'bg_refining_process_12.bmp',
		'bg_refining_process_13.bmp'
	],
	success: [
		'bg_refining_success_00.bmp',
		'bg_refining_success_01.bmp',
		'bg_refining_success_02.bmp',
		'bg_refining_success_03.bmp',
		'bg_refining_success_04.bmp',
		'bg_refining_success_05.bmp',
		'bg_refining_success_06.bmp',
		'bg_refining_success_07.bmp',
		'bg_refining_success_08.bmp'
	],
	fail: [
		'bg_refining_fail_00.bmp',
		'bg_refining_fail_01.bmp',
		'bg_refining_fail_02.bmp',
		'bg_refining_fail_03.bmp',
		'bg_refining_fail_04.bmp',
		'bg_refining_fail_05.bmp',
		'bg_refining_fail_06.bmp',
		'bg_refining_fail_07.bmp',
		'bg_refining_fail_08.bmp',
		'bg_refining_fail_09.bmp',
		'bg_refining_fail_10.bmp',
		'bg_refining_fail_11.bmp',
		'bg_refining_fail_12.bmp',
		'bg_refining_fail_13.bmp',
		'bg_refining_fail_14.bmp'
	],
	success_wait: [
		'bg_refining_success_09.bmp',
		'bg_refining_success_10.bmp',
		'bg_refining_success_11.bmp',
		'bg_refining_success_12.bmp',
		'bg_refining_success_13.bmp',
		'bg_refining_success_14.bmp',
		'bg_refining_success_15.bmp',
		'bg_refining_success_16.bmp'
	],
	fail_wait: [
		'bg_refining_fail_15.bmp',
		'bg_refining_fail_16.bmp',
		'bg_refining_fail_17.bmp',
		'bg_refining_fail_18.bmp',
		'bg_refining_fail_19.bmp'
	]
};

/**
 * Helper: query inside shadow root
 */
function _root() {
	return Refine._shadow || Refine._host;
}

/**
 * Render HTML
 */
Refine.render = () => htmlText;

/**
 * Initialize UI
 */
Refine.init = function init() {
	const root = _root();

	this._host.style.top = '200px';
	this._host.style.left = '300px';

	const titlebarBase = root.querySelector('.titlebar .base');
	if (titlebarBase) {
		titlebarBase.addEventListener('mousedown', event => {
			event.stopImmediatePropagation();
		});
	}

	const closeBtn = root.querySelector('.titlebar .close');
	if (closeBtn) {
		closeBtn.addEventListener('click', onRefineClose);
	}

	const cancelBtn = root.querySelector('.cancel');
	if (cancelBtn) {
		cancelBtn.addEventListener('click', onRefineClose);
	}

	this.draggable('.titlebar');

	// Update success div text
	const successdiv = root.querySelector('.success');
	const initialValue = 0;
	const successtext = DB.getMessage(3724);
	initialsuccess = successtext.replace('%d%', `<span class="number">${initialValue}</span>`);
	if (successdiv) {
		successdiv.innerHTML = initialsuccess;
	}

	// Item drop/drag on .item_to_refine
	const itemToRefine = root.querySelector('.item_to_refine');
	if (itemToRefine) {
		itemToRefine.addEventListener('drop', onItemDrop);
		itemToRefine.addEventListener('dragover', event => {
			event.preventDefault();
			event.stopImmediatePropagation();
		});
		itemToRefine.addEventListener('dragstart', event => {
			const item = event.target.closest('.item');
			if (item) {
				onItemDragStart(event);
			}
		});
		itemToRefine.addEventListener('dragend', event => {
			const item = event.target.closest('.item');
			if (item) {
				onItemDragEnd(event);
			}
		});
		itemToRefine.addEventListener('dblclick', event => {
			const item = event.target.closest('.item');
			if (item && refine_ongoing === 0) {
				onRemoveItem(true);
			}
		});
		itemToRefine.addEventListener('contextmenu', event => {
			const item = event.target.closest('.item');
			if (item) {
				onItemInfo.call(item, event);
			}
		});
	}

	// Materials hover/contextmenu
	const materials = root.querySelector('.materials');
	if (materials) {
		materials.addEventListener('mouseover', event => {
			const item = event.target.closest('.item');
			if (item) {
				onItemOver.call(item, event);
			}
		});
		materials.addEventListener('mouseout', () => {
			onItemOut();
		});
		materials.addEventListener('contextmenu', event => {
			const item = event.target.closest('.item');
			if (item) {
				onItemInfo.call(item, event);
			}
		});
	}

	// Refine_cont hover
	const refineCont = root.querySelector('.refine_cont');
	if (refineCont) {
		refineCont.addEventListener('mouseover', event => {
			onItemOver.call(refineCont, event);
		});
		refineCont.addEventListener('mouseout', () => {
			onItemOut();
		});
	}

	// Refine buttons
	const refineEnabled = root.querySelector('.refine_enabled');
	if (refineEnabled) {
		refineEnabled.addEventListener('click', onRequestRefine);
	}

	const successContEnabled = root.querySelector('.success_refine_cont_enabled');
	if (successContEnabled) {
		successContEnabled.addEventListener('click', onRequestRefine);
	}

	const failContEnabled = root.querySelector('.fail_refine_cont_enabled');
	if (failContEnabled) {
		failContEnabled.addEventListener('click', onRequestRefine);
	}

	const backButton = root.querySelector('.back_button');
	if (backButton) {
		backButton.addEventListener('click', onCancelContRefine);
	}

	// Hide buttons
	if (refineEnabled) {
		refineEnabled.style.display = 'none';
	}

	const someNotifs = root.querySelector('.some_notifs');
	if (someNotifs) {
		someNotifs.style.display = 'none';
	}

	onHideContRefineButtons();
};

/**
 * Append to body
 */
Refine.onAppend = function onAppend() {
	const root = _root();

	Refine.hammer = 0;
	const refineButton = root.querySelector('.refine_button');
	if (refineButton) {
		refineButton.style.display = 'block';
	}

	clearTimeout(Refine.imageLoopTimeout);
	controlPhase('waiting', true, 250);
};

/**
 * Remove Refine Window (and so clean up items)
 */
Refine.onRemove = function onRemove() {
	clearTimeout(Refine.imageLoopTimeout);
	clearTimeout(currentLoopHandle);
	onRemoveItem(true);
	onHideContRefineButtons();
	clearRefineStates();
};

/**
 * Completely clean-up variables for refining
 */
function clearRefineStates() {
	Refine.hammer = 0;
	Refine.imageLoopTimeout = 0;
	Refine.messageTimeOut = 0;
	currentLoopHandle = null;
	refiningMaterials = [];
	blacksmithBlessing = 0;
	refine_item_index = 0;
	refine_item_mat = 0;
	refine_fee = 0;
	refine_bsb = 0;
	refine_result = 0;
	refine_result_div = '';
	refine_can_cont = 0;
	refine_no_mats = 0;
	refine_no_zeny = 0;
	refine_no_bsb = 0;
	refine_new_mats = 0;
	refine_current_chance = 0;
	refine_current_zeny = 0;
}

/**
 * Handles packet received from server to open Refine UI
 * PACKET.ZC.OPEN_REFINING_UI
 */
function onOpenRefineUI() {
	if (!Configs.get('enableRefineUI') || PACKETVER.value < 20161012) {
		console.warn('Renewal Refine is enabled in your server. Please enable refine UI in your configs.');
		return false;
	}

	Refine.append();

	const isInventoryOpen = Inventory.getUI().ui ? Inventory.getUI().ui.is(':visible') : false;

	if (!isInventoryOpen) {
		Inventory.getUI().toggle();
	}

	const refineRect = Refine._host.getBoundingClientRect();
	const refineWidth = Refine._host.offsetWidth;
	const refineHeight = Refine._host.offsetHeight - Inventory.getUI().ui.height();

	Inventory.getUI().ui.css({
		position: 'absolute',
		top: refineRect.top ? refineRect.top + refineHeight : 200,
		left: refineRect.left ? refineRect.left + refineWidth : 300
	});

	return false;
}

/**
 * Handles sending request to server to close Refine UI
 */
function onRefineClose() {
	Refine.remove();

	const pkt = new PACKET.CZ.CLOSE_REFINING_UI();
	Network.sendPacket(pkt);
}

/**
 * Function to control phases and image looping
 * @param {string} phase - The phase to control
 * @param {boolean} shouldLoop - Whether the phase should loop
 * @param {number} interval - Interval between image changes in milliseconds
 * @param {function} callback - A callback function to execute after the phase completes
 */
function controlPhase(phase, shouldLoop, interval, callback) {
	let currentImageIndex = 0;
	const imageArray = images[phase];

	if (!imageArray) {
		console.error('Invalid phase:', phase);
		return;
	}

	function showImages() {
		Client.loadFile(DB.INTERFACE_PATH + 'refining_renewal/' + imageArray[currentImageIndex], function (data) {
			const container = _root().querySelector('.image-container');
			if (container) {
				container.style.backgroundImage = `url(${data})`;
			}
			currentImageIndex++;

			if (currentImageIndex >= imageArray.length) {
				if (shouldLoop) {
					currentImageIndex = 0;
				} else {
					if (callback && typeof callback === 'function') {
						callback();
					}
					return;
				}
			}

			Refine.imageLoopTimeout = setTimeout(showImages, interval);
		});
	}

	showImages();
}

/**
 * Drop an item from inventory to Refine UI
 */
function onItemDrop(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	if (data.type !== 'item' || data.from !== 'Inventory') {
		return;
	}

	if (item) {
		Refine.onRequestItemRefine(item);
	}
}

/**
 * Handles sending the server packet request to refine an item
 */
Refine.onRequestItemRefine = function onRequestItemRefine(item) {
	const pkt = new PACKET.CZ.REFINING_SELECT_ITEM();
	pkt.index = item.index;
	Network.sendPacket(pkt);
};

/**
 * Handles the packet received from the server
 * @param {pkt} - PACKET.ZC.REFINING_MATERIAL_LIST
 */
function onRefineUIUpdateMaterials(pkt) {
	if (!Configs.get('enableRefineUI') || PACKETVER.value < 20161012) {
		console.warn('Renewal Refine is enabled in your server. Please enable refine UI in your configs.');
		return false;
	}

	const root = _root();

	if (pkt && pkt.MaterialInfo.length > 0) {
		const existingItem = root.querySelector('.item_to_refine .item');
		if (existingItem) {
			onRemoveItem(false);
		}

		refine_item_index = pkt.itemIndex;
		const item = Inventory.getUI().getItemByIndex(pkt.itemIndex);

		refiningMaterials = pkt.MaterialInfo;
		blacksmithBlessing = pkt.blacksmithBlessing;

		if (!Refine.hammer) {
			onPopulateMaterials();

			clearTimeout(Refine.imageLoopTimeout);
			const isbsbenabled = blacksmithBlessing ? 'a' : 'b';
			controlPhase(`ready${isbsbenabled}`, false, 250);
		}

		const it = DB.getItemInfo(item.ITID);
		const content = root.querySelector('.item_to_refine');

		content.insertAdjacentHTML(
			'beforeend',
			`<div class="item" data-index="${item.index}" draggable="true">` +
				'<div class="icon"></div>' +
				'<div class="grade"></div>' +
				'</div>'
		);

		Client.loadFile(
			DB.INTERFACE_PATH +
				'item/' +
				(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
				'.bmp',
			function (data) {
				const icon = content.querySelector(`.item[data-index="${item.index}"] .icon`);
				if (icon) {
					icon.style.backgroundImage = `url(${data})`;
				}
			}
		);

		if (item.enchantgrade) {
			Client.loadFile(DB.INTERFACE_PATH + `grade_enchant/grade_icon${item.enchantgrade}.bmp`, function (data) {
				const grade = content.querySelector(`.item[data-index="${item.index}"] .grade`);
				if (grade) {
					grade.style.backgroundImage = `url(${data})`;
				}
			});
		}

		const itemname = root.querySelector('.item_to_refine_name');
		if (itemname) {
			itemname.textContent = DB.getItemName(item);
		}

		// Select previously selected material if available
		if (Refine.hammer >= 1 && refine_item_mat) {
			let materialFound = false;
			let foundItem, foundMaterial;

			for (let i = 0; i < refiningMaterials.length; i++) {
				foundMaterial = refiningMaterials[i];
				if (foundMaterial.itemId === refine_item_mat) {
					foundItem = Inventory.getUI().getItemById(foundMaterial.itemId);
					materialFound = true;
					refine_new_mats = 0;
					break;
				}
			}

			if (!materialFound) {
				refine_new_mats = 1;
				showMessage(3242, 3, 'error');
			}

			if (refine_bsb) {
				if (blacksmithBlessing === 0) {
					refine_new_mats = 1;
					showMessage(3242, 3, 'error');
				}
				refine_bsb = blacksmithBlessing;
			}

			selectMaterial(foundMaterial, foundItem);
		}
	} else {
		showMessage(2970, 3, 'error');
		return false;
	}
}

/**
 * Handles populating materials needed for refining
 */
function onPopulateMaterials() {
	const root = _root();

	// Clear any existing materials
	root.querySelectorAll(
		'.materials .mat_overlay .material_0, .materials .mat_overlay .material_1, .materials .mat_overlay .material_2, .materials .mat_overlay .material_3'
	).forEach(el => {
		el.innerHTML = '';
	});
	const bsbContainer = root.querySelector('.bsb_overlay .bsb');
	if (bsbContainer) {
		bsbContainer.innerHTML = '';
	}

	// Update materials
	for (let i = 0; i < refiningMaterials.length; i++) {
		(function (idx) {
			const material = refiningMaterials[idx];
			const it = DB.getItemInfo(material.itemId);
			const item = Inventory.getUI().getItemById(material.itemId);
			const materialDiv = root.querySelector(`.material_${idx}`);

			if (!materialDiv) {
				return;
			}

			materialDiv.innerHTML = '';

			materialDiv.insertAdjacentHTML(
				'beforeend',
				`<div class="item" data-index="${material.itemId}" draggable="false">` +
					'<div class="icon"></div>' +
					'<div class="mat_count"></div>' +
					'</div>'
			);

			Client.loadFile(
				DB.INTERFACE_PATH +
					'item/' +
					(it.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
					'.bmp',
				function (data) {
					const icon = materialDiv.querySelector(`.item[data-index="${material.itemId}"] .icon`);
					if (icon) {
						icon.style.backgroundImage = `url(${data})`;
					}
				}
			);

			const count = item ? item.count : 0;
			const countmsg = materialDiv.querySelector(`.item[data-index="${material.itemId}"] .mat_count`);
			if (countmsg) {
				if (count === 0) {
					countmsg.innerHTML = `<span style="color: #ce1029;">${count}</span>/1`;
				} else {
					countmsg.textContent = `${count}/1`;
				}
			}

			// Material Selection upon clicking
			const iconEl = materialDiv.querySelector('.icon');
			if (iconEl) {
				iconEl.addEventListener('click', () => {
					const clickedItemId = material.itemId;
					const clickedItem = Inventory.getUI().getItemById(clickedItemId);
					const clickedCount = clickedItem ? clickedItem.count : 0;

					if (clickedCount === 0) {
						return;
					}

					// Check if item ID exists in the mapping and show corresponding message
					for (const messageID in itemMessageMapping) {
						if (itemMessageMapping[messageID].includes(clickedItemId)) {
							showMessage(messageID, 0, 'info');
							break;
						}
					}

					root.querySelectorAll('.mat_overlay').forEach(el => el.classList.remove('selected'));
					materialDiv.closest('.mat_overlay').classList.add('selected');

					// Clone the material and append it to the selected_mat
					const originalItem = materialDiv.querySelector('.item');
					const clonedMaterial = originalItem.cloneNode(true);
					const matCount = clonedMaterial.querySelector('.mat_count');
					if (matCount) {
						matCount.remove();
					}

					const selectedMat = root.querySelector('.selected_mat');
					selectedMat.innerHTML = '';
					selectedMat.appendChild(clonedMaterial);

					// Enable refine button
					const refineEnabled = root.querySelector('.refine_enabled');
					if (refineEnabled) {
						refineEnabled.style.display = 'block';
					}

					// Update success and refining_zeny
					const numberEl = root.querySelector('.success .number');
					if (numberEl) {
						numberEl.textContent = material.chance;
					}
					const chanceRate = root.querySelector('.chance_rate');
					if (chanceRate) {
						chanceRate.textContent = DB.getMessage(3285).replace('%d%', material.chance);
					}
					const refineZeny = root.querySelector('.refine_zeny');
					if (refineZeny) {
						refineZeny.textContent = material.zeny;
					}
					const refineZenyCont = root.querySelector('.refine_zeny_cont');
					if (refineZenyCont) {
						refineZenyCont.textContent = material.zeny;
					}

					// Store fresh chance/zeny for re-apply after the result animation
					refine_current_chance = material.chance;
					refine_current_zeny = material.zeny;

					// Set the refine_item_mat value to the selected material itemId
					refine_item_mat = material.itemId;
					refine_fee = material.zeny;

					const refineCont = root.querySelector('.refine_cont');
					if (refineCont) {
						refineCont.classList.add('item');
						refineCont.setAttribute('data-index', material.itemId);
					}
				});
			}
		})(i);
	}

	// Update blacksmith blessing if applicable
	if (blacksmithBlessing) {
		const bsbDiv = root.querySelector('.bsb_overlay .bsb');
		const bsbItem = DB.getItemInfo(BSB_ITID);
		const item = Inventory.getUI().getItemById(BSB_ITID);
		bsbDiv.insertAdjacentHTML(
			'beforeend',
			`<div class="item" data-index="${BSB_ITID}" draggable="false">` +
				'<div class="icon"></div>' +
				'<div class="mat_count"></div>' +
				'</div>'
		);

		Client.loadFile(
			DB.INTERFACE_PATH +
				'item/' +
				(bsbItem.IsIdentified ? bsbItem.identifiedResourceName : bsbItem.unidentifiedResourceName) +
				'.bmp',
			function (data) {
				const icon = bsbDiv.querySelector(`.item[data-index="${BSB_ITID}"] .icon`);
				if (icon) {
					icon.style.backgroundImage = `url(${data})`;
				}
			}
		);

		const bsbCountOuter = item ? item.count : 0;
		const bsbcountmsg = bsbDiv.querySelector(`.item[data-index="${BSB_ITID}"] .mat_count`);
		if (bsbcountmsg) {
			if (bsbCountOuter === 0) {
				bsbcountmsg.innerHTML = `<span style="color: #ce1029;">${bsbCountOuter}</span>/${blacksmithBlessing}`;
			} else {
				bsbcountmsg.textContent = `${bsbCountOuter}/${blacksmithBlessing}`;
			}
		}

		// Add select functionality
		const bsbIcon = bsbDiv.querySelector('.icon');
		if (bsbIcon) {
			bsbIcon.addEventListener('click', () => {
				const bsbInventoryItem = Inventory.getUI().getItemById(BSB_ITID);
				const currentBsbCount = bsbInventoryItem ? bsbInventoryItem.count : 0;

				if (currentBsbCount >= blacksmithBlessing) {
					const bsbOverlay = bsbDiv.closest('.bsb_overlay');

					if (bsbOverlay.classList.contains('selected')) {
						bsbOverlay.classList.remove('selected');
						const bsbSelected = root.querySelector('.bsb_selected');
						if (bsbSelected) {
							bsbSelected.innerHTML = '';
						}
						const someNotifs = root.querySelector('.some_notifs');
						if (someNotifs) {
							someNotifs.style.display = 'none';
						}
					} else {
						root.querySelectorAll('.bsb_overlay').forEach(el => el.classList.remove('selected'));
						bsbOverlay.classList.add('selected');

						// Clone the blacksmith blessing and append it to the selected_mat
						const originalItem = bsbDiv.querySelector('.item');
						const clonedBSB = originalItem.cloneNode(true);
						const matCount = clonedBSB.querySelector('.mat_count');
						if (matCount) {
							matCount.remove();
						}

						const bsbSelected = root.querySelector('.bsb_selected');
						if (bsbSelected) {
							bsbSelected.innerHTML = '';
							bsbSelected.appendChild(clonedBSB);
						}

						// BSB will be used
						refine_bsb = blacksmithBlessing;

						// Show Info
						showMessage(2967, 0, 'info');
					}
				} else {
					showMessage(2969, 3, 'error');
				}
			});
		}
	}
}

/**
 * Handles update of variables for UI changes based on Refine result
 */
function selectMaterial(material, item) {
	const root = _root();
	const count = item ? item.count || 0 : 0;

	refine_can_cont = 1;

	if (count) {
		refine_no_mats = 0;
	} else {
		refine_no_mats = 1;
		refine_can_cont = 0;
	}

	if (refine_bsb) {
		const bsbinInventory = Inventory.getUI().getItemById(BSB_ITID);
		const bsbCount = bsbinInventory ? bsbinInventory.count || 0 : 0;

		if (!bsbinInventory || bsbCount < refine_bsb) {
			refine_no_bsb = 1;
			refine_can_cont = 0;
		} else {
			refine_no_bsb = 0;
		}
	}

	refine_fee = material.zeny;
	if (Session.zeny < material.zeny) {
		refine_no_zeny = 1;
		refine_can_cont = 0;
	}

	if (onCheckItemBroken()) {
		refine_can_cont = 0;
	}

	refine_current_chance = material.chance;
	refine_current_zeny = material.zeny;

	const chanceRate = root.querySelector('.chance_rate');
	if (chanceRate) {
		chanceRate.textContent = DB.getMessage(3285).replace('%d%', material.chance);
	}
	const refineZenyCont = root.querySelector('.refine_zeny_cont');
	if (refineZenyCont) {
		refineZenyCont.textContent = material.zeny;
	}

	if (refine_can_cont && !refine_new_mats && !refine_item_broken) {
		refine_result_div = refine_result ? 'fail_refine_cont_enabled' : 'success_refine_cont_enabled';
	} else {
		refine_result_div = refine_result ? 'fail_refine_cont_disabled' : 'success_refine_cont_disabled';
		const refineCont = root.querySelector('.refine_cont');
		if (refineCont) {
			refineCont.classList.remove('item');
			refineCont.removeAttribute('data-index');
		}
	}
}

/**
 * Show item name when mouse is over
 */
function onItemOver(event) {
	const root = _root();
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const it = DB.getItemInfo(idx);

	if (!idx) {
		return;
	}

	const itemRect = this.getBoundingClientRect();
	const overlay = root.querySelector('.overlay');
	if (!overlay) {
		return;
	}

	const parentContainer = this.closest('.materials') || this.closest('.refine_cont');
	if (!parentContainer) {
		return;
	}
	const containerRect = parentContainer.getBoundingClientRect();

	let top, left;
	if (parentContainer.classList.contains('materials')) {
		top = itemRect.top - containerRect.top + 30;
		left = itemRect.left - containerRect.left + this.offsetWidth - 39;
	} else if (parentContainer.classList.contains('refine_cont')) {
		top = itemRect.top - containerRect.top + 242;
		left = itemRect.left - containerRect.left + this.offsetWidth + 41;
	}

	overlay.style.display = 'block';
	overlay.style.top = `${top}px`;
	overlay.style.left = `${left}px`;
	overlay.textContent = it.identifiedDisplayName;

	if (it.IsIdentified) {
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
 * Handles clearance of variables and UI components when changing Item
 */
function onRemoveItem(backtowait) {
	const root = _root();

	refiningMaterials = [];
	blacksmithBlessing = 0;

	const itemToRefine = root.querySelector('.item_to_refine');
	if (itemToRefine) {
		itemToRefine.innerHTML = '';
	}
	const itemToRefineName = root.querySelector('.item_to_refine_name');
	if (itemToRefineName) {
		itemToRefineName.textContent = '';
	}

	const successNumber = root.querySelector('.success .number');
	if (successNumber) {
		successNumber.textContent = '0';
	}
	const refineZeny = root.querySelector('.refine_zeny');
	if (refineZeny) {
		refineZeny.innerHTML = '';
	}

	const selectedMat = root.querySelector('.selected_mat');
	if (selectedMat) {
		selectedMat.innerHTML = '';
	}
	const bsbSelected = root.querySelector('.bsb_selected');
	if (bsbSelected) {
		bsbSelected.innerHTML = '';
	}
	root.querySelectorAll('.materials .item').forEach(el => {
		el.innerHTML = '';
	});
	root.querySelectorAll('.bsb .item').forEach(el => {
		el.innerHTML = '';
	});
	root.querySelectorAll('.mat_overlay').forEach(el => el.classList.remove('selected'));
	root.querySelectorAll('.bsb_overlay').forEach(el => el.classList.remove('selected'));

	if (backtowait === true) {
		if (currentLoopHandle) {
			clearTimeout(currentLoopHandle);
			currentLoopHandle = null;
		}
		if (Refine.imageLoopTimeout) {
			clearTimeout(Refine.imageLoopTimeout);
			Refine.imageLoopTimeout = null;
		}

		controlPhase('waiting', true, 250);

		const someNotifs = root.querySelector('.some_notifs');
		if (someNotifs) {
			someNotifs.style.display = 'none';
		}
		const refineButton = root.querySelector('.refine_button');
		if (refineButton) {
			refineButton.style.display = 'block';
		}
		const successEl = root.querySelector('.success');
		if (successEl) {
			successEl.innerHTML = initialsuccess;
			successEl.style.display = 'block';
		}

		onHideContRefineButtons();
		clearRefineStates();
	}
}

/**
 * Handles clearing of material components after requesting to refine
 */
function clearMaterials() {
	const root = _root();

	const successEl = root.querySelector('.success');
	if (successEl) {
		successEl.innerHTML = '';
		successEl.style.display = 'none';
	}
	const refineZeny = root.querySelector('.refine_zeny');
	if (refineZeny) {
		refineZeny.innerHTML = '';
	}

	root.querySelectorAll('.materials .item').forEach(el => {
		el.style.display = 'none';
	});
	root.querySelectorAll('.bsb .item').forEach(el => {
		el.style.display = 'none';
	});

	const selectedMat = root.querySelector('.selected_mat');
	if (selectedMat) {
		selectedMat.innerHTML = '';
	}
	const bsbSelected = root.querySelector('.bsb_selected');
	if (bsbSelected) {
		bsbSelected.innerHTML = '';
	}

	root.querySelectorAll('.mat_overlay').forEach(el => el.classList.remove('selected'));
	root.querySelectorAll('.bsb_overlay').forEach(el => el.classList.remove('selected'));

	const refineButton = root.querySelector('.refine_button');
	if (refineButton) {
		refineButton.style.display = 'none';
	}
}

/**
 * Show a message in .info_msg
 * @param {number} messageID - The message ID to retrieve from DB
 * @param {number} timeout - The duration to show the message, in seconds (0 for infinite)
 * @param {string} type - The type of message ('error' or 'info')
 */
function showMessage(messageID, timeout, type) {
	const root = _root();
	const message = DB.getMessage(messageID);
	let messageClass;

	switch (type) {
		case 'error':
			messageClass = 'red';
			break;
		case 'info':
			messageClass = 'blue';
			break;
		default:
			messageClass = 'red';
			break;
	}

	if (Refine.messageTimeOut) {
		clearTimeout(Refine.messageTimeOut);
	}

	const infoMsg = root.querySelector('.info_msg');
	if (infoMsg) {
		infoMsg.classList.remove('red', 'blue');
		infoMsg.textContent = message;
		infoMsg.classList.add(messageClass);
	}

	const someNotifs = root.querySelector('.some_notifs');
	if (someNotifs) {
		someNotifs.style.display = 'block';
	}

	if (timeout > 0) {
		Refine.messageTimeOut = setTimeout(() => {
			if (infoMsg) {
				infoMsg.classList.remove(messageClass);
			}
			if (someNotifs) {
				someNotifs.style.display = 'none';
			}
		}, timeout * 1000);
	}
}

/**
 * Send the server request to refine the item
 */
function onRequestRefine() {
	const root = _root();
	const item = Inventory.getUI().getItemByIndex(refine_item_index);
	const material = Inventory.getUI().getItemById(refine_item_mat);

	if (!item) {
		return;
	}

	if (!material) {
		return;
	}

	if (Session.zeny < refine_fee) {
		showMessage(2968, 3, 'error');
		return;
	}

	if (!Refine.hammer) {
		clearMaterials();
	}

	Refine.hammer++;

	const refineButton = root.querySelector('.refine_button');
	if (refineButton) {
		refineButton.style.display = 'none';
	}

	const itemToRefineName = root.querySelector('.item_to_refine_name');
	if (itemToRefineName) {
		itemToRefineName.style.display = 'none';
	}
	const successEl = root.querySelector('.success');
	if (successEl) {
		successEl.style.display = 'none';
	}

	refine_ongoing = 1;

	const pkt = new PACKET.CZ.REQ_REFINING();
	pkt.index = refine_item_index;
	pkt.itemId = refine_item_mat;
	pkt.blacksmithBlessing = refine_bsb;
	Network.sendPacket(pkt);
}

/**
 * Handles the received refine result packet
 * @param {pkt} - PACKET.ZC.ACK_ITEMREFINING
 */
Refine.onRefineResult = function onRefineResult(pkt) {
	const root = _root();

	if (pkt) {
		const backButton = root.querySelector('.back_button');
		if (backButton) {
			backButton.style.display = 'none';
		}
		const refineCont = root.querySelector('.refine_cont');
		if (refineCont) {
			refineCont.style.display = 'none';
		}

		const item = Inventory.getUI().removeItem(pkt.itemIndex, 1);
		if (item) {
			item.RefiningLevel = pkt.RefiningLevel;
			Inventory.getUI().addItem(item);
		}

		stopCurrentLoop();

		refine_result = pkt.result;

		switch (pkt.result) {
			case 0:
				refine_can_cont = 1;
				onAnimateResult('success', () => {
					onUpdateRefineUI('success');
					startLoopingPhase('success_wait');
				});
				break;
			case 1:
			case 3:
				onShowFailure(pkt.result);
				break;
			case 2:
				onShowFailure(pkt.result);
				break;
		}
	}
};

/**
 * Handles animation for failure and pass to UI if downgrade or fail
 */
function onShowFailure(result) {
	const showResult = result === 2 ? 'downgrade' : 'fail';

	onAnimateResult('fail', () => {
		onUpdateRefineUI(showResult);
		startLoopingPhase('fail_wait');
	});
}

/**
 * Handles animation sequences depending on Refine result
 */
function onAnimateResult(result, callback) {
	function runSuccessSequence() {
		if (refine_bsb) {
			controlPhase('processa', false, 50, () => {
				controlPhase('success', false, 50, callback);
			});
		} else {
			controlPhase('processb', false, 50, () => {
				controlPhase('success', false, 50, callback);
			});
		}
	}

	function runFailSequence() {
		if (refine_bsb) {
			controlPhase('processa', false, 50, () => {
				controlPhase('fail', false, 50, callback);
			});
		} else {
			controlPhase('processb', false, 50, () => {
				controlPhase('fail', false, 50, callback);
			});
		}
	}

	switch (result) {
		case 'success':
			runSuccessSequence();
			break;
		case 'fail':
			runFailSequence();
			break;
		default:
			if (callback) {
				callback();
			}
			break;
	}
}

/**
 * Store and start phase animation
 */
function startLoopingPhase(phase) {
	currentLoopHandle = controlPhase(phase, true, 75);
}

/**
 * Function to stop current stored on-going phase animation
 */
function stopCurrentLoop() {
	if (currentLoopHandle) {
		clearTimeout(currentLoopHandle);
		currentLoopHandle = null;
	}
	if (Refine.imageLoopTimeout) {
		clearTimeout(Refine.imageLoopTimeout);
		Refine.imageLoopTimeout = null;
	}
}

/**
 * Handles how the UI changes depending on Refine result
 */
function onUpdateRefineUI(result) {
	const root = _root();

	onHideContRefineButtons();

	switch (result) {
		case 'success': {
			const backSuccess = root.querySelector('.back_success');
			if (backSuccess) {
				backSuccess.style.display = 'block';
			}
			const successEl = root.querySelector('.success');
			if (successEl) {
				successEl.textContent = DB.getMessage(2971);
				successEl.style.display = 'block';
			}
			ChatBox.addText(DB.getMessage(498), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break;
		}
		case 'fail': {
			onCheckItemBroken();
			const backFail = root.querySelector('.back_fail');
			if (backFail) {
				backFail.style.display = 'block';
			}
			const successEl = root.querySelector('.success');
			if (successEl) {
				successEl.textContent = DB.getMessage(2972);
				successEl.style.display = 'block';
			}
			ChatBox.addText(DB.getMessage(499), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break;
		}
		case 'downgrade': {
			onCheckItemBroken();
			const backFail = root.querySelector('.back_fail');
			if (backFail) {
				backFail.style.display = 'block';
			}
			const successEl = root.querySelector('.success');
			if (successEl) {
				successEl.textContent = DB.getMessage(2972);
				successEl.style.display = 'block';
			}
			ChatBox.addText(DB.getMessage(1537), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break;
		}
		default:
			break;
	}

	const refineTextCont = root.querySelector('.refine_text_cont');
	if (refineTextCont) {
		refineTextCont.style.display = 'block';
	}

	if (refine_result_div) {
		const resultEl = root.querySelector(`.${refine_result_div}`);
		if (resultEl) {
			resultEl.style.display = 'block';
		}
	}

	if (refine_can_cont && !refine_new_mats && !refine_item_broken) {
		const chanceRate = root.querySelector('.chance_rate');
		if (chanceRate) {
			chanceRate.textContent = DB.getMessage(3285).replace('%d%', refine_current_chance);
			chanceRate.style.display = 'block';
		}
		const refineZenyCont = root.querySelector('.refine_zeny_cont');
		if (refineZenyCont) {
			refineZenyCont.textContent = refine_current_zeny;
			refineZenyCont.style.display = 'block';
		}
	}

	if (refine_no_mats) {
		showMessage(3243, 3, 'error');
	}
	if (refine_no_zeny) {
		showMessage(3244, 3, 'error');
	}
	if (refine_no_bsb) {
		showMessage(3245, 3, 'error');
	}

	const backButton = root.querySelector('.back_button');
	if (backButton) {
		backButton.style.display = 'block';
	}
	const refineCont = root.querySelector('.refine_cont');
	if (refineCont) {
		refineCont.style.display = 'block';
	}

	if (!refine_item_broken) {
		const refineditem = Inventory.getUI().getItemByIndex(refine_item_index);
		if (refineditem) {
			const itemToRefineName = root.querySelector('.item_to_refine_name');
			if (itemToRefineName) {
				itemToRefineName.textContent = DB.getItemName(refineditem);
			}
		}
	}

	const itemToRefineName = root.querySelector('.item_to_refine_name');
	if (itemToRefineName) {
		itemToRefineName.style.display = 'block';
	}

	refine_ongoing = 0;
}

/**
 * Handles hiding all buttons used in Continous Refine
 */
function onHideContRefineButtons() {
	const root = _root();

	const backButton = root.querySelector('.back_button');
	if (backButton) {
		backButton.style.display = 'none';
	}
	const refineCont = root.querySelector('.refine_cont');
	if (refineCont) {
		refineCont.style.display = 'none';
	}
	const backSuccess = root.querySelector('.back_success');
	if (backSuccess) {
		backSuccess.style.display = 'none';
	}
	const backFail = root.querySelector('.back_fail');
	if (backFail) {
		backFail.style.display = 'none';
	}
	const successContEnabled = root.querySelector('.success_refine_cont_enabled');
	if (successContEnabled) {
		successContEnabled.style.display = 'none';
	}
	const successContDisabled = root.querySelector('.success_refine_cont_disabled');
	if (successContDisabled) {
		successContDisabled.style.display = 'none';
	}
	const failContEnabled = root.querySelector('.fail_refine_cont_enabled');
	if (failContEnabled) {
		failContEnabled.style.display = 'none';
	}
	const failContDisabled = root.querySelector('.fail_refine_cont_disabled');
	if (failContDisabled) {
		failContDisabled.style.display = 'none';
	}
	const refineTextCont = root.querySelector('.refine_text_cont');
	if (refineTextCont) {
		refineTextCont.style.display = 'none';
	}
	const chanceRate = root.querySelector('.chance_rate');
	if (chanceRate) {
		chanceRate.style.display = 'none';
	}
	const refineZenyCont = root.querySelector('.refine_zeny_cont');
	if (refineZenyCont) {
		refineZenyCont.style.display = 'none';
	}
}

/**
 * Check if the item being refined is broken
 */
function onCheckItemBroken() {
	const root = _root();
	const refineditem = Inventory.getUI().getItemByIndex(refine_item_index);
	if (!refineditem) {
		refine_result_div = 'fail_refine_cont_disabled';
		const itemToRefineName = root.querySelector('.item_to_refine_name');
		if (itemToRefineName) {
			itemToRefineName.textContent = DB.getMessage(3246);
		}
		refine_item_broken = 1;
		return true;
	} else {
		refine_item_broken = 0;
		return false;
	}
}

/**
 * Handles event when Back button was pressed during Continous Refine
 */
function onCancelContRefine() {
	const root = _root();

	Refine.hammer = 0;
	onHideContRefineButtons();
	stopCurrentLoop();

	if (!refine_item_broken) {
		onPopulateMaterials();
		const isbsbenabled = blacksmithBlessing ? 'a' : 'b';
		controlPhase(`ready${isbsbenabled}`, false, 250);
	} else {
		onRemoveItem(true);
	}

	const refineButton = root.querySelector('.refine_button');
	if (refineButton) {
		refineButton.style.display = 'block';
	}
	const successEl = root.querySelector('.success');
	if (successEl) {
		successEl.innerHTML = initialsuccess;
		successEl.style.display = 'block';
	}
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const ITID = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemById(ITID)
		? Inventory.getUI().getItemById(ITID)
		: Inventory.getUI().getItemByIndex(ITID);

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
 * Check if Refine UI is open
 */
Refine.isRefineOpen = function isRefineOpen() {
	return !!(Refine._host && Refine._host.isConnected);
};

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	event.dataTransfer.setData('text', event.target.id);
}

/**
 * End of dragging an item
 */
function onItemDragEnd(event) {
	if (refine_ongoing) {
		return;
	}

	const rect = Refine._host.getBoundingClientRect();
	const mouseX = event.clientX;
	const mouseY = event.clientY;

	if (mouseX < rect.left || mouseX > rect.right || mouseY < rect.top || mouseY > rect.bottom) {
		onRemoveItem(true);
	}
}

/**
 * Refine announcement received
 * @param {object} pkt - PACKET.ZC.BROADCAST_ITEMREFINING_RESULT
 */
function onBroadcastRefineResult(pkt) {
	if (pkt) {
		const item = DB.getItemInfo(pkt.itemId);
		const itemName = item.identifiedDisplayName;
		let messageID;
		switch (pkt.status) {
			case 0:
				messageID = 3272;
				break;
			case 1:
				messageID = 3271;
				break;
			default:
				break;
		}

		const message = DB.getMessage(messageID)
			.replace('%s', pkt.charName)
			.replace('%d', pkt.refineLevel)
			.replace('%s', itemName);
		ChatBox.addText(message, ChatBox.TYPE.ANNOUNCE, ChatBox.FILTER.PUBLIC_CHAT, '#FFB563');
		Announce.append();
		Announce.set(message, '#FFB563');
	}
}

/**
 * Packet Hooks to functions
 */
Network.hookPacket(PACKET.ZC.OPEN_REFINING_UI, onOpenRefineUI);
Network.hookPacket(PACKET.ZC.REFINING_MATERIAL_LIST, onRefineUIUpdateMaterials);
Network.hookPacket(PACKET.ZC.BROADCAST_ITEMREFINING_RESULT, onBroadcastRefineResult);

/**
 * Create component and export it
 */
export default UIManager.addComponent(Refine);
