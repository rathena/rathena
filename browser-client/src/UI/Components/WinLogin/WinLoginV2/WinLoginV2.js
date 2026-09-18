/**
 * UI/Components/WinLoginV2/WinLoginV2.js
 *
 * WinLoginV2 windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Dependencies
 */
import htmlText from './WinLoginV2.html?raw';
import cssText from './WinLoginV2.css?raw';
import { createWinLogin } from '../WinLoginCommon.js';

export default createWinLogin({
	name: 'WinLoginV2',
	htmlText,
	cssText
});
