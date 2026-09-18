/**
 * UI/Components/Quest/QuestV1/QuestV1.js
 *
 * Quest List window (classic layout).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createQuest } from '../QuestCommon.js';
import QuestHelper from './QuestHelperV1.js';
import htmlText from './QuestV1.html?raw';
import cssText from './QuestV1.css?raw';

export default createQuest({
	name: 'QuestV1',
	htmlText,
	cssText,
	questHelper: QuestHelper,
	renewLayout: false
});
