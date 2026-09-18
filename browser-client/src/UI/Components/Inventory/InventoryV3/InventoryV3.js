/**
 * UI/Components/InventoryV3/InventoryV3.js
 *
 * Chararacter Inventory
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import htmlText from './InventoryV3.html?raw';
import cssText from './InventoryV3.css?raw';
import { createInventory } from '../InventoryCommon.js';

export default createInventory({
	name: 'InventoryV3',
	htmlText,
	cssText,
	defaultHeight: 194,
	favoriteTab: true,
	equipSwitch: true,
	hostDropPreventDefault: true,
	enchantGrade: true,
	inventoryExpansion: true,
	refineEnchant: true,
	useHiddenClass: true,
	tabDropPreventDefault: true
});
