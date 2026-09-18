/**
 * UI/Components/ShortCut/ShortCut.js
 *
 * ShortCut windows component
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import SkillInfo from 'DB/Skills/SkillInfo.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import SkillListMH from 'UI/Components/SkillListMH/SkillListMH.js';
import SkillDescription from 'UI/Components/SkillDescription/SkillDescription.js';
import SkillTargetSelection from 'UI/Components/SkillTargetSelection/SkillTargetSelection.js';
import Guild from 'UI/Components/Guild/Guild.js';
import ShortCutControls from 'Preferences/ShortCutControls.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Configs from 'Core/Configs.js';
import PACKETVER from 'Network/PacketVerManager.js';
import SkillWindow from 'UI/Components/SkillList/SkillList.js';
import htmlText from './ShortCut.html?raw';
import cssText from './ShortCut.css?raw';

/**
 * Create Component
 */
const ShortCut = new GUIComponent('ShortCut', cssText);
ShortCut.render = () => htmlText;

/**
 * @var {Array} ShortCut list
 */
const _list = [];

/**
 * @var {number} max number of rows
 */
let _rowCount = 0;

/**
 * @var {object} server load hotkeys
 */
let _lastServerHotkeys = null;

/**
 * Cache for active animation frames
 */
const _activeAnimations = new Map();

/**
 * @var {Preference} structure to save informations about shortcut
 */
const _preferences = Preferences.get(
	'ShortCut',
	{
		x: 480,
		y: 0,
		size: 1,
		magnet_top: true,
		magnet_bottom: false,
		magnet_left: false,
		magnet_right: false
	},
	1.0
);

/**
 * Initialize UI
 */
ShortCut.init = function init() {
	const root = ShortCut.getRoot();

	const resizeBtn = root.querySelector('.resize');
	if (resizeBtn) {
		resizeBtn.addEventListener('mousedown', onResize);
	}

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
			e.preventDefault();
		});
		closeBtn.addEventListener('click', onClose);
	}

	const container = root.querySelector('#ShortCut');

	// Dropping to the shortcut
	container.addEventListener('drop', e => {
		const target = e.target.closest('.container');
		if (target) {
			onDrop(e, target);
		}
	});
	container.addEventListener('dragover', e => {
		if (e.target.closest('.container')) {
			e.stopImmediatePropagation();
			e.preventDefault();
		}
	});

	// Icons — delegated events
	container.addEventListener('dragstart', e => {
		const icon = e.target.closest('.icon');
		if (icon) {
			onDragStart(e, icon);
		}
	});
	container.addEventListener('dragend', e => {
		const icon = e.target.closest('.icon');
		if (icon) {
			onDragEnd(icon);
		}
	});
	container.addEventListener('dblclick', e => {
		const icon = e.target.closest('.icon');
		if (icon) {
			onUseShortCut(icon);
		}
	});
	container.addEventListener('contextmenu', e => {
		const icon = e.target.closest('.icon');
		if (icon) {
			onElementInfo(e, icon);
		}
	});
	container.addEventListener('mousedown', e => {
		if (e.target.closest('.icon')) {
			e.stopImmediatePropagation();
		}
	});

	this.draggable();

	// Tooltip events
	root.querySelectorAll('.container').forEach(el => {
		el.addEventListener('mouseenter', onContainerMouseEnter);
		el.addEventListener('mouseleave', onContainerMouseLeave);
	});

	// Add to item owner name update queue
	DB.UpdateOwnerName.ShortCut = onUpdateOwnerName;

	Inventory.getUI().onUpdateItem = onUpdateItem;
};

/**
 * Append to body
 */
