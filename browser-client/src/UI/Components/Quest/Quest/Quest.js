/**
 * UI/Components/Quest/Quest/Quest.js
 *
 * Quest List window (renewal layout).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createQuest } from '../QuestCommon.js';
import QuestHelper from './QuestHelper.js';
import QuestWindow from './QuestWindow.js';
import htmlText from './Quest.html?raw';
import cssText from './Quest.css?raw';

export default createQuest({
	name: 'Quest',
	htmlText,
	cssText,
	questHelper: QuestHelper,
	questWindow: QuestWindow,
	renewLayout: true
});
