/**
 * UI/Components/Equipment/EquipmentV3/EquipmentV3.js
 *
 * Chararacter Equipment window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createEquipment } from '../EquipmentCommon.js';
import htmlText from './EquipmentV3.html?raw';
import cssText from './EquipmentV3.css?raw';

export default createEquipment({
	name: 'EquipmentV3',
	htmlText,
	cssText,
	enchantGrade: true,
	switchEquip: true,
	titles: true
});
