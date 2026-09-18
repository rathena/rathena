/**
 * Engine/MapEngine.js
 *
 * Map Engine
 * Manage Map server
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import SoundManager from 'Audio/SoundManager.js';
import BGM from 'Audio/BGM.js';
import Events from 'Core/Events.js';
import Session from 'Engine/SessionStorage.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import Renderer from 'Renderer/Renderer.js';
import Camera from 'Renderer/Camera.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import EntityManager from 'Renderer/EntityManager.js';
import Entity from 'Renderer/Entity/Entity.js';
import Altitude from 'Renderer/Map/Altitude.js';
import MapControl from 'Controls/MapControl.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import EffectManager from 'Renderer/EffectManager.js';
import Escape from 'UI/Components/Escape/Escape.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ChatBoxSettings from 'UI/Components/ChatBoxSettings/ChatBoxSettings.js';
import StatusConst from 'DB/Status/StatusState.js';
import CheckAttendance from 'UI/Components/CheckAttendance/CheckAttendance.js';
import WinStats from 'UI/Components/WinStats/WinStats.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import Storage from 'UI/Components/Storage/Storage.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import Vending from 'UI/Components/Vending/Vending.js';
import VendingReport from 'UI/Components/VendingReport/VendingReport.js';
import ChangeCart from 'UI/Components/ChangeCart/ChangeCart.js';
import CartDecoration from 'UI/Components/CartDecoration/CartDecoration.js';
import ShortCut from 'UI/Components/ShortCut/ShortCut.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import SwitchEquip from 'UI/Components/SwitchEquip/SwitchEquip.js';
import ShortCuts from 'UI/Components/ShortCuts/ShortCuts.js';
import StatusIcons from 'UI/Components/StatusIcons/StatusIcons.js';
import ChatRoomCreate from 'UI/Components/ChatRoomCreate/ChatRoomCreate.js';
import Emoticons from 'UI/Components/Emoticons/Emoticons.js';
import FPS from 'UI/Components/FPS/FPS.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';
import Guild from 'UI/Components/Guild/Guild.js';
import WorldMap from 'UI/Components/WorldMap/WorldMap.js';
import SkillListMH from 'UI/Components/SkillListMH/SkillListMH.js';
import MobileUI from 'UI/Components/MobileUI/MobileUI.js';
import CashShop from 'UI/Components/CashShop/CashShop.js';
import Bank from 'UI/Components/Bank/Bank.js';
import ItemReform from 'UI/Components/ItemReform/ItemReform.js';
import LaphineSys from 'UI/Components/LaphineSys/LaphineSys.js';
import LaphineUpg from 'UI/Components/LaphineUpg/LaphineUpg.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import RodexIcon from 'UI/Components/Rodex/RodexIcon.js';
import Roulette from 'UI/Components/Roulette/Roulette.js';
import PCGoldTimer from 'UI/Components/PCGoldTimer/PCGoldTimer.js';
import Refine from 'UI/Components/Refine/Refine.js';
import Reputation from 'UI/Components/Reputation/Reputation.js';
import PetInformations from 'UI/Components/PetInformations/PetInformations.js';
import HomunInformations from 'UI/Components/HomunInformations/HomunInformations.js';
import MapName from 'UI/Components/MapName/MapName.js';
import Announce from 'UI/Components/Announce/Announce.js';
import Navigation from 'UI/Components/Navigation/Navigation.js';
import CaptchaUpload from 'UI/Components/Captcha/CaptchaUpload.js';
import CaptchaSelector from 'UI/Components/Captcha/CaptchaSelector.js';
import CaptchaAnswer from 'UI/Components/Captcha/CaptchaAnswer.js';
import CaptchaPreview from 'UI/Components/Captcha/CaptchaPreview.js';
import Clan from 'UI/Components/Clan/Clan.js';
import WhisperBox from 'UI/Components/WhisperBox/WhisperBox.js';
import PluginManager from 'Plugins/PluginManager.js';
import SignboardManager from 'Renderer/SignboardManager.js';
import PvPTimer from 'UI/Components/PvPTimer/PvPTimer.js';
import PvPCount from 'UI/Components/PvPCount/PvPCount.js';
import BasicInfo from 'UI/Components/BasicInfo/BasicInfo.js';
import MiniMap from 'UI/Components/MiniMap/MiniMap.js';
import SkillList from 'UI/Components/SkillList/SkillList.js';
import Quest from 'UI/Components/Quest/Quest.js';
import PlayerViewEquip from 'UI/Components/PlayerViewEquip/PlayerViewEquip.js';
import JoystickUI from 'UI/Components/JoystickUI/JoystickUI.js';
import CashShopIcon from 'UI/Components/CashShopIcon/CashShopIcon.js';
import Achievement from 'UI/Components/Achievement/Achievement.js';

import MainEngine from './MapEngine/Main.js';
import MapStateEngine from './MapEngine/MapState.js';
import NPCEngine from './MapEngine/NPC.js';
import EntityEngine from './MapEngine/Entity.js';
import ItemEngine from './MapEngine/Item.js';
import MailEngine from './MapEngine/Mail.js';
import PrivateMessageEngine from './MapEngine/PrivateMessage.js';
import StorageEngine from './MapEngine/Storage.js';
import GroupEngine from './MapEngine/Group.js';
import GuildEngine from './MapEngine/Guild.js';
import SkillEngine from './MapEngine/Skill.js';
import ChatRoomEngine from './MapEngine/ChatRoom.js';
import PetEngine from './MapEngine/Pet.js';
import HomunEngine from './MapEngine/Homun.js';
import MercenaryEngine from './MapEngine/Mercenary.js';
import StoreEngine from './MapEngine/Store.js';
import TradeEngine from './MapEngine/Trade.js';
import FriendsEngine from './MapEngine/Friends.js';
import UIOpenEngine from './MapEngine/UIOpen.js';
import QuestEngine from './MapEngine/Quest.js';
import RodexEngine from './MapEngine/Rodex.js';
import RouletteEngine from './MapEngine/Roulette.js';
import PCGoldTimerEngine from './MapEngine/PCGoldTimer.js';
import CaptchaEngine from './MapEngine/Captcha.js';
import ClanEngine from './MapEngine/Clan.js';
import CashShopEngine from './MapEngine/CashShop.js';
import BankEngine from './MapEngine/Bank.js';
import AchievementEngine from './MapEngine/Achievement.js';

/**
 * @type {string} mapname
 */
