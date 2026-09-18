/**
 * Engine/MapEngine/Item.js
 *
 * Item dropped to the ground
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import ItemObject from 'Renderer/ItemObject.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Session from 'Engine/SessionStorage.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ItemObtain from 'UI/Components/ItemObtain/ItemObtain.js';
import ItemSelection from 'UI/Components/ItemSelection/ItemSelection.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import PlayerViewEquip from 'UI/Components/PlayerViewEquip/PlayerViewEquip.js';
import SwitchEquip from 'UI/Components/SwitchEquip/SwitchEquip.js';
import Storage from 'UI/Components/Storage/Storage.js';
import MakeItemSelection from 'UI/Components/MakeItemSelection/MakeItemSelection.js';
import ItemListWindowSelection from 'UI/Components/MakeItemSelection/ItemListWindowSelection.js';
import EffectManager from 'Renderer/EffectManager.js';

/**
 * Spam an item on the map
 *
 * @param {object} pkt - PACKET.ZC.ITEM_ENTRY
 */
function onItemExistInGround(pkt) {
	const x = pkt.xPos - 0.5 + pkt.subX / 12;
	const y = pkt.yPos - 0.5 + pkt.subY / 12;
	const z = Altitude.getCellHeight(x, y);

	ItemObject.add(pkt.ITAID, pkt.ITID, pkt.IsIdentified, pkt.count, x, y, z);
}

/**
 * Spam a new item on the map
 *
 * @param {object} pkt - PACKET.ZC.ITEM_FALL_ENTRY
 */
function onItemSpamInGround(pkt) {
	const x = pkt.xPos - 0.5 + pkt.subX / 12;
	const y = pkt.yPos - 0.5 + pkt.subY / 12;
	const z = Altitude.getCellHeight(x, y) + 5.0;

	ItemObject.add(pkt.ITAID, pkt.ITID, pkt.IsIdentified, pkt.count, x, y, z, pkt.dropeffectmode, pkt.showdropeffect);
}

/**
 * Spam a new item on the map
 *
 * @param {object} pkt - PACKET.ZC.ITEM_DISAPPEAR
 */
function onItemInGroundVanish(pkt) {
	ItemObject.remove(pkt.ITAID);
}

/**
 * Answer when player pick the item
 *
 * @param {object} pkt - PACKET.ZC.ITEM_PICKUP_ACK3
 */
function onItemPickAnswer(pkt) {
	// Fail
	if (pkt.result !== 0) {
		ChatBox.addText(DB.getMessage(53), ChatBox.TYPE.ERROR, ChatBox.FILTER.ITEM);
		return;
	}

	ItemObtain.append();
	ItemObtain.set(pkt);

	const getTextItem = DB.getItemName(pkt, { showItemOptions: false });

	ChatBox.addText(
		DB.getMessage(153).replace('%s', getTextItem).replace('%d', pkt.count),
		ChatBox.TYPE.BLUE,
		ChatBox.FILTER.ITEM
	);

	Inventory.getUI().addItem(pkt);
}

/**
 * Generic function to add items to inventory
 *
 * @param {object} pkt - PACKET.ZC.EQUIPMENT_ITEMLIST
 */
function onInventorySetList(pkt) {
	Inventory.getUI().setItems(pkt.itemInfo || pkt.ItemInfo);
}

/**
 * Remove item from inventory
 *
 * @param {object} pkt - PACKET.ZC.ITEM_THROW_ACK
 */
function onIventoryRemoveItem(pkt) {
	Inventory.getUI().removeItem(pkt.Index, pkt.count || pkt.Count || 0);
}

/**
 * Remove an item from equipment, add it to inventory
 *
 * @param {object} pkt - PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK
 */
