/**
 * UI/Components/BasicInfo/BasicInfo.js
 *
 * Chararacter Basic information windows
 *
 * Note: For different versions, please use different Object names and main div IDs to avoid conflicts in settings and styles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import BasicInfoV1 from './BasicInfoV1/BasicInfoV1.js';
import BasicInfoV0 from './BasicInfoV0/BasicInfoV0.js';
import BasicInfoV3 from './BasicInfoV3/BasicInfoV3.js';
import BasicInfoV4 from './BasicInfoV4/BasicInfoV4.js';
import BasicInfoV5 from './BasicInfoV5/BasicInfoV5.js';
import UIVersionManager from 'UI/UIVersionManager.js';

const publicName = 'BasicInfo';
const versionInfo = {
	default: BasicInfoV0,
	common: {
		20200520: BasicInfoV5, // not sure the exact client date that started supporting 4th Jobs
		20180124: BasicInfoV4,
		20160101: BasicInfoV3,
		20090601: BasicInfoV1
	},
	re: {},
	prere: {},
	job: {
		Fourth_Class: BasicInfoV5,
		default: BasicInfoV4
	}
};

const BasicInfoController = UIVersionManager.getUIController(publicName, versionInfo);
export default BasicInfoController;
