/**
 * UI/Components/ShortCutOption/ShortCutOption.js
 *
 * Short Cut Settings
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import KEYS from 'Controls/KeyEventHandler.js';
import Preferences from 'Core/Preferences.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ShortCutControls from 'Preferences/ShortCutControls.js';
import BattleMode from 'Controls/BattleMode.js';
import htmlText from './ShortCutOption.html?raw';
import cssText from './ShortCutOption.css?raw';
import Controls from 'Preferences/Controls.js';

const ShortCutOption = new GUIComponent('ShortCutOption', cssText);

const ShortCuts = ShortCutControls.ShortCuts;
let ShortCutsTemp = {};

ShortCutOption.isCapturing = false;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'ShortCutOption',
	{
		x: 300,
		y: 300
	},
	1.0
);

/**
 * Render HTML
 */
ShortCutOption.render = () => htmlText;

/**
 * Initialize UI
 */
ShortCutOption.init = function () {
	const root = this.getRoot();

	let close = root.querySelector('.close');
	function closebtn(btn) {
		if (btn) {
			btn.addEventListener('mousedown', e => {
				e.stopImmediatePropagation();
				ShortCutOption.remove();
			});
			btn.addEventListener('click', e => {
				e.stopImmediatePropagation();
				ShortCutOption.remove();
			});
		}
	}
	closebtn(close);
	close = root.querySelector('.button.close');
	closebtn(close);
	root.querySelectorAll('.tabs button').forEach(function (btn) {
		btn.addEventListener('click', function () {
			root.querySelectorAll('.selectedtab').forEach(function (el) {
				el.classList.remove('selectedtab');
			});
			const tab = this.dataset.index;
			root.querySelectorAll('.' + tab).forEach(function (el) {
				el.classList.add('selectedtab');
			});
		});
	});

	root.querySelectorAll('td').forEach(function (td) {
		td.addEventListener('click', function () {
			if (this.classList.contains('customize')) {
				ShortCutOption.isCapturing = true;
				root.querySelectorAll('td.selected').forEach(function (el) {
					el.classList.remove('selected');
				});
				this.classList.add('selected');
			} else {
				ShortCutOption.isCapturing = false;
				root.querySelectorAll('td.selected').forEach(function (el) {
					el.classList.remove('selected');
				});
			}
		});
	});

	// Joystick
	const bindChange = function (selector, handler) {
		const el = root.querySelector(selector);
		if (el) el.addEventListener('change', handler);
	};

	bindChange('.attackTargetMode', onUpdateTargetOption);
	bindChange('.joySense', onUpdateSense);
	bindChange('.joyQuick', onUpdateJoyQuick);
	bindChange('.joyDeadline', onUpdateJoyDeadline);
	bindChange('.joyReverseStick', onUpdateReverseStick);
	bindChange('.joyAutoHide', onUpdateAutoHide);
	bindChange('.joyDisableVirtualMouse', onUpdateDisableVirtualMouse);

	const resetBtn = root.querySelector('.button.reset');
	if (resetBtn) {
		resetBtn.addEventListener('click', function () {
			resetKeysToDefault();
		});
	}

	const okBtn = root.querySelector('.button.ok');
	if (okBtn) {
		okBtn.addEventListener('click', function () {
			applySettings();
		});
	}

	const cancelBtn = root.querySelector('.button.cancel');
	if (cancelBtn) {
		cancelBtn.addEventListener('click', function () {
			cancelSettings();
		});
	}

	updateKeyList();
	this.draggable('.titlebar');
};

/**
 * Apply preferences once append to body
 */
ShortCutOption.onAppend = function () {
	this._host.style.left = _preferences.x + 'px';
	this._host.style.top = _preferences.y + 'px';
	this._host.style.zIndex = 100;
};

/**
 * Remove from window (and so clean up)
 */
ShortCutOption.onRemove = function () {
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.save();
};

/**
 * Checks if there is a match in the temporary settings
 * Returns the name of the conflicting shortcut, or false if no conflict
 */
function tempMatch(key) {
	const TempState = {};
	let matchSC = false;

	Object.keys(ShortCuts).forEach(function (SC) {
		if (ShortCuts[SC].cust) {
			TempState[SC] = {};
			TempState[SC].key = ShortCuts[SC].cust.key;
			TempState[SC].alt = ShortCuts[SC].cust.alt;
			TempState[SC].ctrl = ShortCuts[SC].cust.ctrl;
			TempState[SC].shift = ShortCuts[SC].cust.shift;
		} else {
			TempState[SC] = {};
			TempState[SC].key = ShortCuts[SC].init.key;
			TempState[SC].alt = ShortCuts[SC].init.alt;
			TempState[SC].ctrl = ShortCuts[SC].init.ctrl;
			TempState[SC].shift = ShortCuts[SC].init.shift;
		}
	});

	Object.keys(ShortCutsTemp).forEach(function (SC) {
		if (ShortCutsTemp[SC].cust) {
			TempState[SC] = {};
			TempState[SC].key = ShortCutsTemp[SC].cust.key;
			TempState[SC].alt = ShortCutsTemp[SC].cust.alt;
			TempState[SC].ctrl = ShortCutsTemp[SC].cust.ctrl;
			TempState[SC].shift = ShortCutsTemp[SC].cust.shift;
		} else {
			TempState[SC] = {};
			TempState[SC].key = ShortCuts[SC].init.key;
			TempState[SC].alt = ShortCuts[SC].init.alt;
			TempState[SC].ctrl = ShortCuts[SC].init.ctrl;
			TempState[SC].shift = ShortCuts[SC].init.shift;
		}
	});

	Object.keys(TempState).every(function (SC) {
		if (TempState[SC]) {
			if (
				TempState[SC].key == key &&
				TempState[SC].alt == KEYS.ALT &&
				TempState[SC].ctrl == KEYS.CTRL &&
				TempState[SC].shift == KEYS.SHIFT
			) {
				matchSC = SC;
				return false;
			} else {
				return true;
			}
		}
	});

	return matchSC;
}

