/**
 * UI/Components/JoystickUI/JoystickSelectionUI.js
 *
 * Handles the UI for selecting a shortcut slot when an item/skill is selected via Context Menu.
 *
 */

import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import ShortCut from 'UI/Components/ShortCut/ShortCut.js';
import htmlText from './JoystickSelectionUI.html?raw';
import cssText from './JoystickSelectionUI.css?raw';

const JoystickSelectionUI = new GUIComponent('JoystickSelectionUI', cssText);
JoystickSelectionUI.render = () => htmlText;

// State variables
let currentTab = 0;
let slotInTab = 0;
let itemData = null;
let clickLock = null;

// Internal Helper: Debounce for gamepad input
function setClickInterval() {
	if (clickLock) {
		clearTimeout(clickLock);
	}
	clickLock = setTimeout(function () {
		clickLock = null;
	}, 200);
}

function isLocked() {
	return clickLock !== null;
}

// Internal Helper: Get Combo String
function getJoystickComboForSlot(slotIndex) {
	const combos = {
		0: 'L1+Y',
		1: 'L1+X',
		2: 'L1+B',
		3: 'L1+A',
		4: 'L2+Y',
		5: 'L2+X',
		6: 'L2+B',
		7: 'L2+A',
		8: 'L1+R1+Y',

		9: 'R1+Y',
		10: 'R1+X',
		11: 'R1+B',
		12: 'R1+A',
		13: 'R2+Y',
		14: 'R2+X',
		15: 'R2+B',
		16: 'R2+A',
		17: 'L1+R1+X',

		18: 'L1+Y (Set2)',
		19: 'L1+X (Set2)',
		20: 'L1+B (Set2)',
		21: 'L1+A (Set2)',
		22: 'L2+Y (Set2)',
		23: 'L2+X (Set2)',
		24: 'L2+B (Set2)',
		25: 'L2+A (Set2)',
		26: 'L1+R1+B',

		27: 'R1+Y (Set2)',
		28: 'R1+X (Set2)',
		29: 'R1+B (Set2)',
		30: 'R1+A (Set2)',
		31: 'R2+Y (Set2)',
		32: 'R2+X (Set2)',
		33: 'R2+B (Set2)',
		34: 'R2+A (Set2)',
		35: 'L1+R1+A'
	};

	return combos[slotIndex];
}

function updateGrid() {
	const root = JoystickSelectionUI.getRoot();
	const grid = root.querySelector('.shortcut-grid');
	if (!grid) return;

	grid.innerHTML = '';

	const startIdx = currentTab * 9;

	for (let i = 0; i < 9; i++) {
		const globalIndex = startIdx + i;
		const slot = ShortCut.getList()[globalIndex];
		const isEmpty = !slot || (!slot.isSkill && !slot.ID);

		const joystickCombo = getJoystickComboForSlot(globalIndex);
		const displayText = joystickCombo || (i + 1).toString();

		const slotDiv = document.createElement('div');
		slotDiv.className = 'slot-btn';
		slotDiv.dataset.index = i;
		slotDiv.textContent = displayText;

		if (isEmpty) {
			slotDiv.classList.add('empty');
		}

		grid.appendChild(slotDiv);
	}

	updateSelection();
}

function updateSelection() {
	const root = JoystickSelectionUI.getRoot();
	const grid = root.querySelector('.shortcut-grid');
	if (!grid) return;

	grid.querySelectorAll('.slot-btn').forEach(el => el.classList.remove('selected'));
	const selected = grid.querySelector(`.slot-btn[data-index="${slotInTab}"]`);
	if (selected) {
		selected.classList.add('selected');
	}
}

function updateTabButtons() {
	const root = JoystickSelectionUI.getRoot();
	const tabButtons = root.querySelector('.tab-buttons');
	if (!tabButtons) return;

	tabButtons.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));
	const active = tabButtons.querySelector(`.tab-btn[data-tab="${currentTab}"]`);
	if (active) {
		active.classList.add('active');
	}
}

function createTabButtons() {
	const root = JoystickSelectionUI.getRoot();
	const tabButtons = root.querySelector('.tab-buttons');
	if (!tabButtons) return;

	tabButtons.innerHTML = '';

	for (let t = 0; t < 4; t++) {
		const tabBtn = document.createElement('button');
		tabBtn.className = 'tab-btn';
		tabBtn.dataset.tab = t;
		tabBtn.textContent = `Tab ${t + 1}`;
		tabButtons.appendChild(tabBtn);
	}
}

function selectSlot() {
	if (!itemData) {
		return;
	}

	const row = currentTab;
	const pos = row * 9 + slotInTab;

	ShortCut.removeElement(itemData.isSkill, itemData.ID, row, itemData.value);
	ShortCut.addElement(pos, itemData.isSkill, itemData.ID, itemData.value);
	ShortCut.onChange(pos, itemData.isSkill, itemData.ID, itemData.value);

	JoystickSelectionUI.hideSelection();
}

/**
 * Main input handler
 * as expected by JoystickButtonInput.js
 */
JoystickSelectionUI.handleGamepadInput = function handleGamepadInput(buttons) {
	if (isLocked()) {
		return true;
	}

	// A - select
	if (buttons[0] !== 'unpressed') {
		setClickInterval();
		selectSlot();
		return true;
	}

	// Select - cancel
	if (buttons[8] !== 'unpressed') {
		setClickInterval();
		JoystickSelectionUI.hideSelection();
		return true;
	}

	// L2 - Previous Tab
	if (buttons[6] !== 'unpressed') {
		setClickInterval();
		if (currentTab > 0) {
			currentTab--;
			slotInTab = 0;
			updateGrid();
			updateTabButtons();
		}
		return true;
	}

	// R2 - Next Tab
	if (buttons[7] !== 'unpressed') {
		setClickInterval();
		if (currentTab < 3) {
			currentTab++;
			slotInTab = 0;
			updateGrid();
			updateTabButtons();
		}
		return true;
	}

	// D-pad up
	if (buttons[12] !== 'unpressed') {
		setClickInterval();
		if (slotInTab >= 3) {
			slotInTab -= 3;
			updateSelection();
		}
		return true;
	}

	// D-pad down
	if (buttons[13] !== 'unpressed') {
		setClickInterval();
		if (slotInTab < 6) {
			slotInTab += 3;
			updateSelection();
		}
		return true;
	}

	// D-pad left
	if (buttons[14] !== 'unpressed') {
		setClickInterval();
		if (slotInTab > 0) {
			slotInTab--;
			updateSelection();
		}
		return true;
	}

	// D-pad right
	if (buttons[15] !== 'unpressed') {
		setClickInterval();
		if (slotInTab < 8) {
			slotInTab++;
			updateSelection();
		}
		return true;
	}

	return false;
};

JoystickSelectionUI.init = function () {
	// Initialize structure once attached
	createTabButtons();
	this._host.style.position = 'fixed';
	this._host.style.display = 'none';
};

JoystickSelectionUI.showSelection = function (data) {
	itemData = data;
	currentTab = 0;
	slotInTab = 0;

	updateGrid();
	updateTabButtons();

	this.focus();
	this._host.style.display = 'block';
	this._fixPositionOverflow();
};

JoystickSelectionUI.hideSelection = function () {
	this._host.style.display = 'none';
	itemData = null;
};

JoystickSelectionUI.active = function () {
	return this._host && this._host.style.display !== 'none';
};

export default UIManager.addComponent(JoystickSelectionUI);
