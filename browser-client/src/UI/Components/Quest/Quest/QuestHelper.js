/**
 * UI/Components/Quest/Quest/QuestHelper.js
 *
 * Quest information window (renewal layout).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createQuestHelper } from '../QuestHelperCommon.js';
import htmlText from './QuestHelper.html?raw';
import cssText from './QuestHelper.css?raw';

export default createQuestHelper({
	name: 'QuestHelper',
	htmlText,
	cssText,
	preferencesKey: 'Quest',
	renewLayout: true
});
