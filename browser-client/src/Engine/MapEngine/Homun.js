/**
 * Engine/MapEngine/Homun.js
 *
 * Manage homunculus
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author IssID
 *
 * @typedef {Object} THomunPacket
 * @prop {number} ATKRange
 * @prop {number} ITID
 * @prop {number} Matk
 * @prop {number} Mdef
 * @prop {number} SKPoint
 * @prop {number} aspd
 * @prop {number} atk
 * @prop {number} bModified
 * @prop {number} critical
 * @prop {number} def
 * @prop {number} exp
 * @prop {number} flee
 * @prop {number} hit
 * @prop {number} hp
 * @prop {TLife} life
 * @prop {number} maxEXP
 * @prop {number} maxHP
 * @prop {number} maxSP
 * @prop {number} nFullness
 * @prop {number} nLevel
 * @prop {number} nRelationship
 * @prop {number} sp
 * @prop {string} szName Homun name
 *
 * @typedef {Object} TLife
 * @prop {number} ap
 * @prop {number} ap_max
 * @prop {HTMLCanvasElement} canvas
 * @prop {CanvasRenderingContext2D} ctx
 * @prop {boolean} display
 * @prop {Entity} entity
 * @prop {number} hp
 * @prop {number} hp_max
 * @prop {number} hunger
 * @prop {number} hunger_max
 * @prop {number} sp
 * @prop {number} sp_max
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Session from 'Engine/SessionStorage.js';
import EntityManager from 'Renderer/EntityManager.js';
import UIManager from 'UI/UIManager.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import HomunInformations from 'UI/Components/HomunInformations/HomunInformations.js';
import SkillListMH from 'UI/Components/SkillListMH/SkillListMH.js';
import Mouse from 'Controls/MouseEventHandler.js';
import StatusProperty from 'DB/Status/StatusProperty.js';

/**
 * @type {THomunPacket} cached homunculus information
 */
let _info = {};

/**
 * Get own homunculus information from server
 *
 * @param {THomunPacket} pkt - PACKET.ZC.PROPERTY_HOMUN
 */
function onHomunInformation(pkt) {
	_info = pkt;

	if (!Session.homunId) {
		return;
	}

	const entity = EntityManager.get(Session.homunId);

	if (!entity) {
		return;
	}

	entity.attack_range = pkt.ATKRange;
	entity.life.hp = pkt.hp;
	entity.life.hp_max = pkt.maxHP;
	entity.life.sp = pkt.sp;
	entity.life.sp_max = pkt.maxSP;
	entity.life.hunger = pkt.nFullness;
	entity.life.hunger_max = 100;
	entity.life.update();

	EntityManager.storeLife(Session.homunId, {
		hp: pkt.hp,
		hp_max: pkt.maxHP,
		sp: pkt.sp,
		sp_max: pkt.maxSP,
		hunger: pkt.nFullness,
		hunger_max: 100
	});

	HomunInformations.setInformations(pkt);
	HomunInformations.startAI();
}

/**
 * Result from feeding your homun
 *
 * @param {object} pkt - PACKET.ZC.FEED_HOMUN
 */
function onFeedResult(pkt) {
	// Fail to feed
	if (!pkt.cRet) {
		ChatBox.addText(
			DB.getMessage(591).replace('%s', DB.getItemInfo(pkt.ITID).identifiedDisplayName),
			ChatBox.TYPE.ERROR,
			ChatBox.FILTER.PUBLIC_LOG
		);
		return;
	}

	// success, what to do ? Action feed ? or is it sent by server ?
}

/**
 * Update homun parameter
 *
 * @param {object} pkt - PACKET.ZC.HO_PAR_CHANGE
 */
function onHomunParameterChange(pkt) {
	if (!Session.homunId) {
		return;
	}

	// UI update
	HomunInformations.setInformations(pkt);

	const entity = EntityManager.get(Session.homunId);

	if (!entity) {
		return;
	}

	switch (pkt.param) {
		case StatusProperty.SPEED:
			entity.walk.speed = pkt.value;
			break;

		case StatusProperty.EXP:
			HomunInformations.base_exp = pkt.value;
			HomunInformations.setExp(HomunInformations.base_exp, HomunInformations.base_exp_next);
			break;

		case StatusProperty.HP:
			entity.life.hp = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.homunId, { hp: pkt.value });
			HomunInformations.setHpSpBar('hp', entity.life.hp, entity.life.hp_max);
			break;

		case StatusProperty.MAXHP:
			entity.life.hp_max = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.homunId, { hp_max: pkt.value });
			HomunInformations.setHpSpBar('hp', entity.life.hp, entity.life.hp_max);
			break;

		case StatusProperty.SP:
			entity.life.sp = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.homunId, { sp: pkt.value });
			HomunInformations.setHpSpBar('sp', entity.life.sp, entity.life.sp_max);
			break;

		case StatusProperty.MAXSP:
			entity.life.sp_max = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.homunId, { sp_max: pkt.value });
			HomunInformations.setHpSpBar('sp', entity.life.sp, entity.life.sp_max);
			break;

		case StatusProperty.CLEVEL:
			entity.clevel = pkt.value;
			break;

		case StatusProperty.MAXEXP:
			HomunInformations.base_exp_next = pkt.value;
			HomunInformations.setExp(HomunInformations.base_exp, HomunInformations.base_exp_next);
			break;

		default:
			console.log('Homun::onHomunParameterChange() - Unsupported type', pkt);
	}
}

/**
 * Update homun information
 *
 * @param {object} pkt - PACKET.ZC.CHANGESTATE_HOMUN
 */