function onEquipementTakeOff(pkt) {
	if (pkt.result) {
		const item = Equipment.getUI().unEquip(pkt.index, pkt.wearLocation);

		if (item) {
			item.WearState = 0;

			const it = DB.getItemInfo(item.ITID);
			ChatBox.addText(
				it.identifiedDisplayName + ' ' + DB.getMessage(171),
				ChatBox.TYPE.ERROR,
				ChatBox.FILTER.ITEM
			);

			if (!(pkt.wearLocation & EquipLocation.AMMO)) {
				Inventory.getUI().addItem(item);
			}
		}

		if (pkt.wearLocation & EquipLocation.HEAD_TOP) {
			Session.Entity.accessory2 = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_TOP);
		}
		if (pkt.wearLocation & EquipLocation.HEAD_MID) {
			Session.Entity.accessory3 = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_MID);
		}
		if (pkt.wearLocation & EquipLocation.HEAD_BOTTOM) {
			Session.Entity.accessory = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_BOTTOM);
		}
		if (pkt.wearLocation & EquipLocation.GARMENT) {
			Session.Entity.robe = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_ROBE);
		}
		if (pkt.wearLocation & EquipLocation.WEAPON) {
			Session.Entity.weapon = 0;
		}
		if (pkt.wearLocation & EquipLocation.SHIELD) {
			Session.Entity.shield = 0;
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_TOP) {
			Session.Entity.accessory2 = Equipment.getUI().checkEquipLoc(EquipLocation.HEAD_TOP);
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_MID) {
			Session.Entity.accessory3 = Equipment.getUI().checkEquipLoc(EquipLocation.HEAD_MID);
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_BOTTOM) {
			Session.Entity.accessory = Equipment.getUI().checkEquipLoc(EquipLocation.HEAD_BOTTOM);
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_ROBE) {
			Session.Entity.robe = Equipment.getUI().checkEquipLoc(EquipLocation.GARMENT);
		}

		if (PACKETVER.value >= 20170208) {
			// Remove from Switch Window as well
			if (!Inventory.getUI().isInEquipSwitchList(pkt.wearLocation)) {
				SwitchEquip.unEquip(pkt.index, pkt.wearLocation);
			}
		}
	}
}

/**
 * Equip an item
 *
 * @param {object} pkt - PACKET.ZC.REQ_WEAR_EQUIP_ACK
 */
function onItemEquip(pkt) {
	if (pkt.result == 1) {
		const item = Inventory.getUI().removeItem(pkt.index, 1);
		Equipment.getUI().equip(item, pkt.wearLocation);
		ChatBox.addText(DB.getItemName(item) + ' ' + DB.getMessage(170), ChatBox.TYPE.BLUE, ChatBox.FILTER.ITEM);

		// Variables for Headgear Checks
		const CostumeCheckTop = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_TOP);
		const CostumeCheckMid = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_MID);
		const CostumeCheckBot = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_HEAD_BOTTOM);
		const CostumeCheckRobe = Equipment.getUI().checkEquipLoc(EquipLocation.COSTUME_ROBE);

		// Display
		if (pkt.wearLocation & EquipLocation.HEAD_TOP) {
			Session.Entity.accessory2 = CostumeCheckTop ? CostumeCheckTop : pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.HEAD_MID) {
			Session.Entity.accessory3 = CostumeCheckMid ? CostumeCheckMid : pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.HEAD_BOTTOM) {
			Session.Entity.accessory = CostumeCheckBot ? CostumeCheckBot : pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.GARMENT) {
			Session.Entity.robe = CostumeCheckRobe ? CostumeCheckRobe : pkt.viewid;
		}

		if (pkt.wearLocation & EquipLocation.WEAPON) {
			Session.Entity.weapon = pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.SHIELD) {
			Session.Entity.shield = pkt.viewid;
		}

		// costume override regular equips
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_TOP) {
			Session.Entity.accessory2 = pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_MID) {
			Session.Entity.accessory3 = pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_HEAD_BOTTOM) {
			Session.Entity.accessory = pkt.viewid;
		}
		if (pkt.wearLocation & EquipLocation.COSTUME_ROBE) {
			Session.Entity.robe = pkt.viewid;
		}
	}

	// Fail to equip
	else {
		ChatBox.addText(DB.getMessage(372), ChatBox.TYPE.ERROR, ChatBox.FILTER.ITEM);
	}
}

