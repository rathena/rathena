/**
 * UI/Components/Equipment/PlayerViewEquipV1/PlayerViewEquipV1.js
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
	{ left: 'costume_head_bottom', right: 'shadow_garment' },
	{ left: 'shadow_floor', right: 'unknown1' },
	{ left: 'unknown2', right: 'unknown3' },
	{ left: 'unknown4', right: 'unknown5' }
];

export default createPlayerViewEquip({
	name: 'PlayerViewEquipV1',
	cssText: baseCss,
	hasTabs: true,
	costumeRows,
	costumeTableBg: 'basic_interface/equipwin_costume.bmp'
});
