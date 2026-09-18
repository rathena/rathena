/**
 * Engine/MapEngine/Storage.js
 *
 * Manage Storage sockets
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

/**
 * Load dependencies
 */
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import Storage from 'UI/Components/Storage/Storage.js';

/*
 * This will hold the items to append to storage
 * Since STORE_EQUIPMENT_ITEMLIST packets are sent before NOTIFY_STOREITEM_COUNTINFO
 * And they are only sent if theres item's on storage!
 */
let itemBuffer = [];

/**
 * Holds the name for the Inventory type received from server
 */
let InvTypeName = '';

/**
 * Get storage informations
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_STOREITEM_COUNTINFO
 */
function onStorageInfo(pkt) {
	if (!(Storage.getUI().__loaded && Storage.getUI().__active)) {
		Storage.getUI().append();
		// Update Storage Title based on InvTypeName
		if (PACKETVER.value >= 20181002) {
			Storage.getUI().ui.find('.titlebar .text').text(InvTypeName);
		}
	}
	Storage.getUI().setItemInfo(pkt.curCount, pkt.maxCount);
	Storage.getUI().setItems(itemBuffer);

	itemBuffer = [];
}

/**
 * Add items to storage
 *
 * @param {object} pkt - PACKET.ZC.STORE_EQUIPMENT_ITEMLIST3
 */
function onStorageList(pkt) {
	itemBuffer = itemBuffer.concat(pkt.ItemInfo || pkt.itemInfo);
}

/**
 * Receive update from server to add item into the storage
 *
 * @param {object} pkt - PACKET.ZC.ADD_ITEM_TO_STORE
 */
function onStorageItemAdded(pkt) {
	Storage.getUI().addItem(Object.assign({}, pkt));
}

/**
 * Remove item from storage
 *
 * @param {object} pkt - PACKET.ZC.DELETE_ITEM_FROM_STORE
 */
function onStorageItemRemoved(pkt) {
	Storage.getUI().removeItem(pkt.index, pkt.count);
}

/**
 * Server want you to close the storage
 *
 * @param {object} pkt - PACKET.ZC.CLOSE_STORE
 */
function onStorageClose() {
	Storage.getUI().remove();
}

/**
 * Send storage close
 * PACKET.CZ.CLOSE_STORE
 */
Storage.onClosePressed = function onClosePressed() {
	const pkt = new PACKET.CZ.CLOSE_STORE();
	Network.sendPacket(pkt);

	Storage.getUI().remove();
};

/**
 * Send item to storage
 * PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE
 */
Storage.reqAddItem = function ReqAddItem(index, count) {
	if (count <= 0) {
		return;
	}

	let pkt;
	if (PACKETVER.value >= 20180307) {
		pkt = new PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE2();
	} else {
		pkt = new PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE();
	}
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

Storage.reqAddItemFromCart = function reqAddItemFromCart(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.MOVE_ITEM_FROM_CART_TO_STORE();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

/**
 * Send frm storage to inventory
 * PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY
 */
Storage.reqRemoveItem = function ReqRemoveItem(index, count) {
	if (count <= 0) {
		return;
	}

	let pkt;
	if (PACKETVER.value >= 20180307) {
		pkt = new PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY2();
	} else {
		pkt = new PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY();
	}
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

Storage.reqMoveItemToCart = function reqMoveItemToCart(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_CART();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

/**
 * Inventory Type has been started
 * Set Inventory Name based from inventory type
 * @param {object} pkt - PACKET.ZC.SPLIT_SEND_ITEMLIST_SET
 */
function onItemListSet(pkt) {
	switch (pkt.invType) {
		case 0: // Inventory - name is always blank
		case 1: // Cart - name is always blank
		case 2: // Storage
		case 3: // Guild Storage
			InvTypeName = pkt.name;
			break;
		default:
			throw new Error("[PACKET.ZC.SPLIT_SEND_ITEMLIST_SET] - Unknown invType '" + pkt.invType + "'.");
	}
}

/**
 * We just know that server inventory type ended
 *
 * @param {object} pkt - PACKET.ZC.SPLIT_SEND_ITEMLIST_RESULT
 */
function onItemListResult(pkt) {
	switch (pkt.invType) {
		case 0: // Inventory
		case 1: // Cart
		case 2: // Storage
		case 3: // Guild Storage
			break;
		default:
			throw new Error("[PACKET.ZC.SPLIT_SEND_ITEMLIST_RESULT] - Unknown invType '" + pkt.invType + "'.");
	}
}

/**
 * Initialize
 */
export default function StorageEngine() {
	Network.hookPacket(PACKET.ZC.STORE_NORMAL_ITEMLIST, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_NORMAL_ITEMLIST2, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_NORMAL_ITEMLIST3, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_NORMAL_ITEMLIST4, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_EQUIPMENT_ITEMLIST, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_EQUIPMENT_ITEMLIST2, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_EQUIPMENT_ITEMLIST3, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_EQUIPMENT_ITEMLIST4, onStorageList);
	Network.hookPacket(PACKET.ZC.STORE_EQUIPMENT_ITEMLIST5, onStorageList);
	Network.hookPacket(PACKET.ZC.NOTIFY_STOREITEM_COUNTINFO, onStorageInfo);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_STORE, onStorageItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_STORE2, onStorageItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_STORE3, onStorageItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_STORE4, onStorageItemAdded);
	Network.hookPacket(PACKET.ZC.CLOSE_STORE, onStorageClose);
	Network.hookPacket(PACKET.ZC.DELETE_ITEM_FROM_STORE, onStorageItemRemoved);
	Network.hookPacket(PACKET.ZC.SPLIT_SEND_ITEMLIST_SET, onItemListSet);
	Network.hookPacket(PACKET.ZC.SPLIT_SEND_ITEMLIST_RESULT, onItemListResult);
}