/**
 * Answer from the server to use an item
 * @param {object} pkt - PACKET.ZC.USE_ITEM_ACK
 */
function onItemUseAnswer(pkt) {
	if (!pkt.hasOwnProperty('AID') || Session.Entity.GID === pkt.AID) {
		if (pkt.result) {
			Inventory.getUI().updateItem(pkt.index, pkt.count);
		} else {
			// should we show a msg in chatbox ?
		}
	}
	if (pkt.result) {
		EffectManager.spamItem(pkt.id, pkt.AID, null, null, null);
	}
}

/**
 * View other player's equipment - CZ_EQUIPWIN_MICROSCOPE
 *
 * @param {number} account id
 */
Equipment.onCheckPlayerEquipment = function onCheckPlayerEquipment(AID) {
	const pkt = new PACKET.CZ.EQUIPWIN_MICROSCOPE();
	pkt.AID = AID;
	Network.sendPacket(pkt);
};

/**
 * Show other player's equipment
 *
 * @param {object} pkt - ZC_EQUIPWIN_MICROSCOPE
 */
function onShowPlayerEquip(pkt) {
	PlayerViewEquip.getUI().append();
	PlayerViewEquip.getUI().setTitleBar(pkt.characterName);
	PlayerViewEquip.getUI().setEquipmentData(pkt.ItemInfo);
	PlayerViewEquip.getUI().setChar2Render(pkt);
}

/**
 * Equip an arrow
 *
 * @param {object} pkt - PACKET_ZC_EQUIP_ARROW
 */
function onArrowEquipped(pkt) {
	const item = Inventory.getUI().getItemByIndex(pkt.index);
	Equipment.getUI().equip(item, EquipLocation.AMMO);
}

/**
 * @var {object} item
 */
let _cardComposition;

/**
 * Ask to get a list of equipments where we can put a card
 *
 * @param {number} card index
 */
function onUseCard(index) {
	_cardComposition = index;
	const pkt = new PACKET.CZ.REQ_ITEMCOMPOSITION_LIST();
	pkt.cardIndex = index;
	Network.sendPacket(pkt);
}

/**
 * Get a list of equipments to insert card
 *
 * @param {object} pkt - PACKET.ZC.ITEMCOMPOSITION_LIST
 */
function onItemCompositionList(pkt) {
	if (!pkt.ITIDList.length) {
		return;
	}

	const card = Inventory.getUI().getItemByIndex(_cardComposition);

	ItemSelection.append();
	ItemSelection.setList(pkt.ITIDList);
	ItemSelection.setTitle(DB.getMessage(522) + '(' + DB.getItemInfo(card.ITID).identifiedDisplayName + ')');
	ItemSelection.onIndexSelected = function (index) {
		if (index >= 0) {
			const _pkt = new PACKET.CZ.REQ_ITEMCOMPOSITION();
			_pkt.cardIndex = _cardComposition;
			_pkt.equipIndex = index;
			Network.sendPacket(_pkt);
		}

		_cardComposition = null;
	};
}

/**
 * Get the result once card inserted
 *
 * @param {object} pkt - PACKET.ZC.ACK_ITEMCOMPOSITION
 */
function onItemCompositionResult(pkt) {
	switch (pkt.result) {
		case 0: {
			// success
			const item = Inventory.getUI().removeItem(pkt.equipIndex, 1);
			const card = Inventory.getUI().removeItem(pkt.cardIndex, 1);

			if (item) {
				for (let i = 0; i < 4; ++i) {
					if (!item.slot['card' + (i + 1)]) {
						item.slot['card' + (i + 1)] = card.ITID;
						break;
					}
				}
				Inventory.getUI().addItem(item);
			}
			break;
		}
		case 1: // Fail
			break;
	}
}

