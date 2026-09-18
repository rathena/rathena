/**
 * Engine/MapEngine/Store.js
 *
 * Manage npc store (buy/sell items)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import Session from 'Engine/SessionStorage.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import EntityManager from 'Renderer/EntityManager.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import NpcStore from 'UI/Components/NpcStore/NpcStore.js';
import Vending from 'UI/Components/Vending/Vending.js';
import VendingReport from 'UI/Components/VendingReport/VendingReport.js';
import VendingShop from 'UI/Components/VendingShop/VendingShop.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';

/**
 * Received items list to buy from cash npc
 *
 * @param {object} pkt - PACKET.ZC.ZC_PC_CASH_POINT_ITEMLIST
 */
function onBuyCashList(pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.CASH_SHOP);
	NpcStore.setList(pkt.itemList);

	const entity = Session.Entity;
	NpcStore.ui.find('.cashuser .buyer').text(entity ? entity.display.name : '');
	NpcStore.ui.find('.cashuser .cashpoints').text(pkt.KafraPoint);

	NpcStore.onSubmit = itemList => {
		// add prompt confirmation first later...
		const _pkt = new PACKET.CZ.PC_BUY_CASH_POINT_ITEM();
		const count = itemList.length;
		_pkt.kafrapts = 0;

		for (let i = 0; i < count; ++i) {
			_pkt.list.push({
				count: itemList[i].count,
				ITID: itemList[i].ITID,
				price: itemList[i].discountprice || itemList[i].price
			});
			//pkt.kafrapts += (itemList[i].discountprice || itemList[i].price) * itemList[i].count;
		}

		Network.sendPacket(_pkt);
	};
}

/**
 * Received items list to sell
 *
 * @param {object} pkt - PACKET.ZC.ZC_PC_CASH_POINT_ITEMLIST
 */
function onBuyVendingList(pkt) {
	VendingShop.append();
	VendingShop.setType(VendingShop.Type.VENDING_LIST);
	VendingShop.setItems(pkt.itemList);
}

/**
 * Received items list to buy
 *
 * @param {object} pkt - PACKET.ZC.ZC_PC_CASH_POINT_ITEMLIST
 */
function onBuyingList(pkt) {
	VendingShop.append();
	VendingShop.setType(VendingShop.Type.BUYING_LIST);
	VendingShop.setItems(pkt.itemList);
}

function onDeleteVendingItem(pkt) {
	// Vending Report
	if (PACKETVER.value >= 20141016) {
		VendingReport.add(pkt);
	}

	VendingShop.removeItem(pkt.index, pkt.count);
}

/**
 * Received items list to buy from npc
 *
 * @param {object} pkt - PACKET.ZC.PC_PURCHASE_ITEMLIST
 */
function onBuyList(pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.BUY);
	NpcStore.setList(pkt.itemList);
	NpcStore.onSubmit = itemList => {
		const _pkt = new PACKET.CZ.PC_PURCHASE_ITEMLIST();
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			_pkt.itemList.push({
				ITID: itemList[i].ITID,
				count: itemList[i].count
			});
		}

		Network.sendPacket(_pkt);
	};
}

/**
 * Received items list to from barter NPC
 *
 * @param {object} pkt - PACKET.ZC.NPC_BARTER_MARKET_ITEMINFO
 */
function onBarterBuyList(pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.BARTER_MARKET);
	NpcStore.setList(pkt.itemList);
	NpcStore.onSubmit = itemList => {
		const _pkt = new PACKET.CZ.NPC_BARTER_MARKET_PURCHASE();
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			const item = Inventory.getUI().getItemById(itemList[i].matcurrency);
			const item_index = item ? item.index : -1;
			_pkt.itemList.push({
				itemId: itemList[i].ITID,
				amount: itemList[i].count,
				invIndex: item_index,
				shopIndex: itemList[i].shopIndex
			});
		}

		Network.sendPacket(_pkt);
	};
}

/**
 * Received items list to from expanded barter NPC
 *
 * @param {object} pkt - PACKET.ZC.NPC_EXPANDED_BARTER_MARKET_ITEMINFO
 */
