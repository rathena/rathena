/**
 * UI/Components/CharCreate/CharCreatev3/CharCreatev3.js
 *
 * Chararacter Creation windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharCreatev3.html?raw';
import cssText from './CharCreatev3.css?raw';
import { createCharCreate } from '../CharCreateCommon.js';

export default createCharCreate({
	name: 'CharCreatev3',
	htmlText,
	cssText,
	hostHeight: 342,
	hostWidth: 576,
	hasRace: true,
	humanCanvasSelector: '#canvas_human',
	doramCanvasSelector: '#canvas_doram',
	modelCanvasSelector: '#canvas_model',
	nameInputSelector: '#char_name',
	nameInputEvent: 'click',
	cancelSelectors: ['.button.close'],
	makeSelector: '.button.make'
});
