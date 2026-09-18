/**
 * UI/Components/CharCreate/CharCreate.js
 *
 * Chararacter Creation windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharCreate.html?raw';
import cssText from './CharCreate.css?raw';
import { createCharCreate } from '../CharCreateCommon.js';

export default createCharCreate({
	name: 'CharCreate',
	htmlText,
	cssText,
	hostHeight: 342,
	hostWidth: 576,
	hasStats: true,
	chargenCanvasSelector: '.chargen canvas',
	graphCanvasSelector: '.graph canvas',
	statButtonsSelector: '.graph ui-button',
	hairArrows: [
		{ selector: '.chargen .left', type: 'head', value: -1 },
		{ selector: '.chargen .right', type: 'head', value: 1 },
		{ selector: '.chargen .up', type: 'headpalette', value: 1 }
	],
	nameInputSelector: 'input',
	cancelSelectors: ['.cancel'],
	makeSelector: '.make'
});