/**
 * Process key
 *
 * @param {object} key
 */
ShortCutOption.onKeyDown = function (event) {
	if (ShortCutOption.isCapturing) {
		if (16 != event.which && 17 != event.which && 18 != event.which) {
			const root = ShortCutOption.getRoot();
			const box = root.querySelector('td.selected');
			const currentSC = box ? box.dataset.button : null;

			if (!box || !currentSC || !ShortCuts[currentSC]) {
				if (box) {
					console.warn('Shortcut "' + currentSC + '" is not defined in ShortCutControls');
				}
				root.querySelectorAll('td.selected').forEach(function (el) {
					el.classList.remove('selected');
				});
				ShortCutOption.isCapturing = false;
				event.preventDefault();
				event.stopImmediatePropagation();
				return false;
			}

			if (event.which == 27) {
				// Escape — limpa o atalho
				ShortCutsTemp[currentSC] = {};
				ShortCutsTemp[currentSC].cust = {};
				ShortCutsTemp[currentSC].cust.key = '';
				ShortCutsTemp[currentSC].cust.alt = false;
				ShortCutsTemp[currentSC].cust.ctrl = false;
				ShortCutsTemp[currentSC].cust.shift = false;
			} else {
				const conflictSC = tempMatch(event.which);

				if (conflictSC && conflictSC !== currentSC) {
					const oldKey = getKey(currentSC);
					const oldAlt = getAlt(currentSC);
					const oldCtrl = getCtrl(currentSC);
					const oldShift = getShift(currentSC);

					ShortCutsTemp[conflictSC] = {};
					ShortCutsTemp[conflictSC].cust = {};
					ShortCutsTemp[conflictSC].cust.key = oldKey;
					ShortCutsTemp[conflictSC].cust.alt = oldAlt;
					ShortCutsTemp[conflictSC].cust.ctrl = oldCtrl;
					ShortCutsTemp[conflictSC].cust.shift = oldShift;

					const conflictCell = root.querySelector("td[data-button='" + conflictSC + "']");
					if (conflictCell) {
						conflictCell.classList.add('changed');
						conflictCell.textContent =
							(oldAlt ? 'ALT + ' : '') +
							(oldCtrl ? 'CTRL + ' : '') +
							(oldShift ? 'SHIFT + ' : '') +
							(oldKey ? KEYS.toReadableKey(parseInt(oldKey, 10)) : 'N/A');
					}
				}

				ShortCutsTemp[currentSC] = {};
				ShortCutsTemp[currentSC].cust = {};
				ShortCutsTemp[currentSC].cust.key = event.which;
				ShortCutsTemp[currentSC].cust.alt = KEYS.ALT;
				ShortCutsTemp[currentSC].cust.ctrl = KEYS.CTRL;
				ShortCutsTemp[currentSC].cust.shift = KEYS.SHIFT;
			}

			box.textContent =
				(getAlt(currentSC) ? 'ALT + ' : '') +
				(getCtrl(currentSC) ? 'CTRL + ' : '') +
				(getShift(currentSC) ? 'SHIFT + ' : '') +
				KEYS.toReadableKey(getKey(currentSC), 10);

			root.querySelectorAll('td.selected').forEach(function (el) {
				el.classList.add('changed');
				el.classList.remove('selected');
			});
			ShortCutOption.isCapturing = false;

			event.preventDefault();
			event.stopImmediatePropagation();
			return false;
		}
	}
};

/**
 * Updates the key list on the UI
 */
function updateKeyList() {
	const root = ShortCutOption.getRoot();
	const cells = root.querySelectorAll('td[data-button]');
	for (let i = 0; i < cells.length; i++) {
		const btnName = cells[i].dataset.button;
		if (getKey(btnName)) {
			cells[i].textContent =
				(getAlt(btnName) ? 'ALT + ' : '') +
				(getCtrl(btnName) ? 'CTRL + ' : '') +
				(getShift(btnName) ? 'SHIFT + ' : '') +
				KEYS.toReadableKey(parseInt(getKey(btnName), 10));
		} else {
			cells[i].textContent = 'N/A';
		}
	}
}

