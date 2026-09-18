/**
 * Engine/MapEngine/Roulette.js
 *
 * Manage Roulette System
 *
 * @author [Your Name]
 */

/**
 * Load dependencies
 */
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Roulette from 'UI/Components/Roulette/Roulette.js';

/**
 * Request to Open Roulette
 */
/*function requestOpenRoulette() {
		let pkt = new PACKET.CZ.REQ_OPEN_ROULETTE();
		Network.sendPacket(pkt);
	}*/ // UNUSED

/**
 * Request Roulette Info
 */
/*function requestRouletteInfo() {
		let pkt = new PACKET.CZ.REQ_ROULETTE_INFO();
		Network.sendPacket(pkt);
	}*/ // UNUSED

/**
 * Request to Close Roulette
 */
/*function requestCloseRoulette() {
		let pkt = new PACKET.CZ.REQ_CLOSE_ROULETTE();
		Network.sendPacket(pkt);
	}*/ // UNUSED

/**
 * Receive Packets
 */

/**
 * Open Roulette Window
 *
 * @param {object} pkt - PACKET.ZC.ACK_OPEN_ROULETTE
 */
function onOpenRoulette(pkt) {
	// pkt structure:
	// {
	//   result: number,       // 0 = success, 1 = fail
	//   serial: number,
	//   step: number,
	//   idx: number,
	//   additionItemID: number,
	//   goldPoint: number,
	//   silverPoint: number,
	//   bronzePoint: number
	// }

	// Server responded (standard rAthena implementation)
	if (pkt.result === 0) {
		// Append component to DOM if not already appended
		if (!Roulette.ui) {
			Roulette.append();
		}
		Roulette.onOpen(pkt);
	} else {
		console.error('Failed to open roulette:', pkt.result);
	}
}

/**
 * Receive Roulette Info (item list)
 *
 * @param {object} pkt - PACKET.ZC.ACK_ROULETTE_INFO
 */
function onRouletteInfo(pkt) {
	// pkt structure:
	// {
	//   serial: number,
	//   items: [{row, position, itemId, count}, ...]
	// }

	// Append component to DOM if not already appended
	if (!Roulette.ui) {
		Roulette.append();
	}
	Roulette.onRouletteInfo(pkt);
}

/**
 * Receive Roulette Spin Result
 *
 * @param {object} pkt - PACKET.ZC.ACK_GENERATE_ROULETTE
 */
function onGenerateRoulette(pkt) {
	// pkt structure:
	// {
	//   result: number,       // 0 = success, other = fail
	//   step: number,
	//   idx: number,
	//   additionItemID: number,
	//   remainGold: number,
	//   remainSilver: number,
	//   remainBronze: number
	// }

	if (pkt.result === 0) {
		// Append component to DOM if not already appended
		if (!Roulette.ui) {
			Roulette.append();
		}
		Roulette.onResult(pkt);
	} else {
		console.error('Roulette spin failed:', pkt.result);
	}
}

/**
 * Receive Roulette Close ACK
 *
 * @param {object} pkt - PACKET.ZC.ACK_CLOSE_ROULETTE
 */
function onCloseRoulette(pkt) {
	// pkt structure:
	// {
	//   result: number  // 0 = success, other = fail
	// }

	if (pkt.result === 0 && Roulette.ui) {
		Roulette.ui.hide();
	}
}

/**
 * Receive Roulette Item Result
 *
 * @param {object} pkt - PACKET.ZC.RECV_ROULETTE_ITEM
 */
function onRecvRouletteItem(pkt) {
	// pkt structure:
	// {
	//   result: number,        // 0=success, 1=failed, 2=overcount, 3=overweight
	//   additionItemID: number
	// }

	if (pkt.result === 0) {
		// Item received successfully
		if (Roulette.ui && typeof Roulette.onItemReceived === 'function') {
			Roulette.onItemReceived(pkt);
		}
	} else {
		let errorMsg = 'Failed to receive roulette item';
		switch (pkt.result) {
			case 1:
				errorMsg = 'Failed to receive item';
				break;
			case 2:
				errorMsg = 'Item count exceeded';
				break;
			case 3:
				errorMsg = 'Overweight';
				break;
		}
		console.error('[Roulette]', errorMsg);
	}
}

/**
 * Initialize
 */
export default function RouletteEngine() {
	Network.hookPacket(PACKET.ZC.ACK_OPEN_ROULETTE, onOpenRoulette);
	Network.hookPacket(PACKET.ZC.ACK_ROULETTE_INFO, onRouletteInfo);
	Network.hookPacket(PACKET.ZC.ACK_GENERATE_ROULETTE, onGenerateRoulette);
	Network.hookPacket(PACKET.ZC.ACK_CLOSE_ROULETTE, onCloseRoulette);
	Network.hookPacket(PACKET.ZC.RECV_ROULETTE_ITEM, onRecvRouletteItem);
}
