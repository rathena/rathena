/**
 * UI/Components/SkillList/SkillListV2/SkillListV2.js
 *
 * Chararacter Skill Window (Episode 13.1 renewal redesign: tabbed list + tree
 * views with skill-point pre-distribution / apply / reset).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createSkillList } from '../SkillListCommon.js';
import htmlText from './SkillListV2.html?raw';
import cssText from './SkillListV2.css?raw';

export default createSkillList({
	name: 'SkillListV2',
	htmlText: htmlText,
	cssText: cssText,
	hasTabs: true,
	showDescOnMiniHover: false,
	touchDrag: true,
	guardMissingJob: true,
	readdSkillOnUpdate: true,
	dragFrom: 'SkillList'
});