function onHomunInformationUpdate(pkt) {
	const entity = EntityManager.get(pkt.GID);

	if (entity) {
		switch (pkt.state) {
			case 0:
				HomunInformations.append();
				Session.homunId = pkt.GID;
				break;

			case 1:
				HomunInformations.setIntimacy(pkt.data);
				break;

			case 2:
				HomunInformations.setHunger(pkt.data);
				entity.life.hunger = pkt.data;
				entity.life.hunger_max = 100;
				entity.life.update();
				break;
		}
	}
}

/**
 * Client request to feed QHomun.
 */
HomunInformations.reqHomunFeed = function reqHomunFeed() {
	// Are you sure you want to feed your homunculus ?
	UIManager.showPromptBox(DB.getMessage(601), 'ok', 'cancel', () => {
		const pkt = new PACKET.CZ.COMMAND_MER();
		pkt.type = 0x22f;
		pkt.command = 1;
		Network.sendPacket(pkt);
	});
};

HomunInformations.sendHomunFeed = function sendHomunFeed() {
	const pkt = new PACKET.CZ.COMMAND_MER();
	pkt.type = 0x22f;
	pkt.command = 1;
	Network.sendPacket(pkt);
};

HomunInformations.reqDeleteHomun = function reqDeleteHomun() {
	// Are you sure that you want to delete?
	UIManager.showPromptBox(DB.getMessage(356), 'ok', 'cancel', () => {
		const pkt = new PACKET.CZ.COMMAND_MER();
		pkt.type = 0x22f;
		pkt.command = 2;
		Network.sendPacket(pkt);
	});
};

/**
 * Client request to do a performance
 * type:
 *     always 0
 * command:
 *     0 = homunculus information
 *     1 = feed
 *     2 = delete
 */
HomunInformations.reqHomunAction = function reqHomunAction(cmd) {
	const pkt = new PACKET.CZ.COMMAND_MER(cmd);
	pkt.type = 0;
	pkt.command = cmd;
	Network.sendPacket(pkt);
};

/**
 * @param gid
 */
HomunInformations.reqMoveToOwner = function reqMoveToOwner(gid) {
	const pkt = new PACKET.CZ.REQUEST_MOVETOOWNER();
	pkt.GID = gid;
	Network.sendPacket(pkt);
};

/**
 * @param GID
 * @param targetGID
 */
HomunInformations.reqAttack = function reqAttack(GID, targetGID) {
	const pkt = new PACKET.CZ.REQUEST_ACTNPC();
	pkt.GID = GID;
	pkt.targetGID = targetGID;
	pkt.action = 0;
	Network.sendPacket(pkt);
};

/**
 * @param GID
 */
HomunInformations.reqMoveTo = function reqMoveTo(GID, x = 0, y = 0) {
	const pkt = new PACKET.CZ.REQUEST_MOVENPC();
	pkt.GID = GID;
	pkt.dest[0] = x > 0 ? x : Mouse.world.x;
	pkt.dest[1] = y > 0 ? y : Mouse.world.y;
	Network.sendPacket(pkt);
};

/**
 * Rename QHomunculus
 *
 * @param {string} new homunculus name
 */
HomunInformations.reqNameEdit = function reqNameEdit(name) {
	//msg 2904(2903)
	const pkt = new PACKET.CZ.RENAME_MER();
	pkt.name = name;
	Network.sendPacket(pkt);
};

/**
 * List of skills
 *
 * @param {object} pkt - PACKET.ZC.MER_SKILLINFO_LIST
 */
function onSkillList(pkt) {
	SkillListMH.homunculus.setSkills(pkt.skillList);
}

/**
 * Update a specified skill
 *
 * @param {object} pkt - PACKET.ZC.SKILLINFO_UPDATE
 */
function onSkillUpdate(pkt) {
	SkillListMH.homunculus.updateSkill(pkt);
}

// function testing( pkt )
// {
// 	console.warn(pkt)
// }

/**
 * Initialize
 */
export default function NPCEngine() {
	Network.hookPacket(PACKET.ZC.PROPERTY_HOMUN, onHomunInformation);
	Network.hookPacket(PACKET.ZC.PROPERTY_HOMUN2, onHomunInformation);
	Network.hookPacket(PACKET.ZC.PROPERTY_HOMUN3, onHomunInformation);
	Network.hookPacket(PACKET.ZC.PROPERTY_HOMUN4, onHomunInformation);
	Network.hookPacket(PACKET.ZC.PROPERTY_HOMUN5, onHomunInformation);
	Network.hookPacket(PACKET.ZC.CHANGESTATE_MER, onHomunInformationUpdate);
	Network.hookPacket(PACKET.ZC.FEED_MER, onFeedResult);
	Network.hookPacket(PACKET.ZC.HO_PAR_CHANGE, onHomunParameterChange);
	Network.hookPacket(PACKET.ZC.HO_PAR_CHANGE2, onHomunParameterChange);
	Network.hookPacket(PACKET.ZC.HOSKILLINFO_LIST, onSkillList);
	Network.hookPacket(PACKET.ZC.HOSKILLINFO_UPDATE, onSkillUpdate);

	// Network.hookPacket( PACKET.ZC.MER_INIT, testing);
	// Network.hookPacket( PACKET.ZC.MER_PROPERTY, testing);
	// Network.hookPacket( PACKET.ZC.MER_PAR_CHANGE, testing);
	// Network.hookPacket( PACKET.ZC.HOMUN_ACT, onHomunAction);
}
