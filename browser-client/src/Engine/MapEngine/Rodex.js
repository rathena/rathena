/**
 * Engine/MapEngine/RodEx.js
 *
 * Manage RodEx Mails
 *
 * @author Alisonrag
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import RodexIcon from 'UI/Components/Rodex/RodexIcon.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import ReadRodex from 'UI/Components/Rodex/ReadRodex.js';
import WriteRodex from 'UI/Components/Rodex/WriteRodex.js';

/**
 * Request to Open RodEx Box
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.openRodexBox = function openRodexBox(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.OPEN_RODEXBOX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Close RodEx Box
 *
 */
Rodex.closeRodexBox = function closeRodexBox() {
	const pkt = new PACKET.CZ.CLOSE_RODEXBOX();
	Network.sendPacket(pkt);
};

/**
 * Request to Read RodEx
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestReadRodex = function requestReadRodex(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_READ_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to get next RodEx Page
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestNextRodexPage = function requestNextRodexPage(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_NEXT_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Refresh current RodEx Page
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestRefreshRodexPage = function requestRefreshRodexPage(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_REFRESH_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Get Zeny from RodEx
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestZenyFromRodex = function requestZenyFromRodex(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_ZENY_FROM_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Get Item from RodEx
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestItemsFromRodex = function requestItemsFromRodex(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_ITEM_FROM_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Delete RodEx
 *
 * @param {int} openType
 * @param {int} MailID
 */
