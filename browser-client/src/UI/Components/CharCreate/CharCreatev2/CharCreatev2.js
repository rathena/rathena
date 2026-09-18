/**
 * UI/Components/CharCreate/CharCreatev2/CharCreatev2.js
 *
 * Chararacter Creation windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharCreatev2.html?raw';
import cssText from './CharCreatev2.css?raw';
import { createCharCreate } from '../CharCreateCommon.js';

export default createCharCreate({
	name: 'CharCreatev2',
	htmlText,
	cssText,
	hostHeight: 286,
	hostWidth: 150,
	chargenCanvasSelector: '.content canvas',
	hairArrows: [
		{ selector: '.content .styleleft', type: 'head', value: -1 },
		{ selector: '.content .styleright', type: 'head', value: 1 },
		{ selector: '.content .colorleft', type: 'headpalette', value: -1 },
		{ selector: '.content .colorright', type: 'headpalette', value: 1 }
	],
	nameInputSelector: 'input',
	cancelSelectors: ['.cancel'],
	makeSelector: '.make'
});
