/**
 * Engine/MapEngine/Clan.js
 *
 * Manage Clan packets and UI
 *
 * clan file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Clan from 'UI/Components/Clan/Clan.js';
import Session from 'Engine/SessionStorage.js';

/**
 * Clan data
 */
let clan = {};

const RELATIONSHIP = {
	ALLY: 0,
	ENEMY: 1
};

/**
 * Reset clan data
 */
function resetClan() {
	clan = {
		clanId: 0,
		name: '',
		master: '',
		territory: '',
		allyCount: 0,
		allyList: [],
		antagonistCount: 0,
		antagonistList: [],
		level: 1,
		membersOnline: 0,
		membersTotal: 0,
		emblem: ''
	};
}

/**
 * Received clan connect info
 * @param {object} pkt - PACKET.ZC.NOTIFY_CLAN_CONNECTINFO
 */
function onNotifyClanConnectInfo(pkt) {
	clan.membersOnline = pkt.membersOnline;
	clan.membersTotal = pkt.membersTotal;
	Clan.setMembersCount(clan);
}

/**
 * Received clan leave
 * @param {object} pkt - PACKET.ZC.ACK_CLAN_LEAVE
 */
function onAckClanLeave(pkt) {
	resetClan();
	Clan.leave();
	Session.hasClan = false;
}

/**
 * Received clan info
 * @param {object} pkt - PACKET.ZC.CLANINFO
 */
function onClanInfo(pkt) {
	clan.clanId = pkt.clanId;
	clan.name = pkt.name;
	clan.master = pkt.master;
	clan.territory = pkt.territory;
	clan.allyCount = pkt.allyCount;
	clan.antagonistCount = pkt.antagonistCount;
	clan.allyList = pkt.allyList;
	clan.antagonistList = pkt.antagonistList;
	clan.level = 1;
	Clan.append();
	Clan.setData(clan);
	Clan.setRelations(RELATIONSHIP.ALLY, clan.allyList);
	Clan.setRelations(RELATIONSHIP.ENEMY, clan.antagonistList);
	Clan.setIllust(clan.clanId);
	Clan.setEmblem(clan.clanId);
	Session.hasClan = true;
}

/**
 * Received clan chat
 * @param {object} pkt - PACKET.ZC.NOTIFY_CLAN_CHAT
 */
function onNotifyClanChat(pkt) {
	const clan_message =
		pkt.memberName && pkt.memberName?.length > 0 ? pkt.memberName + ': ' + pkt.message : pkt.message;
	ChatBox.addText(clan_message, ChatBox.TYPE.CLAN, ChatBox.FILTER.CLAN);
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.CLANINFO, onClanInfo);
	Network.hookPacket(PACKET.ZC.NOTIFY_CLAN_CONNECTINFO, onNotifyClanConnectInfo);
	Network.hookPacket(PACKET.ZC.ACK_CLAN_LEAVE, onAckClanLeave);
	Network.hookPacket(PACKET.ZC.NOTIFY_CLAN_CHAT, onNotifyClanChat);
}
