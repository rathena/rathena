/**
 * Engine/MapEngine/Trade.js
 *
 * Manage Trade packets and UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Trade from 'UI/Components/Trade/Trade.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import UIManager from 'UI/UIManager.js';

/**
 * Convert GID to a random string
 * It's used in the official client
 *
 * @param {number} GID
 * @param {string} randrom string
 */
function tradeGIDEncoding(GID) {
	const table = 'ROHUTNASEW';
	let out;
	let i, count;

	const str = String(GID);
	out = '';

	for (i = 0, count = str.length; i < count; ++i) {
		out += table[str[i]];
	}

	return out;
}

/**
 * Someone ask to start a trade
 * @param {object} pkt - PACKET.ZC.REQ_EXCHANGE_ITEM
 */
function onTradeRequest(pkt) {
	function answer(value) {
		return () => {
			const _pkt = new PACKET.CZ.ACK_EXCHANGE_ITEM();
			_pkt.result = value;
			Network.sendPacket(_pkt);
		};
	}

	let text = `(${pkt.name}) ${DB.getMessage(93)}`;
	Trade.title = pkt.name;

	if ('level' in pkt && 'GID' in pkt) {
		text += `\nPN: ${tradeGIDEncoding(pkt.GID)}\xa0\xa0\xa0\xa0\xa0Lv.${pkt.level}`;
	}

	UIManager.showPromptBox(text, 'ok', 'cancel', answer(3), answer(4));
}

/**
 * Result about trade ask
 * @param {object} pkt - PACKET.ZC.ACK_EXCHANGE_ITEM
 */
function onTradeRequestAnswer(pkt) {
	switch (pkt.result) {
		case 0: // Char is too far
			ChatBox.addText(DB.getMessage(70), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		case 1: // Character does not exist
			ChatBox.addText(DB.getMessage(71), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		case 2: // In another deal
			ChatBox.addText(DB.getMessage(72), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		case 3:
			if ('level' in pkt && 'GID' in pkt) {
				Trade.title += `  Lv${pkt.level} (${tradeGIDEncoding(pkt.GID)})`;
			}
			Trade.append();
			break;

		case 4: // Cancel
			ChatBox.addText(DB.getMessage(74), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		case 5: // AFK ?
			break;
	}
}

/**
 * Try to add an item to the list
 *
 * @param {number} item index in inventory
 * @param {number} count
 */
Trade.reqAddItem = function reqAddItem(index, count) {
	const pkt = new PACKET.CZ.ADD_EXCHANGE_ITEM();
	pkt.index = index;
	pkt.count = count;

	Network.sendPacket(pkt);
};

/**
 * Response from the server when requesting to add an item.
 *
 * @param {object} pkt - PACKET.ZC.ACK_ADD_EXCHANGE_ITEM
 */
function onAddItemResult(pkt) {
	switch (pkt.result) {
		case 1: // overweight
			ChatBox.addText(DB.getMessage(73), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		case 2: // trade canceled
			ChatBox.addText(DB.getMessage(74), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
	}

	Trade.addItemFromInventory(pkt.Index, pkt.result === 0);
}

/**
 * Oher user added another item
 *
 * @param {object} pkt - PACKET.ZC.ADD_EXCHANGE_ITEM
 */
function onItemAdded(pkt) {
	Trade.addItem(pkt);
}

/**
 * Reject deal
 */
Trade.onCancel = function onCancel() {
	const pkt = new PACKET.CZ.CANCEL_EXCHANGE_ITEM();
	Network.sendPacket(pkt);
};

/**
 * Deal canceled
 *
 * @param {object} pkt - PACKET.ZC.CANCEL_EXCHANGE_ITEM
 */
function onTradeCancel(pkt) {
	ChatBox.addText(DB.getMessage(74), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	Trade.remove();
}

/**
 * Conclude the deal
 */
Trade.onConclude = function onConclude() {
	const pkt = new PACKET.CZ.CONCLUDE_EXCHANGE_ITEM();
	Network.sendPacket(pkt);
};

/**
 * Conclude a part of the trade
 *
 * @param {object} pkt - PACKET.ZC.CONCLUDE_EXCHANGE_ITEM
 */
function onTradeConclude(pkt) {
	Trade.conclude(pkt.who ? 'recv' : 'send');
}

/**
 * Submit the trade
 */
Trade.onTradeSubmit = function onTradeSubmit() {
	const pkt = new PACKET.CZ.EXEC_EXCHANGE_ITEM();
	Network.sendPacket(pkt);
};

/**
 * Result of the deal
 *
 * @param {object} pkt - PACKET.ZC.EXEC_EXCHANGE_ITEM
 */
function onTradeSubmitAnswer(pkt) {
	// Fail
	if (pkt.result === 1) {
		ChatBox.addText(DB.getMessage(76), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		Trade.remove();
		return;
	}

	//TODO: Give items...
	ChatBox.addText(DB.getMessage(75), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
	Trade.remove();
}

/**
 * Request to start a deal with another player
 *
 * @param {number} GID
 */
Trade.reqExchange = function requestExhange(GID, name) {
	const pkt = new PACKET.CZ.REQ_EXCHANGE_ITEM();
	pkt.AID = GID;
	Network.sendPacket(pkt);

	Trade.title = name;
};

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.REQ_EXCHANGE_ITEM, onTradeRequest);
	Network.hookPacket(PACKET.ZC.REQ_EXCHANGE_ITEM2, onTradeRequest);
	Network.hookPacket(PACKET.ZC.ACK_EXCHANGE_ITEM, onTradeRequestAnswer);
	Network.hookPacket(PACKET.ZC.ACK_EXCHANGE_ITEM2, onTradeRequestAnswer);
	Network.hookPacket(PACKET.ZC.ACK_ADD_EXCHANGE_ITEM, onAddItemResult);
	Network.hookPacket(PACKET.ZC.ADD_EXCHANGE_ITEM, onItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_EXCHANGE_ITEM2, onItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_EXCHANGE_ITEM3, onItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_EXCHANGE_ITEM4, onItemAdded);
	Network.hookPacket(PACKET.ZC.ADD_EXCHANGE_ITEM5, onItemAdded);
	Network.hookPacket(PACKET.ZC.CANCEL_EXCHANGE_ITEM, onTradeCancel);
	Network.hookPacket(PACKET.ZC.CONCLUDE_EXCHANGE_ITEM, onTradeConclude);
	Network.hookPacket(PACKET.ZC.EXEC_EXCHANGE_ITEM, onTradeSubmitAnswer);
}
