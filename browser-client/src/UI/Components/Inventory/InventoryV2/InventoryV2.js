/**
 * UI/Components/InventoryV2/InventoryV2.js
 *
 * Chararacter Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import htmlText from './InventoryV2.html?raw';
import cssText from './InventoryV2.css?raw';
import { createInventory } from '../InventoryCommon.js';

export default createInventory({
	name: 'InventoryV2',
	htmlText,
	cssText,
	defaultHeight: 193,
	favoriteTab: true,
	equipSwitch: true,
	hostDropPreventDefault: true
});