/**
 * Get the result of a refine action
 *
 * @param {object} pkt - PACKET.ZC.ACK_ITEMREFINING
 */
function onRefineResult(pkt) {
	// Check if refine UI is enabled and packet version is >= 20161012
	if (Configs.get('enableRefineUI') && PACKETVER.value >= 20161012) {
		import('UI/Components/Refine/Refine.js').then(m => m.default.onRefineResult(pkt));
	} else {
		const item = Inventory.getUI().removeItem(pkt.itemIndex, 1);
		if (item) {
			item.RefiningLevel = pkt.RefiningLevel;
			Inventory.getUI().addItem(item);
		}

		switch (pkt.result) {
			case 0: // success
				ChatBox.addText(
					DB.getMessage(498), // Upgrade success!!
					ChatBox.TYPE.BLUE,
					ChatBox.FILTER.PUBLIC_LOG
				);
				break;
			case 1: // failure
				ChatBox.addText(
					DB.getMessage(499), // Upgrade failed!!
					ChatBox.TYPE.BLUE,
					ChatBox.FILTER.PUBLIC_LOG
				);
				break;
			case 2: // downgrade
				ChatBox.addText(
					DB.getMessage(1537), // Is now refining the value lowered
					ChatBox.TYPE.BLUE,
					ChatBox.FILTER.PUBLIC_LOG
				);
				break;
		}
	}
}

/**
 * Generic function to add items to cart
 *
 * @param {object} pkt - PACKET.ZC.CART_EQUIPMENT_ITEMLIST3
 */
function onCartSetList(pkt) {
	CartItems.setItems(pkt.itemInfo || pkt.ItemInfo);
}

/**
 * Generic function to set cart info
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_CARTITEM_COUNTINFO
 */
function onCartSetInfo(pkt) {
	CartItems.setCartInfo(pkt.curCount, pkt.maxCount, pkt.curWeight, pkt.maxWeight);
}

function onCartRemoveItem(pkt) {
	CartItems.removeItem(pkt.index, pkt.count);
}

