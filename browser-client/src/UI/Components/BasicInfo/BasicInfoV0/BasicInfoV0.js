/**
 * UI/Components/BasicInfoV0/BasicInfoV0.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './BasicInfoV0.html?raw';
import cssText from './BasicInfoV0.css?raw';
import { createBasicInfo } from '../BasicInfoCommon.js';

export default createBasicInfo({
	name: 'BasicInfoV0',
	htmlText,
	cssText,
	prefKey: 'BasicInfoV0',
	reduceDefault: false,
	innerId: '#BasicInfoV0',
	topbarItemSelector: '.topbar button',
	toggleButtonsEvent: 'mousedown',
	buttonsSelector: '.buttons button',
	buttonsEvent: 'mousedown',
	buttonKeyBy: 'class',
	miniLayout: true,
	barScale: 0.77
});
