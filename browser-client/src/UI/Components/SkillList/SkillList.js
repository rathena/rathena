/**
 * UI/Components/SkillList/SkillList.js
 *
 * SkillWindow
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import SkillList from './SkillList/SkillList.js';
import SkillListV2 from './SkillListV2/SkillListV2.js';
import UIVersionManager from 'UI/UIVersionManager.js';
import KEYS from 'Controls/KeyEventHandler.js';

const publicName = 'SkillList';
const versionInfo = {
	default: SkillList,
	common: {
		20090601: SkillListV2
	},
	re: {},
	prere: {}
};

const Controller = UIVersionManager.getUIController(publicName, versionInfo);
const _selectUIVersion = Controller.selectUIVersion;

Controller.selectUIVersion = function () {
	_selectUIVersion();

	const component = Controller.getUI();

	component.onKeyDown = e => {
		if ((e.which === KEYS.ESCAPE || e.key === 'Escape') && component.ui.is(':visible')) {
			if (typeof component.toggle === 'function') {
				component.toggle();
			}
		}
	};
};
export default Controller;
