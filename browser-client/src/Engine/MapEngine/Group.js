/**
 * Engine/MapEngine/Group.js
 *
 * Manage group/party
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
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import EntityManager from 'Renderer/EntityManager.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import UIManager from 'UI/UIManager.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import WorldMap from 'UI/Components/WorldMap/WorldMap.js';
import MiniMap from 'UI/Components/MiniMap/MiniMap.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';

/**
 * @var {string} temporary variable to store party name
 */
let _partyName = '';

/**
 * Party namespace
 */
class GroupEngine {
	/**
	 * Initialize engine
	 */
	static init() {
		Network.hookPacket(PACKET.ZC.NOTIFY_HP_TO_GROUPM, onMemberLifeUpdate);
		Network.hookPacket(PACKET.ZC.NOTIFY_HP_TO_GROUPM_R2, onMemberLifeUpdate);
		Network.hookPacket(PACKET.ZC.NOTIFY_CHAT_PARTY, onMemberTalk);
		Network.hookPacket(PACKET.ZC.GROUPINFO_CHANGE, onPartyOption);
		Network.hookPacket(PACKET.ZC.REQ_GROUPINFO_CHANGE_V2, onPartyOption);
		Network.hookPacket(PACKET.ZC.PARTY_CONFIG, onPartyConfig);
		Network.hookPacket(PACKET.ZC.NOTIFY_POSITION_TO_GROUPM, onMemberMove);
		Network.hookPacket(PACKET.ZC.PARTY_JOIN_REQ, onPartyInvitationRequest);
		Network.hookPacket(PACKET.ZC.PARTY_JOIN_REQ_ACK, onPartyInvitationAnswer);
		Network.hookPacket(PACKET.ZC.ACK_REQ_JOIN_GROUP, onPartyInvitationAnswer);
		Network.hookPacket(PACKET.ZC.GROUP_LIST, onPartyList);
		Network.hookPacket(PACKET.ZC.GROUP_LIST2, onPartyList);
		Network.hookPacket(PACKET.ZC.GROUP_LIST3, onPartyList);
		Network.hookPacket(PACKET.ZC.ADD_MEMBER_TO_GROUP, onPartyMemberJoin);
		Network.hookPacket(PACKET.ZC.ADD_MEMBER_TO_GROUP2, onPartyMemberJoin);
		Network.hookPacket(PACKET.ZC.ADD_MEMBER_TO_GROUP3, onPartyMemberJoin);
		Network.hookPacket(PACKET.ZC.ADD_MEMBER_TO_GROUP4, onPartyMemberJoin);
		Network.hookPacket(PACKET.ZC.DELETE_MEMBER_FROM_GROUP, onPartyMemberLeave);
		Network.hookPacket(PACKET.ZC.ACK_MAKE_GROUP, onPartyCreate);
		Network.hookPacket(PACKET.ZC.GROUP_ISALIVE, onPartyIsAlive);

		const PartyUI = PartyFriends.getUI();

		PartyUI.onExpelMember = GroupEngine.onRequestExpel;
		PartyUI.onRequestChangeLeader = GroupEngine.onRequestChangeLeader;
		PartyUI.onRequestLeave = GroupEngine.onRequestLeave;
		PartyUI.onRequestPartyCreation = GroupEngine.onRequestCreation;
		PartyUI.onRequestAddingMember = GroupEngine.onRequestInvitation;
		PartyUI.onRequestSettingUpdate = GroupEngine.onRequestInfoUpdate;
	}

	/**
	 * Create a group (/organize)
	 *
	 * @param {string} party name
	 */
	static onRequestCreationEasy(name) {
		if (Session.hasParty) {
			return;
		}

		_partyName = name;

		const pkt = new PACKET.CZ.MAKE_GROUP();
		pkt.groupName = name;
		Network.sendPacket(pkt);
	}

	/**
	 * Create a group (from interface)
	 *
	 * @param {string} party name
	 * @param {number} option 1
	 * @param {number} option 2
	 */
	static onRequestCreation(name, pickupRule, divisionRule) {
		if (Session.hasParty) {
			return;
		}

		_partyName = name;
		const pkt = new PACKET.CZ.MAKE_GROUP2();
		pkt.groupName = name;
		pkt.ItemPickupRule = pickupRule;
		pkt.ItemDivisionRule = divisionRule;
		Network.sendPacket(pkt);
	}

