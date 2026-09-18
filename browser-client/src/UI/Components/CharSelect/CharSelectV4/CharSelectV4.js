/**
 * UI/Components/CharSelect/CharSelectV4/CharSelectV4.js
 *
 * Chararacter Selection windows
 *
 * This file is part of ROBrowser, Ragnarok Online in the Web Browser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharSelectV4.html?raw';
import cssText from './CharSelectV4.css?raw';
import { createCharSelect } from '../CharSelectCommon.js';

export default createCharSelect({
	name: 'CharSelectV4',
	htmlText,
	cssText,
	gridLayout: true,
	deleteReservation: true,
	defaultMaxSlots: 15
});
