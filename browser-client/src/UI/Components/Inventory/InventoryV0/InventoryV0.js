/**
 * UI/Components/InventoryV0/InventoryV0.js
 *
 * Chararacter Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import htmlText from './InventoryV0.html?raw';
import cssText from './InventoryV0.css?raw';
import { createInventory } from '../InventoryCommon.js';

export default createInventory({
	name: 'InventoryV0',
	htmlText,
	cssText,
	defaultHeight: 2,
	resizableHeight: true,
	tabSprite: true
});
