/**
 * UI/Components/SkillList/SkillList/SkillList.js
 *
 * Chararacter Skill Window (original / oldest version: plain list only, instant
 * per-skill level-up, no tabs, no tree view and no skill-point pre-distribution).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createSkillList } from '../SkillListCommon.js';
import htmlText from './SkillList.html?raw';
import cssText from './SkillList.css?raw';

export default createSkillList({
	name: 'SkillList',
	htmlText: htmlText,
	cssText: cssText,
	listOnly: true,
	containerSelector: '.SkillList',
	preferenceDefaults: {
		x: 100,
		y: 200,
		width: 8,
		height: 8,
		show: false
	}
});
