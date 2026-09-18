/**
 * UI/Components/BasicInfoV3/BasicInfoV3.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './BasicInfoV3.html?raw';
import cssText from './BasicInfoV3.css?raw';
import { createBasicInfo } from '../BasicInfoCommon.js';

export default createBasicInfo({
	name: 'BasicInfoV3',
	htmlText,
	cssText,
	prefKey: 'BasicInfoV3',
	reduceDefault: true,
	innerId: '#BasicInfoV3',
	topbarItemSelector: '.topbar div',
	toggleButtonsEvent: 'mousedown',
	buttonsSelector: '.buttons div',
	buttonsEvent: 'mousedown',
	buttonKeyBy: 'id',
	hasToolbarToggle: true,
	hideIds: ['battle', 'replay'],
	barScale: 1.27
});