	/**
	 * Request to invite someone in your party
	 *
	 * @param {number} account id
	 * @param {string} pseudo
	 */
	static onRequestInvitation(AID, pseudo) {
		if (!Session.hasParty || !Session.isPartyLeader) {
			return;
		}

		ChatBox.addText(
			pseudo + ' ' + DB.getMessage(2059, ' has recieved an invitation to join your party.'),
			ChatBox.TYPE.BLUE,
			ChatBox.FILTER.PARTY_SETUP
		);

		if (PACKETVER.value >= 20130529) {
			const pkt = new PACKET.CZ.PARTY_JOIN_REQ();
			pkt.characterName = pseudo;
			Network.sendPacket(pkt);
		} else {
			const pkt = new PACKET.CZ.REQ_JOIN_GROUP();
			pkt.AID = AID;
			pkt.CharName = pseudo;
			Network.sendPacket(pkt);
		}
	}

	/**
	 * Ask to leave a party (/leave)
	 */
	static onRequestLeave() {
		if (!Session.hasParty) {
			return;
		}

		const pkt = new PACKET.CZ.REQ_LEAVE_GROUP();
		Network.sendPacket(pkt);
	}

	/**
	 * Request to expel someone in your party
	 *
	 * @param {number} account id
	 * @param {string} pseudo
	 */
	static onRequestExpel(AID, pseudo) {
		if (!Session.hasParty || !Session.isPartyLeader) {
			return;
		}

		const pkt = new PACKET.CZ.REQ_EXPEL_GROUP_MEMBER();
		pkt.AID = AID;
		pkt.characterName = pseudo;
		Network.sendPacket(pkt);
	}

	/**
	 * Request to change party option
	 *
	 * @param {number} exp option
	 * @param {number} pickup item option
	 * @param {number} dision item option
	 */
	static onRequestInfoUpdate(expOption, pickupRule, divisionRule) {
		if (!Session.hasParty || !Session.isPartyLeader) {
			return;
		}

		const pkt = new PACKET.CZ.GROUPINFO_CHANGE_V2();
		pkt.expOption = expOption;
		pkt.ItemPickupRule = pickupRule;
		pkt.ItemDivisionRule = divisionRule;
		Network.sendPacket(pkt);
	}

	/**
	 * Request to change party leader
	 *
	 * @param {number} AID
	 */
	static onRequestChangeLeader(AID) {
		if (!Session.hasParty || !Session.isPartyLeader) {
			return;
		}

		const pkt = new PACKET.CZ.CHANGE_GROUP_MASTER();
		pkt.AID = AID;
		Network.sendPacket(pkt);
	}
}
/**
 * Get answer from party creation
 *
 * @param {object} pkt - PACKET.ZC.ACK_MAKE_GROUP
 */
function onPartyCreate(pkt) {
	switch (pkt.result) {
		case 0: {
			// Ok, process
			ChatBox.addText(DB.getMessage(77), ChatBox.TYPE.BLUE, ChatBox.FILTER.PARTY_SETUP);

			// Fallback: If party UI was already initialized by other packets (GROUP_LIST, ADD_MEMBER), skip this.
			if (Session.hasParty) {
				return;
			}

			// Otherwise, perform conditional initialization (e.g. on server versions where this arrives first)
			Session.hasParty = true;

			const entity = Session.Entity;
			const memberData = {
				AID: Session.AID,
				characterName: entity.display.name,
				role: 0, // leader
				state: 0, // online
				mapName: MapRenderer.currentMap
			};

			// Enrich mock member with actual level/class/life info immediately
			if (entity) {
				if (entity.display && entity.display.lvl) {
					memberData.baseLevel = entity.display.lvl;
				}
				if (entity.job !== undefined) {
					memberData.class_ = entity.job;
				}
				if (entity.life && entity.life.display) {
					memberData.life = entity.life;
				}
			}

			PartyFriends.getUI().setParty(_partyName, [memberData]);
			break;
		}
		case 1: // party name already exists
			ChatBox.addText(DB.getMessage(78), ChatBox.TYPE.ERROR, ChatBox.FILTER.PARTY_SETUP);
			break;

		case 2: // already in a party
			ChatBox.addText(DB.getMessage(79), ChatBox.TYPE.ERROR, ChatBox.FILTER.PARTY_SETUP);
			break;

		case 3: // cannot organize parties on this map
			ChatBox.addText(DB.getMessage(1387), ChatBox.TYPE.ERROR, ChatBox.FILTER.PARTY_SETUP);
			break;
	}
}

