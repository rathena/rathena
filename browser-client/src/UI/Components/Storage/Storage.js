/**
 * UI/Components/Storage/Storage.js
 *
 * Character Storage Window
 *
 * Note: For different versions, please use different Object names and main div IDs to avoid conflicts in settings and styles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import StorageV0 from './StorageV0/Storage.js'; // Basic Storage
import StorageV3 from './StorageV3/Storage.js'; // Expanded Storage (search tab)

import UIVersionManager from 'UI/UIVersionManager.js';

const publicName = 'Storage';

const versionInfo = {
	default: StorageV0, // Basic Storage
	common: {
		20181219: StorageV3 // Expanded Storage (search tab)
	},
	re: {},
	prere: {}
};

const StorageController = UIVersionManager.getUIController(publicName, versionInfo);

const _selectUIVersion = StorageController.selectUIVersion;

// Extend default UI selector
StorageController.selectUIVersion = function () {
	_selectUIVersion();
};

// Forward methods to the implementation
const _methods = ['reqAddItem', 'reqAddItemFromCart', 'reqRemoveItem', 'reqMoveItemToCart', 'onClosePressed'];

_methods.forEach(function (method) {
	Object.defineProperty(StorageController, method, {
		set: function (value) {
			StorageV0[method] = value;
			StorageV3[method] = value;
		},
		get: function () {
			return (StorageController.getUI() || StorageV0)[method];
		}
	});
});

export default StorageController;
