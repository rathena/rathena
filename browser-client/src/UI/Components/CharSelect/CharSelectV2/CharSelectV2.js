/**
 * UI/Components/CharSelect/CharSelectV2/CharSelectV2.js
 *
 * Chararacter Selection windows
 *
 * This file is part of ROBrowser, Ragnarok Online in the Web Browser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharSelectV2.html?raw';
import cssText from './CharSelectV2.css?raw';
import { createCharSelect } from '../CharSelectCommon.js';

export default createCharSelect({
	name: 'CharSelectV2',
	htmlText,
	cssText,
	hostHeight: 358,
	deleteReservation: true,
	packetverGatedDelete: true
});
