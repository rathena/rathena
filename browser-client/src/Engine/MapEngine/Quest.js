/**
 * Engine/MapEngine/Quest.js
 *
 * Manage Quest packets and UI
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
import Quest from 'UI/Components/Quest/Quest.js';
import { sanitizeHtml } from 'Utils/HtmlHelper.js';

/**
 * Quest List
 *
 * @param {object} pkt - PACKET.ZC.ALL_QUEST_LIST_V4
 */
function onAllQuestList(pkt) {
	const quest_list = [];
	for (let i = 0; i < pkt.questCount; i++) {
		const quest = pkt.QuestList[i];
		const quest_info = DB.getQuestInfo(quest.questID);
		const local_quest = {
			questID: quest.questID,
			title: quest_info.Title || '',
			summary: quest_info.Summary || '',
			description: quest_info.Description || '',
			icon: quest_info.IconName || 'ico_nq.bmp',
			npc_spr: quest_info.NpcSpr || null,
			npc_navi: quest_info.NpcNavi || null,
			npc_pos_x: quest_info.NpcPosX || null,
			npc_pos_y: quest_info.NpcPosY || null,
			reward_item_list: quest_info.RewardItemList || [],
			reward_exp_base: quest_info.RewardEXP || 0,
			reward_exp_job: quest_info.RewardJEXP || 0,
			active: quest.active,
			start_time: quest.quest_svrTime || 0,
			end_time: quest.quest_endTime || 0,
			count: quest.count,
			hunt_list: []
		};
		if (local_quest.count > 0) {
			for (let j = 0; j < local_quest.count; j++) {
				const hunt = quest.hunt[j];
				const local_hunt = {
					huntID: hunt.huntID || null,
					huntIDCount: hunt.huntIDCount || 0,
					mobType: hunt.mobType || null,
					mobGID: hunt.mobGID || null,
					lvlMin: hunt.lvlMin || null,
					lvlMax: hunt.lvlMax || null,
					huntCount: hunt.huntCount || 0,
					maxCount: hunt.maxCount || 0,
					mobName: hunt.mobName || ''
				};
				const ID = hunt.huntID ? hunt.huntID : hunt.mobGID;
				local_quest.hunt_list[ID] = local_hunt; // prefer huntid over the mobGID
			}
		}
		quest_list[local_quest.questID] = local_quest;
	}
	Quest.getUI().setQuestList(quest_list);
}

/**
 * Quest added
 *
 * @param {object} pkt - PACKET.ZC.ADD_QUEST3
 */
function onAddQuest(pkt) {
	const quest_info = DB.getQuestInfo(pkt.questID);
	const quest = {
		questID: pkt.questID,
		title: quest_info.Title || '',
		summary: quest_info.Summary || '',
		description: quest_info.Description || '',
		icon: quest_info.IconName || 'ico_nq.bmp',
		npc_spr: quest_info.NpcSpr || null,
		npc_navi: quest_info.NpcNavi || null,
		npc_pos_x: quest_info.NpcPosX || null,
		npc_pos_y: quest_info.NpcPosY || null,
		reward_item_list: quest_info.RewardItemList || [],
		reward_exp_base: quest_info.RewardEXP || 0,
		reward_exp_job: quest_info.RewardJEXP || 0,
		active: pkt.active || 1,
		start_time: pkt.quest_svrTime || null,
		end_time: pkt.quest_endTime || null,
		count: pkt.count,
		hunt_list: []
	};
	if (quest.count > 0) {
		for (let i = 0; i < quest.count; i++) {
			const hunt = pkt.hunt[i];
			const local_hunt = {
				huntID: hunt.huntID || null,
				huntIDCount: hunt.huntIDCount || 0,
				mobType: hunt.mobType || null,
				mobGID: hunt.mobGID || null,
				lvlMin: hunt.lvlMin || null,
				lvlMax: hunt.lvlMax || null,
				huntCount: hunt.huntCount || 0,
				maxCount: hunt.maxCount || 0,
				mobName: hunt.mobName || ''
			};
			const ID = hunt.huntID ? hunt.huntID : hunt.mobGID;
			quest.hunt_list[ID] = local_hunt; // prefer huntid over the mobGID
		}
	}
	Quest.getUI().addQuest(quest, quest.questID);
}

/**
 * Quest Hunt updated
 *
 * @param {object} pkt - PACKET.ZC.UPDATE_MISSION_HUNT4
 */
