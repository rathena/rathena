/**
 * Engine/MapEngine/Bank.js
 *
 * Manage Bank sockets
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Session from 'Engine/SessionStorage.js';
import Bank from 'UI/Components/Bank/Bank.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';

class BankEngine {
	static init() {
		Network.hookPacket(PACKET.ZC.ACK_OPEN_BANKING, onOpenBank);
		Network.hookPacket(PACKET.ZC.BANKING_CHECK, onBankInfo);
		Network.hookPacket(PACKET.ZC.ACK_BANKING_DEPOSIT, onBankDepoUpdate);
		Network.hookPacket(PACKET.ZC.ACK_BANKING_WITHDRAW, onBankWithdrawUpdate);
		Network.hookPacket(PACKET.ZC.ACK_CLOSE_BANKING, onBankClose);
	}
}

function onOpenBank(pkt) {
	const send_pkt = new PACKET.CZ.REQ_BANKING_CHECK();
	send_pkt.AID = Session.AID;
	Network.sendPacket(send_pkt);
}

function onBankInfo(pkt) {
	if (!Bank.__active) {
		Bank.append();
		Bank.updateBankDisplay(pkt.money, Session.zeny);
		Bank.focusInput();
	}
}

function onBankClose() {
	if (Bank.__active) {
		Bank.remove();
	}
}

function onBankDepoUpdate(pkt) {
	if (!pkt) {
		return;
	}

	switch (pkt.reason) {
		case 0:
			Bank.updateBankDisplay(pkt.money, Session.zeny);
			Bank.clearError();
			break;
		case 1:
			break;
		case 2:
			Bank.setError(DB.getMessage(2780));
			ChatBox.addText(DB.getMessage(2456), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 3:
			Bank.setError(DB.getMessage(2783));
			ChatBox.addText(DB.getMessage(2787), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		default:
			break;
	}

	Bank.clearInput();
}

function onBankWithdrawUpdate(pkt) {
	if (!pkt) {
		return;
	}

	switch (pkt.reason) {
		case 0:
			Bank.updateBankDisplay(pkt.money, Session.zeny);
			Bank.clearError();
			Bank.clearInput();
			break;
		case 1:
			Bank.setError(DB.getMessage(2786));
			ChatBox.addText(DB.getMessage(2455), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2:
			break;
		default:
			break;
	}
}

export default BankEngine;
