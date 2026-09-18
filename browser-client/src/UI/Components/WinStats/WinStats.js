/**
 * UI/Components/WinStats/WinStats.js
 *
 * Chararacter Basic information windows
 *
 * Note: For different versions, please use different Object names and main div IDs to avoid conflicts in settings and styles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import WinStats from './WinStats/WinStats.js';
import WinStatsV1 from './WinStatsV1/WinStatsV1.js';
import WinStatsV2 from './WinStatsV2/WinStatsV2.js';
import UIVersionManager from 'UI/UIVersionManager.js';
import KEYS from 'Controls/KeyEventHandler.js';

const publicName = 'WinStats';
const versionInfo = {
    default: WinStats,
    common: {
        20090617: WinStatsV1,
        20200520: WinStatsV2
    },
    re: {},
    prere: {}
};

const WinStatsController = UIVersionManager.getUIController(publicName, versionInfo);
const _selectUIVersion = WinStatsController.selectUIVersion;

// Extend default UI selector
WinStatsController.selectUIVersion = function () {
	_selectUIVersion();

	const component = WinStatsController.getUI();

	// Escape to close the UI
	component.onKeyDown = function onKeyDown(e) {
		if ((e.which === KEYS.ESCAPE || e.key === 'Escape') && component.ui.is(':visible')) {
			if (typeof component.toggle === 'function') {
				component.toggle();
			}
		}
	};
};
export default WinStatsController;
