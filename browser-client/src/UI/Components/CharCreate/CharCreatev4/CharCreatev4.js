/**
 * UI/Components/CharCreate/CharCreatev4/CharCreatev4.js
 *
 * Chararacter Creation windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharCreatev4.html?raw';
import cssText from './CharCreatev4.css?raw';
import { createCharCreate } from '../CharCreateCommon.js';

export default createCharCreate({
	name: 'CharCreatev4',
	htmlText,
	cssText,
	hostHeight: 342,
	hostWidth: 576,
	hasRace: true,
	gridHairstyle: true,
	humanCanvasSelector: '#human',
	doramCanvasSelector: '#doram',
	modelCanvasSelector: '#style_model',
	nameInputSelector: '#char_name',
	nameInputEvent: 'mousedown',
	cancelSelectors: ['.cancel', '.return'],
	makeSelector: '.make'
});
