/**
 * Engine/MapEngine/Mail.js
 *
 * Manage Mails
 *
 * @author Francisco Wallison
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Mail from 'UI/Components/Mail/Mail.js';
import ReadMail from 'UI/Components/Mail/ReadMail.js';

/**
 * Request to send mail
 *
 * @param {object} pkt - PACKET.CZ.MAIL_SEND
 */
/// pkt:
///     ReceiveName
///     Header
///    	msg_len
/// 	msg
Mail.parseMailSend = function parseMailSend(mail) {
	const pkt = new PACKET.CZ.MAIL_SEND();

	pkt.ReceiveName = mail.ReceiveName.replace(/^(\$|\%)/, '').replace(/\t/g, '');
	pkt.Header = mail.Header.replace(/^(\$|\%)/, '');
	pkt.msg_len = mail.msg_len;
	pkt.msg = mail.msg.replace(/^(\$|\%)/, '');
	Network.sendPacket(pkt);
};

/**
 * Request to reset mail item and/or Zeny
 * CZ_MAIL_RESET_ITEM
 * @param {int} type
 */
/// type:
///     0 = reset all
///     1 = remove item
///     2 = remove zeny
Mail.parseMailWinopen = function parseMailWinopen(type) {
	const pkt = new PACKET.CZ.MAIL_RESET_ITEM();
	pkt.Type = type;
	Network.sendPacket(pkt);
};

/**
 * Request to add an item or Zeny to mail.
 * PACKET.CZ.MAIL_ADD_ITEM
 * index : 0 - Zeny; >= 2 - Inventory item
 * @param {int} index
 * count : amout of zeny or number of item
 * @param {int} count
 */
Mail.parseMailSetattach = function parseMailSetattach(index, count) {
	if (count <= 0) {
		return;
	}

	const pkt = new PACKET.CZ.MAIL_ADD_ITEM();
	pkt.index = index;
	pkt.count = count;
	Network.sendPacket(pkt);
};

/**
 * Request to receive mail's attachment
 * CZ_MAIL_GET_ITEM
 * @param {int} MailID
 */
