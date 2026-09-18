/**
 * UI/Components/Equipment/Equipment.js
 *
 * Chararacter Basic information windows
 *
 * Note: For different versions, please use different Object names and main div IDs to avoid conflicts in settings and styles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import EquipmentV0 from './EquipmentV0/EquipmentV0.js';
import EquipmentV1 from './EquipmentV1/EquipmentV1.js';
import EquipmentV2 from './EquipmentV2/EquipmentV2.js';
import EquipmentV3 from './EquipmentV3/EquipmentV3.js';
import EquipmentV4 from './EquipmentV4/EquipmentV4.js';

import UIVersionManager from 'UI/UIVersionManager.js';
import DB from 'DB/DBManager.js';
import KEYS from 'Controls/KeyEventHandler.js';

const publicName = 'Equipment';
const versionInfo = {
	default: EquipmentV0,
	common: {
		20220831: EquipmentV4,
		20170208: EquipmentV3,
		20150225: EquipmentV2,
		20101124: EquipmentV1
	},
	re: {},
	prere: {}
};

const EquipmentController = UIVersionManager.getUIController(publicName, versionInfo);

const _selectUIVersion = EquipmentController.selectUIVersion;

// Extend default UI selector
EquipmentController.selectUIVersion = function () {
	_selectUIVersion();

	//Add selected UI to item owner name update queue
	const component = EquipmentController.getUI();
	DB.UpdateOwnerName.Equipment = component.onUpdateOwnerName;

	// Escape to close the UI
	component.onKeyDown = function onKeyDown(e) {
		if ((e.which === KEYS.ESCAPE || e.key === 'Escape') && component.ui.is(':visible')) {
			if (typeof component.toggle === 'function') {
				component.toggle();
			}
		}
	};
};

export default EquipmentController;
