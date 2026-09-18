/**
 * DB/DBManager.js
 *
 * Manage and load DB files
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import Configs from 'Core/Configs.js';
import TextEncoding from 'Utils/CodepageManager.js';
import JobId from './Jobs/JobConst.js';
import ClassTable from './Jobs/JobNameTable.js';
import PaletteTable from './Jobs/PalNameTable.js';
import WeaponAction from './Jobs/WeaponAction.js';
import WeaponJobTable from './Jobs/WeaponJobTable.js';
import BabyTable from './Jobs/BabyTable.js';
import HairIndexTable from './Jobs/HairIndexTable.js';
import MonsterTable from './Monsters/MonsterTable.js';
import MonsterNameTable from './Monsters/MonsterNameTable.js';
import PetIllustration from './Pets/PetIllustration.js';
import PetAction from './Pets/PetAction.js';
import ItemTable from './Items/ItemTable.js';
import HatTable from './Items/HatTable.js';
import ShieldTable from './Items/ShieldTable.js';
import WeaponTable from './Items/WeaponTable.js';
import WeaponType from './Items/WeaponType.js';
import WeaponTypeExpansion from './Items/WeaponTypeExpansion.js';
import WeaponSoundTable from './Items/WeaponSoundTable.js';
import WeaponHitSoundTable from './Items/WeaponHitSoundTable.js';
import RobeTable from './Items/RobeTable.js';
import RandomOption from 'DB/Items/ItemRandomOptionTable.js';
import WorldMap from './Map/WorldMap.js';
import SKID from './Skills/SkillConst.js';
import SkillInfo from './Skills/SkillInfo.js';
import SkillTreeView from './Skills/SkillTreeView.js';
import JobHitSoundTable from './Jobs/JobHitSoundTable.js';
import WeaponTrailTable from './Items/WeaponTrailTable.js';
import TownInfo from './TownInfo.js';
import StatusInfo from './Status/StatusInfo.js';
import SC from './Status/StatusConst.js';
import XmlParse from 'Vendors/xmlparse.js';
import Base62 from 'Utils/Base62.js';
import { BSON } from 'bson';
import PetEmotionTable from './Pets/PetEmotionTable.js';
import PetHungryState from './Pets/PetHungryState.js';
import PetFriendlyState from './Pets/PetFriendlyState.js';
import PetMessageConst from './Pets/PetMessageConst.js';
import MapInfo from './Map/MapTable.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import MemoryManager from 'Core/MemoryManager.js';

//Pet
//MapName

/**
 * @type {Object} lua instance
 */
let lua;
let HO_AI;
let MER_AI;
let default_HO_AI;
let default_MER_AI;

/**
 * @const {Array} message string
 */
const MsgStringTable = [];

/**
 * @const {Array} message string
 */
const JokeTable = [];

/**
 * @const {Array} message string
 */
const ScreamTable = [];

/**
 * @const {Array} map table
 * struct { string name; string mp3; object fog }
 */
const MapTable = {};

/**
 * @type {Object} SkillDescription Table
 */
let SkillDescription = {};

/**
 * @const {Array} ASCII sex
 */
const SexTable = ['\xbf\xa9', '\xb3\xb2'];

/**
 * @type {Object} Pet Talk json object
 */
let PetTalkTable = {};

/**
 * @const {Object} CheckAttendanceTable Attendance config
 */
const CheckAttendanceTable = { Config: {}, Rewards: [] };

/**
 * @const {Array} buyingStoreItemList Table
 */
const buyingStoreItemList = new Array();

/**
 * @const {Array} LaphineSysTable Table
 */
const LaphineSysTable = [];

/**
 * @const {Array} LaphineUpgTable Table
 */
const LaphineUpgTable = [];

/**
 * @const {Object} ItemDBName Table
 * json object
 */
const ItemDBNameTbl = {};

/**
 * @const {Object} ItemReform Table
 * json object
 */
const ItemReformTable = { ReformInfo: {}, ReformItemList: {} };

/**
 * @type {Object} EnchantList Table json object
 */
let EnchantListTable = {};

/**
 * @const {Object} SignBoardTranslated Table
 */
const SignBoardTranslatedTable = {};

/**
 * @type {Object} SignBoard Table
 */
let SignBoardTable = {};

/**
 * @const {Object} NaviMap Table
 */
const NaviMapTable = {};

/**
 * @const {Object} NaviMob Table
 */
const NaviMobTable = {};

/**
 * @const {Object} NaviNpc Table
 */
const NaviNpcTable = {};

/**
 * @const {Object} NaviLink Table
 */
const NaviLinkTable = {};

/**
 * @const {Object} NaviLinkDistance Table
 */
const NaviLinkDistanceTable = {};

/**
 * @const {Object} NaviNpcDistance Table
 */
const NaviNpcDistanceTable = {};

/**
 * @const {Object} QuestInfo Table
 */
const QuestInfo = {};

/**
 * @const {Object} Title Table
 */
const TitleTable = {};

/**
 * @type {Object} PetDBTable
 */
let PetDBTable = {};
let EggIDToJobID = {};

/**
 * @const {Object} Reputation Table
 */
const ReputeGroup = {};
const ReputeInfo = {};

/**
 * @const {Object} Achievement Table
 */
const AchievementTable = {};

/**
 * @const {Object} CSV Tables
 */
const MsgEmotionCSV = {};

/**
 * @const {Object} Hat Effect Tables
 */
const HatEffectID = {};
const HatEffectInfo = {};
const FootPrintEffectInfo = {};

/**
 * @const {Array} CashShopBanner Table
 */
const CashShopBannerTable = [];

/**
 * @const {Object} Ez2streffect Table
 */
const Ez2streffect = {};

const unknownItem = {
	unidentifiedDisplayName: 'Unknown Item',
	unidentifiedResourceName: '\xbb\xe7\xb0\xfa',
	unidentifiedDescriptionName: ['...'],
	identifiedDisplayName: 'Unknown Item',
	identifiedResourceName: '\xbb\xe7\xb0\xfa',
	identifiedDescriptionName: ['...'],
	slotCount: 0,
	ClassNum: 0
};

/**
 * @const {Array} User charpage init
 */
const servers = Configs.get('servers', []);
const langType = servers[0] && servers[0].langtype ? parseInt(servers[0].langtype, 0) : 0;

// setup default encoding
const userCharpage = TextEncoding.detectEncodingByLangtype(langType, Configs.get('disableKorean'));
const grfCharpage = 'windows-1252';
TextEncoding.setCharset(grfCharpage);

// create decoders
const userStringDecoder = TextEncoding;

/**
 * DB NameSpace
 */
class DB {
	/**
	 * @type {Object} file alias list
	 */
	static mapalias = {};

	/**
	 * @type {Object} CharName by GID list
	 */
	static CNameTable = {};

	/**
	 * @type {string} interface path
	 */
	static INTERFACE_PATH = 'data/texture/\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/';

	/**
	 * @type {string} lua path
	 */
	static LUA_PATH = 'data/luafiles514/lua files/';

	/**
	 * @type {Object} UpdateOwnerName Table
	 */
	static UpdateOwnerName = {};

	/**
	 * Initialize DB
	 */
	static init() {
		DB.isLoaded = false;
		DB.count = 0;
		DB.index = 0;

		// Callback
		let index = 0,
			count = 0;
		function onLoad() {
			count++;
			return function OnLoadClosure() {
				index++;

				if (DB.onProgress) {
					DB.onProgress(index, count);
				}

				if (index === count && DB.onReady) {
					DB.onReady();
				}
			};
		}

		loadFontFromClient('System/Font/');

		// Loading TXT Tables
		loadTable(
			'data/mp3nametable.txt',
			'#',
			2,
			function (_index, key, val) {
				(MapTable[key] || (MapTable[key] = {})).mp3 = val;
			},
			onLoad()
		);
		loadTable(
			'data/mapnametable.txt',
			'#',
			2,
			function (_index, key, val) {
				(MapTable[key] || (MapTable[key] = {})).name = val;
			},
			onLoad(),
			true
		);
		const loadmsg = onLoad();
		loadTable(
			'data/msgstringtable.txt',
			'#',
			1,
			(_index, val) => {
				MsgStringTable[_index] = val;
			},
			() => loadCSV('data/msgstringtable.csv', MsgStringTable, 0, 1, loadmsg),
			true
		);

		loadTable(
			'data/resnametable.txt',
			'#',
			2,
			function (_index, key, val) {
				DB.mapalias[key] = val;
			},
			onLoad()
		);
	}

