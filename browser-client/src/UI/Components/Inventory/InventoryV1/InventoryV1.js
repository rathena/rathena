/**
 * UI/Components/InventoryV1/InventoryV1.js
 *
 * Chararacter Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import htmlText from './InventoryV1.html?raw';
import cssText from './InventoryV1.css?raw';
import { createInventory } from '../InventoryCommon.js';

export default createInventory({
	name: 'InventoryV1',
	htmlText,
	cssText,
	defaultHeight: 193,
	favoriteTab: true
});
