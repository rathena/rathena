/**
 * UI/Components/CharSelect/CharSelectV3/CharSelectV3.js
 *
 * Chararacter Selection windows
 *
 * This file is part of ROBrowser, Ragnarok Online in the Web Browser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharSelectV3.html?raw';
import cssText from './CharSelectV3.css?raw';
import { createCharSelect } from '../CharSelectCommon.js';

export default createCharSelect({
	name: 'CharSelectV3',
	htmlText,
	cssText,
	hostHeight: 358,
	deleteReservation: true,
	pageBalls: true
});