/**
 * Receive dead/alive status update for a party member
 *
 * @param {object} pkt - PACKET.ZC.GROUP_ISALIVE
 */
function onPartyIsAlive(pkt) {
	PartyFriends.getUI().updateMemberDead(pkt.AID, pkt.isDead);
}

/**
 * Get list of party members
 *
 * @param {object} pkt - PACKET.ZC.GROUP_LIST
 */
function onPartyList(pkt) {
	let entity;

	Session.hasParty = true;
	const count = pkt.groupInfo.length;

	for (let i = 0; i < count; ++i) {
		entity = EntityManager.get(pkt.groupInfo[i].AID);
		if (entity) {
			// Enrich life data if available
			if (entity.life.display) {
				pkt.groupInfo[i].life = entity.life;
			}
			// Enrich level/class when the packet variant doesn't include them (e.g. GROUP_LIST 0xfb)
			if (!pkt.groupInfo[i].baseLevel && entity.display && entity.display.lvl) {
				pkt.groupInfo[i].baseLevel = entity.display.lvl;
			}
			if (!pkt.groupInfo[i].class_ && entity.job !== undefined) {
				pkt.groupInfo[i].class_ = entity.job;
			}
		}
	}

	PartyFriends.getUI().setParty(pkt.groupName, pkt.groupInfo);
	WorldMap.updatePartyMembers(pkt);
}

/**
 * Update a member in party
 *
 * @param {object} pkt - PACKET.ZC.ADD_MEMBER_TO_GROUP
 */
function onPartyMemberJoin(pkt) {
	const entity = EntityManager.get(pkt.AID);

	if (entity) {
		if (entity.life.display) {
			pkt.life = entity.life;
		}
		// Enrich level/class when the packet variant doesn't include them
		if (!pkt.baseLevel && entity.display && entity.display.lvl) {
			pkt.baseLevel = entity.display.lvl;
		}
		if (!pkt.class_ && entity.job !== undefined) {
			pkt.class_ = entity.job;
		}
	}

	const PartyUI = PartyFriends.getUI();

	if (pkt.AID === Session.AID) {
		Session.hasParty = true;
	}
	PartyUI.setOptions(pkt.expOption, pkt.ItemPickupRule, pkt.ItemDivisionRule);
	PartyUI.addPartyMember(pkt);
}

/**
 * Remove a player from the group
 *
 * @param {object} pkt - PACKET.ZC.DELETE_MEMBER_FROM_GROUP
 */
function onPartyMemberLeave(pkt) {
	switch (pkt.result) {
		case 0: // leave
		case 1: // expel
			break;

		case 2:
			// Cannot leave a party in this map
			ChatBox.addText(DB.getMessage(1872), ChatBox.TYPE.ERROR, ChatBox.FILTER.PARTY_SETUP);
			return;

		case 3:
			// Cannot withdraw/break the party in this map
			ChatBox.addText(DB.getMessage(1873), ChatBox.TYPE.ERROR, ChatBox.FILTER.PARTY_SETUP);
			return;
	}

	if (Session.AID === pkt.AID) {
		Session.hasParty = false;
	}

	PartyFriends.getUI().removePartyMember(pkt.AID, pkt.characterName);
}

/**
 * Display entity life
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_HP_TO_GROUPM
 */
function onMemberLifeUpdate(pkt) {
	EntityManager.storeLife(pkt.AID, { hp: pkt.hp, hp_max: pkt.maxhp });
	const entity = EntityManager.get(pkt.AID);

	if (entity) {
		entity.life.hp = pkt.hp;
		entity.life.hp_max = pkt.maxhp;
		entity.life.update();

		if (entity && entity.life && entity.life.canvas) {
			PartyFriends.getUI().updateMemberLife(pkt.AID, entity.life.canvas, pkt.hp, pkt.maxhp);
		}
	}
}