Rodex.requestDeleteRodex = function requestDeleteRodex(openType = 0, MailID = 0) {
	const pkt = new PACKET.CZ.REQ_DELETE_RODEX();
	pkt.openType = openType;
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Request to Cancel Write RodEx
 *
 */
WriteRodex.requestCancelWriteRodex = function requestCancelWriteRodex() {
	const pkt = new PACKET.CZ.REQ_CANCEL_WRITE_RODEX();
	Network.sendPacket(pkt);
};

/**
 * Request to Add Item in RodEx
 *
 * @param {int} index
 * @param {int} count
 */
WriteRodex.requestAddItemRodex = function requestAddItemRodex(index, count) {
	const pkt = new PACKET.CZ.REQ_ADD_ITEM_RODEX();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

/**
 * Request to Remove Item in RodEx
 *
 * @param {int} index
 * @param {int} count
 */
WriteRodex.requestRemoveItemRodex = function requestRemoveItemRodex(index, count) {
	if (count <= 0) {
		return;
	}
	const pkt = new PACKET.CZ.REQ_REMOVE_RODEX_ITEM();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

/**
 * Request to Open Write RodEx Window
 *
 * @param {string} name
 */
Rodex.requestOpenWriteRodex = function requestOpenWriteRodex(name = '') {
	const pkt = new PACKET.CZ.REQ_OPEN_WRITE_RODEX();
	pkt.name = name;
	Network.sendPacket(pkt);
};

/**
 * Request to Get all Items and Zenys from RodEx Box
 *
 * @param {int} MailID
 * @param {int} MailReturnID
 * @param {int} MailAccountID
 */
Rodex.openAllRodex = function openAllRodex(MailID = 0, MailReturnID = 0, MailAccountID = 0) {
	const pkt = new PACKET.CZ.OPEN_ALL_RODEX();
	pkt.MailID = MailID;
	pkt.MailReturnID = MailReturnID;
	pkt.MailAccountID = MailAccountID;
	Network.sendPacket(pkt);
};

/**
 * Request to refresh RodEx Box
 *
 * @param {int} MailID
 * @param {int} MailReturnID
 * @param {int} MailAccountID
 */
Rodex.updateAllRodex = function updateAllRodex(MailID = 0, MailReturnID = 0, MailAccountID = 0) {
	const pkt = new PACKET.CZ.UPDATE_ALL_RODEX();
	pkt.MailID = MailID;
	pkt.MailReturnID = MailReturnID;
	pkt.MailAccountID = MailAccountID;
	Network.sendPacket(pkt);
};

/**
 * Request to Send Rodex
 *
 * @param {string} receiver
 * @param {string} sender
 * @param {int} zeny
 * @param {int} Titlelength
 * @param {int} Bodylength
 * @param {int} CharID
 * @param {int} title
 * @param {string} body
 */
WriteRodex.requestSendRodex = function requestSendRodex(
	receiver,
	sender,
	zeny,
	Titlelength,
	Bodylength,
	CharID,
	title,
	body
) {
	let pkt;
	if (PACKETVER.value >= 20160330) {
		pkt = new PACKET.CZ.REQ_SEND_RODEX2();
	} else {
		pkt = new PACKET.CZ.REQ_SEND_RODEX();
	}
	pkt.receiver = receiver;
	pkt.sender = sender;
	pkt.zeny = zeny;
	pkt.Titlelength = Titlelength;
	pkt.Bodylength = Bodylength;
	pkt.CharID = CharID;
	pkt.title = title;
	pkt.body = body;
	Network.sendPacket(pkt);
};

WriteRodex.validateName = function (name) {
	let pkt;
	if (PACKETVER.value >= 20201104) {
		pkt = new PACKET.CZ.CHECK_RODEX_RECEIVE();
	} else {
		pkt = new PACKET.CZ.CHECK_RECEIVE_CHARACTER_NAME();
	}
	pkt.name = name;
	Network.sendPacket(pkt);
};

/**
 * Receive Packets
 */

/**
 * Server sent request about Rodex Icon
 *
 * @param {object} pkt - PACKET.ZC.RODEX_ICON
 */
function rodexIcon(pkt) {
	if (pkt.show) {
		RodexIcon.append();
	} else {
		RodexIcon.remove();
	}
}

/**
 * Server sent Rodex Mail List
 *
 * @param {object} pkt - PACKET.ZC.ACK_RODEX_LIST
 */
function rodexList(pkt) {
	Rodex.initData(pkt);
}

/**
 * Server failed on retrieve Rodex Mail List
 *
 * @param {object} pkt - PACKET.ZC.ACK_FAILED_ALL_RODEX_LIST
 */
function rodexGetListFailed(pkt) {
	Rodex.getListFailed(pkt);
}

/**
 * Server sent information about Rodex Mail to Read
 *
 * @param {object} pkt - PACKET.ZC.ACK_READ_RODEX
 */
function rodexRead(pkt) {
	const mail = Rodex.getMailByID(pkt.MailID);
	ReadRodex.append();
	ReadRodex.initData(pkt, mail);
}

/**
 * Notification about the result of request to get zeny from Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_ZENY_FROM_RODEX
 */
function rodexGetZeny(pkt) {
	switch (pkt.result) {
		case 1: // failure
			ChatBox.addText(DB.getMessage(2592), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2: //  too many zenys
			ChatBox.addText(DB.getMessage(2593), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			break;
		default:
			ChatBox.addText(DB.getMessage(2591), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			ReadRodex.clearZeny();
	}
}

/**
 * Notification about the result of request to get item from Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_ITEM_FROM_RODEX
 */
function rodexGetItem(pkt) {
	switch (pkt.result) {
		case 1: // failure
			ChatBox.addText(DB.getMessage(2589), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2: //  too many items
			ChatBox.addText(DB.getMessage(2590), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			break;
		default:
			ChatBox.addText(DB.getMessage(2588), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
			ReadRodex.clearItemList();
	}
}

/**
 * Server allowed to write Rodex Mail (under some conditions it contais the receiver name)
 *
 * @param {object} pkt - PACKET.ZC.ACK_OPEN_WRITE_RODEX
 */
function openWindowsWriteMail(pkt) {
	WriteRodex.append();
	WriteRodex.initData(pkt);
}

/**
 * Notification about the result of the request to add item into Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_ADD_ITEM_RODEX
 */
function onRodexItemAdded(pkt) {
	if (!pkt.result) {
		ChatBox.addText(DB.getMessage(2594), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		WriteRodex.addItem(pkt);
	} else {
		ChatBox.addText(DB.getMessage(2589), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Notification about the result of the request to remove item from Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_REMOVE_RODEX_ITEM
 */
function onRodexRemoveItem(pkt) {
	if (pkt.result) {
		ChatBox.addText(DB.getMessage(2589), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		WriteRodex.removeItem(pkt.index, pkt.count, pkt.weight);
	} else {
		ChatBox.addText(DB.getMessage(2588), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Notification about the result of the request to Send Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_SEND_RODEX
 */
function rodexSend(pkt) {
	if (!pkt.result) {
		ChatBox.addText(DB.getMessage(2587), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	} else {
		ChatBox.addText(DB.getMessage(2597), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	}
	WriteRodex.close();
}

/**
 *  Notification about the result of the request to Delete Rodex Mail
 *
 * @param {object} pkt - PACKET.ZC.ACK_DELETE_RODEX
 */
function rodexDelete(pkt) {
	if (!pkt.result) {
		ChatBox.addText(DB.getMessage(1038), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		Rodex.updateDeletedMailContent(pkt.openType, pkt.MailID);
		ReadRodex.close();
	} else {
		ChatBox.addText(DB.getMessage(1039), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	}
}

function rodexCharacterInfo(pkt) {
	if (pkt.CharID > 0) {
		WriteRodex.characterInfo(pkt);
	} else {
		ChatBox.addText(DB.getMessage(2615), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.RODEX_ICON, rodexIcon);
	Network.hookPacket(PACKET.ZC.ACK_RODEX_LIST, rodexList);
	Network.hookPacket(PACKET.ZC.ACK_RODEX_LIST2, rodexList);
	Network.hookPacket(PACKET.ZC.ACK_RODEX_LIST3, rodexList);
	Network.hookPacket(PACKET.ZC.ACK_RODEX_LIST4, rodexList);
	Network.hookPacket(PACKET.ZC.ACK_FAILED_ALL_RODEX_LIST, rodexGetListFailed);
	Network.hookPacket(PACKET.ZC.ACK_READ_RODEX, rodexRead);
	Network.hookPacket(PACKET.ZC.ACK_READ_RODEX2, rodexRead);
	Network.hookPacket(PACKET.ZC.ACK_ZENY_FROM_RODEX, rodexGetZeny);
	Network.hookPacket(PACKET.ZC.ACK_ITEM_FROM_RODEX, rodexGetItem);
	Network.hookPacket(PACKET.ZC.ACK_OPEN_WRITE_RODEX, openWindowsWriteMail);
	Network.hookPacket(PACKET.ZC.ACK_ADD_ITEM_RODEX, onRodexItemAdded);
	Network.hookPacket(PACKET.ZC.ACK_ADD_ITEM_RODEX2, onRodexItemAdded);
	Network.hookPacket(PACKET.ZC.ACK_REMOVE_RODEX_ITEM, onRodexRemoveItem);
	Network.hookPacket(PACKET.ZC.ACK_SEND_RODEX, rodexSend);
	Network.hookPacket(PACKET.ZC.ACK_DELETE_RODEX, rodexDelete);
	Network.hookPacket(PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME, rodexCharacterInfo);
	Network.hookPacket(PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME2, rodexCharacterInfo);
}
