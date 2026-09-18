/**
 * UI/Components/Quest/QuestV1/QuestHelperV1.js
 *
 * Quest information window (classic layout).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createQuestHelper } from '../QuestHelperCommon.js';
import htmlText from './QuestHelperV1.html?raw';
import cssText from './QuestHelperV1.css?raw';

export default createQuestHelper({
	name: 'QuestHelperV1',
	htmlText,
	cssText,
	preferencesKey: 'QuestHelperV1',
	renewLayout: false
});
