/**
 * DB/Map/MapState.js
 *
 * List of Map Property, Type and State
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alison Serafim
 */

export default {
	MapProperty: {
		// seems to be used only 0, 1 and 3
		NOTHING: 0,
		FREEPVPZONE: 1,
		EVENTPVPZONE: 2,
		AGITZONE: 3,
		PKSERVERZONE: 4, // message "You are in a PK area. Please beware of sudden attacks." in color 0x9B9BFF (light red)
		PVPSERVERZONE: 5,
		DENYSKILLZONE: 6
	},

	MapType: {
		VILLAGE: 0,
		VILLAGE_IN: 1,
		FIELD: 2,
		DUNGEON: 3,
		ARENA: 4,
		PENALTY_FREEPKZONE: 5,
		NOPENALTY_FREEPKZONE: 6,
		EVENT_GUILDWAR: 7,
		AGIT: 8,
		DUNGEON2: 9,
		DUNGEON3: 10,
		PKSERVER: 11,
		PVPSERVER: 12,
		DENYSKILL: 13,
		TURBOTRACK: 14,
		JAIL: 15,
		MONSTERTRACK: 16,
		PORINGBATTLE: 17,
		AGIT_SIEGEV15: 18,
		BATTLEFIELD: 19,
		PVP_TOURNAMENT: 20,
		//Map types 21 - 24 not used.
		SIEGE_LOWLEVEL: 25,
		//Map types 26 - 28 remains opens for future types.
		UNUSED: 29
	},

	MapFlag: {
		PVP: 1 << 0,
		GVG: 1 << 1,
		SIEGE: 1 << 2,
		USE_SIMPLE_EFFECT: 1 << 3,
		DISABLE_LOCKON: 1 << 4,
		COUNT_PK: 1 << 5,
		NO_PARTY_FORMATION: 1 << 6,
		BATTLEFIELD: 1 << 7,
		DISABLE_COSTUMEITEM: 1 << 8,
		USECART: 1 << 9,
		SUNMOONSTAR_MIRACLE: 1 << 10
	}
};