/**
 * Display player message
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_CHAT_PARTY
 */
function onMemberTalk(pkt) {
	const entity = EntityManager.get(pkt.AID);

	if (entity) {
		entity.dialog.set(pkt.msg);
	}

	ChatBox.addText(pkt.msg, ChatBox.TYPE.PARTY, ChatBox.FILTER.PARTY);
}

/**
 * Move minimap viewpoint
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_POSITION_TO_GROUPM
 */
function onMemberMove(pkt) {
	// Server remove mark with "-1" as position
	if (pkt.xPos < 0 || pkt.yPos < 0) {
		MiniMap.getUI().removePartyMemberMark(pkt.AID);
	} else {
		MiniMap.getUI().addPartyMemberMark(pkt.AID, pkt.xPos, pkt.yPos);
	}
}

/**
 * Get party information
 *
 * @param {object} pkt - PACKET.ZC.GROUPINFO_CHANGE
 */
function onPartyOption(pkt) {
	PartyFriends.getUI().setOptions(pkt.expOption, pkt.ItemPickupRule, pkt.ItemDivisionRule);

	ChatBox.addText(
		DB.getMessage(291) + '  - ' + DB.getMessage(292) + '  : ' + DB.getMessage(287 + pkt.expOption),
		ChatBox.TYPE.PRIVATE,
		ChatBox.FILTER.PARTY_SETUP
	);

	// Some packets don't have ItemPickupRule and ItemDivisionRule so we need to check if they exist
	if (pkt.ItemPickupRule !== undefined) {
		ChatBox.addText(
			DB.getMessage(291) + '  - ' + DB.getMessage(293) + '  : ' + DB.getMessage(289 + pkt.ItemPickupRule),
			ChatBox.TYPE.PRIVATE,
			ChatBox.FILTER.PARTY_SETUP
		);
	}
	if (pkt.ItemDivisionRule !== undefined) {
		ChatBox.addText(
			DB.getMessage(291) + '  - ' + DB.getMessage(738) + '  : ' + DB.getMessage(287 + pkt.ItemDivisionRule),
			ChatBox.TYPE.PRIVATE,
			ChatBox.FILTER.PARTY_SETUP
		);
	}
}

/**
 * Get party configs
 *
 * @param {object} pkt - PACKET.ZC.PARTY_CONFIG
 */
function onPartyConfig(pkt) {
	ChatBox.addText(DB.getMessage(pkt.bRefuseJoinMsg ? 1325 : 1326), ChatBox.TYPE.INFO, ChatBox.FILTER.PARTY_SETUP);
}

/**
 * Get a request from someone to join a team
 *
 * @param {object} pkt - PACKET.ZC.PARTY_JOIN_REQ
 */
function onPartyInvitationRequest(packet) {
	const GRID = packet.GRID;

	function onAnswer(accept) {
		return function () {
			const pkt = new PACKET.CZ.PARTY_JOIN_REQ_ACK();
			pkt.GRID = GRID;
			pkt.bAccept = accept;
			Network.sendPacket(pkt);
		};
	}

	UIManager.showPromptBox(packet.groupName + ' ' + DB.getMessage(94), 'ok', 'cancel', onAnswer(1), onAnswer(0));
}

/**
 * Answer from a player to join your group
 *
 * @param {object} pkt - PACKET.ZC.PARTY_JOIN_REQ_ACK
 */
function onPartyInvitationAnswer(pkt) {
	let id = 1,
		color = ChatBox.TYPE.ERROR;

	switch (pkt.answer) {
		case 0:
			id = 80;
			break;
		case 1:
			id = 81;
			break;

		case 2:
			id = 82;
			color = ChatBox.TYPE.BLUE;
			break;

		case 3:
			id = 83;
			break;
		case 4:
			id = 608;
			break;
		case 5:
			id = 1324;
			break;
		// no 6 ?
		case 7:
			id = 71;
			break;
		case 8:
			id = 1388;
			break;
		case 9:
			id = 1871;
			break;
	}

	ChatBox.addText(DB.getMessage(id).replace('%s', pkt.characterName), color, ChatBox.FILTER.PARTY_SETUP);
}

/**
 * Export s
 */
export default GroupEngine;
