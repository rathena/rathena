/**
 * UI/Components/SwitchEquip/SwitchEquip.js
 *
 * Chararacter Switch Equipment Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import Camera from 'Renderer/Camera.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import htmlText from './SwitchEquip.html?raw';
import cssText from './SwitchEquip.css?raw';
import Entity from 'Renderer/Entity/Entity.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';

/**
 * Create Component
 */
const SwitchEquip = new GUIComponent('SwitchEquip', cssText);

SwitchEquip.render = () => htmlText;

/**
 * Escape HTML entities
 */
function _escapeHtml(str) {
	const div = document.createElement('div');
	div.appendChild(document.createTextNode(str));
	return div.innerHTML;
}

/**
 * @var {Array} switchequipment list
 */
SwitchEquip._list = {};

/**
 * @var {CanvasRenderingContext2D} canvas context
 */
const _swapctx = [];

/**
 * Initialize UI
 */
SwitchEquip.init = function init() {
	const root = SwitchEquip.getRoot();
	const canvases = root.querySelectorAll('canvas');
	_swapctx.push(canvases[0].getContext('2d'));
	_swapctx.push(canvases[1].getContext('2d'));

	const panel = root.querySelector('.panel');
	if (panel) {
		panel.addEventListener('dragover', onDragOver);
		panel.addEventListener('dragleave', onDragLeave);
		panel.addEventListener('drop', onDrop);
	}

	const closeBtn = root.querySelector('.closeswap');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => {
			SwitchEquip.toggle();
		});
	}

	const swapBtn = root.querySelector('.onswap');
	if (swapBtn) {
		swapBtn.addEventListener('click', () => {
			SwitchEquip.RequestSwitch();
		});
	}

	// Set the active tab based on Equipment UI's current tab
	const currentEquipTabId = Equipment.getUI().getCurrentTabId();
	SwitchEquip.showSwapTab(currentEquipTabId);

	// Bind items on swapcontent tables
	const swapcontents = root.querySelectorAll('.swapcontent');
	for (const content of swapcontents) {
		content.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) onSwitchEquipInfo.call(item, e);
		});
		content.addEventListener('dblclick', e => {
			const item = e.target.closest('.item');
			if (item) onSwitchEquipUnEquip.call(item, e);
		});
		content.addEventListener('mouseover', e => {
			const btn = e.target.closest('button');
			if (btn) onSwitchEquipOver.call(btn, e);
		});
		content.addEventListener('mouseout', e => {
			const btn = e.target.closest('button');
			if (btn) onSwitchEquipOut();
		});
	}
};

/**
 * Show the swap tab with the given tabId, hiding other tabs.
 *
 * @param {string} tabId - The ID of the tab to show.
 */
SwitchEquip.showSwapTab = function showSwapTab(tabId) {
	const root = SwitchEquip.getRoot();
	if (!root) return;

	const swapTabId = 'swap' + tabId;
	const swapContentDivs = {
		swapgeneral: root.querySelector('#swapgeneral'),
		swapcostume: root.querySelector('#swapcostume')
	};

	for (const id in swapContentDivs) {
		if (swapContentDivs[id]) {
			if (id === swapTabId) {
				swapContentDivs[id].classList.remove('hide');
			} else {
				swapContentDivs[id].classList.add('hide');
			}
		}
	}
};

/**
 * Append to body
 */
SwitchEquip.onAppend = function onAppend() {
	// Set the active tab based on Equipment UI's current tab
	const currentEquipTabId = Equipment.getUI().getCurrentTabId();
	SwitchEquip.showSwapTab(currentEquipTabId);

	const root = SwitchEquip.getRoot();
	const canvas = root ? root.querySelector('canvas') : null;
	if (canvas && this._host.style.display !== 'none') {
		Renderer.render(swaprender);
	}
};

/**
 * Remove Inventory from window (and so clean up items)
 */
SwitchEquip.onRemove = function onRemove() {
	Renderer.stop(swaprender);
	SwitchEquip._list = {};

	const root = SwitchEquip.getRoot();
	if (root) {
		root.querySelectorAll('.col1, .col3, .ammo').forEach(el => {
			el.innerHTML = '';
		});
	}
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
SwitchEquip.onShortCut = function onShurtCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			this.toggle();
			break;
	}
};

