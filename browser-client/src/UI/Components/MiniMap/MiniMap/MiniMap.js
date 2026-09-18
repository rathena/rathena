/**
 * UI/Components/MiniMap/MiniMap.js
 *
 * MiniMap UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './MiniMap.html?raw';
import cssText from './MiniMap.css?raw';
import { createMiniMap } from '../MiniMapCommon.js';

export default createMiniMap({
	name: 'MiniMap',
	htmlText,
	cssText
});
