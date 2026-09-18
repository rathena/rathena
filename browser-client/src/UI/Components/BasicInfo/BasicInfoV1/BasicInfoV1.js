/**
 * UI/Components/BasicInfo/BasicInfoV1.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './BasicInfoV1.html?raw';
import cssText from './BasicInfoV1.css?raw';
import { createBasicInfo } from '../BasicInfoCommon.js';

export default createBasicInfo({
	name: 'BasicInfoV1',
	htmlText,
	cssText,
	prefKey: 'BasicInfoV1',
	reduceDefault: true,
	innerId: '#BasicInfoV1',
	topbarItemSelector: '.topbar button',
	toggleButtonsEvent: 'mousedown',
	buttonsSelector: '.buttons button',
	buttonsEvent: 'mousedown',
	buttonKeyBy: 'class',
	infoOpensWinStats: false,
	barScale: 1.27
});
