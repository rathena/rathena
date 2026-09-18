/**
 * UI/Components/BasicInfoV4/BasicInfoV4.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './BasicInfoV4.html?raw';
import cssText from './BasicInfoV4.css?raw';
import { createBasicInfo } from '../BasicInfoCommon.js';

export default createBasicInfo({
	name: 'BasicInfoV4',
	htmlText,
	cssText,
	prefKey: 'BasicInfoV4',
	reduceDefault: true,
	innerId: '#BasicInfoV4',
	topbarItemSelector: '.topbar div',
	topbarDblClick: true,
	toggleButtonsEvent: 'click',
	buttonsSelector: '.buttons button',
	buttonsEvent: 'click',
	buttonKeyBy: 'id',
	partyViaGetUI: true,
	hasToolbarToggle: true,
	hideIds: ['battle', 'replay', 'tipbox', 'shortcut', 'agency'],
	barScale: 1.27
});