ShortCut.onAppend = function onAppend() {
	// Set height first so position clamping uses correct dimensions
	this._host.style.height = `${34 * _preferences.size}px`;
	const rect = this._host.getBoundingClientRect();
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - rect.height)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - rect.width)}px`;
	this.magnet.TOP = _preferences.magnet_top;
	this.magnet.BOTTOM = _preferences.magnet_bottom;
	this.magnet.LEFT = _preferences.magnet_left;
	this.magnet.RIGHT = _preferences.magnet_right;

	SkillWindow.getUI().onUpdateSkill = onUpdateSkill;

	// Initialize tooltips for empty slots
	updateEmptySlotTooltips();
};

/**
 * When removed, clean up
 */
ShortCut.onRemove = function onRemove() {
	// Hide tooltip
	const root = ShortCut.getRoot();
	const tooltip = root.querySelector('.shortcut-tooltip');
	if (tooltip) {
		tooltip.classList.remove('show');
	}

	// Cancels all active animation loops defensively to prevent leaks in unattached elements
	for (const [index, animationId] of _activeAnimations.entries()) {
		cancelAnimationFrame(animationId);
	}
	_activeAnimations.clear();

	// Save preferences
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.size = Math.floor(parseInt(this._host.style.height, 10) / 34);
	_preferences.magnet_top = this.magnet.TOP;
	_preferences.magnet_bottom = this.magnet.BOTTOM;
	_preferences.magnet_left = this.magnet.LEFT;
	_preferences.magnet_right = this.magnet.RIGHT;
	_preferences.save();
};

/**
 * Request to clean the list
 * Used only from MapEngine when exiting the game
 */
ShortCut.clean = function clean() {
	// Cancels all active animation loops immediately to prevent post-logout TypeError
	for (const [index, animationId] of _activeAnimations.entries()) {
		cancelAnimationFrame(animationId);
	}
	_activeAnimations.clear();

	_list.length = 0;
	const root = ShortCut.getRoot();
	root.querySelectorAll('.container').forEach(el => {
		el.innerHTML = '';
	});
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
ShortCut.onShortCut = function onShortCut(key) {
	switch (key.cmd.replace(/\d+$/, '')) {
		case 'EXECUTE':
			clickElement(parseInt(key.cmd.match(/\d+$/).toString(), 10));
			break;

		case 'EXTEND':
			_preferences.size = (_preferences.size + 1) % (_rowCount + 1);
			_preferences.save();
			this._host.style.height = `${_preferences.size * 34}px`;
			break;
	}
};

ShortCut.useSkill = function useSkill(id, level) {
	if (id > 10000 && id < 10100) {
		Guild.useSkillID(id, level);
	} else if (id > 8000 && id < 8044) {
		// if one of them don't have the skill, it returns early
		SkillListMH.mercenary.useSkillID(id, level);
		SkillListMH.homunculus.useSkillID(id, level);
	} else {
		SkillWindow.getUI().useSkillID(id, level);
	}
};

ShortCut.getSkillById = function getSkillById(id) {
	let skill;

	if (id > 10000 && id < 10100) {
		skill = Guild.getSkillById(id);
	} else if (id > 8000 && id < 8044) {
		skill = SkillListMH.mercenary.getSkillById(id);
		if (!skill) {
			skill = SkillListMH.homunculus.getSkillById(id);
		}
	} else {
		skill = SkillWindow.getUI().getSkillById(id);
	}

	return skill;
};

/**
 * Bind UI with list of shortcut
 *
 * @param {Array} shortcut list
 */
ShortCut.setList = function setList(list) {
	let skill;
	const root = ShortCut.getRoot();

	root.querySelectorAll('.container').forEach(el => {
		el.innerHTML = '';
	});
	_list.length = list.length;
	_rowCount = Math.min(4, Math.floor(list.length / 9));

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].isSkill) {
			skill = ShortCut.getSkillById(list[i].ID);

			if (skill && skill.level) {
				ShortCut.addElement(i, true, list[i].ID, list[i].count || skill.level);
			} else {
				if (!_list[i]) {
					_list[i] = {};
				}

				_list[i].isSkill = true;
				_list[i].ID = list[i].ID;
				_list[i].count = list[i].count;
			}
		} else {
			ShortCut.addElement(i, list[i].isSkill, list[i].ID, list[i].count);
		}
	}
};

/**
 * Update tooltip for empty slots with hotkey only
 */
function updateEmptySlotTooltips() {
	const root = ShortCut.getRoot();
	const containers = root.querySelectorAll('.container');

	for (let i = 0; i < containers.length; ++i) {
		// Only update empty slots
		if (!_list[i] || (!_list[i].isSkill && !_list[i].ID)) {
			const hotkey = getHotKeyString(i);
			if (hotkey) {
				containers[i].setAttribute('data-tooltip', hotkey);
			}
		}
	}
}

/**
 * Update all tooltips (for both empty and filled slots)
 * Called when hotkey settings change
 */
ShortCut.updateAllTooltips = function updateAllTooltips() {
	const root = ShortCut.getRoot();

	for (let i = 0, size = _list.length; i < size; ++i) {
		const container = root.querySelector(`.container[data-index="${i}"]`);
		if (!container) {
			continue;
		}

		const hotkey = getHotKeyString(i);

		// Update empty slots
		if (!_list[i] || (!_list[i].isSkill && !_list[i].ID)) {
			if (hotkey) {
				container.setAttribute('data-tooltip', hotkey);
			}
		}
		// Update filled slots
		else if (_list[i] && (_list[i].isSkill || _list[i].ID)) {
			let name = '';
			if (_list[i].isSkill && SkillInfo[_list[i].ID]) {
				name = SkillInfo[_list[i].ID].SkillName;
			} else if (_list[i].ID) {
				const item = Inventory.getUI().getItemById(_list[i].ID);
				if (item) {
					name = DB.getItemName(item);
				}
			}

			if (name) {
				const tooltipText = hotkey ? `[ ${hotkey} ] ${name}` : name;
				container.setAttribute('data-tooltip', tooltipText);
			}
		}
	}
};

/**
 * Get hotkey string for shortcut index
 *
 * @param {number} index of the shortcut slot
 * @return {string} hotkey string or empty string
 */
function getHotKeyString(index) {
	const shortcutKeys = [
		'F1_1',
		'F1_2',
		'F1_3',
		'F1_4',
		'F1_5',
		'F1_6',
		'F1_7',
		'F1_8',
		'F1_9',
		'F2_1',
		'F2_2',
		'F2_3',
		'F2_4',
		'F2_5',
		'F2_6',
		'F2_7',
		'F2_8',
		'F2_9',
		'F3_1',
		'F3_2',
		'F3_3',
		'F3_4',
		'F3_5',
		'F3_6',
		'F3_7',
		'F3_8',
		'F3_9',
		'F4_1',
		'F4_2',
		'F4_3',
		'F4_4',
		'F4_5',
		'F4_6',
		'F4_7',
		'F4_8',
		'F4_9'
	];

	if (index < 0 || index >= shortcutKeys.length) {
		return '';
	}

	const scKey = shortcutKeys[index];
	const shortcut = ShortCutControls.ShortCuts[scKey];

	if (!shortcut) {
		return '';
	}

	const key = shortcut.cust ? shortcut.cust.key : shortcut.init.key;
	const alt = shortcut.cust ? shortcut.cust.alt : shortcut.init.alt;
	const ctrl = shortcut.cust ? shortcut.cust.ctrl : shortcut.init.ctrl;
	const shift = shortcut.cust ? shortcut.cust.shift : shortcut.init.shift;

	if (!key) {
		return '';
	}

	let hotkeyStr = '';
	if (alt) {
		hotkeyStr += 'ALT + ';
	}
	if (ctrl) {
		hotkeyStr += 'CTRL + ';
	}
	if (shift) {
		hotkeyStr += 'SHIFT + ';
	}
	hotkeyStr += KEYS.toReadableKey(key);

	return hotkeyStr;
}

/**
 * Show fixed tooltip on container hover
 */
function onContainerMouseEnter(event) {
	const container = event.currentTarget;
	const tooltipText = container.getAttribute('data-tooltip');

	if (tooltipText) {
		const root = ShortCut.getRoot();
		const tooltip = root.querySelector('.shortcut-tooltip');
		const host = ShortCut._host;
		const hostRect = host.getBoundingClientRect();

		tooltip.textContent = tooltipText;
		tooltip.classList.add('show');

		// Calculate tooltip dimensions
		const tooltipRect = tooltip.getBoundingClientRect();

		// Check if there's enough space below
		const windowHeight = window.innerHeight;
		const spaceBelow = windowHeight - (hostRect.top + hostRect.height);
		const showAbove = spaceBelow < tooltipRect.height + 10;

		// Position tooltip centered horizontally
		const left = hostRect.left + hostRect.width / 2 - tooltipRect.width / 2;
		let top;

		if (showAbove) {
			top = hostRect.top - tooltipRect.height - 2;
		} else {
			top = hostRect.top + hostRect.height + 2;
		}

		tooltip.style.left = `${left}px`;
		tooltip.style.top = `${top}px`;
	}
}

/**
 * Hide fixed tooltip on container leave
 */
function onContainerMouseLeave() {
	const root = ShortCut.getRoot();
	const tooltip = root.querySelector('.shortcut-tooltip');
	if (tooltip) {
		tooltip.classList.remove('show');
	}
}

ShortCut.setElement = function setElement(isSkill, ID, count) {
	for (let i = 0, size = _list.length; i < size; ++i) {
		if (_list[i] && _list[i].isSkill == isSkill && _list[i].ID === ID) {
			if (isSkill && _list[i].count && _list[i].count <= count) {
				ShortCut.addElement(i, isSkill, ID, _list[i].count);
			} else {
				ShortCut.addElement(i, isSkill, ID, count);
			}
		}
	}
};

/**
 * Resizing hotkey window
 */
function onResize(event) {
	const host = ShortCut._host;
	const top = host.offsetTop;
	let lastHeight = 0;

	function resizing() {
		let h = Math.floor((Mouse.screen.y - top) / 34 + 1);

		// Maximum and minimum window size
		h = Math.min(Math.max(h, 1), _rowCount);

		if (h === lastHeight) {
			return;
		}

		host.style.height = `${h * 34}px`;
		_preferences.size = h;
		_preferences.save();
		lastHeight = h;
	}

	// Start resizing
	const _Interval = setInterval(resizing, 30);

	// Stop resizing on left click
	const mouseUpHandler = _event => {
		if (_event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', mouseUpHandler);
		}
	};
	window.addEventListener('mouseup', mouseUpHandler);

	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Add an element to shortcut
 *
 * @param {number} index of the element
 * @param {boolean} is a skill ?
 * @param {number} ID
 * @param {number} count or level
 */
ShortCut.addElement = function addElement(index, isSkill, ID, count) {
	let file, name;
	const root = ShortCut.getRoot();
	const ui = root.querySelector(`.container[data-index="${index}"]`);
	if (!ui) return;
	ui.innerHTML = '';

	if (!_list[index]) {
		_list[index] = {};
	}

	_list[index].isSkill = isSkill;
	_list[index].ID = ID;

	if (isSkill) {
		// Do not display if no level.
		if (!count) {
			return;
		} else {
			_list[index].count = count;
			file = SkillInfo[ID].Name;
			name = SkillInfo[ID].SkillName;
		}
	} else {
		_list[index].count = count;
		const item = Inventory.getUI().getItemById(ID);

		// Do not display items not in inventory
		if (!item) {
			return;
		}

		const it = DB.getItemInfo(ID);
		file = item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName;
		name = DB.getItemName(item);

		// If equipment, do not display count
		if (item.type === ItemType.WEAPON || item.type === ItemType.ARMOR || item.type === ItemType.SHADOWGEAR) {
			count = 1;
		}
		// Get item count
		else {
			count = item.count;
		}

		// Do not display item if there is none in the inventory
		if (!count) {
			return;
		}
	}

	// Get hotkey for this slot
	const hotkey = getHotKeyString(index);
	const tooltipText = hotkey ? `[ ${hotkey} ] ${name}` : name;

	Client.loadFile(`${DB.INTERFACE_PATH}item/${file}.bmp`, url => {
		ui.innerHTML = '<div draggable="true" class="icon"><div class="img"></div><div class="amount"></div></div>';

		ui.querySelector('.img').style.backgroundImage = `url(${url})`;
		ui.querySelector('.amount').textContent = count;
		ui.setAttribute('data-tooltip', tooltipText);
	});
};

/**
 * Displays the cooldown overlay on an icon
 *
 * @param {number} index of the icon
 * @param {number} delay in ms
 */
function setDelayOnIndex(index, delay) {
	// Safety validation to ensure the index exists in the list
	if (!_list[index]) {
		return;
	}

	// do nothing, the new delay would end sooner.
	if (_list[index].Delay && _list[index].Delay >= Renderer.tick + delay) {
		return;
	}

	_list[index].Delay = Renderer.tick + delay;
	const root = ShortCut.getRoot();
	const ui = root.querySelector(`.container[data-index="${index}"]`);
	if (!ui) return;
	const existing = ui.querySelector('.cooldown-overlay');
	if (existing) {
		existing.remove();
	}

	const overlay = document.createElement('div');
	overlay.className = 'cooldown-overlay';
	const icon = ui.querySelector('.icon');
	if (icon) {
		icon.appendChild(overlay);
		const img = icon.querySelector('.img');
		if (img) {
			img.style.filter = 'none';
		}
	}

	// Cancel any previous animation frame scheduled for the same index
	if (_activeAnimations.has(index)) {
		cancelAnimationFrame(_activeAnimations.get(index));
		_activeAnimations.delete(index);
	}

	function updateCooldown() {
		// Safety check against post-destruction or logout leaks
		if (!_list || !_list[index]) {
			overlay.remove();
			if (_activeAnimations.has(index)) {
				cancelAnimationFrame(_activeAnimations.get(index));
				_activeAnimations.delete(index);
			}
			return;
		}

		const now = Renderer.tick;
		const remaining = _list[index].Delay - now;

		if (remaining <= 0 || !_list[index].Delay) {
			overlay.remove();
			_list[index].Delay = 0;
			if (_activeAnimations.has(index)) {
				cancelAnimationFrame(_activeAnimations.get(index));
				_activeAnimations.delete(index);
			}
			return;
		}

		const percentage = remaining / delay;
		const degrees = (1 - percentage) * 360;
		overlay.style.background = `conic-gradient(transparent 0deg, transparent ${degrees}deg, rgba(0,0,0,0.75) ${degrees}deg)`;

		const animationId = requestAnimationFrame(updateCooldown);
		_activeAnimations.set(index, animationId);
	}

	const animationId = requestAnimationFrame(updateCooldown);
	_activeAnimations.set(index, animationId);
}

/**
 * Displays the cooldown over every skill
 *
 * @param {number} delay in ms
 */
ShortCut.setGlobalSkillDelay = function setGlobalSkillDelay(delay) {
	_list.forEach((element, index) => {
		if (element.isSkill) {
			setDelayOnIndex(index, delay);
		}
	});
};

/**
 * Displays the cooldown over a single skill
 *
 * @param {number} ID of the skill
 * @param {number} delay in ms
 */
ShortCut.setSkillDelay = function setSkillDelay(ID, delay) {
	_list.forEach((element, index) => {
		if (element.isSkill && element.ID == ID) {
			setDelayOnIndex(index, delay);
		}
	});
};

/**
 * Remove an element from shortcut
 *
 * @param {boolean} is a skill ?
 * @param {number} ID of the element to remove
 * @param {number} row id
 * @param {number} amount (optional)
 */
ShortCut.removeElement = function removeElement(isSkill, ID, row, amount) {
	// Do not need to modify empty slot
	if (!ID) {
		return;
	}

	const root = ShortCut.getRoot();

	for (let i = row * 9, count = Math.min(_list.length, row * 9 + 9); i < count; ++i) {
		if (_list[i] && _list[i].isSkill == isSkill && _list[i].ID === ID && (!isSkill || _list[i].count == amount)) {
			const container = root.querySelector(`.container[data-index="${i}"]`);
			if (container) {
				container.innerHTML = '';
			}
			_list[i].isSkill = 0;
			_list[i].ID = 0;
			_list[i].count = 0;

			ShortCut.onChange(i, 0, 0, 0);
		}
	}
};

/**
 * Drop something in the shortcut
 * Does the client allow other source than shortcut, inventory
 * and skill window to save to shortcut ?
 */
function onDrop(event, target) {
	let data, element;
	const index = parseInt(target.getAttribute('data-index'), 10);
	const row = Math.floor(index / 9);

	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		element = data.data;
	} catch (_e) {
		return;
	}

	// Do not process others things than item and skill
	if (data.type !== 'item' && data.type !== 'skill') {
		return;
	}

	switch (data.from) {
		case 'SkillList':
		case 'Guild':
		case 'SkillListMH':
			ShortCut.removeElement(
				true,
				element.SKID,
				row,
				element.selectedLevel ? element.selectedLevel : element.level
			);
			ShortCut.addElement(
				index,
				true,
				element.SKID,
				element.selectedLevel ? element.selectedLevel : element.level
			);
			ShortCut.onChange(index, true, element.SKID, element.selectedLevel ? element.selectedLevel : element.level);
			break;

		case 'Inventory':
			ShortCut.removeElement(false, element.ITID, row);
			ShortCut.addElement(index, false, element.ITID, 0);
			ShortCut.onChange(index, false, element.ITID, 0);
			break;

		case 'ShortCut':
			ShortCut.removeElement(element.isSkill, element.ID, row, element.isSkill ? element.count : null);
			ShortCut.addElement(index, element.isSkill, element.ID, element.count);
			ShortCut.onChange(index, element.isSkill, element.ID, element.count);
			break;
	}
}

/**
 * Stop the drag and drop
 */
function onDragEnd(icon) {
	delete window._OBJ_DRAG_;
	icon.classList.remove('hide');
}

/**
 * Prepare data to be stored in the dragged element
 * to change position in the shortcut.
 */
function onDragStart(event, icon) {
	const index = parseInt(icon.parentNode.getAttribute('data-index'), 10);
	icon.classList.add('hide');

	// Extract image from css to get it when dragging the element
	const img = new Image();
	img.decoding = 'async';
	img.src = icon
		.querySelector('.img')
		.style.backgroundImage.match(/\(([^)]+)/)[1]
		.replace(/"/g, '');

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: _list[index].isSkill ? 'skill' : 'item',
				from: 'ShortCut',
				data: _list[index]
			})
		)
	);
}

/**
 * Get informations from a skill/item when
 * using right click on it.
 */
function onElementInfo(event, icon) {
	const index = parseInt(icon.parentNode.getAttribute('data-index'), 10);
	const element = _list[index];

	event.stopImmediatePropagation();
	event.preventDefault();

	// Display skill informations
	if (element.isSkill) {
		if (SkillDescription.uid === _list[index].ID) {
			SkillDescription.remove();
		} else {
			SkillDescription.append();
			SkillDescription.setSkill(_list[index].ID);
		}
	}
	// Display item informations
	else {
		if (ItemInfo.uid === _list[index].ID) {
			ItemInfo.remove();
			return;
		}

		ItemInfo.append();
		ItemInfo.uid = _list[index].ID;
		ItemInfo.setItem(Inventory.getUI().getItemById(_list[index].ID));
	}
}

/**
 * Double-click on a shortcut
 */
function onUseShortCut(icon) {
	const index = parseInt(icon.parentNode.getAttribute('data-index'), 10);
	clickElement(index);
}

/**
 * Clicking on a shortcut
 *
 * @param {number} shortcut index
 */
function clickElement(index) {
	const shortcut = _list[index];

	SkillTargetSelection.remove();

	// Nothing here ?
	if (!shortcut) {
		return;
	}

	// Execute skill
	if (shortcut.isSkill) {
		ShortCut.useSkill(shortcut.ID, shortcut.count);
	}
	// Use the item
	else {
		const item = Inventory.getUI().getItemById(_list[index].ID);
		if (item) {
			Inventory.getUI().useItem(item);
		}
	}
}

/**
 * Closing the window
 */
function onClose() {
	ShortCut._host.style.height = '0px';
	_preferences.size = 0;
	_preferences.save();
}

/**
 * Hook Inventory, get informations when there is a change
 * to update the shortcut
 *
 * @param {number} index
 * @param {number} count
 */
function onUpdateItem(index, count) {
	ShortCut.setElement(false, index, count);
}

/**
 * Hook Skill List, get informations when there is a change
 * to update the shortcut
 *
 * @param {number} skill id
 * @param {number} level
 */
function onUpdateSkill(id, level) {
	ShortCut.setElement(true, id, level);
}

Guild.onUpdateSkill = (id, level) => {
	ShortCut.setElement(true, id, level);
};

SkillListMH.mercenary.onUpdateSkill = (id, level) => {
	ShortCut.setElement(true, id, level);
};

SkillListMH.homunculus.onUpdateSkill = (id, level) => {
	ShortCut.setElement(true, id, level);
};

function onUpdateOwnerName() {
	for (const index in _list) {
		if (!_list[index].isSkill) {
			ShortCut.setElement(false, _list[index].ID, _list[index].count);
		}
	}
}

/**
 * Method to define to notify a change.
 *
 * @param {number} index
 * @param {boolean} isSkill
 * @param {number} id
 * @param {number} count
 */
ShortCut.onChange = function onChange(/*index, isSkill, ID, count*/) {};

function convertHotkeysToServerFormat() {
	const serverData = {
		Type: 1,
		data: {
			EmotionHotkey: [],
			UserHotkey_V2: {
				SkillBar_1Tab: []
			}
		}
	};

	const emotionKeys = [
		'Macro1',
		'Macro2',
		'Macro3',
		'Macro4',
		'Macro5',
		'Macro6',
		'Macro7',
		'Macro8',
		'Macro9',
		'Macro10'
	];
	emotionKeys.forEach((key, index) => {
		const shortcut = ShortCutControls.ShortCuts[key];
		if (shortcut && shortcut.cust && shortcut.cust.emotion) {
			serverData.data.EmotionHotkey[index] = shortcut.cust.emotion;
		}
	});

	const shortcutKeys = [
		'F1_1',
		'F1_2',
		'F1_3',
		'F1_4',
		'F1_5',
		'F1_6',
		'F1_7',
		'F1_8',
		'F1_9',
		'F2_1',
		'F2_2',
		'F2_3',
		'F2_4',
		'F2_5',
		'F2_6',
		'F2_7',
		'F2_8',
		'F2_9',
		'F3_1',
		'F3_2',
		'F3_3',
		'F3_4',
		'F3_5',
		'F3_6',
		'F3_7',
		'F3_8',
		'F3_9',
		'F4_1',
		'F4_2',
		'F4_3',
		'F4_4',
		'F4_5',
		'F4_6',
		'F4_7',
		'F4_8',
		'F4_9'
	];

	shortcutKeys.forEach((key, index) => {
		const shortcut = ShortCutControls.ShortCuts[key];
		if (shortcut) {
			const keyData = shortcut.cust || shortcut.init;
			serverData.data.UserHotkey_V2.SkillBar_1Tab.push({
				desc: `Skill ${index + 1}`,
				index: index,
				key1: keyData.key || 0,
				key2: 0
			});
		}
	});

	return serverData;
}

function convertHotkeysFromServerFormat(serverData) {
	if (!serverData || !serverData.data) {
		return;
	}

	if (serverData.data.EmotionHotkey) {
		const emotionKeys = [
			'Macro1',
			'Macro2',
			'Macro3',
			'Macro4',
			'Macro5',
			'Macro6',
			'Macro7',
			'Macro8',
			'Macro9',
			'Macro10'
		];
		serverData.data.EmotionHotkey.forEach((emotion, index) => {
			if (emotion && emotionKeys[index]) {
				if (!ShortCutControls.ShortCuts[emotionKeys[index]].cust) {
					ShortCutControls.ShortCuts[emotionKeys[index]].cust = {};
				}
				ShortCutControls.ShortCuts[emotionKeys[index]].cust.emotion = emotion;
			}
		});
	}

	if (serverData.data.UserHotkey_V2 && serverData.data.UserHotkey_V2.SkillBar_1Tab) {
		const shortcutKeys = [
			'F1_1',
			'F1_2',
			'F1_3',
			'F1_4',
			'F1_5',
			'F1_6',
			'F1_7',
			'F1_8',
			'F1_9',
			'F2_1',
			'F2_2',
			'F2_3',
			'F2_4',
			'F2_5',
			'F2_6',
			'F2_7',
			'F2_8',
			'F2_9',
			'F3_1',
			'F3_2',
			'F3_3',
			'F3_4',
			'F3_5',
			'F3_6',
			'F3_7',
			'F3_8',
			'F3_9',
			'F4_1',
			'F4_2',
			'F4_3',
			'F4_4',
			'F4_5',
			'F4_6',
			'F4_7',
			'F4_8',
			'F4_9'
		];

		serverData.data.UserHotkey_V2.SkillBar_1Tab.forEach(skillData => {
			if (skillData && skillData.index < shortcutKeys.length) {
				const key = shortcutKeys[skillData.index];
				if (key && skillData.key1) {
					if (!ShortCutControls.ShortCuts[key].cust) {
						ShortCutControls.ShortCuts[key].cust = {};
					}
					ShortCutControls.ShortCuts[key].cust.key = skillData.key1;
				}
			}
		});
	}
}

function haveHotkeysChanged(currentData) {
	if (!_lastServerHotkeys) {
		return true;
	}
	return JSON.stringify(currentData) !== JSON.stringify(_lastServerHotkeys);
}

ShortCut.saveToServer = function saveToServer() {
	if (PACKETVER.value >= 20170315 && Session.WebToken) {
		const hotkeys = JSON.stringify(convertHotkeysToServerFormat());
		if (!haveHotkeysChanged(hotkeys)) {
			return;
		}
		const formData = new FormData();
		formData.append('AID', Session.AID);
		formData.append('WorldName', Session.ServerName);
		formData.append('AuthToken', Session.WebToken);
		formData.append('data', hotkeys);

		const xhr = new XMLHttpRequest();
		let webserverAddress = '';
		if (window.location.protocol !== 'http:' && window.location.protocol !== 'https:') {
			webserverAddress = Configs.get('webserverAddress', 'http://127.0.0.1:8888');
		}
		xhr.open('POST', `${webserverAddress}/userconfig/save`, true);
		xhr.timeout = 5000;
		xhr.onload = () => {
			if (xhr.status === 200) {
				console.log('Hotkeys saved to server successfully');
			} else {
				console.warn('Hotkey save returned non-200 status:', xhr.status);
			}
		};
		xhr.onerror = () => {
			console.warn('Hotkey save failed: web-server unreachable');
		};
		xhr.ontimeout = () => {
			console.warn('Hotkey save timed out');
		};
		xhr.send(formData);
	}
};

ShortCut.loadFromServer = function loadFromServer(callback) {
	if (PACKETVER.value >= 20170315 && Session.WebToken) {
		const formData = new FormData();
		formData.append('AID', Session.AID);
		formData.append('WorldName', Session.ServerName);
		formData.append('AuthToken', Session.WebToken);

		const xhr = new XMLHttpRequest();
		let webserverAddress = '';
		if (window.location.protocol !== 'http:' && window.location.protocol !== 'https:') {
			webserverAddress = Configs.get('webserverAddress', 'http://127.0.0.1:8888');
		}
		xhr.open('POST', `${webserverAddress}/userconfig/load`, true);
		xhr.timeout = 5000;
		xhr.onload = () => {
			if (xhr.status === 200) {
				try {
					const serverData = JSON.parse(xhr.responseText);
					_lastServerHotkeys = JSON.parse(JSON.stringify(serverData));
					convertHotkeysFromServerFormat(serverData);
				} catch (e) {
					console.error('Error parsing server hotkeys:', e);
				}
			} else {
				console.warn('Hotkey load returned non-200 status:', xhr.status);
			}
			if (callback) {
				callback();
			}
		};
		xhr.onerror = () => {
			console.warn('Hotkey load failed: web-server unreachable');
			if (callback) {
				callback();
			}
		};
		xhr.ontimeout = () => {
			console.warn('Hotkey load timed out');
			if (callback) {
				callback();
			}
		};
		xhr.send(formData);
	} else if (callback) {
		callback();
	}
};

ShortCut.getList = function getList() {
	return _list;
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(ShortCut);
