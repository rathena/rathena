/**
 * UI/Components/CharSelect/CharSelect.js
 *
 * Character Selection
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import CharSelect from './CharSelect/CharSelect.js';
import CharSelectV2 from './CharSelectV2/CharSelectV2.js';
import CharSelectV3 from './CharSelectV3/CharSelectV3.js';
import CharSelectV4 from './CharSelectV4/CharSelectV4.js';
import UIVersionManager from 'UI/UIVersionManager.js';

const publicName = 'CharSelect';
const versionInfo = {
	default: CharSelect,
	common: {
		20180124: CharSelectV4,
		20141016: CharSelectV3,
		20100803: CharSelectV2,
		20100728: CharSelect,
		20100720: CharSelectV2
	},
	re: {},
	prere: {}
};

const Controller = UIVersionManager.getUIController(publicName, versionInfo);
export default Controller;
