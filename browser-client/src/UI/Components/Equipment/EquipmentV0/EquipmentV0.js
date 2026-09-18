/**
 * UI/Components/Equipment/EquipmentV0/EquipmentV0.js
 *
 * Chararacter Equipment window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createEquipment } from '../EquipmentCommon.js';
import htmlText from './EquipmentV0.html?raw';
import cssText from './EquipmentV0.css?raw';

export default createEquipment({
	name: 'EquipmentV0',
	htmlText,
	cssText,
	entityRender: false,
	statsDefault: false
});