CartItems.reqRemoveItem = function ReqRemoveItem(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.MOVE_ITEM_FROM_CART_TO_BODY();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

function reqMoveItemToCart(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_CART();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
}

Inventory.reqMoveItemToWriteRodex = function reqMoveItemToWriteRodex(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.REQ_ADD_ITEM_RODEX();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

function onCartItemAdded(pkt) {
	CartItems.addItem(pkt);
}

function onAckAddItemToCart(pkt) {
	switch (pkt.result) {
		case 0:
			ChatBox.addText(DB.getMessage(220), ChatBox.TYPE.ERROR, ChatBox.FILTER.ITEM);
			break;

		case 1:
			ChatBox.addText(DB.getMessage(221), ChatBox.TYPE.ERROR, ChatBox.FILTER.ITEM);
			break;
	}
}

/**
 * Get a list of items to create
 *
 * @param {object} pkt - PACKET.ZC.MAKABLEITEMLIST
 */
function onMakeitemList(pkt) {
	if (!pkt.itemList.length) {
		return;
	}

	MakeItemSelection.append();
	MakeItemSelection.setList(pkt.itemList);
	MakeItemSelection.setTitle(DB.getMessage(425));
	MakeItemSelection.onIndexSelected = function (index, material) {
		if (index >= -1) {
			const _pkt = new PACKET.CZ.REQMAKINGITEM();
			_pkt.itemList.ITID = index;
			_pkt.itemList.material_ID = {};
			_pkt.itemList.material_ID[0] = material[0] && material[0].ITID ? material[0].ITID : 0;
			_pkt.itemList.material_ID[1] = material[1] && material[1].ITID ? material[1].ITID : 0;
			_pkt.itemList.material_ID[2] = material[2] && material[2].ITID ? material[2].ITID : 0;
			Network.sendPacket(_pkt);
		}
	};
}

/**
 * Get a list of items to create
 *
 * @param {object} pkt - PACKET.ZC.ITEMLISTWIN_OPEN
 */
function onListWinItem(ptk) {
	if (!ptk.Type) {
		ItemListWindowSelection.append();
	}
}

/**
 * item lis twin
 * @param {object} inforMaterialList
 */
ItemListWindowSelection.onItemListWindowSelected = function onItemListWindowSelected(inforMaterialList) {
	let pkt;
	if (PACKETVER.value >= 20180307) {
		pkt = new PACKET.CZ.ITEMLISTWIN_RES2();
	} else {
		pkt = new PACKET.CZ.ITEMLISTWIN_RES();
	}

	pkt.Type = inforMaterialList.Type;
	pkt.Action = inforMaterialList.Action;
	pkt.MaterialList = inforMaterialList.MaterialList;

	Network.sendPacket(pkt);
};

/**
 * Get a list of items to create
 *
 * @param {object} pkt - PACKET.ZC.MAKINGITEM_LIST
 */
function onMakeitem_List(pkt) {
	let itemList;
	let makeType;
	if (PACKETVER.value >= 20211103) {
		if (!pkt.items.length) {
			return;
		}
		itemList = pkt.items.map(item => item.itemId);
		makeType = pkt.makeItem;
	} else {
		if (!pkt.idList.length) {
			return;
		}
		itemList = pkt.idList;
		makeType = itemList[0]; // First item is mktype for older versions
		itemList = itemList.slice(1); // Remove mktype from item list
	}

	MakeItemSelection.append();
	MakeItemSelection.setCookingList(itemList, makeType);
	MakeItemSelection.setTitle(DB.getMessage(425));
	MakeItemSelection.onIndexSelected = function (index, material, mkType) {
		if (index >= -1) {
			const _pkt = new PACKET.CZ.REQ_MAKINGITEM();
			_pkt.mkType = mkType;
			_pkt.id = index;
			Network.sendPacket(_pkt);
		}
	};
}

/**
 * Result of Inventory Expansion
 *
 * @param {object} pkt - PACKET.ZC.EXTEND_BODYITEM_SIZE
 */
function onBodyItemSize(pkt) {
	if (pkt) {
		const baselimit = 100; // Base Limit
		const newlimit = baselimit + pkt.type;
		Inventory.getUI().ui.find('.mcnt').text(newlimit);
	}
}

/**
 * Result of Inventory Expansion
 *
 * @param {object} pkt - PACKET.ZC.RECOVER_PENALTY_OVERWEIGHT
 */
function onRecoverPenaltyOverweight(pkt) {
	// TODO add it as status check
	// show percent to status, still a wip
	if (Session.Entity) {
		Session.Entity.overWeightPercent = pkt.percentage;
	}
}

/**
 * Result of Inventory Expansion
 *
 * @param {object} pkt - PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL
 */
function onItemListNormal(pkt) {
	switch (pkt.invType) {
		case 0:
			Inventory.getUI().setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		case 1:
			CartItems.setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		case 2:
			Storage.getUI().append();
			Storage.getUI().setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		default:
			throw new Error("[PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL] - Unknown invType '" + pkt.invType + "'.");
	}
}

/**
 * Result of Inventory Expansion
 *
 * @param {object} pkt - PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP
 */
function onItemListEquip(pkt) {
	switch (pkt.invType) {
		case 0:
			Inventory.getUI().setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		case 1:
			CartItems.setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		case 2:
			Storage.getUI().setItems(pkt.itemInfo || pkt.ItemInfo);
			break;
		default:
			throw new Error("[PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL] - Unknown invType '" + pkt.invType + "'.");
	}
}

/**
 * Updates the favorite status of an item in the Inventory UI
 *
 * @param {object} pkt - PACKET.ZC.ITEM_FAVORITE
 * pkt.favorite:
 *  0 = move item to personal tab
 * 	1 = move item to normal tab
 */
function onFavItemList(pkt) {
	if (pkt) {
		// So if favorite is 0, we send 1 to change item.PlaceETCTab to 1
		const isfavitem = pkt.favorite ? 0 : 1;
		Inventory.getUI().updatePlaceETCTab(pkt.index, isfavitem);
	}
}

/**
 * Received Switch Equip List
 */
function onSwitchEquipList(pkt) {
	if (pkt && pkt.ItemInfo) {
		pkt.ItemInfo.forEach(function (item) {
			if (Inventory.getUI().getItemByIndex(item.index)) {
				Inventory.getUI().addItemtoSwitch(item.index);
			}
		});
	}
}

/**
 * Add item to Switch Equip
 */
function onSwitchEquipAdd(pkt) {
	if (pkt) {
		switch (pkt.flag) {
			case 0:
				Inventory.getUI().addItemtoSwitch(pkt.index);
				break;
			case 1:
			case 2:
				break;
			default:
				throw new Error('[PACKET.ZC.REQ_WEAR_SWITCHEQUIP_ADD_RESULT] - Error!');
		}
	}
}

/**
 * Remove item to Switch Equip
 */
function onSwitchEquipRemove(pkt) {
	if (pkt) {
		switch (pkt.flag) {
			case 0:
				Inventory.getUI().removeItemFromSwitch(pkt.index);
				break;
			case 1:
				break;
			case 2:
				break;
			default:
				throw new Error('[PACKET.ZC.REQ_WEAR_SWITCHEQUIP_ADD_RESULT] - Error!');
		}
	}
}

/**
 * Initialize
 */
export default function ItemEngine() {
	Network.hookPacket(PACKET.ZC.ITEM_ENTRY, onItemExistInGround);
	Network.hookPacket(PACKET.ZC.ITEM_FALL_ENTRY, onItemSpamInGround);
	Network.hookPacket(PACKET.ZC.ITEM_FALL_ENTRY2, onItemSpamInGround);
	Network.hookPacket(PACKET.ZC.ITEM_FALL_ENTRY3, onItemSpamInGround);
	Network.hookPacket(PACKET.ZC.ITEM_DISAPPEAR, onItemInGroundVanish);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK2, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK3, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK5, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK6, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK7, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_PICKUP_ACK8, onItemPickAnswer);
	Network.hookPacket(PACKET.ZC.ITEM_THROW_ACK, onIventoryRemoveItem);
	Network.hookPacket(PACKET.ZC.NORMAL_ITEMLIST, onInventorySetList);
	Network.hookPacket(PACKET.ZC.NORMAL_ITEMLIST2, onInventorySetList);
	Network.hookPacket(PACKET.ZC.NORMAL_ITEMLIST3, onInventorySetList);
	Network.hookPacket(PACKET.ZC.NORMAL_ITEMLIST4, onInventorySetList);
	Network.hookPacket(PACKET.ZC.CART_NORMAL_ITEMLIST3, onCartSetList);
	Network.hookPacket(PACKET.ZC.CART_NORMAL_ITEMLIST4, onCartSetList);
	Network.hookPacket(PACKET.ZC.CART_EQUIPMENT_ITEMLIST3, onCartSetList);
	Network.hookPacket(PACKET.ZC.CART_EQUIPMENT_ITEMLIST4, onCartSetList);
	Network.hookPacket(PACKET.ZC.CART_EQUIPMENT_ITEMLIST5, onCartSetList);
	Network.hookPacket(PACKET.ZC.NOTIFY_CARTITEM_COUNTINFO, onCartSetInfo);
	Network.hookPacket(PACKET.ZC.EQUIPMENT_ITEMLIST, onInventorySetList);
	Network.hookPacket(PACKET.ZC.EQUIPMENT_ITEMLIST2, onInventorySetList);
	Network.hookPacket(PACKET.ZC.EQUIPMENT_ITEMLIST3, onInventorySetList);
	Network.hookPacket(PACKET.ZC.EQUIPMENT_ITEMLIST4, onInventorySetList);
	Network.hookPacket(PACKET.ZC.EQUIPMENT_ITEMLIST5, onInventorySetList);
	Network.hookPacket(PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK, onEquipementTakeOff);
	Network.hookPacket(PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK2, onEquipementTakeOff);
	Network.hookPacket(PACKET.ZC.ACK_TAKEOFF_EQUIP_V5, onEquipementTakeOff);
	Network.hookPacket(PACKET.ZC.REQ_WEAR_EQUIP_ACK, onItemEquip);
	Network.hookPacket(PACKET.ZC.REQ_WEAR_EQUIP_ACK2, onItemEquip);
	Network.hookPacket(PACKET.ZC.ACK_WEAR_EQUIP_V5, onItemEquip);
	Network.hookPacket(PACKET.ZC.DELETE_ITEM_FROM_BODY, onIventoryRemoveItem);
	Network.hookPacket(PACKET.ZC.DELETE_ITEM_FROM_CART, onCartRemoveItem);
	Network.hookPacket(PACKET.ZC.USE_ITEM_ACK, onItemUseAnswer);
	Network.hookPacket(PACKET.ZC.USE_ITEM_ACK2, onItemUseAnswer);
	Network.hookPacket(PACKET.ZC.EQUIP_ARROW, onArrowEquipped);
	Network.hookPacket(PACKET.ZC.ITEMCOMPOSITION_LIST, onItemCompositionList);
	Network.hookPacket(PACKET.ZC.ACK_ITEMCOMPOSITION, onItemCompositionResult);
	Network.hookPacket(PACKET.ZC.ACK_ITEMREFINING, onRefineResult);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_CART, onCartItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_CART2, onCartItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_CART3, onCartItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_ITEM_TO_CART4, onCartItemAdded);
	Network.hookPacket(PACKET.ZC.MAKABLEITEMLIST, onMakeitemList);
	Network.hookPacket(PACKET.ZC.MAKINGITEM_LIST, onMakeitem_List);
	Network.hookPacket(PACKET.ZC.ACK_ADDITEM_TO_CART, onAckAddItemToCart);
	Network.hookPacket(PACKET.ZC.ITEMLISTWIN_OPEN, onListWinItem);
	Network.hookPacket(PACKET.ZC.EXTEND_BODYITEM_SIZE, onBodyItemSize);
	Network.hookPacket(PACKET.ZC.RECOVER_PENALTY_OVERWEIGHT, onRecoverPenaltyOverweight);
	Network.hookPacket(PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL, onItemListNormal);
	Network.hookPacket(PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP, onItemListEquip);
	Network.hookPacket(PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP2, onItemListEquip);

	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V2, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V3, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V4, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V5, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V6, onShowPlayerEquip);
	Network.hookPacket(PACKET.ZC.EQUIPWIN_MICROSCOPE_V7, onShowPlayerEquip);

	/* Favorite Tab */
	Network.hookPacket(PACKET.ZC.ITEM_FAVORITE, onFavItemList);

	/* Switch Equipment*/
	Network.hookPacket(PACKET.ZC.SEND_SWAP_EQUIPITEM_INFO, onSwitchEquipList);
	Network.hookPacket(PACKET.ZC.REQ_WEAR_SWITCHEQUIP_ADD_RESULT, onSwitchEquipAdd);
	Network.hookPacket(PACKET.ZC.REQ_WEAR_SWITCHEQUIP_REMOVE_RESULT, onSwitchEquipRemove);

	Inventory.getUI().onUseCard = onUseCard;
	Inventory.getUI().reqMoveItemToCart = reqMoveItemToCart;
}
