/**
 * Engine/MapEngine/PCGoldTimer.js
 *
 * Manage PCGoldTimer Mails
 *
 * @author Alisonrag
 */

import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PCGoldTimer from 'UI/Components/PCGoldTimer/PCGoldTimer.js';

/**
 * Server sent request about PCGoldTimer Point
 *
 * @param {object} pkt - PACKET.ZC.GOLDPCCAFE_POINT
 */
function onPCGoldTimerPoint(pkt) {
	PCGoldTimer.setData(pkt);
	PCGoldTimer.append();
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.GOLDPCCAFE_POINT, onPCGoldTimerPoint);
}
