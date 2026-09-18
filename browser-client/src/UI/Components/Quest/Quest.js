/**
 * UI/Components/Quest/Quest.js
 *
 * Quest Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import Quest from './Quest/Quest.js';
import QuestV1 from './QuestV1/QuestV1.js';
import UIVersionManager from 'UI/UIVersionManager.js';
import KEYS from 'Controls/KeyEventHandler.js';

const publicName = 'Quest';
const versionInfo = {
	default: QuestV1,
	common: {
		20180307: Quest
	},
	re: {},
	prere: {}
};

const Controller = UIVersionManager.getUIController(publicName, versionInfo);
const _selectUIVersion = Controller.selectUIVersion;

// Extend default UI selector
Controller.selectUIVersion = function () {
	_selectUIVersion();

	const component = Controller.getUI();

	// Escape to close the UI
	component.onKeyDown = function onKeyDown(e) {
		if ((e.which === KEYS.ESCAPE || e.key === 'Escape') && component.ui.is(':visible')) {
			if (typeof component.toggle === 'function') {
				component.toggle();
			}
		}
	};
};
export default Controller;