/**
 * Toggle the visibility of the UI,
 * rendering or stopping the renderer based on visibility.
 */
SwitchEquip.toggle = function toggle() {
	if (this._host.style.display === 'none') {
		this._host.style.display = '';
		Renderer.render(swaprender);
		this.focus();
	} else {
		this._host.style.display = 'none';
		Renderer.stop(swaprender);
	}
};

/**
 * Add an equipment to the window
 *
 * @param {Item} item
 * @param {item.Location} location
 * @param {boolean} inSwitchList
 */
SwitchEquip.equip = function equip(item, location, inSwitchList) {
	const it = DB.getItemInfo(item.ITID);
	item.equipped = location;
	SwitchEquip._list[item.index] = item;

	const add3Dots = (string, limit) => {
		if (string.length > limit) {
			return string.substring(0, limit) + '...';
		}
		return string;
	};

	const root = SwitchEquip.getRoot();
	const selector = getSelectorFromLocation(location);
	const el = root.querySelector(selector);
	if (el) {
		el.innerHTML =
			`<div class="item" data-index="${item.index}">` +
			'<button></button>' +
			`<span class="itemName">${add3Dots(_escapeHtml(DB.getItemName(item)), 19)}</span>` +
			'</div>';
	}

	Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', data => {
		const button = root.querySelector(`.item[data-index="${item.index}"] button`);
		if (button) {
			button.style.backgroundImage = `url(${data})`;
			if (!inSwitchList) {
				button.style.filter = 'grayscale(100%)';
			}
		}
	});
};

/**
 * Remove equipment from window
 *
 * @param {number} item index
 * @param {number} item location
 */
SwitchEquip.unEquip = function unEquip(index, location) {
	const root = SwitchEquip.getRoot();
	const selector = getSelectorFromLocation(location);
	const item = SwitchEquip._list[index];
	item.equipped = 0;

	const el = root.querySelector(selector);
	if (el) el.innerHTML = '';
	delete SwitchEquip._list[index];
};

/**
 * Rendering character
 */
const swaprender = (function swaprenderClosure() {
	const _cleanColor = new Float32Array([1.0, 1.0, 1.0, 1.0]);
	const _savedColor = new Float32Array(4);
	const _animation = {
		tick: 0,
		frame: 0,
		repeat: true,
		play: true,
		next: false,
		delay: 0,
		save: false
	};

	return function _renderFrame() {
		const swap_character = new Entity();
		swap_character.set({
			GID: Session.Entity.GID + '_SWAPEQUIP',
			objecttype: swap_character.constructor.TYPE_PC,
			job: Session.Entity.job,
			sex: Session.Entity.sex,
			name: '',
			hideShadow: true,
			head: Session.Entity.head,
			headpalette: Session.Entity.headpalette,
			bodypalette: Session.Entity.bodypalette
		});

		const currentEquipTabId = Equipment.getUI().getCurrentTabId();

		if (currentEquipTabId === 'general') {
			swap_character.accessory = SwitchEquip.checkEquipLoc(EquipLocation.HEAD_BOTTOM);
			swap_character.accessory2 = SwitchEquip.checkEquipLoc(EquipLocation.HEAD_TOP);
			swap_character.accessory3 = SwitchEquip.checkEquipLoc(EquipLocation.HEAD_MID);
			swap_character.robe = SwitchEquip.checkEquipLoc(EquipLocation.GARMENT);
		} else if (currentEquipTabId === 'costume') {
			swap_character.accessory = SwitchEquip.checkEquipLoc(EquipLocation.COSTUME_HEAD_BOTTOM);
			swap_character.accessory2 = SwitchEquip.checkEquipLoc(EquipLocation.COSTUME_HEAD_TOP);
			swap_character.accessory3 = SwitchEquip.checkEquipLoc(EquipLocation.COSTUME_HEAD_MID);
			swap_character.robe = SwitchEquip.checkEquipLoc(EquipLocation.COSTUME_ROBE);
		}

		_savedColor.set(swap_character.effectColor);
		swap_character.effectColor.set(_cleanColor);

		Camera.direction = 0;
		swap_character.direction = 0;
		swap_character.headDir = 0;
		swap_character.action = swap_character.ACTION.IDLE;
		swap_character.animation = _animation;

		for (let i = 0; i < _swapctx.length; i++) {
			const ctx = _swapctx[i];
			SpriteRenderer.bind2DContext(ctx, 30, 130);
			ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
			swap_character.renderEntity(ctx);
		}
	};
})();

