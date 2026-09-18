/**
 * Engine/MapEngine/Pet.js
 *
 * Manage Pets
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
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import EntityManager from 'Renderer/EntityManager.js';
import UIManager from 'UI/UIManager.js';
import SlotMachine from 'UI/Components/SlotMachine/SlotMachine.js';
import SkillTargetSelection from 'UI/Components/SkillTargetSelection/SkillTargetSelection.js';
import ItemSelection from 'UI/Components/ItemSelection/ItemSelection.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import PetInformations from 'UI/Components/PetInformations/PetInformations.js';
import Emotions from 'DB/Emotions.js';
import PetMessageConst from 'DB/Pets/PetMessageConst.js';
import PetEvolution from 'UI/Components/PetEvolution/PetEvolution.js';

/**
 * Server ask to select a monster
 *
 * @param {object} pkt - PACKET.ZC.START_CAPTURE
 */
function onStartCapture(pkt) {
	const fakeSkill = { SKID: -10, level: 0 };

	SkillTargetSelection.append();
	SkillTargetSelection.set(fakeSkill, SkillTargetSelection.TYPE.PET, 'Capture Monster');
	SkillTargetSelection.onPetSelected = function onPetSelected(gid) {
		SlotMachine.append();
		SlotMachine.onTry = function onTry() {
			const _pkt = new PACKET.CZ.TRYCAPTURE_MONSTER();
			_pkt.targetAID = gid;
			Network.sendPacket(_pkt);
		};
	};
}

/**
 * Received capture result from server
 *
 * @param {object} pkt - PACKET.ZC.TRYCAPTURE_MONSTER
 */
function onCaptureResult(pkt) {
	SlotMachine.setResult(pkt.result);
}

/**
 * Get pet list from server
 *
 * @param {object} pkt - PACKET.ZC.PETEGG_LIST
 */
function onPetList(pkt) {
	if (!pkt.eggList.length) {
		return;
	}

	ItemSelection.append();
	ItemSelection.setList(pkt.eggList);
	ItemSelection.setTitle(DB.getMessage(599));
	ItemSelection.onIndexSelected = function (index) {
		if (index > -1) {
			const _pkt = new PACKET.CZ.SELECT_PETEGG();
			_pkt.index = index;
			Network.sendPacket(_pkt);
		}
	};
}

/**
 * Get own pet information from server
 *
 * @param {object} pkt - PACKET.ZC.PROPERTY_PET
 */
function onPetInformation(pkt) {
	PetInformations.append();
	PetInformations.setInformations(pkt);

	if (Session.petId) {
		const entity = EntityManager.get(Session.petId);
		if (entity) {
			const oldHungry = Session.pet.hungry || pkt.nFullness;
			Session.pet.job = pkt.job;
			Session.pet.clevel = pkt.nLevel;
			Session.pet.name = pkt.szName;
			Session.pet.friendly = pkt.nRelationship;
			Session.pet.oldHungry = oldHungry;
			Session.pet.hungry = pkt.nFullness;

			entity.display.name = pkt.szName;
			entity.life.intimacy = pkt.nRelationship;
			entity.life.hp = pkt.nFullness;
			entity.life.hp_max = 100;
			entity.life.update();
		}
	}
}

/**
 * Result from feeding your pet
 *
 * @param {object} pkt - PACKET.ZC.FEED_PET
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
	} else {
		const friendly = DB.getPetFriendlyState(Session.pet.friendly);
		const hunger = DB.getPetHungryState(Session.pet.oldHungry);
		const talk = DB.getPetTalkNumber(Session.pet.job, PetMessageConst.PM_FEEDING, hunger, Session.Sex);
		const emotion = DB.getPetEmotion(hunger, friendly, PetMessageConst.PM_FEEDING);

		if (emotion > 0) {
			const _pkt = new PACKET.CZ.PET_ACT();
			_pkt.data = emotion + '2'; // don't know what is the last digit but it needed. @MrUnzO
			Network.sendPacket(_pkt);
		}
		if (Session.pet.friendly > 900) {
			const _pkt = new PACKET.CZ.PET_ACT();
			_pkt.data = talk;
			Network.sendPacket(_pkt);
		}
	}
}

/**
 * Pet talk (don't know where to put this) (MrUnzO)
 *
 */
function petTalk(GID, msg) {
	const entity = EntityManager.get(GID);
	ChatBox.addText(entity.display.name + ' : ' + msg, ChatBox.TYPE.PUBLIC, ChatBox.FILTER.PUBLIC_CHAT);
	entity.dialog.set(msg);
}

/**
 * Update pet information
 *
 * @param {object} pkt - PACKET.ZC.CHANGESTATE_PET
 */