/**
 * Resets key bindings to initial
 */
function resetKeysToDefault() {
	const root = ShortCutOption.getRoot();
	Object.keys(ShortCuts).forEach(function (SC) {
		// Set empty customs
		ShortCutsTemp[SC] = {};
		ShortCutsTemp[SC].cust = false;
		const cell = root.querySelector("td[data-button='" + SC + "']");
		if (cell) {
			if (ShortCuts[SC].cust != ShortCutsTemp[SC].cust) {
				cell.classList.add('changed');
			} else {
				cell.classList.remove('changed');
			}
		}
	});
	updateKeyList();
}

/**
 * Applies the key bindings
 */
function applySettings() {
	Object.keys(ShortCutsTemp).forEach(function (SC) {
		// Copy settings
		if (ShortCutsTemp[SC].cust) {
			ShortCuts[SC].cust = {};
			ShortCuts[SC].cust.key = ShortCutsTemp[SC].cust.key;
			ShortCuts[SC].cust.alt = ShortCutsTemp[SC].cust.alt;
			ShortCuts[SC].cust.ctrl = ShortCutsTemp[SC].cust.ctrl;
			ShortCuts[SC].cust.shift = ShortCutsTemp[SC].cust.shift;
		} else {
			ShortCuts[SC].cust = false;
		}
	});

	ShortCutControls.save();
	BattleMode.reload();
	ShortCutsTemp = {};
	updateKeyList();

	const root = ShortCutOption.getRoot();
	root.querySelectorAll('td.changed').forEach(function (el) {
		el.classList.remove('changed');
	});

	// Update ShortCut tooltips if the component is loaded
	const ShortCut = UIManager.getComponent('ShortCut');
	if (ShortCut && ShortCut.updateAllTooltips) {
		ShortCut.updateAllTooltips();
	}
}

/**
 * Cancels the key bindings
 */
function cancelSettings() {
	ShortCutsTemp = {};
	updateKeyList();

	const root = ShortCutOption.getRoot();
	root.querySelectorAll('td.changed').forEach(function (el) {
		el.classList.remove('changed');
	});
}

/**
 * Get shortcut key setting
 */
function getKey(sc) {
	if (ShortCutsTemp[sc]) {
		return ShortCutsTemp[sc].cust ? ShortCutsTemp[sc].cust.key : ShortCuts[sc].init.key;
	} else if (ShortCuts[sc]) {
		return ShortCuts[sc].cust ? ShortCuts[sc].cust.key : ShortCuts[sc].init.key;
	} else {
		return false;
	}
}

/**
 * Get shortcut alt setting
 */
function getAlt(sc) {
	if (ShortCutsTemp[sc]) {
		return ShortCutsTemp[sc].cust ? ShortCutsTemp[sc].cust.alt : ShortCuts[sc].init.alt;
	} else if (ShortCuts[sc]) {
		return ShortCuts[sc].cust ? ShortCuts[sc].cust.alt : ShortCuts[sc].init.alt;
	} else {
		return false;
	}
}

/**
 * Get shortcut ctrl setting
 */
function getCtrl(sc) {
	if (ShortCutsTemp[sc]) {
		return ShortCutsTemp[sc].cust ? ShortCutsTemp[sc].cust.ctrl : ShortCuts[sc].init.ctrl;
	} else if (ShortCuts[sc]) {
		return ShortCuts[sc].cust ? ShortCuts[sc].cust.ctrl : ShortCuts[sc].init.ctrl;
	} else {
		return false;
	}
}

/**
 * Get shortcut shift setting
 */
function getShift(sc) {
	if (ShortCutsTemp[sc]) {
		return ShortCutsTemp[sc].cust ? ShortCutsTemp[sc].cust.shift : ShortCuts[sc].init.shift;
	} else if (ShortCuts[sc]) {
		return ShortCuts[sc].cust ? ShortCuts[sc].cust.shift : ShortCuts[sc].init.shift;
	} else {
		return false;
	}
}

// Joystick
function onUpdateTargetOption() {
	Controls.attackTargetMode = parseInt(this.value, 10);
	Controls.save();
}

function onUpdateSense() {
	Controls.joySense = parseFloat(this.value, 10);
	Controls.save();
}

function onUpdateJoyQuick() {
	Controls.joyQuick = parseInt(this.value, 10);
	Controls.save();
}

function onUpdateJoyDeadline() {
	Controls.joyDeadline = parseInt(this.value, 10);
	Controls.save();
}

function onUpdateAutoHide() {
	Controls.joyAutoHide = !!this.checked;
	Controls.save();
}

function onUpdateReverseStick() {
	Controls.joyReverseStick = !!this.checked;
	Controls.save();
}

function onUpdateDisableVirtualMouse() {
	Controls.joyDisableVirtualMouse = !!this.checked;
	Controls.save();
}

ShortCutOption.mouseMode = GUIComponent.MouseMode.STOP;
ShortCutOption.needFocus = true;
export default UIManager.addComponent(ShortCutOption);
