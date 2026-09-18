/**
 * UI/Components/Equipment/PlayerViewEquipV0/PlayerViewEquipV0.js
 *
 * Show a player equip when allowed
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import baseCss from '../PlayerViewEquip.css?raw';
import { createPlayerViewEquip } from '../PlayerViewEquipCommon.js';

export default createPlayerViewEquip({
	name: 'PlayerViewEquipV0',
	cssText: baseCss,
	hasTabs: false,
	costumeRows: null,
	costumeTableBg: null
});