function onPetInformationUpdate(pkt) {
	const entity = EntityManager.get(pkt.GID);
	let path;

	if (!entity) {
		return;
	}

	switch (pkt.type) {
		case 0: // know what our pet is //PET_PRE_INIT
			Session.petId = pkt.GID; // should we delete it later ?
			Session.pet.GID = pkt.GID;
			Session.pet.job = entity._job;
			break;

		case 1: //
			PetInformations.setIntimacy(pkt.data);
			Session.pet.friendly = pkt.data;
			break;

		case 2: {
			PetInformations.setHunger(pkt.data);
			entity.life.hp = pkt.data;
			entity.life.hp_max = 100;
			entity.life.update();

			const oldHungry = Session.pet.hungry;
			Session.pet.oldHungry = oldHungry;
			Session.pet.hungry = pkt.data;

			break;
		}
		case 3: /// 3 = accessory ID
			if (pkt.data) {
				path = DB.getPetEquipPath(pkt.data);
			}

			if (!path && entity.files.body.spr) {
				path = entity.files.body.spr.replace(/\.spr$/i, '.act');
			}

			if (path) {
				Client.loadFile(path, function () {
					entity.files.body.act = path;
				});
			}
			break;

		case 4: {
			/// 4 = performance (data = 1~3: normal, 4: special)
			const action = [entity.ACTION.PERF1, entity.ACTION.PERF2, entity.ACTION.PERF3, entity.ACTION.SPECIAL];
			entity.setAction({
				action: action[(pkt.data - 1 + action.length) % action.length],
				frame: 0,
				repeat: false,
				play: true,
				next: {
					action: entity.ACTION.IDLE,
					frame: 0,
					repeat: true,
					play: true,
					next: false
				}
			});
			break;
		}
		case 5: /// 5 = accessory
	}
}

/**
 * Perform a pet action
 *
 * @param {object} pkt - PACKET.ZC.PET_ACT
 */
function onPetAction(pkt) {
	const entity = EntityManager.get(pkt.GID);
	if (!entity) {
		return;
	}
	if (pkt.data < 5000) {
		const emotionId = parseInt(pkt.data / 10);
		entity.attachments.add({
			frame: Emotions.indexes[emotionId],
			file: 'emotion',
			play: true,
			head: true,
			repeat: false,
			depth: 5.0
		});
	} else {
		const text = DB.getPetTalk(pkt.data);
		if (text) {
			petTalk(pkt.GID, text);
		}
	}
}

/**
 * Client request to feed QPet.
 */
PetInformations.reqPetFeed = function reqPetFeed() {
	// Are you sure you want to feed your pet ?
	UIManager.showPromptBox(DB.getMessage(601), 'ok', 'cancel', () => {
		const pkt = new PACKET.CZ.COMMAND_PET();
		pkt.cSub = 1;
		Network.sendPacket(pkt);
	});
};

/**
 * Client request to do a performance
 */
PetInformations.reqPetAction = function reqPetAction() {
	const pkt = new PACKET.CZ.COMMAND_PET();
	pkt.cSub = 2;
	Network.sendPacket(pkt);
};

/**
 * Qpet -> Egg
 */
PetInformations.reqBackToEgg = function reqBackToEgg() {
	const pkt = new PACKET.CZ.COMMAND_PET();
	pkt.cSub = 3;
	Network.sendPacket(pkt);

	PetInformations.remove();
};

/**
 * UnEquip pet accessory
 */
PetInformations.reqUnEquipPet = function reqUnEquipPet() {
	const pkt = new PACKET.CZ.COMMAND_PET();
	pkt.cSub = 4;
	Network.sendPacket(pkt);
};

/**
 * Rename QPet
 *
 * @param {string} new pet name
 */
PetInformations.reqNameEdit = function reqNameEdit(name) {
	const pkt = new PACKET.CZ.RENAME_PET();
	pkt.szName = name;
	Network.sendPacket(pkt);
};

/// PACKET.CZ.PETEGG_INFO | Not implemented client side ?
/// PACKET.CZ.COMMAND_PET | 0 = pet information ??? Already sent by server ?
/// PACKET.CZ.PET_ACT     | not needed.

/**
 * Request a pet evolution
 * @param {number} baseJobID - The Job ID of the base pet
 * @throws {Error} If the pet's friendly level is less than 1000
 */
PetInformations.reqEvolution = function reqEvolution(baseJobID) {
	if (Session.pet.friendly < 1000) {
		ChatBox.addText(DB.getMessage(2576), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	} else {
		// Call Evolution UI
		PetEvolution.prepare();
		PetEvolution.SetInfo(baseJobID);
		PetEvolution.append();
		PetEvolution.ui.show();
		if (PetInformations.ui) {
			PetInformations.ui.hide();
		}
	}
};

/**
 * Initialize
 */
export default function NPCEngine() {
	Network.hookPacket(PACKET.ZC.START_CAPTURE, onStartCapture);
	Network.hookPacket(PACKET.ZC.TRYCAPTURE_MONSTER, onCaptureResult);
	Network.hookPacket(PACKET.ZC.PETEGG_LIST, onPetList);
	Network.hookPacket(PACKET.ZC.PROPERTY_PET, onPetInformation);
	Network.hookPacket(PACKET.ZC.FEED_PET, onFeedResult);
	Network.hookPacket(PACKET.ZC.CHANGESTATE_PET, onPetInformationUpdate);
	Network.hookPacket(PACKET.ZC.PET_ACT, onPetAction);
}