function onExpandedBarterBuyList(pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.BARTER_MARKET_EXTENDED);
	NpcStore.setList(pkt.itemList);
	NpcStore.onSubmit = itemList => {
		const _pkt = new PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_PURCHASE();
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			_pkt.itemList.push({
				itemId: itemList[i].ITID,
				shopIndex: itemList[i].index,
				amount: itemList[i].count
			});
		}

		Network.sendPacket(_pkt);
	};
}

/**
 * Received purchased informations
 *
 * @param {object} pkt - PACKET_ZC_PC_PURCHASE_RESULT
 */
function onBuyResult(pkt) {
	NpcStore.remove();

	switch (pkt.result) {
		case 0:
			ChatBox.addText(DB.getMessage(54), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break; // success
		case 1:
			ChatBox.addText(DB.getMessage(55), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // zeny
		case 2:
			ChatBox.addText(DB.getMessage(56), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // overweight
		case 4:
			ChatBox.addText(DB.getMessage(230), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // out of stock
		case 5:
			ChatBox.addText(DB.getMessage(281), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // trade
		// case 6: 6 = Because the store information was incorrect the item was not purchased.
		case 7:
			ChatBox.addText(DB.getMessage(1797), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // no sale information
		case 11:
			ChatBox.addText(DB.getMessage(3554), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // Exchange failed
		case 12:
			ChatBox.addText(DB.getMessage(3555), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break; // Exchange successfuly completed
		case 13:
			ChatBox.addText(DB.getMessage(3557), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // The item has been sold and out of stock
		case 14:
			ChatBox.addText(DB.getMessage(3556), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // Not enough items to exchange
		default:
			ChatBox.addText(DB.getMessage(57), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // deal failed
	}

	if (NpcStore.getCurrentType() >= 4 && NpcStore.getCurrentType() != NpcStore.Type.CASH_SHOP) {
		// Marketshop && Barter
		NpcStore.closeStore();
	}
}

/**
 * Received purchased informations
 *
 * @param {object} pkt - PACKET_ZC_PC_CASH_POINT_UPDATE
 */

function onBuyCashResult(pkt) {
	if (NpcStore.getCurrentType() >= 4 && NpcStore.getCurrentType() != NpcStore.Type.CASH_SHOP) {
		NpcStore.setClosePacketSent(true);
		// Marketshop && Barter
		NpcStore.remove();
		NpcStore.closeStore();
	}

	switch (pkt.Error) {
		case 0:
			ChatBox.addText(DB.getMessage(54), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break; // success
		case 1:
			ChatBox.addText(DB.getMessage(1227), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // zeny
		case 2:
			ChatBox.addText(DB.getMessage(1228), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // overweight
		case 4:
			ChatBox.addText(DB.getMessage(1229), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // out of stock
		case 5:
			ChatBox.addText(DB.getMessage(1230), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // trade
		case 6:
			ChatBox.addText(DB.getMessage(1254), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 7:
			ChatBox.addText(DB.getMessage(1813), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // no sale information
		default:
			ChatBox.addText(DB.getMessage(1814), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // deal failed
	}

	NpcStore.ui.find('.cashuser .cashpoints').text(pkt.KafraPoint);
}

/**
 * Received purchased informations
 *
 * @param {object} pkt - FAILED_TRADE_BUYING_STORE_TO_SELLER
 */

function onSellToBuyingStoreResult(pkt) {
	switch (pkt.Result) {
		case 6:
			ChatBox.addText(DB.getMessage(1742), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // The trade failed, because the entered amount of item %s is higher, than the buyer is willing to buy.
		case 7:
			ChatBox.addText(DB.getMessage(1740), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // The trade failed, because the buyer is lacking required balance.
		default:
			ChatBox.addText(DB.getMessage(57), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break; // deal failed
	}
}

/**
 * Received items list to buy from npc
 *
 * @param {object} pkt - PACKET.ZC.PC_SELL_ITEMLIST
 */
function onSellList(pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.SELL);
	NpcStore.setList(pkt.itemList);
	NpcStore.onSubmit = itemList => {
		const _pkt = new PACKET.CZ.PC_SELL_ITEMLIST();
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			_pkt.itemList.push({
				index: itemList[i].index,
				count: itemList[i].count
			});
		}

		Network.sendPacket(_pkt);
	};
}

/**
 * Receive sell list result
 *
 * @param {object} pkt - PACKET_ZC.PC.SELL_RESULT
 */
function onSellResult(pkt) {
	NpcStore.setClosePacketSent(true);
	NpcStore.remove();

	// success
	if (pkt.result === 0) {
		ChatBox.addText(DB.getMessage(54), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
	}

	// Fail
	else {
		ChatBox.addText(DB.getMessage(57), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Received items list to buy from player
 *
 * @param {object} pkt - PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC
 */
function onVendingStoreList(_pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.VENDING_STORE);
	NpcStore.setList(_pkt.itemList);

	// Get seller name
	const entity = EntityManager.get(_pkt.AID);
	NpcStore.ui.find('.seller').text(entity ? entity.display.name : '');

	// Bying items
	NpcStore.onSubmit = function (itemList) {
		NpcStore.setClosePacketSent(true);
		NpcStore.remove();

		let pkt;

		if (_pkt instanceof PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC3) {
			pkt = new PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC2();
			pkt.UniqueID = _pkt.UniqueID;
		} else if (_pkt instanceof PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC2) {
			pkt = new PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC2();
			pkt.UniqueID = _pkt.UniqueID;
		} else {
			pkt = new PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC();
		}

		pkt.AID = _pkt.AID;
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			pkt.itemList.push({
				index: itemList[i].index,
				count: itemList[i].count
			});
		}

		Network.sendPacket(pkt);
	};
}

/**
 * Received items list to buy from player
 *
 * @param {object} pkt - PACKET.ZC.ACK_ITEMLIST_BUYING_STORE
 */
function onBuyingStoreList(_pkt) {
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.BUYING_STORE);
	NpcStore.setList(_pkt.itemList);
	NpcStore.setPriceLimit(_pkt.limitZeny);

	// Get seller name
	const entity = EntityManager.get(_pkt.AID);
	NpcStore.ui.find('.seller').text(entity ? entity.display.name : '');

	// Bying items
	NpcStore.onSubmit = function (itemList) {
		NpcStore.setClosePacketSent(true);
		NpcStore.remove();

		const pkt = new PACKET.CZ.REQ_TRADE_BUYING_STORE();
		pkt.UniqueID = _pkt.UniqueID;
		pkt.AID = _pkt.AID;

		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			pkt.itemList.push({
				index: itemList[i].index,
				ITID: itemList[i].ITID,
				count: itemList[i].count
			});
		}

		Network.sendPacket(pkt);
	};
}

/**
 * Open vending creation window with X slots
 *
 * @param {object} pkt - PACKET.ZC.PACKET_ZC_OPENSTORE
 */
function onOpenVending(pkt) {
	if (Vending.isOpen) {
		return;
	}

	// Otherwise, open another merchant's shop
	Vending.setType(Vending.Type.VENDING_STORE);
	Vending.onVendingSkill(pkt);
}

/**
 * Open Buying creation window with X slots
 *
 * @param {object} pkt - PACKET.ZC.PACKET_ZC_OPENSTORE
 */
function onOpenBuying(pkt) {
	Vending.setType(Vending.Type.BUYING_STORE);
	Vending.onBuyingSkill(pkt);
}

/**
 * Open vending creation window with X slots
 *
 * @param {object} pkt - PACKET.ZC.ACK_OPENSTORE2
 */
function onOpenVendingResult(pkt) {
	// TODO: check what it do in client
}

/**
 * Open vending creation window with X slots
 *
 * @param {object} pkt - PACKET.ZC.ACK_OPENSTORE2
 */
function onOpenBuyingResult(pkt) {
	// client use same message for all errors, i just documented it here:
	switch (pkt.Result) {
		case 1:
			ChatBox.addText(DB.getMessage(1741), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG); // "Failed to open buying store." - invalid item/amount/price
			break;
		case 2:
			ChatBox.addText(DB.getMessage(1741), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG); // "Total amount of then possessed items exceeds the weight limit by %d. Please re-enter." - not able to carry all wanted items without getting overweight (90%)
			break;
		default:
			ChatBox.addText(DB.getMessage(1741), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Received items list to from Marketshop NPC
 *
 * @param {object} pkt - PACKET.ZC.NPC_MARKET_OPEN2
 */
function onMarketShop(pkt) {
	// Initialize the NPC store for Market Shop
	NpcStore.append();
	NpcStore.setType(NpcStore.Type.MARKETSHOP); // Set the type to MARKETSHOP
	NpcStore.setList(pkt.itemList); // Set the item list from the packet

	// Define the submission callback
	NpcStore.onSubmit = itemList => {
		const _pkt = new PACKET.CZ.NPC_MARKET_PURCHASE(); // Use the market purchase packet
		const count = itemList.length;

		for (let i = 0; i < count; ++i) {
			_pkt.itemList.push({
				itemId: itemList[i].ITID, // Item ID
				amount: itemList[i].count // Quantity to purchase
			});
		}

		// Send the constructed packet
		Network.sendPacket(_pkt);
	};
}

/**
 * Handles marketshop purchase result packet
 *
 * @param {PACKET.ZC.NPC_MARKET_PURCHASE_RESULT} pkt
 * @param {PACKET.ZC.NPC_MARKET_PURCHASE_RESULT2} pkt
 */
function onMarketShopResult(pkt) {
	if (pkt) {
		switch (pkt.result) {
			case 0: // PACKETVER.value >= 20190807 success
			case 1: // PACKETVER.value < 20190807 success
				ChatBox.addText(DB.getMessage(54), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
				NpcStore.onMarketShopResultUI(pkt.itemList);
				break;
			default:
				break;
		}
	}
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.PC_CASH_POINT_ITEMLIST, onBuyCashList);
	Network.hookPacket(PACKET.ZC.PC_CASH_POINT_UPDATE, onBuyCashResult);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST, onBuyList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST2, onBuyList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_RESULT, onBuyResult);
	Network.hookPacket(PACKET.ZC.PC_SELL_ITEMLIST, onSellList);
	Network.hookPacket(PACKET.ZC.PC_SELL_RESULT, onSellResult);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC, onVendingStoreList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC2, onVendingStoreList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC3, onVendingStoreList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_RESULT_FROMMC, onBuyResult);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_MYITEMLIST, onBuyVendingList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_MYITEMLIST2, onBuyVendingList);
	Network.hookPacket(PACKET.ZC.DELETEITEM_FROM_MCSTORE, onDeleteVendingItem);
	Network.hookPacket(PACKET.ZC.DELETEITEM_FROM_MCSTORE2, onDeleteVendingItem);
	Network.hookPacket(PACKET.ZC.OPENSTORE, onOpenVending);
	Network.hookPacket(PACKET.ZC.ACK_OPENSTORE2, onOpenVendingResult);
	Network.hookPacket(PACKET.ZC.OPEN_BUYING_STORE, onOpenBuying);
	Network.hookPacket(PACKET.ZC.FAILED_OPEN_BUYING_STORE_TO_BUYER, onOpenBuyingResult);
	Network.hookPacket(PACKET.ZC.MYITEMLIST_BUYING_STORE, onBuyingList);
	Network.hookPacket(PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC3, onVendingStoreList);
	Network.hookPacket(PACKET.ZC.ACK_ITEMLIST_BUYING_STORE, onBuyingStoreList);
	Network.hookPacket(PACKET.ZC.FAILED_TRADE_BUYING_STORE_TO_SELLER, onSellToBuyingStoreResult);
	Network.hookPacket(PACKET.ZC.NPC_MARKET_OPEN2, onMarketShop);
	Network.hookPacket(PACKET.ZC.NPC_MARKET_PURCHASE_RESULT, onMarketShopResult);
	Network.hookPacket(PACKET.ZC.NPC_MARKET_PURCHASE_RESULT2, onMarketShopResult);
	Network.hookPacket(PACKET.ZC.NPC_BARTER_MARKET_ITEMINFO, onBarterBuyList);
	Network.hookPacket(PACKET.ZC.NPC_EXPANDED_BARTER_MARKET_ITEMINFO, onExpandedBarterBuyList);
}
