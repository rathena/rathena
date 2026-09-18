/**
 * UI/Components/WinStats/WinStatsV1.js
 *
 * Chararacter Statistiques Informations
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import htmlText from './WinStatsV1.html?raw';
import cssText from './WinStatsV1.css?raw';
import { createWinStats } from '../WinStatsCommon.js';

export default createWinStats({
	name: 'WinStatsV1',
	htmlText,
	cssText,
	hasTraits: false
});
