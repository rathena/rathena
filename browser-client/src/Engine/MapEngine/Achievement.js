/**
 * Engine/MapEngine/Achievement.js
 *
 * Manage Achievement packets and UI
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
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import Announce from 'UI/Components/Announce/Announce.js';
import 'UI/Components/Achievement/Achievement.js';
import EffectConst from 'DB/Effects/EffectConst.js';
import EffectManager from 'Renderer/EffectManager.js';

function initSessionAchievement() {
	if (!Session.Achievement) {
		Session.Achievement = {
			total_achievements: 0,
			total_points: 0,
			rank: 0,
			current_rank_points: 0,
			next_rank_points: 0,
			list: {}
		};
	}
}

function onAllAchievementList(pkt) {
	initSessionAchievement();

	Session.Achievement.total_achievements = pkt.total_achievements;
	Session.Achievement.total_points = pkt.total_points;
	Session.Achievement.rank = pkt.rank;
	Session.Achievement.current_rank_points = pkt.current_rank_points;
	Session.Achievement.next_rank_points = pkt.next_rank_points;

	const achTable = DB.getAchievementTable();

	pkt.ach_list.forEach(ach => {
		// Attach the static information from the DB (e.g. title, summary, reward, score)
		ach.info = achTable[ach.ach_id] || null;
		Session.Achievement.list[ach.ach_id] = ach;
	});

	const ui = UIManager.components.Achievement;
	if (ui) ui.updateHeaderAndView();
}

function onAchievementUpdate(pkt) {
	initSessionAchievement();

	Session.Achievement.total_points = pkt.total_points;
	Session.Achievement.rank = pkt.rank;
	Session.Achievement.current_rank_points = pkt.current_rank_points;
	Session.Achievement.next_rank_points = pkt.next_rank_points;

	const achTable = DB.getAchievementTable();

	pkt.ach_list.forEach(ach => {
		// Update the specific achievement, keeping or setting the static DB info
		ach.info = achTable[ach.ach_id] || null;

		const oldAch = Session.Achievement.list[ach.ach_id];
		const isNewCompletion = ach.completed && (!oldAch || !oldAch.completed);

		Session.Achievement.list[ach.ach_id] = ach;

		if (isNewCompletion && ach.info && ach.info.title) {
			Announce.append();
			Announce.set(`${DB.getMessage(2681).replace('%s', ach.info.title)}`, '#FFFFFF', {
				width: '100%',
				height: 50,
				fontSize: 20,
				life: 10000
			});

			const EF_Init_Par = {
				effectId: EffectConst.EF_ACH_COMPLETE,
				ownerAID: Session.AID
			};

			EffectManager.spam(EF_Init_Par);
		}
	});

	const ui = UIManager.components.Achievement;
	if (ui) ui.updateHeaderAndView();
}

function onRequestAchievementRewardACK(pkt) {
	// If the reward request was successful (failed === 0), mark it as claimed
	if (pkt.failed === 0 && Session.Achievement && Session.Achievement.list[pkt.ach_id]) {
		Session.Achievement.list[pkt.ach_id].reward = 1;
	}

	const ui = UIManager.components.Achievement;
	if (ui) ui.updateHeaderAndView();
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.ALL_ACH_LIST, onAllAchievementList);
	Network.hookPacket(PACKET.ZC.ACH_UPDATE, onAchievementUpdate);
	Network.hookPacket(PACKET.ZC.REQ_ACH_REWARD_ACK, onRequestAchievementRewardACK);
}
