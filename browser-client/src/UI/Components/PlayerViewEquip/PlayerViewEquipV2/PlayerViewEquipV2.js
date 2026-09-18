/**
 * UI/Components/Equipment/PlayerViewEquipV2/PlayerViewEquipV2.js
 *
 * Show a player equip when allowed
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import baseCss from '../PlayerViewEquip.css?raw';
import { createPlayerViewEquip } from '../PlayerViewEquipCommon.js';

const costumeRows = [
	{ left: 'costume_head_top', right: 'costume_head_mid' },
	{ left: 'costume_head_bottom', right: 'shadow_armor' },
	{ left: 'shadow_weapon', right: 'shadow_shield' },
	{ left: 'shadow_garment', right: 'shadow_shoes' },
	{ left: 'shadow_accessory1', right: 'shadow_accessory2' }
];

export default createPlayerViewEquip({
	name: 'PlayerViewEquipV2',
	cssText: baseCss,
	hasTabs: true,
	costumeRows,
	costumeTableBg: 'basic_interface/equipwin_special.bmp'
});
