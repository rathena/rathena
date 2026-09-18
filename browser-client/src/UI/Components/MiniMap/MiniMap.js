/**
 * UI/Components/MiniMap/MiniMap.js
 *
 * MiniMap windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import MiniMap from './MiniMap/MiniMap.js';
import MiniMapV2 from './MiniMapV2/MiniMapV2.js';
import UIVersionManager from 'UI/UIVersionManager.js';

const publicName = 'MiniMap';
const versionInfo = {
	default: MiniMap,
	common: {
		20180124: MiniMapV2
	},
	re: {},
	prere: {}
};

const Controller = UIVersionManager.getUIController(publicName, versionInfo);

/**
 * Proxy for getMemberColor
 */
Controller.getMemberColor = function getMemberColor(key) {
	const ui = Controller.getUI();
	return ui && ui.getMemberColor ? ui.getMemberColor(key) : 'white';
};
export default Controller;