	static isLoaded = false;
	static count = 0;
	static index = 0;
	static async lazyInit() {
		console.log('Loading DB files...');
		await startLua();
		// Callback
		DB.index = 0;
		DB.count = 0;
		function onLoad() {
			DB.count++;
			return function OnLoadClosure() {
				DB.index++;

				if (DB.onProgress) {
					DB.onProgress(DB.index, DB.count);
				}

				if (DB.index === DB.count) {
					DB.isLoaded = true;
					// Force cleanup of DB file data (lua, txt, csv, bson blobs) that are no longer needed
					// gl is null here because we may not have a WebGL context yet during lazy loading
					MemoryManager.forceClean(null, /\.(lub|lua|txt|csv|bson)$/i);
				}
			};
		}

		// CSV Tables - Client Date is not sure since when they were added
		if (PACKETVER.value >= 20230302) {
			loadCSV('data/simplemsg/msg_emotion.csv', MsgEmotionCSV, 0, 2, onLoad());
		}
		// TODO: load these load files by PACKETVER
		if (Configs.get('loadLua')) {
			// Item
			let iteminfoNames = [];
			const customII = Configs.get('customItemInfo', []);

			if (Array.isArray(customII) && customII.length > 0) {
				// add custom client info table
				iteminfoNames = iteminfoNames.concat(customII);
				tryLoadLuaAliases(loadItemInfo, iteminfoNames, null, onLoad(), true);
			} else {
				iteminfoNames = iteminfoNames.concat(getSystemAliases('System/itemInfo.lub'));
				tryLoadLuaAliases(loadItemInfo, iteminfoNames, null, onLoad());
			}

			loadLuaTable(
				[DB.LUA_PATH + 'datainfo/accessoryid.lub', DB.LUA_PATH + 'datainfo/accname.lub'],
				'AccNameTable',
				function (json) {
					Object.assign(HatTable, json);
				},
				onLoad()
			);
			loadLuaTable(
				[DB.LUA_PATH + 'datainfo/spriterobeid.lub', DB.LUA_PATH + 'datainfo/spriterobename.lub'],
				'RobeNameTable',
				function (json) {
					Object.assign(RobeTable, json);
				},
				onLoad()
			);

			if (PACKETVER.value >= 20141008) {
				loadLuaTable(
					[DB.LUA_PATH + 'datainfo/npcidentity.lub', DB.LUA_PATH + 'datainfo/jobname.lub'],
					'JobNameTable',
					function (json) {
						Object.assign(MonsterTable, json);
					},
					onLoad(),
					function () {
						loadPetInfo(DB.LUA_PATH + 'datainfo/petinfo.lub', null, function () {
							tryLoadLuaAliases(
								loadPetEvolution,
								getSystemAliases('System/PetEvolutionCln.lub'),
								null,
								onLoad()
							);
						});
					}
				);
			} else {
				loadLuaTable(
					[DB.LUA_PATH + 'datainfo/npcidentity.lub', DB.LUA_PATH + 'datainfo/jobname.lub'],
					'JobNameTable',
					function (json) {
						Object.assign(MonsterTable, json);
					},
					onLoad()
				);
			}

			loadLuaTable(
				[DB.LUA_PATH + 'datainfo/enumvar.lub', DB.LUA_PATH + 'datainfo/addrandomoptionnametable.lub'],
				'NameTable_VAR',
				function (json) {
					Object.assign(RandomOption, json);
				},
				onLoad()
			);
			loadItemDBTable(DB.LUA_PATH + 'ItemDBNameTbl.lub', null, onLoad());

			// Weapon tables
			loadWeaponTable(DB.LUA_PATH + 'datainfo/weapontable.lub', null, onLoad());

			// Title tables
			if (PACKETVER.value >= 20170208) {
				loadTitleTable(DB.LUA_PATH + 'datainfo/titletable.lub', null, onLoad());
			}

			// Skill - load skillid.lub to populate SKID, then load description
			const onSkillEnd = onLoad();
			loadLuaValue(DB.LUA_PATH + 'skillinfoz/skillid.lub', 'SKID', json => {
				if (json && typeof json === 'object') {
					// Validate and merge entries into SKID
					for (const k in json) {
						if (Object.prototype.hasOwnProperty.call(json, k)) {
							const value = json[k];
							if (typeof value === 'number' && value > 0) {
								SKID[k] = value;
							}
						}
					}
				}
				// Load description - skillid.lub is re-executed harmlessly (Lua just repopulates globals)
				loadLuaTable(
					[DB.LUA_PATH + 'skillinfoz/skillid.lub', DB.LUA_PATH + 'skillinfoz/skilldescript.lub'],
					'SKILL_DESCRIPT',
					_json => {
						SkillDescription = _json;
					},
					() => {
						// Calls after skillids and descs been populated
						loadSkillInfoList(DB.LUA_PATH + 'skillinfoz/skillinfolist.lub', null, () => {
							loadSkillTreeView(DB.LUA_PATH + 'skillinfoz/skilltreeview.lub', null, () => {
								// Load ez2streffect, PACKETVER unknown when the while has been added, tied to default PACKETVER of rathena for 4th job
								if (PACKETVER.value >= 20211103) {
									const bsonOnLoad = onLoad();
									loadBSONFile('data/contentdata/effectdata/ez2streffect.bson', Ez2streffect, () => {
										Promise.all([
											import('DB/Effects/EffectTable.js'),
											import('DB/Skills/SkillEffect.js')
										]).then(([EffectTable, SkillEffect]) => {
											mergeEz2Effects(EffectTable.default, SkillEffect.default);
											bsonOnLoad();
										});
									});
								}
								// Skill Lua finished
								onSkillEnd();
							});
						});
					}
				);
			});

			// Status
			loadStateIconInfo(DB.LUA_PATH + 'stateicon/', null, onLoad());

			// Legacy Navigation
			if (PACKETVER.value >= 20111010) {
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_map_krpri.lub',
					'Navi_Map',
					function (json) {
						Object.assign(NaviMapTable, json);
					},
					onLoad()
				);
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_mob_krpri.lub',
					'Navi_Mob',
					function (json) {
						Object.assign(NaviMobTable, json);
					},
					onLoad()
				);
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_npc_krpri.lub',
					'Navi_Npc',
					function (json) {
						Object.assign(NaviNpcTable, json);
					},
					onLoad()
				);
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_link_krpri.lub',
					'Navi_Link',
					function (json) {
						Object.assign(NaviLinkTable, json);
					},
					onLoad()
				);
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_linkdistance_krpri.lub',
					'Navi_Distance',
					function (json) {
						Object.assign(NaviLinkDistanceTable, json);
					},
					onLoad()
				);
				loadLuaValue(
					DB.LUA_PATH + 'navigation/navi_npcdistance_krpri.lub',
					'Navi_NpcDistance',
					function (json) {
						Object.assign(NaviNpcDistanceTable, json);
					},
					onLoad()
				);
			}

			// HatEffect
			if (PACKETVER.value >= 20150507) {
				loadHatEffectInfo(onLoad());
			}

			// LaphineSys
			if (PACKETVER.value >= 20160601) {
				loadLaphineSysFile(DB.LUA_PATH + 'datainfo/lapineddukddakbox.lub', null, onLoad());
			}

			// LaphineUpg
			if (PACKETVER.value >= 20170726) {
				loadLaphineUpgFile(DB.LUA_PATH + 'datainfo/lapineupgradebox.lub', null, onLoad());
			}

			// ItemReform
			if (PACKETVER.value >= 20200916) {
				loadItemReformFile(DB.LUA_PATH + 'ItemReform/ItemReformSystem.lub', null, onLoad());
			}

			// EnchantList
			if (PACKETVER.value >= 20211103) {
				loadEnchantListFile(DB.LUA_PATH + 'Enchant/EnchantList', onLoad());
			}

			// MapName
			if (Configs.get('enableMapName') /*PACKETVER.value >= 20190605*/) {
				// We allow this feature to be enabled on any version due to popular demand
				tryLoadLuaAliases(
					loadMapTbl,
					getSystemAliases('System/mapInfo.lub'),
					function (json) {
						for (const key in json) {
							if (json.hasOwnProperty(key)) {
								MapInfo[key] = json[key];
							}
						}
						updateMapTable();
					},
					onLoad()
				);
			}

			// EntitySignBoard
			const onSignBoardEnd = onLoad();
			loadSignBoardList(DB.LUA_PATH + 'SignBoardList.lub', null, () => {
				// this is not official, its a translation file
				loadSignBoardData('SystemEN/Sign_Data.lub', null, onSignBoardEnd);
			});

			// CheckAttendance
			if (Configs.get('enableCheckAttendance') && PACKETVER.value >= 20180307) {
				loadAttendanceFile('System/CheckAttendance.lub', null, onLoad());
			}

			// Quest
			const onQuestEnd = onLoad();
			tryLoadLuaAliases(loadQuestInfo, getSystemAliases('System/OngoingQuestInfoList.lub'), null, () => {
				// this is not official, its a translation file
				loadQuestInfo('SystemEN/OngoingQuests.lub', null, onQuestEnd);
			});

			// TODO: System/RecommendedQuests.lub

			// Achievements
			if (Configs.get('enableAchievements') && PACKETVER.value >= 20150513) {
				loadLuaValue(
					'System/achievement_list.lub',
					'achievement_tbl',
					function (json) {
						if (json) {
							Object.assign(AchievementTable, json);
						}
					},
					onLoad()
				);
			}

			// Cash Shop Banner - implemented early 2018
			if (Configs.get('enableCashShop') && PACKETVER.value >= 20180000) {
				loadCashShopBanner(DB.LUA_PATH + 'datainfo/tb_cashshop_banner.lub', null, onLoad());
			}
		} else {
			// Item
			loadTable(
				'data/num2itemdisplaynametable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).unidentifiedDisplayName = val.replace(/_/g, ' ');
				},
				onLoad(),
				true
			);
			loadTable(
				'data/num2itemresnametable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).unidentifiedResourceName = val;
				},
				onLoad()
			);
			loadTable(
				'data/num2itemdesctable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).unidentifiedDescriptionName = val.split('\n');
				},
				onLoad(),
				true
			);
			loadTable(
				'data/idnum2itemdisplaynametable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).identifiedDisplayName = val.replace(/_/g, ' ');
				},
				onLoad(),
				true
			);
			loadTable(
				'data/idnum2itemresnametable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).identifiedResourceName = val;
				},
				onLoad()
			);
			loadTable(
				'data/idnum2itemdesctable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).identifiedDescriptionName = val.split('\n');
				},
				onLoad(),
				true
			);
			loadTable(
				'data/itemslotcounttable.txt',
				'#',
				2,
				function (_index, key, val) {
					(ItemTable[key] || (ItemTable[key] = {})).slotCount = val;
				},
				onLoad()
			);

			// Skill
			loadTable(
				'data/skilldesctable.txt',
				'#',
				2,
				function (_index, key, val) {
					SkillDescription[SKID[key]] = val.replace('\r\n', '\n');
				},
				onLoad(),
				true
			);
			// TODO: data/skillnametable.txt	- ?
			// TODO: data/skilltreeview.txt	- Replaces DB/Skills/SkillTreeView.js
			// TODO: data/leveluseskillspamount.txt	- Replaces DB/Skills/SkillInfo.js -> SkillInfo.SpAmount

			// Quest
			loadTable('data/questid2display.txt', '#', 6, parseQuestEntry, onLoad(), true);
		}

		// Load ItemMoveInfo and attach to ItemTable
		if (PACKETVER.value >= 20150422) {
			loadMoveInfoTable(onLoad());
		}

		// WoldMap
		loadWorldMapInfo(DB.LUA_PATH + 'worldviewdata/', onLoad());
		// Town Info
		const onTownInfoEnd = onLoad();
		loadTownInfoFile('System/Towninfo.lub', null, () => {
			// this is not official, its a translation file
			loadTownInfoFile('SystemEN/Towninfo.lub', null, onTownInfoEnd);
		});

		// Forging/Creation
		loadTable(
			'data/metalprocessitemlist.txt',
			'#',
			2,
			function (_index, key, val) {
				(ItemTable[key] || (ItemTable[key] = {})).processitemlist = val.split('\n');
			},
			onLoad(),
			true
		);

		// Card
		loadTable(
			'data/num2cardillustnametable.txt',
			'#',
			2,
			function (_index, key, val) {
				(ItemTable[key] || (ItemTable[key] = {})).illustResourcesName = val;
			},
			onLoad()
		);
		loadTable(
			'data/cardprefixnametable.txt',
			'#',
			2,
			function (_index, key, val) {
				(ItemTable[key] || (ItemTable[key] = {})).prefixName = val;
			},
			onLoad(),
			true
		);
		loadTable(
			'data/cardpostfixnametable.txt',
			'#',
			1,
			function (_index, key) {
				(ItemTable[key] || (ItemTable[key] = {})).isPostfix = true;
			},
			onLoad(),
			true
		);

		// EtcMapData
		loadTable('data/fogparametertable.txt', '#', 5, parseFogEntry, onLoad());
		loadTable('data/indoorrswtable.txt', '#', 1, parseIndoorEntry, onLoad());

		// Frost/Scream
		loadTable(
			'data/ba_frostjoke.txt',
			'\t',
			1,
			function (_index, val) {
				JokeTable[_index] = val;
			},
			onLoad(),
			true
		);
		loadTable(
			'data/dc_scream.txt',
			'\t',
			1,
			function (_index, val) {
				ScreamTable[_index] = val;
			},
			onLoad(),
			true
		);

		// Tips
		// TODO: /tipoftheday.txt
		// TODO: /GuildTip.txt

		loadXMLFile(
			'data/pettalktable.xml',
			function (json) {
				PetTalkTable = json['monster_talk_table'];
			},
			onLoad()
		);

		if (PACKETVER.value >= 20100427) {
			loadTable(
				'data/buyingstoreitemlist.txt',
				'#',
				1,
				function (_index, key) {
					buyingStoreItemList.push(parseInt(key, 10));
				},
				onLoad()
			);
		}

		// Reputation
		if (PACKETVER.value >= 20220330) {
			loadBSONFile('data/contentdata/repute/reputegroupdata.bson', ReputeGroup, function () {});
			loadBSONFile('data/contentdata/repute/reputeinfodata.bson', ReputeInfo, function () {});
		}

		Network.hookPacket(PACKET.ZC.ACK_REQNAME_BYGID, onUpdateOwnerName);
		Network.hookPacket(PACKET.ZC.ACK_REQNAME_BYGID2, onUpdateOwnerName);

		const onAIDriverLoaded = onLoad();
		import('Core/AIDriver.js').then(module => {
			module.default.initAI(onAIDriverLoaded);
		});
	}
	static getHOAI_VM() {
		return HO_AI;
	}

	static getMERAI_VM() {
		return MER_AI;
	}

	static getDefaultHOAI_VM() {
		return default_HO_AI;
	}

	static getDefaultMERAI_VM() {
		return default_MER_AI;
	}

	static getAllTitles() {
		return TitleTable;
	}

	static getAchievementTable() {
		return AchievementTable;
	}

	static getTitleString(titleID) {
		return TitleTable[titleID] || '';
	}

	static getSkillName(skillID) {
		return (SkillInfo[skillID] && SkillInfo[skillID].SkillName) || '';
	}

	/**
	 * Actor Type checks
	 *
	 * @param {number} jobid
	 */
	static isNPC(jobid) {
		return (
			(jobid > 45 && jobid < 130) ||
			(jobid >= 401 && jobid < 1000) ||
			(jobid >= 10001 && jobid < 19999) ||
			(jobid > 22300 && jobid < 22313) ||
			jobid == 32767 ||
			jobid == -1
		);
	}

	static isMercenary(jobid) {
		return jobid >= 6017 && jobid <= 6046;
	}

	static isHomunculus(jobid) {
		return (jobid >= 6001 && jobid <= 6016) || (jobid >= 6048 && jobid <= 6052);
	}

	static isMonster(jobid) {
		return (
			(jobid >= 1001 && jobid <= 3999) ||
			(jobid >= 20000 && jobid < 20834) ||
			(jobid >= 20852 && jobid < 22301) ||
			(jobid > 22313 && jobid < 22322)
		); // 22322 = last monster released on EP21
	}

	static isPlayer(jobid) {
		return (jobid >= 0 && jobid < 45) || (jobid >= 4001 && jobid <= 4361) || jobid == 4294967294;
	}

	static isDoram(jobid) {
		return (jobid >= 4217 && jobid <= 4220) || jobid === 4308 || jobid === 4315;
	}

	/**
	 * Is character id a baby ?
	 *
	 * @param {number} job id
	 * @return {boolean} is baby
	 */
	static isBaby(jobid) {
		return BabyTable.indexOf(jobid) > -1;
	}

	/**
	 * Is character a Assassin class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_ASSASSIN = new Set([
		JobId.ASSASSIN,
		JobId.ASSASSIN_H,
		JobId.GUILLOTINE_CROSS,
		JobId.GUILLOTINE_CROSS_H,
		JobId.SHADOW_CROSS,

		JobId.ASSASSIN_B,
		JobId.GUILLOTINE_CROSS_B,

		JobId.DOG_ASSASSIN,
		JobId.DOG_ASSA_X,
		JobId.DOG_G_CROSS,
		JobId.DOG_ASSASSIN_B,
		JobId.DOG_G_CROSS_B,
		JobId.SHADOW_CROSS_RIDING
	]);
	static isAssassin(jobid) {
		return this.S_JOBS_ASSASSIN.has(jobid);
	}

	/**
	 * Is character a Hunter class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_HUNTER = new Set([
		JobId.HUNTER,
		JobId.HUNTER_H,
		JobId.RANGER,
		JobId.RANGER_H,
		JobId.WINDHAWK,
		JobId.WINDHAWK2,

		JobId.HUNTER_B,
		JobId.RANGER_B,

		JobId.OSTRICH_HUNTER,
		JobId.OSTRICH_SNIPER,
		JobId.OSTRICH_RANGER,
		JobId.OSTRICH_HUNTER_B,
		JobId.OSTRICH_RANGER_B,
		JobId.WINDHAWK_RIDING
	]);
	static isHunter(jobid) {
		return this.S_JOBS_HUNTER.has(jobid);
	}

	/**
	 * Is character a Monk class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_MONK = new Set([
		JobId.MONK,
		JobId.MONK_H,
		JobId.SURA,
		JobId.SURA_H,
		JobId.SURA_2ND,
		JobId.INQUISITOR,

		JobId.MONK_B,
		JobId.SURA_B,

		JobId.SHEEP_MONK,
		JobId.SHEEP_CHAMP, // lolz.. MONK_H
		JobId.SHEEP_SURA,
		JobId.SHEEP_MONK_B,
		JobId.SHEEP_SURA_B,
		JobId.INQUISITOR_RIDING
	]);
	static isMonk(jobid) {
		return this.S_JOBS_MONK.has(jobid);
	}

	/**
	 * Is character a SN class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_SN = new Set([
		JobId.SUPERNOVICE,
		JobId.SUPERNOVICE2,
		JobId.HYPER_NOVICE,

		JobId.SUPERNOVICE_B,
		JobId.SUPERNOVICE2_B,

		JobId.PORING_SNOVICE,
		JobId.PORING_SNOVICE2,
		JobId.PORING_SNOVICE_B,
		JobId.PORING_SNOVICE2_B,
		JobId.HYPER_NOVICE_RIDING
	]);
	static isSuperNovice(jobid) {
		return this.S_JOBS_SN.has(jobid);
	}

	/**
	 * Is character a TaeKwon class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_TAEKWON = new Set([
		JobId.TAEKWON,
		JobId.STAR,
		JobId.STAR2,
		JobId.LINKER,
		JobId.STAR_EMPEROR,
		JobId.STAR_EMPEROR2,
		JobId.SOUL_REAPER,
		JobId.SOUL_REAPER2,
		JobId.SKY_EMPEROR,
		JobId.SKY_EMPEROR2,
		JobId.SOUL_ASCETIC,
		//JobId.SOUL_ASCETIC2, //missing from const

		JobId.TAEKWON_B,
		JobId.STAR_B,
		JobId.STAR2_B,
		JobId.LINKER_B,
		JobId.STAR_EMPEROR_B,
		JobId.STAR_EMPEROR2_B,
		JobId.SOUL_REAPER_B,
		JobId.SOUL_REAPER2_B,
		//JobId.SOUL_ASCETIC_B, //missing from const
		//JobId.SOUL_ASCETIC2_B //missing from const

		JobId.PORING_TAEKWON,
		JobId.PORING_STAR,
		JobId.FROG_LINKER,
		JobId.PORING_TAEKWON_B,
		JobId.PORING_STAR_B,
		JobId.FROG_LINKER_B,
		JobId.HAETAE_STAR_EMPEROR,
		JobId.HAETAE_SOUL_REAPER,
		JobId.HAETAE_STAR_EMPEROR_B,
		JobId.HAETAE_SOUL_REAPER_B,

		JobId.SKY_EMPEROR_RIDING,
		JobId.SOUL_ASCETIC_RIDING
	]);
	static isTaeKwon(jobid) {
		return this.S_JOBS_TAEKWON.has(jobid);
	}

	/**
	 * Is character a Gunslinger class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_GS = new Set([
		JobId.GUNSLINGER,
		JobId.REBELLION,
		JobId.NIGHT_WATCH,

		JobId.GUNSLINGER_B,
		JobId.REBELLION_B,

		JobId.PECO_GUNNER,
		JobId.PECO_REBELLION,
		JobId.NIGHT_WATCH_RIDING,
		JobId.PECO_GUNNER_B,
		JobId.PECO_REBELLION_B
	]);
	static isGunslinger(jobid) {
		return this.S_JOBS_GS.has(jobid);
	}

	/**
	 * Is character a Royal Guard class
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_RG = new Set([
		JobId.ROYAL_GUARD,
		JobId.ROYAL_GUARD_H,
		JobId.ROYAL_GUARD2,
		JobId.ROYAL_GUARD2_H,
		JobId.IMPERIAL_GUARD,

		JobId.ROYAL_GUARD_B,
		JobId.ROYAL_GUARD2_B,

		JobId.LION_ROYAL_GUARD,
		JobId.LION_ROYAL_GUARD_B,
		JobId.IMPERIAL_GUARD_RIDING
	]);
	static isRoyalGuard(jobid) {
		return this.S_JOBS_RG.has(jobid);
	}

	/**
	 * Is character using Madogear
	 *
	 * @param {number} jobid
	 * @return {boolean}
	 */
	static S_JOBS_MADO = new Set([JobId.MECHANIC2, JobId.MECHANIC2_H, JobId.MECHANIC2_B, JobId.MEISTER2]);
	static isMadogear(jobid) {
		return this.S_JOBS_MADO.has(jobid);
	}

	static isElem(jobid) {
		return (jobid >= 2114 && jobid <= 2125) || (jobid >= 20816 && jobid <= 20820);
	}

	static isAbr(jobid) {
		return jobid >= 20834 && jobid <= 20837;
	}

	static isBionic(jobid) {
		return jobid >= 20848 && jobid <= 20851;
	}

	static isWarp(jobid) {
		return jobid == 45 || jobid == 139;
	}

	/**
	 * @return {string} path to body sprite/action
	 * @param {number} id entity
	 * @param {boolean} sex
	 * @param {number} alternative sprite
	 * @return {string}
	 */
	static getBodyPath(id, sex, alternative = -1, cashMountCostume = false) {
		// TODO: Warp STR file
		if (id === 45) {
			return null;
		}

		// Not visible sprite
		if (id === 111 || id === 139 || id == 2337) {
			return null;
		}

		// PC
		if (DB.isPlayer(id)) {
			// DORAM
			let result = DB.isDoram(id)
				? 'data/sprite/\xb5\xb5\xb6\xf7\xc1\xb7/\xb8\xf6\xc5\xeb/'
				: 'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/\xb8\xf6\xc5\xeb/';
			result += SexTable[sex] + '/';

			if (PACKETVER.value > 20141022 && alternative > 0 && id !== alternative) {
				const use_costume =
					alternative > JobId.COSTUME_SECOND_JOB_START && alternative < JobId.COSTUME_SECOND_JOB_END;

				if (use_costume) {
					result += 'costume_1/';
				}

				result += (cashMountCostume ? ClassTable[id] : ClassTable[alternative]) || ClassTable[0];

				result += '_' + SexTable[sex];

				if (use_costume) {
					result += '_1';
				}
				return result;
			}

			result += ClassTable[id] || ClassTable[0];
			result += '_' + SexTable[sex];

			return result;
		}

		// NPC
		if (DB.isNPC(id)) {
			return 'data/sprite/npc/' + (MonsterTable[id] || MonsterTable[46]).toLowerCase();
		}

		// MERC
		if (DB.isMercenary(id)) {
			// archer - female path | lancer and swordman - male path
			// mercenary entry on monster table have sex path included
			return 'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/\xb8\xf6\xc5\xeb/' + MonsterTable[id];
		}

		// HOMUN
		if (DB.isHomunculus(id)) {
			return 'data/sprite/homun/' + (MonsterTable[id] || MonsterTable[1002]).toLowerCase();
		}

		//
		// OTHER ACTORS
		//
		switch (id) {
			case '11_FALCON':
			case '4034_FALCON':
				// 2nd
				return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xb8\xc5';
			case '4012_FALCON':
				// rebirth
				return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xb8\xc5\x32';
			case '4056_FALCON':
			case '4062_FALCON':
			case '4098_FALCON':
				// 3rd
				return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/owl';
			case '4257_FALCON':
			case '4270_FALCON':
			case '4278_FALCON':
				// 4th
				return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/windhawk_hawk';
			case '4056_WUG':
			case '4062_WUG':
			case '4098_WUG':
				// 3rd
				return 'data/sprite/\xb8\xf3\xbd\xba\xc5\xcd/\xbf\xf6\xb1\xd7';
			case '4257_WUG':
				// 4th
				return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/windhawk_wolf';
			default:
				if (typeof id === 'string') {
					// default for gm or customs
					if (id.includes('_FALCON')) {
						return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xb8\xc5';
					} else if (id.includes('_WUG')) {
						return 'data/sprite/\xb8\xf3\xbd\xba\xc5\xcd/\xbf\xf6\xb1\xd7';
					}
				}
				break;
		}

		// MONSTER
		return 'data/sprite/\xb8\xf3\xbd\xba\xc5\xcd/' + (MonsterTable[id] || MonsterTable[1001]).toLowerCase();
	}

	/**
	 * @return {string} path of admin clothes
	 * @param {boolean} sex
	 */
	static getAdminPath(sex) {
		return (
			'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/\xb8\xf6\xc5\xeb/' +
			SexTable[sex] +
			'/\xbf\xee\xbf\xb5\xc0\xda_' +
			SexTable[sex]
		);
	}

	/**
	 * @return {string} path to body palette
	 * @param {number} id entity
	 * @param {number} pal
	 * @param {boolean} sex
	 */
	static getBodyPalPath(id, pal, sex) {
		if (id === 0 || !(id in PaletteTable)) {
			return null;
		}

		return 'data/palette/\xb8\xf6/' + PaletteTable[id] + '_' + SexTable[sex] + '_' + pal + '.pal';
	}

	/**
	 * @return {string} path to head sprite/action
	 * @param {number} id hair style
	 * @param {number} job job id
	 * @param {boolean} sex
	 * @param {boolean} orcish
	 */
	static getHeadPath(id, job, sex, orcish) {
		// ORC HEAD
		if (orcish) {
			return 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/orcface';
		}

		// DORAM
		if (DB.isDoram(job)) {
			return (
				'data/sprite/\xb5\xb5\xb6\xf7\xc1\xb7/\xb8\xd3\xb8\xae\xc5\xeb/' +
				SexTable[sex] +
				'/' +
				(HairIndexTable[sex + 2][id] || id) +
				'_' +
				SexTable[sex]
			);
		}

		return (
			'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/\xb8\xd3\xb8\xae\xc5\xeb/' +
			SexTable[sex] +
			'/' +
			(HairIndexTable[sex][id] || id) +
			'_' +
			SexTable[sex]
		);
	}

	/**
	 * @return {string} path to head palette
	 * @param {number} id hair style
	 * @param {number} pal id
	 * @param {number} job job id
	 * @param {boolean} sex
	 */
	static getHeadPalPath(id, pal, job, sex) {
		if (job === 4218 || job === 4220) {
			return (
				'data/palette/\xb5\xb5\xb6\xf7\xc1\xb7/\xb8\xd3\xb8\xae/\xb8\xd3\xb8\xae' +
				(HairIndexTable[sex + 2][id] || id) +
				'_' +
				SexTable[sex] +
				'_' +
				pal +
				'.pal'
			);
		}

		return (
			'data/palette/\xb8\xd3\xb8\xae/\xb8\xd3\xb8\xae' +
			(HairIndexTable[sex][id] || id) +
			'_' +
			SexTable[sex] +
			'_' +
			pal +
			'.pal'
		);
	}

	/**
	 * @return {string} path to hat
	 * @param {number} id hair style
	 * @param {boolean} sex
	 */
	static getHatPath(id, sex) {
		if (id === 0 || !(id in HatTable)) {
			return null;
		}

		return 'data/sprite/\xbe\xc7\xbc\xbc\xbb\xe7\xb8\xae/' + SexTable[sex] + '/' + SexTable[sex] + HatTable[id];
	}

	/**
	 * @return {string} path to Robe
	 * @param {number} id robe id
	 * @param {number} job class
	 * @param {boolean} sex
	 */
	static getRobePath(id, job, sex) {
		if (id === 0 || !(id in RobeTable)) {
			return null;
		}

		return (
			'data/sprite/\xb7\xce\xba\xea/' +
			RobeTable[id] +
			'/' +
			SexTable[sex] +
			'/' +
			(ClassTable[job] || ClassTable[0]) +
			'_' +
			SexTable[sex]
		);
	}

	/**
	 * @return {string} path to spr robe on costume root folder
	 * @param {number} id robe id
	 * @param {number} job class
	 * @param {boolean} sex
	 */
	static getRobePathNoSex(id, job, sex) {
		if (id === 0 || !(id in RobeTable)) {
			return null;
		}
		return `data/sprite/\xb7\xce\xba\xea/${RobeTable[id]}/${RobeTable[id]}${DB.isDoram(job) ? '_doram' : ''}`;
	}

	/**
	 * @return {string} Path to pets equipements
	 * @param {number} id (pets)
	 */
	static getPetEquipPath(id) {
		if (id === 0 || !(id in PetAction)) {
			return null;
		}

		return 'data/sprite/' + PetAction[id];
	}

	/**
	 * @return {string} Path to pets equipements
	 * @param {number} id (pets)
	 */
	static getPetIllustPath(id) {
		return 'data/texture/' + (PetIllustration[id] || PetIllustration[1002]);
	}

	/**
	 * is shield checking
	 *
	 * @param {integer} id
	 * @return {boolean} is shield?
	 *
	 * @author MrUnzO
	 */
	static isShield(id) {
		// shields has the following ranges:
		// 2100 - 2199
		// 28900 - 28999
		// 460000 - 460099
		if ((id >= 2100 && id <= 2199) || (id >= 28900 && id <= 28999) || (id >= 460000 && id <= 460099)) {
			return true;
		}
		return false;
	}

	static getPCAttackMotion(job, sex, itemID, isDualWeapon) {
		if (isDualWeapon) {
			switch (job) {
				case JobId.THIEF:
				case JobId.THIEF_H:
					return 5.75;
				case JobId.MERCHANT:
				case JobId.MERCHANT_H:
					return 5.85;
			}
		} else {
			switch (job) {
				case JobId.NOVICE:
				case JobId.NOVICE_H:
				case JobId.NOVICE_B:
				case JobId.SUPERNOVICE:
				case JobId.SUPERNOVICE_B:
				case JobId.SUPERNOVICE2:
				case JobId.SUPERNOVICE2_B:
				case JobId.HYPER_NOVICE:
					switch (sex) {
						case 1:
							return 5.85;
					}
					break;
				case JobId.ASSASSIN:
				case JobId.ASSASSIN_H:
				case JobId.ASSASSIN_B:
				case JobId.GUILLOTINE_CROSS:
				case JobId.GUILLOTINE_CROSS_H:
				case JobId.GUILLOTINE_CROSS_B:
				case JobId.SHADOW_CROSS:
					switch (DB.getWeaponType(itemID)) {
						case WeaponType.KATAR:
						case WeaponType.SHORTSWORD_SHORTSWORD:
						case WeaponType.SWORD_SWORD:
						case WeaponType.AXE_AXE:
						case WeaponType.SHORTSWORD_SWORD:
						case WeaponType.SHORTSWORD_AXE:
						case WeaponType.SWORD_AXE:
							return 3.0;
					}
					break;
			}
		}
		return 6;
	}

	static isDualWeapon(job, sex, weaponType) {
		let dualWeapon = false;

		switch (job) {
			case JobId.GUNSLINGER:
			case JobId.GUNSLINGER_B:
			case JobId.REBELLION:
			case JobId.REBELLION_B:
			case JobId.NIGHT_WATCH: {
				switch (weaponType) {
					case WeaponType.GUN_RIFLE:
					case WeaponType.GUN_GATLING:
					case WeaponType.GUN_SHOTGUN:
					case WeaponType.GUN_GRANADE:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.NINJA:
			case JobId.NINJA_B:
			case JobId.KAGEROU:
			case JobId.KAGEROU_B:
			case JobId.SHINKIRO:
			case JobId.OBORO:
			case JobId.OBORO_B:
			case JobId.SHIRANUI: {
				switch (weaponType) {
					case WeaponType.SYURIKEN:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.GANGSI:
			case JobId.DEATHKNIGHT:
			case JobId.COLLECTOR: {
				// �ӽ�
				break;
			}
			case JobId.TAEKWON:
			case JobId.TAEKWON_B:
			case JobId.STAR:
			case JobId.STAR_B:
			case JobId.STAR2:
			case JobId.STAR2_B:
			case JobId.EMPEROR:
			case JobId.EMPEROR_B:
			case JobId.EMPEROR2:
			case JobId.EMPEROR2_B:
			case JobId.SKY_EMPEROR:
			case JobId.SKY_EMPEROR2: {
				break;
			}
			case JobId.LINKER:
			case JobId.LINKER_B:
			case JobId.REAPER:
			case JobId.REAPER_B:
			case JobId.SOUL_ASCETIC: {
				//case JobId.SOUL_ASCETIC2:??
				switch (weaponType) {
					case WeaponType.SHORTSWORD:
						if (sex == 1) {
							dualWeapon = true;
						} // male
						break;
					case WeaponType.ROD:
					case WeaponType.TWOHANDROD:
						if (sex == 0) {
							dualWeapon = true;
						} // Female
						break;
				}
				break;
			}
			case JobId.SWORDMAN:
			case JobId.SWORDMAN_H:
			case JobId.SWORDMAN_B: {
				switch (weaponType) {
					case WeaponType.TWOHANDSWORD:
					case WeaponType.TWOHANDSPEAR:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.ARCHER:
			case JobId.ARCHER_H:
			case JobId.ARCHER_B: {
				switch (weaponType) {
					case WeaponType.BOW:
						break;
					default:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.THIEF:
			case JobId.THIEF_H:
			case JobId.THIEF_B: {
				switch (weaponType) {
					case WeaponType.BOW:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.MAGICIAN:
			case JobId.MAGICIAN_H:
			case JobId.MAGICIAN_B: {
				switch (weaponType) {
					case WeaponType.TWOHANDROD:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.MERCHANT:
			case JobId.MERCHANT_H:
			case JobId.MERCHANT_B: {
				switch (weaponType) {
					case WeaponType.TWOHANDAXE:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.ACOLYTE:
			case JobId.ACOLYTE_H:
			case JobId.ACOLYTE_B: {
				break;
			}
			case JobId.NOVICE:
			case JobId.NOVICE_H:
			case JobId.NOVICE_B:
			case JobId.SUPERNOVICE:
			case JobId.SUPERNOVICE_B:
			case JobId.SUPERNOVICE2:
			case JobId.SUPERNOVICE2_B:
			case JobId.HYPER_NOVICE: {
				switch (sex) {
					case 0:
						switch (weaponType) {
							case WeaponType.TWOHANDSWORD:
							case WeaponType.TWOHANDAXE:
							case WeaponType.TWOHANDROD:
							case WeaponType.TWOHANDMACE:
								break;
							case WeaponType.SHORTSWORD:
								dualWeapon = true;
								break;
						}
						break;
					case 1:
						switch (weaponType) {
							case WeaponType.TWOHANDSWORD:
							case WeaponType.TWOHANDAXE:
							case WeaponType.TWOHANDROD:
							case WeaponType.TWOHANDMACE:
								dualWeapon = true;
								break;
							case WeaponType.SHORTSWORD:
								break;
						}
						break;
				}
				break;
			}
			case JobId.KNIGHT:
			case JobId.KNIGHT2:
			case JobId.CHICKEN:
			case JobId.KNIGHT_H:
			case JobId.CHICKEN_H:
			case JobId.KNIGHT_B:
			case JobId.CHICKEN_B:
			case JobId.KNIGHT2_H:
			case JobId.KNIGHT2_B:
			case JobId.RUNE_KNIGHT:
			case JobId.RUNE_KNIGHT_H:
			case JobId.RUNE_KNIGHT_B:
			case JobId.RUNE_KNIGHT2:
			case JobId.RUNE_KNIGHT2_H:
			case JobId.RUNE_KNIGHT2_B:
			case JobId.DRAGON_KNIGHT:
			case JobId.DRAGON_KNIGHT2: {
				switch (weaponType) {
					case WeaponType.TWOHANDSPEAR:
					case WeaponAction.TWOHANDSWORD:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.PRIEST:
			case JobId.PRIEST_H:
			case JobId.PRIEST_B:
			case JobId.ARCHBISHOP:
			case JobId.ARCHBISHOP_H:
			case JobId.ARCHBISHOP_B:
			case JobId.CARDINAL: {
				switch (weaponType) {
					case WeaponType.BOOK:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.WIZARD:
			case JobId.WIZARD_H:
			case JobId.WIZARD_B:
			case JobId.WARLOCK:
			case JobId.WARLOCK_H:
			case JobId.WARLOCK_B:
			case JobId.ARCH_MAGE: {
				switch (weaponType) {
					case WeaponType.SHORTSWORD:
						if (sex == 1) {
							dualWeapon = true;
						}
						break;
					case WeaponType.ROD:
					case WeaponType.TWOHANDROD:
						if (sex == 0) {
							dualWeapon = true;
						}
						break;
				}
				break;
			}
			case JobId.BLACKSMITH:
			case JobId.BLACKSMITH_H:
			case JobId.BLACKSMITH_B:
			case JobId.MECHANIC:
			case JobId.MECHANIC_H:
			case JobId.MECHANIC_B:
			case JobId.MECHANIC2:
			case JobId.MECHANIC2_H:
			case JobId.MECHANIC2_B:
			case JobId.MEISTER:
			case JobId.MEISTER2: {
				switch (weaponType) {
					case WeaponType.SWORD:
					case WeaponType.AXE:
					case WeaponType.TWOHANDAXE:
					case WeaponType.MACE:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.ASSASSIN:
			case JobId.ASSASSIN_H:
			case JobId.ASSASSIN_B:
			case JobId.GUILLOTINE_CROSS:
			case JobId.GUILLOTINE_CROSS_H:
			case JobId.GUILLOTINE_CROSS_B:
			case JobId.SHADOW_CROSS: {
				switch (weaponType) {
					case WeaponType.KATAR:
					case WeaponType.SHORTSWORD_SHORTSWORD:
					case WeaponType.SHORTSWORD_SWORD:
					case WeaponType.SHORTSWORD_AXE:
					case WeaponType.SWORD_SWORD:
					case WeaponType.SWORD_AXE:
					case WeaponType.AXE_AXE:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.HUNTER:
			case JobId.HUNTER_H:
			case JobId.HUNTER_B:
			case JobId.RANGER:
			case JobId.RANGER_H:
			case JobId.RANGER_B:
			case JobId.RANGER2:
			case JobId.RANGER2_H:
			case JobId.RANGER2_B:
			case JobId.WINDHAWK:
			case JobId.WINDHAWK2: {
				switch (weaponType) {
					case WeaponType.BOW:
						dualWeapon = true;
						break;
				}
				break;
			}
			case JobId.SAGE:
			case JobId.SAGE_H:
			case JobId.SAGE_B:
			case JobId.SORCERER:
			case JobId.SORCERER_H:
			case JobId.SORCERER_B:
			case JobId.ELEMENTAL_MASTER:
				{
					switch (weaponType) {
						case WeaponType.BOOK:
						case WeaponType.ROD:
						case WeaponType.TWOHANDROD:
						case WeaponType.TWOHANDSPEAR: // ��ս������� ���� ���̵� ����� �����Ǹ� �����.
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.ALCHEMIST:
			case JobId.ALCHEMIST_H:
			case JobId.ALCHEMIST_B:
			case JobId.GENETIC:
			case JobId.GENETIC_H:
			case JobId.GENETIC_B:
			case JobId.BIOLO:
				{
					switch (weaponType) {
						case WeaponType.SWORD:
						case WeaponType.AXE:
						case WeaponType.TWOHANDAXE:
						case WeaponType.MACE:
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.CRUSADER:
			case JobId.CRUSADER_H:
			case JobId.CRUSADER_B:
			case JobId.CHICKEN2:
			case JobId.CHICKEN2_H:
			case JobId.CHICKEN2_B:
			case JobId.CRUSADER2:
			case JobId.CRUSADER2_H:
			case JobId.CRUSADER2_B:
			case JobId.ROYAL_GUARD:
			case JobId.ROYAL_GUARD_H:
			case JobId.ROYAL_GUARD_B:
			case JobId.ROYAL_GUARD2:
			case JobId.ROYAL_GUARD2_H:
			case JobId.ROYAL_GUARD2_B:
			case JobId.IMPERIAL_GUARD:
			case JobId.IMPERIAL_GUARD2:
				{
					switch (weaponType) {
						case WeaponType.SPEAR:
						case WeaponType.TWOHANDSPEAR:
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.MONK:
			case JobId.MONK_H:
			case JobId.MONK_B:
			case JobId.SURA:
			case JobId.SURA_H:
			case JobId.SURA_B:
			case JobId.INQUISITOR:
				{
					switch (weaponType) {
						case WeaponType.KNUKLE:
						case WeaponType.NONE:
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.ROGUE:
			case JobId.ROGUE_H:
			case JobId.ROGUE_B:
			case JobId.SHADOW_CHASER:
			case JobId.SHADOW_CHASER_H:
			case JobId.SHADOW_CHASER_B:
			case JobId.ABYSS_CHASER:
				{
					switch (weaponType) {
						case WeaponType.BOW:
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.BARD:
			case JobId.BARD_H:
			case JobId.BARD_B:
			case JobId.MINSTREL:
			case JobId.MINSTREL_H:
			case JobId.MINSTREL_B:
			case JobId.TROUBADOUR:
			case JobId.DANCER:
			case JobId.DANCER_H:
			case JobId.DANCER_B:
			case JobId.WANDERER:
			case JobId.WANDERER_H:
			case JobId.WANDERER_B:
			case JobId.TROUVERE:
				{
					switch (weaponType) {
						case WeaponType.BOW:
							dualWeapon = true;
							break;
					}
				}
				break;
			case JobId.DO_SUMMONER1:
			case JobId.DO_SUMMONER_B1:
			case JobId.SPIRIT_HANDLER:
				break;
		}
		return dualWeapon;
	}

	static getWeaponType(itemID, realType = false, considerDualHandIds = false) {
		const id = Number(itemID);

		if (isNaN(id) || id < 0) {
			return WeaponType.NONE;
		}

		if (realType && id in WeaponTypeExpansion) {
			return WeaponTypeExpansion[id];
		}

		if (considerDualHandIds && id <= WeaponType.SWORD_AXE) {
			return id;
		}

		// if itemID is lesser then WeaponType.MAX, return the itemID
		if (id < WeaponType.MAX) {
			return id;
		}

		// look for classnum in ItemTable
		if (id in ItemTable && 'ClassNum' in ItemTable[id]) {
			const classNum = ItemTable[id].ClassNum;

			// look for classNum in WeaponTypeExpansion
			if (classNum in WeaponTypeExpansion) {
				return WeaponTypeExpansion[classNum];
			}

			// ClassNUm is the real weapon type
			return classNum;
		} else {
			// if itemID is not in ItemTable, try to find the corresponding weapon type based on the range id
			const ranges = [
				{ min: 1100, max: 1149, type: WeaponType.SWORD },
				{ min: 1150, max: 1199, type: WeaponType.TWOHANDSWORD },
				{ min: 1200, max: 1249, type: WeaponType.SHORTSWORD },
				{ min: 1250, max: 1299, type: WeaponType.KATAR },
				{ min: 1300, max: 1349, type: WeaponType.AXE },
				{ min: 1350, max: 1399, type: WeaponType.TWOHANDAXE },
				{ min: 1400, max: 1449, type: WeaponType.SPEAR },
				{ min: 1450, max: 1499, type: WeaponType.TWOHANDSPEAR },
				{ min: 1500, max: 1549, type: WeaponType.MACE },
				{ min: 1550, max: 1599, type: WeaponType.BOOK },
				{ min: 1600, max: 1699, type: WeaponType.ROD },
				{ min: 1700, max: 1749, type: WeaponType.BOW },
				{ min: 1750, max: 1799, type: WeaponType.NONE },
				{ min: 1800, max: 1849, type: WeaponType.KNUKLE },
				{ min: 1900, max: 1949, type: WeaponType.INSTRUMENT },
				{ min: 1950, max: 1999, type: WeaponType.WHIP },
				{ min: 20000, max: 20999, type: WeaponType.TWOHANDROD },
				{ min: 13000, max: 13099, type: WeaponType.SHORTSWORD },
				{ min: 13100, max: 13149, type: WeaponType.GUN_HANDGUN },
				{ min: 13150, max: 13199, type: WeaponType.GUN_RIFLE },
				{ min: 13300, max: 13399, type: WeaponType.SYURIKEN },
				{ min: 13400, max: 13499, type: WeaponType.SWORD },
				{ min: 18100, max: 18499, type: WeaponType.BOW },
				{ min: 21000, max: 21999, type: WeaponType.TWOHANDSWORD }
			];

			// Find the corresponding range
			for (const range of ranges) {
				if (id >= range.min && id <= range.max) {
					return range.type;
				}
			}

			const gunGatling = [13157, 13158, 13159, 13172, 13177];
			if (gunGatling.indexOf(id) > -1) {
				return WeaponType.GUN_GATLING;
			}

			const gunShotGun = [13154, 13155, 13156, 13167, 13168, 13169, 13173, 13178];
			if (gunShotGun.indexOf(id) > -1) {
				return WeaponType.GUN_SHOTGUN;
			}

			const gunGranade = [13160, 13161, 13162, 13174, 13179];
			if (gunGranade.indexOf(id) > -1) {
				return WeaponType.GUN_GRANADE;
			}
		}

		// if itemID is not in any range, return NONE
		return WeaponType.NONE;
	}

	/**
	 * @return {string} Path to shield
	 * @param {number} id shield
	 * @param {number} job class
	 * @param {boolean} sex
	 */
	static getShieldPath(id, job, sex) {
		if (id === 0) {
			return null;
		}

		// Dual weapon (based on range id)
		if (!DB.isShield(id)) {
			return DB.getWeaponPath(id, job, sex);
		}

		const baseClass = WeaponJobTable[job] || WeaponJobTable[0];

		// ItemID to View Id
		if (id in ItemTable && 'ClassNum' in ItemTable[id]) {
			id = ItemTable[id].ClassNum;
		}

		return (
			'data/sprite/\xb9\xe6\xc6\xd0/' +
			baseClass +
			'/' +
			baseClass +
			'_' +
			SexTable[sex] +
			'_' +
			(ShieldTable[id] || ShieldTable[1])
		);
	}

	/**
	 * @return {string} Path to weapon
	 * @param {number} id weapon
	 * @param {number} job class
	 * @param {boolean} sex
	 */
	static getWeaponPath(id, job, sex, leftid = false) {
		if (id === 0) {
			return null;
		}

		const baseClass = WeaponJobTable[job] || WeaponJobTable[0];

		id = DB.getWeaponType(id);

		// TODO: CHECK IF THIS IS CORRECT
		if (leftid) {
			if (leftid in ItemTable && 'ClassNum' in ItemTable[leftid]) {
				leftid = ItemTable[leftid].ClassNum;
			}

			// Create dualhand Id
			const right = Object.keys(WeaponType).find(key => WeaponType[key] === id);
			const left = Object.keys(WeaponType).find(key => WeaponType[key] === leftid);
			if (right && left) {
				id = WeaponType[right + '_' + left];
			}
		}

		return (
			'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/' +
			baseClass +
			'/' +
			baseClass +
			'_' +
			SexTable[sex] +
			(WeaponTable[id] || '_' + id)
		);
	}

	/**
	 * @return {string} Path to weapon trail
	 * @param {number} id weapon
	 * @param {number} job class
	 * @param {boolean} sex
	 */
	static getWeaponTrail(id, job, sex) {
		if (id === 0) {
			return null;
		}

		const baseClass = WeaponJobTable[job] || WeaponJobTable[0];

		const realId = DB.getWeaponType(id, true, true);

		return (
			'data/sprite/\xc0\xce\xb0\xa3\xc1\xb7/' +
			baseClass +
			'/' +
			baseClass +
			'_' +
			SexTable[sex] +
			WeaponTrailTable[realId]
		);
	}

	/**
	 * @param {number} cart id
	 */
	static getCartPath(num) {
		const id = Math.max(Math.min(num, 13), 0); //cap 0-13
		return [
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbd\xb4\xb3\xeb\xbc\xd5\xbc\xf6\xb7\xb9',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb9',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb91',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb92',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb93',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb94',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb95',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb96',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb97',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xd5\xbc\xf6\xb7\xb98',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xb1\xb9\xb0\xbb\xf3\xc0\xda\xc4\xab\xc6\xae',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xc6\xf7\xb8\xb5\xbd\xc6\xc0\xba\xc4\xab\xc6\xae',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xc6\xf7\xb8\xb5\xc4\xab\xc6\xae',
			'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xb8\xb6\xb5\xb5\xc4\xab\xc6\xae'
		][id];
	}

	/**
	 * @return {string} Path to weapon sound
	 * @param {number} weapon id
	 */
	static getWeaponSound(id) {
		const type = DB.getWeaponType(id, true);
		return WeaponSoundTable[type];
	}

	/**
	 * @return {string} Path to eapon sound
	 * @param {number} weapon id
	 */
	static getWeaponHitSound(id) {
		const type = DB.getWeaponType(id, true, true);

		const hitSound = WeaponHitSoundTable[type];

		// if array return random item
		if (Array.isArray(hitSound)) {
			return hitSound[Math.floor(Math.random() * hitSound.length)];
		}

		return hitSound;
	}

	/**
	 * @return {string} Path to eapon sound [MrUnzO]
	 * @param {number} weapon id
	 */
	static getJobHitSound(job_id) {
		if (!job_id) {
			return JobHitSoundTable[0];
		}

		return JobHitSoundTable[job_id] || JobHitSoundTable[0];
	}

	/**
	 * @return {number} weapon viewid
	 * @param {number} id weapon
	 */
	static getWeaponViewID(id) {
		// validate if is number
		if (isNaN(id)) {
			return 0;
		}

		// if id is 0, return 0
		if (id === 0) {
			return 0;
		}

		// if less then weapon type.MAX, return the id
		if (id < WeaponType.MAX) {
			return id;
		}

		// try to get view from classnum in ItemTable
		if (id in ItemTable && 'ClassNum' in ItemTable[id]) {
			return ItemTable[id].ClassNum;
		}

		// all cases failed, return weapon type to use base sprite
		return DB.getWeaponType(id);
	}

	/**
	 * @return {number} weapon action frame
	 * @param {number} id weapon
	 * @param {number} job
	 * @param {number} sex
	 */
	static getWeaponAction(id, job, sex) {
		const type = DB.getWeaponType(id, true);

		if (job in WeaponAction) {
			if (WeaponAction[job] instanceof Array) {
				if (type in WeaponAction[job][sex]) {
					return WeaponAction[job][sex][type];
				}
			} else if (type in WeaponAction[job]) {
				return WeaponAction[job][type];
			}
		}

		return 0;
	}

	/**
	 * @return {{frame: number, length: number}|null} attack slice for jobs with combined action sheets
	 * @param {number} job
	 * @param {number} weapon
	 */
	static getAttackSlice(job, weapon) {
		const type = DB.getWeaponType(weapon, true);
		switch (job) {
			case JobId.MONK:
			case JobId.MONK_H:
			case JobId.MONK_B:
			case JobId.SURA:
			case JobId.SURA_H:
			case JobId.SURA_B:
			case JobId.SURA_2ND:
			case JobId.INQUISITOR:
			case JobId.INQUISITOR_RIDING:
				if (type === WeaponType.KNUKLE || type === WeaponType.NONE) {
					return { frame: 0, length: 5 };
				}
				break;
			case JobId.DO_SUMMONER:
			case JobId.DO_SUMMONER_B:
			case JobId.SPIRIT_HANDLER:
			case JobId.SPIRIT_HANDLER_RIDING1:
				return { frame: 0, length: 4 };
		}
		return null;
	}

	static mountWeapon(weaponID, shieldID) {
		const _weapon = DB.getWeaponType(weaponID, true);
		const _shield = DB.getWeaponType(shieldID, true);

		let weapon;

		const viewId = _weapon + _shield;

		switch (viewId) {
			case 2:
				weapon = WeaponType.SHORTSWORD_SHORTSWORD;
				break;
			case 3:
				weapon = WeaponType.SHORTSWORD_SWORD;
				break;
			case 4:
				weapon = WeaponType.SWORD_SWORD;
				break;
			case 7:
				weapon = WeaponType.SHORTSWORD_AXE;
				break;
			case 8:
				weapon = WeaponType.SWORD_AXE;
				break;
			case 12:
				weapon = WeaponType.AXE_AXE;
				break;
			default:
				weapon = viewId;
				break;
		}

		return weapon;
	}

	static isBow(weaponType) {
		return (
			weaponType == WeaponType.BOW ||
			weaponType == WeaponType.CrossBow ||
			weaponType == WeaponType.Arbalest ||
			weaponType == WeaponType.Kakkung ||
			weaponType == WeaponType.Hunter_Bow ||
			weaponType == WeaponType.Bow_Of_Rudra
		);
	}

	static isKatar(weaponType) {
		return weaponType == WeaponType.KATAR;
	}

	/**
	 * Get back informations from id
	 *
	 * @param {number} item id
	 * @return {object} item
	 */
	static getItemInfo(itemid) {
		const item = ItemTable[itemid] || unknownItem;

		if (!item._decoded) {
			item.identifiedDescriptionName =
				item.identifiedDescriptionName instanceof Array
					? item.identifiedDescriptionName.join('\n')
					: item.identifiedDescriptionName;
			item.unidentifiedDescriptionName =
				item.unidentifiedDescriptionName instanceof Array
					? item.unidentifiedDescriptionName.join('\n')
					: item.unidentifiedDescriptionName;
			item.prefixName = item.prefixName || '';
			item.isPostfix = item.isPostfix || false;
			item.processitemlist =
				item.processitemlist && item.processitemlist instanceof Array ? item.processitemlist.join('\n') : '';
			item._decoded = true;
		}

		return item;
	}

	/**
	 * Get back item path
	 *
	 * @param {number} item id
	 * @param {boolean} is identify
	 * @return {string} path
	 */
	static getItemPath(itemid, identify) {
		const it = DB.getItemInfo(itemid);
		return (
			'data/sprite/\xbe\xc6\xc0\xcc\xc5\xdb/' +
			(identify ? it.identifiedResourceName : it.unidentifiedResourceName)
		);
	}

	/**
	 * Get full item name
	 *
	 * @param {object} item - The item object containing details about the item.
	 * @param {object} [options] - Optional parameters to customize the output.
	 * @param {boolean} [options.showItemRefine=true] - Whether to show the refining level of the item.
	 * @param {boolean} [options.showItemGrade=true] - Whether to show the grade of the item.
	 * @param {boolean} [options.showItemSlots=true] - Whether to show the number of slots on the item.
	 * @param {boolean} [options.showItemPrefix=true] - Whether to show the prefix of the item.
	 * @param {boolean} [options.showItemPostfix=true] - Whether to show the postfix of the item.
	 * @param {boolean} [options.showItemOptions=true] - Whether to show the number of options on the item.
	 * @return {string} - The full name of the item with all applicable details.
	 */
	static getItemName(item, options = {}) {
		const {
			showItemRefine = true,
			showItemGrade = true,
			showItemSlots = true,
			showItemPrefix = true,
			showItemPostfix = true,
			showItemOptions = true
		} = options;

		const it = DB.getItemInfo(item.ITID);
		let str = '';
		let prefix = '';
		let postfix = '';
		let showprefix = false;
		let showpostfix = false;

		if (!item.IsIdentified) {
			return it.unidentifiedDisplayName;
		}

		if (item.RefiningLevel && showItemRefine) {
			str = '+' + item.RefiningLevel + ' ';
		}

		if (item.enchantgrade && showItemGrade) {
			const list = ['', 'D', 'C', 'B', 'A'];
			str += '[' + list[item.enchantgrade] + '] ';
		}

		//Hide slots for forged weapons
		let showslots = true;
		if (item.slot) {
			let very = '';
			let name = '';
			let elem = '';

			switch (item.slot.card1) {
				case 0x00ff: {
					// FORGE
					showslots = false;
					if (item.slot.card2 >= 3840) {
						very = MsgStringTable[461]; //Very Very Very Strong
					} else if (item.slot.card2 >= 2560) {
						very = MsgStringTable[460]; //Very Very Strong
					} else if (item.slot.card2 >= 1024) {
						very = MsgStringTable[459]; //Very Strong
					}
					switch (Math.abs(item.slot.card2 % 10)) {
						case 1:
							elem = MsgStringTable[452];
							break; // 's Ice
						case 2:
							elem = MsgStringTable[454];
							break; // 's Earth
						case 3:
							elem = MsgStringTable[451];
							break; // 's Fire
						case 4:
							elem = MsgStringTable[453];
							break; // 's Wind
						default:
							elem = MsgStringTable[450];
							break; // 's
					}

					const GID = (item.slot.card4 << 16) + item.slot.card3;
					name = '<font color="red" class="owner-' + GID + '">Unknown</font>';
					if (DB.CNameTable[GID] && DB.CNameTable[GID] !== 'Unknown') {
						name = '<font color="#87cefa" class="owner-' + GID + '">' + DB.CNameTable[GID] + '</font>';
					} else {
						DB.UpdateOwnerName[GID] = function (pkt) {
							delete DB.UpdateOwnerName[pkt.GID];
							setTimeout(() => {
								const elements = document.querySelectorAll('.owner-' + pkt.GID);
								for (let i = 0; i < elements.length; i++) {
									elements[i].innerText = pkt.CName;
									elements[i].style.color = 'blue';
								}
							}, 1000);
						};
						DB.getNameByGID(GID);
					}

					str += very + ' ' + name + elem + ' ';
					break;
				}
				case 0x00fe: // CREATE
					elem = MsgStringTable[450];
					break;
				case 0xff00: // PET
					break;
				// Show card prefix
				default: {
					const list = ['', 'Double ', 'Triple ', 'Quadruple '];
					const cards = {};
					const cardList = [];

					for (let i = 1; i <= 4; ++i) {
						const card = item.slot['card' + i];

						if (card) {
							//store order
							if (!cardList.includes(card)) {
								cardList.push(card);
							}

							//store details
							if (cards[card]) {
								cards[card].count++;
							} else {
								cards[card] = {};
								cards[card].isPostfix = DB.getItemInfo(card).isPostfix;
								cards[card].prefixName = DB.getItemInfo(card).prefixName;
								cards[card].count = 0;
							}
						}
					}

					//create prefixes and postfixes in order
					cardList.forEach(card => {
						if (cards[card].isPostfix) {
							postfix += ' ' + list[cards[card].count] + cards[card].prefixName;
							showpostfix = true;
						} else {
							prefix += list[cards[card].count] + cards[card].prefixName + ' ';
							showprefix = true;
						}
					});
					break;
				}
			}
			switch (item.slot.card4) {
				case 0x1: //BELOVED PET
					showslots = false;
					str = DB.getMessage(756) + ' ' + str;
					break;
			}
		}

		if (showprefix && showItemPrefix) {
			str += prefix;
		}

		str += it.identifiedDisplayName;

		if (showpostfix && showItemPostfix) {
			str += postfix;
		}

		if (it.slotCount > 0 && showslots && showItemSlots) {
			str += ' [' + it.slotCount + ']';
		}

		if (item.Options && showItemOptions) {
			const numOfOptions = item.Options.filter(Option => Option?.index && Option?.index !== 0).length;
			if (numOfOptions) {
				str += ' [' + numOfOptions + ' Option]';
			}
		}

		return str;
	}

	/**
	 * Get random option name
	 *
	 * @param {integer} id
	 * @return {string} item full name
	 */
	static getOptionName(id) {
		if (!(id in RandomOption)) {
			return 'UNKNOWN RANDOM OPTION';
		}
		return RandomOption[id];
	}
	/**
	 * Get a message from msgstringtable
	 *
	 * @param {number} message id
	 * @param {string} optional string to show if the text isn't defined
	 * @return {string} message
	 */
	static getMessage(id, defaultText) {
		if (!(id in MsgStringTable)) {
			return defaultText !== undefined ? defaultText : 'NO MSG ' + id;
		}

		return MsgStringTable[id];
	}

	/**
	 * Get a message string from the MsgEmotionCSV
	 *
	 * @param {string} key - The key to search for
	 * @return {string|null} - The value associated with the given key, or null if not found.
	 */
	static getMessageEmotionCSV(keyOrIndex) {
		if (typeof keyOrIndex === 'number') {
			// Get keys as array just for this lookup
			const keys = Object.keys(MsgEmotionCSV);
			const key = keys[keyOrIndex];
			return key ? MsgEmotionCSV[key] : null;
		}
		// string key lookup
		return MsgEmotionCSV[keyOrIndex] ?? null;
	}

	/**
	 * Get Skill Description from DB
	 *
	 * @param {number} skill id
	 */
	static getSkillDescription(id) {
		return SkillDescription[id] || '...';
	}

	/**
	 * @param {string} filename
	 * @return {object}
	 */
	static getMap(mapname) {
		const map = mapname.replace('.gat', '.rsw');

		return MapTable[map] || null;
	}

	/**
	 * Get a message from msgstringtable
	 *
	 * @param {string} mapname
	 * @param {string} default name if not found
	 * @return {string} map location
	 */
	static getMapName(mapname, defaultName) {
		if (!mapname) {
			return typeof defaultName === 'undefined' ? DB.getMessage(187) : defaultName;
		}
		const map = mapname.replace('.gat', '.rsw');

		if (!(map in MapTable) || !MapTable[map].name) {
			return typeof defaultName === 'undefined' ? DB.getMessage(187) : defaultName;
		}

		return MapTable[map].name;
	}

	/**
	 * Get monster name
	 *
	 * @param {number} job id
	 */
	static getMonsterName(job) {
		return MonsterNameTable[job] ?? 'Unknown';
	}

	/**
	 * Get back town information by mapname
	 * @param {number} efst id
	 */
	static getTownInfo(mapname) {
		return TownInfo[mapname] || null;
	}

	/**
	 * Get back map information by mapname
	 * @param {number} efst id
	 */
	static getMapInfo(mapname) {
		return MapInfo[mapname] || null;
	}

	/**
	 * Get the whole Laphine Synthesis Table
	 * @returns LaphineSysTable
	 */
	static getLaphineSysList() {
		return LaphineSysTable;
	}

	/**
	 * Get Laphine Synthesis information by itemId
	 * @param {number} itemId
	 * @returns LaphineSysTable[key] if itemId found
	 */
	static getLaphineSysInfoById(itemId) {
		for (const key in LaphineSysTable) {
			if (LaphineSysTable[key].ItemID === itemId) {
				return LaphineSysTable[key];
			}
		}
		return null;
	}

	/**
	 * Retrieves the Laphine Upgrade Table.
	 *
	 * @return {Object} The Laphine Upgrade Table.
	 */
	static getLaphineUpgList() {
		return LaphineUpgTable;
	}

	/**
	 * Retrieves the Laphine Upgrade information by the given item ID.
	 *
	 * @param {number} itemId - The ID of the item to search for.
	 * @return {Object|null} The Laphine Upgrade information if found, or null if not found.
	 */
	static getLaphineUpgInfoById(itemId) {
		for (const key in LaphineUpgTable) {
			if (LaphineUpgTable[key].ItemID === itemId) {
				return LaphineUpgTable[key];
			}
		}
		return null;
	}

	/**
	 * Retrieves the Enchant group info by the given group ID.
	 *
	 * @param {number} groupId - The Enchant group ID.
	 * @return {Object|null} The Enchant group info if found, or null.
	 */
	static getEnchantGroup(groupId) {
		return EnchantListTable[groupId] || null;
	}

	/**
	 * Retrieves all Enchant groups.
	 *
	 * @return {Object} Enchant group table.
	 */
	static getEnchantGroups() {
		return EnchantListTable;
	}

	/**
	 * Retrieves Enchant slot info by group and slot.
	 *
	 * @param {number} groupId - The Enchant group ID.
	 * @param {number} slotNum - Slot index.
	 * @return {Object|null} Enchant slot info if found.
	 */
	static getEnchantSlot(groupId, slotNum) {
		const group = EnchantListTable[groupId];
		if (!group || !group.slots) {
			return null;
		}
		return group.slots[slotNum] || null;
	}

	/**
	 * Returns the item ID associated with a given base item.
	 *
	 * @param {string} baseItem - The base item to get the ID for.
	 * @return {number} The item ID associated with the base item.
	 */
	static getItemIdfromBase(baseItem) {
		return ItemDBNameTbl[baseItem];
	}

	/**
	 * Retrieves the base item associated with a given item ID.
	 *
	 * @param {number} itemId - The ID of the item to search for.
	 * @return {string|null} The base item associated with the item ID, or null if not found.
	 */
	static getBasefromItemID(itemId) {
		for (const key in ItemDBNameTbl) {
			if (ItemDBNameTbl[key] === itemId) {
				return key;
			}
		}
		return null; // Return null if not found
	}

	/**
	 * Finds the reform list associated with a given item ID.
	 *
	 * @param {number} itemId - The ID of the item to search for.
	 * @return {Object|null} The reform list associated with the item ID, or null if not found.
	 */
	static findReformListByItemID(itemId) {
		// First, get the base item from the item ID
		const baseItem = DB.getBasefromItemID(itemId);

		// Check if the base item was found and if it exists as a key in ReformItemList
		if (baseItem && ItemReformTable.ReformItemList.hasOwnProperty(baseItem)) {
			return ItemReformTable.ReformItemList[baseItem];
		} else {
			return null; // Return null if not found
		}
	}

	/**
	 * Retrieves the reform information for a given reform ID.
	 *
	 * @param {string} reformId - The ID of the reform to retrieve information for.
	 * @return {Object|null} The reform information object if found, or null if not found.
	 */
	static getReformInfo(reformId) {
		// Check if the reformId exists in the ReformInfo
		if (ItemReformTable.ReformInfo[reformId]) {
			return ItemReformTable.ReformInfo[reformId];
		} else {
			return null; // Return null if the reform ID is not found
		}
	}

	/**
	 * Retrieves information for all reform IDs in the provided array.
	 *
	 * @param {Array} reformIds - An array of reform IDs to retrieve information for.
	 * @return {Array} An array of reform information objects.
	 */
	static getAllReformInfos(reformIds) {
		const reformInfos = [];

		for (let i = 0; i < reformIds.length; i++) {
			const reformId = reformIds[i];
			const reformInfo = DB.getReformInfo(reformId);

			if (reformInfo) {
				reformInfos.push(reformInfo);
			} else {
				console.error('Reform Info not found for reform ID:', reformId);
			}
		}

		return reformInfos;
	}

	/**
	 * Finds a signboard in the given map based on the provided coordinates.
	 *
	 * @param {string} mapname - The name of the map to search in.
	 * @param {number} x - The x-coordinate of the signboard.
	 * @param {number} y - The y-coordinate of the signboard.
	 * @param {number} [tolerance=1] - The tolerance value for matching coordinates.
	 * @return {Object|null} The signboard object if found, or null if not found.
	 */
	static findSignboard(mapname, x, y, tolerance = 1) {
		const mapData = SignBoardTable[mapname];
		if (mapData) {
			for (const xKey in mapData) {
				if (Math.abs(x - xKey) <= tolerance) {
					const yData = mapData[xKey];
					for (const yKey in yData) {
						if (Math.abs(y - yKey) <= tolerance) {
							return yData[yKey];
						}
					}
				}
			}
		}
		return null;
	}

	/**
	 * Get all signboards for a specific map
	 * @param {string} mapname - Map name
	 * @return {Object} Signboard data for the map
	 */
	static getAllSignboardsForMap(mapname) {
		return SignBoardTable[mapname] || null;
	}

	/**
	 * Retrieves the translated signboard description based on the provided description.
	 *
	 * @param {string} description - The description of the signboard.
	 * @return {string} The translated signboard description if found, otherwise the original description.
	 */
	static getTranslatedSignBoard(description) {
		return SignBoardTranslatedTable[description] || description;
	}

	static getRandomJoke() {
		return JokeTable[Math.round(Math.random() * (JokeTable.length - 1))];
	}

	static getRandomScream() {
		return ScreamTable[Math.round(Math.random() * (ScreamTable.length - 1))];
	}

	/**
	 * @type {Object<number, Function[]>} Pending name-resolution callbacks keyed by GID
	 * @private
	 */
	static _nameCallbacks = {};

	/**
	 * @type {Object<number, ReturnType<typeof setTimeout>>} Per-GID TTL timers
	 * @private
	 */
	static _nameCallbackTimers = {};

	/**
	 * Request a character name by GID from the server.
	 * Returns a Promise that resolves with the name once the server responds.
	 * If the name is already cached, resolves immediately.
	 * If a request is already in-flight, piggybacks on the pending response.
	 *
	 * If the server never responds within `ttl` milliseconds the pending
	 * callbacks are resolved with 'Unknown', the CNameTable entry is cleared
	 * (so the next caller can retry), and all references are freed.
	 *
	 * @param {number} GID
	 * @param {number} [ttl=8000] - Milliseconds before treating the request as timed-out
	 * @returns {Promise<string>} resolves with the character name
	 */
	static getNameByGID(GID, ttl = 8000) {
		// Already have a real name cached — resolve immediately
		if (DB.CNameTable[GID] && DB.CNameTable[GID] !== 'Unknown') {
			return Promise.resolve(DB.CNameTable[GID]);
		}

		// Already requested (pending 'Unknown') — don't re-send, just
		// return a promise that piggybacks on the existing in-flight request
		if (DB.CNameTable[GID] === 'Unknown') {
			return new Promise(resolve => {
				if (!DB._nameCallbacks[GID]) {
					DB._nameCallbacks[GID] = [];
				}
				DB._nameCallbacks[GID].push(resolve);
			});
		}

		// New request — send to server
		let pkt;
		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.REQNAME_BYGID2();
		} else {
			pkt = new PACKET.CZ.REQNAME_BYGID();
		}
		pkt.GID = GID;
		Network.sendPacket(pkt);
		DB.CNameTable[pkt.GID] = 'Unknown';

		// Arm a TTL timer so callbacks are never leaked if the server is silent
		DB._nameCallbackTimers[GID] = setTimeout(() => {
			// Only act if the entry hasn't already been resolved by the server
			if (DB._nameCallbacks[GID]) {
				// Clear the 'Unknown' sentinel so the next caller can retry
				delete DB.CNameTable[GID];
				DB._resolveNameCallbacks(GID, 'Unknown');
			}
			delete DB._nameCallbackTimers[GID];
		}, ttl);

		return new Promise(resolve => {
			if (!DB._nameCallbacks[GID]) {
				DB._nameCallbacks[GID] = [];
			}
			DB._nameCallbacks[GID].push(resolve);
		});
	}

	/**
	 * Notify all pending name-resolution callbacks for a GID.
	 * Called internally when a name response packet arrives.
	 *
	 * @param {number} GID
	 * @param {string} name
	 * @private
	 */
	static _resolveNameCallbacks(GID, name) {
		// Cancel the TTL timer if the server replied before it fired
		if (DB._nameCallbackTimers[GID]) {
			clearTimeout(DB._nameCallbackTimers[GID]);
			delete DB._nameCallbackTimers[GID];
		}
		const callbacks = DB._nameCallbacks[GID];
		if (callbacks) {
			delete DB._nameCallbacks[GID];
			for (const cb of callbacks) {
				cb(name);
			}
		}
	}

	/**
	 * Get Pet talk message
	 *
	 * @param {integer} message data combined with mob id, hungryState, actionState
	 * @return {string} pet telk sentence
	 *
	 * @author MrUnzO
	 */
	static getPetTalk(data) {
		// Structure:
		// Examaple: 1013010
		// 1013  |      01     |     0
		// mobID | hungryState | actionState
		const mobId = parseInt(data.toString().substring(0, 4)) || 1001;
		const hungryState = parseInt(data.toString().substring(4, 6)) || 0;
		const actionState = parseInt(data.toString().substring(6, 7)) || 0;

		if (hungryState >= Object.keys(PetHungryState).length || actionState >= Object.keys(PetMessageConst).length) {
			return false;
		}

		let mobName, hungryText, actionText;
		if (mobId && mobId >= 1000 && mobId < 4000) {
			mobName = (MonsterTable[mobId] || MonsterTable[1001]).toLowerCase();
		}
		if (hungryState !== null && hungryState !== undefined) {
			hungryText = DB.getPetHungryText(parseInt(hungryState));
		}

		if (actionState !== null && actionState !== undefined) {
			actionText = DB.getPetActText(parseInt(actionState));
		}

		if (!mobName || !hungryText || !actionText) {
			return false;
		}

		if (
			PetTalkTable &&
			PetTalkTable[mobName] &&
			PetTalkTable[mobName][hungryText] &&
			PetTalkTable[mobName][hungryText][actionText]
		) {
			let rnd = 0;
			if (PetTalkTable[mobName][hungryText][actionText] instanceof Array) {
				rnd = parseInt((Math.random() * 100) % PetTalkTable[mobName][hungryText][actionText].length);
				return TextEncoding.decodeString(PetTalkTable[mobName][hungryText][actionText][rnd]);
			}
			return TextEncoding.decodeString(PetTalkTable[mobName][hungryText][actionText]);
		}

		return false;
	}

	/**
	 * Get Pet Hungry state
	 *
	 * @param {integer} hunger
	 * @return {integer} hunger state
	 *
	 * @author MrUnzO
	 */
	static getPetHungryState(hunger) {
		if (!hunger) {
			return 0;
		}
		if (hunger > 90 && hunger <= 100) {
			return PetHungryState.PET_FULL;
		} else if (hunger > 75 && hunger <= 90) {
			return PetHungryState.PET_ENOUGH;
		} else if (hunger > 25 && hunger <= 75) {
			return PetHungryState.PET_SATISFIED;
		} else if (hunger > 10 && hunger <= 25) {
			return PetHungryState.PET_HUNGRY;
		} else if (hunger >= 0 && hunger <= 10) {
			return PetHungryState.PET_HUNGER;
		}
		return 0;
	}

	/**
	 * Get Pet Friendly state
	 *
	 * @param {integer} friendly
	 * @return {integer} friendly state
	 *
	 * @author MrUnzO
	 */
	static getPetFriendlyState(friendly) {
		if (!friendly) {
			return 0;
		}
		if (friendly > 900 && friendly <= 1000) {
			return PetFriendlyState.PET_FAMILIAR;
		} else if (friendly > 750 && friendly <= 900) {
			return PetFriendlyState.PET_FRIENDLY;
		} else if (friendly > 250 && friendly <= 750) {
			return PetFriendlyState.PET_NORMAL;
		} else if (friendly > 100 && friendly <= 250) {
			return PetFriendlyState.PET_AWKWARD;
		} else if (friendly >= 0 && friendly <= 100) {
			return PetFriendlyState.PET_ASHAMED;
		}
		return 0;
	}

	/**
	 * Get Pet Action text
	 *
	 * @param {integer} action
	 * @return {string} action string
	 *
	 * @author MrUnzO
	 */
	static getPetActText(action) {
		switch (action) {
			case PetMessageConst.PM_FEEDING:
				return 'feeding';
			case PetMessageConst.PM_HUNTING:
				return 'hunting';
			case PetMessageConst.PM_DANGER:
				return 'danger';
			case PetMessageConst.PM_DEAD:
				return 'dead';
			case PetMessageConst.PM_NORMAL:
				return 'stand';
			case PetMessageConst.PM_CONNENCT:
				return 'connect';
			case PetMessageConst.PM_LEVELUP:
				return 'levelup';
			case PetMessageConst.PM_PERFORMANCE1:
				return 'perfor_1';
			case PetMessageConst.PM_PERFORMANCE2:
				return 'perfor_2';
			case PetMessageConst.PM_PERFORMANCE3:
				return 'perfor_3';
			case PetMessageConst.PM_PERFORMANCE_S:
				return 'perfor_s';
		}

		return 'stand';
	}

	/**
	 * Get Pet Hungry state text
	 *
	 * @param {integer} hungry state
	 * @return {String} hungry state text
	 *
	 * @author MrUnzO
	 */
	static getPetHungryText(state) {
		switch (state) {
			case PetHungryState.PET_HUNGER:
				return 'hungry';
			case PetHungryState.PET_HUNGRY:
				return 'bit_hungry';
			case PetHungryState.PET_SATISFIED:
				return 'noting';
			case PetHungryState.PET_ENOUGH:
				return 'full';
			case PetHungryState.PET_FULL:
				return 'so_full';
		}
		return 'hungry';
	}

	/**
	 * Get Pet Emotion ID
	 *
	 * @param {integer} hungry state
	 * @param {integer} friendly state
	 * @param {integer} action
	 * @return {integer} emotion id
	 *
	 * @author MrUnzO
	 */
	static getPetEmotion(hunger, friendly, act) {
		if (PetEmotionTable[hunger][friendly][act]) {
			return PetEmotionTable[hunger][friendly][act];
		}

		return false;
	}

	/**
	 * Get Pet talk number message (for send to server to distribute)
	 *
	 * @param {integer} job (mob id)
	 * @param {integer} action
	 * @param {integer} hungry state
	 * @return {integer} message data
	 *
	 * @author MrUnzO
	 */
	static getPetTalkNumber(job, act, hungry) {
		return parseInt(job.toString() + ('0' + hungry).slice(-2) + act.toString());
	}

	/**
	 * Indoor checking
	 *
	 * @param {string} map name
	 * @return {boolean} is indoor?
	 *
	 * @author MrUnzO
	 */
	static isIndoor(mapname) {
		if (mapname === undefined) {
			return -1;
		}
		let map;
		if (mapname.substring(mapname.length - 4, mapname.length) == '.gat') {
			map = mapname.replace('.gat', '.rsw');
		} else {
			map = mapname;
		}
		if (MapTable[map] === undefined) {
			return false;
		}
		return MapTable[map].indoor || false;
	}

	/**
	 * Get Quest Info by ID
	 *
	 * @param {integer} questID (quest id)
	 *
	 * @author alisonrag
	 */
	static getQuestInfo(questID) {
		return (
			QuestInfo[questID] || {
				Title: 'Unknown Quest',
				Description: [],
				Summary: 'Uknown Quest',
				IconName: '',
				NpcSpr: null,
				NpcNavi: null,
				NpcPosX: null,
				NpcPosY: null,
				RewardItemList: [],
				RewardEXP: 0,
				RewardJEXP: 0
			}
		);
	}

	static getCheckAttendanceInfo() {
		return CheckAttendanceTable;
	}

	static getBuyingStoreItemList() {
		return buyingStoreItemList;
	}

	static isBuyable(id) {
		return buyingStoreItemList.includes(id);
	}

	/**
	 * Is item id a pet egg?
	 *
	 * Used for older versions,
	 * because item type PETEGG didn't exist back then
	 * and type ARMOR was used with equip location 0,
	 * but this is not usable in vending since location
	 * is not received with packet...
	 *
	 * @param {integer} id
	 * @returns {boolean}
	 */
	static isPetEgg(id) {
		return id >= 9000 && id <= 9150;
	}

	/**
	 * Get Job Class Category
	 *
	 * @param {integer} JobId
	 *
	 */
	static getJobClass(job) {
		switch (job) {
			case JobId.NOVICE:
			case JobId.DO_SUMMONER1:
				return 'Base_Class';

			case JobId.SWORDMAN:
			case JobId.MAGICIAN:
			case JobId.ARCHER:
			case JobId.ACOLYTE:
			case JobId.MERCHANT:
			case JobId.THIEF:
				return 'First_Class';

			case JobId.KNIGHT:
			case JobId.PRIEST:
			case JobId.WIZARD:
			case JobId.BLACKSMITH:
			case JobId.HUNTER:
			case JobId.ASSASSIN:
			case JobId.KNIGHT2:
			case JobId.CRUSADER:
			case JobId.MONK:
			case JobId.SAGE:
			case JobId.ROGUE:
			case JobId.ALCHEMIST:
			case JobId.BARD:
			case JobId.DANCER:
			case JobId.CRUSADER2:
			case JobId.SUPERNOVICE:
				return 'Second_Class';

			case JobId.GUNSLINGER:
			case JobId.NINJA:
			case JobId.TAEKWON:
				return 'Expanded_First_Class';

			case JobId.NOVICE_H:
				return 'Rebirth_Class';

			case JobId.SWORDMAN_H:
			case JobId.MAGICIAN_H:
			case JobId.ARCHER_H:
			case JobId.ACOLYTE_H:
			case JobId.MERCHANT_H:
			case JobId.THIEF_H:
				return 'Rebirth_First_Class';

			case JobId.KNIGHT_H:
			case JobId.PRIEST_H:
			case JobId.WIZARD_H:
			case JobId.BLACKSMITH_H:
			case JobId.HUNTER_H:
			case JobId.ASSASSIN_H:
			case JobId.KNIGHT2_H:
			case JobId.CRUSADER_H:
			case JobId.MONK_H:
			case JobId.SAGE_H:
			case JobId.ROGUE_H:
			case JobId.ALCHEMIST_H:
			case JobId.BARD_H:
			case JobId.DANCER_H:
			case JobId.CRUSADER2_H:
				return 'Rebirth_Second_Class';

			case JobId.STAR:
			case JobId.STAR2:
			case JobId.LINKER:
			case JobId.KAGEROU:
			case JobId.OBORO:
			case JobId.REBELLION:
				return 'Expanded_Second_Class';

			case JobId.RUNE_KNIGHT:
			case JobId.WARLOCK:
			case JobId.RANGER:
			case JobId.ARCHBISHOP:
			case JobId.MECHANIC:
			case JobId.ROYAL_GUARD:
			case JobId.SORCERER:
			case JobId.MINSTREL:
			case JobId.WANDERER:
			case JobId.SURA:
			case JobId.GENETIC:
			case JobId.SHADOW_CHASER:
			case JobId.RUNE_KNIGHT2:
			case JobId.ROYAL_GUARD2:
			case JobId.RANGER2:
			case JobId.MECHANIC2:
				return 'Normal_Third_Class';

			case JobId.RUNE_KNIGHT_H:
			case JobId.WARLOCK_H:
			case JobId.RANGER_H:
			case JobId.ARCHBISHOP_H:
			case JobId.MECHANIC_H:
			case JobId.GUILLOTINE_CROSS_H:
			case JobId.ROYAL_GUARD_H:
			case JobId.SORCERER_H:
			case JobId.MINSTREL_H:
			case JobId.WANDERER_H:
			case JobId.SURA_H:
			case JobId.GENETIC_H:
			case JobId.SHADOW_CHASER_H:
				return 'Rebirth_Third_Class';

			case JobId.EMPEROR:
			case JobId.REAPER:
			case JobId.EMPEROR2:
				return 'Expanded_Third_Class';

			case JobId.DRAGON_KNIGHT:
			case JobId.MEISTER:
			case JobId.SHADOW_CROSS:
			case JobId.ARCH_MAGE:
			case JobId.CARDINAL:
			case JobId.WINDHAWK:
			case JobId.IMPERIAL_GUARD:
			case JobId.BIOLO:
			case JobId.ABYSS_CHASER:
			case JobId.ELEMENTAL_MASTER:
			case JobId.INQUISITOR:
			case JobId.TROUBADOUR:
			case JobId.TROUVERE:
			case JobId.WINDHAWK2:
			case JobId.MEISTER2:
			case JobId.DRAGON_KNIGHT2:
			case JobId.IMPERIAL_GUARD2:
			case JobId.SKY_EMPEROR:
			case JobId.SOUL_ASCETIC:
			case JobId.SHINKIRO:
			case JobId.SHIRANUI:
			case JobId.NIGHT_WATCH:
			case JobId.HYPER_NOVICE:
			case JobId.SPIRIT_HANDLER:
			case JobId.SKY_EMPEROR2:
				return 'Fourth_Class';

			default:
				return 'Base_Class';
		}
	}

	/**
	 * Load Clan Emblem file
	 * Icons for group reads from texture/유저인터페이스/clan_system/...
	 *
	 * @param {integer} clanId
	 * @param {function} callback to run once the file is loaded
	 *
	 * @author alisonrag
	 */
	static loadClanEmblem(clanId, callback) {
		Client.loadFile(
			DB.INTERFACE_PATH + 'clan_system/clan_emblem' + clanId.toString().padStart(2, '0') + '.bmp',
			function (dataURI) {
				const img = new Image();
				img.decoding = 'async';
				img.src = dataURI; // String Base64

				// wait image load to call the callback
				img.onload = function () {
					callback(img);
				};
			}
		);
	}

	/**
	 * Load Group Emblem file
	 * Icons for group reads from texture/유저인터페이스/group/...
	 *
	 * @param {integer} groupId
	 * @param {function} callback to run once the file is loaded
	 *
	 * @author alisonrag
	 */
	static loadGroupEmblem(groupId, callback) {
		const extension = [22, 23, 24, 25].includes(groupId) ? 'gif' : 'bmp'; // for some reason 22 ~ 25 group emblem has .gif extension

		Client.loadFile(DB.INTERFACE_PATH + 'group/group_' + groupId + '.' + extension, function (dataURI) {
			const img = new Image();
			img.decoding = 'async';
			img.src = dataURI; // String Base64

			// wait image load to call the callback
			img.onload = function () {
				callback(img);
			};
		});
	}

	/**
	 * Load Mob Type file
	 * Icons for Miniboss and MVP from texture/À¯ÀúÀÎÅÍÆäÀÌ½º/montype_...bmp
	 *
	 * @param {integer} mobType
	 * @param {function} callback to run once the file is loaded
	 *
	 */
	static loadMobEmblem(mobType, callback) {
		let monType = '';

		switch (mobType) {
			case 1:
				monType = 'montype_boss.bmp';
				break;
			case 2:
				monType = 'montype_mvp.bmp';
				break;
			default:
				console.error('Unknown mob type:', mobType);
				return;
		}

		Client.loadFile(DB.INTERFACE_PATH + monType, function (dataURI) {
			const img = new Image();
			img.decoding = 'async';
			img.src = dataURI; // String Base64

			// wait image load to call the callback
			img.onload = function () {
				callback(img);
			};
		});
	}
	/**
	 * Search for NPCs or MOBs in the navigation tables
	 *
	 * @param {string} query - The search query
	 * @param {string} type - The type of search (ALL, NPC, MOB)
	 * @returns {Array} Array of search results
	 */
	static searchNavigation(query, type) {
		if (!query || query.length < 2) {
			return [];
		}

		query = query.toLowerCase();
		const results = [];

		// Search NPCs if type is ALL or NPC
		if (type === 'ALL' || type === 'NPC') {
			// NaviNpcTable structure: [["map_name", npc_id, npc_type, class_id, "npc_name", "", x, y], ...]
			for (let i = 0; i < NaviNpcTable.length; i++) {
				const npc = NaviNpcTable[i];
				const mapName = npc[0];
				const npcId = npc[1];
				const npcName = npc[4] || '';

				// Skip if no name
				if (!npcName) {
					continue;
				}

				// Check if the NPC name contains the query
				if (npcName.toLowerCase().indexOf(query) !== -1) {
					results.push({
						type: 'NPC',
						id: npcId,
						name: npcName,
						mapName: mapName,
						x: npc[6],
						y: npc[7]
					});
				}
			}
		}

		// Search MOBs if type is ALL or MOB
		if (type === 'ALL' || type === 'MOB') {
			// NaviMobTable structure: [["map_name", spawn_id, mob_type, mob_class, "mob_name", "sprite_name", level, mob_info], ...]
			for (let i = 0; i < NaviMobTable.length; i++) {
				const mob = NaviMobTable[i];
				const mapName = mob[0];
				const mobId = mob[3]; // Using mob_class as the ID
				const mobName = mob[4] || '';

				// Skip if no name
				if (!mobName) {
					continue;
				}

				// Check if the MOB name contains the query
				if (mobName.toLowerCase().indexOf(query) !== -1) {
					// Note: mob_info might contain position data, but structure is unclear
					// For now, we're not including x/y coordinates for mobs
					results.push({
						type: 'MOB',
						id: mobId,
						name: mobName,
						mapName: mapName,
						x: null,
						y: null
					});
				}
			}
		}

		// Sort results by name
		results.sort(function (a, b) {
			return a.name.localeCompare(b.name);
		});

		// Limit to 50 results to avoid performance issues
		return results.slice(0, 50);
	}

	/**
	 * Get the NaviLinkTable
	 *
	 * @returns {Array} The NaviLinkTable
	 */
	static getNaviLinkTable() {
		return NaviLinkTable;
	}

	/**
	 * Get the NaviLinkDistanceTable
	 *
	 * @returns {Array} The NaviLinkDistanceTable
	 */
	static getNaviLinkDistanceTable() {
		return NaviLinkDistanceTable;
	}

	/**
	 * Get the NaviNpcDistanceTable
	 *
	 * @returns {Array} The NaviNpcDistanceTable
	 */
	static getNaviNpcDistanceTable() {
		return NaviNpcDistanceTable;
	}

	static createItemLink(item) {
		if (!item) {
			return null;
		}

		// Handle legacy formats (ITEMLINK and ITEM)
		if (PACKETVER.value < 20151104) {
			return `<ITEMLINK>${item.name}<INFO>${item.ITID}</INFO></ITEMLINK>`;
		}

		if (PACKETVER.value < 20160113) {
			return `<ITEM>${item.name}<INFO>${item.ITID}</INFO></ITEM>`;
		}

		// Handle ITEML format (newest, most complex)
		let data = '';

		// Encode equipment location (5 chars)
		data += Base62.encode(item.location || 0).padStart(5, '0');

		// Encode is equipment flag (1 char)
		const isEquip = item.type === 5 ? '1' : '0';
		data += isEquip;

		// Encode item ID (variable length)
		data += Base62.encode(item.ITID || 512);

		// Encode refine level (optional, starts with %)
		if (item.RefiningLevel > 0) {
			data += '%';
			data += Base62.encode(item.RefiningLevel).padStart(2, '0');
		}

		// Encode item sprite number (optional, starts with &)
		if (PACKETVER.value >= 20161116) {
			data += '&';
			const spriteNumber = item.wItemSpriteNumber ? item.wItemSpriteNumber : 0;
			data += Base62.encode(spriteNumber).padStart(2, '0');
		}

		// Encode enchant grade (optional, starts with ')
		if (PACKETVER.value >= 20200724 && item.enchantgrade > 0) {
			data += "'";
			data += Base62.encode(item.enchantgrade).padStart(2, '0');
		}

		// Determine separators based on packet version
		let card_sep, optid_sep, optpar_sep, optval_sep;
		if (PACKETVER.value >= 20200724) {
			card_sep = ')';
			optid_sep = '+';
			optpar_sep = ',';
			optval_sep = '-';
		} else if (PACKETVER.value >= 20161116) {
			card_sep = '(';
			optid_sep = '*';
			optpar_sep = '+';
			optval_sep = ',';
		} else {
			card_sep = "'";
			optid_sep = ')';
			optpar_sep = '*';
			optval_sep = '+';
		}

		// Encode cards (4 cards)
		const cardKeys = ['card1', 'card2', 'card3', 'card4'];
		for (let i = 0; i < 4; i++) {
			const cardValue = item.slot?.[cardKeys[i]] || 0;
			if (cardValue > 0) {
				data += card_sep;
				data += Base62.encode(cardValue).padStart(2, '0');
			}
		}

		// Encode random options (up to 5)
		if (item.Options) {
			item.Options.forEach(option => {
				if (option.index > 0) {
					data += optid_sep;
					data += Base62.encode(option.index).padStart(2, '0');
					data += optpar_sep;
					data += Base62.encode(option.param).padStart(2, '0');
					data += optval_sep;
					data += Base62.encode(option.value).padStart(2, '0');
				}
			});
		}

		return `<ITEML>${data}</ITEML>`;
	}

	// <ITEMLINK> (Oldest format)
	// Used for NPC message item links in clients from 2010-01-01 to before 2015-11-04.
	// example: <ITEMLINK>Display Name<INFO>Item ID</INFO></ITEMLINK>
	// <ITEM> (Middle format)
	// Used for NPC message item links in clients from 2015-11-04 onwards, replacing <ITEMLINK>.
	// example: <ITEM>Display Name<INFO>Item ID</INFO></ITEM>
	// <ITEML> (Newest format)
	// Used for player-generated item links (like Shift+Click from inventory) in clients from 2016-01-13 onwards.
	// example: <ITEML>encoded_item_data</ITEML>
	static parseItemLink(itemLink) {
		if (!itemLink) {
			return null;
		}

		const item = {
			ITID: 512,
			name: 'Unknown Item',
			type: 1,
			location: 0,
			slot: {
				card1: 0,
				card2: 0,
				card3: 0,
				card4: 0
			},
			nRandomOptionCnt: 0,
			Options: [{ index: 0, value: 0, param: 0 }],
			RefiningLevel: 0,
			enchantgrade: 0,
			IsIdentified: 1,
			IsDamaged: 0,
			wItemSpriteNumber: 0
		};

		let content = null;

		// parse ITEMLINK and ITEM format
		content = itemLink.match(/<(ITEMLINK|ITEM)>([\s\S]*?)<INFO>([\s\S]*?)<\/INFO><\/\1>/);
		if (content) {
			const [, , name, id] = content;
			item.ITID = id;
			item.name = name;
			return item;
		}

		content = itemLink.match(/<ITEML>([\s\S]*?)<\/ITEML>/);
		if (!content) {
			return item;
		}

		const data = content[1];
		let pos = 0;

		try {
			// Parse equipment location (5 chars)
			item.location = Base62.decode(data.substr(pos, 5));
			pos += 5;

			// Parse is equipment flag (1 char)
			const isEquip = data[pos] === '1';

			// TODO: add equipment type
			item.type = isEquip ? 5 : 0; // Default to armor type if equipment
			pos += 1;

			// Parse item ID (variable length until special char)
			let itemIdStr = '';
			while (pos < data.length && !"%&')(*+,-".includes(data[pos])) {
				itemIdStr += data[pos];
				pos++;
			}
			item.ITID = Base62.decode(itemIdStr);

			// Parse refine level (optional, starts with %)
			if (pos < data.length && data[pos] === '%') {
				pos++; // Skip %
				item.RefiningLevel = Base62.decode(data.substr(pos, 2));
				pos += 2;
			}

			if (PACKETVER.value >= 20161116 && pos < data.length && data[pos] === '&') {
				pos++; // Skip &
				item.wItemSpriteNumber = Base62.decode(data.substr(pos, 2));
				pos += 2;
			}

			if (PACKETVER.value >= 20200724 && pos < data.length && data[pos] === "'") {
				pos++; // Skip '
				item.enchantgrade = Base62.decode(data.substr(pos, 2));
				pos += 2;
			}

			// Determine separators based on detected packet version
			let card_sep, optid_sep, optpar_sep, optval_sep;
			if (PACKETVER.value >= 20200724) {
				card_sep = ')';
				optid_sep = '+';
				optpar_sep = ',';
				optval_sep = '-';
			} else if (PACKETVER.value >= 20161116) {
				card_sep = '(';
				optid_sep = '*';
				optpar_sep = '+';
				optval_sep = ',';
			} else {
				card_sep = "'";
				optid_sep = ')';
				optpar_sep = '*';
				optval_sep = '+';
			}

			// Parse cards
			const cardKeys = ['card1', 'card2', 'card3', 'card4'];
			let cardIndex = 0;

			while (cardIndex < 4 && pos < data.length) {
				if (data[pos] !== card_sep) {
					break;
				} // não tem mais carta

				pos++; // skip

				// take all characters that are not card separators or random option separators
				let cardStr = '';
				while (pos < data.length) {
					const c = data[pos];
					// stop if next separator is a card separator or random option separator
					if (c === card_sep || c === optid_sep) {
						break;
					}
					// stop if next separator is a future separator (security)
					if ("%&'()*+,-".includes(c)) {
						break;
					}
					cardStr += c;
					pos++;
				}

				// if nothing was taken, it's an empty card (0)
				if (cardStr === '' || cardStr === '00') {
					item.slot[cardKeys[cardIndex]] = 0;
				} else {
					item.slot[cardKeys[cardIndex]] = Base62.decode(cardStr);
				}

				cardIndex++;
			}

			// fill the cards that didn't exist with 0
			while (cardIndex < 4) {
				item.slot[cardKeys[cardIndex]] = 0;
				cardIndex++;
			}

			// Parse random options (variable count)
			let optionIdx = 0;
			while (pos < data.length && data[pos] === optid_sep && optionIdx < 5) {
				pos++; // Skip option ID separator
				const optId = Base62.decode(data.substr(pos, 2));
				pos += 2;

				if (pos >= data.length || data[pos] !== optpar_sep) {
					break;
				}
				pos++; // Skip param separator
				const optParam = Base62.decode(data.substr(pos, 2));
				pos += 2;

				if (pos >= data.length || data[pos] !== optval_sep) {
					break;
				}
				pos++; // Skip value separator
				const optValue = Base62.decode(data.substr(pos, 2));
				pos += 2;

				item.Options.push({
					index: optId,
					value: optValue,
					param: optParam
				});
				optionIdx++;
			}

			// Fill remaining option slots with zeros
			while (item.Options.length < 5 + 1) {
				item.Options.push({ index: 0, value: 0, param: 0 });
			}

			item.nRandomOptionCnt = optionIdx;

			item.name = DB.getItemName(item);

			return item;
		} catch (error) {
			console.error('Error parsing item link:', error);
			return null;
		}
	}

	static getItemNameFromLink(itemLink) {
		if (!itemLink) {
			return null;
		}

		const item = DB.parseItemLink(itemLink);
		return item.name;
	}

	/**
	 * Format a Unix timestamp (seconds) into MM/DD HH:mm
	 *
	 * @param {number} unixTimestamp - Unix time in seconds
	 * @returns {string} Formatted date string (MM/DD HH:mm)
	 */
	static formatUnixDate(unixTimestamp) {
		const d = new Date(unixTimestamp * 1000);

		return (
			String(d.getMonth() + 1).padStart(2, '0') +
			'/' +
			String(d.getDate()).padStart(2, '0') +
			' ' +
			String(d.getHours()).padStart(2, '0') +
			':' +
			String(d.getMinutes()).padStart(2, '0')
		);
	}

	/**
	 * Convert RO color codes (^RRGGBB) to HTML spans
	 *
	 * @param {string} msg - Message with RO color codes
	 * @returns {string} Message formatted to HTML
	 */
	static formatMsgToHtml(msg) {
		let hasOpenSpan = false;

		msg = msg.replace(/\^([0-9a-fA-F]{6})/g, (_, color) => {
			const close = hasOpenSpan ? '</span>' : '';
			hasOpenSpan = true;
			return close + `<span style="color:#${color}">`;
		});

		if (hasOpenSpan) {
			msg += '</span>';
		}
		return msg;
	}

	/**
	 * Get pet data by job ID
	 *
	 * @param {number} jobID - Job ID
	 * @returns {?Object} Pet data or null if not found
	 */
	static getPetByJobID(jobID) {
		return PetDBTable[jobID] || null;
	}

	/**
	 * Get pet evolution data by job ID
	 *
	 * @param {number} jobID - Job ID
	 * @returns {?Object} Pet evolution data or null if not found
	 */
	static getPetEvolutionByJob(jobID) {
		const pet = PetDBTable[jobID];
		return pet && pet.Evolution ? pet.Evolution : null;
	}

	/**
	 * Get pet data by pet egg ID
	 *
	 * @param {number|string} eggID - Pet egg ID
	 * @returns {?Object} Pet data or null if not found
	 */
	static getPetByEggID(eggID) {
		for (const jobID in PetDBTable) {
			const pet = PetDBTable[jobID];
			if (pet.PetEggID === Number(eggID)) {
				return pet;
			}
		}
		return null;
	}

	/**
	 * Get the entire reputation group list
	 *
	 * @returns {Object} Reputation group list
	 */
	static getReputeGroup() {
		return ReputeGroup;
	}

	/**
	 * Get the reputation group list for a given group id
	 *
	 * @param {string} repute_group - Reputation group id
	 * @returns {?Object} Reputation group list or null if not found
	 */
	static getReputeGroupList(repute_group) {
		const group = ReputeGroup[repute_group];
		return (group && group.ReputeList) || null;
	}

	/**
	 * Get the entire reputation information list
	 *
	 * @returns {Object} Reputation information list
	 */
	static getReputeInfo() {
		return ReputeInfo;
	}

	/**
	 * Get the reputation data for a given reputation ID
	 *
	 * @param {number|string} repute_id - Reputation ID
	 * @returns {?Object} Reputation data or null if not found
	 */
	static getReputeData(repute_id) {
		return ReputeInfo[repute_id] || null;
	}

	/**
	 * Get a Hateffect Info by ID
	 * @param {number} id - Hateffect ID
	 * @returns {Object|null} Hateffect info or null if not found
	 */
	static getHatResource(id) {
		return HatEffectInfo[id] || null;
	}

	/**
	 * Get the CashShopBannerTable
	 *
	 * @returns {Array} CashShopBannerTable
	 */
	static getCashShopBannerTable() {
		return CashShopBannerTable;
	}
}

async function startLua() {
	const wasmUrl = (await import('Vendors/liblua5.1.wasm?url')).default;
	const CLua = (await import('Vendors/wasmoon-lua5.1.js')).default;
	const [l, ha, ma, dha, dma] = await Promise.all([
		CLua.Lua.create({ customWasmUri: wasmUrl }),
		CLua.Lua.create({ customWasmUri: wasmUrl }),
		CLua.Lua.create({ customWasmUri: wasmUrl }),
		CLua.Lua.create({ customWasmUri: wasmUrl }),
		CLua.Lua.create({ customWasmUri: wasmUrl })
	]);
	lua = l;
	HO_AI = ha;
	MER_AI = ma;
	default_HO_AI = dha;
	default_MER_AI = dma;
}

function loadFontFromClient(fontPath) {
	console.log('Loading file "' + fontPath + 'SCDream4.otf"...');
	Client.loadFile(
		fontPath + 'SCDream4.otf',
		function (fontData4) {
			const base64_4 = arrayBufferToBase64(fontData4);
			const fontUrl4 = 'data:font/opentype;base64,' + base64_4;
			console.log('Loading file "' + fontPath + 'SCDream6.otf"...');
			Client.loadFile(
				fontPath + 'SCDream6.otf',
				function (fontData6) {
					const base64_6 = arrayBufferToBase64(fontData6);
					const fontUrl6 = 'data:font/opentype;base64,' + base64_6;

					const style = document.createElement('style');
					style.textContent = `
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl6}') format('opentype');  
								font-weight: 100; /* Thin */  
								font-style: normal;  
							}  
							
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl6}') format('opentype');  
								font-weight: 200; /* Extra Light */  
								font-style: normal;  
							}  
							
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl6}') format('opentype');  
								font-weight: 300; /* Light */  
								font-style: normal;  
							}  											
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl6}') format('opentype');  
								font-weight: 400; /* Normal */  
								font-style: normal;  
							}  
							
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl4}') format('opentype');  
								font-weight: 700; /* Bold */  
								font-style: normal;  
							}
								
							@font-face {  
								font-family: 'SCDream';  
								src: url('${fontUrl4}') format('opentype');  
								font-weight: 900; /* Black/Bolder */  
								font-style: normal;  
							}  								
						`;
					document.head.appendChild(style);
				},
				function () {
					console.warn('[loadFontFromClient] - Failed to load client font:', fontPath);
				}
			);
		},
		function () {
			console.warn('[loadFontFromClient] - Failed to load client font:', fontPath);
		}
	);
}

// Helper function to convert ArrayBuffer to base64
function arrayBufferToBase64(buffer) {
	let binary = '';
	const bytes = new Uint8Array(buffer);
	const len = bytes.byteLength;
	for (let i = 0; i < len; i++) {
		binary += String.fromCharCode(bytes[i]);
	}
	return btoa(binary);
}

/**
 * get System folder variants
 */
function getSystemAliases(basePath) {
	basePath = basePath.replace(/\.(lub|lua)$/i, ''); // Prevents extension been passed

	const suffixes = ['', '_true', '_sak', '_Sakray']; // Priority order
	const extensions = ['.lub', '.lua'];
	const fileList = [];

	for (let s = 0; s < suffixes.length; s++) {
		for (let e = 0; e < extensions.length; e++) {
			fileList.push(basePath + suffixes[s] + extensions[e]);
		}
	}
	return fileList;
}

/**
 * Load TXT table
 *
 * @param {string} filename to load
 * @param {string} item separator character
 * @param {number} size of each group
 * @param {function} callback to call for each group
 * @param {function} onEnd to run once the file is loaded
 */
function loadTable(filename, separator, size, callback, onEnd, useCharPage = false) {
	Client.loadFile(
		filename,
		function (buffer) {
			console.log('Loading file "' + filename + '"...');

			let data = buffer instanceof ArrayBuffer ? new Uint8Array(buffer) : buffer;
			data = TextEncoding.decode(data, useCharPage ? userCharpage : null);

			// Remove commented lines
			const content = ('\n' + data).replace(/\n(\/\/[^\n]+)/g, '');
			const elements = content.split(separator);
			const count = elements.length;
			const args = new Array(size + 1);

			for (let i = 0; i < count; i++) {
				if (i % size === 0) {
					if (i) {
						callback.apply(null, args);
					}
					args[i % size] = i;
				}

				args[(i % size) + 1] = elements[i].replace(/^\s+|\s+$/g, ''); // trim
			}

			onEnd();
		},
		onEnd
	);
}

/**
 * Load CSV table with Base64-encoded columns
 * Can pick which columns to use for key/value
 * @param {string} filename - path to CSV
 * @param {object} targetTable - object to populate
 * @param {number} keyIndex - 0-based index of column to use as key
 * @param {number} valueIndex - 0-based index of column to use as value
 * @param {function} onEnd - callback when done
 */
function loadCSV(filename, targetTable, keyIndex, valueIndex, onEnd) {
	Client.loadFile(
		filename,
		function (data) {
			console.log('Loading file "' + filename + '"...');

			const bytes = data instanceof ArrayBuffer ? new Uint8Array(data) : data;
			let text = '';
			let isBase64 = true;

			// Convert buffer to a raw "binary string".
			// This prevents atob() from throwing "outside latin1 range" errors.
			for (let i = 0, count = bytes.length; i < count; i++) {
				text += String.fromCharCode(bytes[i]);
			}

			// Check if the file is Base64 encoded
			if (!text.trimEnd().endsWith('=')) {
				text = TextEncoding.decode(bytes, 'utf-8');
				isBase64 = false;
			}

			// Split lines
			const lines = text.split(/\r?\n/);
			let index = 0;
			for (let i = 0; i < lines.length; i++) {
				const line = lines[i].trim();
				if (!line || line.startsWith('//')) {
					continue;
				}

				const parts = isBase64 ? line.split(',') : line.split('\t');
				if (parts.length <= Math.max(keyIndex, valueIndex)) {
					continue;
				}

				try {
					// Decode columns from Base64
					const value = isBase64 ? base64DecodeUtf8(parts[valueIndex].trim()) : parts[valueIndex].trim();
					targetTable[index] = value;
					index++;
				} catch (e) {
					console.error('Base64 decode failed on line', i + 1, ':', line, e);
				}
			}
			if (typeof onEnd === 'function') {
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Decode a Base64 string as UTF-8
 * @param {string} str - Base64 encoded string
 * @returns {string} decoded UTF-8 string
 */
function base64DecodeUtf8(str) {
	try {
		const bytes = Uint8Array.from(atob(str), c => c.charCodeAt(0));
		return TextEncoding.decode(bytes, 'utf-8');
	} catch (e) {
		console.warn('Base64 UTF-8 decode failed:', str, e);
		return str;
	}
}

/**
 * Load ItemMoveInfoV5.txt and attach move info to ItemTable
 *
 * @param {function} onEnd
 */
function loadMoveInfoTable(onEnd) {
	Client.loadFile(
		'data/ItemMoveInfoV5.txt',
		function (file) {
			console.log('Loading file "ItemMoveInfoV5.txt"...');

			let data = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
			data = TextEncoding.decode(data, userCharpage);
			const lines = data.split(/\r?\n/);

			for (let line of lines) {
				line = line.trim();
				if (!line || line.startsWith('//')) {
					continue;
				}

				// Remove inline comments
				const commentIndex = line.indexOf('//');
				if (commentIndex !== -1) {
					line = line.slice(0, commentIndex).trim();
				}

				const cols = line.split(/\s+/);
				if (cols.length < 9) {
					console.warn('Invalid ItemMoveInfo line:', line);
					continue;
				}

				const [key, drop, exchange, storage, cart, npcSale, mail, auction, guildStorage] = cols;

				const item = ItemTable[key] || (ItemTable[key] = {});

				item.moveInfo = {
					Drop: drop === '1',
					Exchange: exchange === '1',
					Storage: storage === '1',
					Cart: cart === '1',
					NPCSale: npcSale === '1',
					Mail: mail === '1',
					Auction: auction === '1',
					GuildStorage: guildStorage === '1'
				};
			}
			onEnd();
		},
		onEnd
	);
}

/**
 * LoadXML to json object
 *
 * @param {string} filename to load
 * @param {function} onEnd to run once the file is loaded
 *
 * @author MrUnzO
 */
function loadXMLFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			console.log('Loading file "' + filename + '"...');
			let xml = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
			xml = TextEncoding.decode(xml, userCharpage);
			xml = xml.replace(/^.*<\?xml/, '<?xml');
			const parser = new DOMParser();
			const parsedXML = parser.parseFromString(xml, 'application/xml');
			const json = XmlParse.xml2json(parsedXML);
			callback.call(null, json);
			onEnd();
		},
		onEnd
	);
}

/**
 * Load BSON to JS object / table
 *
 * @param {string} filename  File to load
 * @param {object|array} targetTable
 * @param {function} onEnd    Called when finished (success or error)
 *
 */
function loadBSONFile(filename, targetTable, onEnd) {
	Client.loadFile(
		filename,
		function (arrayBuffer) {
			try {
				console.log('Loading file "' + filename + '"...');

				const bytes = new Uint8Array(arrayBuffer);
				let offset = 0;

				const docs = [];

				while (offset < bytes.length) {
					const size =
						bytes[offset] |
						(bytes[offset + 1] << 8) |
						(bytes[offset + 2] << 16) |
						(bytes[offset + 3] << 24);

					const slice = bytes.slice(offset, offset + size);
					docs.push(BSON.deserialize(slice));

					offset += size;
				}

				// ---- AS-IS TABLE HANDLING ----

				let data;
				if (docs.length === 1) {
					data = docs[0]; // single root document
				} else {
					data = docs; // true multi-doc BSON
				}

				// Clear target table
				for (const k in targetTable) {
					delete targetTable[k];
				}

				// If root has exactly ONE key, unwrap it
				if (typeof data === 'object' && !Array.isArray(data) && Object.keys(data).length === 1) {
					const rootKey = Object.keys(data)[0];
					data = data[rootKey];
				}

				// Copy AS-IS
				Object.assign(targetTable, data);
			} catch (e) {
				console.error('BSON parse error:', e);
			} finally {
				if (onEnd) {
					onEnd();
				}
			}
		},
		onEnd
	);
}

/**
 * Mapping of effect name suffixes to SkillEffect field names.
 * Default is 'effectId'.
 */
const SUFFIX_TO_FIELD = {
	hit: 'hitEffectId',
	hitsub: 'hitEffectId',
	target: 'hitEffectId',
	cast: 'effectId',
	cast_bottom: 'effectId',
	single: 'effectId',
	bottom: 'effectId',
	begin: 'effectId',
	start: 'effectId',
	end: 'effectId',
	loop: 'effectId',
	buff: 'effectId',
	aura: 'effectId',
	main: 'effectId',
	sync: 'effectId'
};

/**
 * Hardcoded mapping for specific effect names that override suffix logic.
 */
const HARDCODED_FIELD_MAPPING = {
	broken_limit_buff: 'effectId',
	abyss_flame_target: 'hitEffectId'
};

/**
 * Merge Ez2STREffect from BSON into EffectTable and SkillEffect
 */
function mergeEz2Effects(EffectTable, SkillEffect) {
	let count = 0;
	let skillCount = 0;
	const skillBaseToId = {};
	const effectNameToId = {};
	const effectMetadata = {};

	// Pre-calculate SKID lookup maps for O(1) matching
	const skidFuzzy = {};
	for (const key in SKID) {
		const upperKey = key.toUpperCase();
		const val = SKID[key];
		skidFuzzy[upperKey] = val;

		let lastUnderscore = -1;
		while ((lastUnderscore = upperKey.indexOf('_', lastUnderscore + 1)) !== -1) {
			const suffix = upperKey.substring(lastUnderscore + 1);
			if (suffix && !skidFuzzy[suffix]) {
				skidFuzzy[suffix] = val;
			}
		}
	}

	// Optimized helper to find skill by name
	function findFuzzySkillId(name) {
		return skidFuzzy[name.toUpperCase()] || null;
	}

	// Phase 1: Pre-Analysis and Anchoring
	for (const effectName in Ez2streffect) {
		const entry = Ez2streffect[effectName];
		let baseName = effectName;
		let isSuffixed = false;

		let targetField = entry.IsToGround || entry.IsFloor ? 'groundEffectId' : 'effectId';

		for (const suffix in SUFFIX_TO_FIELD) {
			if (effectName.endsWith('_' + suffix)) {
				baseName = effectName.slice(0, -(suffix.length + 1));
				isSuffixed = true;
				targetField = SUFFIX_TO_FIELD[suffix];
				break;
			}
		}

		if (HARDCODED_FIELD_MAPPING[effectName]) {
			targetField = HARDCODED_FIELD_MAPPING[effectName];
		}

		const filePath = entry.FilePath || '';
		const soundPath = entry.SoundPath || '';
		const pathParts = filePath ? filePath.split('\\') : [];
		const lastSlashSound = soundPath.lastIndexOf('\\');
		const soundFile = lastSlashSound !== -1 ? soundPath.substring(lastSlashSound + 1) : soundPath;
		const soundBase = soundFile.replace(/\.wav$/i, '');

		effectMetadata[effectName] = {
			baseName: baseName,
			isSuffixed: isSuffixed,
			pathParts: pathParts,
			soundBase: soundBase,
			field: targetField
		};

		// Pass 1: Exact Name Match for Base
		const skillId1 = findFuzzySkillId(baseName);
		if (skillId1) {
			effectNameToId[effectName] = skillId1;
			continue;
		}

		// Pass 2: Base Anchoring from Path (Non-suffixed only)
		if (!isSuffixed) {
			for (let i = 0, len = pathParts.length; i < len; i++) {
				const segment = pathParts[i];
				if (segment) {
					const skillId2 = skillBaseToId[segment] || findFuzzySkillId(segment);
					if (skillId2) {
						skillBaseToId[segment] = skillId2;
						effectNameToId[effectName] = skillId2;
						break;
					}
				}
			}
		}
	}

	// Phase 2: Inheritance and Assignments
	for (const effectName in Ez2streffect) {
		const entry = Ez2streffect[effectName];
		const meta = effectMetadata[effectName];
		let skillId = effectNameToId[effectName];

		// Pass 3: Path Fallback for unmapped
		if (!skillId) {
			for (let i = 0; i < meta.pathParts.length; i++) {
				const segment = meta.pathParts[i] || '';
				if (segment) {
					skillId = skillBaseToId[segment] || findFuzzySkillId(segment);
					if (skillId) {
						break;
					}
				}
			}
		}

		// Pass 4: Sound Fallback
		if (!skillId && meta.soundBase) {
			skillId = findFuzzySkillId(meta.soundBase);
		}

		if (skillId) {
			effectNameToId[effectName] = skillId;
		}

		// Pass 5: Suffix Inheritance
		// Run this after all anchoring passes to ensure variants inherit from anchored bases
		if (!skillId && meta.isSuffixed && effectNameToId[meta.baseName]) {
			skillId = effectNameToId[meta.baseName];
			effectNameToId[effectName] = skillId;
		}

		// Pass 5: EffectTable Assignment
		const lastSlash = (entry.FilePath || '').lastIndexOf('\\');
		const texturePath = lastSlash !== -1 ? entry.FilePath.substring(0, lastSlash + 1) : '';

		EffectTable[effectName] = [
			{
				type: 'STR',
				file: (entry.FilePath || '').replace(/\\/g, '/').replace(/\.str$/i, ''),
				texturePath: texturePath,
				renderBeforeEntities: entry.IsFloor ? false : true,
				xOffset: entry.PosX || 0,
				yOffset: entry.PosY || 0,
				wav: entry.SoundPath ? entry.SoundPath.replace(/\\/g, '/').replace(/\.wav$/i, '') : null,
				delayLate: entry.StartDelayTime || 0,
				repeat: !!entry.IsInfinite,
				attachedEntity: true
			}
		];
		count++;

		// Pass 6: SkillEffect Mapping with differentiated fields
		if (skillId) {
			const skillEntry = SkillEffect[skillId] || (SkillEffect[skillId] = {});
			const field = meta.field;

			if (skillEntry[field]) {
				if (!Array.isArray(skillEntry[field])) {
					skillEntry[field] = [skillEntry[field]];
				}
				if (skillEntry[field].indexOf(effectName) === -1) {
					skillEntry[field].push(effectName);
				}
			} else {
				skillEntry[field] = effectName;
			}
			skillCount++;
		}
	}

	console.log(`[DBManager] Loaded ${count} effects and mapped ${skillCount} skills.`);
}

/**
 * Load CheckAttendance file to object
 *
 * @param {string} filename to load
 * @param {function} onEnd to run once the file is loaded
 *
 * @author alisonrag
 */
function loadAttendanceFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				const buffer = new Uint8Array(file);
				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;
				// create CheckAttendance required functions in context
				ctx.InsertCheckAttendanceConfig = (EvendOnOff, StartDate, EndDate) => {
					CheckAttendanceTable.Config.StartDate = StartDate;
					CheckAttendanceTable.Config.EndDate = EndDate;
					return 1;
				};
				ctx.InsertCheckAttendanceReward = (day, item_id, quantity) => {
					CheckAttendanceTable.Rewards[day - 1] = { day: day, item_id: item_id, quantity: quantity };
					return 1;
				};
				// mount file
				lua.mountFile('CheckAttendance.lub', buffer);
				// execute file
				await lua.doFile('CheckAttendance.lub');
				// execute main lua function
				lua.doStringSync('main()');
			} catch (error) {
				console.error('[loadAttendanceFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('CheckAttendance.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads WorldMap data from Lua files (Language, List, and Table)
 * Replaces DB/Map/WorldMap.js
 *
 * @param {string} basePath - Path to the directory containing worldviewdata files
 * @param {function} onEnd - Function to run when done
 */
function loadWorldMapInfo(basePath, onEnd) {
	const files = ['worldviewdata_language.lub', 'worldviewdata_list.lub', 'worldviewdata_table.lub'];

	const dirPath = basePath.endsWith('/') ? basePath : basePath + '/';
	const loadedBuffers = [];

	// Recursive loader to ensure sequential loading
	function loadNext(index) {
		if (index >= files.length) {
			processWorldMapLua();
			return;
		}

		const fullPath = dirPath + files[index];
		console.log('Loading file "' + fullPath + '"...');

		Client.loadFile(
			fullPath,
			function (data) {
				loadedBuffers.push({ name: files[index], data: data });
				loadNext(index + 1);
			},
			function () {
				console.error('[loadWorldMapInfo] - Failed to load ' + fullPath);
				// If a file fails, we might not be able to generate the map, but we continue to avoid hanging
				if (onEnd) {
					onEnd();
				}
			}
		);
	}

	async function processWorldMapLua() {
		try {
			const ctx = lua.ctx;

			// Function to add a World Category (e.g., Midgard)
			ctx.AddWorldMapCategory = (id, name, tableKey) => {
				const decodedName = userStringDecoder.decode(name);
				const decodedId = userStringDecoder.decode(id);
				const decodedTableKey = userStringDecoder.decode(tableKey);

				WorldMap.push({
					id: decodedId || decodedTableKey, // fallback to table name if id missing
					ep_from: 0,
					ep_to: 99,
					name: decodedName,
					maps: [],
					_tableKey: decodedTableKey // Internal use for mapping maps to this world
				});
				return 1;
			};

			// Function to add a Map to a specific World
			ctx.AddMapToWorld = (
				dgIndex,
				worldTableKey,
				rswName,
				left,
				top,
				right,
				bottom,
				nameDisplay,
				level,
				mapType
			) => {
				const decodedTableKey = userStringDecoder.decode(worldTableKey);
				let decodedRsw = userStringDecoder.decode(rswName);
				const decodedName = userStringDecoder.decode(nameDisplay, userCharpage);
				const decodedLevel = level ? userStringDecoder.decode(level) : '';

				// Find the world this map belongs to
				const world = WorldMap.find(w => w._tableKey === decodedTableKey);

				if (mapType === 1) {
					const originalMap = world.maps.find(m => m.index === dgIndex && m.type === 0);
					if (originalMap) {
						decodedRsw = originalMap.id;
					} // Copy rsw name
				}

				if (world) {
					// clean .rsw extension for ID
					const mapId = decodedRsw.toLowerCase().replace('.rsw', '').replace('.gat', '');

					world.maps.push({
						index: dgIndex,
						id: mapId,
						ep_from: 0,
						ep_to: 99,
						name: decodedName,
						top: top,
						left: left,
						width: right - left, // Calculate width
						height: bottom - top, // Calculate height
						moblevel: decodedLevel,
						type: mapType
					});
				}
				return 1;
			};

			// Mount and execute all files
			for (let i = 0; i < loadedBuffers.length; i++) {
				const f = loadedBuffers[i];
				const buffer = f.data instanceof ArrayBuffer ? new Uint8Array(f.data) : f.data;
				lua.mountFile(f.name, buffer);
				await lua.doFile(f.name);
			}

			// Clean Hardcoded DB Safely
			WorldMap.splice(0, WorldMap.length);

			// Execute the processing script
			// Logic: Iterate over World_List -> Get Table Name -> Iterate over that Table -> Extract Data
			lua.doStringSync(`
					function main_worldmap_process()
						if World_List == nil then return end

						for _, worldData in ipairs(World_List) do
							-- worldData structure: { Name, MainTableName, DungeonTableName, BgImage }
							local worldName = worldData[1]
							local mainTableStr = worldData[2]
							local dgTableStr = worldData[3]
							local resourceStr = worldData[4]

							-- Add the world category
							AddWorldMapCategory(resourceStr, worldName, mainTableStr)

							-- Fetch the actual table using the string name (global variable)
							local mapTable = _G[mainTableStr]

							if mapTable ~= nil then
								for _, mapEntry in ipairs(mapTable) do
									-- mapEntry structure: { Index, "map.rsw", left, top, right, bottom, LocalizedName, ... }
									local index = mapEntry[1]
									local rswName = mapEntry[2]
									local left = mapEntry[3]
									local top = mapEntry[4]
									local right = mapEntry[5]
									local bottom = mapEntry[6]
									local nameDisplay = mapEntry[7] or "" -- This is resolved from WORLD_MSGID by Lua automatically
									local level = mapEntry[8] or 0

									AddMapToWorld(index, mainTableStr, rswName, left, top, right, bottom, nameDisplay, level, 0)
								end
							end

							mapTable = _G[dgTableStr]

							if mapTable ~= nil then
								for _, mapEntry in ipairs(mapTable) do
									local index = mapEntry[1]
									local left = mapEntry[2]
									local top = mapEntry[3]
									local right = mapEntry[4]
									local bottom = mapEntry[5]
									local nameDisplay = mapEntry[6] or ""
									local level = mapEntry[7] or 0

									AddMapToWorld(index, mainTableStr, "", left, top, right, bottom, nameDisplay, level, 1)
								end
							end

						end
					end
					main_worldmap_process()
				`);

			// Clean up internal keys
			WorldMap.forEach(w => delete w._tableKey);
		} catch (e) {
			console.error('[loadWorldMapInfo] Lua Error:', e);
		} finally {
			// Unmount files
			loadedBuffers.forEach(f => lua.unmountFile(f.name));
			if (onEnd) {
				onEnd();
			}
		}
	}

	// Start the chain
	loadNext(0);
}

function loadTitleTable(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				const ctx = lua.ctx;

				ctx.AddTitle = function (titleID, titleName) {
					TitleTable[titleID] = userStringDecoder.decode(titleName, userCharpage);
					return 1;
				};

				lua.mountFile(filename, buffer);
				await lua.doFile(filename);

				lua.doStringSync(`  
						function main_title()  
							if title_tbl then  
								for titleID, titleName in pairs(title_tbl) do  
									result, msg = AddTitle(titleID, titleName)  
									if not result then  
										return false, msg  
									end  
								end  
							end  
						return true, "success"  
						end
						main_title()  
					`);
			} catch (error) {
				console.error('[loadTitleTable] Error: ', error);
			} finally {
				lua.unmountFile(filename);
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Load Town Info file
 *
 * @param {string} filename - relative file path (e.g., 'System/Towninfo.lub')
 * @param {function} callback - (Unused/Legacy)
 * @param {function} onEnd - Function to run when done
 */
function loadTownInfoFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			console.log('Loading file "' + filename + '"...');
			try {
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create AddTownInfo required functions in context
				ctx.AddTownInfo = function AddTownInfo(mapName, name, X, Y, TYPE) {
					mapName = userStringDecoder.decode(mapName);
					TownInfo[mapName] = [];
					TownInfo[mapName].push({
						Name: userStringDecoder.decode(name, userCharpage),
						X: X,
						Y: Y,
						Type: TYPE
					});
				};
				// mount file
				lua.mountFile(filename, buffer);
				// execute file
				await lua.doFile(filename);
				// execute main lua function
				lua.doStringSync('main()');
			} catch (error) {
				console.error('[loadTownInfoFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

function loadQuestInfo(filename, callback, onEnd) {
	const loadPromise = new Promise((resolve, reject) => {
		Client.loadFile(filename, resolve, reject);
	});

	loadPromise
		.then(async file => {
			console.log('Loading file "' + filename + '"...');

			try {
				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context

				// add quest info
				ctx.AddQuestInfo = (
					QuestID,
					Title,
					Summary,
					IconName,
					NpcSpr,
					NpcNavi,
					NpcPosX,
					NpcPosY,
					RewardEXP,
					RewardJEXP
				) => {
					QuestInfo[QuestID] = {
						Title: userStringDecoder.decode(Title, userCharpage),
						Summary: userStringDecoder.decode(Summary, userCharpage),
						IconName: userStringDecoder.decode(IconName),
						Description: [],
						NpcSpr: NpcSpr instanceof Uint8Array ? userStringDecoder.decode(NpcSpr) : null,
						NpcNavi: NpcNavi instanceof Uint8Array ? userStringDecoder.decode(NpcNavi) : null,
						NpcPosX: NpcPosX,
						NpcPosY: NpcPosY,
						RewardItemList: [],
						RewardEXP: RewardEXP,
						RewardJEXP: RewardJEXP
					};

					return 1;
				};

				// add quest description
				ctx.AddQuestDescription = (QuestID, QuestDescription) => {
					QuestInfo[QuestID].Description.push(userStringDecoder.decode(QuestDescription, userCharpage));
					return 1;
				};

				// add quest reward item
				ctx.AddQuestRewardItem = (QuestID, ItemID, ItemNum) => {
					QuestInfo[QuestID].RewardItemList.push({ ItemID: ItemID, ItemNum: ItemNum });
					return 1;
				};

				// mount file
				lua.mountFile(filename, buffer);
				// execute file
				await lua.doFile(filename);

				// create and execute our own main function
				lua.doStringSync(`
					function main_quest()
					-- Check if QuestInfoList is a table and not nil
					if type(QuestInfoList) ~= "table" or QuestInfoList == nil then
						return false, "Error: Quest table is nil or not a table"
					end

					for QuestID, DESC in pairs(QuestInfoList) do
						-- Ensure DESC is a table, use empty table if nil
						DESC = type(DESC) == "table" and DESC or {}

						-- Provide default values for DESC fields if they are nil
						local questData = {
							Title = DESC.Title or "Unknown Quest",
							Summary = DESC.Summary or "Unknown Quest",
							IconName = DESC.IconName or "",
							NpcSpr = DESC.NpcSpr or "",
							NpcNavi = DESC.NpcNavi or "",
							NpcPosX = DESC.NpcPosX or 0,
							NpcPosY = DESC.NpcPosY or 0,
							RewardItemList = DESC.RewardItemList or {},
							RewardEXP = tonumber(DESC.RewardEXP) or 0,
							RewardJEXP = tonumber(DESC.RewardJEXP) or 0,

							Description = type(DESC.Description) == "table" and DESC.Description or {}
						}

						result, msg = AddQuestInfo(QuestID, questData.Title, questData.Summary, questData.IconName, 
												questData.NpcSpr, questData.NpcNavi, questData.NpcPosX, 
												questData.NpcPosY, questData.RewardEXP, 
												questData.RewardJEXP)
						if not result then
							return false, msg
						end

						-- Iterate over RewardItemList table, use empty table if nil
						for k, v in pairs(questData.RewardItemList) do
							result, msg = AddQuestRewardItem(QuestID, v.ItemID, v.ItemNum)
							if not result then
								return false, msg
							end
						end

						-- Iterate over Description table, use empty table if nil
						for k, v in pairs(questData.Description) do
							result, msg = AddQuestDescription(QuestID, v or "No description available")
							if not result then
								return false, msg
							end
						end
					end
					return true, "good"
				end

				main_quest()
					`);
			} catch (error) {
				console.error('[loadQuestInfo] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd(true);
			}
		})
		.catch(error => {
			if (typeof onEnd === 'function') {
				onEnd(false);
			}
		});
}

/**
 * Attempt to load files sequentially (Recursive - Fallback).
 * Stop immediately on the first success.
 *
 * @param {Function} loaderFunc
 * @param {Array<String>} file name aliases array.
 * @param {Function} dataCallback - Callback to process the loaded data (the original `callback`, e.g., the one that receives the `json` from mapInfo).
 * @param {Function} onEnd - Called when the entire process finishes (success or total failure).
 * @param {Bool} try load all aliases
 */
function tryLoadLuaAliases(rFunc, files, callBack, onEnd, loadAll = false) {
	const totalFiles = files.length;
	if (totalFiles === 0) {
		if (typeof onEnd === 'function') {
			onEnd();
		}
		return;
	}

	let finishedCount = 0;
	let failedCount = 0;
	const trackedOnEnd = isSuccess => {
		finishedCount++;
		if (!isSuccess) {
			failedCount++;
		}

		if (finishedCount === totalFiles || (isSuccess && !loadAll)) {
			if (failedCount === totalFiles) {
				console.error(
					`[tryLoadLuaAliases] ERROR: All ${totalFiles} tryes to find ${files[0]} failed. Verify your ${files[0]} filename.`
				);
			}
			if (typeof onEnd === 'function') {
				onEnd();
			}
		}
	};

	function tryNext(index) {
		if (index >= totalFiles) {
			return;
		}

		if (
			files[index].indexOf('System/') !== 0 &&
			files[index].indexOf('SystemEN/') !== 0 &&
			files[index].indexOf('System\\') !== 0 &&
			files[index].indexOf('SystemEN\\') !== 0
		) {
			files[index] = 'System/' + files[index];
		}

		rFunc(files[index], callBack, isSuccess => {
			trackedOnEnd(isSuccess); //await last lua parsing
			if ((isSuccess && loadAll) || !isSuccess) {
				tryNext(index + 1);
			}
		});
	}
	tryNext(0);
}

/* Load ItemInfo file to object
 *
 * @param {string} filename to load
 * @param {function} onEnd to run once the file is loaded
 *
 * @author alisonrag
 */
function loadItemInfo(filename, callback, onEnd) {
	const loadPromise = new Promise((resolve, reject) => {
		Client.loadFile(filename, resolve, reject);
	});

	loadPromise
		.then(async file => {
			let wasSuccessful = false;
			try {
				console.log('Loading file "' + filename + '"...');
				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create itemInfo required functions in context
				ctx.AddItem = (
					ItemID,
					unidentifiedDisplayName,
					unidentifiedResourceName,
					identifiedDisplayName,
					identifiedResourceName,
					slotCount,
					ClassNum
				) => {
					ItemTable[ItemID] = {
						...(typeof ItemTable[ItemID] === 'object' && ItemTable[ItemID]),
						unidentifiedDisplayName: userStringDecoder.decode(unidentifiedDisplayName, userCharpage),
						unidentifiedResourceName: userStringDecoder.decode(unidentifiedResourceName),
						identifiedDisplayName: userStringDecoder.decode(identifiedDisplayName, userCharpage),
						identifiedResourceName: userStringDecoder.decode(identifiedResourceName),
						unidentifiedDescriptionName: [],
						identifiedDescriptionName: [],
						EffectID: null,
						costume: null,
						PackageID: null,
						slotCount: slotCount,
						ClassNum: ClassNum
					};
					return 1;
				};
				ctx.AddItemUnidentifiedDesc = (ItemID, v) => {
					ItemTable[ItemID].unidentifiedDescriptionName.push(userStringDecoder.decode(v, userCharpage));
					return 1;
				};
				ctx.AddItemIdentifiedDesc = (ItemID, v) => {
					ItemTable[ItemID].identifiedDescriptionName.push(userStringDecoder.decode(v, userCharpage));
					return 1;
				};
				ctx.AddItemEffectInfo = (ItemID, EffectID) => {
					ItemTable[ItemID].EffectID = EffectID;
					return 1;
				};
				ctx.AddItemIsCostume = (ItemID, costume) => {
					ItemTable[ItemID].costume = costume;
					return 1;
				};
				ctx.AddItemPackageID = (ItemID, PackageID) => {
					ItemTable[ItemID].PackageID = PackageID;
					return 1;
				};
				// mount file
				lua.mountFile(filename, buffer);
				// execute file
				await lua.doFile(filename);
				// create and execute our own main function
				// this is necessary because some servers has main function on itemInfo.lub and others load it from itemInfo_f.lub
				// doing this way we avoid to have to load the other file
				// on my tests dont care if the main() is on itemInfo.lub or itemInfo_f.lub the content is always the same
				lua.doStringSync(`
						function main_item()
							_processedItems = _processedItems or {} 
							for ItemID, DESC in pairs(tbl) do
								if not _processedItems[ItemID] and #DESC.identifiedDescriptionName > 0 then
									_processedItems[ItemID] = true 
									result, msg = AddItem(ItemID, DESC.unidentifiedDisplayName, DESC.unidentifiedResourceName, DESC.identifiedDisplayName, DESC.identifiedResourceName, DESC.slotCount, DESC.ClassNum)
									if not result then
										return false, msg
									end
									for k, v in pairs(DESC.unidentifiedDescriptionName) do
										result, msg = AddItemUnidentifiedDesc(ItemID, v)
										if not result then
											return false, msg
										end
									end
									for k, v in pairs(DESC.identifiedDescriptionName) do
										result, msg = AddItemIdentifiedDesc(ItemID, v)
										if not result then
											return false, msg
										end
									end
									if nil ~= DESC.EffectID then
										result, msg = AddItemEffectInfo(ItemID, DESC.EffectID)
										if not result then
											return false, msg
										end
									end
									if nil ~= DESC.costume then
										result, msg = AddItemIsCostume(ItemID, DESC.costume)
										if not result then
											return false, msg
										end
									end
									if nil ~= DESC.PackageID then
										result, msg = AddItemPackageID(ItemID, DESC.PackageID)
										if not result then
											return false, msg
										end
									end
								end
							end
							return true, "good"
							end
						main_item()
						`);
				wasSuccessful = true;
			} catch (error) {
				console.error('[loadItemInfo] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd(wasSuccessful);
			}
		})
		.catch(error => {
			if (typeof onEnd === 'function') {
				onEnd(false);
			}
		});
}

/**
 * load lapineddukddakbox.lub to array
 *
 * @param {string} filename to load
 * @param {function} callback to run once the file is loaded
 * @param {function} onEnd to run after the callback
 *
 */
function loadLaphineSysFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddLaphineSysItem = (key, ItemID, NeedCount, NeedRefineMin, NeedRefineMax, NeedSource_String) => {
					const decoded_key = key && key.length > 1 ? userStringDecoder.decode(key) : null;
					const decoded_NeedSource_String =
						NeedSource_String && NeedSource_String.length > 1
							? userStringDecoder.decode(NeedSource_String, userCharpage)
							: '';
					LaphineSysTable[decoded_key] = {
						ItemID: ItemID,
						NeedCount: NeedCount,
						NeedRefineMin: NeedRefineMin,
						NeedRefineMax: NeedRefineMax,
						SourceItems: new Array(),
						NeedSource_String: decoded_NeedSource_String
					};
					return 1;
				};

				ctx.AddLaphineSysSourceItem = (key, name, count, ItemID) => {
					const decoded_key = key && key.length > 1 ? userStringDecoder.decode(key) : null;
					const decoded_name = name && name.length > 1 ? userStringDecoder.decode(name) : '';
					LaphineSysTable[decoded_key].SourceItems.push({
						name: decoded_name,
						count: count,
						id: ItemID
					});
					return 1;
				};

				// mount file
				lua.mountFile('lapineddukddakbox.lub', buffer);

				// execute file
				await lua.doFile('lapineddukddakbox.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_laphynesys()
                            for key, value in pairs(tblLapineDdukddakBox.sources) do
                                result, msg = AddLaphineSysItem(key, value.ItemID, value.NeedCount, value.NeedRefineMin, value.NeedRefineMax, value.NeedSource_String)
                                if not result then
                                    return false, msg
                                end
                                for _, item in ipairs(value.SourceItems) do
                                    result, msg = AddLaphineSysSourceItem(key, item[1], item[2], item[3])
                                    if not result then
                                        return false, msg
                                    end

                                end
                            end
                        end
                        main_laphynesys()
					`);
			} catch (error) {
				console.error('[loadLaphineSysFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('lapineddukddakbox.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 *  load LapineUpgradeBox.lub to json object
 *
 * @param {string} filename to load
 * @param {function} callback to run once the file is loaded
 * @param {function} onEnd to run after the callback
 *
 */
function loadLaphineUpgFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddLaphineUpgradeItem = (
					key,
					ItemID,
					NeedCount,
					NeedRefineMin,
					NeedRefineMax,
					NeedSource_String,
					NeedOptionNumMin,
					NotSocketEnchantItem
				) => {
					const decoded_key = key && key.length > 1 ? userStringDecoder.decode(key) : null;
					const decoded_NeedSource_String =
						NeedSource_String && NeedSource_String.length > 1
							? userStringDecoder.decode(NeedSource_String, userCharpage)
							: '';

					LaphineUpgTable[decoded_key] = {
						ItemID: ItemID,
						NeedCount: NeedCount,
						NeedRefineMin: NeedRefineMin,
						NeedRefineMax: NeedRefineMax,
						NeedOptionNumMin: NeedOptionNumMin,
						NotSocketEnchantItem: NotSocketEnchantItem,
						TargetItems: new Array(),
						NeedSource_String: decoded_NeedSource_String
					};
					return 1;
				};

				ctx.AddLaphineUpgradeTargetItem = (key, name, ItemID) => {
					const decoded_key = key && key.length > 1 ? userStringDecoder.decode(key) : null;
					const decoded_name = name && name.length > 1 ? userStringDecoder.decode(name) : '';
					LaphineUpgTable[decoded_key].TargetItems.push({
						name: decoded_name,
						id: ItemID
					});
					return 1;
				};

				// mount file
				lua.mountFile('lapineupgradebox.lub', buffer);

				// execute file
				await lua.doFile('lapineupgradebox.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_laphyneupg()
                            for key, value in pairs(tblLapineUpgradeBox.targets) do
                                result, msg = AddLaphineUpgradeItem(key, value.ItemID, value.NeedCount, value.NeedRefineMin, value.NeedRefineMax, value.NeedSource_String, value.NeedOptionNumMin, value.NotSocketEnchantItem)
                                if not result then
                                    return false, msg
                                end
                                for _, item in ipairs(value.TargetItems) do
                                    result, msg = AddLaphineUpgradeTargetItem(key, item[1], item[2])
                                    if not result then
                                        return false, msg
                                    end

                                end
                            end
                        end
                        main_laphyneupg()
					`);
			} catch (error) {
				console.error('[loadLaphineUpgFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('lapineupgradebox.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads ItemDBNameTbl.lub which is used in newer UI System (holds ItemDBName and ItemID)
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadItemDBTable(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddDBItemName = (baseItem, itemID) => {
					const decoded_baseItem =
						baseItem && baseItem.length > 1 ? userStringDecoder.decode(baseItem) : null;
					ItemDBNameTbl[decoded_baseItem] = itemID;
					return 1;
				};

				// mount file
				lua.mountFile('ItemDBNameTbl.lub', buffer);

				// execute file
				await lua.doFile('ItemDBNameTbl.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_itemDBTable()
                            for key, value in pairs(ItemDBNameTbl) do
                                result, msg = AddDBItemName(key, value)
                                if not result then
                                    return false, msg
                                end
                        	end
						end
                        main_itemDBTable()
					`);
			} catch (error) {
				console.error('[loadItemDBTable] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('ItemDBNameTbl.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads ItemReformSystem.lub file into json content.
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to call with the processed data.
 * @param {function} onEnd - The function to call when the loading is complete.
 * @return {void}
 */
function loadItemReformFile(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddReformInfo = (
					key,
					BaseItem,
					ResultItem,
					NeedRefineMin,
					NeedRefineMax,
					NeedOptionNumMin,
					IsEmptySocket,
					ChangeRefineValue,
					RandomOptionCode,
					PreserveSocketItem,
					PreserveGrade
				) => {
					const decoded_BaseItem =
						BaseItem && BaseItem.length > 1 ? userStringDecoder.decode(BaseItem) : null;
					const decoded_ResultItem =
						ResultItem && ResultItem.length > 1 ? userStringDecoder.decode(ResultItem) : null;
					const decoded_RandomOptionCode =
						RandomOptionCode && RandomOptionCode.length > 1
							? userStringDecoder.decode(RandomOptionCode)
							: null;

					ItemReformTable.ReformInfo[key] = {
						BaseItem: decoded_BaseItem,
						BaseItemId: DB.getItemIdfromBase(decoded_BaseItem),
						ResultItem: decoded_ResultItem,
						ResultItemId: DB.getItemIdfromBase(decoded_ResultItem),
						NeedRefineMin: NeedRefineMin,
						NeedRefineMax: NeedRefineMax,
						NeedOptionNumMin: NeedOptionNumMin,
						IsEmptySocket: IsEmptySocket,
						ChangeRefineValue: ChangeRefineValue,
						RandomOptionCode: decoded_RandomOptionCode,
						PreserveSocketItem: PreserveSocketItem,
						PreserveGrade: PreserveGrade,
						Materials: [],
						InformationString: []
					};
					return 1;
				};

				ctx.ReformInfoAddInformationString = (key, string) => {
					const decoded_string =
						string && string.length > 1 ? userStringDecoder.decode(string, userCharpage) : null;
					ItemReformTable.ReformInfo[key].InformationString.push(decoded_string);
					return 1;
				};

				ctx.ReformInfoAddMaterial = (key, Material, Amount) => {
					const decoded_Material =
						Material && Material.length > 1 ? userStringDecoder.decode(Material) : null;
					ItemReformTable.ReformInfo[key].Materials.push({
						Material: decoded_Material,
						Amount: Amount,
						MaterialItemID: DB.getItemIdfromBase(decoded_Material)
					});
					return 1;
				};

				ctx.AddReformItem = (baseItem, itemID) => {
					const decoded_baseItem =
						baseItem && baseItem.length > 1 ? userStringDecoder.decode(baseItem) : null;
					if (!ItemReformTable.ReformItemList[decoded_baseItem]) {
						ItemReformTable.ReformItemList[decoded_baseItem] = [];
					}
					ItemReformTable.ReformItemList[decoded_baseItem].push(itemID);
					return 1;
				};

				// mount file
				lua.mountFile('ItemReformSystem.lub', buffer);

				// execute file
				await lua.doFile('ItemReformSystem.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_itemReform()
							for key, value in pairs(ReformInfo) do
								result, msg = AddReformInfo(key, value.BaseItem, value.ResultItem, value.NeedRefineMin, value.NeedRefineMax, value.NeedOptionNumMin, value.IsEmptySocket, value.ChangeRefineValue, value.RandomOptionCode, value.PreserveSocketItem, value.PreserveGrade)
								if not result then
									return false, msg
								end
								if type(value.InformationString) == "table" then
									for _, info in pairs(value.InformationString) do
										result, msg = ReformInfoAddInformationString(key, info)
										if not result then
											return false, msg
										end
									end
								end
								if type(value.Material) == "table" then
									for material, quantity in pairs(value.Material) do
										result, msg = ReformInfoAddMaterial(key, material, quantity)
										if not result then
											return false, msg
										end
									end
								end
							end
							for key, itemList in pairs(ReformItemList) do
								for index, value in ipairs(itemList) do
									result, msg = AddReformItem(key, value)
									if not result then
										return false, msg
									end
								end
							end
						end
                        main_itemReform()
					`);
			} catch (error) {
				console.error('[loadItemReformFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('ItemReformSystem.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads Enchant/EnchantList(_f).lub and parses it into EnchantListTable.
 *
 * @param {string} basePath - Base path without suffix (e.g., '.../Enchant/EnchantList')
 * @param {function} callback - Optional callback after load
 * @param {function} onEnd - The function to call when the loading is complete.
 * @return {void}
 */
function loadEnchantListFile(basePath, onEnd) {
	const normalizedBase = basePath.replace(/\.(lub|lua)$/i, '');
	const defFile = normalizedBase + '_f.lub';
	const listFile = normalizedBase + '.lub';

	Client.loadFile(
		defFile,
		async function (file) {
			try {
				console.log('Loading file "' + defFile + '"...');

				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				const ctx = lua.ctx;

				EnchantListTable = {};

				const decodeLuaString = value => {
					if (value == null) {
						return null;
					}
					if (typeof value === 'string') {
						return value;
					}
					if (value instanceof Uint8Array) {
						return userStringDecoder.decode(value);
					}
					if (value instanceof ArrayBuffer) {
						return userStringDecoder.decode(new Uint8Array(value));
					}
					return String(value);
				};

				const resolveItem = baseName => ({
					base: baseName,
					id: DB.getItemIdfromBase(baseName) || 0
				});

				const ensureGroup = id => {
					const key = Number(id);
					if (!EnchantListTable[key]) {
						EnchantListTable[key] = {
							id: key,
							slotOrder: [],
							targetItems: [],
							condition: { minRefine: 0, minGrade: 0 },
							allowRandomOption: true,
							reset: { enabled: false, rate: 0, zeny: 0, materials: [] },
							caution: '',
							slots: {}
						};
					}
					return EnchantListTable[key];
				};

				const ensureSlot = (group, slotNum) => {
					const key = Number(slotNum);
					if (!group.slots[key]) {
						group.slots[key] = {
							slot: key,
							require: { zeny: 0, materials: [] },
							successRate: 0,
							gradeBonus: {},
							random: {},
							perfect: {},
							upgrade: {}
						};
					}
					return group.slots[key];
				};

				ctx.MessageBox = message => {
					console.error('[loadEnchantListFile] ' + decodeLuaString(message));
					return 1;
				};

				ctx.C_GetSlotCount = itemDb => {
					const baseName = decodeLuaString(itemDb);
					const itemId = DB.getItemIdfromBase(baseName);
					const item = itemId ? ItemTable[itemId] : null;
					return item && item.slotCount ? Number(item.slotCount) : 0;
				};

				ctx.MAX_SLOT_NUM = 4;
				ctx.MAX_MATERIAL_NUM = 10;
				ctx.MAX_REFINE_LEVEL = 20;
				ctx.MAX_GRADE_LEVEL = 4;
				ctx.IS_CLIENT = true;

				ctx.AddEnchantGroup = enchantId => {
					ensureGroup(enchantId);
					return 1;
				};
				ctx.AddEnchantSlotOrder = (enchantId, slotNum) => {
					const group = ensureGroup(enchantId);
					group.slotOrder.push(Number(slotNum));
					return 1;
				};
				ctx.AddEnchantTargetItem = (enchantId, itemDb) => {
					const group = ensureGroup(enchantId);
					const baseName = decodeLuaString(itemDb);
					group.targetItems.push(resolveItem(baseName));
					return 1;
				};
				ctx.SetEnchantCondition = (enchantId, minRefine, minGrade) => {
					const group = ensureGroup(enchantId);
					group.condition = { minRefine: minRefine, minGrade: minGrade };
					return 1;
				};
				ctx.SetEnchantRandomOption = (enchantId, allow) => {
					const group = ensureGroup(enchantId);
					group.allowRandomOption = !!allow;
					return 1;
				};
				ctx.SetEnchantReset = (enchantId, enabled, rate, zeny) => {
					const group = ensureGroup(enchantId);
					group.reset = { enabled: !!enabled, rate: rate, zeny: zeny, materials: [] };
					return 1;
				};
				ctx.SetEnchantCaution = (enchantId, message) => {
					const group = ensureGroup(enchantId);
					group.caution = userStringDecoder.decode(message, userCharpage);
					return 1;
				};
				ctx.AddEnchantResetMaterial = (enchantId, itemDb, count) => {
					const group = ensureGroup(enchantId);
					const baseName = decodeLuaString(itemDb);
					if (!group.reset) {
						group.reset = { enabled: false, rate: 0, zeny: 0, materials: [] };
					}
					group.reset.materials.push({ ...resolveItem(baseName), count: count });
					return 1;
				};
				ctx.AddEnchantSlot = (enchantId, slotNum) => {
					const group = ensureGroup(enchantId);
					ensureSlot(group, slotNum);
					return 1;
				};
				ctx.SetEnchantRequire = (enchantId, slotNum, zeny) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					slot.require = { zeny: zeny, materials: [] };
					return 1;
				};
				ctx.AddEnchantRequireMaterial = (enchantId, slotNum, itemDb, count) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					slot.require.materials.push({ ...resolveItem(baseName), count: count });
					return 1;
				};
				ctx.SetEnchantSuccessRate = (enchantId, slotNum, rate) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					slot.successRate = rate;
					return 1;
				};
				ctx.AddEnchantGradeBonus = (enchantId, slotNum, grade, bonus) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					slot.gradeBonus[grade] = bonus;
					return 1;
				};
				ctx.AddEnchantRate = (enchantId, slotNum, grade, itemDb, rate) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					if (!slot.random[grade]) {
						slot.random[grade] = [];
					}
					slot.random[grade].push({ ...resolveItem(baseName), rate: rate });
					return 1;
				};
				ctx.AddPerfectEnchant = (enchantId, slotNum, itemDb, zeny) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					slot.perfect[baseName] = { ...resolveItem(baseName), zeny: zeny, materials: [] };
					return 1;
				};
				ctx.AddPerfectEnchantMaterial = (enchantId, slotNum, itemDb, matDb, count) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					const matName = decodeLuaString(matDb);
					if (!slot.perfect[baseName]) {
						slot.perfect[baseName] = { ...resolveItem(baseName), zeny: 0, materials: [] };
					}
					slot.perfect[baseName].materials.push({ ...resolveItem(matName), count: count });
					return 1;
				};
				ctx.AddUpgradeEnchant = (enchantId, slotNum, itemDb, resultDb, zeny) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					const resultName = decodeLuaString(resultDb);
					slot.upgrade[baseName] = {
						...resolveItem(baseName),
						result: resolveItem(resultName),
						zeny: zeny,
						materials: []
					};
					return 1;
				};
				ctx.AddUpgradeEnchantMaterial = (enchantId, slotNum, itemDb, matDb, count) => {
					const group = ensureGroup(enchantId);
					const slot = ensureSlot(group, slotNum);
					const baseName = decodeLuaString(itemDb);
					const matName = decodeLuaString(matDb);
					if (!slot.upgrade[baseName]) {
						slot.upgrade[baseName] = {
							...resolveItem(baseName),
							result: resolveItem(baseName),
							zeny: 0,
							materials: []
						};
					}
					slot.upgrade[baseName].materials.push({ ...resolveItem(matName), count: count });
					return 1;
				};

				lua.mountFile('EnchantList_f.lub', buffer);
				await lua.doFile('EnchantList_f.lub');

				Client.loadFile(
					listFile,
					async function (fileList) {
						try {
							console.log('Loading file "' + listFile + '"...');

							const listBuffer = fileList instanceof ArrayBuffer ? new Uint8Array(fileList) : fileList;
							lua.mountFile('EnchantList.lub', listBuffer);
							await lua.doFile('EnchantList.lub');

							lua.doStringSync(`
									function main_enchantlist()
										for enchantNum, info in pairs(Table) do
											AddEnchantGroup(enchantNum)
											if info.SlotOrder ~= nil then
												for _, slotNum in ipairs(info.SlotOrder) do
													AddEnchantSlotOrder(enchantNum, slotNum)
												end
											end
											if info.TargetItemTbl ~= nil then
												for _, itemDb in ipairs(info.TargetItemTbl) do
													AddEnchantTargetItem(enchantNum, itemDb)
												end
											end
											if info.Condition ~= nil then
												SetEnchantCondition(enchantNum, info.Condition.MinRefine, info.Condition.MinGrade)
											end
											if info.bApproveRandomOpt ~= nil then
												SetEnchantRandomOption(enchantNum, info.bApproveRandomOpt)
											end
											if info.Reset ~= nil then
												SetEnchantReset(enchantNum, info.Reset.bReset, info.Reset.Rate, info.Reset.Zeny)
												if info.Reset.MatTbl ~= nil then
													for matItem, matCount in pairs(info.Reset.MatTbl) do
														AddEnchantResetMaterial(enchantNum, matItem, matCount)
													end
												end
											end
											if info.CautionMsg ~= nil then
												SetEnchantCaution(enchantNum, info.CautionMsg)
											end
											if info.Slot ~= nil then
												for slotNum, slotInfo in pairs(info.Slot) do
													AddEnchantSlot(enchantNum, slotNum)
													if slotInfo.RequireTbl ~= nil then
														SetEnchantRequire(enchantNum, slotNum, slotInfo.RequireTbl.Zeny)
														if slotInfo.RequireTbl.MatTbl ~= nil then
															for matItem, matCount in pairs(slotInfo.RequireTbl.MatTbl) do
																AddEnchantRequireMaterial(enchantNum, slotNum, matItem, matCount)
															end
														end
													end
													if slotInfo.SuccessRate ~= nil then
														SetEnchantSuccessRate(enchantNum, slotNum, slotInfo.SuccessRate)
													end
													if slotInfo.GradeBonusTbl ~= nil then
														for grade, bonus in pairs(slotInfo.GradeBonusTbl) do
															AddEnchantGradeBonus(enchantNum, slotNum, grade, bonus)
														end
													end
													if slotInfo.EnchantRateTbl ~= nil then
														for grade, rateTbl in pairs(slotInfo.EnchantRateTbl) do
															for itemDb, rate in pairs(rateTbl) do
																AddEnchantRate(enchantNum, slotNum, grade, itemDb, rate)
															end
														end
													end
													if slotInfo.PerfectECTbl ~= nil then
														for itemDb, perfect in pairs(slotInfo.PerfectECTbl) do
															AddPerfectEnchant(enchantNum, slotNum, itemDb, perfect.Zeny)
															if perfect.MatTbl ~= nil then
																for matItem, matCount in pairs(perfect.MatTbl) do
																	AddPerfectEnchantMaterial(enchantNum, slotNum, itemDb, matItem, matCount)
																end
															end
														end
													end
													if slotInfo.UpgradeECTbl ~= nil then
														for itemDb, upgrade in pairs(slotInfo.UpgradeECTbl) do
															AddUpgradeEnchant(enchantNum, slotNum, itemDb, upgrade.ResultItemDB, upgrade.Zeny)
															if upgrade.MatTbl ~= nil then
																for matItem, matCount in pairs(upgrade.MatTbl) do
																	AddUpgradeEnchantMaterial(enchantNum, slotNum, itemDb, matItem, matCount)
																end
															end
														end
													end
												end
											end
										end
									end
									main_enchantlist()
								`);
						} catch (error) {
							console.error('[loadEnchantListFile] Error: ', error);
						} finally {
							lua.unmountFile('EnchantList.lub');
							lua.unmountFile('EnchantList_f.lub');
							onEnd();
						}
					},
					function () {
						console.error('[loadEnchantListFile] Missing file: ' + listFile);
						lua.unmountFile('EnchantList_f.lub');
						onEnd();
					}
				);
			} catch (error) {
				console.error('[loadEnchantListFile] Error: ', error);
				lua.unmountFile('EnchantList_f.lub');
				onEnd();
			}
		},
		function () {
			console.error('[loadEnchantListFile] Missing file: ' + defFile);
			onEnd();
		}
	);
}

/**
 * Loads lub files for Hateffects & Footprints
 */
function loadHatEffectInfo(onEnd) {
	const basePath = DB.LUA_PATH + 'hateffectinfo/';
	const idFile = basePath + 'hateffectids.lub';
	const infoFile = basePath + 'hateffectinfo.lub';

	Client.loadFile(
		idFile,
		async function (idBuf) {
			const ctx = lua.ctx;

			ctx.MessageBox = function (msg) {
				console.warn('[HatEffect] ' + msg);
				return 1;
			};

			function decodeLuaString(v) {
				return userStringDecoder.decode(v);
			}

			ctx.__js_hat_effect_id = function (key, value) {
				HatEffectID[decodeLuaString(key)] = Number(value);
				return 1;
			};

			ctx.__js_hat_effect_info = function (id, info) {
				id = Number(id);

				HatEffectInfo[id] = {
					resourceFileName: info.resourceFileName ? decodeLuaString(info.resourceFileName) : null,

					hatEffectID: info.hatEffectID ? Number(info.hatEffectID) : 0,
					hatEffectPos: info.hatEffectPos || 0,
					hatEffectPosX: info.hatEffectPosX || 0,

					isAttachedHead: !!info.isAttachedHead,
					isIgnoreRiding: !!info.isIgnoreRiding,
					isRenderBeforeCharacter: !!info.isRenderBeforeCharacter,

					isAdjustPositionWhenShrinkState: !!info.isAdjustPositionWhenShrinkState,
					isAdjustSizeWhenShrinkState: !!info.isAdjustSizeWhenShrinkState,

					footprint: !!info.footprint
				};
				return 1;
			};

			ctx.__js_footprint_effect_info = function (id, info) {
				id = Number(id);

				FootPrintEffectInfo[id] = {
					type: info.Type || 0,

					pngLeft: info.PngFile_Left ? decodeLuaString(info.PngFile_Left) : null,
					pngRight: info.PngFile_Right ? decodeLuaString(info.PngFile_Right) : null,

					strBottomLeft: info.StrFile_Bottom_Left ? decodeLuaString(info.StrFile_Bottom_Left) : null,
					strBottomRight: info.StrFile_Bottom_Right ? decodeLuaString(info.StrFile_Bottom_Right) : null,

					strTopLeft: info.StrFile_Top_Left ? decodeLuaString(info.StrFile_Top_Left) : null,
					strTopRight: info.StrFile_Top_Right ? decodeLuaString(info.StrFile_Top_Right) : null,

					scaleBottom: info.Scale_Bottom ?? 0,
					scaleTop: info.Scale_Top ?? 0,
					heightTop: info.Height_Top ?? 0,

					stride: info.Stride ?? 50,
					gap: info.Gap ?? 2,
					isAdjustAngle: !!info.IsAdjustAngle
				};

				return 1;
			};

			// Load INFO file (AFTER IDs)
			async function LoadHatEffectInfo() {
				return Client.loadFile(infoFile, async function (infoBuf) {
					try {
						lua.mountFile(
							'hateffectinfo.lub',
							infoBuf instanceof ArrayBuffer ? new Uint8Array(infoBuf) : infoBuf
						);
						await lua.doFile('hateffectinfo.lub');
						await lua.doString(`
							if hatEffectTable ~= nil then
								for id, info in pairs(hatEffectTable) do
									__js_hat_effect_info(id, info)
								end
							end
						`);
						await LoadFootPrint();
					} catch (e) {
						console.error('[HatEffect] Info load error', e);
					} finally {
						lua.unmountFile('hateffectinfo.lub');
					}
				});
			}

			// Load FOOTPRINT file (AFTER Info)
			async function LoadFootPrint() {
				return Client.loadFile(basePath + 'footprinteffectinfo.lub', async function (buf) {
					try {
						lua.mountFile(
							'footprinteffectinfo.lub',
							buf instanceof ArrayBuffer ? new Uint8Array(buf) : buf
						);
						await lua.doFile('footprinteffectinfo.lub');
						await lua.doString(`
							if FootPrintEffectTable ~= nil then
								for id, info in pairs(FootPrintEffectTable) do
									__js_footprint_effect_info(id, info)
								end
							end
						`);
					} catch (e) {
						console.error('[FootPrintEffect] load error', e);
					} finally {
						lua.unmountFile('footprinteffectinfo.lub');
					}
				});
			}

			try {
				lua.mountFile('hateffectids.lub', idBuf instanceof ArrayBuffer ? new Uint8Array(idBuf) : idBuf);
				await lua.doFile('hateffectids.lub');
				await lua.doString(`
					if HatEFID ~= nil then
						for k, v in pairs(HatEFID) do
							__js_hat_effect_id(k, v)
						end
					end
				`);

				await LoadHatEffectInfo();
			} catch (e) {
				console.error('[HatEffect] ID load error', e);
				return;
			} finally {
				lua.unmountFile('hateffectids.lub');
			}

			// All files loaded
			if (typeof onEnd === 'function') {
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads the System/Sign_Data_EN.lub to json object.
 *
 * @param {string} filename - The name of the Lua file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadSignBoardData(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddSignBoardData = (key, translation) => {
					const decoded_key = key && key.length > 1 ? userStringDecoder.decode(key) : null;
					const decoded_translation =
						translation && translation.length > 1 ? userStringDecoder.decode(translation) : null;
					SignBoardTranslatedTable[decoded_key] = decoded_translation;
					return 1;
				};

				// mount file
				lua.mountFile('Sign_Data.lub', buffer);

				// execute file
				await lua.doFile('Sign_Data.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_SignBoardData()
                            for key, value in pairs(SignBoardData) do
                                result, msg = AddSignBoardData(key, value)
                                if not result then
                                    return false, msg
                                end
                        	end
						end
                        main_SignBoardData()
					`);
			} catch (error) {
				console.error('[loadSignBoardData] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('Sign_Data.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Load SignBoardList.lub to JSON object
 *
 * @param {string} filename - The name of the Lua file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadSignBoardList(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// create signboard list
				const signBoardList = [];

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddSignBoard = (mapname, x, y, height, type, icon_location, description, color) => {
					const decoded_mapname = mapname && mapname.length > 1 ? userStringDecoder.decode(mapname) : null;
					const decoded_icon_location =
						icon_location && icon_location.length > 1 ? userStringDecoder.decode(icon_location) : null;
					const decoded_description =
						description && description.length > 1
							? userStringDecoder.decode(description, userCharpage)
							: null;
					const decoded_color = color && color.length > 1 ? userStringDecoder.decode(color) : null;

					signBoardList.push({
						mapname: decoded_mapname,
						x: x,
						y: y,
						height: height,
						type: type,
						icon_location: decoded_icon_location,
						description: decoded_description,
						color: decoded_color
					});

					return 1;
				};

				// mount file
				lua.mountFile('SignBoardList.lub', buffer);

				// execute file
				await lua.doFile('SignBoardList.lub');

				// create and execute our own main function
				lua.doStringSync(`
						function main_SignBoardList()
                            for key, value in pairs(SignBoardList) do
								local description = "";
								local color = "";
								if value[7] ~= nil then
									description = value[7];
								end
								if value[8] ~= nil then
									color = value[8];
								end
                                result, msg = AddSignBoard(value[1], value[2], value[3], value[4], value[5], value[6], description, color)
                                if not result then
                                    return false, msg
                                end
                        	end
						end
                        main_SignBoardList()
					`);

				SignBoardTable = preprocessSignboardData(signBoardList);
			} catch (error) {
				console.error('[loadSignBoardList] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile('SignBoardList.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Preprocesses an array of signboard objects and organizes them into a nested dictionary.
 *
 * @param {Array} signboardArray - The array of signboard objects.
 * @return {Object} The nested dictionary containing the preprocessed signboard data.
 */
function preprocessSignboardData(signboardArray) {
	const signboardDict = {};

	for (const signboard of signboardArray) {
		const { mapname, x, y } = signboard;
		if (!signboardDict[mapname]) {
			signboardDict[mapname] = {};
		}
		if (!signboardDict[mapname][x]) {
			signboardDict[mapname][x] = {};
		}
		signboardDict[mapname][x][y] = signboard;
	}

	return signboardDict;
}

/**
 * Load weapontable.lub to WeaponTable and WeaponHitSoundTable
 *
 * @param {string} filename to load
 * @param {function} callback to run once the file is loaded
 * @param {function} onEnd to run after the callback
 */
function loadWeaponTable(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create required functions in context
				ctx.AddWeaponName = (weaponID, weaponName) => {
					const decoded_weaponName =
						weaponName && weaponName.length > 0 ? userStringDecoder.decode(weaponName) : '';
					WeaponTable[weaponID] = decoded_weaponName;
					return 1;
				};

				ctx.AddWeaponHitSound = (weaponID, soundFile) => {
					const decoded_soundFile =
						soundFile && soundFile.length > 0 ? userStringDecoder.decode(soundFile) : '';
					WeaponHitSoundTable[weaponID] = decoded_soundFile;
					return 1;
				};

				ctx.AddExpansionWeapon = (weaponID, expansionWeaponID) => {
					WeaponTypeExpansion[weaponID] = expansionWeaponID;
					return 1;
				};

				// mount file
				lua.mountFile('weapontable.lub', buffer);

				// execute file
				await lua.doFile('weapontable.lub');

				// create and execute our own main function
				lua.doStringSync(`  
						function main_weapontable()  
							-- Process WeaponNameTable  
							if type(WeaponNameTable) == "table" then  
								for weaponID, weaponName in pairs(WeaponNameTable) do  
									result, msg = AddWeaponName(weaponID, weaponName)  
									if not result then  
										return false, msg  
									end  
								end  
							end  
							
							-- Process WeaponHitWaveNameTable  (seems like gravity is still using the hardcoded sound files)
							-- uncomment this if you want to use the custom sound files
							--if type(WeaponHitWaveNameTable) == "table" then  
							--	for weaponID, soundFile in pairs(WeaponHitWaveNameTable) do  
							--		result, msg = AddWeaponHitSound(weaponID, soundFile)  
							--		if not result then  
							--			return false, msg  
							--		end  
							--	end  
							--end  

							-- Process WeaponTypeExpansionTable
							if type(Expansion_Weapon_IDs) == "table" then  
								for weaponID, expansionWeaponID in pairs(Expansion_Weapon_IDs) do  
									result, msg = AddExpansionWeapon(weaponID, expansionWeaponID)  
									if not result then  
										return false, msg  
									end  
								end  
							end  
							
							return true, "success"  
						end  
						main_weapontable()  
					`);
			} catch (error) {
				console.error('[loadWeaponTable] Error: ', error);
			} finally {
				// release file from memory
				lua.unmountFile('weapontable.lub');

				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads skillinfolist.lub which replaces part of DB/Skills/SkillInfo.js
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadSkillInfoList(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// Create automatic JT_ mappings
				const jobIdWithJT = { ...JobId };
				for (const [key, value] of Object.entries(JobId)) {
					jobIdWithJT[`JT_${key}`] = value;
				}
				ctx.JOBID = jobIdWithJT;
				await lua.doString(`
						if JOBID then
							__JOBID_ORIGINAL = JOBID
							JOBID = setmetatable({}, {
								__index = function(t, k)
									local id = __JOBID_ORIGINAL[k]
									return id ~= nil and id or 0
								end
							})
						end
					`);

				// create required functions in context
				ctx.AddSkillInfo = (
					skillId,
					resName,
					skillName,
					maxLv,
					spAmount,
					bSeperateLv,
					attackRange,
					skillScale
				) => {
					// Convert to format expected by SkillInfo.js
					const toArray = v => {
						if (Array.isArray(v)) {
							return v;
						}
						if (typeof v === 'object' && v !== null) {
							return Object.keys(v)
								.map(Number)
								.sort((a, b) => a - b)
								.map(k => v[k]);
						}
						return [];
					};
					SkillInfo[skillId] = {
						Name: userStringDecoder.decode(resName),
						SkillName: userStringDecoder.decode(skillName, userCharpage),
						MaxLv: maxLv,
						SpAmount: toArray(spAmount),
						bSeperateLv: bSeperateLv,
						AttackRange: toArray(attackRange),
						SkillScale: skillScale,
						NeedSkillList: {},
						_NeedSkillList: []
					};

					return 1;
				};

				ctx.AddSkillRequirement = (skillId, requiredSkillId, requiredLevel) => {
					SkillInfo[skillId]._NeedSkillList.push([requiredSkillId, requiredLevel]);
					return 1;
				};

				ctx.AddJobSkillRequirement = (skillId, jobId, requiredSkillId, requiredLevel) => {
					if (!SkillInfo[skillId].NeedSkillList[jobId]) {
						SkillInfo[skillId].NeedSkillList[jobId] = [];
					}
					SkillInfo[skillId].NeedSkillList[jobId].push([requiredSkillId, requiredLevel]);
					return 1;
				};

				ctx.SKID = SKID;
				await lua.doString(`
						if SKID then
							__SKID_ORIGINAL = SKID 

							SKID = setmetatable({}, {
								__index = function(t, k)
								local id = __SKID_ORIGINAL[k]
								return id ~= nil and id or 0 
								end
							})
						end
					`);

				// mount file
				lua.mountFile('skillinfolist.lub', buffer);

				// execute file
				await lua.doFile('skillinfolist.lub');

				// create and execute our own main function
				lua.doStringSync(`  
						function main_skillInfoList()  
							if not SKILL_INFO_LIST then  
								return false, "Error: SKILL_INFO_LIST is nil or not a table"  
							end  
						
							for skillId, skillData in pairs(SKILL_INFO_LIST) do 
								local resName = skillData[1] or "" 
								local skillName = skillData.SkillName or ""  
								local maxLv = skillData.MaxLv or 1  
								local spAmount = skillData.SpAmount or {}  
								local bSeperateLv = skillData.bSeperateLv or false  
								local attackRange = skillData.AttackRange or {}  
								local skillScale = skillData.SkillScale or {}  

								result, msg = AddSkillInfo(skillId, resName, skillName, maxLv, spAmount, bSeperateLv, attackRange, skillScale)  
								if not result then  
									return false, msg  
								end
								
								if skillData._NeedSkillList then  
									for _, req in ipairs(skillData._NeedSkillList) do  
										if req[1] and req[2] then  
											AddSkillRequirement(skillId, req[1], req[2])  
										end  
									end  
								end  
								
								if skillData.NeedSkillList then  
									for jobId, reqList in pairs(skillData.NeedSkillList) do  
										if reqList then  
											for _, req in ipairs(reqList) do  
												if req[1] and req[2] then  
													AddJobSkillRequirement(skillId, jobId, req[1], req[2])  
												end  
											end  
										end  
									end  
								end 

							end  
							return true, "good"  
							end
						main_skillInfoList()  
					`);
			} catch (error) {
				console.error('[loadSkillInfoList] Error: ', error);
			} finally {
				// release file from memory
				lua.unmountFile('skillinfolist.lub');
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads jobinheritlist.lub and skilltreeview.lub which replaces DB/Skills/SkillTreeView.js
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadSkillTreeView(filename, callback, onEnd) {
	// First load jobinheritlist.lub
	Client.loadFile(
		DB.LUA_PATH + 'skillinfoz/jobinheritlist.lub',
		async function (file) {
			try {
				console.log(`Loading file "${DB.LUA_PATH}skillinfoz/jobinheritlist.lub"...`);
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// Mount and execute jobinheritlist.lub
				lua.mountFile('jobinheritlist.lub', buffer);
				await lua.doFile('jobinheritlist.lub');

				// Now load skilltreeview.lub
				loadSkillTreeViewData(filename, callback, onEnd);
			} catch (error) {
				console.error('[loadSkillTreeView - jobinheritlist] Error: ', error);
				onEnd();
			}
		},
		onEnd
	);
}

function loadSkillTreeViewData(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				const ctx = lua.ctx;
				// Create automatic JT_ mappings
				const jobIdWithJT = { ...JobId };
				for (const [key, value] of Object.entries(JobId)) {
					jobIdWithJT[`JT_${key}`] = value;
				}
				ctx.JOBID = jobIdWithJT;

				// Function to add skill tree data using job hierarchy from jobinheritlist.lub
				ctx.AddSkillTreeView = function (jobId, beforeJob) {
					// Calculate list and beforeJob from inheritance chain
					let list = 1;
					// TODO: Find another way to do that
					if (jobId === JobId.NOVICE) {
						list = 1;
					} else if (
						jobId < JobId.KNIGHT ||
						jobId === JobId.TAEKWON ||
						(jobId >= JobId.SUPERNOVICE && jobId <= JobId.NINJA) ||
						jobId == JobId.DO_SUMMONER ||
						jobId == JobId.DRUID
					) {
						list = 1;
					} else if (
						jobId < JobId.NOVICE_H ||
						jobId == JobId.STAR ||
						jobId == JobId.LINKER ||
						(jobId >= JobId.KAGEROU && jobId <= JobId.REBELLION) ||
						jobId == JobId.SUPERNOVICE2 ||
						jobId == JobId.SPIRIT_HANDLER ||
						jobId == JobId.KARNOS
					) {
						list = 2;
					} else if (
						(jobId <= JobId.THIEF_H && jobId >= JobId.NOVICE_H) ||
						(jobId >= JobId.NOVICE_B && jobId <= JobId.THIEF_B) ||
						jobId == JobId.DO_SUMMONER_B ||
						jobId == JobId.NINJA_B ||
						jobId == JobId.TAEKWON_B ||
						jobId == JobId.GUNSLINGER_B
					) {
						list = 1;
					} else if (
						jobId < JobId.RUNE_KNIGHT ||
						(jobId >= JobId.KNIGHT_B && jobId <= JobId.DANCER_B) ||
						(jobId >= JobId.KAGEROU_B && jobId <= JobId.REBELLION_B)
					) {
						list = 2;
					} else if (
						jobId < JobId.DRAGON_KNIGHT ||
						jobId == JobId.STAR_EMPEROR ||
						jobId == JobId.SOUL_REAPER ||
						(jobId >= JobId.RUNE_KNIGHT_B && jobId <= JobId.SHADOW_CHASER_B) ||
						jobId === JobId.EMPEROR_B ||
						jobId === JobId.REAPER_B ||
						jobId == JobId.ALITEA
					) {
						list = 3;
					} else if (jobId <= JobId.TROUVERE || (jobId >= JobId.SKY_EMPEROR && jobId <= JobId.HYPER_NOVICE)) {
						list = 4;
					} else {
						list = 1;
						console.error(`[loadSkillTreeViewData] Failed to find inherith list job: (${jobId})`);
					}
					// Create the skill tree entry
					const entry = {
						list: list,
						beforeJob: beforeJob
					};

					SkillTreeView[jobId] = entry;
					return 1;
				};

				ctx.AddSkillToJob = function (jobId, pos, skillId) {
					if (SkillTreeView[jobId]) {
						SkillTreeView[jobId][skillId] = Number(pos);
					}
					return 1;
				};

				lua.doStringSync(`
						JobSkillTab = {}
					
						function JobSkillTab.ChangeSkillTabName(in_job, in_1sttab, in_2ndtab, in_3rdtab, in_4thtab)
							local tbl = {
								job = in_job,
								TabName1st = in_1sttab,
								TabName2nd = in_2ndtab,
								TabName3rd = in_3rdtab,
								TabName4th = in_4thtab
							}
							JobSkillTab[#JobSkillTab + 1] = tbl
							return true
						end
					`);

				ctx.SKID = SKID;
				await lua.doString(`
						if SKID then
							__SKID_ORIGINAL = SKID 

							SKID = setmetatable({}, {
								__index = function(t, k)
								local id = __SKID_ORIGINAL[k]
								return id ~= nil and id or 0 
								end
							})
						end
					`);

				lua.mountFile('skilltreeview.lub', buffer);
				await lua.doFile('skilltreeview.lub');

				lua.doStringSync(`    
						function main_skillTreeView()    
							if not SKILL_TREEVIEW_FOR_JOB then    
								return false, "Error: SKILL_TREEVIEW_FOR_JOB is nil or not a table"    
							end    
					
							for jobId, skillData in pairs(SKILL_TREEVIEW_FOR_JOB) do      
								local beforeJob = JOB_INHERIT_LIST[jobId] or nil  
								result, msg = AddSkillTreeView(jobId, beforeJob)      
								if not result then      
									return false, msg      
								end   
								
								for pos, skillId in pairs(skillData) do    
									result, msg = AddSkillToJob(jobId, pos, skillId)    
									if not result then    
										return false, msg    
									end    
								end  
							end    
							return true, "good"    
						end    
						
						main_skillTreeView()    
					`);
			} catch (error) {
				console.error('[loadSkillTreeView] Error: ', error);
			} finally {
				lua.unmountFile('skilltreeview.lub');
				lua.unmountFile('jobinheritlist.lub');
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Load State Icon Info (StatusInfo) from Lua files
 * Loads efstids.lub, stateiconimginfo.lub, and stateiconinfo.lub sequentially,
 * synchronizing EFST_IDs with StatusConst and populating the StatusInfo table.
 *
 * @param {string} basePath - Directory path (e.g., DB.LUA_PATH + 'stateicon/')
 * @param {function} callback - (Unused/Legacy)
 * @param {function} onEnd - Function to run when done
 */
function loadStateIconInfo(basePath, callback, onEnd) {
	const files = ['efstids.lub', 'stateiconimginfo.lub', 'stateiconinfo.lub'];

	const loadedBuffers = [];

	const dirPath = basePath.endsWith('/') ? basePath : basePath + '/';

	function loadNext(index) {
		if (index >= files.length) {
			processLuaData();
			return;
		}

		const fullPath = dirPath + files[index];
		console.log('Loading file "' + fullPath + '"...');

		Client.loadFile(
			fullPath,
			function (data) {
				loadedBuffers.push({ name: files[index], data: data });
				loadNext(index + 1);
			},
			function () {
				console.error('[loadStateIconInfo] - Failed to load ' + fullPath);
				if (onEnd) {
					onEnd();
				}
			}
		);
	}

	async function processLuaData() {
		try {
			const ctx = lua.ctx;

			ctx.mergeSC = (key, value) => {
				// Keys might be passed as Uint8Array from Lua by Wasmoon
				const safeKey =
					typeof key === 'string' ? key : key instanceof Uint8Array ? userStringDecoder.decode(key) : key;

				if (
					typeof safeKey === 'string' &&
					typeof value === 'number' &&
					value >= 0 &&
					safeKey.startsWith('EFST_')
				) {
					const jsKey = safeKey.replace('EFST_', '');
					SC[jsKey] = value;
				}

				// Add BLANK back if it was cleared and not defined in LUB
				if (!SC.BLANK) {
					SC.BLANK = -1;
				}
				return 1;
			};

			ctx.SetStatusInfo = (id, haveTimeLimit, posTimeLimitStr) => {
				if (!StatusInfo[id]) {
					StatusInfo[id] = {};
				}
				StatusInfo[id].haveTimeLimit = haveTimeLimit;
				StatusInfo[id].posTimeLimitStr = posTimeLimitStr;
				StatusInfo[id].descript = [];
				return 1;
			};

			ctx.AddStatusDesc = (id, desc, r, g, b) => {
				if (!StatusInfo[id]) {
					return 0;
				}
				const text = userStringDecoder.decode(desc, userCharpage);
				let color = null;
				if (r >= 0 && g >= 0 && b >= 0) {
					color = `rgb(${r}, ${g}, ${b})`;
				}
				StatusInfo[id].descript.push([text, color]);
				return 1;
			};

			ctx.SetStatusIcon = (id, iconName) => {
				const icon = userStringDecoder.decode(iconName);
				if (!StatusInfo[id]) {
					StatusInfo[id] = { descript: [] };
				}
				StatusInfo[id].icon = icon;
				return 1;
			};

			for (let i = 0; i < loadedBuffers.length; i++) {
				const f = loadedBuffers[i];
				const buffer = f.data instanceof ArrayBuffer ? new Uint8Array(f.data) : f.data;
				lua.mountFile(f.name, buffer);
				await lua.doFile(f.name);

				// This prevents the 'table index is nil' crash in stateiconimginfo.lub.
				// return SC.BLANK on nil
				if (f.name === 'efstids.lub') {
					await lua.doString(`
							if EFST_IDs then
								__EFST_IDS_ORIGINAL = EFST_IDs 

								EFST_IDs = setmetatable({}, {
									__index = function(t, k)
										local id = __EFST_IDS_ORIGINAL[k]
										return id ~= nil and id or -1
									end
								})
							end
						`);
				}
			}

			lua.doStringSync(`
					function extract_status_info()
						-- Sync StatusConst (SC) with the full list of EFST_IDs from the original table
						if type(__EFST_IDS_ORIGINAL) == "table" then
							for k, v in pairs(__EFST_IDS_ORIGINAL) do
								if type(k) == "string" and type(v) == "number" then
									mergeSC(k, v)
								end
							end
							
							local mt = getmetatable(__EFST_IDS_ORIGINAL)
							if mt and type(mt.__index) == "table" then
								for k, v in pairs(mt.__index) do
									if type(k) == "string" and type(v) == "number" then
										mergeSC(k, v)
									end
								end
							end
						end

						-- Process Basic Info & Descriptions
						if StateIconList ~= nil then
							for id, info in pairs(StateIconList) do
								SetStatusInfo(id, info.haveTimeLimit, info.posTimeLimitStr)

								if info.descript ~= nil then
									for _, descLine in ipairs(info.descript) do
										local text = descLine[1]
										local colorData = descLine[2]
										local r, g, b = -1, -1, -1

										if type(colorData) == "table" and #colorData >= 3 then
											r = colorData[1]
											g = colorData[2]
											b = colorData[3]
										end

										AddStatusDesc(id, text, r, g, b)
									end
								end
							end
						end

						-- Process Icons 
						if StateIconImgList ~= nil then
							for priorityId, list in pairs(StateIconImgList) do 
								if type(list) == "table" then
									for id, iconName in pairs(list) do
										SetStatusIcon(id, iconName)
									end
								end
							end
						end
					end

					extract_status_info()
				`);
		} catch (e) {
			console.error('[loadStateIconInfo] Lua Error:', e);
		} finally {
			loadedBuffers.forEach(f => lua.unmountFile(f.name));
			if (onEnd) {
				onEnd();
			}
		}
	}

	loadNext(0);
}

/* Load Ragnarok Lua table to object
 * A lot of ragnarok lua tables are splited in 2 files ( 1 - ID table, 2 - Table of values )
 * @param {Array} list of files to be load (must be 2 files)
 * @param {String} name of table in lua file
 * @param {function} callback to run once the file is loaded
 * @param {function} onEnd to run once the file is loaded
 * @param {function?} contextFunc - Function to execute after parsing but before unmounting
 *
 * @author alisonrag
 */
function loadLuaTable(file_list, table_name, callback, onEnd, contextFunc) {
	const id_filename = file_list[0];
	const value_table_filename = file_list[1];

	try {
		console.log('Loading file "' + id_filename + '"...');
		Client.loadFile(id_filename, async function (file) {
			try {
				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				// mount file
				lua.mountFile(id_filename, buffer);
				// execute file
				await lua.doFile(id_filename);
				loadValueTable();
			} catch (hException) {
				console.error(`(${id_filename}) error: `, hException);
			}
		});

		function loadValueTable() {
			console.log('Loading file "' + value_table_filename + '"...');
			Client.loadFile(value_table_filename, async function (file) {
				try {
					// check if file is ArrayBuffer and convert to Uint8Array if necessary
					const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
					// mount file
					lua.mountFile(value_table_filename, buffer);
					// execute file
					await lua.doFile(value_table_filename);
					parseTable();
				} catch (hException) {
					console.error(`(${value_table_filename}) error: `, hException);
				}
			});
		}

		function parseTable() {
			// declare the table object
			const table = {};

			// get context
			const ctx = lua.ctx;

			// create context function
			ctx.addKeyAndValueToTable = (key, value) => {
				table[key] = userStringDecoder.decode(value, userCharpage);
				return 1;
			};

			// used in some specific cases like skilldescript.lub
			ctx.addKeyAndMoreValuesToTable = (key, value) => {
				if (!table[key]) {
					table[key] = '';
				}
				table[key] += userStringDecoder.decode(value, userCharpage) + '\n';
				return 1;
			};

			// create and execute a wrapper lua code
			// this way we let webassembly handle the code and still can get the table data
			lua.doStringSync(`
						function main_table()
							for key, value in pairs(${table_name}) do
								if type(value) == "table" then
									for k, v in pairs(value) do
										result, msg = addKeyAndMoreValuesToTable(key, v)
									end
								else
									result, msg = addKeyAndValueToTable(key, value)
								end
								if not result then
									return false, msg
								end
							end
							return true, "good"
						end
						main_table()
					`);

			if (typeof contextFunc === 'function') {
				contextFunc();
			}

			// unmount files
			lua.unmountFile(value_table_filename);
			lua.unmountFile(id_filename);

			// return table
			callback.call(null, table);
		}
	} catch (e) {
		console.error('error: ', e);
	} finally {
		onEnd.call();
	}
}

/**
 * Extracts a variable from a Lua file and converts it to a JavaScript value
 * @param {String} file_path - Path to the Lua file
 * @param {String} variable_name - Name of the variable to extract
 * @param {function} callback - Function to run with the extracted value
 * @param {function} onEnd - Function to run once the process is complete
 *
 * @author guicaulada
 */
function loadLuaValue(file_path, variable_name, callback, onEnd) {
	try {
		console.log('Loading file "' + file_path + '"...');
		Client.loadFile(file_path, async function (file) {
			try {
				// Check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// Mount file
				lua.mountFile(file_path, buffer);

				// Execute file
				await lua.doFile(file_path);

				// Get context
				const ctx = lua.ctx;

				// Initialize result variable
				let result = null;

				// Add key-value pairs to objects at any nesting level
				ctx.extractValue = value => {
					result = JSON.parse(userStringDecoder.decode(value));
				};

				// Create and execute a wrapper Lua code to extract the variable
				lua.doStringSync(
					String.raw`
							local function escape_str(str)
								return str:gsub("\\", "\\\\"):gsub("\"", "\\\"")
							end

							local function to_json(value)
								if type(value) == "boolean" then
									return tostring(value)
								elseif type(value) == "number" then
									return value
								elseif type(value) == "string" then
									return "\"" .. escape_str(value) .. "\""
								elseif type(value) == "table" then
									local is_array = true
									local index = 1
									for k, _ in pairs(value) do
										if type(k) ~= "number" or k ~= index then
											is_array = false
											break
										end
										index = index + 1
									end

									local result = {}
									if is_array then
										for _, v in ipairs(value) do
											table.insert(result, to_json(v))
										end
										return "[" .. table.concat(result, ",") .. "]"
									else
										for k, v in pairs(value) do
											if type(k) == "string" then
												table.insert(result, "\"" .. escape_str(k) .. "\":" .. to_json(v))
											elseif type(k) == "number" then
												table.insert(result, "\"" .. tostring(k) .. "\":" .. to_json(v))
											end
										end
										return "{" .. table.concat(result, ",") .. "}"
									end
								else
									return "null"
								end
							end
						` +
						`
							extractValue(to_json(${variable_name}))
						`
				);

				// Unmount file
				lua.unmountFile(file_path);

				// Return the extracted value
				callback.call(null, result);
			} catch (hException) {
				console.error(`(${file_path}) error: `, hException);
				callback.call(null, null);
			} finally {
				if (onEnd) {
					onEnd.call();
				}
			}
		});
	} catch (e) {
		console.error('error: ', e);
		if (onEnd) {
			onEnd.call();
		}
	}
}

/**
 * Loads System/mapInfo.lub
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the parsed data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 */
function loadMapTbl(filename, callback, onEnd) {
	const loadPromise = new Promise((resolve, reject) => {
		Client.loadFile(filename, resolve, reject);
	});

	loadPromise
		.then(async file => {
			try {
				console.log('Loading file "' + filename + '"...');
				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context, a proxy. It will be used to interact with lua conveniently
				const ctx = lua.ctx;

				// create mapInfo required functions in context
				ctx.AddMapDisplayName = (name, displayName, notify_enter) => {
					const decoded_name = userStringDecoder.decode(name);
					MapInfo[decoded_name] = {
						displayName: userStringDecoder.decode(displayName, userCharpage),
						notifyEnter: notify_enter,
						signName: {
							subTitle: null,
							mainTitle: null
						},
						backgroundBmp: null
					};

					return 1;
				};

				ctx.AddMapSignName = (name, subTitle, mainTitle) => {
					const decoded_name = userStringDecoder.decode(name);
					const decoded_subTitle =
						subTitle && subTitle.length > 1 ? userStringDecoder.decode(subTitle, userCharpage) : null;
					const decoded_mainTitle =
						mainTitle && mainTitle.length > 1 ? userStringDecoder.decode(mainTitle, userCharpage) : null;
					MapInfo[decoded_name].signName = {
						subTitle: decoded_subTitle,
						mainTitle: decoded_mainTitle
					};
					return 1;
				};

				ctx.AddMapBackgroundBmp = (name, backgroundBmp) => {
					const decoded_name = userStringDecoder.decode(name);
					MapInfo[decoded_name].backgroundBmp = backgroundBmp
						? userStringDecoder.decode(backgroundBmp)
						: 'field';
					return 1;
				};

				// mount file
				lua.mountFile(filename, buffer);

				// execute file
				await lua.doFile(filename);

				// execute main function
				lua.doStringSync('main()');
			} catch (error) {
				console.error('[loadMapTbl] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd(true);
			}
		})
		.catch(error => {
			onEnd(false);
		});
}

/**
 * Loads datainfo/petInfo.lub and populate PetDBTable
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadPetInfo(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				lua.mountFile(filename, buffer);
				await lua.doFile(filename);

				// Read Lua table
				const readLuaTable = tableName => {
					const result = {};
					const ctx = lua.ctx;

					ctx.__push_kv = (k, v) => {
						const key = k instanceof Uint8Array ? userStringDecoder.decode(k) : k;
						const val = v instanceof Uint8Array ? userStringDecoder.decode(v) : v;
						result[key] = val;
						return 1;
					};

					lua.doStringSync(`
							for k, v in pairs(${tableName}) do
								__push_kv(k, v)
							end
						`);

					return result;
				};

				// Read all pet tables
				const eggMap = readLuaTable('PetEggItemID_PetJobID');
				const petNames = readLuaTable('PetNameTable');
				const petStrings = readLuaTable('PetStringTable');
				const petIllusts = readLuaTable('PetIllustNameTable_Eng');
				const petAccIDs = readLuaTable('PetAccIDs');
				const petAccNames = readLuaTable('PetAccActNameTable');

				PetDBTable = {};

				for (const eggId in eggMap) {
					const jobId = Number(eggMap[eggId]);
					const petName = petNames[jobId] || null;
					const accKey = petName ? `ACC_${petName.toUpperCase()}` : null;
					const accId = accKey ? petAccIDs[accKey] || null : null;

					PetDBTable[jobId] = {
						PetJobID: jobId,
						PetEggID: Number(eggId),
						PetName: petName,
						PetString: petStrings[jobId] || null,
						PetIllust: petIllusts[jobId] || null,
						PetAcc_ID: accId,
						PetAcc_Name: accId ? petAccNames[accId] || null : null
					};
				}

				EggIDToJobID = {};

				for (const jobID in PetDBTable) {
					const pet = PetDBTable[jobID];
					EggIDToJobID[pet.PetEggID] = Number(jobID);
				}
			} catch (error) {
				console.error('[loadPetInfo] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd();
			}
		},
		onEnd
	);
}

/**
 * Loads System/PetEvolutionCLN_true.lub and attach to PetDBTable
 *
 * @param {string} filename - The name of the file to load.
 * @param {function} callback - The function to invoke with the loaded data.
 * @param {function} onEnd - The function to invoke when loading is complete.
 * @return {void}
 */
function loadPetEvolution(filename, callback, onEnd) {
	const loadPromise = new Promise((resolve, reject) => {
		Client.loadFile(filename, resolve, reject);
	});

	loadPromise
		.then(async file => {
			let wasSuccessful = false;
			try {
				console.log('Loading file "' + filename + '"...');
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;
				lua.mountFile(filename, buffer);

				const ctx = lua.ctx;

				// InsertEvolutionRecipeLGU(BaseEggID, TargetEggID, MaterialID, Amount)
				ctx.InsertEvolutionRecipeLGU = (baseEggID, targetEggID, matID, amt) => {
					const baseJobID = EggIDToJobID[baseEggID];
					if (!baseJobID || !PetDBTable[baseJobID]) {
						console.warn(`[PetEvolution] Unknown base EggID ${baseEggID}`);
						return 1;
					}

					const pet = PetDBTable[baseJobID];

					if (!pet.Evolution) {
						pet.Evolution = {};
					}
					if (!pet.Evolution[targetEggID]) {
						pet.Evolution[targetEggID] = [];
					}

					pet.Evolution[targetEggID].push({
						MaterialID: Number(matID),
						Amount: Number(amt)
					});

					return 1;
				};

				// InsertPetAutoFeeding(PetID)
				ctx.InsertPetAutoFeeding = eggID => {
					const jobID = EggIDToJobID[eggID];

					if (!jobID || !PetDBTable[jobID]) {
						console.warn(`[PetAutoFeeding] Unknown EggID ${eggID}`);
						return 1;
					}

					PetDBTable[jobID].AutoFeeding = true;
					return 1;
				};

				// Execute Lua
				await lua.doFile(filename);
				lua.doStringSync('main()');
				wasSuccessful = true;
			} catch (error) {
				console.error('[loadPetEvolutionFile] Error: ', error);
			} finally {
				// release file from memmory
				lua.unmountFile(filename);
				// call onEnd
				onEnd(wasSuccessful);
			}
		})
		.catch(error => {
			if (typeof onEnd === 'function') {
				onEnd(false);
			}
		});
}
/**
 * Fog entry parser
 *
 * @param {number} index
 * @param {mixed} key
 * @param {string} near
 * @param {string} far
 * @param {string} color
 * @param {string} factor
 */
function parseFogEntry(index, key, near, far, color, factor) {
	const int_color = parseInt(color, 16);
	const map = MapTable[key] || (MapTable[key] = {});

	map.fog = {
		near: parseFloat(near),
		far: parseFloat(far),
		color: [(255 & (int_color >> 16)) / 255.0, (255 & (int_color >> 8)) / 255.0, (255 & (int_color >> 0)) / 255.0],
		factor: parseFloat(factor)
	};
}

/**
 * Indoor entry parser
 *
 * @param {number} index
 * @param {mixed} key
 */
function parseIndoorEntry(index, key) {
	key = key.replace('.gat', '.rsw');
	const map = MapTable[key] || (MapTable[key] = {});
	map.indoor = true;
}

/**
 * Quest entry parser
 *
 * @param {number} index
 * @param {string} title
 * @param {string} group
 * @param {string} image
 * @param {string} description
 * @param {string} summary
 */
function parseQuestEntry(index, key, title, group, image, description, summary) {
	const quest = QuestInfo[key] || (QuestInfo[key] = {});

	quest.Title = title;
	quest.Group = group;
	quest.Image = image;
	quest.Description = description;
	quest.Summary = summary;
}

/**
 * Load CashShopBanner file
 *
 * @param {string} filename to load
 * @param {function} callback - (Unused)
 * @param {function} onEnd to run once the file is loaded
 */
function loadCashShopBanner(filename, callback, onEnd) {
	Client.loadFile(
		filename,
		async function (file) {
			try {
				console.log('Loading file "' + filename + '"...');

				// check if file is ArrayBuffer and convert to Uint8Array if necessary
				const buffer = file instanceof ArrayBuffer ? new Uint8Array(file) : file;

				// get context
				const ctx = lua.ctx;

				// Define the function that Lua will call
				// add_cashshop_banner( bitmap_name, url )
				ctx.add_cashshop_banner = function (bitmap_name, url) {
					const decoded_bmp = userStringDecoder.decode(bitmap_name);
					const decoded_url = userStringDecoder.decode(url);

					CashShopBannerTable.push({
						bmp: decoded_bmp,
						url: decoded_url
					});
					return 1;
				};

				// mount file
				lua.mountFile(filename, buffer);

				// execute file to load the table and function definition
				await lua.doFile(filename);

				// execute the main function: set_cashshop_banner()
				lua.doStringSync(`
						if set_cashshop_banner then
							set_cashshop_banner()
						end
					`);
			} catch (error) {
				console.error('[loadCashShopBanner] Error: ', error);
			} finally {
				lua.unmountFile(filename);
				onEnd(true);
			}
		},
		// onError for Client.loadFile
		function () {
			if (onEnd) {
				onEnd(false);
			}
		}
	);
}

function onUpdateOwnerName(pkt) {
	DB.CNameTable[pkt.GID] = pkt.CName;
	DB._resolveNameCallbacks(pkt.GID, pkt.CName);
	DB.UpdateOwnerName[pkt.GID] = pkt;
}

/**
 * Function to update MapTable with MapInfo values
 */
function updateMapTable() {
	for (const key in MapInfo) {
		if (MapInfo.hasOwnProperty(key)) {
			if (MapTable[key]) {
				MapTable[key].name = MapInfo[key].displayName;
			} else {
				MapTable[key] = { name: MapInfo[key].displayName };
			}
		}
	}
}

/**
 * Export
 */
export default DB;
