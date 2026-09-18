/**
 * Engine/Replay/ReplayTypes.js
 *
 * Ragnarok Online Replay Format (.rrf) Types & Constants
 * Specification derived from adsonpleal/rrfparser reference implementation.
 */

import Configs from 'Core/Configs.js';

/**
 * Informational replay logging, only emitted in development builds.
 * Warnings and errors are reported unconditionally.
 *
 * @param {...*} args
 */
export function replayLog(...args) {
	if (Configs.get('development', false)) {
		console.log(...args);
	}
}

export const ContainerType = {
	None: 0,
	PacketStream: 1,
	ReplayData: 2,
	Session: 3,
	Status: 4,
	Quests: 6,
	GroupAndFriends: 7,
	Items: 8,
	Companions: 9,
	InitialPackets: 14,
	InitialEntities: 15,
	InitialFloorItems: 16,
	Efst: 17,
	EfstList: 18,
	EntitiesInfo: 21
};

export const ContainerTypeNames = {};

for (const [name, code] of Object.entries(ContainerType)) {
	ContainerTypeNames[code] = name;
}

export const ItemChunkKind = {
	INVENTORY: 4510,
	CART: 4516,
	CART_MIRROR: 4518,
	EQUIPPED: 4601,
	EQUIPPED_COSTUME: 4603
};

export const ItemRecordTag = {
	START_CHAIN: 281,
	END_CHAIN: 282,
	POS: 285,
	WEAR_STATE: 286,
	QTY: 287,
	CARDS: 290,
	NAMEID: 291,
	REFINE: 295,
	SPRITE: 298,
	GRADE: 299,
	OPTIONS_COUNT: 300,
	OPTIONS: 0x012d // 301
};

export const ReplayOpCodes = {
	ZC_ROOM_NEWENTRY: 201,
	ZC_STORE_ENTRY: 202,
	ZC_PROGRESS: 203,
	ZC_BUYING_STORE_ENTRY: 204,
	ZC_ITEM_ENTRY: 207,
	ZC_NOTIFY_MOVEENTRY8: 208,
	ZC_NOTIFY_STANDENTRY7: 210,
	ZC_NOTIFY_MOVEENTRY11: 214,
	ZC_NOTIFY_STANDENTRY11: 217,

	Begin_950: 950,
	End_950: 951,
	Region: 961,
	Service: 962,
	Sex: 963,
	Charactername: 964,
	Mapname: 965,
	Maptype: 966,
	PosX: 967,
	PosY: 968,
	Direction: 969,
	Length: 970,

	Begin_1000: 1000,
	End_1000: 1001,
	Aid: 1010,
	Gid: 1011,
	Job: 1014,
	Exp: 1015,
	Level: 1016,
	JobPoint: 1017,
	NextExp: 1018,
	JobLevel: 1019,
	SkillPoint: 1020,
	Str: 1024,
	Agi: 1025,
	Vit: 1026,
	Int: 1027,
	Dex: 1028,
	Luk: 1029,
	PlusStr: 1030,
	PlusAgi: 1031,
	PlusVit: 1032,
	PlusInt: 1033,
	PlusDex: 1034,
	PlusLuk: 1035,
	ASPD: 1036,
	AttackPower: 1037,
	MDefPower: 1038,
	PlusASPD: 1039,
	ItemDefPower: 1040,
	PlusDefPower: 1041,
	RefiningPower: 1042,
	MaxMattPower: 1043,
	MinMattPower: 1044,
	PlusMDefPower: 1045,
	HitSuccessValue: 1046,
	AvoidSuccessValue: 1047,
	CriticalSuccessValue: 1048,
	PlusAvoidSuccessValue: 1049,
	EquipArrowIndex: 1050,
	Money: 1051,
	Speed: 1052,
	Honor: 1053,
	NextJobExp: 1054,
	JobExp: 1055,
	Virtue: 1056,
	Head: 1060,
	Weapon: 1061,
	Shield: 1062,
	BodyPalette: 1063,
	HeadPalette: 1064,
	Accessory: 1065,
	Accessory2: 1066,
	Accessory3: 1067,
	EffectState: 1070,
	RobePalette: 1071,
	CartCurCount: 1086,
	CartMaxCount: 1087,
	CartCurWeight: 1088,
	CartMaxWeight: 1089,

	Begin_2000: 2000,
	End_2000: 2001,
	BonusStr: 2010,
	BonusAgi: 2011,
	BonusVit: 2012,
	BonusInt: 2013,
	BonusDex: 2014,
	BonusLuk: 2015,

	CurWeight: 2017,
	MaxWeight: 2018,
	Hp: 2029,
	MaxHp: 2030,
	Sp: 2031,
	MaxSp: 2032,

	Begin_3500: 3500,
	End_3500: 3501,
	QuestInfo: 3510,

	Begin_4000: 4000,
	End_4000: 4001,
	GroupName: 4014,
	GroupInfo: 4015,
	FriendInfo: 4016,

	Begin_4500: 4500,
	End_4500: 4501,
	InventoryItems: 4510,
	EquippedItems: 4601,
	EquippedShadowItems: 4603,
	CartItems: 4516,

	Begin_5000: 5000,
	End_5000: 5001,

	Begin_5300: 5300,
	PetGid: 5301,
	PetIsNameModified: 5302,
	PetName: 5303,
	PetAccessory: 5304,
	PetJob: 5305,
	PetLevel: 5306,
	PetFullness: 5307,
	PetRelation: 5308,
	PetEggIndex: 5309,
	PetOldFullness: 5310,

	Begin_5500: 5500,
	End_5500: 5501,

	Begin_6500: 6500,
	End_6500: 6501,

	Begin_7000: 7000,
	End_7000: 7001,

	Begin_8000: 8000,
	End_8000: 8001,

	Begin_10000: 10000,
	End_10000: 10001,

	Begin_13000: 13000,
	NameList: 13003,
	End_13000: 13001,

	Begin_14000: 14000,
	End_14000: 14001,

	Begin_15000: 15000,
	End_15000: 15001,

	Begin_17000: 17000,
	End_17000: 17001,

	Begin_19000: 19000,
	End_19000: 19001,
	EfstInfo: 19011,

	Begin_20000: 20000,
	End_20000: 20001,

	Begin_25000: 25000,
	End_25000: 25001,

	Begin_26000: 26000,
	End_26000: 26001
};

export const ReplayOpCodeName = {};

for (const [name, code] of Object.entries(ReplayOpCodes)) {
	ReplayOpCodeName[code] = name;
}

/**
 * States of the Replay playback state machine.
 */
export const ReplayState = {
	IDLE: 'IDLE',
	LOADING_REPLAY: 'LOADING_REPLAY',
	APPLYING_SESSION: 'APPLYING_SESSION',
	LOADING_MAP: 'LOADING_MAP',
	PLAYING_INITIAL_DATA: 'PLAYING_INITIAL_DATA',
	INITIAL_DATA_COMPLETE: 'INITIAL_DATA_COMPLETE',
	PLAYING_PACKET_STREAM: 'PLAYING_PACKET_STREAM',
	REPLAY_FINISHED: 'REPLAY_FINISHED'
};