Mail.parseMailgetattach = function parseMailgetattach(MailID) {
	const pkt = new PACKET.CZ.MAIL_GET_ITEM();
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Mail inbox list request.
 * CZ_MAIL_GET_LIST
 */
Mail.parseMailrefreshinbox = function parseMailrefreshinbox() {
	const pkt = new PACKET.CZ.MAIL_GET_LIST();
	Network.sendPacket(pkt);
};

/**
 * Request to return a mail.
 * CZ_REQ_MAIL_RETURN
 * @param {int} MailID
 * @param {string} ReceiveName
 */
Mail.returnMail = function returnMail(MailID, ReceiveName) {
	const pkt = new PACKET.CZ.REQ_MAIL_RETURN();
	pkt.MailID = MailID;
	pkt.ReceiveName = ReceiveName;
	Network.sendPacket(pkt);
};

/**
 * Request to open a mail
 * PACKET.CZ.MAIL_OPEN
 * @param {int} MailID
 */
Mail.openMail = function openMail(MailID) {
	const pkt = new PACKET.CZ.MAIL_OPEN();
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
	const updateMail = {
		MailID: MailID,
		isOpen: 1
	};
	Mail.mailReceiveUpdate(updateMail);
};

/**
 * Request to delete a mail.
 * PACKET.CZ.MAIL_DELETE
 * @param {int} MailID
 */
Mail.deleteMail = function deleteMail(MailID) {
	const pkt = new PACKET.CZ.MAIL_DELETE();
	pkt.MailID = MailID;
	Network.sendPacket(pkt);
};

/**
 * Send mail list
 * PACKET.ZC.MAIL_WINDOWS
 */
Mail.onClosePressed = function onClosePressed() {
	Mail.remove();
};

/**
 * Send from mail to inventory
 */
Mail.reqRemoveItem = function ReqRemoveItem(index, count) {
	this.parseMailWinopen(1); // remove item
	//BUG::does not update inventory, default behavior
	Mail.removeItem();
};

/**
 * Opens a mail
 * PACKET.ZC.MAIL_REQ_OPEN
 */
function mailReqOpen(inforMail) {
	Mail.parseMailrefreshinbox();
	ReadMail.openEmail(inforMail);
}

/**
 * Notification about the result of deleting a mail.
 * @param {int} result - PACKET.ZC.ACK_MAIL_DELETE
 */
/// Result:
///     0 = success
///     1 = failure
function mailDelete(result) {
	if (!result.Result) {
		// An auction with at least one bidder cannot be canceled.
		ChatBox.addText(DB.getMessage(1038), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		ReadMail.remove();
		Mail.parseMailrefreshinbox();
	}
}

/**
 * Opens/closes the mail window
 *
 * @param {object} pkt - PACKET.ZC.MAIL_WINDOWS
 */
/// type:
///     0 = open
///     1 = close
function openWindowsMail(pkt) {
	if (pkt.Type) {
		Mail.remove();
	} else {
		Mail.append();
	}
}

/**
 * Lists mails stored in inbox.
 *
 * @param {object} read - PACKET.ZC.MAIL_REQ_GET_LIST
 */
/// read:
///     0 = unread
///     1 = read
function mailRefreshinbox(read) {
	Mail.mailList(read);
}

/**
 * Notification about the result of adding an item to mail
 *
 * @param {int} result - PACKET.ZC.ACK_MAIL_ADD_ITEM
 */
/// result:
///     0 = success
///     1 = failure
function mailSetattachment(result) {
	if (!result.result) {
		Mail.addItemSub(result.Index);
	}
}

/**
 * Notification about the result of sending a mail
 *
 * @param {int} result - PACKET.ZC.MAIL_REQ_SEND
 */
/// Result:
///     0 = success
///     1 = recipinent does not exist
function mailSend(result) {
	if (result.Result) {
		ChatBox.addText(DB.getMessage(1032), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		// clear zeny and item field
		Mail.clearFieldsItemZeny();
	} else {
		ChatBox.addText(DB.getMessage(1031), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		ReadMail.remove();
	}
}

/**
 * Notification about new mail.
 * ZC_MAIL_RECEIVE
 * @param {object} result
 */
function mailNew(result) {
	const newMail = {
		DeleteTime: new Date().getTime() / 1000,
		FromName: result.FromName,
		HEADER: result.Header,
		MailID: result.MailID,
		isOpen: 0
	};

	if (result.MailID != 0) {
		ChatBox.addText(DB.getMessage(1101), ChatBox.TYPE.MAIL, ChatBox.FILTER.PUBLIC_LOG);
		Mail.mailReceiveUpdate(newMail);
	}
}

/**
 * Notification about the result of returning a mail
 * ZC_ACK_MAIL_RETURN
 * @param {object} result
 */
function mailReturn(result) {
	if (result.MailID != 0 && result.Result === 0) {
		ChatBox.addText(DB.getMessage(1176), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		ReadMail.remove();
		Mail.parseMailrefreshinbox();
	}
}

/**
 * Notification about the result of returning a mail
 * ZC_MAIL_REQ_GET_ITEM
 * @param {int} result
 */
function mailGetItem(result) {
	if (!result.Result || result.Result == 2) {
		ReadMail.resetItemZeny();
		Mail.parseMailrefreshinbox();
	}
}

/**
 * Initialize
 */
export default function MailEngine() {
	Network.hookPacket(PACKET.ZC.MAIL_WINDOWS, openWindowsMail);
	Network.hookPacket(PACKET.ZC.MAIL_REQ_GET_LIST, mailRefreshinbox);
	Network.hookPacket(PACKET.ZC.ACK_MAIL_ADD_ITEM, mailSetattachment);
	Network.hookPacket(PACKET.ZC.MAIL_REQ_SEND, mailSend);
	Network.hookPacket(PACKET.ZC.MAIL_RECEIVE, mailNew);
	Network.hookPacket(PACKET.ZC.MAIL_REQ_OPEN, mailReqOpen);
	Network.hookPacket(PACKET.ZC.ACK_MAIL_DELETE, mailDelete);
	Network.hookPacket(PACKET.ZC.ACK_MAIL_RETURN, mailReturn);
	Network.hookPacket(PACKET.ZC.MAIL_REQ_GET_ITEM, mailGetItem);
}
