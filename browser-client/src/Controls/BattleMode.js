/**
 * Controls/BattleMode.js
 *
 * Manage the battle mode
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import KEYS from 'Controls/KeyEventHandler.js';
import ProcessCommand from 'Controls/ProcessCommand.js';
import Preferences from 'Preferences/ShortCutControls.js';
import UIManager from 'UI/UIManager.js';

let KeyTable = getKeyTable();

/**
 * Create Namespace
 */
class BattleMode {
	/**
	 * Update key table if setting changes
	 */
	static reload() {
		KeyTable = getKeyTable();
	}

	static getKeyName(keyId) {
		let keyName = keyId;

		if (KEYS.SHIFT) {
			keyName = `SHIFT-${keyName}`;
		}
		if (KEYS.ALT) {
			keyName = `ALT-${keyName}`;
		}
		if (KEYS.CTRL) {
			keyName = `CTRL-${keyName}`;
		}

		return keyName;
	}

	static match(keyId) {
		return KeyTable[BattleMode.getKeyName(keyId)];
	}

	/**
	 * BattleMode processing
	 *
	 * @param {number} key pressed id
	 * @return {boolean} is shortcut found ?
	 */
	static process(keyId) {
		if (UIManager.getComponent('ShortCutOption').isCapturing) {
			return false;
		}

		const keyName = BattleMode.getKeyName(keyId);

		const key = KeyTable[keyName];
		if (key) {
			if (key.component === '_SLASHCOMMAND') {
				ProcessCommand.processCommand(key.cmd);
			} else {
				const component = UIManager.getComponent(key.component);
				if (component.onShortCut) {
					component.onShortCut(key);
				}
			}
			return true;
		}
		return false;
	}

	/**
	 * Convert component key to a readable string
	 *
	 * @param {string} component name
	 * @param {string} command type
	 * @return {string} readable key pressed
	 */
	static shortcutToKeyString(component, cmd) {
		let shortcut;
		let i;

		const keys = Object.keys(KeyTable);
		const count = keys.length;

		for (i = 0; i < count; ++i) {
			shortcut = KeyTable[keys[i]];

			if (shortcut.component === component && shortcut.cmd === cmd) {
				const str = [];
				const keyName = keys[i];

				// Parse modifier prefixes and numeric key code from compound key string
				// Format: "CTRL-ALT-SHIFT-65"
				if (keyName.indexOf('CTRL-') !== -1) {
					str.push('CTRL');
				}

				if (keyName.indexOf('ALT-') !== -1) {
					str.push('ALT');
				}

				if (keyName.indexOf('SHIFT-') !== -1) {
					str.push('SHIFT');
				}

				// The numeric key code is always the last segment after the final '-'
				const lastDash = keyName.lastIndexOf('-');
				const keyCode = parseInt(lastDash !== -1 ? keyName.substring(lastDash + 1) : keyName, 10);
				const tmp = KEYS.toReadableKey(keyCode);

				if (tmp) {
					str.push(tmp);
				}

				return str.join(' + ');
			}
		}

		return 'None';
	}
}
/**
 *	Translates the shortcut table into directly indexable format for event processing
 */
function getKeyTable() {
	const keySettings = {};

	const ShortCuts = Preferences.ShortCuts;

	if (ShortCuts) {
		Object.keys(ShortCuts).forEach(SC => {
			// Get initial settings
			let key = ShortCuts[SC].init.key;
			let shift = ShortCuts[SC].init.shift;
			let alt = ShortCuts[SC].init.alt;
			let ctrl = ShortCuts[SC].init.ctrl;

			// Get custom settings
			if (ShortCuts[SC].cust) {
				key = ShortCuts[SC].cust.key;
				shift = ShortCuts[SC].cust.shift;
				alt = ShortCuts[SC].cust.alt;
				ctrl = ShortCuts[SC].cust.ctrl;
			}

			// Only add if key is defined
			if (key) {
				let keyName = key;

				if (shift) {
					keyName = `SHIFT-${keyName}`;
				}
				if (alt) {
					keyName = `ALT-${keyName}`;
				}
				if (ctrl) {
					keyName = `CTRL-${keyName}`;
				}

				keySettings[keyName] = { component: ShortCuts[SC].component, cmd: ShortCuts[SC].cmd };
			}
		});
	}
	return keySettings;
}

/**
 * Export
 */
export default BattleMode;
