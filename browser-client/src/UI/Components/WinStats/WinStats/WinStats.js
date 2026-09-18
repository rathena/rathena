/**
 * UI/Components/WinStats/WinStats.js
 *
 * Chararacter Statistiques Informations
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './WinStats.html?raw';
import cssText from './WinStats.css?raw';
import { createWinStats } from '../WinStatsCommon.js';

export default createWinStats({
	name: 'WinStats',
	htmlText,
	cssText,
	hasTraits: false
});
