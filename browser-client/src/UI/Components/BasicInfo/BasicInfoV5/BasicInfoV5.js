/**
 * UI/Components/BasicInfoV5/BasicInfoV5.js
 *
 * Chararacter Basic information windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './BasicInfoV5.html?raw';
import cssText from './BasicInfoV5.css?raw';
import { createBasicInfo } from '../BasicInfoCommon.js';

export default createBasicInfo({
	name: 'BasicInfoV5',
	htmlText,
	cssText,
	prefKey: 'BasicInfoV5',
	reduceDefault: true,
	innerId: '#BasicInfoV5',
	topbarItemSelector: '.topbar div',
	topbarDblClick: true,
	toggleButtonsEvent: 'click',
	buttonsSelector: '.buttons > div[id]',
	buttonsEvent: 'click',
	buttonKeyBy: 'id',
	partyViaGetUI: true,
	hasToolbarToggle: true,
	hideIds: ['battle', 'replay', 'tipbox', 'shortcut', 'agency'],
	barScale: 1.27,
	hasApBar: true
});