let _mapName = '';

/**
 * @type {boolean} is initialized
 */
let _isInitialised = false;

let _exiting = false;
let _exitTimer = null;

let snCounter = 0;
let chatLines = 0;

const packetMap = new Map();
/**
 * @namespace MapEngine
 */
class MapEngine {
	/**
	 * @type {boolean} do we need to update UI versions?
	 */
	static needsUIVerUpdate = false;

	/**
	 * Connect to Map Server
	 *
	 * @param {number} IP
	 * @param {number} port
	 * @param {string} mapName
	 */
	static init(ip, port, mapName) {
		_mapName = mapName;
		_exiting = false;
		_exitTimer = null;

		// Connect to char server
		const forceAddress = Configs.get('forceUseAddress');
		const server_info = Configs.getServer();
		const current_ip = forceAddress ? server_info.address : Network.utils.longToIP(ip);
		Network.connect(
			current_ip,
			port,
			success => {
				// Force reloading map
				MapRenderer.currentMap = '';

				// Fail to connect...
				if (!success) {
					UIManager.showErrorBox(DB.getMessage(1));
					return;
				}

				// Success, try to login.
				let pkt;
				if (PACKETVER.value >= 20180307) {
					pkt = new PACKET.CZ.ENTER2();
				} else {
					pkt = new PACKET.CZ.ENTER();
				}
				pkt.AID = Session.AID;
				pkt.GID = Session.GID;
				pkt.AuthCode = Session.AuthCode;
				pkt.clientTime = Date.now();
				pkt.Sex = Session.Sex;
				Network.sendPacket(pkt);

				// Server send back AID
				Network.read(fp => {
					// PACKETVER < 20070521: the map-server prefixes the stream with a raw
					// 4-byte account id (no packet header) before the first real packet.
					// It must be consumed even when TCP coalesces it with following packets;
					// the previous fp.length === 4 check only worked when the AID arrived in
					// its own segment, otherwise the parser read the AID as an opcode and
					// desynced the whole stream.
					if (PACKETVER.value < 20070521) {
						Session.AID = fp.readLong();
						Session.Entity.GID = Session.AID;
					}
				});

				const hbt = new PACKET.CZ.HBT();
				const is_sec_hbt = Configs.get('sec_HBT', null);

				// Ping
				let ping;
				const SP = Session.ping;

				if (PACKETVER.value >= 20180307) {
					ping = new PACKET.CZ.REQUEST_TIME2();
				} else {
					ping = new PACKET.CZ.REQUEST_TIME();
				}
				const startTick = Date.now();
				Network.setPing(() => {
					if (is_sec_hbt) {
						Network.sendPacket(hbt);
					}

					ping.clientTime = Date.now() - startTick;

					if (!SP.returned && SP.pingTime) {
						console.warn('[Network] The server did not answer the previous PING!');
					}
					SP.pingTime = ping.clientTime;
					SP.returned = false;

					Network.sendPacket(ping);
				});

				Session.Playing = true;
			},
			true
		);

		// Select UI version when needed
		if (MapEngine.needsUIVerUpdate || !_isInitialised) {
			if (PACKETVER.value < 20200520) {
				BasicInfo.selectUIVersion();
			}
			MiniMap.selectUIVersion();
			SkillList.selectUIVersion();
			Quest.selectUIVersion();
			Equipment.selectUIVersion();
			PlayerViewEquip.selectUIVersion();
			WinStats.selectUIVersion();
			Inventory.selectUIVersion();
			Storage.selectUIVersion();
			PartyFriends.selectUIVersion();
		}

		// Do not hook multiple time
		if (!_isInitialised) {
			_isInitialised = true;

			MapControl.init();
			MapControl.onRequestWalk = onRequestWalk;
			MapControl.onRequestStopWalk = onRequestStopWalk;
			MapControl.onRequestDropItem = onDropItem;

			// Hook packets
			Network.hookPacket(PACKET.ZC.AID, onReceiveAccountID);
			Network.hookPacket(PACKET.ZC.ACCEPT_ENTER, onConnectionAccepted);
			Network.hookPacket(PACKET.ZC.ACCEPT_ENTER2, onConnectionAccepted);
			Network.hookPacket(PACKET.ZC.ACCEPT_ENTER3, onConnectionAccepted);
			Network.hookPacket(PACKET.ZC.NPCACK_MAPMOVE, onMapChange);
			Network.hookPacket(PACKET.ZC.NPCACK_SERVERMOVE, onServerChange);
			Network.hookPacket(PACKET.ZC.NPCACK_SERVERMOVE2, onServerChange);
			Network.hookPacket(PACKET.ZC.ACCEPT_QUIT, onExitSuccess);
			Network.hookPacket(PACKET.ZC.REFUSE_QUIT, onExitFail);
			Network.hookPacket(PACKET.ZC.RESTART_ACK, onRestartAnswer);
			Network.hookPacket(PACKET.ZC.ACK_REQ_DISCONNECT, onDisconnectAnswer);
			Network.hookPacket(PACKET.ZC.NOTIFY_TIME, onPong);
			Network.hookPacket(PACKET.ZC.PING_LIVE, onPingLive);
			Network.hookPacket(PACKET.ZC.CONFIG_NOTIFY, onConfigNotify);
			Network.hookPacket(PACKET.ZC.CONFIG_NOTIFY2, onConfigNotify);
			Network.hookPacket(PACKET.ZC.CONFIG_NOTIFY3, onConfigNotify);
			Network.hookPacket(PACKET.ZC.CONFIG_NOTIFY4, onConfigNotify);
			Network.hookPacket(PACKET.ZC.CONFIG, onConfig);
			Network.hookPacket(PACKET.ZC.REFUSE_ENTER, onConnectionRefused);

			// hook reassembly packets and map the responses
			for (let i = 1; i <= 42; i++) {
				const id = String(i).padStart(2, '0');

				const ZC = PACKET.ZC[`REASSEMBLY_AUTH${id}`];
				const CZ = PACKET.CZ[`REASSEMBLY_AUTH${id}`];

				packetMap.set(ZC, CZ);
				Network.hookPacket(ZC, onReassemblyAuth);
			}

			// Extend controller
			MainEngine();
			MapStateEngine();
			NPCEngine();
			EntityEngine();
			ItemEngine();
			MailEngine();
			PrivateMessageEngine();
			StorageEngine();
			GroupEngine.init();
			GuildEngine.init();
			SkillEngine();
			ChatRoomEngine();
			PetEngine();
			HomunEngine();
			MercenaryEngine();
			StoreEngine();
			TradeEngine();
			FriendsEngine.init();
			UIOpenEngine();
			QuestEngine();
			RodexEngine();
			RouletteEngine();
			PCGoldTimerEngine();
			CaptchaEngine();
			ClanEngine();
			if (Configs.get('enableCashShop')) {
				CashShopEngine();
			}

			if (Configs.get('enableAchievements') && PACKETVER.value >= 20150513) {
				AchievementEngine();
			}

			if (Configs.get('enableBank')) {
				BankEngine.init();
			}

			// Prepare UI
			Escape.prepare();
			PvPTimer.prepare();
			PvPCount.prepare();
			Inventory.getUI().prepare();
			CartItems.prepare();
			Vending.prepare();
			ChangeCart.prepare();
			Equipment.getUI().prepare();
			ShortCuts.prepare();
			ShortCut.prepare();
			ChatRoomCreate.prepare();
			Emoticons.prepare();
			FPS.prepare();
			PartyFriends.getUI().prepare();
			StatusIcons.prepare();
			ChatBox.prepare();
			ChatBoxSettings.prepare();
			Guild.prepare();
			WorldMap.prepare();
			SkillListMH.homunculus.prepare();
			SkillListMH.mercenary.prepare();
			Rodex.prepare();
			RodexIcon.prepare();
			Roulette.prepare();
			PCGoldTimer.prepare();
			Navigation.prepare();
			CaptchaUpload.prepare();
			CaptchaSelector.prepare();
			CaptchaAnswer.prepare();
			CaptchaPreview.prepare();
			Clan.prepare();

			if (Configs.get('enableMapName')) {
				MapName.prepare();
			}

			if (Configs.get('enableCashShop')) {
				CashShopIcon.prepare();
				CashShop.prepare();
			}

			if (Configs.get('enableBank')) {
				Bank.prepare();
			}

			if (PACKETVER.value >= 20090617) {
				WhisperBox.prepare();
				WhisperBox.init();
			}

			if (PACKETVER.value >= 20141016) {
				VendingReport.prepare();
			}

			if (PACKETVER.value >= 20160601) {
				LaphineSys.prepare();
			}

			if (PACKETVER.value >= 20170726) {
				LaphineUpg.prepare();
			}

			if (Configs.get('enableRefineUI') && PACKETVER.value >= 20161012) {
				Refine.prepare();
			}

			if (PACKETVER.value >= 20170208) {
				SwitchEquip.prepare();
				SwitchEquip.onAddSwitchEquip = onAddSwitchEquip;
				SwitchEquip.onRemoveSwitchEquip = onRemoveSwitchEquip;
			}

			if (Configs.get('enableCheckAttendance') && PACKETVER.value >= 20180307) {
				CheckAttendance.prepare();
			}

			if (PACKETVER.value >= 20200916) {
				ItemReform.prepare();
			}

			if (PACKETVER.value >= 20220330) {
				Reputation.prepare();
			}

			if (Configs.get('enableAchievements') && PACKETVER.value >= 20150513) {
				Achievement.prepare();
			}

			// Bind UI
			PetInformations.onConfigUpdate = onConfigUpdate;
			HomunInformations.onConfigUpdate = onConfigUpdate;
			Escape.onExitRequest = onExitRequest;
			Escape.onCharSelectionRequest = onRestartRequest;
			Escape.onReturnSavePointRequest = onReturnSavePointRequest;
			Escape.onResurectionRequest = onResurectionRequest;
			ChatBox.onRequestTalk = onRequestTalk;
			WhisperBox.onRequestTalk = onRequestTalk;
		}

		// Init selected UIs when needed
		if (MapEngine.needsUIVerUpdate || !_isInitialised) {
			// Prepare UIs
			MiniMap.getUI().prepare();
			SkillList.getUI().prepare();
			if (PACKETVER.value < 20200520) {
				BasicInfo.getUI().prepare();
			}
			Equipment.getUI().prepare();
			Quest.getUI().prepare();
			WinStats.getUI().prepare();
			PartyFriends.selectUIVersion();

			// Bind UIs
			WinStats.getUI().onRequestUpdate = onRequestStatUpdate;
			Equipment.getUI().onUnEquip = onUnEquip;
			Equipment.getUI().onConfigUpdate = onConfigUpdate;
			Equipment.getUI().onEquipItem = onEquipItem;
			Equipment.getUI().onRemoveOption = onRemoveOption;
			Inventory.getUI().onUseItem = onUseItem;
			Inventory.getUI().onEquipItem = onEquipItem;

			// Avoid zone server change init
			MapEngine.needsUIVerUpdate = false;
		}
	}
}

