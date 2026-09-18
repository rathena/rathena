/**
 * Engine/MapEngine/Mercenary.js
 *
 * Manage mercenary
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
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
import MercenaryInformations from 'UI/Components/MercenaryInformations/MercenaryInformations.js';
import SkillListMH from 'UI/Components/SkillListMH/SkillListMH.js';
import Mouse from 'Controls/MouseEventHandler.js';

/**
 * Initialize Mercenary information
 *
 * @param {object} pkt - PACKET.ZC.MER_INIT
 */
function onMercenaryInit(pkt) {
	Session.mercId = pkt.AID;
	const entity = EntityManager.get(pkt.AID);

	if (entity) {
		entity.attack_range = pkt.ATKRange;
		entity.life.hp = pkt.hp;
		entity.life.hp_max = pkt.maxHP;
		entity.life.sp = pkt.sp;
		entity.life.sp_max = pkt.maxSP;
		entity.life.update();
		EntityManager.storeLife(pkt.AID, {
			hp: pkt.hp,
			hp_max: pkt.maxHP,
			sp: pkt.sp,
			sp_max: pkt.maxSP
		});
	}

	if (entity && entity.life.display) {
		pkt.life = entity.life;
	}

	// Add skill points to the packet
	pkt.SKPoint = pkt.SKPoint || 0;

	MercenaryInformations.append();
	MercenaryInformations.setInformations(pkt);
	MercenaryInformations.startAI();
}

/**
 * Update Mercenary information
 *
 * @param {object} pkt - PACKET.ZC.MER_PROPERTY
 */
function onMercenaryProperty(pkt) {
	const entity = EntityManager.get(Session.mercId);

	if (entity) {
		entity.life.hp = pkt.hp;
		entity.life.hp_max = pkt.maxHP;
		entity.life.sp = pkt.sp;
		entity.life.sp_max = pkt.maxSP;
		entity.life.update();
		EntityManager.storeLife(Session.mercId, {
			hp: pkt.hp,
			hp_max: pkt.maxHP,
			sp: pkt.sp,
			sp_max: pkt.maxSP
		});
	}

	if (entity && entity.life.display) {
		pkt.life = entity.life;
	}

	MercenaryInformations.setInformations(pkt);
}

/**
 * Update parameter
 *
 * @param {object} pkt - PACKET.ZC.MER_PAR_CHANGE
 */
function onParameterChange(pkt) {
	const entity = EntityManager.get(Session.mercId);
	if (!entity) {
		return;
	}

	switch (pkt.param) {
		case 0x0: // HP
			entity.life.hp = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.mercId, { hp: pkt.value });
			break;

		case 0x1: // SP
			entity.life.sp = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.mercId, { sp: pkt.value });
			break;

		case 0x2: // MaxHP
			entity.life.hp_max = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.mercId, { hp_max: pkt.value });
			break;

		case 0x3: // MaxSP
			entity.life.sp_max = pkt.value;
			entity.life.update();
			EntityManager.storeLife(Session.mercId, { sp_max: pkt.value });
			break;
	}
}

/**
 * Get mercenary skills
 *
 * @param {object} pkt - PACKET.ZC.MER_SKILLINFO_LIST
 */
function onSkillList(pkt) {
	SkillListMH.mercenary.setSkills(pkt.skillList);
}

/**
 * Update skill
 *
 * @param {object} pkt - PACKET.ZC.MER_SKILLINFO_UPDATE
 */
function onSkillUpdate(pkt) {
	SkillListMH.mercenary.updateSkill(pkt);
}

/**
 * Request to delete mercenary
 */
MercenaryInformations.reqDeleteMercenary = function reqDeleteMercenary() {
	UIManager.showPromptBox(DB.getMessage(356), 'ok', 'cancel', function () {
		const pkt = new PACKET.CZ.MER_COMMAND();
		pkt.command = 2;
		Network.sendPacket(pkt);
		MercenaryInformations.ui.hide();
		SkillListMH.mercenary.ui.hide();
	});
};

/**
 * Request to move mercenary to owner
 */
MercenaryInformations.reqMoveToOwner = function reqMoveToOwner(gid) {
	const pkt = new PACKET.CZ.REQUEST_MOVETOOWNER();
	pkt.GID = gid;
	Network.sendPacket(pkt);
};

/**
 * Request mercenary to attack target
 */
MercenaryInformations.reqAttack = function reqAttack(GID, targetGID) {
	const pkt = new PACKET.CZ.REQUEST_ACTNPC();
	pkt.GID = GID;
	pkt.targetGID = targetGID;
	pkt.action = 0;
	Network.sendPacket(pkt);
};

/**
 * Request mercenary to move to location
 */
MercenaryInformations.reqMoveTo = function reqMoveTo(GID, x = 0, y = 0) {
	const pkt = new PACKET.CZ.REQUEST_MOVENPC();
	pkt.GID = GID;
	pkt.dest[0] = x > 0 ? x : Mouse.world.x;
	pkt.dest[1] = y > 0 ? y : Mouse.world.y;
	Network.sendPacket(pkt);
};

/**
 * Initialize
 */
export default function MercenaryEngine() {
	Network.hookPacket(PACKET.ZC.MER_INIT, onMercenaryInit);
	Network.hookPacket(PACKET.ZC.MER_PROPERTY, onMercenaryProperty);
	Network.hookPacket(PACKET.ZC.MER_PAR_CHANGE, onParameterChange);
	Network.hookPacket(PACKET.ZC.MER_SKILLINFO_LIST, onSkillList);
	Network.hookPacket(PACKET.ZC.MER_SKILLINFO_UPDATE, onSkillUpdate);
}
