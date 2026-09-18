/**
 * UI/Components/Equipment/EquipmentV4/EquipmentV4.js
 *
 * Chararacter Equipment window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createEquipment } from '../EquipmentCommon.js';
import htmlText from './EquipmentV4.html?raw';
import cssText from './EquipmentV4.css?raw';

export default createEquipment({
	name: 'EquipmentV4',
	htmlText,
	cssText,
	enchantGrade: true,
	switchEquip: true,
	titles: true,
	costumeConfig: true,
	damageSkin: true
});