/**
 * Pong from server
 * TODO: check the time ?
 */
function onPong(pkt) {
	const SP = Session.ping;

	SP.returned = true;
	SP.pongTime = 0;
	SP.value = SP.pongTime - SP.pingTime;

	Session.serverTick = pkt.time + SP.value / 2; // Adjust with half ping
}

/**
 * Ping from server?
 */
function onPingLive(pkt) {
	const pong_pkt = new PACKET.CZ.PING_LIVE();
	Network.sendPacket(pong_pkt);
}

/**
 * Receive user config from server
 *
 * @param {object} pkt - PACKET_ZC_CONFIG
 */
function onConfig(pkt) {
	switch (pkt.Config) {
		case 0:
			Equipment.getUI().setEquipConfig(pkt.Value);
			ChatBox.addText(DB.getMessage(1358 + (pkt.Value ? 1 : 0)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 1:
			Session.Entity.call_flag = pkt.Value;
			ChatBox.addText(DB.getMessage(2978 + (pkt.Value ? 0 : 1)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2:
			PetInformations.setFeedConfig(pkt.Value);
			ChatBox.addText(DB.getMessage(2579 + (pkt.Value ? 0 : 1)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 3:
			HomunInformations.setFeedConfig(pkt.Value);
			ChatBox.addText(DB.getMessage(3282 + (pkt.Value ? 0 : 1)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 5:
			Equipment.getUI().setCostumeConfig(pkt.Value);
			break;
		default:
			console.error('[PACKET_ZC_CONFIG] Unknown Config Type %d (value:%d)', pkt.Config, pkt.Value);
	}
}

/**
 * Show some system configs
 *
 * @param {object} pkt - PACKET_ZC_CONFIG_NOTIFY
 */
function onConfigNotify(pkt) {
	if (typeof pkt.show_eq_flag !== 'undefined') {
		Equipment.getUI().setEquipConfig(pkt.show_eq_flag);
		ChatBox.addText(DB.getMessage(1358 + (pkt.show_eq_flag ? 1 : 0)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
	}
	if (typeof pkt.pet_autofeeding_flag !== 'undefined') {
		PetInformations.setFeedConfig(pkt.pet_autofeeding_flag);
		ChatBox.addText(
			DB.getMessage(2579 + (pkt.pet_autofeeding_flag ? 0 : 1)),
			ChatBox.TYPE.INFO,
			ChatBox.FILTER.PUBLIC_LOG
		);
	}
	if (typeof pkt.call_flag !== 'undefined') {
		Session.Entity.call_flag = pkt.call_flag;
		ChatBox.addText(DB.getMessage(2978 + (pkt.call_flag ? 0 : 1)), ChatBox.TYPE.INFO, ChatBox.FILTER.PUBLIC_LOG);
	}
	if (typeof pkt.homunculus_autofeeding_flag !== 'undefined') {
		HomunInformations.setFeedConfig(pkt.homunculus_autofeeding_flag);
		ChatBox.addText(
			DB.getMessage(3282 + (pkt.homunculus_autofeeding_flag ? 0 : 1)),
			ChatBox.TYPE.INFO,
			ChatBox.FILTER.PUBLIC_LOG
		);
	}
}

/**
 * Server update our account id
 *
 * @param {object} pkt - PACKET.ZC.AID
 */
function onReceiveAccountID(pkt) {
	Session.AID = pkt.AID;
	Session.Entity.GID = pkt.AID;
}

/**
 * Map accept us to enter the map
 *
 * @param {object} pkt - PACKET.ZC.ACCEPT_ENTER
 */
function onConnectionAccepted(pkt) {
	Session.Entity.onWalkEnd = onWalkEnd;

	if ('sex' in pkt && pkt.sex < 2) {
		Session.Entity.sex = pkt.sex;
	}

	// Reset
	Session.petId = 0;
	Session.hasParty = false;
	Session.isPartyLeader = false;
	Session.hasGuild = false;
	Session.guildRight = 0;

	Session.homunId = 0;

	// clevel is already populated by Entity.set() in the Player constructor

	Session.mapState = {
		property: 0,
		type: 0,
		flag: 0,
		isPVPZone: false,
		isAgitZone: false,
		isPVP: false,
		isGVG: false,
		isSiege: false,
		isNoLockOn: false,
		showPVPCounter: false,
		showBFCounter: false,
		isBattleField: false
	};

	if (PACKETVER.value >= 20200520) {
		BasicInfo.selectUIVersionWithJob(DB.getJobClass(Session.Entity.job));
		BasicInfo.getUI().prepare();
	}

	BasicInfo.getUI().update('blvl', Session.Entity.clevel);
	BasicInfo.getUI().update('jlvl', Session.Entity.joblevel);
	BasicInfo.getUI().update('zeny', Session.Entity.money);
	BasicInfo.getUI().update('name', Session.Entity.display.name);
	BasicInfo.getUI().update('job', Session.Entity.job);

	// Fix http://forum.robrowser.com/?topic=32177.0
	onMapChange({
		xPos: pkt.PosDir[0],
		yPos: pkt.PosDir[1],
		mapName: _mapName
	});
}

/**
 * onConnectionRefused
 *
 * @param {object} pkt - PACKET.ZC.REFUSE_ENTER
 */
function onConnectionRefused(pkt) {
	UIManager.showErrorBox(DB.getMessage(9)); // MSI_ACCESS_DENIED = Rejected from Server.
}

/**
 * Changing map, loading new map
 *
 * @param {object} pkt - PACKET.ZC.NPCACK_MAPMOVE
 */
function onMapChange(pkt) {
	MapRenderer.onLoad = () => {
		Session.Entity.set({
			PosDir: [pkt.xPos, pkt.yPos, 0],
			// Use Session.AID rather than Session.Entity.GID here:
			// EntityManager removes Session.Entity during map transition, which
			// triggers Entity.clean() and sets this.GID = -1. Reading the GID
			// back from the entity at this point would produce -1.
			// Session.AID (account ID) equals the player's entity GID on the
			// map server and is never mutated by entity cleanup.
			GID: Session.AID
		});
		EntityManager.add(Session.Entity);
		if (Session.Entity.effectState & StatusConst.EffectState.FALCON) {
			if (!Session.Entity.falcon) {
				Session.Entity.falcon = new Entity();
			}

			Session.Entity.falcon.set({
				objecttype: Session.Entity.falcon.constructor.TYPE_FALCON,
				GID: Session.Entity.GID + '_FALCON',
				PosDir: [Session.Entity.position[0], Session.Entity.position[1], 0],
				job: Session.Entity._job + '_FALCON',
				speed: Math.max(Session.Entity.walk.speed - 50, 1),
				name: '',
				hp: -1,
				maxhp: -1,
				hideShadow: true
			});
			EntityManager.add(Session.Entity.falcon);
		}
		if (Session.Entity.effectState & StatusConst.EffectState.WUG) {
			if (!Session.Entity.wug) {
				Session.Entity.wug = new Entity();
			}

			Session.Entity.wug.set({
				objecttype: Session.Entity.wug.constructor.TYPE_WUG,
				GID: Session.Entity.GID + '_WUG',
				PosDir: [Session.Entity.position[0], Session.Entity.position[1], 0],
				job: Session.Entity._job + '_WUG',
				speed: Math.max(Session.Entity.walk.speed - 50, 1),
				name: '',
				hp: -1,
				maxhp: -1
			});
			EntityManager.add(Session.Entity.wug);
		}
		// free and load aura so it loads in new map
		Session.Entity.aura.free();
		Session.Entity.aura.load(EffectManager);

		// Spawn all signboards for the current map
		const mapName = MapRenderer.currentMap.replace('.gat', '').toLowerCase();
		const signboards = DB.getAllSignboardsForMap(mapName);

		if (signboards) {
			for (const x in signboards) {
				for (const y in signboards[x]) {
					const signboardData = signboards[x][y];
					SignboardManager.add(parseInt(x), parseInt(y), signboardData);
				}
			}
		}

		// Initialize camera
		Camera.setTarget(Session.Entity);
		Camera.init();

		// Add Game UI
		MiniMap.getUI().append();
		MiniMap.getUI().setMap(MapRenderer.currentMap);
		if (Configs.get('enableMapName')) {
			MapName.setMap(MapRenderer.currentMap);
			MapName.append();
		}
		ChatBox.append();
		ChatBoxSettings.append();
		BasicInfo.getUI().append();
		Escape.append();
		Inventory.getUI().append();
		CartItems.append();
		Vending.append();
		ChangeCart.append();
		CartDecoration.append();
		Equipment.getUI().append();
		ShortCuts.append();
		StatusIcons.append();
		ShortCut.append();
		ChatRoomCreate.append();
		Emoticons.append();
		SkillList.getUI().append();
		FPS.append();
		PartyFriends.getUI().append();
		Guild.append();
		WorldMap.append();
		SkillListMH.homunculus.append();
		SkillListMH.mercenary.append();
		MobileUI.append();
		JoystickUI.append();
		Navigation.append();
		Roulette.append();
		if (Configs.get('enableAchievements') && PACKETVER.value >= 20150513) {
			Achievement.append();
		}

		if (Session.PCGoldTimer) {
			PCGoldTimer.append();
		}

		WinStats.getUI().append();

		Quest.getUI().append();

		if (Configs.get('enableCashShop')) {
			CashShopIcon.append();
		}

		if (Configs.get('enableCheckAttendance') && PACKETVER.value >= 20180307) {
			CheckAttendance.append();
		}

		// Reload plugins
		PluginManager.init();

		// Map loaded
		Network.sendPacket(new PACKET.CZ.NOTIFY_ACTORINIT());

		// Rates Info
		if (Session.ratesInfo) {
			Announce.append();
			Announce.set(Session.ratesInfo, '#FFFF00', true);
		}

		// Request cash shop items
		if (PACKETVER.value >= 20130320 && Session.requestCashShop) {
			Network.sendPacket(new PACKET.CZ.PC_CASH_POINT_ITEMLIST());
			Session.requestCashShop = false;
		}

		// send request blocking play cancel
		if (PACKETVER.value >= 20130320) {
			Network.sendPacket(new PACKET.CZ.BLOCKING_PLAY_CANCEL());
		}
	};

	MapRenderer.setMap(pkt.mapName);
}

/**
 * Change zone server
 *
 * @param {object} pkt - PACKET.ZC.NPCACK_SERVERMOVE
 */
function onServerChange(pkt) {
	MapEngine.init(pkt.addr.ip, pkt.addr.port, pkt.mapName);
}

/**
 * Resets the per-character UI state shared by the exit and restart flows.
 * Components that were never prepared have no root element to clean.
 */
function cleanGameUI() {
	WhisperBox.clearAll();

	const tasks = [
		[BasicInfo, 'remove'],
		[PlayerViewEquip, 'remove'],
		[StatusIcons, 'clean'],
		[ChatBox, 'clean'],
		[ShortCut, 'clean'],
		[Quest, 'clean'],
		[PartyFriends, 'clean'],
		[CashShop, 'clean']
	];

	for (const [target, method] of tasks) {
		const component = typeof target.getUI === 'function' ? target.getUI() : target;

		if (component && component.__loaded && typeof component[method] === 'function') {
			component[method]();
		}
	}
}

/**
 * Ask the server to disconnect
 */
function onExitRequest() {
	const pkt = new PACKET.CZ.REQUEST_QUIT();
	Network.sendPacket(pkt);

	// Wait a second, if no answer from the server, then close it.
	_exitTimer = Events.setTimeout(() => {
		_exitTimer = null;
		onExitSuccess();
	}, 1000);
}

/**
 * Server don't want us to disconnect yet
 *
 * @param {object} pkt - PACKET.ZC.REFUSE_QUIT
 */
function onExitFail(pkt) {
	ChatBox.addText(DB.getMessage(502), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
}

/**
 * Server accept to disconnect us
 *
 * @param {object} pkt - PACKET.ZC.REFUSE_QUIT
 */
function onExitSuccess() {
	if (_exiting) {
		return;
	}
	_exiting = true;

	if (_exitTimer !== null) {
		Events.clearTimeout(_exitTimer);
		_exitTimer = null;
	}

	if (PACKETVER.value >= 20170315 && Session.WebToken) {
		ShortCut.saveToServer();
	}

	GuildEngine.guild_id = 0;
	cleanGameUI();
	Session.Achievement = null;
	Mouse.intersect = false;
	UIManager.removeComponents();
	Network.close();
	Renderer.stop();
	MapRenderer.free();
	SoundManager.stop();
	BGM.stop();
	import('Engine/GameEngine.js').then(m => m.default.reload());
}

/**
 * Try to return to char-server
 */
function onRestartRequest() {
	const pkt = new PACKET.CZ.RESTART();
	pkt.type = 1;
	Network.sendPacket(pkt);
}

/**
 * Go back to save point request
 */
function onReturnSavePointRequest() {
	const pkt = new PACKET.CZ.RESTART();
	pkt.type = 0;
	Network.sendPacket(pkt);
}

/**
 * Resurection feature
 */
function onResurectionRequest() {
	const pkt = new PACKET.CZ.STANDING_RESURRECTION();
	Network.sendPacket(pkt);
}

/**
 * Does the server want you to return to char-server ?
 *
 * @param {object} pkt - PACKET.ZC.RESTART_ACK
 */
function onRestartAnswer(pkt) {
	if (!pkt.type) {
		// Have to wait 10sec
		ChatBox.addText(DB.getMessage(502), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	} else {
		GuildEngine.guild_id = 0;
		cleanGameUI();
		Session.Achievement = null;
		Mouse.intersect = false;
		MapRenderer.free();
		Renderer.stop();
		onRestart();
	}
}

/**
 * Response from server to disconnect
 * @param pkt - {object}
 */
function onDisconnectAnswer(pkt) {
	switch (pkt.result) {
		// Disconnect
		case 0:
			Renderer.stop();
			onExitSuccess();
			break;

		case 1:
			// Have to wait 10 sec
			ChatBox.addText(DB.getMessage(502), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		default:
	}
}

/**
 * ChatBox talk
 *
 * @param {string} user
 * @param {string} text
 * @param {number} target
 */
function onRequestTalk(user, text, target) {
	let pkt;
	const flag_party = text[0] === '%' || KEYS.CTRL;
	const flag_guild =
		text[0] === '$' ||
		(KEYS.ALT &&
			!(
				KEYS[0] ||
				KEYS[1] ||
				KEYS[2] ||
				KEYS[3] ||
				KEYS[4] ||
				KEYS[5] ||
				KEYS[6] ||
				KEYS[7] ||
				KEYS[8] ||
				KEYS[9]
			));

	text = text.replace(/^(\$|%)/, '');

	// Private messages
	if (user && user.length) {
		pkt = new PACKET.CZ.WHISPER();
		pkt.receiver = user;
		pkt.msg = text;
		Network.sendPacket(pkt);
		ChatBox.PrivateMessageStorage.nick = user;
		ChatBox.PrivateMessageStorage.msg = text;
		return;
	}

	// Set off/on flags
	if (flag_party) {
		target = (target & ~ChatBox.TYPE.PARTY) | (~target & ChatBox.TYPE.PARTY);
	}

	if (flag_guild) {
		target = (target & ~ChatBox.TYPE.GUILD) | (~target & ChatBox.TYPE.GUILD);
	}

	// Get packet
	if (target & ChatBox.TYPE.PARTY) {
		pkt = new PACKET.CZ.REQUEST_CHAT_PARTY();
	} else if (target & ChatBox.TYPE.GUILD) {
		pkt = new PACKET.CZ.GUILD_CHAT();
	} else if (target & ChatBox.TYPE.CLAN) {
		pkt = new PACKET.CZ.CLAN_CHAT();
	} else {
		pkt = new PACKET.CZ.REQUEST_CHAT();
		chatLines++;
	}

	// send packet
	pkt.msg = Session.Entity.display.name + ' : ' + text;
	Network.sendPacket(pkt);

	//Super Novice Chant
	if (chatLines > 7 && DB.isSuperNovice(Session.Entity._job)) {
		if (Math.floor((BasicInfo.getUI().base_exp / BasicInfo.getUI().base_exp_next) * 1000.0) % 100 == 0) {
			if (text == DB.getMessage(790)) {
				snCounter = 1;
			} else if (
				snCounter == 1 &&
				text == DB.getMessage(791) + ' ' + Session.Entity.display.name + ' ' + DB.getMessage(792)
			) {
				snCounter = 2;
			} else if (snCounter == 2 && text == DB.getMessage(793)) {
				snCounter = 3;
			} else if (snCounter == 3) {
				snCounter = 0;
				pkt = new PACKET.CZ.CHOPOKGI();
				Network.sendPacket(pkt);
			} else {
				snCounter = 0;
			}
		}
	}
}

/**
 * Remove cart/peco/falcon
 */
function onRemoveOption() {
	const pkt = new PACKET.CZ.REQ_CARTOFF();
	Network.sendPacket(pkt);
}

/**
 * @type {number} walk timer
 */
let _walkTimer = null;

/**
 * @type {number} Last delay to walk
 */
let _walkLastTick = 0;

/**
 * Ask to move
 */
function onRequestWalk() {
	Events.clearTimeout(_walkTimer);

	// If siting, update direction
	if (Session.Entity.action === Session.Entity.ACTION.SIT || KEYS.SHIFT) {
		Session.Entity.lookTo(Mouse.world.x, Mouse.world.y);

		let pkt;
		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.CHANGE_DIRECTION2();
		} else {
			pkt = new PACKET.CZ.CHANGE_DIRECTION();
		}
		pkt.headDir = Session.Entity.headDir;
		pkt.dir = Session.Entity.direction;
		Network.sendPacket(pkt);
		return;
	}

	walkIntervalProcess();
}

/**
 * Stop moving
 */
function onRequestStopWalk() {
	Events.clearTimeout(_walkTimer);
}

/**
 * Moving function
 */
function walkIntervalProcess() {
	// setTimeout isn't accurate, so reduce the value
	// to avoid possible errors.
	if (_walkLastTick + 200 > Renderer.tick) {
		return;
	}

	const isWalkable = Mouse.world.x > -1 && Mouse.world.y > -1;
	const isCurrentPos =
		Math.round(Session.Entity.position[0]) === Mouse.world.x &&
		Math.round(Session.Entity.position[1]) === Mouse.world.y;

	if (isWalkable && !isCurrentPos) {
		let pkt;
		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.REQUEST_MOVE2();
		} else {
			pkt = new PACKET.CZ.REQUEST_MOVE();
		}
		if (!checkFreeCell(Mouse.world.x, Mouse.world.y, 9, pkt.dest)) {
			pkt.dest[0] = Mouse.world.x;
			pkt.dest[1] = Mouse.world.y;
		}

		Network.sendPacket(pkt);
	}

	Events.clearTimeout(_walkTimer);
	_walkTimer = Events.setTimeout(walkIntervalProcess, 500);
	_walkLastTick = +Renderer.tick;
}

/**
 * Search free cells around a position
 *
 * @param {number} x
 * @param {number} y
 * @param {number} range
 * @param {array} out
 */
function checkFreeCell(x, y, range, out) {
	let _x, _y, r;
	const d_x = Session.Entity.position[0] < x ? -1 : 1;
	const d_y = Session.Entity.position[1] < y ? -1 : 1;

	// Search possible positions
	for (r = 0; r <= range; ++r) {
		for (_x = -r; _x <= r; ++_x) {
			for (_y = -r; _y <= r; ++_y) {
				if (isFreeCell(x + _x * d_x, y + _y * d_y)) {
					out[0] = x + _x * d_x;
					out[1] = y + _y * d_y;
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * Does a cell is free (walkable, and no entity on)
 *
 * @param {number} x
 * @param {number} y
 * @param {returns} is free
 */
function isFreeCell(x, y) {
	if (!(Altitude.getCellType(x, y) & Altitude.TYPE.WALKABLE)) {
		return false;
	}

	let free = true;

	EntityManager.forEach(function (entity) {
		if (
			entity.objecttype != entity.constructor.TYPE_EFFECT &&
			entity.objecttype != entity.constructor.TYPE_UNIT &&
			entity.objecttype != entity.constructor.TYPE_TRAP &&
			Math.round(entity.position[0]) === x &&
			Math.round(entity.position[1]) === y
		) {
			free = false;
			return false;
		}

		return true;
	});

	return free;
}

/**
 * If the character moved to attack, once it finished to move ask to attack
 */
function onWalkEnd() {
	// No action to do ?
	if (Session.moveAction) {
		// Not sure why, but there is a synchronization error with the
		// server when moving to attack (wrong position).
		// So wait 50ms to be sure we are at the correct position before
		// performing an action
		Events.setTimeout(() => {
			if (Session.moveAction) {
				Network.sendPacket(Session.moveAction);
				Session.moveAction = null;
			}
		}, 50);
	}
}

/**
 * Ask server to update status
 *
 * @param {number} id
 * @param {number} amount
 */
function onRequestStatUpdate(id, amount) {
	const pkt = new PACKET.CZ.STATUS_CHANGE();
	pkt.statusID = id;
	pkt.changeAmount = amount;

	Network.sendPacket(pkt);
}

/**
 * Drop item to the floor
 *
 * @param {number} index in inventory
 * @param {number} count to drop
 */
function onDropItem(index, count) {
	if (count) {
		let pkt;
		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.ITEM_THROW2();
		} else {
			pkt = new PACKET.CZ.ITEM_THROW();
		}
		pkt.Index = index;
		pkt.count = count;
		Network.sendPacket(pkt);
	}
}

/**
 * Use an item
 *
 * @param {number} item's index
 */
function onUseItem(index) {
	// Items are not usable when Laphine Synthesis, Upgrade, ItemReform UI is open (if they are available at all)
	if (
		(LaphineSys.__loaded && LaphineSys.__active && LaphineSys.ui.is(':visible')) ||
		(LaphineUpg.__loaded && LaphineUpg.__active && LaphineUpg.ui.is(':visible')) ||
		(ItemReform.__loaded && ItemReform.__active && ItemReform.ui.is(':visible'))
	) {
		return false;
	}

	let pkt;
	if (PACKETVER.value >= 20180307) {
		// not sure - this date is when the shuffle packets stoped
		pkt = new PACKET.CZ.USE_ITEM2();
	} else {
		pkt = new PACKET.CZ.USE_ITEM();
	}
	pkt.index = index;
	pkt.AID = Session.Entity.GID;
	Network.sendPacket(pkt);
}

/**
 * Equip item
 *
 * @param {number} item's index
 * @param {number} where to equip
 */
function onEquipItem(index, location) {
	const pkt = new PACKET.CZ.REQ_WEAR_EQUIP();
	pkt.index = index;
	pkt.wearLocation = location;
	Network.sendPacket(pkt);
}

/**
 * Take off an equip
 *
 * @param {number} index to unequip
 */
function onUnEquip(index) {
	const pkt = new PACKET.CZ.REQ_TAKEOFF_EQUIP();
	pkt.index = index;
	Network.sendPacket(pkt);
}

/**
 * Add Switch Equip
 */
function onAddSwitchEquip(index, location) {
	const pkt = new PACKET.CZ.REQ_WEAR_SWITCHEQUIP_ADD();
	pkt.index = index;
	pkt.wearLocation = location;
	Network.sendPacket(pkt);
}

/**
 * Remove Switch Equip
 */
function onRemoveSwitchEquip(index) {
	const pkt = new PACKET.CZ.REQ_WEAR_SWITCHEQUIP_REMOVE();
	pkt.index = index;
	Network.sendPacket(pkt);
}

/**
 * Update config
 *
 * @param {number} config id (only type:0 is supported - equip)
 * @param {number} val
 */
function onConfigUpdate(type, val) {
	const pkt = new PACKET.CZ.CONFIG();
	pkt.Config = type;
	pkt.Value = val;
	Network.sendPacket(pkt);
}

/**
 * Go back from map-server to char-server
 */
function onRestart() {
	import('Engine/CharEngine.js').then(m => m.default.reload());
}

/**
 * Reply to reassembly auth packet
 * @param {object} pkt - packet
 */
function onReassemblyAuth(pkt) {
	for (const [ZC, CZ] of packetMap.entries()) {
		if (pkt instanceof ZC) {
			console.warn(`Reassembly Auth ${ZC.id} => ${CZ.id}`);
			Network.sendPacket(new CZ());
			return;
		}
	}
}

/**
 * Export
 */
export default MapEngine;
