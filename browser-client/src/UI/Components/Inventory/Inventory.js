/**
 * UI/Components/Inventory/Inventory.js
 *
 * Character Inventory Window
 *
 * Note: For different versions, please use different Object names and main div IDs to avoid conflicts in settings and styles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import InventoryV0 from './InventoryV0/InventoryV0.js';
import InventoryV1 from './InventoryV1/InventoryV1.js';
import InventoryV2 from './InventoryV2/InventoryV2.js';
import InventoryV3 from './InventoryV3/InventoryV3.js';

import UIVersionManager from 'UI/UIVersionManager.js';
import DB from 'DB/DBManager.js';
import KEYS from 'Controls/KeyEventHandler.js';

const publicName = 'Inventory';
const versionInfo = {
	default: InventoryV0, // Basic Inventory
	common: {
		20181219: InventoryV3, // Inventory Expansion
		20170208: InventoryV2, // Equipment Switch
		20111207: InventoryV1 // Favorite Tab
	},
	re: {},
	prere: {}
};

const InventoryController = UIVersionManager.getUIController(publicName, versionInfo);

const _selectUIVersion = InventoryController.selectUIVersion;

// Extend default UI selector
InventoryController.selectUIVersion = function () {
	_selectUIVersion();

	//Add selected UI to item owner name update queue
	const component = InventoryController.getUI();
	DB.UpdateOwnerName.Inventory = component.onUpdateOwnerName;

	// Escape to close the UI
	component.onKeyDown = function onKeyDown(e) {
		if ((e.which === KEYS.ESCAPE || e.key === 'Escape') && component.ui.is(':visible')) {
			if (typeof component.toggle === 'function') {
				component.toggle();
			}
		}
	};
};

export default InventoryController;