/**
 * Find elements in html base on item location
 *
 * @param {number} location
 * @returns {string} selector
 */
function getSelectorFromLocation(location) {
	const selector = [];

	if (location & EquipLocation.HEAD_TOP) selector.push('.swap_head_top');
	if (location & EquipLocation.HEAD_MID) selector.push('.swap_head_mid');
	if (location & EquipLocation.HEAD_BOTTOM) selector.push('.swap_head_bottom');
	if (location & EquipLocation.ARMOR) selector.push('.swap_armor');
	if (location & EquipLocation.WEAPON) selector.push('.swap_weapon');
	if (location & EquipLocation.SHIELD) selector.push('.swap_shield');
	if (location & EquipLocation.GARMENT) selector.push('.swap_garment');
	if (location & EquipLocation.SHOES) selector.push('.swap_shoes');
	if (location & EquipLocation.ACCESSORY1) selector.push('.swap_accessory1');
	if (location & EquipLocation.ACCESSORY2) selector.push('.swap_accessory2');
	if (location & EquipLocation.AMMO) selector.push('.swap_ammo');

	// Costume Tab
	if (location & EquipLocation.COSTUME_HEAD_TOP) selector.push('.swap_costume_head_top');
	if (location & EquipLocation.COSTUME_HEAD_MID) selector.push('.swap_costume_head_mid');
	if (location & EquipLocation.COSTUME_HEAD_BOTTOM) selector.push('.swap_costume_head_bottom');
	if (location & EquipLocation.SHADOW_ARMOR) selector.push('.swap_shadow_armor');
	if (location & EquipLocation.SHADOW_WEAPON) selector.push('.swap_shadow_weapon');
	if (location & EquipLocation.SHADOW_SHIELD) selector.push('.swap_shadow_shield');
	if (location & EquipLocation.COSTUME_ROBE) selector.push('.swap_shadow_garment');
	if (location & EquipLocation.SHADOW_SHOES) selector.push('.swap_shadow_shoes');
	if (location & EquipLocation.SHADOW_R_ACCESSORY_SHADOW) selector.push('.swap_shadow_accessory1');
	if (location & EquipLocation.SHADOW_L_ACCESSORY_SHADOW) selector.push('.swap_shadow_accessory2');

	return selector.join(', ');
}

/**
 * Drag an item over the equipment, show where to place the item
 */
function onDragOver(event) {
	if (window._OBJ_DRAG_) {
		const data = window._OBJ_DRAG_;

		if (data.type === 'item') {
			const item = data.data;

			if (
				(item.type === ItemType.WEAPON || item.type === ItemType.ARMOR) &&
				item.IsIdentified &&
				!item.IsDamaged
			) {
				const selector = getSelectorFromLocation('location' in item ? item.location : item.WearLocation);
				const root = SwitchEquip.getRoot();
				const el = root.querySelector(selector);

				if (el) {
					Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/item_invert.bmp', _data => {
						el.style.backgroundImage = `url(${_data})`;
					});
				}
			}
		}
	}

	event.stopImmediatePropagation();
	event.preventDefault();
	return false;
}

/**
 * Drag out the window
 */
function onDragLeave(event) {
	const root = SwitchEquip.getRoot();
	root.querySelectorAll('td').forEach(td => {
		td.style.backgroundImage = 'none';
	});
	event.stopImmediatePropagation();
	return false;
}

/**
 * Drop an item in the equipment, equip it if possible
 */
function onDrop(event) {
	let data;

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		// Ignore parsing error
	}

	if (data && data.type === 'item') {
		const item = data.data;

		if (
			(item.type === ItemType.WEAPON || item.type === ItemType.ARMOR || item.type === ItemType.AMMO) &&
			item.IsIdentified &&
			!item.IsDamaged
		) {
			const root = SwitchEquip.getRoot();
			root.querySelectorAll('td').forEach(td => {
				td.style.backgroundImage = 'none';
			});
			SwitchEquip.onAddSwitchEquip(item.index, 'location' in item ? item.location : item.WearState);
		}
	}

	event.stopImmediatePropagation();
	return false;
}

/**
 * Right click on an item
 */
