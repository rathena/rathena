/**
 * UI/Components/Equipment/EquipmentV2/EquipmentV2.js
 *
 * Chararacter Equipment window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createEquipment } from '../EquipmentCommon.js';
import htmlText from './EquipmentV2.html?raw';
import cssText from './EquipmentV2.css?raw';

export default createEquipment({ name: 'EquipmentV2', htmlText, cssText });