function onUpdateMissionHunt(pkt) {
	for (let i = 0; i < pkt.questCount; i++) {
		const local_hunt = pkt.hunt[i];
		const ID = local_hunt.huntID ? local_hunt.huntID : local_hunt.mobGID;

		if (local_hunt.questID !== undefined) {
			// server sent info with questID
			if (Quest.getUI().questExists(local_hunt.questID)) {
				Quest.getUI().updateMissionHunt(local_hunt, local_hunt.questID, ID);
			} else {
				// create new one
				const quest_info = DB.getQuestInfo(local_hunt.questID);
				const local_quest = {
					questID: local_hunt.questID,
					title: quest_info.Title ? sanitizeHtml(quest_info.Title) : '',
					summary: quest_info.Summary ? sanitizeHtml(quest_info.Summary) : '',
					description: quest_info.Description ? sanitizeHtml(quest_info.Description) : '',
					icon: quest_info.IconName ? quest_info.IconName : 'ico_nq.bmp',
					npc_spr: quest_info.NpcSpr || null,
					npc_navi: quest_info.NpcNavi || null,
					npc_pos_x: quest_info.NpcPosX || null,
					npc_pos_y: quest_info.NpcPosY || null,
					reward_item_list: quest_info.RewardItemList || [],
					reward_exp_base: quest_info.RewardEXP || 0,
					reward_exp_job: quest_info.RewardJEXP || 0,
					active: 1,
					start_time: null,
					end_time: null,
					count: 1,
					hunt_list: []
				};
				local_quest.hunt_list[ID] = {
					huntID: local_hunt.huntID || null,
					huntIDCount: local_hunt.huntIDCount || 0,
					mobType: local_hunt.mobType || null,
					mobGID: local_hunt.mobGID || null,
					lvlMin: local_hunt.lvlMin || null,
					lvlMax: local_hunt.lvlMax || null,
					huntCount: local_hunt.huntCount || 0,
					maxCount: local_hunt.maxCount || 0,
					mobName: local_hunt.mobName || ''
				};
				Quest.getUI().addQuest(local_quest, local_quest.questID);
			}
		} else {
			// server sent info with huntID
			const quest_saved_id = Quest.getUI().getQuestIDByServerID(ID);
			if (quest_saved_id > 0) {
				// update quest
				Quest.getUI().updateMissionHunt(local_hunt, quest_saved_id, ID);
			}
		}
	}
}

/**
 * Quest actived or disabled
 *
 * @param {object} pkt - PACKET.ZC.ACTIVE_QUEST
 */
function onActiveQuest(pkt) {
	Quest.getUI().toggleQuestActive(pkt.questID, pkt.active);
}

/**
 * Quest deleted
 *
 * @param {object} pkt - PACKET.ZC.DEL_QUEST
 */
function onDeleteQuest(pkt) {
	Quest.getUI().removeQuest(pkt.questID);
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.ALL_QUEST_LIST, onAllQuestList);
	Network.hookPacket(PACKET.ZC.ALL_QUEST_MISSION, onAllQuestList);
	Network.hookPacket(PACKET.ZC.ALL_QUEST_LIST_V2, onAllQuestList);
	Network.hookPacket(PACKET.ZC.ALL_QUEST_LIST_V3, onAllQuestList);
	Network.hookPacket(PACKET.ZC.ALL_QUEST_LIST_V4, onAllQuestList);
	Network.hookPacket(PACKET.ZC.ADD_QUEST, onAddQuest);
	Network.hookPacket(PACKET.ZC.ADD_QUEST2, onAddQuest);
	Network.hookPacket(PACKET.ZC.ADD_QUEST3, onAddQuest);
	Network.hookPacket(PACKET.ZC.UPDATE_MISSION_HUNT, onUpdateMissionHunt);
	Network.hookPacket(PACKET.ZC.UPDATE_MISSION_HUNT2, onUpdateMissionHunt);
	Network.hookPacket(PACKET.ZC.UPDATE_MISSION_HUNT3, onUpdateMissionHunt);
	Network.hookPacket(PACKET.ZC.UPDATE_MISSION_HUNT4, onUpdateMissionHunt);
	Network.hookPacket(PACKET.ZC.ACTIVE_QUEST, onActiveQuest);
	Network.hookPacket(PACKET.ZC.DEL_QUEST, onDeleteQuest);
}
