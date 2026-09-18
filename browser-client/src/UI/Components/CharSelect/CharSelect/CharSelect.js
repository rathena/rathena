/**
 * UI/Components/CharSelect/CharSelect.js
 *
 * Chararacter Selection windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './CharSelect.html?raw';
import cssText from './CharSelect.css?raw';
import { createCharSelect } from '../CharSelectCommon.js';

export default createCharSelect({
	name: 'CharSelect',
	htmlText,
	cssText,
	hostHeight: 342
});