function onSwitchEquipInfo(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = SwitchEquip._list[index];

	if (item) {
		if (ItemInfo.uid === item.ITID) {
			ItemInfo.remove();
		} else {
			ItemInfo.append();
			ItemInfo.uid = item.ITID;
			ItemInfo.setItem(item);
		}
	}

	event.stopImmediatePropagation();
	event.preventDefault();
	return false;
}

/**
 * Double click on an equipment to remove it
 */
function onSwitchEquipUnEquip() {
	const index = parseInt(this.getAttribute('data-index'), 10);
	SwitchEquip.onRemoveSwitchEquip(index);

	const root = SwitchEquip.getRoot();
	const overlay = root.querySelector('.switchoverlay');
	if (overlay) overlay.style.display = 'none';
}

/**
 * When mouse is over an equipment, display the item name
 */
function onSwitchEquipOver() {
	const idx = parseInt(this.parentNode.getAttribute('data-index'), 10);
	const item = SwitchEquip._list[idx];

	if (!item) {
		return;
	}

	const root = SwitchEquip.getRoot();
	const overlay = root.querySelector('.switchoverlay');
	if (!overlay) return;

	const posTop = this.offsetTop;
	const posLeft = this.offsetLeft;

	if (!posTop && !posLeft) {
		return;
	}

	overlay.style.display = '';
	overlay.style.top = `${posTop - 22}px`;
	overlay.style.left = `${posLeft - 22}px`;
	overlay.textContent = DB.getItemName(item);
}

/**
 * Remove the item name
 */
function onSwitchEquipOut() {
	const root = SwitchEquip.getRoot();
	const overlay = root.querySelector('.switchoverlay');
	if (overlay) overlay.style.display = 'none';
}

/**
 * Update the owner name for the equipment items
 */
SwitchEquip.onUpdateOwnerName = function () {
	const root = SwitchEquip.getRoot();
	for (const index in SwitchEquip._list) {
		const item = SwitchEquip._list[index];
		if (item.slot && [0x00ff, 0x00fe, 0xff00].includes(item.slot.card1)) {
			const nameEl = root.querySelector(`.item[data-index="${index}"] .itemName`);
			if (nameEl) nameEl.textContent = _escapeHtml(DB.getItemName(item));
		}
	}
};

/**
 * Get the number of equipment items excluding AMMO location
 *
 * @returns {number} The number of equipment items
 */
SwitchEquip.getNumber = function () {
	let num = 0;
	for (const key in SwitchEquip._list) {
		if (SwitchEquip._list[key].location && SwitchEquip._list[key].location !== EquipLocation.AMMO) {
			num++;
		}
	}
	return num;
};

/**
 * Check if a specific location
 *
 * @param {number} location The location to check
 * @returns {number} The sprite number of the item in the specified location, or 0 if not equipped
 */
SwitchEquip.checkEquipLoc = function checkEquipLoc(location) {
	const switchList = Inventory.getUI().equipswitchlist;
	for (let i = 0; i < switchList.length; i++) {
		const item = switchList[i];
		if (item.location & location) {
			return item.wItemSpriteNumber;
		}
	}

	return 0;
};

/**
 * Send an equipment switch request to the server.
 */
function sendEquipSwitchRequest() {
	const pkt = new PACKET.CZ.REQ_FULLSWITCH();
	Network.sendPacket(pkt);
}

/**
 * Request an equipment switch button
 * Disabling the button temporarily and updating its background image.
 */
SwitchEquip.RequestSwitch = function () {
	sendEquipSwitchRequest();

	const root = SwitchEquip.getRoot();
	const button = root.querySelector('#swap-button');
	if (!button) return;

	button.disabled = true;
	button.classList.add('disabled');

	Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/btn_change2_disable.bmp', data => {
		button.style.backgroundImage = `url(${data})`;
	});

	setTimeout(() => {
		button.disabled = false;
		button.classList.remove('disabled');
		Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/btn_change2_normal.bmp', data => {
			button.style.backgroundImage = `url(${data})`;
		});
	}, 10000);
};

/**
 * Method to define
 */
SwitchEquip.onRemoveSwitchEquip = function onRemoveSwitchEquip(/* index */) {};
SwitchEquip.onAddSwitchEquip = function onAddSwitchEquip(/* index, location */) {};

/**
 * Create component and export it
 */
export default UIManager.addComponent(SwitchEquip);
