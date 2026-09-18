/**
 * UI/Components/Equipment/EquipmentV1/EquipmentV1.js
 *
 * Chararacter Equipment window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createEquipment } from '../EquipmentCommon.js';
import htmlText from './EquipmentV1.html?raw';
import cssText from './EquipmentV1.css?raw';

export default createEquipment({ name: 'EquipmentV1', htmlText, cssText });
