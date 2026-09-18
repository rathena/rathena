/**
 * UI/Components/Storage/StorageV0/Storage.js
 *
 * Account Storage
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import { createStorage } from '../StorageCommon.js';
import htmlText from './Storage.html?raw';
import cssText from './Storage.css?raw';

export default createStorage({
	name: 'Storage',
	htmlText,
	cssText,
	asyncDragImage: true
});
