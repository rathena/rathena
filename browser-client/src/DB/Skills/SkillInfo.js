/**
 * DB/Skills/SkillInfo.js
 *
 * Manage skills
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import SK from './SkillConst.js';
import JobId from 'DB/Jobs/JobConst.js';

const SkillInfo = {};

SkillInfo[SK.SN_WINDWALK] = {
	Name: 'SN_WINDWALK',
	SkillName: 'Wind Walker',
	MaxLv: 10,
	SpAmount: [46, 52, 58, 64, 70, 76, 82, 88, 94, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AC_CONCENTRATION, 9]]
};
SkillInfo[SK.AB_VITUPERATUM] = {
	Name: 'AB_VITUPERATUM',
	SkillName: 'Vituperatum',
	MaxLv: 5,
	SpAmount: [144, 120, 106, 92, 78],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 5, 5],
	_NeedSkillList: [
		[SK.AB_EXPIATIO, 1],
		[SK.AB_EPICLESIS, 1]
	]
};
SkillInfo[SK.AB_CONVENIO] = {
	Name: 'AB_CONVENIO',
	SkillName: 'Convenio',
	MaxLv: 1,
	SpAmount: [70],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.AB_ANCILLA, 1],
		[SK.AB_ORATIO, 5]
	]
};
SkillInfo[SK.AL_RUWACH] = {
	Name: 'AL_RUWACH',
	SkillName: 'Ruwach',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [10]
};
SkillInfo[SK.WS_MELTDOWN] = {
	Name: 'WS_MELTDOWN',
	SkillName: 'Shattering Strike',
	MaxLv: 10,
	SpAmount: [50, 50, 60, 60, 70, 70, 80, 80, 90, 90],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BS_SKINTEMPER, 3],
		[SK.BS_HILTBINDING, 1],
		[SK.BS_WEAPONRESEARCH, 5],
		[SK.BS_OVERTHRUST, 3]
	]
};
SkillInfo[SK.WS_CREATECOIN] = {
	Name: 'WS_CREATECOIN',
	SkillName: 'Coin Craft',
	MaxLv: 3,
	SpAmount: [10, 20, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.MER_MAGNIFICAT] = {
	Name: 'MER_MAGNIFICAT',
	SkillName: 'Magnificat',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.WS_CREATENUGGET] = {
	Name: 'WS_CREATENUGGET',
	SkillName: 'Nugget Craft',
	MaxLv: 3,
	SpAmount: [10, 20, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.WS_CARTBOOST] = {
	Name: 'WS_CARTBOOST',
	SkillName: 'Cart Boost',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.MC_PUSHCART, 5], [SK.BS_HILTBINDING, 1], [SK.MC_CARTREVOLUTION], [SK.MC_CHANGECART]]
};
SkillInfo[SK.WS_SYSTEMCREATE] = {
	Name: 'WS_SYSTEMCREATE',
	SkillName: 'Battle Machine Craft',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.ST_CHASEWALK] = {
	Name: 'ST_CHASEWALK',
	SkillName: 'Stealth',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.TF_HIDING, 5],
		[SK.RG_TUNNELDRIVE, 3]
	]
};
SkillInfo[SK.ST_REJECTSWORD] = {
	Name: 'ST_REJECTSWORD',
	SkillName: 'Counter Instinct',
	MaxLv: 5,
	SpAmount: [10, 15, 20, 25, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.ST_STEALBACKPACK] = {
	Name: 'ST_STEALBACKPACK',
	SkillName: 'Steal Lunch Money',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_HEATER] = {
	Name: 'EL_HEATER',
	SkillName: 'Heater',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.CR_ALCHEMY] = {
	Name: 'CR_ALCHEMY',
	SkillName: 'Alchemy',
	MaxLv: 0,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: []
};
SkillInfo[SK.CR_SYNTHESISPOTION] = {
	Name: 'CR_SYNTHESISPOTION',
	SkillName: 'Potion Synthesis',
	MaxLv: 0,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: []
};
SkillInfo[SK.CG_ARROWVULCAN] = {
	Name: 'CG_ARROWVULCAN',
	SkillName: 'Arrow Vulcan',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.AC_DOUBLE, 5],
			[SK.AC_SHOWER, 5],
			[SK.BA_MUSICALSTRIKE, 1]
		],
		[JobId.DANCER_H]: [
			[SK.AC_DOUBLE, 5],
			[SK.AC_SHOWER, 5],
			[SK.DC_THROWARROW, 1]
		]
	}
};
SkillInfo[SK.CG_MOONLIT] = {
	Name: 'CG_MOONLIT',
	SkillName: 'Sheltering Bliss',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.AC_CONCENTRATION, 5],
			[SK.BA_MUSICALLESSON, 7]
		],
		[JobId.DANCER_H]: [
			[SK.AC_CONCENTRATION, 5],
			[SK.DC_DANCINGLESSON, 7]
		]
	}
};
SkillInfo[SK.CG_MARIONETTE] = {
	Name: 'CG_MARIONETTE',
	SkillName: 'Marionette Control',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [7],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.BA_MUSICALLESSON, 5]
		],
		[JobId.DANCER_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.DC_DANCINGLESSON, 5]
		]
	}
};
SkillInfo[SK.LK_SPIRALPIERCE] = {
	Name: 'LK_SPIRALPIERCE',
	SkillName: 'Clashing Spiral',
	MaxLv: 5,
	SpAmount: [18, 21, 24, 27, 30],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 4, 4],
	_NeedSkillList: [
		[SK.KN_SPEARMASTERY, 5],
		[SK.KN_PIERCE, 5],
		[SK.KN_RIDING, 1],
		[SK.KN_SPEARSTAB, 5]
	]
};
SkillInfo[SK.LK_HEADCRUSH] = {
	Name: 'LK_HEADCRUSH',
	SkillName: 'Traumatic Blow',
	MaxLv: 5,
	SpAmount: [23, 23, 23, 23, 23],
	bSeperateLv: false,
	AttackRange: [4, 4, 4, 4, 4],
	_NeedSkillList: [
		[SK.KN_SPEARMASTERY, 9],
		[SK.KN_RIDING, 1]
	]
};
SkillInfo[SK.LK_JOINTBEAT] = {
	Name: 'LK_JOINTBEAT',
	SkillName: 'Vital Strike',
	MaxLv: 10,
	SpAmount: [12, 12, 14, 14, 16, 16, 18, 18, 20, 20],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
	_NeedSkillList: [
		[SK.KN_CAVALIERMASTERY, 3],
		[SK.LK_HEADCRUSH, 3]
	]
};
SkillInfo[SK.AL_PNEUMA] = {
	Name: 'AL_PNEUMA',
	SkillName: 'Pneuma',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.AL_WARP, 4]]
};
SkillInfo[SK.HW_NAPALMVULCAN] = {
	Name: 'HW_NAPALMVULCAN',
	SkillName: 'Napalm Vulcan',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_NAPALMBEAT, 5]]
};
SkillInfo[SK.CH_SOULCOLLECT] = {
	Name: 'CH_SOULCOLLECT',
	SkillName: 'Zen',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.MO_EXPLOSIONSPIRITS, 5]]
};
SkillInfo[SK.PF_MINDBREAKER] = {
	Name: 'PF_MINDBREAKER',
	SkillName: 'Mind Breaker',
	MaxLv: 5,
	SpAmount: [12, 15, 18, 21, 24],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_SRECOVERY, 3],
		[SK.PF_SOULBURN, 2]
	]
};
SkillInfo[SK.PF_MEMORIZE] = {
	Name: 'PF_MEMORIZE',
	SkillName: 'Foresight',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SA_ADVANCEDBOOK, 5],
		[SK.SA_FREECAST, 5],
		[SK.SA_AUTOSPELL, 1]
	]
};
SkillInfo[SK.PF_FOGWALL] = {
	Name: 'PF_FOGWALL',
	SkillName: 'Blinding Mist',
	MaxLv: 1,
	SpAmount: [25],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [
		[SK.SA_VIOLENTGALE, 2],
		[SK.SA_DELUGE, 2]
	]
};
SkillInfo[SK.PF_SPIDERWEB] = {
	Name: 'PF_SPIDERWEB',
	SkillName: 'Fiber Lock',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.SA_DRAGONOLOGY, 4]]
};
SkillInfo[SK.ASC_METEORASSAULT] = {
	Name: 'ASC_METEORASSAULT',
	SkillName: 'Meteor Assault',
	MaxLv: 10,
	SpAmount: [10, 12, 14, 16, 18, 20, 22, 24, 26, 28],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AS_KATAR, 5],
		[SK.AS_RIGHT, 3],
		[SK.AS_SONICBLOW, 5],
		[SK.ASC_BREAKER, 1]
	]
};
SkillInfo[SK.ASC_CDP] = {
	Name: 'ASC_CDP',
	SkillName: 'Create Deadly Poison',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.TF_POISON, 10],
		[SK.TF_DETOXIFY, 1],
		[SK.AS_ENCHANTPOISON, 5]
	]
};
SkillInfo[SK.WE_BABY] = {
	Name: 'WE_BABY',
	SkillName: 'Mom, Dad, I love you!',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.WE_CALLPARENT] = {
	Name: 'WE_CALLPARENT',
	SkillName: 'Mom, Dad, I miss you!',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WE_CALLBABY] = {
	Name: 'WE_CALLBABY',
	SkillName: 'Come to me, honey~',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.TK_RUN] = {
	Name: 'TK_RUN',
	SkillName: 'Sprint',
	MaxLv: 10,
	SpAmount: [100, 90, 80, 70, 60, 50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_READYSTORM] = {
	Name: 'TK_READYSTORM',
	SkillName: 'Tornado Stance',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_STORMKICK, 1]]
};
SkillInfo[SK.TK_STORMKICK] = {
	Name: 'TK_STORMKICK',
	SkillName: 'Tornado Kick',
	MaxLv: 7,
	SpAmount: [14, 12, 10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_READYDOWN] = {
	Name: 'TK_READYDOWN',
	SkillName: 'Heel Drop Stance',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_DOWNKICK, 1]]
};
SkillInfo[SK.TK_DOWNKICK] = {
	Name: 'TK_DOWNKICK',
	SkillName: 'Heel Drop',
	MaxLv: 7,
	SpAmount: [14, 12, 10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AL_TELEPORT] = {
	Name: 'AL_TELEPORT',
	SkillName: 'Teleport',
	MaxLv: 2,
	SpAmount: [10, 9],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.AL_RUWACH, 1]]
};
SkillInfo[SK.TK_READYTURN] = {
	Name: 'TK_READYTURN',
	SkillName: 'Roundhouse Stance',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_TURNKICK, 1]]
};
SkillInfo[SK.TK_TURNKICK] = {
	Name: 'TK_TURNKICK',
	SkillName: 'Roundhouse',
	MaxLv: 7,
	SpAmount: [14, 12, 10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_READYCOUNTER] = {
	Name: 'TK_READYCOUNTER',
	SkillName: 'Counter Kick Stance',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_COUNTER, 1]]
};
SkillInfo[SK.TK_COUNTER] = {
	Name: 'TK_COUNTER',
	SkillName: 'Counter Kick',
	MaxLv: 7,
	SpAmount: [14, 12, 10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_DODGE] = {
	Name: 'TK_DODGE',
	SkillName: 'Tumbling',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_JUMPKICK, 7]]
};
SkillInfo[SK.TK_JUMPKICK] = {
	Name: 'TK_JUMPKICK',
	SkillName: 'Flying Kick',
	MaxLv: 7,
	SpAmount: [70, 60, 50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.TK_HPTIME] = {
	Name: 'TK_HPTIME',
	SkillName: 'Peaceful Break',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_SPTIME] = {
	Name: 'TK_SPTIME',
	SkillName: 'Happy Break',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_POWER] = {
	Name: 'TK_POWER',
	SkillName: 'Kihop',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.TK_SEVENWIND] = {
	Name: 'TK_SEVENWIND',
	SkillName: 'Mild Wind',
	MaxLv: 7,
	SpAmount: [20, 20, 20, 20, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.TK_HPTIME, 5],
		[SK.TK_SPTIME, 5],
		[SK.TK_POWER, 5]
	]
};
SkillInfo[SK.TK_HIGHJUMP] = {
	Name: 'TK_HIGHJUMP',
	SkillName: 'Leap',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [2, 4, 6, 8, 10]
};
SkillInfo[SK.SG_FEEL] = {
	Name: 'SG_FEEL',
	SkillName: 'Solar, Lunar and Stellar Perception',
	MaxLv: 3,
	SpAmount: [100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.SG_SUN_WARM] = {
	Name: 'SG_SUN_WARM',
	SkillName: 'Solar Heat',
	MaxLv: 3,
	SpAmount: [20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 1]]
};
SkillInfo[SK.SG_MOON_WARM] = {
	Name: 'SG_MOON_WARM',
	SkillName: 'Lunar Heat',
	MaxLv: 3,
	SpAmount: [20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 2]]
};
SkillInfo[SK.SG_STAR_WARM] = {
	Name: 'SG_STAR_WARM',
	SkillName: 'Stellar Heat',
	MaxLv: 3,
	SpAmount: [10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 3]]
};
SkillInfo[SK.SG_SUN_COMFORT] = {
	Name: 'SG_SUN_COMFORT',
	SkillName: 'Solar Protection',
	MaxLv: 4,
	SpAmount: [70, 60, 50, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 1]]
};
SkillInfo[SK.AL_WARP] = {
	Name: 'AL_WARP',
	SkillName: 'Warp Portal',
	MaxLv: 4,
	SpAmount: [35, 32, 29, 26],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_TELEPORT, 2]]
};
SkillInfo[SK.SG_MOON_COMFORT] = {
	Name: 'SG_MOON_COMFORT',
	SkillName: 'Lunar Protection',
	MaxLv: 4,
	SpAmount: [70, 60, 50, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 2]]
};
SkillInfo[SK.SG_STAR_COMFORT] = {
	Name: 'SG_STAR_COMFORT',
	SkillName: 'Stellar Protection',
	MaxLv: 4,
	SpAmount: [70, 60, 50, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.SG_FEEL, 3]]
};
SkillInfo[SK.SG_HATE] = {
	Name: 'SG_HATE',
	SkillName: 'Solar, Lunar and Stellar Opposition',
	MaxLv: 3,
	SpAmount: [100, 100, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9]
};
SkillInfo[SK.SG_SUN_ANGER] = {
	Name: 'SG_SUN_ANGER',
	SkillName: 'Solar Wrath',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_HATE, 1]]
};
SkillInfo[SK.SG_MOON_ANGER] = {
	Name: 'SG_MOON_ANGER',
	SkillName: 'Lunar Wrath',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_HATE, 2]]
};
SkillInfo[SK.SG_STAR_ANGER] = {
	Name: 'SG_STAR_ANGER',
	SkillName: 'Stellar Wrath',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SG_HATE, 3]]
};
SkillInfo[SK.SG_SUN_BLESS] = {
	Name: 'SG_SUN_BLESS',
	SkillName: 'Solar Blessings',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SG_FEEL, 1],
		[SK.SG_HATE, 1]
	]
};
SkillInfo[SK.SG_MOON_BLESS] = {
	Name: 'SG_MOON_BLESS',
	SkillName: 'Lunar Blessings',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SG_FEEL, 2],
		[SK.SG_HATE, 2]
	]
};
SkillInfo[SK.SG_STAR_BLESS] = {
	Name: 'SG_STAR_BLESS',
	SkillName: 'Stellar Blessings',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SG_FEEL, 3],
		[SK.SG_HATE, 3]
	]
};
SkillInfo[SK.SG_DEVIL] = {
	Name: 'SG_DEVIL',
	SkillName: 'Solar, Lunar and Stellar Shadow ',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_DEVELOPMENT] = {
	Name: 'GD_DEVELOPMENT',
	SkillName: 'Permanent Development',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SG_FRIEND] = {
	Name: 'SG_FRIEND',
	SkillName: 'Solar, Lunar and Stellar Team-Up',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.SG_KNOWLEDGE] = {
	Name: 'SG_KNOWLEDGE',
	SkillName: 'Solar, Lunar and Stellar Courier ',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SG_FUSION] = {
	Name: 'SG_FUSION',
	SkillName: 'Solar, Lunar and Stellar Union ',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SG_KNOWLEDGE, 9]]
};
SkillInfo[SK.SL_ALCHEMIST] = {
	Name: 'SL_ALCHEMIST',
	SkillName: 'Alchemist Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.AM_BERSERKPITCHER] = {
	Name: 'AM_BERSERKPITCHER',
	SkillName: 'Aid Berserk Potion',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.SL_MONK] = {
	Name: 'SL_MONK',
	SkillName: 'Monk Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.AL_HEAL] = {
	Name: 'AL_HEAL',
	SkillName: 'Heal',
	MaxLv: 10,
	SpAmount: [13, 16, 19, 22, 25, 28, 31, 34, 37, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	NeedSkillList: {
		[JobId.CRUSADER]: [
			[SK.CR_TRUST, 10],
			[SK.AL_DEMONBANE, 5]
		]
	}
};
SkillInfo[SK.SL_STAR] = {
	Name: 'SL_STAR',
	SkillName: 'Taekwon Master Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SL_SAGE] = {
	Name: 'SL_SAGE',
	SkillName: 'Sage Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MER_QUICKEN] = {
	Name: 'MER_QUICKEN',
	SkillName: 'Weapon Quicken',
	MaxLv: 10,
	SpAmount: [14, 18, 22, 26, 30, 34, 38, 42, 46, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SL_CRUSADER] = {
	Name: 'SL_CRUSADER',
	SkillName: 'Crusader Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SL_SUPERNOVICE] = {
	Name: 'SL_SUPERNOVICE',
	SkillName: 'Super Novice Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_STAR, 1]]
};
SkillInfo[SK.SL_KNIGHT] = {
	Name: 'SL_KNIGHT',
	SkillName: 'Knight Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_CRUSADER, 1]]
};
SkillInfo[SK.SL_WIZARD] = {
	Name: 'SL_WIZARD',
	SkillName: 'Wizard Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_SAGE, 1]]
};
SkillInfo[SK.SL_PRIEST] = {
	Name: 'SL_PRIEST',
	SkillName: 'Priest Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_MONK, 1]]
};
SkillInfo[SK.SL_BARDDANCER] = {
	Name: 'SL_BARDDANCER',
	SkillName: 'Bard and Dancer Spirits',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.EL_TROPIC] = {
	Name: 'EL_TROPIC',
	SkillName: 'Tropic',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SL_ROGUE] = {
	Name: 'SL_ROGUE',
	SkillName: 'Rogue Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_ASSASIN, 1]]
};
SkillInfo[SK.SL_ASSASIN] = {
	Name: 'SL_ASSASIN',
	SkillName: 'Assassin Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SL_BLACKSMITH] = {
	Name: 'SL_BLACKSMITH',
	SkillName: 'Blacksmith Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_ALCHEMIST, 1]]
};
SkillInfo[SK.BS_ADRENALINE2] = {
	Name: 'BS_ADRENALINE2',
	SkillName: 'Advanced Adrenaline Rush',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [64],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.BS_ADRENALINE, 5]]
};
SkillInfo[SK.SL_HUNTER] = {
	Name: 'SL_HUNTER',
	SkillName: 'Hunter Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_BARDDANCER, 1]]
};
SkillInfo[SK.SL_SOULLINKER] = {
	Name: 'SL_SOULLINKER',
	SkillName: 'Soul Linker Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_STAR, 1]]
};
SkillInfo[SK.SL_KAIZEL] = {
	Name: 'SL_KAIZEL',
	SkillName: 'Kaizel',
	MaxLv: 7,
	SpAmount: [120, 110, 100, 90, 80, 70, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_PRIEST, 1]]
};
SkillInfo[SK.SL_KAAHI] = {
	Name: 'SL_KAAHI',
	SkillName: 'Kaahi',
	MaxLv: 7,
	SpAmount: [30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SL_CRUSADER, 1],
		[SK.SL_MONK, 1],
		[SK.SL_PRIEST, 1]
	]
};
SkillInfo[SK.AL_INCAGI] = {
	Name: 'AL_INCAGI',
	SkillName: 'Increase Agility',
	MaxLv: 10,
	SpAmount: [18, 21, 24, 27, 30, 33, 36, 39, 42, 45],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_HEAL, 3]]
};
SkillInfo[SK.SL_KAUPE] = {
	Name: 'SL_KAUPE',
	SkillName: 'Kaupe',
	MaxLv: 3,
	SpAmount: [20, 30, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [
		[SK.SL_ASSASIN, 1],
		[SK.SL_ROGUE, 1]
	]
};
SkillInfo[SK.SL_KAITE] = {
	Name: 'SL_KAITE',
	SkillName: 'Kaite',
	MaxLv: 7,
	SpAmount: [70, 70, 70, 70, 70, 70, 70],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SL_SAGE, 1],
		[SK.SL_WIZARD, 1]
	]
};
SkillInfo[SK.SL_KAINA] = {
	Name: 'SL_KAINA',
	SkillName: 'Kaina',
	MaxLv: 7,
	SpAmount: [0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TK_SPTIME, 1]]
};
SkillInfo[SK.SL_STIN] = {
	Name: 'SL_STIN',
	SkillName: 'Estin',
	MaxLv: 7,
	SpAmount: [18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_WIZARD, 1]]
};
SkillInfo[SK.SL_STUN] = {
	Name: 'SL_STUN',
	SkillName: 'Estun',
	MaxLv: 7,
	SpAmount: [18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_WIZARD, 1]]
};
SkillInfo[SK.SL_SMA] = {
	Name: 'SL_SMA',
	SkillName: 'Esma',
	MaxLv: 10,
	SpAmount: [8, 16, 24, 32, 40, 48, 56, 64, 72, 80],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SL_STIN, 7],
		[SK.SL_STUN, 7]
	]
};
SkillInfo[SK.SL_SWOO] = {
	Name: 'SL_SWOO',
	SkillName: 'Eswoo',
	MaxLv: 7,
	SpAmount: [75, 65, 55, 45, 35, 25, 15],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_PRIEST, 1]]
};
SkillInfo[SK.SL_SKE] = {
	Name: 'SL_SKE',
	SkillName: 'Eske',
	MaxLv: 3,
	SpAmount: [45, 30, 15],
	bSeperateLv: false,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SL_KNIGHT, 1]]
};
SkillInfo[SK.SL_SKA] = {
	Name: 'SL_SKA',
	SkillName: 'Eska',
	MaxLv: 3,
	SpAmount: [100, 80, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SL_MONK, 1]]
};
SkillInfo[SK.ST_PRESERVE] = {
	Name: 'ST_PRESERVE',
	SkillName: 'Preserve',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.RG_PLAGIARISM, 10]]
};
SkillInfo[SK.ST_FULLSTRIP] = {
	Name: 'ST_FULLSTRIP',
	SkillName: 'Full Divestment',
	MaxLv: 5,
	SpAmount: [22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STRIPWEAPON, 5]]
};
SkillInfo[SK.WS_WEAPONREFINE] = {
	Name: 'WS_WEAPONREFINE',
	SkillName: 'Upgrade Weapon',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_WEAPONRESEARCH, 10]]
};
SkillInfo[SK.CR_SLIMPITCHER] = {
	Name: 'CR_SLIMPITCHER',
	SkillName: 'Aid Condensed Potion',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AM_POTIONPITCHER, 5]]
};
SkillInfo[SK.CR_FULLPROTECTION] = {
	Name: 'CR_FULLPROTECTION',
	SkillName: 'Full Chemical Protection',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AM_CP_WEAPON, 5],
		[SK.AM_CP_ARMOR, 5],
		[SK.AM_CP_SHIELD, 5],
		[SK.AM_CP_HELM, 5]
	]
};
SkillInfo[SK.AL_DECAGI] = {
	Name: 'AL_DECAGI',
	SkillName: 'Decrease Agility',
	MaxLv: 10,
	SpAmount: [15, 17, 19, 21, 23, 25, 27, 29, 31, 33],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_INCAGI, 1]]
};
SkillInfo[SK.PA_SHIELDCHAIN] = {
	Name: 'PA_SHIELDCHAIN',
	SkillName: 'Rapid Smiting',
	MaxLv: 5,
	SpAmount: [28, 31, 34, 37, 40],
	bSeperateLv: true,
	AttackRange: [7, 7, 9, 9, 11],
	_NeedSkillList: [[SK.CR_SHIELDBOOMERANG, 5]]
};
SkillInfo[SK.HP_MANARECHARGE] = {
	Name: 'HP_MANARECHARGE',
	SkillName: 'Spiritual Thrift',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.PR_MACEMASTERY, 10],
		[SK.AL_DEMONBANE, 10]
	]
};
SkillInfo[SK.PF_DOUBLECASTING] = {
	Name: 'PF_DOUBLECASTING',
	SkillName: 'Double Bolt',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_AUTOSPELL, 1]]
};
SkillInfo[SK.HW_GANBANTEIN] = {
	Name: 'HW_GANBANTEIN',
	SkillName: 'Ganbantein',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [18],
	_NeedSkillList: [
		[SK.WZ_ESTIMATION, 1],
		[SK.WZ_ICEWALL, 1]
	]
};
SkillInfo[SK.HW_GRAVITATION] = {
	Name: 'HW_GRAVITATION',
	SkillName: 'Gravitational Field',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [18, 18, 18, 18, 18],
	_NeedSkillList: [
		[SK.WZ_QUAGMIRE, 1],
		[SK.HW_MAGICCRASHER, 1],
		[SK.HW_MAGICPOWER, 10]
	]
};
SkillInfo[SK.WS_CARTTERMINATION] = {
	Name: 'WS_CARTTERMINATION',
	SkillName: 'High Speed Cart Ram',
	MaxLv: 10,
	SpAmount: [15, 15, 15, 15, 15, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MC_MAMMONITE, 10],
		[SK.BS_HAMMERFALL, 5],
		[SK.WS_CARTBOOST, 1]
	]
};
SkillInfo[SK.WS_OVERTHRUSTMAX] = {
	Name: 'WS_OVERTHRUSTMAX',
	SkillName: 'Maximum Power-Thrust',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_OVERTHRUST, 5]]
};
SkillInfo[SK.CG_LONGINGFREEDOM] = {
	Name: 'CG_LONGINGFREEDOM',
	SkillName: 'Longing for Freedom',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.CG_MARIONETTE, 1],
			[SK.BA_DISSONANCE, 3],
			[SK.BA_MUSICALLESSON, 10]
		],
		[JobId.DANCER_H]: [
			[SK.CG_MARIONETTE, 1],
			[SK.DC_UGLYDANCE, 3],
			[SK.DC_DANCINGLESSON, 10]
		]
	}
};
SkillInfo[SK.CG_HERMODE] = {
	Name: 'CG_HERMODE',
	SkillName: "Hermode's Rod",
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.BA_MUSICALLESSON, 10]
		],
		[JobId.DANCER_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.DC_DANCINGLESSON, 10]
		]
	}
};
SkillInfo[SK.CG_TAROTCARD] = {
	Name: 'CG_TAROTCARD',
	SkillName: 'Tarot Card of Fate',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.BA_DISSONANCE, 3]
		],
		[JobId.DANCER_H]: [
			[SK.AC_CONCENTRATION, 10],
			[SK.DC_UGLYDANCE, 3]
		]
	}
};
SkillInfo[SK.CR_ACIDDEMONSTRATION] = {
	Name: 'CR_ACIDDEMONSTRATION',
	SkillName: 'Acid Bomb',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AM_DEMONSTRATION, 5],
		[SK.AM_ACIDTERROR, 5]
	]
};
SkillInfo[SK.CR_CULTIVATION] = {
	Name: 'CR_CULTIVATION',
	SkillName: 'Cultivate Plant',
	MaxLv: 2,
	SpAmount: [10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1]
};
SkillInfo[SK.TK_MISSION] = {
	Name: 'TK_MISSION',
	SkillName: 'Taekwon Mission',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TK_POWER, 5]]
};
SkillInfo[SK.SL_HIGH] = {
	Name: 'SL_HIGH',
	SkillName: '1st Transcendent Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SL_SUPERNOVICE, 5]]
};
SkillInfo[SK.KN_ONEHAND] = {
	Name: 'KN_ONEHAND',
	SkillName: 'One Handed Quicken',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KN_TWOHANDQUICKEN, 10]]
};
SkillInfo[SK.AL_HOLYWATER] = {
	Name: 'AL_HOLYWATER',
	SkillName: 'Aqua Benedicta',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AM_TWILIGHT1] = {
	Name: 'AM_TWILIGHT1',
	SkillName: 'Spiritual Potion Creation 1',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AM_PHARMACY, 10]]
};
SkillInfo[SK.AM_TWILIGHT2] = {
	Name: 'AM_TWILIGHT2',
	SkillName: 'Spiritual Potion Creation 2',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AM_PHARMACY, 10]]
};
SkillInfo[SK.AM_TWILIGHT3] = {
	Name: 'AM_TWILIGHT3',
	SkillName: 'Spiritual Potion Creation 3',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AM_PHARMACY, 10]]
};
SkillInfo[SK.HT_POWER] = {
	Name: 'HT_POWER',
	SkillName: 'Beast Charge',
	MaxLv: 1,
	Type: 'Soul',
	SpAmount: [12],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.AC_DOUBLE, 10]]
};
SkillInfo[SK.GS_GLITTERING] = {
	Name: 'GS_GLITTERING',
	SkillName: 'Coin Flip',
	MaxLv: 5,
	SpAmount: [2, 2, 2, 2, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.RK_ENCHANTBLADE] = {
	Name: 'RK_ENCHANTBLADE',
	SkillName: 'Enchant Blade',
	MaxLv: 10,
	SpAmount: [34, 38, 42, 46, 50, 54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RK_RUNEMASTERY, 2]]
};
SkillInfo[SK.GS_FLING] = {
	Name: 'GS_FLING',
	SkillName: 'Coin Fling',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.RK_WINDCUTTER] = {
	Name: 'RK_WINDCUTTER',
	SkillName: 'Wind Cutter',
	MaxLv: 5,
	SpAmount: [23, 26, 29, 32, 35],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RK_ENCHANTBLADE, 5]]
};
SkillInfo[SK.GS_TRIPLEACTION] = {
	Name: 'GS_TRIPLEACTION',
	SkillName: 'Triple Action',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.RK_DRAGONHOWLING] = {
	Name: 'RK_DRAGONHOWLING',
	SkillName: 'Dragon Howling',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RK_DRAGONTRAINING, 2]]
};
SkillInfo[SK.GS_BULLSEYE] = {
	Name: 'GS_BULLSEYE',
	SkillName: "Bull's Eye",
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.GS_GLITTERING, 5]]
};
SkillInfo[SK.RK_REFRESH] = {
	Name: 'RK_REFRESH',
	SkillName: 'Refresh',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GS_MADNESSCANCEL] = {
	Name: 'GS_MADNESSCANCEL',
	SkillName: 'Last Stand',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.GS_GLITTERING, 4]]
};
SkillInfo[SK.RK_STORMBLAST] = {
	Name: 'RK_STORMBLAST',
	SkillName: 'Storm Blast',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GS_ADJUSTMENT] = {
	Name: 'GS_ADJUSTMENT',
	SkillName: "Gunslinger's Panic",
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.GS_GLITTERING, 4]]
};
SkillInfo[SK.GC_VENOMIMPRESS] = {
	Name: 'GC_VENOMIMPRESS',
	SkillName: 'Venom Impression',
	MaxLv: 5,
	SpAmount: [12, 16, 20, 24, 28],
	bSeperateLv: true,
	AttackRange: [10, 10, 10, 10, 10],
	_NeedSkillList: [[SK.AS_ENCHANTPOISON, 3]]
};
SkillInfo[SK.GS_INCREASING] = {
	Name: 'GS_INCREASING',
	SkillName: 'Increase Accuracy',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.GS_GLITTERING, 2]]
};
SkillInfo[SK.GC_CREATENEWPOISON] = {
	Name: 'GC_CREATENEWPOISON',
	SkillName: 'New Poison Creation',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.GC_RESEARCHNEWPOISON, 1]]
};
SkillInfo[SK.GS_MAGICALBULLET] = {
	Name: 'GS_MAGICALBULLET',
	SkillName: 'Magical Bullet',
	MaxLv: 1,
	SpAmount: [7],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.GC_COUNTERSLASH] = {
	Name: 'GC_COUNTERSLASH',
	SkillName: 'Counter Slash',
	MaxLv: 10,
	SpAmount: [5, 8, 11, 14, 17, 19, 21, 23, 25, 27],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GC_WEAPONBLOCKING, 1]]
};
SkillInfo[SK.GS_CRACKER] = {
	Name: 'GS_CRACKER',
	SkillName: 'Cracker',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.GC_CLOAKINGEXCEED] = {
	Name: 'GC_CLOAKINGEXCEED',
	SkillName: 'Cloaking Exceed',
	MaxLv: 5,
	SpAmount: [45, 45, 45, 45, 45],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_CLOAKING, 3]]
};
SkillInfo[SK.GS_SINGLEACTION] = {
	Name: 'GS_SINGLEACTION',
	SkillName: 'Single Action',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GC_CROSSRIPPERSLASHER] = {
	Name: 'GC_CROSSRIPPERSLASHER',
	SkillName: 'Cross Ripper Slasher',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [9, 10, 11, 12, 13],
	_NeedSkillList: [[SK.GC_ROLLINGCUTTER, 1]]
};
SkillInfo[SK.GS_SNAKEEYE] = {
	Name: 'GS_SNAKEEYE',
	SkillName: 'Snake Eyes',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AB_CLEMENTIA] = {
	Name: 'AB_CLEMENTIA',
	SkillName: 'Clementia',
	MaxLv: 3,
	SpAmount: [280, 320, 360],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.AL_BLESSING, 1]]
};
SkillInfo[SK.SM_SWORD] = {
	Name: 'SM_SWORD',
	SkillName: 'Sword Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AL_CRUCIS] = {
	Name: 'AL_CRUCIS',
	SkillName: 'Signum Crucis',
	MaxLv: 10,
	SpAmount: [35, 35, 35, 35, 35, 35, 35, 35, 35, 35],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AL_DEMONBANE, 3]]
};
SkillInfo[SK.GS_TRACKING] = {
	Name: 'GS_TRACKING',
	SkillName: 'Tracking',
	MaxLv: 10,
	SpAmount: [15, 20, 25, 30, 35, 40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_SINGLEACTION, 5]]
};
SkillInfo[SK.GS_DISARM] = {
	Name: 'GS_DISARM',
	SkillName: 'Disarm',
	MaxLv: 5,
	SpAmount: [15, 20, 25, 30, 35],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_TRACKING, 7]]
};
SkillInfo[SK.GS_PIERCINGSHOT] = {
	Name: 'GS_PIERCINGSHOT',
	SkillName: 'Wounding Shot',
	MaxLv: 5,
	SpAmount: [11, 12, 13, 14, 15],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_TRACKING, 5]]
};
SkillInfo[SK.GS_RAPIDSHOWER] = {
	Name: 'GS_RAPIDSHOWER',
	SkillName: 'Trigger Happy Shot',
	MaxLv: 10,
	SpAmount: [22, 24, 26, 28, 30, 32, 34, 36, 38, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_CHAINACTION, 3]]
};
SkillInfo[SK.GS_DESPERADO] = {
	Name: 'GS_DESPERADO',
	SkillName: 'Desperado',
	MaxLv: 10,
	SpAmount: [32, 34, 36, 38, 40, 42, 44, 46, 48, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GS_RAPIDSHOWER, 5]]
};
SkillInfo[SK.GS_GATLINGFEVER] = {
	Name: 'GS_GATLINGFEVER',
	SkillName: 'Gatling Feaver',
	MaxLv: 10,
	SpAmount: [30, 32, 34, 36, 38, 40, 42, 44, 46, 48],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.GS_RAPIDSHOWER, 7],
		[SK.GS_DESPERADO, 5]
	]
};
SkillInfo[SK.GS_DUST] = {
	Name: 'GS_DUST',
	SkillName: 'Crowd Control Shot',
	MaxLv: 10,
	SpAmount: [3, 6, 9, 12, 15, 18, 21, 24, 27, 30],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.GS_SINGLEACTION, 5]]
};
SkillInfo[SK.GS_FULLBUSTER] = {
	Name: 'GS_FULLBUSTER',
	SkillName: 'Full Blast',
	MaxLv: 10,
	SpAmount: [20, 25, 30, 35, 40, 45, 50, 55, 60, 65],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_DUST, 3]]
};
SkillInfo[SK.GS_SPREADATTACK] = {
	Name: 'GS_SPREADATTACK',
	SkillName: 'Spread Attack',
	MaxLv: 10,
	SpAmount: [13, 16, 19, 22, 25, 28, 31, 34, 37, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_SINGLEACTION, 5]]
};
SkillInfo[SK.GS_GROUNDDRIFT] = {
	Name: 'GS_GROUNDDRIFT',
	SkillName: 'Gunslinger Mine',
	MaxLv: 10,
	SpAmount: [3, 6, 9, 12, 15, 18, 21, 24, 27, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GS_SPREADATTACK, 7]]
};
SkillInfo[SK.NJ_TOBIDOUGU] = {
	Name: 'NJ_TOBIDOUGU',
	SkillName: 'Dagger Throwing Practice',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NJ_SYURIKEN] = {
	Name: 'NJ_SYURIKEN',
	SkillName: 'Throw Shuriken',
	MaxLv: 10,
	SpAmount: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_TOBIDOUGU, 1]]
};
SkillInfo[SK.NJ_KUNAI] = {
	Name: 'NJ_KUNAI',
	SkillName: 'Throw Kunai',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_SYURIKEN, 5]]
};
SkillInfo[SK.NJ_HUUMA] = {
	Name: 'NJ_HUUMA',
	SkillName: 'Throw Huuma Shuriken',
	MaxLv: 5,
	SpAmount: [15, 20, 25, 30, 35],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.NJ_TOBIDOUGU, 5],
		[SK.NJ_KUNAI, 5]
	]
};
SkillInfo[SK.NJ_ZENYNAGE] = {
	Name: 'NJ_ZENYNAGE',
	SkillName: 'Throw Coins',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.NJ_TOBIDOUGU, 10],
		[SK.NJ_HUUMA, 5]
	]
};
SkillInfo[SK.AL_ANGELUS] = {
	Name: 'AL_ANGELUS',
	SkillName: 'Angelus',
	MaxLv: 10,
	SpAmount: [23, 26, 29, 32, 35, 38, 41, 44, 47, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AL_DP, 3]]
};
SkillInfo[SK.NJ_KASUMIKIRI] = {
	Name: 'NJ_KASUMIKIRI',
	SkillName: 'Haze Slasher',
	MaxLv: 10,
	SpAmount: [8, 8, 8, 8, 8, 8, 8, 8, 8, 8],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_SHADOWJUMP, 1]]
};
SkillInfo[SK.NJ_SHADOWJUMP] = {
	Name: 'NJ_SHADOWJUMP',
	SkillName: 'Shadow Leap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [6, 8, 10, 12, 14],
	_NeedSkillList: [[SK.NJ_TATAMIGAESHI, 1]]
};
SkillInfo[SK.NJ_KIRIKAGE] = {
	Name: 'NJ_KIRIKAGE',
	SkillName: 'Shadow Slash',
	MaxLv: 5,
	SpAmount: [10, 11, 12, 13, 14],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_KASUMIKIRI, 5]]
};
SkillInfo[SK.NJ_UTSUSEMI] = {
	Name: 'NJ_UTSUSEMI',
	SkillName: 'Cicada Skin Shed',
	MaxLv: 5,
	SpAmount: [12, 15, 18, 21, 24],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_SHADOWJUMP, 5]]
};
SkillInfo[SK.NJ_BUNSINJYUTSU] = {
	Name: 'NJ_BUNSINJYUTSU',
	SkillName: 'Mirror Image',
	MaxLv: 10,
	SpAmount: [30, 32, 34, 36, 38, 40, 42, 44, 46, 48],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.NJ_NEN, 1],
		[SK.NJ_UTSUSEMI, 4],
		[SK.NJ_KIRIKAGE, 3]
	]
};
SkillInfo[SK.NJ_NINPOU] = {
	Name: 'NJ_NINPOU',
	SkillName: 'Ninja Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NJ_KOUENKA] = {
	Name: 'NJ_KOUENKA',
	SkillName: 'Flaming Petals',
	MaxLv: 10,
	SpAmount: [18, 20, 22, 24, 26, 28, 30, 32, 34, 36],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_NINPOU, 1]]
};
SkillInfo[SK.NJ_KAENSIN] = {
	Name: 'NJ_KAENSIN',
	SkillName: 'Blaze Shield',
	MaxLv: 10,
	SpAmount: [25, 25, 25, 25, 25, 25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_KOUENKA, 5]]
};
SkillInfo[SK.NJ_BAKUENRYU] = {
	Name: 'NJ_BAKUENRYU',
	SkillName: 'Exploding Dragon',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.NJ_NINPOU, 10],
		[SK.NJ_KAENSIN, 7]
	]
};
SkillInfo[SK.NJ_HYOUSENSOU] = {
	Name: 'NJ_HYOUSENSOU',
	SkillName: 'Freezing Spear',
	MaxLv: 10,
	SpAmount: [15, 18, 21, 24, 27, 30, 33, 36, 39, 42],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_NINPOU, 1]]
};
SkillInfo[SK.NJ_SUITON] = {
	Name: 'NJ_SUITON',
	SkillName: 'Watery Evasion',
	MaxLv: 10,
	SpAmount: [15, 18, 21, 24, 27, 30, 33, 36, 39, 42],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_HYOUSENSOU, 5]]
};
SkillInfo[SK.NJ_HYOUSYOURAKU] = {
	Name: 'NJ_HYOUSYOURAKU',
	SkillName: 'Snow Flake Draft',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.NJ_NINPOU, 10],
		[SK.NJ_SUITON, 7]
	]
};
SkillInfo[SK.NJ_HUUJIN] = {
	Name: 'NJ_HUUJIN',
	SkillName: 'Wind Blade',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_NINPOU, 1]]
};
SkillInfo[SK.NJ_RAIGEKISAI] = {
	Name: 'NJ_RAIGEKISAI',
	SkillName: 'Lightning Jolt',
	MaxLv: 5,
	SpAmount: [16, 20, 24, 28, 32],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.NJ_HUUJIN, 5]]
};
SkillInfo[SK.NJ_KAMAITACHI] = {
	Name: 'NJ_KAMAITACHI',
	SkillName: 'First Wind',
	MaxLv: 5,
	SpAmount: [24, 28, 32, 36, 40],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [
		[SK.NJ_NINPOU, 10],
		[SK.NJ_RAIGEKISAI, 5]
	]
};
SkillInfo[SK.AL_BLESSING] = {
	Name: 'AL_BLESSING',
	SkillName: 'Blessing',
	MaxLv: 10,
	SpAmount: [28, 32, 36, 40, 44, 48, 52, 56, 60, 64],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_DP, 5]]
};
SkillInfo[SK.NJ_ISSEN] = {
	Name: 'NJ_ISSEN',
	SkillName: 'Killing Strike',
	MaxLv: 10,
	SpAmount: [55, 60, 65, 70, 75, 80, 85, 90, 95, 100],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	_NeedSkillList: [
		[SK.NJ_TOBIDOUGU, 7],
		[SK.NJ_NEN, 1],
		[SK.NJ_KIRIKAGE, 5]
	]
};
SkillInfo[SK.MB_FIGHTING] = {
	Name: 'MB_FIGHTING',
	SkillName: 'Munak Fighting',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_NEUTRAL] = {
	Name: 'MB_NEUTRAL',
	SkillName: 'Bongun Neutral',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_TAIMING_PUTI] = {
	Name: 'MB_TAIMING_PUTI',
	SkillName: 'Puti Taming',
	MaxLv: 7,
	SpAmount: [0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_WHITEPOTION] = {
	Name: 'MB_WHITEPOTION',
	SkillName: 'White Potion',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MB_MENTAL] = {
	Name: 'MB_MENTAL',
	SkillName: 'Mental Errands',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MB_CARDPITCHER] = {
	Name: 'MB_CARDPITCHER',
	SkillName: 'Card Pitcher',
	MaxLv: 10,
	SpAmount: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_PETPITCHER] = {
	Name: 'MB_PETPITCHER',
	SkillName: 'Kick the Baby',
	MaxLv: 10,
	SpAmount: [10, 9, 8, 7, 6, 5, 4, 3, 2, 1],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_BODYSTUDY] = {
	Name: 'MB_BODYSTUDY',
	SkillName: 'Body Study',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_BODYALTER] = {
	Name: 'MB_BODYALTER',
	SkillName: 'Alter Body',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MB_PETMEMORY] = {
	Name: 'MB_PETMEMORY',
	SkillName: 'Pet Memory',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MB_M_TELEPORT] = {
	Name: 'MB_M_TELEPORT',
	SkillName: 'Pet Teleport',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_B_GAIN] = {
	Name: 'MB_B_GAIN',
	SkillName: 'Bongun Gain',
	MaxLv: 7,
	SpAmount: [12, 15, 18, 21, 24, 27, 30],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_M_GAIN] = {
	Name: 'MB_M_GAIN',
	SkillName: 'Munak Gain',
	MaxLv: 7,
	SpAmount: [1, 1, 1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_MISSION] = {
	Name: 'MB_MISSION',
	SkillName: 'Mission Timing',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AL_CURE] = {
	Name: 'AL_CURE',
	SkillName: 'Cure',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.AL_HEAL, 2]],
	NeedSkillList: { [JobId.CRUSADER]: [[SK.CR_TRUST, 5]] }
};
SkillInfo[SK.MB_MUNAKBALL] = {
	Name: 'MB_MUNAKBALL',
	SkillName: 'Munak Ball',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_SCROLL] = {
	Name: 'MB_SCROLL',
	SkillName: 'Pet Scroll',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_B_GATHERING] = {
	Name: 'MB_B_GATHERING',
	SkillName: 'Bongun Gathering',
	MaxLv: 7,
	SpAmount: [17, 15, 13, 11, 9, 7, 5],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_M_GATHERING] = {
	Name: 'MB_M_GATHERING',
	SkillName: 'Munak Gathering',
	MaxLv: 7,
	SpAmount: [32, 30, 28, 26, 24, 22, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_B_EXCLUDE] = {
	Name: 'MB_B_EXCLUDE',
	SkillName: 'Bongun Exclude',
	MaxLv: 5,
	SpAmount: [180, 160, 140, 120, 100],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MB_B_DRIFT] = {
	Name: 'MB_B_DRIFT',
	SkillName: 'Bongun Drift',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_B_WALLRUSH] = {
	Name: 'MB_B_WALLRUSH',
	SkillName: 'Bongun Wall Rush',
	MaxLv: 7,
	SpAmount: [9, 10, 11, 12, 13, 14, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_M_WALLRUSH] = {
	Name: 'MB_M_WALLRUSH',
	SkillName: 'Munak Wall Rush',
	MaxLv: 7,
	SpAmount: [9, 10, 11, 12, 13, 14, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_B_WALLSHIFT] = {
	Name: 'MB_B_WALLSHIFT',
	SkillName: 'Bongun Wallshift',
	MaxLv: 5,
	SpAmount: [13, 11, 9, 7, 5],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_M_WALLCRASH] = {
	Name: 'MB_M_WALLCRASH',
	SkillName: 'Munak Wall Crash',
	MaxLv: 7,
	SpAmount: [27, 25, 23, 21, 19, 17, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_M_REINCARNATION] = {
	Name: 'MB_M_REINCARNATION',
	SkillName: 'Munak Reincarnation',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MB_B_EQUIP] = {
	Name: 'MB_B_EQUIP',
	SkillName: 'Bongun Equip',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SL_DEATHKNIGHT] = {
	Name: 'SL_DEATHKNIGHT',
	SkillName: 'Deathknight Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SL_COLLECTOR] = {
	Name: 'SL_COLLECTOR',
	SkillName: "Soul Collector's Spirit",
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SL_NINJA] = {
	Name: 'SL_NINJA',
	SkillName: 'Ninja Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MC_INCCARRY] = {
	Name: 'MC_INCCARRY',
	SkillName: 'Enlarge Weight Limit',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AM_TWILIGHT4] = {
	Name: 'AM_TWILIGHT4',
	SkillName: 'Spiritual Potion Creation 4',
	MaxLv: 1,
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_BERSERKAIZER] = {
	Name: 'DE_BERSERKAIZER',
	SkillName: 'Berserk Kaizer',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_DARKPOWER] = {
	Name: 'DA_DARKPOWER',
	SkillName: 'Dark Power',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DE_PASSIVE] = {
	Name: 'DE_PASSIVE',
	SkillName: 'Death Passive',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_PATTACK] = {
	Name: 'DE_PATTACK',
	SkillName: 'Death Attack',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PSPEED] = {
	Name: 'DE_PSPEED',
	SkillName: 'Death Speed',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PDEFENSE] = {
	Name: 'DE_PDEFENSE',
	SkillName: 'Death Defense',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PCRITICAL] = {
	Name: 'DE_PCRITICAL',
	SkillName: 'Death Critical',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PHP] = {
	Name: 'DE_PHP',
	SkillName: 'Death HP',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PSP] = {
	Name: 'DE_PSP',
	SkillName: 'Death SP',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_RESET] = {
	Name: 'DE_RESET',
	SkillName: 'Death Reset',
	MaxLv: 1,
	SpAmount: [280],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_RANKING] = {
	Name: 'DE_RANKING',
	SkillName: 'Ranking',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_PTRIPLE] = {
	Name: 'DE_PTRIPLE',
	SkillName: 'Death Triple',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_ENERGY] = {
	Name: 'DE_ENERGY',
	SkillName: 'Energy',
	MaxLv: 5,
	SpAmount: [1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MC_DISCOUNT] = {
	Name: 'MC_DISCOUNT',
	SkillName: 'Discount',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MC_INCCARRY, 3]]
};
SkillInfo[SK.DE_SLASH] = {
	Name: 'DE_SLASH',
	SkillName: 'Slash',
	MaxLv: 5,
	SpAmount: [10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_COIL] = {
	Name: 'DE_COIL',
	SkillName: 'Coil',
	MaxLv: 7,
	SpAmount: [8, 10, 12, 14, 16, 18, 20],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.DE_WAVE] = {
	Name: 'DE_WAVE',
	SkillName: 'Wave',
	MaxLv: 7,
	SpAmount: [55, 50, 45, 40, 35, 30, 25],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_REBIRTH] = {
	Name: 'DE_REBIRTH',
	SkillName: 'Rebirth',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.DE_AURA] = {
	Name: 'DE_AURA',
	SkillName: 'Aura',
	MaxLv: 7,
	SpAmount: [80, 75, 70, 65, 60, 55, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_FREEZER] = {
	Name: 'DE_FREEZER',
	SkillName: 'Freezer',
	MaxLv: 7,
	SpAmount: [20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.DE_CHANGEATTACK] = {
	Name: 'DE_CHANGEATTACK',
	SkillName: 'Change Attack',
	MaxLv: 7,
	SpAmount: [80, 70, 60, 50, 40, 30, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_PUNISH] = {
	Name: 'DE_PUNISH',
	SkillName: 'Death Punish',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_POISON] = {
	Name: 'DE_POISON',
	SkillName: 'Death Poison',
	MaxLv: 7,
	SpAmount: [14, 12, 10, 8, 6, 4, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_INSTANT] = {
	Name: 'DE_INSTANT',
	SkillName: 'Instant',
	MaxLv: 7,
	SpAmount: [50, 100, 150, 200, 250, 300, 350],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_WARNING] = {
	Name: 'DE_WARNING',
	SkillName: 'Warning',
	MaxLv: 7,
	SpAmount: [50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.DE_RANKEDKNIFE] = {
	Name: 'DE_RANKEDKNIFE',
	SkillName: 'Ranked Knife',
	MaxLv: 7,
	SpAmount: [20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.DE_RANKEDGRADIUS] = {
	Name: 'DE_RANKEDGRADIUS',
	SkillName: 'Death Gradisu',
	MaxLv: 7,
	SpAmount: [20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_GAUGE] = {
	Name: 'DE_GAUGE',
	SkillName: 'Gauge',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DE_GTIME] = {
	Name: 'DE_GTIME',
	SkillName: 'G Time',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MC_OVERCHARGE] = {
	Name: 'MC_OVERCHARGE',
	SkillName: 'Overcharge',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MC_DISCOUNT, 3]]
};
SkillInfo[SK.DE_GSKILL] = {
	Name: 'DE_GSKILL',
	SkillName: 'G Skill',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_GKILL] = {
	Name: 'DE_GKILL',
	SkillName: 'G Kill',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_ACCEL] = {
	Name: 'DE_ACCEL',
	SkillName: 'Acceleration',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_BLOCKDOUBLE] = {
	Name: 'DE_BLOCKDOUBLE',
	SkillName: 'Double Block',
	MaxLv: 3,
	SpAmount: [40, 30, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.DE_BLOCKMELEE] = {
	Name: 'DE_BLOCKMELEE',
	SkillName: 'Melee Block',
	MaxLv: 3,
	SpAmount: [40, 30, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.DE_BLOCKFAR] = {
	Name: 'DE_BLOCKFAR',
	SkillName: 'Far Black',
	MaxLv: 3,
	SpAmount: [100, 75, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.DE_FRONTATTACK] = {
	Name: 'DE_FRONTATTACK',
	SkillName: 'Front Attack',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_DANGERATTACK] = {
	Name: 'DE_DANGERATTACK',
	SkillName: 'Dangerous Attack',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_TWINATTACK] = {
	Name: 'DE_TWINATTACK',
	SkillName: 'Twin Attack',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_WINDATTACK] = {
	Name: 'DE_WINDATTACK',
	SkillName: 'Wind Attack',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DE_WATERATTACK] = {
	Name: 'DE_WATERATTACK',
	SkillName: 'Water Attack',
	MaxLv: 10,
	SpAmount: [40, 40, 40, 40, 40, 40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_ENERGY] = {
	Name: 'DA_ENERGY',
	SkillName: 'Energy',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_CLOUD] = {
	Name: 'DA_CLOUD',
	SkillName: 'Cloud',
	MaxLv: 10,
	SpAmount: [40, 40, 40, 40, 40, 40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_FIRSTSLOT] = {
	Name: 'DA_FIRSTSLOT',
	SkillName: 'First Slot',
	MaxLv: 5,
	SpAmount: [100, 90, 80, 70, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_HEADDEF] = {
	Name: 'DA_HEADDEF',
	SkillName: 'Head Defense',
	MaxLv: 4,
	SpAmount: [60, 60, 60, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9]
};
SkillInfo[SK.MC_PUSHCART] = {
	Name: 'MC_PUSHCART',
	SkillName: 'Pushcart',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MC_INCCARRY, 5]]
};
SkillInfo[SK.DA_TRANSFORM] = {
	Name: 'DA_TRANSFORM',
	SkillName: 'Transform',
	MaxLv: 5,
	SpAmount: [180, 150, 120, 90, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_EXPLOSION] = {
	Name: 'DA_EXPLOSION',
	SkillName: 'Explosion',
	MaxLv: 5,
	SpAmount: [140, 120, 100, 80, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_REWARD] = {
	Name: 'DA_REWARD',
	SkillName: 'Reward',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_CRUSH] = {
	Name: 'DA_CRUSH',
	SkillName: 'Crush',
	MaxLv: 5,
	SpAmount: [130, 110, 90, 70, 50],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_ITEMREBUILD] = {
	Name: 'DA_ITEMREBUILD',
	SkillName: 'Item Rebuild',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_ILLUSION] = {
	Name: 'DA_ILLUSION',
	SkillName: 'Illusion',
	MaxLv: 5,
	SpAmount: [120, 100, 80, 60, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_NUETRALIZE] = {
	Name: 'DA_NUETRALIZE',
	SkillName: 'Neutralize',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_RUNNER] = {
	Name: 'DA_RUNNER',
	SkillName: 'Runner',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.DA_TRANSFER] = {
	Name: 'DA_TRANSFER',
	SkillName: 'Transfer',
	MaxLv: 5,
	SpAmount: [70, 60, 50, 40, 30],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.DA_WALL] = {
	Name: 'DA_WALL',
	SkillName: 'Wall',
	MaxLv: 5,
	SpAmount: [10, 20, 30, 40, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.RETURN_TO_ELDICASTES] = {
	Name: 'RETURN_TO_ELDICASTES',
	SkillName: 'To El Dicastes',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_REVENGE] = {
	Name: 'DA_REVENGE',
	SkillName: 'Revenge',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_EARPLUG] = {
	Name: 'DA_EARPLUG',
	SkillName: 'Earplug',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_CONTRACT] = {
	Name: 'DA_CONTRACT',
	SkillName: 'Contract',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_BLACK] = {
	Name: 'DA_BLACK',
	SkillName: 'Black',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MC_IDENTIFY] = {
	Name: 'MC_IDENTIFY',
	SkillName: 'Item Appraisal',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_MAGICCART] = {
	Name: 'DA_MAGICCART',
	SkillName: 'Magic Cart',
	MaxLv: 5,
	SpAmount: [50, 40, 30, 20, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_COPY] = {
	Name: 'DA_COPY',
	SkillName: 'Copy',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_CRYSTAL] = {
	Name: 'DA_CRYSTAL',
	SkillName: 'Crystal',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_EXP] = {
	Name: 'DA_EXP',
	SkillName: 'Experience',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_CARTSWING] = {
	Name: 'DA_CARTSWING',
	SkillName: 'Cart Swing',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_REBUILD] = {
	Name: 'DA_REBUILD',
	SkillName: 'Rebuild',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DA_JOBCHANGE] = {
	Name: 'DA_JOBCHANGE',
	SkillName: 'Job Change',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_EDARKNESS] = {
	Name: 'DA_EDARKNESS',
	SkillName: 'Eternal Darkness',
	MaxLv: 5,
	SpAmount: [1100, 900, 700, 500, 300],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.DA_EGUARDIAN] = {
	Name: 'DA_EGUARDIAN',
	SkillName: 'Guardian',
	MaxLv: 5,
	SpAmount: [1300, 1100, 900, 700, 500],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_TIMEOUT] = {
	Name: 'DA_TIMEOUT',
	SkillName: 'Time Out',
	MaxLv: 3,
	SpAmount: [500, 300, 100],
	bSeperateLv: false,
	AttackRange: [9, 9, 9]
};
SkillInfo[SK.ALL_TIMEIN] = {
	Name: 'ALL_TIMEIN',
	SkillName: 'Time',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_ZENYRANK] = {
	Name: 'DA_ZENYRANK',
	SkillName: 'Zeny Rank',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DA_ACCESSORYMIX] = {
	Name: 'DA_ACCESSORYMIX',
	SkillName: 'Accessory mix',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.NPC_EARTHQUAKE] = {
	Name: 'NPC_EARTHQUAKE',
	SkillName: 'Earthquake',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[15, 15],
		[19, 19],
		[23, 23],
		[27, 27],
		[11, 11],
		[15, 15],
		[19, 19],
		[23, 23],
		[27, 27]
	]
};
SkillInfo[SK.NPC_EARTHQUAKE_K] = {
	Name: 'NPC_EARTHQUAKE_K',
	SkillName: 'Earthquake',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[15, 15],
		[19, 19],
		[23, 23],
		[27, 27],
		[11, 11],
		[15, 15],
		[19, 19],
		[23, 23],
		[27, 27]
	]
};
SkillInfo[SK.EL_CIRCLE_OF_FIRE] = {
	Name: 'EL_CIRCLE_OF_FIRE',
	SkillName: 'Circle of Fire',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MC_VENDING] = {
	Name: 'MC_VENDING',
	SkillName: 'Vending',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MC_PUSHCART, 3]]
};
SkillInfo[SK.EL_TIDAL_WEAPON] = {
	Name: 'EL_TIDAL_WEAPON',
	SkillName: 'Tidal Weapon',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.NPC_DRAGONFEAR] = {
	Name: 'NPC_DRAGONFEAR',
	SkillName: 'Dragon Fear',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [6, 6, 6, 6, 6],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_PULSESTRIKE2] = {
	Name: 'NPC_PULSESTRIKE2',
	SkillName: 'Pulse Strike',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	SkillScale: [[11, 11]]
};
SkillInfo[SK.NPC_PULSESTRIKE] = {
	Name: 'NPC_PULSESTRIKE',
	SkillName: 'Pulse Strike',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[15, 15],
		[15, 15],
		[15, 15],
		[15, 15],
		[15, 15]
	]
};
SkillInfo[SK.NPC_HELLJUDGEMENT] = {
	Name: 'NPC_HELLJUDGEMENT',
	SkillName: "Hell's Judgement",
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESILENCE] = {
	Name: 'NPC_WIDESILENCE',
	SkillName: 'Bedlam',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDEFREEZE] = {
	Name: 'NPC_WIDEFREEZE',
	SkillName: 'Frozen Heart',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDEBLEEDING] = {
	Name: 'NPC_WIDEBLEEDING',
	SkillName: 'Bloody Party',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESTONE] = {
	Name: 'NPC_WIDESTONE',
	SkillName: "Medusa's Stare",
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDECONFUSE] = {
	Name: 'NPC_WIDECONFUSE',
	SkillName: 'Confusion Rule',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESLEEP] = {
	Name: 'NPC_WIDESLEEP',
	SkillName: 'Morpheus Slumber',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_EVILLAND] = {
	Name: 'NPC_EVILLAND',
	SkillName: 'Evil Land',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	SkillScale: [
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[29, 29]
	]
};
SkillInfo[SK.MC_MAMMONITE] = {
	Name: 'MC_MAMMONITE',
	SkillName: 'Mammonite',
	MaxLv: 10,
	SpAmount: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_SLOWCAST] = {
	Name: 'NPC_SLOWCAST',
	SkillName: 'Slow Cast',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_CRITICALWOUND] = {
	Name: 'NPC_CRITICALWOUND',
	SkillName: 'Critical Wounds',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7]
};
SkillInfo[SK.NPC_STONESKIN] = {
	Name: 'NPC_STONESKIN',
	SkillName: 'Stone Skin',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_ANTIMAGIC] = {
	Name: 'NPC_ANTIMAGIC',
	SkillName: 'Deadzone',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_WIDECURSE] = {
	Name: 'NPC_WIDECURSE',
	SkillName: 'Cursed Fate',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESTUN] = {
	Name: 'NPC_WIDESTUN',
	SkillName: 'Stunning Gaze',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_VAMPIRE_GIFT] = {
	Name: 'NPC_VAMPIRE_GIFT',
	SkillName: "Vampire's Gift",
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_WIDESOULDRAIN] = {
	Name: 'NPC_WIDESOULDRAIN',
	SkillName: 'Souless Defeat',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ALL_INCCARRY] = {
	Name: 'ALL_INCCARRY',
	SkillName: 'Increase Capacity',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_HELLPOWER] = {
	Name: 'NPC_HELLPOWER',
	SkillName: "Hell's Power",
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.AC_OWL] = {
	Name: 'AC_OWL',
	SkillName: "Owl's Eye",
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GM_SANDMAN] = {
	Name: 'GM_SANDMAN',
	SkillName: 'Goodnight, Sweety',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.ALL_CATCRY] = {
	Name: 'ALL_CATCRY',
	SkillName: "Monster's Cry",
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ALL_PARTYFLEE] = {
	Name: 'ALL_PARTYFLEE',
	SkillName: 'Blowing Wind !!',
	MaxLv: 10,
	SpAmount: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ALL_ANGEL_PROTECT] = {
	Name: 'ALL_ANGEL_PROTECT',
	SkillName: 'Thank You So Much!!',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.ALL_DREAM_SUMMERNIGHT] = {
	Name: 'ALL_DREAM_SUMMERNIGHT',
	SkillName: 'Summer Dream',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ALL_REVERSEORCISH] = {
	Name: 'ALL_REVERSEORCISH',
	SkillName: 'Reverse Orcish',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ALL_WEWISH] = {
	Name: 'ALL_WEWISH',
	SkillName: "Sing along with the Singing Crystal's tune:",
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AC_VULTURE] = {
	Name: 'AC_VULTURE',
	SkillName: "Vulture's Eye",
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AC_OWL, 3]],
	NeedSkillList: { [JobId.ROGUE]: [] }
};
SkillInfo[SK.AC_CONCENTRATION] = {
	Name: 'AC_CONCENTRATION',
	SkillName: 'Improve Concentration',
	MaxLv: 10,
	SpAmount: [25, 30, 35, 40, 45, 50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AC_VULTURE, 1]]
};
SkillInfo[SK.AC_DOUBLE] = {
	Name: 'AC_DOUBLE',
	SkillName: 'Double Strafe',
	MaxLv: 10,
	SpAmount: [12, 12, 12, 12, 12, 12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	NeedSkillList: { [JobId.ROGUE]: [[SK.AC_VULTURE, 10]] }
};
SkillInfo[SK.HLIF_HEAL] = {
	Name: 'HLIF_HEAL',
	SkillName: 'Healing Hands',
	MaxLv: 5,
	SpAmount: [13, 16, 19, 22, 25],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HFLI_MOON] = {
	Name: 'HFLI_MOON',
	SkillName: 'Moonlight',
	MaxLv: 5,
	SpAmount: [4, 8, 12, 16, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_XENO_SLASHER] = {
	Name: 'MH_XENO_SLASHER',
	SkillName: 'Xeno Slasher',
	MaxLv: 10,
	SpAmount: [85, 90, 95, 100, 105, 110, 115, 120, 125, 130],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.MH_STEINWAND] = {
	Name: 'MH_STEINWAND',
	SkillName: 'Stein Wand',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_LAVA_SLIDE] = {
	Name: 'MH_LAVA_SLIDE',
	SkillName: 'Lava Slide',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.AC_SHOWER] = {
	Name: 'AC_SHOWER',
	SkillName: 'Arrow Shower',
	MaxLv: 10,
	SpAmount: [15, 15, 15, 15, 15, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AC_DOUBLE, 5]]
};
SkillInfo[SK.GD_KAFRACONTRACT] = {
	Name: 'GD_KAFRACONTRACT',
	SkillName: 'Contract With Kafra',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SM_TWOHAND] = {
	Name: 'SM_TWOHAND',
	SkillName: 'Two Handed Sword Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SM_SWORD, 1]]
};
SkillInfo[SK.TF_DOUBLE] = {
	Name: 'TF_DOUBLE',
	SkillName: 'Double Attack',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MA_LANDMINE] = {
	Name: 'MA_LANDMINE',
	SkillName: 'Land Mine',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.MER_REGAIN] = {
	Name: 'MER_REGAIN',
	SkillName: 'Regain',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.EL_FIRE_CLOAK] = {
	Name: 'EL_FIRE_CLOAK',
	SkillName: 'Fire Cloak',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.TF_MISS] = {
	Name: 'TF_MISS',
	SkillName: 'Improve Dodge',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_WIND_SLASH] = {
	Name: 'EL_WIND_SLASH',
	SkillName: 'Wind Slash',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [11]
};
SkillInfo[SK.TF_STEAL] = {
	Name: 'TF_STEAL',
	SkillName: 'Steal',
	MaxLv: 10,
	SpAmount: [10, 10, 10, 10, 10, 10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.TF_HIDING] = {
	Name: 'TF_HIDING',
	SkillName: 'Hiding',
	MaxLv: 10,
	SpAmount: [10, 10, 10, 10, 10, 10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TF_STEAL, 5]]
};
SkillInfo[SK.TF_POISON] = {
	Name: 'TF_POISON',
	SkillName: 'Envenom',
	MaxLv: 10,
	SpAmount: [12, 12, 12, 12, 12, 12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
};
SkillInfo[SK.TF_DETOXIFY] = {
	Name: 'TF_DETOXIFY',
	SkillName: 'Detoxify',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.TF_POISON, 3]]
};
SkillInfo[SK.ALL_RESURRECTION] = {
	Name: 'ALL_RESURRECTION',
	SkillName: 'Resurrection',
	MaxLv: 4,
	SpAmount: [60, 60, 60, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_SRECOVERY, 4],
		[SK.PR_STRECOVERY, 1]
	]
};
SkillInfo[SK.KN_SPEARMASTERY] = {
	Name: 'KN_SPEARMASTERY',
	SkillName: 'Spear Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_GUARDRESEARCH] = {
	Name: 'GD_GUARDRESEARCH',
	SkillName: 'Guardian Research',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KN_PIERCE] = {
	Name: 'KN_PIERCE',
	SkillName: 'Pierce',
	MaxLv: 10,
	SpAmount: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.KN_SPEARMASTERY, 1]]
};
SkillInfo[SK.MA_SANDMAN] = {
	Name: 'MA_SANDMAN',
	SkillName: 'Sandman',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.MER_TENDER] = {
	Name: 'MER_TENDER',
	SkillName: 'Tender',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.EL_FIRE_MANTLE] = {
	Name: 'EL_FIRE_MANTLE',
	SkillName: 'Fire Mantle',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KN_BRANDISHSPEAR] = {
	Name: 'KN_BRANDISHSPEAR',
	SkillName: 'Brandish Spear',
	MaxLv: 10,
	SpAmount: [24, 24, 24, 24, 24, 24, 24, 24, 24, 24],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.KN_RIDING, 1],
		[SK.KN_SPEARSTAB, 3]
	]
};
SkillInfo[SK.EL_HURRICANE] = {
	Name: 'EL_HURRICANE',
	SkillName: 'Hurricane',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [11]
};
SkillInfo[SK.KN_SPEARSTAB] = {
	Name: 'KN_SPEARSTAB',
	SkillName: 'Spear Stab',
	MaxLv: 10,
	SpAmount: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
	_NeedSkillList: [[SK.KN_PIERCE, 5]]
};
SkillInfo[SK.KN_SPEARBOOMERANG] = {
	Name: 'KN_SPEARBOOMERANG',
	SkillName: 'Spear Boomerang',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 5, 7, 9, 11],
	_NeedSkillList: [[SK.KN_PIERCE, 3]]
};
SkillInfo[SK.KN_TWOHANDQUICKEN] = {
	Name: 'KN_TWOHANDQUICKEN',
	SkillName: 'Two Hand Quicken',
	MaxLv: 10,
	SpAmount: [14, 18, 22, 26, 30, 34, 38, 42, 46, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SM_TWOHAND, 1]]
};
SkillInfo[SK.KN_AUTOCOUNTER] = {
	Name: 'KN_AUTOCOUNTER',
	SkillName: 'Counter Attack',
	MaxLv: 5,
	SpAmount: [3, 3, 3, 3, 3],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SM_TWOHAND, 1]]
};
SkillInfo[SK.KN_BOWLINGBASH] = {
	Name: 'KN_BOWLINGBASH',
	SkillName: 'Bowling Bash',
	MaxLv: 10,
	SpAmount: [13, 14, 15, 16, 17, 18, 19, 20, 21, 22],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SM_BASH, 10],
		[SK.SM_MAGNUM, 3],
		[SK.SM_TWOHAND, 5],
		[SK.KN_TWOHANDQUICKEN, 10],
		[SK.KN_AUTOCOUNTER, 5]
	],
	NeedSkillList: { [JobId.SUPERNOVICE2]: [[SK.KN_AUTOCOUNTER, 5]] }
};
SkillInfo[SK.KN_CHARGEATK] = {
	Name: 'KN_CHARGEATK',
	SkillName: 'Charge Attack',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [14]
};
SkillInfo[SK.CR_SHRINK] = {
	Name: 'CR_SHRINK',
	SkillName: 'Shrink',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AS_SONICACCEL] = {
	Name: 'AS_SONICACCEL',
	SkillName: 'Sonic Acceleration',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AS_VENOMKNIFE] = {
	Name: 'AS_VENOMKNIFE',
	SkillName: 'Venom Knife',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [35],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.RG_CLOSECONFINE] = {
	Name: 'RG_CLOSECONFINE',
	SkillName: 'Close Confine',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [2]
};
SkillInfo[SK.WZ_SIGHTBLASTER] = {
	Name: 'WZ_SIGHTBLASTER',
	SkillName: 'Sight Blaster',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KN_RIDING] = {
	Name: 'KN_RIDING',
	SkillName: 'Peco Peco Ride',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SM_ENDURE, 1]]
};
SkillInfo[SK.SA_ELEMENTWATER] = {
	Name: 'SA_ELEMENTWATER',
	SkillName: 'Elemental Change - Water',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.HT_PHANTASMIC] = {
	Name: 'HT_PHANTASMIC',
	SkillName: 'Phantasmic Arrow',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.BA_PANGVOICE] = {
	Name: 'BA_PANGVOICE',
	SkillName: 'Pang Voice',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.DC_WINKCHARM] = {
	Name: 'DC_WINKCHARM',
	SkillName: 'Charming Wink',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.BS_UNFAIRLYTRICK] = {
	Name: 'BS_UNFAIRLYTRICK',
	SkillName: 'Dubious Salesmanship',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.BS_GREED] = {
	Name: 'BS_GREED',
	SkillName: 'Greed',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.PR_REDEMPTIO] = {
	Name: 'PR_REDEMPTIO',
	SkillName: 'Redemptio',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [800],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MO_KITRANSLATION] = {
	Name: 'MO_KITRANSLATION',
	SkillName: 'Spiritual Bestowment',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.MO_BALKYOUNG] = {
	Name: 'MO_BALKYOUNG',
	SkillName: 'Excruciating Palm',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SA_ELEMENTGROUND] = {
	Name: 'SA_ELEMENTGROUND',
	SkillName: 'Elemental Change - Ground',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.SA_ELEMENTFIRE] = {
	Name: 'SA_ELEMENTFIRE',
	SkillName: 'Elemental Change - Fire',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.SA_ELEMENTWIND] = {
	Name: 'SA_ELEMENTWIND',
	SkillName: 'Elemental Change - Wind',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.SM_RECOVERY] = {
	Name: 'SM_RECOVERY',
	SkillName: 'Increase HP Recovery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.KN_CAVALIERMASTERY] = {
	Name: 'KN_CAVALIERMASTERY',
	SkillName: 'Cavalier Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KN_RIDING, 1]]
};
SkillInfo[SK.AB_HIGHNESSHEAL] = {
	Name: 'AB_HIGHNESSHEAL',
	SkillName: 'High Heal',
	MaxLv: 5,
	SpAmount: [70, 100, 130, 160, 190],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.AB_RENOVATIO, 1]]
};
SkillInfo[SK.AB_DUPLELIGHT_MELEE] = {
	Name: 'AB_DUPLELIGHT_MELEE',
	SkillName: 'Duple Strike',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11]
};
SkillInfo[SK.MER_BENEDICTION] = {
	Name: 'MER_BENEDICTION',
	SkillName: 'Benediction',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.PR_MACEMASTERY] = {
	Name: 'PR_MACEMASTERY',
	SkillName: 'Mace Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_WATER_SCREEN] = {
	Name: 'EL_WATER_SCREEN',
	SkillName: 'Water Screen',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.PR_IMPOSITIO] = {
	Name: 'PR_IMPOSITIO',
	SkillName: 'Impositio Manus',
	MaxLv: 5,
	SpAmount: [59, 62, 65, 68, 71],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_HURRICANE_ATK] = {
	Name: 'EL_HURRICANE_ATK',
	SkillName: 'Hurricange Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.PR_SUFFRAGIUM] = {
	Name: 'PR_SUFFRAGIUM',
	SkillName: 'Suffragium',
	MaxLv: 3,
	SpAmount: [8, 8, 8],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.PR_IMPOSITIO, 2]]
};
SkillInfo[SK.PR_ASPERSIO] = {
	Name: 'PR_ASPERSIO',
	SkillName: 'Aspersio',
	MaxLv: 5,
	SpAmount: [14, 18, 22, 26, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AL_HOLYWATER, 1],
		[SK.PR_IMPOSITIO, 3]
	]
};
SkillInfo[SK.PR_BENEDICTIO] = {
	Name: 'PR_BENEDICTIO',
	SkillName: 'B.S Sacramenti ',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.PR_ASPERSIO, 5],
		[SK.PR_GLORIA, 3]
	]
};
SkillInfo[SK.WL_SIENNAEXECRATE] = {
	Name: 'WL_SIENNAEXECRATE',
	SkillName: 'Sienna Execrate',
	MaxLv: 5,
	SpAmount: [32, 34, 36, 38, 40],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.WL_SUMMONSTONE, 1]]
};
SkillInfo[SK.WL_CRIMSONROCK] = {
	Name: 'WL_CRIMSONROCK',
	SkillName: 'Crimson Rock',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_SUMMONFB, 1]]
};
SkillInfo[SK.WL_SUMMONBL] = {
	Name: 'WL_SUMMONBL',
	SkillName: 'Summon Lightning Ball',
	MaxLv: 2,
	SpAmount: [10, 50],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.WZ_VERMILION, 1]]
};
SkillInfo[SK.WL_READING_SB] = {
	Name: 'WL_READING_SB',
	SkillName: 'Reading Spell Book',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WL_READING_SB_READING] = {
	Name: 'WL_READING_SB_READING',
	SkillName: 'Reading Spell Book',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.PR_SANCTUARY] = {
	Name: 'PR_SANCTUARY',
	SkillName: 'Sanctuary',
	MaxLv: 10,
	SpAmount: [15, 18, 21, 24, 27, 30, 33, 36, 39, 42],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_HEAL, 1]]
};
SkillInfo[SK.RA_CLUSTERBOMB] = {
	Name: 'RA_CLUSTERBOMB',
	SkillName: 'Bomb Cluster',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 3]]
};
SkillInfo[SK.RA_WUGSTRIKE] = {
	Name: 'RA_WUGSTRIKE',
	SkillName: 'Warg Strike',
	MaxLv: 5,
	SpAmount: [20, 22, 24, 26, 28],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RA_TOOTHOFWUG, 1]]
};
SkillInfo[SK.RA_CAMOUFLAGE] = {
	Name: 'RA_CAMOUFLAGE',
	SkillName: 'Camouflage',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RA_RANGERMAIN, 1]]
};
SkillInfo[SK.RA_MAIZETRAP] = {
	Name: 'RA_MAIZETRAP',
	SkillName: 'Maze Trap',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 1]]
};
SkillInfo[SK.NC_MADOLICENCE] = {
	Name: 'NC_MADOLICENCE',
	SkillName: 'Madogear License',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.NC_FLAMELAUNCHER] = {
	Name: 'NC_FLAMELAUNCHER',
	SkillName: 'Flame Launcher',
	MaxLv: 3,
	SpAmount: [20, 20, 20],
	bSeperateLv: true,
	AttackRange: [5, 5, 5],
	_NeedSkillList: [[SK.NC_VULCANARM, 1]]
};
SkillInfo[SK.NC_HOVERING] = {
	Name: 'NC_HOVERING',
	SkillName: 'Hover',
	MaxLv: 1,
	SpAmount: [25],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NC_ACCELERATION, 1]]
};
SkillInfo[SK.PR_SLOWPOISON] = {
	Name: 'PR_SLOWPOISON',
	SkillName: 'Slow Poison',
	MaxLv: 4,
	SpAmount: [6, 8, 10, 12],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9]
};
SkillInfo[SK.NC_ANALYZE] = {
	Name: 'NC_ANALYZE',
	SkillName: 'Analyze',
	MaxLv: 3,
	SpAmount: [30, 30, 30],
	bSeperateLv: false,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.NC_INFRAREDSCAN, 1]]
};
SkillInfo[SK.NC_REPAIR] = {
	Name: 'NC_REPAIR',
	SkillName: 'Repair',
	MaxLv: 5,
	SpAmount: [25, 30, 35, 40, 45],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [[SK.NC_MADOLICENCE, 2]]
};
SkillInfo[SK.NC_POWERSWING] = {
	Name: 'NC_POWERSWING',
	SkillName: 'Power Swing',
	MaxLv: 10,
	SpAmount: [20, 22, 24, 26, 28, 30, 32, 34, 36, 38],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.NC_AXEBOOMERANG, 3]]
};
SkillInfo[SK.NC_DISJOINT] = {
	Name: 'NC_DISJOINT',
	SkillName: 'Divest FAW ',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [5],
	_NeedSkillList: [[SK.NC_SILVERSNIPER, 1]]
};
SkillInfo[SK.SC_SHADOWFORM] = {
	Name: 'SC_SHADOWFORM',
	SkillName: 'Shadow Formation',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.RG_TUNNELDRIVE, 3]]
};
SkillInfo[SK.SC_DEADLYINFECT] = {
	Name: 'SC_DEADLYINFECT',
	SkillName: 'Deadly Infection',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SC_SHADOWFORM, 3],
		[SK.SC_AUTOSHADOWSPELL, 5]
	]
};
SkillInfo[SK.SC_LAZINESS] = {
	Name: 'SC_LAZINESS',
	SkillName: 'Masquerade-Laziness',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [
		[SK.SC_ENERVATION, 1],
		[SK.SC_GROOMY, 1],
		[SK.SC_IGNORANCE, 1]
	]
};
SkillInfo[SK.PR_STRECOVERY] = {
	Name: 'PR_STRECOVERY',
	SkillName: 'Status Recovery',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.SC_BLOODYLUST] = {
	Name: 'SC_BLOODYLUST',
	SkillName: 'Bloody Lust ',
	MaxLv: 3,
	SpAmount: [60, 70, 80],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [[SK.SC_DIMENSIONDOOR, 3]]
};
SkillInfo[SK.LG_CANNONSPEAR] = {
	Name: 'LG_CANNONSPEAR',
	SkillName: 'Cannon Spear',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.LG_PINPOINTATTACK, 1]]
};
SkillInfo[SK.LG_REFLECTDAMAGE] = {
	Name: 'LG_REFLECTDAMAGE',
	SkillName: 'Reflect Damage',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_REFLECTSHIELD, 5]]
};
SkillInfo[SK.LG_SHIELDSPELL] = {
	Name: 'LG_SHIELDSPELL',
	SkillName: 'Shield Spell',
	MaxLv: 3,
	SpAmount: [50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.LG_SHIELDPRESS, 3],
		[SK.LG_EARTHDRIVE, 2]
	]
};
SkillInfo[SK.LG_BANDING] = {
	Name: 'LG_BANDING',
	SkillName: 'Banding',
	MaxLv: 5,
	SpAmount: [30, 36, 42, 48, 54],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.LG_PINPOINTATTACK, 3],
		[SK.LG_RAGEBURST, 1]
	]
};
SkillInfo[SK.LG_EARTHDRIVE] = {
	Name: 'LG_EARTHDRIVE',
	SkillName: 'Earth Drive',
	MaxLv: 5,
	SpAmount: [52, 60, 68, 76, 84],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.LG_REFLECTDAMAGE, 3]]
};
SkillInfo[SK.SR_SKYNETBLOW] = {
	Name: 'SR_SKYNETBLOW',
	SkillName: 'Sky Blow',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_DRAGONCOMBO, 3]]
};
SkillInfo[SK.PR_KYRIE] = {
	Name: 'PR_KYRIE',
	SkillName: 'Kyrie Eleison',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 25, 25, 25, 30, 30, 30, 35],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AL_ANGELUS, 2]]
};
SkillInfo[SK.SR_LIGHTNINGWALK] = {
	Name: 'SR_LIGHTNINGWALK',
	SkillName: 'Lightning Walk',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_WINDMILL, 1]]
};
SkillInfo[SK.SR_GATEOFHELL] = {
	Name: 'SR_GATEOFHELL',
	SkillName: 'Gates of Hell',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.SR_TIGERCANNON, 5],
		[SK.SR_RAMPAGEBLASTER, 1]
	]
};
SkillInfo[SK.SR_GENTLETOUCH_CHANGE] = {
	Name: 'SR_GENTLETOUCH_CHANGE',
	SkillName: 'Gentle Touch-Convert',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SR_GENTLETOUCH_QUIET, 1],
		[SK.SR_GENTLETOUCH_CURE, 1],
		[SK.SR_GENTLETOUCH_ENERGYGAIN, 3]
	]
};
SkillInfo[SK.WA_SYMPHONY_OF_LOVER] = {
	Name: 'WA_SYMPHONY_OF_LOVER',
	SkillName: 'Lover Symphony',
	MaxLv: 5,
	SpAmount: [60, 69, 78, 87, 96],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.PR_MAGNIFICAT] = {
	Name: 'PR_MAGNIFICAT',
	SkillName: 'Magnificat',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MI_HARMONIZE] = {
	Name: 'MI_HARMONIZE',
	SkillName: 'Harmonize',
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.PR_GLORIA] = {
	Name: 'PR_GLORIA',
	SkillName: 'Gloria',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.PR_KYRIE, 4],
		[SK.PR_MAGNIFICAT, 3]
	],
	NeedSkillList: { [JobId.SUPERNOVICE2]: [[SK.PR_SANCTUARY, 7]] }
};
SkillInfo[SK.WM_POEMOFNETHERWORLD] = {
	Name: 'WM_POEMOFNETHERWORLD',
	SkillName: 'Song of Despair',
	MaxLv: 5,
	SpAmount: [12, 16, 20, 24, 28],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_LESSON, 1]]
};
SkillInfo[SK.WM_SIRCLEOFNATURE] = {
	Name: 'WM_SIRCLEOFNATURE',
	SkillName: 'Circle of Nature',
	MaxLv: 5,
	SpAmount: [42, 46, 50, 54, 58],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LESSON, 1]]
};
SkillInfo[SK.PR_LEXDIVINA] = {
	Name: 'PR_LEXDIVINA',
	SkillName: 'Lex Divina',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 18, 16, 14, 12, 10],
	bSeperateLv: false,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.AL_RUWACH, 1]]
};
SkillInfo[SK.WM_LERADS_DEW] = {
	Name: 'WM_LERADS_DEW',
	SkillName: "Lerad's Dew",
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.MINSTREL]: [
			[SK.MI_HARMONIZE, 1],
			[SK.MI_RUSH_WINDMILL, 1],
			[SK.MI_ECHOSONG, 1]
		],
		[JobId.WANDERER]: [
			[SK.WA_SWING_DANCE, 1],
			[SK.WA_SYMPHONY_OF_LOVER, 1],
			[SK.WA_MOONLIT_SERENADE, 1]
		]
	}
};
SkillInfo[SK.SO_FIREWALK] = {
	Name: 'SO_FIREWALK',
	SkillName: 'Fire Walk',
	MaxLv: 5,
	SpAmount: [30, 34, 38, 42, 46],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_VOLCANO, 1]]
};
SkillInfo[SK.SO_DIAMONDDUST] = {
	Name: 'SO_DIAMONDDUST',
	SkillName: 'Diamond Dust',
	MaxLv: 5,
	SpAmount: [50, 56, 62, 68, 74],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SA_DELUGE, 3]]
};
SkillInfo[SK.SO_STRIKING] = {
	Name: 'SO_STRIKING',
	SkillName: 'Striking',
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SA_FLAMELAUNCHER, 1],
		[SK.SA_FROSTWEAPON, 1],
		[SK.SA_LIGHTNINGLOADER, 1],
		[SK.SA_SEISMICWEAPON, 1]
	]
};
SkillInfo[SK.SO_ARRULLO] = {
	Name: 'SO_ARRULLO',
	SkillName: 'Arrullo',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 9],
	_NeedSkillList: [[SK.SO_WARMER, 2]]
};
SkillInfo[SK.PR_TURNUNDEAD] = {
	Name: 'PR_TURNUNDEAD',
	SkillName: 'Turn Undead',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	_NeedSkillList: [
		[SK.ALL_RESURRECTION, 1],
		[SK.PR_LEXDIVINA, 3]
	]
};
SkillInfo[SK.SO_EL_SYMPATHY] = {
	Name: 'SO_EL_SYMPATHY',
	SkillName: 'Spirit Sympathy',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SO_EL_CONTROL, 3]]
};
SkillInfo[SK.SO_WIND_INSIGNIA] = {
	Name: 'SO_WIND_INSIGNIA',
	SkillName: 'Wind Insignia',
	MaxLv: 3,
	SpAmount: [22, 30, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SO_SUMMON_VENTUS, 3]]
};
SkillInfo[SK.GN_REMODELING_CART] = {
	Name: 'GN_REMODELING_CART',
	SkillName: 'Cart Remodeling',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.GN_THORNS_TRAP] = {
	Name: 'GN_THORNS_TRAP',
	SkillName: 'Thorn Trap',
	MaxLv: 5,
	SpAmount: [22, 26, 30, 34, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 2]]
};
SkillInfo[SK.GN_CRAZYWEED] = {
	Name: 'GN_CRAZYWEED',
	SkillName: 'Crazy Vines',
	MaxLv: 10,
	SpAmount: [24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.GN_WALLOFTHORN, 3]]
};
SkillInfo[SK.PR_LEXAETERNA] = {
	Name: 'PR_LEXAETERNA',
	SkillName: 'Lex Aeterna',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.PR_LEXDIVINA, 5]]
};
SkillInfo[SK.GN_MIX_COOKING] = {
	Name: 'GN_MIX_COOKING',
	SkillName: 'Mixed Cooking',
	MaxLv: 2,
	SpAmount: [5, 40],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 1]]
};
SkillInfo[SK.GD_EXTENSION] = {
	Name: 'GD_EXTENSION',
	SkillName: 'Guild Extension',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AB_SECRAMENT] = {
	Name: 'AB_SECRAMENT',
	SkillName: 'Sacrament',
	MaxLv: 5,
	SpAmount: [100, 120, 140, 160, 180],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.AB_EXPIATIO, 1],
		[SK.AB_EPICLESIS, 1]
	]
};
SkillInfo[SK.PR_MAGNUS] = {
	Name: 'PR_MAGNUS',
	SkillName: 'Magnus Exorcismus',
	MaxLv: 10,
	SpAmount: [40, 42, 44, 46, 48, 50, 52, 54, 56, 58],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_SAFETYWALL, 1],
		[SK.PR_LEXAETERNA, 1],
		[SK.PR_TURNUNDEAD, 3]
	]
};
SkillInfo[SK.ALL_BUYING_STORE] = {
	Name: 'ALL_BUYING_STORE',
	SkillName: 'Open Buying Store',
	MaxLv: 2,
	SpAmount: [30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1]
};
SkillInfo[SK.SM_BASH] = {
	Name: 'SM_BASH',
	SkillName: 'Bash',
	MaxLv: 10,
	SpAmount: [8, 8, 8, 8, 8, 15, 15, 15, 15, 15],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WZ_FIREPILLAR] = {
	Name: 'WZ_FIREPILLAR',
	SkillName: 'Fire Pillar',
	MaxLv: 10,
	SpAmount: [75, 75, 75, 75, 75, 75, 75, 75, 75, 75],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_FIREWALL, 1]]
};
SkillInfo[SK.MA_REMOVETRAP] = {
	Name: 'MA_REMOVETRAP',
	SkillName: 'Remove Trap',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [2]
};
SkillInfo[SK.MER_RECUPERATE] = {
	Name: 'MER_RECUPERATE',
	SkillName: 'Recuperate',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.WZ_SIGHTRASHER] = {
	Name: 'WZ_SIGHTRASHER',
	SkillName: 'Sightrasher',
	MaxLv: 10,
	SpAmount: [35, 37, 39, 41, 43, 45, 47, 49, 51, 53],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MG_SIGHT, 1],
		[SK.MG_LIGHTNINGBOLT, 1]
	]
};
SkillInfo[SK.EL_WATER_DROP] = {
	Name: 'EL_WATER_DROP',
	SkillName: 'Water Drop',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WZ_FIREIVY] = {
	Name: 'WZ_FIREIVY',
	SkillName: 'Fire Ivy',
	MaxLv: 0,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: []
};
SkillInfo[SK.EL_TYPOON_MIS] = {
	Name: 'EL_TYPOON_MIS',
	SkillName: 'Typhoon Mist',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [11]
};
SkillInfo[SK.WZ_METEOR] = {
	Name: 'WZ_METEOR',
	SkillName: 'Meteor Storm',
	MaxLv: 10,
	SpAmount: [20, 24, 30, 34, 40, 44, 50, 54, 60, 64],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_THUNDERSTORM, 1],
		[SK.WZ_SIGHTRASHER, 2]
	]
};
SkillInfo[SK.WZ_JUPITEL] = {
	Name: 'WZ_JUPITEL',
	SkillName: 'Jupitel Thunder',
	MaxLv: 10,
	SpAmount: [20, 23, 26, 29, 32, 35, 38, 41, 44, 47],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_NAPALMBEAT, 1],
		[SK.MG_LIGHTNINGBOLT, 1]
	]
};
SkillInfo[SK.WZ_VERMILION] = {
	Name: 'WZ_VERMILION',
	SkillName: 'Lord of Vermilion',
	MaxLv: 10,
	SpAmount: [60, 64, 68, 72, 76, 80, 84, 88, 92, 96],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_THUNDERSTORM, 1],
		[SK.WZ_JUPITEL, 5]
	]
};
SkillInfo[SK.WZ_WATERBALL] = {
	Name: 'WZ_WATERBALL',
	SkillName: 'Waterball',
	MaxLv: 5,
	SpAmount: [15, 20, 20, 25, 25],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_COLDBOLT, 1],
		[SK.MG_LIGHTNINGBOLT, 1]
	]
};
SkillInfo[SK.WZ_ICEWALL] = {
	Name: 'WZ_ICEWALL',
	SkillName: 'Ice Wall',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_STONECURSE, 1],
		[SK.MG_FROSTDIVER, 1]
	]
};
SkillInfo[SK.WZ_FROSTNOVA] = {
	Name: 'WZ_FROSTNOVA',
	SkillName: 'Frost Nova',
	MaxLv: 10,
	SpAmount: [45, 43, 41, 39, 37, 35, 33, 31, 29, 27],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WZ_ICEWALL, 1]]
};
SkillInfo[SK.WZ_STORMGUST] = {
	Name: 'WZ_STORMGUST',
	SkillName: 'Storm Gust',
	MaxLv: 10,
	SpAmount: [78, 78, 78, 78, 78, 78, 78, 78, 78, 78],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_FROSTDIVER, 1],
		[SK.WZ_JUPITEL, 3]
	]
};
SkillInfo[SK.WZ_EARTHSPIKE] = {
	Name: 'WZ_EARTHSPIKE',
	SkillName: 'Earth Spike',
	MaxLv: 5,
	SpAmount: [14, 18, 22, 26, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_STONECURSE, 1]],
	NeedSkillList: { [JobId.SAGE]: [[SK.SA_SEISMICWEAPON, 1]] }
};
SkillInfo[SK.WZ_HEAVENDRIVE] = {
	Name: 'WZ_HEAVENDRIVE',
	SkillName: "Heaven's Drive",
	MaxLv: 5,
	SpAmount: [28, 32, 36, 40, 44],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WZ_EARTHSPIKE, 3]],
	NeedSkillList: { [JobId.SAGE]: [[SK.WZ_EARTHSPIKE, 1]] }
};
SkillInfo[SK.WZ_QUAGMIRE] = {
	Name: 'WZ_QUAGMIRE',
	SkillName: 'Quagmire',
	MaxLv: 5,
	SpAmount: [5, 10, 15, 20, 25],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WZ_HEAVENDRIVE, 1]]
};
SkillInfo[SK.WZ_ESTIMATION] = {
	Name: 'WZ_ESTIMATION',
	SkillName: 'Sense',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.HLIF_BRAIN] = {
	Name: 'HLIF_BRAIN',
	SkillName: 'Brain Surgery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HFLI_SPEED] = {
	Name: 'HFLI_SPEED',
	SkillName: 'Accelerated Flight',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_NEEDLE_OF_PARALYZE] = {
	Name: 'MH_NEEDLE_OF_PARALYZE',
	SkillName: 'Needle of Paralysis',
	MaxLv: 10,
	SpAmount: [42, 48, 54, 60, 66, 72, 78, 84, 90, 96],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5]
};
SkillInfo[SK.MH_STYLE_CHANGE] = {
	Name: 'MH_STYLE_CHANGE',
	SkillName: 'Style Change',
	MaxLv: 1,
	SpAmount: [35],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MH_ANGRIFFS_MODUS] = {
	Name: 'MH_ANGRIFFS_MODUS',
	SkillName: 'Angriffs Modus',
	MaxLv: 5,
	SpAmount: [60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_VOLCANIC_ASH] = {
	Name: 'MH_VOLCANIC_ASH',
	SkillName: 'Volcanic Ash',
	MaxLv: 5,
	SpAmount: [60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7]
};
SkillInfo[SK.BS_IRON] = {
	Name: 'BS_IRON',
	SkillName: 'Iron Tempering',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_GLORYGUILD] = {
	Name: 'GD_GLORYGUILD',
	SkillName: 'Guild Glory',
	MaxLv: 0,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: []
};
SkillInfo[SK.BS_STEEL] = {
	Name: 'BS_STEEL',
	SkillName: 'Steel Tempering',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_IRON, 1]]
};
SkillInfo[SK.SM_PROVOKE] = {
	Name: 'SM_PROVOKE',
	SkillName: 'Provoke',
	MaxLv: 10,
	SpAmount: [4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.BS_ENCHANTEDSTONE] = {
	Name: 'BS_ENCHANTEDSTONE',
	SkillName: 'Enchanted Stone Craft',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_IRON, 1]]
};
SkillInfo[SK.MA_CHARGEARROW] = {
	Name: 'MA_CHARGEARROW',
	SkillName: 'Arrow Repel',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.MER_MENTALCURE] = {
	Name: 'MER_MENTALCURE',
	SkillName: 'Mental Cure',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.BS_ORIDEOCON] = {
	Name: 'BS_ORIDEOCON',
	SkillName: 'Research Oridecon',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_ENCHANTEDSTONE, 1]]
};
SkillInfo[SK.EL_WATER_BARRIER] = {
	Name: 'EL_WATER_BARRIER',
	SkillName: 'Water Barrier',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.BS_DAGGER] = {
	Name: 'BS_DAGGER',
	SkillName: 'Smith Dagger',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.EL_TYPOON_MIS_ATK] = {
	Name: 'EL_TYPOON_MIS_ATK',
	SkillName: 'Typhoon Mist Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [11]
};
SkillInfo[SK.BS_SWORD] = {
	Name: 'BS_SWORD',
	SkillName: 'Smith Sword',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_DAGGER, 1]]
};
SkillInfo[SK.BS_TWOHANDSWORD] = {
	Name: 'BS_TWOHANDSWORD',
	SkillName: 'Smith Two-handed Sword',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_SWORD, 1]]
};
SkillInfo[SK.BS_AXE] = {
	Name: 'BS_AXE',
	SkillName: 'Smith Axe',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_SWORD, 2]]
};
SkillInfo[SK.BS_MACE] = {
	Name: 'BS_MACE',
	SkillName: 'Smith Mace',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_KNUCKLE, 1]]
};
SkillInfo[SK.BS_KNUCKLE] = {
	Name: 'BS_KNUCKLE',
	SkillName: 'Smith Brass Knuckle',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_DAGGER, 1]]
};
SkillInfo[SK.BS_SPEAR] = {
	Name: 'BS_SPEAR',
	SkillName: 'Smith Spear',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.BS_DAGGER, 2]]
};
SkillInfo[SK.BS_HILTBINDING] = {
	Name: 'BS_HILTBINDING',
	SkillName: 'Hilt Binding',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.BS_FINDINGORE] = {
	Name: 'BS_FINDINGORE',
	SkillName: 'Ore Discovery',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.BS_HILTBINDING, 1],
		[SK.BS_STEEL, 1]
	]
};
SkillInfo[SK.BS_WEAPONRESEARCH] = {
	Name: 'BS_WEAPONRESEARCH',
	SkillName: 'Weaponry Research',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_HILTBINDING, 1]]
};
SkillInfo[SK.BS_REPAIRWEAPON] = {
	Name: 'BS_REPAIRWEAPON',
	SkillName: 'Repair Weapon',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [2],
	_NeedSkillList: [[SK.BS_WEAPONRESEARCH, 1]]
};
SkillInfo[SK.BS_SKINTEMPER] = {
	Name: 'BS_SKINTEMPER',
	SkillName: 'Skin Tempering',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.BS_HAMMERFALL] = {
	Name: 'BS_HAMMERFALL',
	SkillName: 'Hammerfall',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_LEADERSHIP] = {
	Name: 'GD_LEADERSHIP',
	SkillName: 'Guild Leadership',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.BS_ADRENALINE] = {
	Name: 'BS_ADRENALINE',
	SkillName: 'Adrenaline Rush',
	MaxLv: 5,
	SpAmount: [20, 23, 26, 29, 32],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_HAMMERFALL, 2]]
};
SkillInfo[SK.SM_MAGNUM] = {
	Name: 'SM_MAGNUM',
	SkillName: 'Magnum Break',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SM_BASH, 5]]
};
SkillInfo[SK.BS_WEAPONPERFECT] = {
	Name: 'BS_WEAPONPERFECT',
	SkillName: 'Weapon Perfection',
	MaxLv: 5,
	SpAmount: [18, 16, 14, 12, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BS_WEAPONRESEARCH, 2],
		[SK.BS_ADRENALINE, 2]
	]
};
SkillInfo[SK.MA_SHARPSHOOTING] = {
	Name: 'MA_SHARPSHOOTING',
	SkillName: 'Focused Arrow Strike',
	MaxLv: 5,
	SpAmount: [18, 21, 24, 27, 30],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MER_COMPRESS] = {
	Name: 'MER_COMPRESS',
	SkillName: 'Compress',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.BS_OVERTHRUST] = {
	Name: 'BS_OVERTHRUST',
	SkillName: 'Power Thrust',
	MaxLv: 5,
	SpAmount: [18, 16, 14, 12, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BS_ADRENALINE, 3]]
};
SkillInfo[SK.EL_WIND_STEP] = {
	Name: 'EL_WIND_STEP',
	SkillName: 'Wind Step',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.BS_MAXIMIZE] = {
	Name: 'BS_MAXIMIZE',
	SkillName: 'Maximize Power',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BS_WEAPONPERFECT, 3],
		[SK.BS_OVERTHRUST, 2]
	]
};
SkillInfo[SK.EL_STONE_HAMMER] = {
	Name: 'EL_STONE_HAMMER',
	SkillName: 'Stone Hammer',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [5]
};
SkillInfo[SK.HT_SKIDTRAP] = {
	Name: 'HT_SKIDTRAP',
	SkillName: 'Skid Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.HT_LANDMINE] = {
	Name: 'HT_LANDMINE',
	SkillName: 'Land Mine',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.HT_ANKLESNARE] = {
	Name: 'HT_ANKLESNARE',
	SkillName: 'Anklesnare',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_SKIDTRAP, 1]]
};
SkillInfo[SK.HT_SHOCKWAVE] = {
	Name: 'HT_SHOCKWAVE',
	SkillName: 'Shockwave Trap',
	MaxLv: 5,
	SpAmount: [45, 45, 45, 45, 45],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_ANKLESNARE, 1]]
};
SkillInfo[SK.HT_SANDMAN] = {
	Name: 'HT_SANDMAN',
	SkillName: 'Sandman',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_FLASHER, 1]]
};
SkillInfo[SK.HT_FLASHER] = {
	Name: 'HT_FLASHER',
	SkillName: 'Flasher',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_SKIDTRAP, 1]]
};
SkillInfo[SK.HT_FREEZINGTRAP] = {
	Name: 'HT_FREEZINGTRAP',
	SkillName: 'Freezing Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_FLASHER, 1]]
};
SkillInfo[SK.HT_BLASTMINE] = {
	Name: 'HT_BLASTMINE',
	SkillName: 'Blast Mine',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [
		[SK.HT_LANDMINE, 1],
		[SK.HT_SANDMAN, 1],
		[SK.HT_FREEZINGTRAP, 1]
	]
};
SkillInfo[SK.HT_CLAYMORETRAP] = {
	Name: 'HT_CLAYMORETRAP',
	SkillName: 'Claymore Trap',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [
		[SK.HT_SHOCKWAVE, 1],
		[SK.HT_BLASTMINE, 1]
	]
};
SkillInfo[SK.HT_REMOVETRAP] = {
	Name: 'HT_REMOVETRAP',
	SkillName: 'Remove Trap',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [2],
	_NeedSkillList: [[SK.HT_LANDMINE, 1]],
	NeedSkillList: { [JobId.ROGUE]: [[SK.AC_DOUBLE, 5]] }
};
SkillInfo[SK.HT_TALKIEBOX] = {
	Name: 'HT_TALKIEBOX',
	SkillName: 'Talkie Box',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [
		[SK.HT_REMOVETRAP, 1],
		[SK.HT_SHOCKWAVE, 1]
	]
};
SkillInfo[SK.RK_SONICWAVE] = {
	Name: 'RK_SONICWAVE',
	SkillName: 'Sonic Wave',
	MaxLv: 10,
	SpAmount: [33, 36, 39, 42, 45, 48, 51, 54, 57, 60],
	bSeperateLv: true,
	AttackRange: [7, 7, 8, 8, 9, 9, 10, 10, 11, 11],
	_NeedSkillList: [[SK.RK_ENCHANTBLADE, 3]]
};
SkillInfo[SK.RK_HUNDREDSPEAR] = {
	Name: 'RK_HUNDREDSPEAR',
	SkillName: 'Hundred Spears',
	MaxLv: 10,
	SpAmount: [60, 60, 60, 60, 60, 60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.RK_PHANTOMTHRUST, 3]]
};
SkillInfo[SK.RK_IGNITIONBREAK] = {
	Name: 'RK_IGNITIONBREAK',
	SkillName: 'Ignition Break',
	MaxLv: 5,
	SpAmount: [35, 40, 45, 50, 55],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.RK_DEATHBOUND, 5],
		[SK.RK_SONICWAVE, 2],
		[SK.RK_WINDCUTTER, 3]
	]
};
SkillInfo[SK.RK_DRAGONBREATH] = {
	Name: 'RK_DRAGONBREATH',
	SkillName: "Dragon's Breath",
	MaxLv: 10,
	SpAmount: [30, 35, 40, 45, 50, 55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RK_DRAGONTRAINING, 2]]
};
SkillInfo[SK.RK_RUNEMASTERY] = {
	Name: 'RK_RUNEMASTERY',
	SkillName: 'Rune Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.RK_CRUSHSTRIKE] = {
	Name: 'RK_CRUSHSTRIKE',
	SkillName: 'Crushing Strike',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.HT_BEASTBANE] = {
	Name: 'HT_BEASTBANE',
	SkillName: 'Beastbane',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.RK_VITALITYACTIVATION] = {
	Name: 'RK_VITALITYACTIVATION',
	SkillName: 'Vitality Activation',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.RK_FIGHTINGSPIRIT] = {
	Name: 'RK_FIGHTINGSPIRIT',
	SkillName: 'Determination',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.RK_PHANTOMTHRUST] = {
	Name: 'RK_PHANTOMTHRUST',
	SkillName: 'Phantom Thrust',
	MaxLv: 5,
	SpAmount: [15, 18, 21, 24, 27],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [[SK.KN_BRANDISHSPEAR, 2]]
};
SkillInfo[SK.GC_CROSSIMPACT] = {
	Name: 'GC_CROSSIMPACT',
	SkillName: 'Cross Impact',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.AS_SONICBLOW, 10]]
};
SkillInfo[SK.GC_RESEARCHNEWPOISON] = {
	Name: 'GC_RESEARCHNEWPOISON',
	SkillName: 'New Poison Research',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GC_ANTIDOTE] = {
	Name: 'GC_ANTIDOTE',
	SkillName: 'Antidote',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [5],
	_NeedSkillList: [[SK.GC_RESEARCHNEWPOISON, 5]]
};
SkillInfo[SK.GC_WEAPONBLOCKING] = {
	Name: 'GC_WEAPONBLOCKING',
	SkillName: 'Weapon Blocking',
	MaxLv: 5,
	SpAmount: [40, 36, 32, 28, 24],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_LEFT, 5]]
};
SkillInfo[SK.HT_FALCON] = {
	Name: 'HT_FALCON',
	SkillName: 'Falconry Mastery',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.HT_BEASTBANE, 1]]
};
SkillInfo[SK.GC_POISONSMOKE] = {
	Name: 'GC_POISONSMOKE',
	SkillName: 'Poisonous Smoke',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [
		[SK.GC_POISONINGWEAPON, 5],
		[SK.GC_VENOMPRESSURE, 5]
	]
};
SkillInfo[SK.GC_PHANTOMMENACE] = {
	Name: 'GC_PHANTOMMENACE',
	SkillName: 'Phantom Menace',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.GC_CLOAKINGEXCEED, 5],
		[SK.GC_DARKILLUSION, 5]
	]
};
SkillInfo[SK.GC_ROLLINGCUTTER] = {
	Name: 'GC_ROLLINGCUTTER',
	SkillName: 'Rolling Cutter',
	MaxLv: 5,
	SpAmount: [5, 5, 5, 5, 5],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_SONICBLOW, 10]]
};
SkillInfo[SK.AB_JUDEX] = {
	Name: 'AB_JUDEX',
	SkillName: 'Judex',
	MaxLv: 10,
	SpAmount: [20, 23, 26, 29, 32, 34, 36, 38, 40, 42],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.PR_TURNUNDEAD, 1]]
};
SkillInfo[SK.AB_ADORAMUS] = {
	Name: 'AB_ADORAMUS',
	SkillName: 'Adoramus',
	MaxLv: 10,
	SpAmount: [32, 40, 48, 56, 64, 72, 80, 88, 96, 104],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.AB_JUDEX, 5],
		[SK.AB_ANCILLA, 1],
		[SK.PR_MAGNUS, 1]
	]
};
SkillInfo[SK.AB_CANTO] = {
	Name: 'AB_CANTO',
	SkillName: 'Cantocandidus',
	MaxLv: 3,
	SpAmount: [200, 220, 240],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.AL_INCAGI, 1]]
};
SkillInfo[SK.SM_ENDURE] = {
	Name: 'SM_ENDURE',
	SkillName: 'Endure',
	MaxLv: 10,
	SpAmount: [10, 10, 10, 10, 10, 10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SM_PROVOKE, 5]]
};
SkillInfo[SK.HT_STEELCROW] = {
	Name: 'HT_STEELCROW',
	SkillName: 'Steel Crow',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.HT_BLITZBEAT, 5]]
};
SkillInfo[SK.AB_LAUDARAMUS] = {
	Name: 'AB_LAUDARAMUS',
	SkillName: 'Lauda Ramus',
	MaxLv: 4,
	SpAmount: [50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11],
	_NeedSkillList: [[SK.AB_LAUDAAGNUS, 2]]
};
SkillInfo[SK.AB_CLEARANCE] = {
	Name: 'AB_CLEARANCE',
	SkillName: 'Clearance',
	MaxLv: 5,
	SpAmount: [54, 60, 66, 72, 78],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.AB_LAUDARAMUS, 2]]
};
SkillInfo[SK.AB_DUPLELIGHT] = {
	Name: 'AB_DUPLELIGHT',
	SkillName: 'Duple Light',
	MaxLv: 10,
	SpAmount: [55, 60, 65, 70, 75, 80, 85, 90, 95, 100],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.PR_ASPERSIO, 1]]
};
SkillInfo[SK.AB_DUPLELIGHT_MAGIC] = {
	Name: 'AB_DUPLELIGHT_MAGIC',
	SkillName: 'Duple Magic',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11]
};
SkillInfo[SK.HT_BLITZBEAT] = {
	Name: 'HT_BLITZBEAT',
	SkillName: 'Blitz Beat',
	MaxLv: 5,
	SpAmount: [10, 13, 16, 19, 22],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.HT_FALCON, 1]]
};
SkillInfo[SK.HT_DETECTING] = {
	Name: 'HT_DETECTING',
	SkillName: 'Detect',
	MaxLv: 4,
	SpAmount: [8, 8, 8, 8],
	bSeperateLv: false,
	AttackRange: [3, 5, 7, 9],
	_NeedSkillList: [
		[SK.AC_CONCENTRATION, 1],
		[SK.HT_FALCON, 1]
	]
};
SkillInfo[SK.HT_SPRINGTRAP] = {
	Name: 'HT_SPRINGTRAP',
	SkillName: 'Spring Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [4, 5, 6, 7, 8],
	_NeedSkillList: [[SK.HT_FALCON], [SK.HT_REMOVETRAP, 1]]
};
SkillInfo[SK.EL_WIND_CURTAIN] = {
	Name: 'EL_WIND_CURTAIN',
	SkillName: 'Wind Curtain',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AS_RIGHT] = {
	Name: 'AS_RIGHT',
	SkillName: 'Righthand Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_ROCK_CRUSHER] = {
	Name: 'EL_ROCK_CRUSHER',
	SkillName: 'Rock Crusher',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [3]
};
SkillInfo[SK.AS_LEFT] = {
	Name: 'AS_LEFT',
	SkillName: 'Lefthand Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_RIGHT, 2]]
};
SkillInfo[SK.AS_KATAR] = {
	Name: 'AS_KATAR',
	SkillName: 'Katar Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AS_CLOAKING] = {
	Name: 'AS_CLOAKING',
	SkillName: 'Cloaking',
	MaxLv: 10,
	SpAmount: [15, 15, 15, 15, 15, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TF_HIDING, 2]]
};
SkillInfo[SK.AS_SONICBLOW] = {
	Name: 'AS_SONICBLOW',
	SkillName: 'Sonic Blow',
	MaxLv: 10,
	SpAmount: [16, 18, 20, 22, 24, 26, 28, 30, 32, 34],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_KATAR, 4]]
};
SkillInfo[SK.AS_GRIMTOOTH] = {
	Name: 'AS_GRIMTOOTH',
	SkillName: 'Grimtooth',
	MaxLv: 5,
	SpAmount: [3, 3, 3, 3, 3],
	bSeperateLv: false,
	AttackRange: [2, 3, 4, 5, 6],
	_NeedSkillList: [
		[SK.AS_CLOAKING, 2],
		[SK.AS_SONICBLOW, 5]
	]
};
SkillInfo[SK.AS_ENCHANTPOISON] = {
	Name: 'AS_ENCHANTPOISON',
	SkillName: 'Enchant Poison',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TF_POISON, 1]]
};
SkillInfo[SK.WL_RADIUS] = {
	Name: 'WL_RADIUS',
	SkillName: 'Radius',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.WL_HELLINFERNO] = {
	Name: 'WL_HELLINFERNO',
	SkillName: 'Hell Inferno',
	MaxLv: 5,
	SpAmount: [64, 70, 76, 82, 88],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_CRIMSONROCK, 2]]
};
SkillInfo[SK.WL_EARTHSTRAIN] = {
	Name: 'WL_EARTHSTRAIN',
	SkillName: 'Earth Strain',
	MaxLv: 5,
	SpAmount: [70, 78, 86, 94, 102],
	bSeperateLv: true,
	AttackRange: [6, 6, 6, 6, 6],
	_NeedSkillList: [[SK.WL_SIENNAEXECRATE, 2]]
};
SkillInfo[SK.AS_POISONREACT] = {
	Name: 'AS_POISONREACT',
	SkillName: 'Poison React',
	MaxLv: 10,
	SpAmount: [25, 30, 35, 40, 45, 50, 55, 60, 45, 45],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AS_ENCHANTPOISON, 3]]
};
SkillInfo[SK.WL_SUMMONWB] = {
	Name: 'WL_SUMMONWB',
	SkillName: 'Summon Water Ball',
	MaxLv: 2,
	SpAmount: [10, 50],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.WZ_STORMGUST, 1]]
};
SkillInfo[SK.WL_FREEZE_SP] = {
	Name: 'WL_FREEZE_SP',
	SkillName: 'Freezing Spell',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AS_VENOMDUST] = {
	Name: 'AS_VENOMDUST',
	SkillName: 'Venom Dust',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.AS_ENCHANTPOISON, 5]]
};
SkillInfo[SK.RA_WUGMASTERY] = {
	Name: 'RA_WUGMASTERY',
	SkillName: 'Warg Mastery',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.RA_WUGBITE] = {
	Name: 'RA_WUGBITE',
	SkillName: 'Warg Bite',
	MaxLv: 5,
	SpAmount: [40, 44, 46, 48, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RA_WUGSTRIKE, 1]]
};
SkillInfo[SK.RA_RESEARCHTRAP] = {
	Name: 'RA_RESEARCHTRAP',
	SkillName: 'Trap Research',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.HT_CLAYMORETRAP, 1],
		[SK.HT_REMOVETRAP, 1]
	]
};
SkillInfo[SK.AS_SPLASHER] = {
	Name: 'AS_SPLASHER',
	SkillName: 'Venom Splasher',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AS_VENOMDUST, 5],
		[SK.AS_POISONREACT, 5]
	]
};
SkillInfo[SK.NC_BOOSTKNUCKLE] = {
	Name: 'NC_BOOSTKNUCKLE',
	SkillName: 'Knuckle Boost',
	MaxLv: 5,
	SpAmount: [5, 10, 15, 20, 25],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.NC_MADOLICENCE, 1]]
};
SkillInfo[SK.NC_COLDSLOWER] = {
	Name: 'NC_COLDSLOWER',
	SkillName: 'Ice Launcher',
	MaxLv: 3,
	SpAmount: [20, 20, 20],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [[SK.NC_VULCANARM, 3]]
};
SkillInfo[SK.NC_F_SIDESLIDE] = {
	Name: 'NC_F_SIDESLIDE',
	SkillName: 'Front Slide',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NC_HOVERING, 1]]
};
SkillInfo[SK.NV_FIRSTAID] = {
	Name: 'NV_FIRSTAID',
	SkillName: 'First Aid',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [3],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.NC_MAGNETICFIELD] = {
	Name: 'NC_MAGNETICFIELD',
	SkillName: 'Magnetic Field',
	MaxLv: 3,
	SpAmount: [60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.NC_EMERGENCYCOOL, 1]]
};
SkillInfo[SK.NC_TRAININGAXE] = {
	Name: 'NC_TRAININGAXE',
	SkillName: 'Axe Mastery ',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.NC_AXETORNADO] = {
	Name: 'NC_AXETORNADO',
	SkillName: 'Axe Tornado ',
	MaxLv: 5,
	SpAmount: [45, 45, 45, 45, 45],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NC_TRAININGAXE, 1]]
};
SkillInfo[SK.NV_TRICKDEAD] = {
	Name: 'NV_TRICKDEAD',
	SkillName: 'Play Dead',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SC_TRIANGLESHOT] = {
	Name: 'SC_TRIANGLESHOT',
	SkillName: 'Triangle Shot',
	MaxLv: 10,
	SpAmount: [22, 24, 26, 28, 30, 32, 34, 36, 38, 40],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 9, 9, 9, 9, 11, 11, 11],
	_NeedSkillList: [[SK.AC_DOUBLE, 7]]
};
SkillInfo[SK.SC_ENERVATION] = {
	Name: 'SC_ENERVATION',
	SkillName: 'Masquerade-Enervation',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [[SK.SC_BODYPAINT, 1]]
};
SkillInfo[SK.MG_SRECOVERY] = {
	Name: 'MG_SRECOVERY',
	SkillName: 'Increase SP Recovery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SM_MOVINGRECOVERY] = {
	Name: 'SM_MOVINGRECOVERY',
	SkillName: 'HP Recovery While Moving',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SC_FEINTBOMB] = {
	Name: 'SC_FEINTBOMB',
	SkillName: 'Feint Bomb',
	MaxLv: 10,
	SpAmount: [24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SC_DIMENSIONDOOR, 3]]
};
SkillInfo[SK.LG_BANISHINGPOINT] = {
	Name: 'LG_BANISHINGPOINT',
	SkillName: 'Vanishing Point',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 25, 25, 25, 25, 25],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.KN_SPEARMASTERY, 1]]
};
SkillInfo[SK.LG_PINPOINTATTACK] = {
	Name: 'LG_PINPOINTATTACK',
	SkillName: 'Pinpoint Attack',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.LG_BANISHINGPOINT, 5]]
};
SkillInfo[SK.SM_FATALBLOW] = {
	Name: 'SM_FATALBLOW',
	SkillName: 'Fatal Blow',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.LG_MOONSLASHER] = {
	Name: 'LG_MOONSLASHER',
	SkillName: 'Moonslasher',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KN_SPEARMASTERY, 1]]
};
SkillInfo[SK.LG_HESPERUSLIT] = {
	Name: 'LG_HESPERUSLIT',
	SkillName: 'Hesperus Lit',
	MaxLv: 5,
	SpAmount: [37, 44, 51, 58, 65],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [
		[SK.LG_PRESTIGE, 3],
		[SK.LG_BANDING, 3]
	]
};
SkillInfo[SK.SR_EARTHSHAKER] = {
	Name: 'SR_EARTHSHAKER',
	SkillName: 'Earth Shaker',
	MaxLv: 5,
	SpAmount: [36, 40, 44, 48, 52],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_DRAGONCOMBO, 3]]
};
SkillInfo[SK.SM_AUTOBERSERK] = {
	Name: 'SM_AUTOBERSERK',
	SkillName: 'Berserk',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SR_KNUCKLEARROW] = {
	Name: 'SR_KNUCKLEARROW',
	SkillName: 'Knuckle Arrow',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: false,
	AttackRange: [7, 7, 8, 8, 9, 9, 10, 10, 11, 11],
	_NeedSkillList: [[SK.SR_LIGHTNINGWALK, 1]]
};
SkillInfo[SK.SR_ASSIMILATEPOWER] = {
	Name: 'SR_ASSIMILATEPOWER',
	SkillName: 'Power Absorb',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.MO_ABSORBSPIRITS, 1],
		[SK.SR_POWERVELOCITY, 1]
	]
};
SkillInfo[SK.SR_GENTLETOUCH_QUIET] = {
	Name: 'SR_GENTLETOUCH_QUIET',
	SkillName: 'Gentle Touch-Silence',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: []
};
SkillInfo[SK.AC_MAKINGARROW] = {
	Name: 'AC_MAKINGARROW',
	SkillName: 'Arrow Crafting',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WA_MOONLIT_SERENADE] = {
	Name: 'WA_MOONLIT_SERENADE',
	SkillName: 'Moonlight Serenade',
	MaxLv: 5,
	SpAmount: [84, 96, 108, 120, 134],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.AC_CHARGEARROW] = {
	Name: 'AC_CHARGEARROW',
	SkillName: 'Arrow Repel',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.TF_SPRINKLESAND] = {
	Name: 'TF_SPRINKLESAND',
	SkillName: 'Sand Attack',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [9],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.TF_BACKSLIDING] = {
	Name: 'TF_BACKSLIDING',
	SkillName: 'Back Slide',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [7],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.TF_PICKSTONE] = {
	Name: 'TF_PICKSTONE',
	SkillName: 'Find Stone',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [2],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WM_VOICEOFSIREN] = {
	Name: 'WM_VOICEOFSIREN',
	SkillName: "Siren's Voice",
	MaxLv: 5,
	SpAmount: [48, 56, 64, 72, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_POEMOFNETHERWORLD, 3]]
};
SkillInfo[SK.WM_RANDOMIZESPELL] = {
	Name: 'WM_RANDOMIZESPELL',
	SkillName: 'Improvised Song',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_POEMOFNETHERWORLD, 1]]
};
SkillInfo[SK.TF_THROWSTONE] = {
	Name: 'TF_THROWSTONE',
	SkillName: 'Stone Fling',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [2],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.WM_MELODYOFSINK] = {
	Name: 'WM_MELODYOFSINK',
	SkillName: 'Sinking Melody',
	MaxLv: 5,
	SpAmount: [120, 130, 140, 150, 160],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_SONG_OF_MANA, 1]]
};
SkillInfo[SK.MC_CARTREVOLUTION] = {
	Name: 'MC_CARTREVOLUTION',
	SkillName: 'Cart Revolution',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [12],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SO_POISON_BUSTER] = {
	Name: 'SO_POISON_BUSTER',
	SkillName: 'Poison Burst',
	MaxLv: 5,
	SpAmount: [70, 90, 110, 130, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SO_CLOUD_KILL, 2]]
};
SkillInfo[SK.SO_WARMER] = {
	Name: 'SO_WARMER',
	SkillName: 'Warmer',
	MaxLv: 5,
	SpAmount: [40, 52, 64, 76, 88],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SA_VOLCANO, 1],
		[SK.SA_VIOLENTGALE, 1]
	]
};
SkillInfo[SK.SO_EL_CONTROL] = {
	Name: 'SO_EL_CONTROL',
	SkillName: 'Spirit Control ',
	MaxLv: 4,
	SpAmount: [10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.SO_EL_ANALYSIS, 1]]
};
SkillInfo[SK.MC_CHANGECART] = {
	Name: 'MC_CHANGECART',
	SkillName: 'Change Cart',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SO_EL_CURE] = {
	Name: 'SO_EL_CURE',
	SkillName: 'Spirit Cure',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SO_EL_SYMPATHY, 1]]
};
SkillInfo[SK.SO_EARTH_INSIGNIA] = {
	Name: 'SO_EARTH_INSIGNIA',
	SkillName: 'Earth Insignia',
	MaxLv: 3,
	SpAmount: [22, 30, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SO_SUMMON_TERA, 3]]
};
SkillInfo[SK.MC_LOUD] = {
	Name: 'MC_LOUD',
	SkillName: 'Crazy Uproar',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [8],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GN_BLOOD_SUCKER] = {
	Name: 'GN_BLOOD_SUCKER',
	SkillName: 'Blood Sucker',
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 3]]
};
SkillInfo[SK.AL_HOLYLIGHT] = {
	Name: 'AL_HOLYLIGHT',
	SkillName: 'Holy Light',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.GN_MAKEBOMB] = {
	Name: 'GN_MAKEBOMB',
	SkillName: 'Bomb Creation',
	MaxLv: 2,
	SpAmount: [5, 40],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.GN_MIX_COOKING, 1]]
};
SkillInfo[SK.GD_SOULCOLD] = {
	Name: 'GD_SOULCOLD',
	SkillName: 'Cold Heart',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MG_ENERGYCOAT] = {
	Name: 'MG_ENERGYCOAT',
	SkillName: 'Energy Coat',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ALL_GUARDIAN_RECALL] = {
	Name: 'ALL_GUARDIAN_RECALL',
	SkillName: 'Call of Guardian',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MG_SIGHT] = {
	Name: 'MG_SIGHT',
	SkillName: 'Sight',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MS_BASH] = {
	Name: 'MS_BASH',
	SkillName: 'Bash',
	MaxLv: 10,
	SpAmount: [8, 8, 8, 8, 8, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ML_BRANDISH] = {
	Name: 'ML_BRANDISH',
	SkillName: 'Brandish Spear',
	MaxLv: 10,
	SpAmount: [12, 12, 12, 12, 12, 12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
};
SkillInfo[SK.MER_AUTOBERSERK] = {
	Name: 'MER_AUTOBERSERK',
	SkillName: 'Berserk',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.EL_ZEPHYR] = {
	Name: 'EL_ZEPHYR',
	SkillName: 'Zephyr',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.EL_FIRE_ARROW] = {
	Name: 'EL_FIRE_ARROW',
	SkillName: 'Fire Arrow',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.EL_ROCK_CRUSHER_ATK] = {
	Name: 'EL_ROCK_CRUSHER_ATK',
	SkillName: 'Rock Crusher Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [5]
};
SkillInfo[SK.MG_NAPALMBEAT] = {
	Name: 'MG_NAPALMBEAT',
	SkillName: 'Napalm Beat',
	MaxLv: 10,
	SpAmount: [9, 9, 9, 12, 12, 12, 15, 15, 15, 18],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.HAMI_CASTLE] = {
	Name: 'HAMI_CASTLE',
	SkillName: 'Castling',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HVAN_CAPRICE] = {
	Name: 'HVAN_CAPRICE',
	SkillName: 'Caprice',
	MaxLv: 5,
	SpAmount: [22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.MH_PAIN_KILLER] = {
	Name: 'MH_PAIN_KILLER',
	SkillName: 'Pain Killer',
	MaxLv: 10,
	SpAmount: [48, 52, 56, 60, 64, 68, 72, 76, 80, 84],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5]
};
SkillInfo[SK.MH_SILVERVEIN_RUSH] = {
	Name: 'MH_SILVERVEIN_RUSH',
	SkillName: 'Silvervein Rush',
	MaxLv: 10,
	SpAmount: [17, 19, 21, 23, 25, 27, 29, 31, 33, 35],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3, 3, 3, 3, 3, 3]
};
SkillInfo[SK.MH_CBC] = {
	Name: 'MH_CBC',
	SkillName: 'C.B.C : Continual Break Combo',
	MaxLv: 5,
	SpAmount: [10, 20, 30, 40, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_HAWKEYES] = {
	Name: 'GD_HAWKEYES',
	SkillName: 'Sharp Gaze',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MG_SAFETYWALL] = {
	Name: 'MG_SAFETYWALL',
	SkillName: 'Safety Wall',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 35, 35, 35, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_NAPALMBEAT, 7],
		[SK.MG_SOULSTRIKE, 5]
	],
	NeedSkillList: {
		[JobId.PRIEST]: [
			[SK.PR_SANCTUARY, 3],
			[SK.PR_ASPERSIO, 4]
		]
	}
};
SkillInfo[SK.MS_MAGNUM] = {
	Name: 'MS_MAGNUM',
	SkillName: 'Magnum Break',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ML_SPIRALPIERCE] = {
	Name: 'ML_SPIRALPIERCE',
	SkillName: 'Clashing Spiral',
	MaxLv: 5,
	SpAmount: [18, 21, 24, 27, 30],
	bSeperateLv: false,
	AttackRange: [4, 4, 4, 4, 4]
};
SkillInfo[SK.MER_DECAGI] = {
	Name: 'MER_DECAGI',
	SkillName: 'Decrease AGI',
	MaxLv: 10,
	SpAmount: [15, 17, 19, 21, 23, 25, 27, 29, 31, 33],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.EL_SOLID_SKIN] = {
	Name: 'EL_SOLID_SKIN',
	SkillName: 'Solid Skin',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.EL_FIRE_BOMB] = {
	Name: 'EL_FIRE_BOMB',
	SkillName: 'Fire Bomb',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.EL_STONE_RAIN] = {
	Name: 'EL_STONE_RAIN',
	SkillName: 'Stone Rain',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.MG_SOULSTRIKE] = {
	Name: 'MG_SOULSTRIKE',
	SkillName: 'Soul Strike',
	MaxLv: 10,
	SpAmount: [18, 14, 24, 20, 30, 26, 36, 32, 42, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_NAPALMBEAT, 4]]
};
SkillInfo[SK.RG_SNATCHER] = {
	Name: 'RG_SNATCHER',
	SkillName: 'Gank',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TF_STEAL, 1]]
};
SkillInfo[SK.RG_STEALCOIN] = {
	Name: 'RG_STEALCOIN',
	SkillName: 'Mug',
	MaxLv: 10,
	SpAmount: [15, 15, 15, 15, 15, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_SNATCHER, 4]]
};
SkillInfo[SK.RG_BACKSTAP] = {
	Name: 'RG_BACKSTAP',
	SkillName: 'Back Stab',
	MaxLv: 10,
	SpAmount: [16, 16, 16, 16, 16, 16, 16, 16, 16, 16],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STEALCOIN, 4]]
};
SkillInfo[SK.RG_TUNNELDRIVE] = {
	Name: 'RG_TUNNELDRIVE',
	SkillName: 'Stalk',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TF_HIDING, 1]]
};
SkillInfo[SK.RG_RAID] = {
	Name: 'RG_RAID',
	SkillName: 'Sightless Mind',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.RG_TUNNELDRIVE, 2],
		[SK.RG_BACKSTAP, 2]
	]
};
SkillInfo[SK.RG_STRIPWEAPON] = {
	Name: 'RG_STRIPWEAPON',
	SkillName: 'Divest Weapon',
	MaxLv: 5,
	SpAmount: [17, 19, 21, 23, 25],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STRIPARMOR, 5]]
};
SkillInfo[SK.RG_STRIPSHIELD] = {
	Name: 'RG_STRIPSHIELD',
	SkillName: 'Divest Shield',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STRIPHELM, 5]]
};
SkillInfo[SK.RG_STRIPARMOR] = {
	Name: 'RG_STRIPARMOR',
	SkillName: 'Divest Armor',
	MaxLv: 5,
	SpAmount: [17, 19, 21, 23, 25],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STRIPSHIELD, 5]]
};
SkillInfo[SK.RG_STRIPHELM] = {
	Name: 'RG_STRIPHELM',
	SkillName: 'Divest Helm',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_STEALCOIN, 2]]
};
SkillInfo[SK.RG_INTIMIDATE] = {
	Name: 'RG_INTIMIDATE',
	SkillName: 'Snatch',
	MaxLv: 5,
	SpAmount: [13, 16, 19, 22, 25],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.RG_BACKSTAP, 4],
		[SK.RG_RAID, 5]
	]
};
SkillInfo[SK.RG_GRAFFITI] = {
	Name: 'RG_GRAFFITI',
	SkillName: 'Scribble',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.RG_FLAGGRAFFITI, 5]]
};
SkillInfo[SK.GD_BATTLEORDER] = {
	Name: 'GD_BATTLEORDER',
	SkillName: 'Battle Command',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.RG_FLAGGRAFFITI] = {
	Name: 'RG_FLAGGRAFFITI',
	SkillName: 'Piece',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_CLEANER, 1]]
};
SkillInfo[SK.RG_CLEANER] = {
	Name: 'RG_CLEANER',
	SkillName: 'Remover',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.RG_GANGSTER, 1]]
};
SkillInfo[SK.RG_GANGSTER] = {
	Name: 'RG_GANGSTER',
	SkillName: 'Slyness',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.RG_STRIPSHIELD, 3]]
};
SkillInfo[SK.GD_ITEMEMERGENCYCALL] = {
	Name: 'GD_ITEMEMERGENCYCALL',
	SkillName: 'Faux Urgent Call',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.MG_COLDBOLT] = {
	Name: 'MG_COLDBOLT',
	SkillName: 'Cold Bolt',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.RG_COMPULSION] = {
	Name: 'RG_COMPULSION',
	SkillName: 'Haggle',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_GANGSTER, 1]]
};
SkillInfo[SK.DE_GPAIN] = {
	Name: 'DE_GPAIN',
	SkillName: 'G Pain',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MS_BOWLINGBASH] = {
	Name: 'MS_BOWLINGBASH',
	SkillName: 'Bowling Bash',
	MaxLv: 10,
	SpAmount: [13, 14, 15, 16, 17, 18, 19, 20, 21, 22],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
};
SkillInfo[SK.ML_DEFENDER] = {
	Name: 'ML_DEFENDER',
	SkillName: 'Defending Aura',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.RG_PLAGIARISM] = {
	Name: 'RG_PLAGIARISM',
	SkillName: 'Intimidate',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_INTIMIDATE, 5]]
};
SkillInfo[SK.SR_DRAGONCOMBO] = {
	Name: 'SR_DRAGONCOMBO',
	SkillName: 'Dragon Combo',
	MaxLv: 10,
	SpAmount: [3, 4, 5, 6, 7, 8, 9, 10, 11, 12],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.MO_TRIPLEATTACK, 5]]
};
SkillInfo[SK.SC_STRIPACCESSARY] = {
	Name: 'SC_STRIPACCESSARY',
	SkillName: 'Divest Accessory ',
	MaxLv: 5,
	SpAmount: [15, 18, 21, 24, 27],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.RG_STRIPWEAPON, 1]]
};
SkillInfo[SK.GD_GLORYWOUNDS] = {
	Name: 'GD_GLORYWOUNDS',
	SkillName: 'Glorious Wounds',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.AM_AXEMASTERY] = {
	Name: 'AM_AXEMASTERY',
	SkillName: 'Axe Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GD_GUARDUP] = {
	Name: 'GD_GUARDUP',
	SkillName: 'Strengthen Guardians',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.GD_APPROVAL] = {
	Name: 'GD_APPROVAL',
	SkillName: 'Official Guild Approval',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MER_INCAGI] = {
	Name: 'MER_INCAGI',
	SkillName: 'Increase Agility',
	MaxLv: 10,
	SpAmount: [18, 21, 24, 27, 30, 33, 36, 39, 42, 45],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.AM_LEARNINGPOTION] = {
	Name: 'AM_LEARNINGPOTION',
	SkillName: 'Potion Research',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MER_BLESSING] = {
	Name: 'MER_BLESSING',
	SkillName: 'Blessing',
	MaxLv: 10,
	SpAmount: [28, 32, 36, 40, 44, 48, 52, 56, 60, 64],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MER_KYRIE] = {
	Name: 'MER_KYRIE',
	SkillName: 'Kyrie Eleison',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 25, 25, 25, 30, 30, 30, 35],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.EL_STONE_SHIELD] = {
	Name: 'EL_STONE_SHIELD',
	SkillName: 'Stone Shield',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AM_PHARMACY] = {
	Name: 'AM_PHARMACY',
	SkillName: 'Prepare Potion',
	MaxLv: 10,
	SpAmount: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_LEARNINGPOTION, 5]]
};
SkillInfo[SK.MER_ESTIMATION] = {
	Name: 'MER_ESTIMATION',
	SkillName: 'Sense',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.MER_LEXDIVINA] = {
	Name: 'MER_LEXDIVINA',
	SkillName: 'Lex Divina',
	MaxLv: 10,
	SpAmount: [20, 20, 20, 20, 20, 18, 16, 14, 12, 10],
	bSeperateLv: false,
	AttackRange: [5, 5, 5, 5, 5, 5, 5, 5, 5, 5]
};
SkillInfo[SK.MER_SCAPEGOAT] = {
	Name: 'MER_SCAPEGOAT',
	SkillName: 'Scapegoat',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AM_DEMONSTRATION] = {
	Name: 'AM_DEMONSTRATION',
	SkillName: 'Bomb',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AM_PHARMACY, 4]]
};
SkillInfo[SK.MER_PROVOKE] = {
	Name: 'MER_PROVOKE',
	SkillName: 'Provoke',
	MaxLv: 10,
	SpAmount: [4, 5, 6, 7, 8, 9, 10, 11, 12, 13],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MER_CRASH] = {
	Name: 'MER_CRASH',
	SkillName: 'Crash',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MER_SIGHT] = {
	Name: 'MER_SIGHT',
	SkillName: 'Sight',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AM_ACIDTERROR] = {
	Name: 'AM_ACIDTERROR',
	SkillName: 'Acid Terror',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AM_PHARMACY, 5]]
};
SkillInfo[SK.LG_SHIELDPRESS] = {
	Name: 'LG_SHIELDPRESS',
	SkillName: 'Shield Press',
	MaxLv: 10,
	SpAmount: [10, 12, 14, 16, 18, 20, 22, 24, 26, 28],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_SHIELDCHARGE, 3]]
};
SkillInfo[SK.ML_AUTOGUARD] = {
	Name: 'ML_AUTOGUARD',
	SkillName: 'Guard',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ML_PIERCE] = {
	Name: 'ML_PIERCE',
	SkillName: 'Pierce',
	MaxLv: 10,
	SpAmount: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
};
SkillInfo[SK.AM_POTIONPITCHER] = {
	Name: 'AM_POTIONPITCHER',
	SkillName: 'Aid Potion',
	MaxLv: 5,
	SpAmount: [1, 1, 1, 1, 1],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AM_PHARMACY, 3]]
};
SkillInfo[SK.MA_FREEZINGTRAP] = {
	Name: 'MA_FREEZINGTRAP',
	SkillName: 'Freezing Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.MA_SKIDTRAP] = {
	Name: 'MA_SKIDTRAP',
	SkillName: 'Skid Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3]
};
SkillInfo[SK.MA_SHOWER] = {
	Name: 'MA_SHOWER',
	SkillName: 'Arrow Shower',
	MaxLv: 10,
	SpAmount: [15, 15, 15, 15, 15, 15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.AM_CANNIBALIZE] = {
	Name: 'AM_CANNIBALIZE',
	SkillName: 'Summon Flora',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 4, 4],
	_NeedSkillList: [[SK.AM_PHARMACY, 6]]
};
SkillInfo[SK.MA_DOUBLE] = {
	Name: 'MA_DOUBLE',
	SkillName: 'Double Strafe',
	MaxLv: 10,
	SpAmount: [12, 12, 12, 12, 12, 12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MS_BERSERK] = {
	Name: 'MS_BERSERK',
	SkillName: 'Frenzy',
	MaxLv: 1,
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MS_REFLECTSHIELD] = {
	Name: 'MS_REFLECTSHIELD',
	SkillName: 'Shield Reflect',
	MaxLv: 10,
	SpAmount: [35, 40, 45, 50, 55, 60, 65, 70, 75, 80],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.AM_SPHEREMINE] = {
	Name: 'AM_SPHEREMINE',
	SkillName: 'Summon Marine Sphere',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_PHARMACY, 2]]
};
SkillInfo[SK.MS_PARRYING] = {
	Name: 'MS_PARRYING',
	SkillName: 'Parry',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_PYROCLASTIC] = {
	Name: 'MH_PYROCLASTIC',
	SkillName: 'Pyroclastic',
	MaxLv: 10,
	SpAmount: [20, 28, 36, 44, 52, 56, 60, 64, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_GRANITIC_ARMOR] = {
	Name: 'MH_GRANITIC_ARMOR',
	SkillName: 'Granitic Armor',
	MaxLv: 5,
	SpAmount: [54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.AM_CP_WEAPON] = {
	Name: 'AM_CP_WEAPON',
	SkillName: 'Alchemical Weapon',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_CP_ARMOR, 3]]
};
SkillInfo[SK.MH_MAGMA_FLOW] = {
	Name: 'MH_MAGMA_FLOW',
	SkillName: 'Magma Flow',
	MaxLv: 5,
	SpAmount: [34, 38, 42, 46, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_BLAST] = {
	Name: 'EL_BLAST',
	SkillName: 'Blast Mine',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MH_TINDER_BREAKER] = {
	Name: 'MH_TINDER_BREAKER',
	SkillName: 'Tinder Breaker',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [3, 4, 5, 6, 7]
};
SkillInfo[SK.AM_CP_SHIELD] = {
	Name: 'AM_CP_SHIELD',
	SkillName: 'Synthesized Shield',
	MaxLv: 5,
	SpAmount: [25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_CP_HELM, 3]]
};
SkillInfo[SK.MH_HEILIGE_STANGE] = {
	Name: 'MH_HEILIGE_STANGE',
	SkillName: 'Heilage Stange',
	MaxLv: 10,
	SpAmount: [48, 54, 60, 66, 72, 78, 84, 90, 96, 102],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MH_GOLDENE_FERSE] = {
	Name: 'MH_GOLDENE_FERSE',
	SkillName: 'Goldene Ferse',
	MaxLv: 5,
	SpAmount: [60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.NPC_ALLHEAL] = {
	Name: 'NPC_ALLHEAL',
	SkillName: 'Full Heal',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.AM_CP_ARMOR] = {
	Name: 'AM_CP_ARMOR',
	SkillName: 'Synthetic Armor',
	MaxLv: 5,
	SpAmount: [25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_CP_SHIELD, 3]]
};
SkillInfo[SK.MH_SONIC_CRAW] = {
	Name: 'MH_SONIC_CRAW',
	SkillName: 'Sonic Claw',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_SILENT_BREEZE] = {
	Name: 'MH_SILENT_BREEZE',
	SkillName: 'Silent Breeze',
	MaxLv: 5,
	SpAmount: [45, 54, 63, 72, 81],
	bSeperateLv: true,
	AttackRange: [5, 5, 7, 7, 9]
};
SkillInfo[SK.AM_CP_HELM] = {
	Name: 'AM_CP_HELM',
	SkillName: 'Biochemical Helm',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_PHARMACY, 2]]
};
SkillInfo[SK.MH_ERASER_CUTTER] = {
	Name: 'MH_ERASER_CUTTER',
	SkillName: 'Eraser Cutter',
	MaxLv: 10,
	SpAmount: [25, 30, 35, 40, 45, 50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7]
};
SkillInfo[SK.MH_OVERED_BOOST] = {
	Name: 'MH_OVERED_BOOST',
	SkillName: 'Over Boost',
	MaxLv: 5,
	SpAmount: [70, 90, 110, 130, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MH_LIGHT_OF_REGENE] = {
	Name: 'MH_LIGHT_OF_REGENE',
	SkillName: 'Light of Regeneration',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.AM_BIOETHICS] = {
	Name: 'AM_BIOETHICS',
	SkillName: 'Bioethics',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MH_POISON_MIST] = {
	Name: 'MH_POISON_MIST',
	SkillName: 'Poison Mist',
	MaxLv: 5,
	SpAmount: [65, 75, 85, 95, 105],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5]
};
SkillInfo[SK.MH_SUMMON_LEGION] = {
	Name: 'MH_SUMMON_LEGION',
	SkillName: 'Summon Legion',
	MaxLv: 5,
	SpAmount: [60, 80, 100, 120, 140],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.HVAN_EXPLOSION] = {
	Name: 'HVAN_EXPLOSION',
	SkillName: 'Self-Destruction',
	MaxLv: 3,
	SpAmount: [1, 1, 1],
	bSeperateLv: true,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.AM_BIOTECHNOLOGY] = {
	Name: 'AM_BIOTECHNOLOGY',
	SkillName: 'Biotechnology',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SA_CREATECON] = {
	Name: 'SA_CREATECON',
	SkillName: 'Create Elemental Converter',
	MaxLv: 1,
	Type: 'Quest',
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.EL_WILD_STORM] = {
	Name: 'EL_WILD_STORM',
	SkillName: 'Wild Storm',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MG_FROSTDIVER] = {
	Name: 'MG_FROSTDIVER',
	SkillName: 'Frost Driver',
	MaxLv: 10,
	SpAmount: [25, 24, 23, 22, 21, 20, 19, 18, 17, 16],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_COLDBOLT, 5]]
};
SkillInfo[SK.AM_CREATECREATURE] = {
	Name: 'AM_CREATECREATURE',
	SkillName: 'Creature Creation',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HFLI_SBR44] = {
	Name: 'HFLI_SBR44',
	SkillName: 'S.B.R.44',
	MaxLv: 3,
	SpAmount: [1, 1, 1],
	bSeperateLv: true,
	AttackRange: [9, 9, 9]
};
SkillInfo[SK.HFLI_FLEET] = {
	Name: 'HFLI_FLEET',
	SkillName: 'Flitting',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HAMI_BLOODLUST] = {
	Name: 'HAMI_BLOODLUST',
	SkillName: 'Blood Lust',
	MaxLv: 3,
	SpAmount: [120, 120, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.AM_CULTIVATION] = {
	Name: 'AM_CULTIVATION',
	SkillName: 'Cultivation',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HAMI_SKIN] = {
	Name: 'HAMI_SKIN',
	SkillName: 'Adamantium Skin',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_CURSED_SOIL] = {
	Name: 'EL_CURSED_SOIL',
	SkillName: 'Cursed Soil',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.HLIF_CHANGE] = {
	Name: 'HLIF_CHANGE',
	SkillName: 'Mental Charge',
	MaxLv: 3,
	SpAmount: [100, 100, 100],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.AM_FLAMECONTROL] = {
	Name: 'AM_FLAMECONTROL',
	SkillName: 'Flame Control',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HLIF_AVOID] = {
	Name: 'HLIF_AVOID',
	SkillName: 'Urgent Escape',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.LG_OVERBRAND] = {
	Name: 'LG_OVERBRAND',
	SkillName: 'Overbrand',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.LG_MOONSLASHER, 3],
		[SK.LG_PINPOINTATTACK, 1]
	]
};
SkillInfo[SK.ALL_ODINS_RECALL] = {
	Name: 'ALL_ODINS_RECALL',
	SkillName: 'Call of Odin',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AM_CALLHOMUN] = {
	Name: 'AM_CALLHOMUN',
	SkillName: 'Call Homunculus',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AM_REST, 1]]
};
SkillInfo[SK.SR_RIDEINLIGHTNING] = {
	Name: 'SR_RIDEINLIGHTNING',
	SkillName: 'Lightning Ride',
	MaxLv: 5,
	SpAmount: [25, 30, 35, 40, 45],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.MO_FINGEROFFENSIVE, 3]]
};
SkillInfo[SK.SR_HOWLINGOFLION] = {
	Name: 'SR_HOWLINGOFLION',
	SkillName: "Lion's Howl",
	MaxLv: 5,
	SpAmount: [70, 70, 70, 70, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SR_RIDEINLIGHTNING, 3],
		[SK.SR_ASSIMILATEPOWER, 1]
	]
};
SkillInfo[SK.SR_TIGERCANNON] = {
	Name: 'SR_TIGERCANNON',
	SkillName: 'Tiger Cannon',
	MaxLv: 10,
	SpAmount: [30, 35, 40, 45, 50, 55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_FALLENEMPIRE, 3]]
};
SkillInfo[SK.AM_REST] = {
	Name: 'AM_REST',
	SkillName: 'Vaporize',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AM_BIOETHICS, 1]]
};
SkillInfo[SK.GN_CHANGEMATERIAL] = {
	Name: 'GN_CHANGEMATERIAL',
	SkillName: 'Change Material',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GN_SLINGITEM] = {
	Name: 'GN_SLINGITEM',
	SkillName: 'Item Sling',
	MaxLv: 1,
	SpAmount: [4],
	bSeperateLv: false,
	AttackRange: [11],
	_NeedSkillList: [[SK.GN_CHANGEMATERIAL, 1]]
};
SkillInfo[SK.GN_MANDRAGORA] = {
	Name: 'GN_MANDRAGORA',
	SkillName: 'Mandragora Howl',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GN_HELLS_PLANT, 3]]
};
SkillInfo[SK.AM_DRILLMASTER] = {
	Name: 'AM_DRILLMASTER',
	SkillName: 'Drillmaster',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.GN_HELLS_PLANT] = {
	Name: 'GN_HELLS_PLANT',
	SkillName: 'Hell Plant',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GN_BLOOD_SUCKER, 3]]
};
SkillInfo[SK.GN_FIRE_EXPANSION] = {
	Name: 'GN_FIRE_EXPANSION',
	SkillName: 'Fire Expansion',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GN_DEMONIC_FIRE, 3]]
};
SkillInfo[SK.GN_DEMONIC_FIRE] = {
	Name: 'GN_DEMONIC_FIRE',
	SkillName: 'Demonic Fire ',
	MaxLv: 5,
	SpAmount: [24, 28, 32, 36, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.GN_SPORE_EXPLOSION, 3]]
};
SkillInfo[SK.AM_HEALHOMUN] = {
	Name: 'AM_HEALHOMUN',
	SkillName: 'Heal Homunculus',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.GN_WALLOFTHORN] = {
	Name: 'GN_WALLOFTHORN',
	SkillName: 'Thorn Wall',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.GN_THORNS_TRAP, 3]]
};
SkillInfo[SK.SR_CRESCENTELBOW] = {
	Name: 'SR_CRESCENTELBOW',
	SkillName: 'Crescent Elbow',
	MaxLv: 5,
	SpAmount: [80, 80, 80, 80, 80],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_WINDMILL, 1]]
};
SkillInfo[SK.GN_CARTBOOST] = {
	Name: 'GN_CARTBOOST',
	SkillName: 'Geneticist Cart Boost',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GN_REMODELING_CART, 3]]
};
SkillInfo[SK.AM_RESURRECTHOMUN] = {
	Name: 'AM_RESURRECTHOMUN',
	SkillName: 'Homunculus Resurrection',
	MaxLv: 5,
	SpAmount: [74, 68, 62, 56, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AM_CALLHOMUN, 1]]
};
SkillInfo[SK.GN_CARTCANNON] = {
	Name: 'GN_CARTCANNON',
	SkillName: 'Cart Cannon',
	MaxLv: 5,
	SpAmount: [40, 42, 46, 48, 50],
	bSeperateLv: true,
	AttackRange: [7, 8, 9, 10, 11],
	_NeedSkillList: [[SK.GN_REMODELING_CART, 2]]
};
SkillInfo[SK.GN_CART_TORNADO] = {
	Name: 'GN_CART_TORNADO',
	SkillName: 'Cart Tornado',
	MaxLv: 10,
	SpAmount: [30, 30, 30, 30, 30, 30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GN_REMODELING_CART, 1]]
};
SkillInfo[SK.GN_TRAINING_SWORD] = {
	Name: 'GN_TRAINING_SWORD',
	SkillName: 'Sword Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.CR_TRUST] = {
	Name: 'CR_TRUST',
	SkillName: 'Faith',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_WATER_SCREW_ATK] = {
	Name: 'EL_WATER_SCREW_ATK',
	SkillName: 'Water Screw Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.EL_WATER_SCREW] = {
	Name: 'EL_WATER_SCREW',
	SkillName: 'Water Screw',
	MaxLv: 1,
	SpAmount: [60],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.EL_ICE_NEEDLE] = {
	Name: 'EL_ICE_NEEDLE',
	SkillName: 'Ice Needle',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.CR_AUTOGUARD] = {
	Name: 'CR_AUTOGUARD',
	SkillName: 'Guard',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_FIRE_WAVE_ATK] = {
	Name: 'EL_FIRE_WAVE_ATK',
	SkillName: 'Fire Wave Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.EL_FIRE_WAVE] = {
	Name: 'EL_FIRE_WAVE',
	SkillName: 'Fire Wave',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.EL_FIRE_BOMB_ATK] = {
	Name: 'EL_FIRE_BOMB_ATK',
	SkillName: 'Fire Bomb Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [6]
};
SkillInfo[SK.CR_SHIELDCHARGE] = {
	Name: 'CR_SHIELDCHARGE',
	SkillName: 'Smite',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.CR_AUTOGUARD, 5]]
};
SkillInfo[SK.EL_UPHEAVAL] = {
	Name: 'EL_UPHEAVAL',
	SkillName: 'Upheaval',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.HAMI_DEFENCE] = {
	Name: 'HAMI_DEFENCE',
	SkillName: 'Amistr Bulwark',
	MaxLv: 5,
	SpAmount: [20, 25, 30, 35, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.HVAN_CHAOTIC] = {
	Name: 'HVAN_CHAOTIC',
	SkillName: 'Chaotic Blessings',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.CR_SHIELDBOOMERANG] = {
	Name: 'CR_SHIELDBOOMERANG',
	SkillName: 'Shield Boomerang',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [3, 5, 7, 9, 11],
	_NeedSkillList: [[SK.CR_SHIELDCHARGE, 3]]
};
SkillInfo[SK.MH_MIDNIGHT_FRENZY] = {
	Name: 'MH_MIDNIGHT_FRENZY',
	SkillName: 'Midnight Frenzy',
	MaxLv: 10,
	SpAmount: [18, 21, 24, 27, 30, 33, 36, 39, 42, 45],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3, 3, 3, 3, 3, 3]
};
SkillInfo[SK.MH_EQC] = {
	Name: 'MH_EQC',
	SkillName: 'E.Q.C : Eternal Quick Combo',
	MaxLv: 5,
	SpAmount: [24, 28, 32, 36, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.EL_GUST] = {
	Name: 'EL_GUST',
	SkillName: 'Gust',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.CR_REFLECTSHIELD] = {
	Name: 'CR_REFLECTSHIELD',
	SkillName: 'Shield Reflect',
	MaxLv: 10,
	SpAmount: [35, 40, 45, 50, 55, 60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_SHIELDBOOMERANG, 3]]
};
SkillInfo[SK.EL_CHILLY_AIR] = {
	Name: 'EL_CHILLY_AIR',
	SkillName: 'Chilly Air',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.EL_COOLER] = {
	Name: 'EL_COOLER',
	SkillName: 'Cooler',
	MaxLv: 0,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: []
};
SkillInfo[SK.GD_REGENERATION] = {
	Name: 'GD_REGENERATION',
	SkillName: 'Regeneration',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.CR_HOLYCROSS] = {
	Name: 'CR_HOLYCROSS',
	SkillName: 'Holy Cross',
	MaxLv: 10,
	SpAmount: [11, 12, 13, 14, 15, 16, 17, 18, 19, 20],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.CR_TRUST, 7]]
};
SkillInfo[SK.SO_CLOUD_KILL] = {
	Name: 'SO_CLOUD_KILL',
	SkillName: 'Killing Cloud',
	MaxLv: 5,
	SpAmount: [48, 56, 64, 70, 78],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WZ_HEAVENDRIVE, 5]]
};
SkillInfo[SK.EL_AQUAPLAY] = {
	Name: 'EL_AQUAPLAY',
	SkillName: 'Aquaplay',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SO_EL_ACTION] = {
	Name: 'SO_EL_ACTION',
	SkillName: 'Elemental Action',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [5],
	_NeedSkillList: [[SK.SO_EL_CONTROL, 3]]
};
SkillInfo[SK.CR_GRANDCROSS] = {
	Name: 'CR_GRANDCROSS',
	SkillName: 'Grand Cross',
	MaxLv: 10,
	SpAmount: [37, 44, 51, 58, 65, 72, 78, 86, 93, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.CR_TRUST, 10],
		[SK.CR_HOLYCROSS, 6]
	]
};
SkillInfo[SK.SO_WATER_INSIGNIA] = {
	Name: 'SO_WATER_INSIGNIA',
	SkillName: 'Water Insignia',
	MaxLv: 3,
	SpAmount: [22, 30, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SO_SUMMON_AQUA, 3]]
};
SkillInfo[SK.SR_RAISINGDRAGON] = {
	Name: 'SR_RAISINGDRAGON',
	SkillName: 'Rising Dragon',
	MaxLv: 10,
	SpAmount: [120, 120, 120, 120, 120, 120, 120, 120, 120, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MO_CALLSPIRITS, 5],
		[SK.SR_POWERVELOCITY, 1]
	]
};
SkillInfo[SK.SR_POWERVELOCITY] = {
	Name: 'SR_POWERVELOCITY',
	SkillName: 'Power Implantation',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [[SK.MO_CALLSPIRITS, 5]]
};
SkillInfo[SK.CR_DEVOTION] = {
	Name: 'CR_DEVOTION',
	SkillName: 'Sacrifice',
	MaxLv: 5,
	SpAmount: [25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [7, 8, 9, 10, 11],
	_NeedSkillList: [
		[SK.CR_GRANDCROSS, 4],
		[SK.CR_REFLECTSHIELD, 5]
	]
};
SkillInfo[SK.SO_SUMMON_AQUA] = {
	Name: 'SO_SUMMON_AQUA',
	SkillName: 'Call Aqua',
	MaxLv: 3,
	SpAmount: [100, 150, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SO_EL_CONTROL, 1],
		[SK.SO_DIAMONDDUST, 3]
	]
};
SkillInfo[SK.NV_BASIC] = {
	Name: 'NV_BASIC',
	SkillName: 'Basic Skill',
	MaxLv: 9,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MG_STONECURSE] = {
	Name: 'MG_STONECURSE',
	SkillName: 'Stone Curse',
	MaxLv: 10,
	SpAmount: [25, 24, 23, 22, 21, 20, 19, 18, 17, 16],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2]
};
SkillInfo[SK.CR_PROVIDENCE] = {
	Name: 'CR_PROVIDENCE',
	SkillName: 'Resistant Souls',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AL_DP, 5],
		[SK.AL_HEAL, 5]
	]
};
SkillInfo[SK.AB_EUCHARISTICA] = {
	Name: 'AB_EUCHARISTICA',
	SkillName: 'Eucharistica',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AB_EXPIATIO, 1],
		[SK.AB_EPICLESIS, 1]
	]
};
SkillInfo[SK.CR_DEFENDER] = {
	Name: 'CR_DEFENDER',
	SkillName: 'Defending Aura',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_SHIELDBOOMERANG, 1]]
};
SkillInfo[SK.AB_SILENTIUM] = {
	Name: 'AB_SILENTIUM',
	SkillName: 'Silentium',
	MaxLv: 5,
	SpAmount: [64, 68, 72, 76, 80],
	bSeperateLv: true,
	AttackRange: [4, 5, 6, 7, 8],
	_NeedSkillList: [[SK.AB_CLEARANCE, 1]]
};
SkillInfo[SK.CR_SPEARQUICKEN] = {
	Name: 'CR_SPEARQUICKEN',
	SkillName: 'Spear Quicken',
	MaxLv: 10,
	SpAmount: [24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KN_SPEARMASTERY, 10]]
};
SkillInfo[SK.SO_SUMMON_TERA] = {
	Name: 'SO_SUMMON_TERA',
	SkillName: 'Call Tera',
	MaxLv: 3,
	SpAmount: [100, 150, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SO_EL_CONTROL, 1],
		[SK.SO_EARTHGRAVE, 3]
	]
};
SkillInfo[SK.MO_IRONHAND] = {
	Name: 'MO_IRONHAND',
	SkillName: 'Iron Fists',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AL_DEMONBANE, 10],
		[SK.AL_DP, 10]
	]
};
SkillInfo[SK.SO_SUMMON_VENTUS] = {
	Name: 'SO_SUMMON_VENTUS',
	SkillName: 'Call Ventus',
	MaxLv: 3,
	SpAmount: [100, 150, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SO_EL_CONTROL, 1],
		[SK.SO_VARETYR_SPEAR, 3]
	]
};
SkillInfo[SK.MO_SPIRITSRECOVERY] = {
	Name: 'MO_SPIRITSRECOVERY',
	SkillName: 'Spiritual Cadence',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_BLADESTOP, 2]]
};
SkillInfo[SK.SO_EL_ANALYSIS] = {
	Name: 'SO_EL_ANALYSIS',
	SkillName: 'Analyze Element',
	MaxLv: 2,
	SpAmount: [10, 20],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [
		[SK.SA_FLAMELAUNCHER, 1],
		[SK.SA_FROSTWEAPON, 1],
		[SK.SA_LIGHTNINGLOADER, 1],
		[SK.SA_SEISMICWEAPON, 1]
	]
};
SkillInfo[SK.MO_CALLSPIRITS] = {
	Name: 'MO_CALLSPIRITS',
	SkillName: 'Summon Spirit Sphere',
	MaxLv: 5,
	SpAmount: [8, 8, 8, 8, 8],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_IRONHAND, 2]]
};
SkillInfo[SK.SO_VARETYR_SPEAR] = {
	Name: 'SO_VARETYR_SPEAR',
	SkillName: 'Varetyr Spear',
	MaxLv: 10,
	SpAmount: [65, 70, 75, 80, 85, 90, 95, 100, 105, 110],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SA_SEISMICWEAPON, 1],
		[SK.SA_VIOLENTGALE, 4]
	]
};
SkillInfo[SK.MO_ABSORBSPIRITS] = {
	Name: 'MO_ABSORBSPIRITS',
	SkillName: 'Spiritual Sphere Absorption',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.MO_CALLSPIRITS, 5]]
};
SkillInfo[SK.SO_VACUUM_EXTREME] = {
	Name: 'SO_VACUUM_EXTREME',
	SkillName: 'Extreme Vacuum',
	MaxLv: 5,
	SpAmount: [34, 42, 50, 58, 66],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SA_LANDPROTECTOR, 2]]
};
SkillInfo[SK.MO_TRIPLEATTACK] = {
	Name: 'MO_TRIPLEATTACK',
	SkillName: 'Raging Trifecta Blow',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_DODGE, 5]]
};
SkillInfo[SK.EL_POWER_OF_GAIA] = {
	Name: 'EL_POWER_OF_GAIA',
	SkillName: 'Power of Gaia',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MO_BODYRELOCATION] = {
	Name: 'MO_BODYRELOCATION',
	SkillName: 'Snap',
	MaxLv: 1,
	SpAmount: [14],
	bSeperateLv: false,
	AttackRange: [18],
	_NeedSkillList: [
		[SK.MO_SPIRITSRECOVERY, 2],
		[SK.MO_EXTREMITYFIST, 3],
		[SK.MO_STEELBODY, 3]
	]
};
SkillInfo[SK.SR_GENTLETOUCH_ENERGYGAIN] = {
	Name: 'SR_GENTLETOUCH_ENERGYGAIN',
	SkillName: 'Gentle Touch-Energy Gain',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_GENTLETOUCH_CURE, 1]]
};
SkillInfo[SK.MO_DODGE] = {
	Name: 'MO_DODGE',
	SkillName: 'Flee',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MO_IRONHAND, 5],
		[SK.MO_CALLSPIRITS, 5]
	]
};
SkillInfo[SK.SO_EARTHGRAVE] = {
	Name: 'SO_EARTHGRAVE',
	SkillName: 'Earth Grave',
	MaxLv: 5,
	SpAmount: [62, 70, 78, 86, 94],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WZ_EARTHSPIKE, 5]]
};
SkillInfo[SK.MO_INVESTIGATE] = {
	Name: 'MO_INVESTIGATE',
	SkillName: 'Occult Impaction',
	MaxLv: 5,
	SpAmount: [10, 14, 17, 19, 20],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.MO_CALLSPIRITS, 5]]
};
SkillInfo[SK.SO_SPELLFIST] = {
	Name: 'SO_SPELLFIST',
	SkillName: 'Spell Fist',
	MaxLv: 10,
	SpAmount: [40, 44, 48, 52, 56, 60, 64, 68, 72, 76],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_AUTOSPELL, 4]]
};
SkillInfo[SK.MO_FINGEROFFENSIVE] = {
	Name: 'MO_FINGEROFFENSIVE',
	SkillName: 'Throw Spirit Sphere',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MO_INVESTIGATE, 3]]
};
SkillInfo[SK.SO_ELECTRICWALK] = {
	Name: 'SO_ELECTRICWALK',
	SkillName: 'Electric Walk',
	MaxLv: 5,
	SpAmount: [30, 34, 38, 42, 46],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_VIOLENTGALE, 1]]
};
SkillInfo[SK.MO_STEELBODY] = {
	Name: 'MO_STEELBODY',
	SkillName: 'Mental Strength',
	MaxLv: 5,
	SpAmount: [200, 200, 200, 200, 200],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_COMBOFINISH, 3]]
};
SkillInfo[SK.WM_UNLIMITED_HUMMING_VOICE] = {
	Name: 'WM_UNLIMITED_HUMMING_VOICE',
	SkillName: 'Infinite Humming',
	MaxLv: 5,
	SpAmount: [120, 130, 140, 150, 160],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.WM_BEYOND_OF_WARCRY, 1],
		[SK.WM_SOUND_OF_DESTRUCTION, 1]
	]
};
SkillInfo[SK.MO_BLADESTOP] = {
	Name: 'MO_BLADESTOP',
	SkillName: 'Root',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_DODGE, 5]]
};
SkillInfo[SK.WA_SWING_DANCE] = {
	Name: 'WA_SWING_DANCE',
	SkillName: 'Swing Dance',
	MaxLv: 5,
	SpAmount: [96, 112, 128, 144, 160],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.MO_EXPLOSIONSPIRITS] = {
	Name: 'MO_EXPLOSIONSPIRITS',
	SkillName: 'Fury',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_ABSORBSPIRITS, 1]]
};
SkillInfo[SK.WM_SATURDAY_NIGHT_FEVER] = {
	Name: 'WM_SATURDAY_NIGHT_FEVER',
	SkillName: 'Saturday Night Fever',
	MaxLv: 5,
	SpAmount: [150, 160, 170, 180, 190],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_DANCE_WITH_WUG, 1]]
};
SkillInfo[SK.MO_EXTREMITYFIST] = {
	Name: 'MO_EXTREMITYFIST',
	SkillName: 'Guillotine Fist',
	MaxLv: 5,
	SpAmount: [1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.MO_EXPLOSIONSPIRITS, 3],
		[SK.MO_FINGEROFFENSIVE, 3]
	]
};
SkillInfo[SK.MG_FIREBALL] = {
	Name: 'MG_FIREBALL',
	SkillName: 'Fire Ball',
	MaxLv: 10,
	SpAmount: [25, 25, 25, 25, 25, 25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_FIREBOLT, 4]]
};
SkillInfo[SK.MO_CHAINCOMBO] = {
	Name: 'MO_CHAINCOMBO',
	SkillName: 'Raging Quadruple Blow',
	MaxLv: 5,
	SpAmount: [5, 6, 7, 8, 9],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.MO_TRIPLEATTACK, 5]]
};
SkillInfo[SK.WM_SOUND_OF_DESTRUCTION] = {
	Name: 'WM_SOUND_OF_DESTRUCTION',
	SkillName: 'Song of Destruction',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.WM_SATURDAY_NIGHT_FEVER, 3],
		[SK.WM_MELODYOFSINK, 3]
	]
};
SkillInfo[SK.MO_COMBOFINISH] = {
	Name: 'MO_COMBOFINISH',
	SkillName: 'Raging Thrust',
	MaxLv: 5,
	SpAmount: [3, 4, 5, 6, 7],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.MO_CHAINCOMBO, 3]]
};
SkillInfo[SK.WM_DANCE_WITH_WUG] = {
	Name: 'WM_DANCE_WITH_WUG',
	SkillName: 'Dances with Wargs',
	MaxLv: 5,
	SpAmount: [120, 140, 160, 180, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.MINSTREL]: [
			[SK.MI_HARMONIZE, 1],
			[SK.MI_RUSH_WINDMILL, 1],
			[SK.MI_ECHOSONG, 1]
		],
		[JobId.WANDERER]: [
			[SK.WA_SWING_DANCE, 1],
			[SK.WA_SYMPHONY_OF_LOVER, 1],
			[SK.WA_MOONLIT_SERENADE, 1]
		]
	}
};
SkillInfo[SK.SA_ADVANCEDBOOK] = {
	Name: 'SA_ADVANCEDBOOK',
	SkillName: 'Study',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_SONG_OF_MANA] = {
	Name: 'WM_SONG_OF_MANA',
	SkillName: 'Song Of Mana',
	MaxLv: 5,
	SpAmount: [120, 140, 160, 180, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.MINSTREL]: [
			[SK.MI_HARMONIZE, 1],
			[SK.MI_RUSH_WINDMILL, 1],
			[SK.MI_ECHOSONG, 1]
		],
		[JobId.WANDERER]: [
			[SK.WA_SWING_DANCE, 1],
			[SK.WA_SYMPHONY_OF_LOVER, 1],
			[SK.WA_MOONLIT_SERENADE, 1]
		]
	}
};
SkillInfo[SK.SA_CASTCANCEL] = {
	Name: 'SA_CASTCANCEL',
	SkillName: 'Cast Cancel',
	MaxLv: 5,
	SpAmount: [2, 2, 2, 2, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_ADVANCEDBOOK, 2]]
};
SkillInfo[SK.WL_WHITEIMPRISON] = {
	Name: 'WL_WHITEIMPRISON',
	SkillName: 'White Imprison',
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_SOULEXPANSION, 3]]
};
SkillInfo[SK.SA_MAGICROD] = {
	Name: 'SA_MAGICROD',
	SkillName: 'Magic Rod',
	MaxLv: 5,
	SpAmount: [2, 2, 2, 2, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_ADVANCEDBOOK, 4]]
};
SkillInfo[SK.WL_STASIS] = {
	Name: 'WL_STASIS',
	SkillName: 'Stasis',
	MaxLv: 5,
	SpAmount: [50, 60, 70, 80, 90],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_DRAINLIFE, 1]]
};
SkillInfo[SK.SA_SPELLBREAKER] = {
	Name: 'SA_SPELLBREAKER',
	SkillName: 'Spell Breaker',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SA_MAGICROD, 1]]
};
SkillInfo[SK.WL_TETRAVORTEX] = {
	Name: 'WL_TETRAVORTEX',
	SkillName: 'Tetra Vortex',
	MaxLv: 10,
	SpAmount: [120, 150, 180, 210, 240, 200, 240, 280, 320, 360],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.WL_CHAINLIGHTNING, 5],
		[SK.WL_HELLINFERNO, 5],
		[SK.WL_JACKFROST, 5],
		[SK.WL_EARTHSTRAIN, 5]
	]
};
SkillInfo[SK.SA_FREECAST] = {
	Name: 'SA_FREECAST',
	SkillName: 'Free Cast',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_CASTCANCEL, 1]]
};
SkillInfo[SK.WM_GREAT_ECHO] = {
	Name: 'WM_GREAT_ECHO',
	SkillName: 'Great Echo',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_METALICSOUND, 1]]
};
SkillInfo[SK.SA_AUTOSPELL] = {
	Name: 'SA_AUTOSPELL',
	SkillName: 'Hindsight',
	MaxLv: 10,
	SpAmount: [35, 35, 35, 35, 35, 35, 35, 35, 35, 35],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_FREECAST, 4]]
};
SkillInfo[SK.RA_ARROWSTORM] = {
	Name: 'RA_ARROWSTORM',
	SkillName: 'Arrow Storm',
	MaxLv: 10,
	SpAmount: [24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RA_AIMEDBOLT, 5]]
};
SkillInfo[SK.SA_FLAMELAUNCHER] = {
	Name: 'SA_FLAMELAUNCHER',
	SkillName: 'Endow Blaze',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_FIREBOLT, 1],
		[SK.SA_ADVANCEDBOOK, 5]
	]
};
SkillInfo[SK.RA_WUGRIDER] = {
	Name: 'RA_WUGRIDER',
	SkillName: 'Warg Ride',
	MaxLv: 3,
	SpAmount: [2, 2, 2],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.RA_WUGMASTERY, 1]]
};
SkillInfo[SK.SA_FROSTWEAPON] = {
	Name: 'SA_FROSTWEAPON',
	SkillName: 'Endow Tsunami',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_COLDBOLT, 1],
		[SK.SA_ADVANCEDBOOK, 5]
	]
};
SkillInfo[SK.RA_MAGENTATRAP] = {
	Name: 'RA_MAGENTATRAP',
	SkillName: 'Magenta Trap',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 1]]
};
SkillInfo[SK.SA_LIGHTNINGLOADER] = {
	Name: 'SA_LIGHTNINGLOADER',
	SkillName: 'Endow Tornado',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_LIGHTNINGBOLT, 1],
		[SK.SA_ADVANCEDBOOK, 5]
	]
};
SkillInfo[SK.NC_PILEBUNKER] = {
	Name: 'NC_PILEBUNKER',
	SkillName: 'Pile Bunker',
	MaxLv: 3,
	SpAmount: [50, 50, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [[SK.NC_BOOSTKNUCKLE, 2]]
};
SkillInfo[SK.SA_SEISMICWEAPON] = {
	Name: 'SA_SEISMICWEAPON',
	SkillName: 'Endow Quake',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_STONECURSE, 1],
		[SK.SA_ADVANCEDBOOK, 5]
	]
};
SkillInfo[SK.NC_B_SIDESLIDE] = {
	Name: 'NC_B_SIDESLIDE',
	SkillName: 'Back Slide',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NC_HOVERING, 1]]
};
SkillInfo[SK.SA_DRAGONOLOGY] = {
	Name: 'SA_DRAGONOLOGY',
	SkillName: 'Dragonology',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SA_ADVANCEDBOOK, 9]]
};
SkillInfo[SK.NC_NEUTRALBARRIER] = {
	Name: 'NC_NEUTRALBARRIER',
	SkillName: 'Neutral Barrier',
	MaxLv: 3,
	SpAmount: [80, 90, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.NC_MAGNETICFIELD, 2]]
};
SkillInfo[SK.SA_VOLCANO] = {
	Name: 'SA_VOLCANO',
	SkillName: 'Volcano',
	MaxLv: 5,
	SpAmount: [48, 46, 44, 42, 40],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.SA_FLAMELAUNCHER, 2]]
};
SkillInfo[SK.NC_SILVERSNIPER] = {
	Name: 'NC_SILVERSNIPER',
	SkillName: 'FAW Silver Sniper',
	MaxLv: 5,
	SpAmount: [25, 30, 35, 40, 45],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.NC_RESEARCHFE, 2]]
};
SkillInfo[SK.SA_DELUGE] = {
	Name: 'SA_DELUGE',
	SkillName: 'Deluge',
	MaxLv: 5,
	SpAmount: [48, 46, 44, 42, 40],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.SA_FROSTWEAPON, 2]]
};
SkillInfo[SK.SC_BODYPAINT] = {
	Name: 'SC_BODYPAINT',
	SkillName: 'Body Painting',
	MaxLv: 5,
	SpAmount: [10, 15, 20, 25, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.SA_VIOLENTGALE] = {
	Name: 'SA_VIOLENTGALE',
	SkillName: 'Whirlwind',
	MaxLv: 5,
	SpAmount: [48, 46, 44, 42, 40],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.SA_LIGHTNINGLOADER, 2]]
};
SkillInfo[SK.MG_FIREWALL] = {
	Name: 'MG_FIREWALL',
	SkillName: 'Fire Wall',
	MaxLv: 10,
	SpAmount: [40, 40, 40, 40, 40, 40, 40, 40, 40, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.MG_SIGHT, 1],
		[SK.MG_FIREBALL, 5]
	]
};
SkillInfo[SK.SA_LANDPROTECTOR] = {
	Name: 'SA_LANDPROTECTOR',
	SkillName: 'Land Protector',
	MaxLv: 5,
	SpAmount: [66, 62, 58, 54, 50],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SA_DELUGE, 3],
		[SK.SA_VIOLENTGALE, 3],
		[SK.SA_VOLCANO, 3]
	]
};
SkillInfo[SK.WM_GLOOMYDAY] = {
	Name: 'WM_GLOOMYDAY',
	SkillName: 'Gloomy Shyness',
	MaxLv: 5,
	SpAmount: [42, 46, 50, 54, 58],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_RANDOMIZESPELL, 1]]
};
SkillInfo[SK.SA_DISPELL] = {
	Name: 'SA_DISPELL',
	SkillName: 'Dispell',
	MaxLv: 5,
	SpAmount: [1, 1, 1, 1, 1],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SA_SPELLBREAKER, 3]]
};
SkillInfo[SK.LG_FORCEOFVANGUARD] = {
	Name: 'LG_FORCEOFVANGUARD',
	SkillName: 'Vanguard Force',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.SA_ABRACADABRA] = {
	Name: 'SA_ABRACADABRA',
	SkillName: 'Hocus-pocus',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SA_AUTOSPELL, 5],
		[SK.SA_DISPELL, 1],
		[SK.SA_LANDPROTECTOR, 1]
	]
};
SkillInfo[SK.LG_RAYOFGENESIS] = {
	Name: 'LG_RAYOFGENESIS',
	SkillName: 'Genesis Ray',
	MaxLv: 10,
	SpAmount: [30, 40, 50, 60, 70, 80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_GRANDCROSS, 5]]
};
SkillInfo[SK.SA_MONOCELL] = {
	Name: 'SA_MONOCELL',
	SkillName: 'Mono Cell',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.SR_FALLENEMPIRE] = {
	Name: 'SR_FALLENEMPIRE',
	SkillName: 'Fallen Empire',
	MaxLv: 10,
	SpAmount: [18, 21, 24, 27, 30, 33, 36, 39, 42, 45],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.SR_DRAGONCOMBO, 3]]
};
SkillInfo[SK.SA_CLASSCHANGE] = {
	Name: 'SA_CLASSCHANGE',
	SkillName: 'Class Change',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.SR_WINDMILL] = {
	Name: 'SR_WINDMILL',
	SkillName: 'Windmill',
	MaxLv: 1,
	SpAmount: [45],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SR_CURSEDCIRCLE, 1]]
};
SkillInfo[SK.SA_SUMMONMONSTER] = {
	Name: 'SA_SUMMONMONSTER',
	SkillName: 'Monster Chant',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SR_GENTLETOUCH_CURE] = {
	Name: 'SR_GENTLETOUCH_CURE',
	SkillName: 'Gentle Touch-Cure',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.SR_GENTLETOUCH_QUIET, 1]]
};
SkillInfo[SK.SA_REVERSEORCISH] = {
	Name: 'SA_REVERSEORCISH',
	SkillName: 'Grampus Morph',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_LULLABY_DEEPSLEEP] = {
	Name: 'WM_LULLABY_DEEPSLEEP',
	SkillName: 'Deep Sleep Lullaby',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LESSON, 1]]
};
SkillInfo[SK.SA_DEATH] = {
	Name: 'SA_DEATH',
	SkillName: 'Grim Reaper',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.WM_DEADHILLHERE] = {
	Name: 'WM_DEADHILLHERE',
	SkillName: 'Death Valley',
	MaxLv: 5,
	SpAmount: [50, 53, 56, 59, 62],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.WM_SIRCLEOFNATURE, 3]]
};
SkillInfo[SK.SA_FORTUNE] = {
	Name: 'SA_FORTUNE',
	SkillName: 'Gold Digger',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.WM_SEVERE_RAINSTORM] = {
	Name: 'WM_SEVERE_RAINSTORM',
	SkillName: 'Severe Rainstorm',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	NeedSkillList: {
		[JobId.MINSTREL]: [[SK.BA_MUSICALSTRIKE, 5]],
		[JobId.WANDERER]: [[SK.DC_THROWARROW, 5]]
	}
};
SkillInfo[SK.SA_TAMINGMONSTER] = {
	Name: 'SA_TAMINGMONSTER',
	SkillName: 'Beastly Hypnosis',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.MI_RUSH_WINDMILL] = {
	Name: 'MI_RUSH_WINDMILL',
	SkillName: 'Windmill Rush',
	MaxLv: 5,
	SpAmount: [82, 88, 94, 100, 106],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.SA_QUESTION] = {
	Name: 'SA_QUESTION',
	SkillName: 'Questioning',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_REVERBERATION] = {
	Name: 'WM_REVERBERATION',
	SkillName: 'Reverberation',
	MaxLv: 5,
	SpAmount: [56, 62, 68, 74, 80],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	NeedSkillList: {
		[JobId.MINSTREL]: [[SK.BA_DISSONANCE, 5]],
		[JobId.WANDERER]: [[SK.DC_UGLYDANCE, 5]]
	}
};
SkillInfo[SK.SA_GRAVITY] = {
	Name: 'SA_GRAVITY',
	SkillName: 'Gravity',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_METALICSOUND] = {
	Name: 'WM_METALICSOUND',
	SkillName: 'Metallic Sound',
	MaxLv: 10,
	SpAmount: [62, 64, 66, 68, 70, 72, 74, 76, 78, 80],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_REVERBERATION, 5]]
};
SkillInfo[SK.SA_LEVELUP] = {
	Name: 'SA_LEVELUP',
	SkillName: 'Leveling',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_LESSON] = {
	Name: 'WM_LESSON',
	SkillName: 'Voice Lessons',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SA_INSTANTDEATH] = {
	Name: 'SA_INSTANTDEATH',
	SkillName: 'Suicide',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MI_ECHOSONG] = {
	Name: 'MI_ECHOSONG',
	SkillName: 'Echo Song',
	MaxLv: 5,
	SpAmount: [86, 92, 98, 104, 110],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LULLABY_DEEPSLEEP, 1]]
};
SkillInfo[SK.SA_FULLRECOVERY] = {
	Name: 'SA_FULLRECOVERY',
	SkillName: 'Rejuvenation',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.WM_DOMINION_IMPULSE] = {
	Name: 'WM_DOMINION_IMPULSE',
	SkillName: 'Dominion Impulse',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [11],
	_NeedSkillList: [[SK.WM_REVERBERATION, 1]]
};
SkillInfo[SK.SA_COMA] = {
	Name: 'SA_COMA',
	SkillName: 'Coma',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MG_FIREBOLT] = {
	Name: 'MG_FIREBOLT',
	SkillName: 'Fire Bolt',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.BD_ADAPTATION] = {
	Name: 'BD_ADAPTATION',
	SkillName: 'Amp',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WM_BEYOND_OF_WARCRY] = {
	Name: 'WM_BEYOND_OF_WARCRY',
	SkillName: 'Warcry from Beyond',
	MaxLv: 5,
	SpAmount: [120, 130, 140, 150, 160],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LERADS_DEW, 1]]
};
SkillInfo[SK.BD_ENCORE] = {
	Name: 'BD_ENCORE',
	SkillName: 'Encore',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.BD_ADAPTATION, 1]]
};
SkillInfo[SK.SR_GENTLETOUCH_REVITALIZE] = {
	Name: 'SR_GENTLETOUCH_REVITALIZE',
	SkillName: 'Gentle Touch-Revitalize',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SR_GENTLETOUCH_QUIET, 1],
		[SK.SR_GENTLETOUCH_CURE, 1],
		[SK.SR_GENTLETOUCH_ENERGYGAIN, 3]
	]
};
SkillInfo[SK.BD_LULLABY] = {
	Name: 'BD_LULLABY',
	SkillName: 'Lullaby',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1],
	NeedSkillList: { [JobId.BARD]: [[SK.BA_WHISTLE, 10]], [JobId.DANCER]: [[SK.DC_HUMMING, 10]] }
};
SkillInfo[SK.SO_PSYCHIC_WAVE] = {
	Name: 'SO_PSYCHIC_WAVE',
	SkillName: 'Psychic Wave',
	MaxLv: 5,
	SpAmount: [48, 56, 64, 70, 78],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SA_DISPELL, 1]]
};
SkillInfo[SK.BD_RICHMANKIM] = {
	Name: 'BD_RICHMANKIM',
	SkillName: 'Mental Sensing',
	MaxLv: 5,
	SpAmount: [62, 68, 74, 80, 86],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BD_SIEGFRIED, 3]]
};
SkillInfo[SK.SO_SUMMON_AGNI] = {
	Name: 'SO_SUMMON_AGNI',
	SkillName: 'Call Agni',
	MaxLv: 3,
	SpAmount: [100, 150, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SO_EL_CONTROL, 1],
		[SK.SO_WARMER, 3]
	]
};
SkillInfo[SK.BD_ETERNALCHAOS] = {
	Name: 'BD_ETERNALCHAOS',
	SkillName: 'Down Tempo',
	MaxLv: 1,
	SpAmount: [120],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.BD_ROKISWEIL, 1]]
};
SkillInfo[SK.SO_FIRE_INSIGNIA] = {
	Name: 'SO_FIRE_INSIGNIA',
	SkillName: 'Fire Insignia',
	MaxLv: 3,
	SpAmount: [22, 30, 38],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [[SK.SO_SUMMON_AGNI, 3]]
};
SkillInfo[SK.BD_DRUMBATTLEFIELD] = {
	Name: 'BD_DRUMBATTLEFIELD',
	SkillName: 'Battle Theme',
	MaxLv: 5,
	SpAmount: [50, 54, 58, 62, 66],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: {
		[JobId.BARD]: [[SK.BA_APPLEIDUN, 10]],
		[JobId.DANCER]: [[SK.DC_SERVICEFORYOU, 10]]
	}
};
SkillInfo[SK.SR_CURSEDCIRCLE] = {
	Name: 'SR_CURSEDCIRCLE',
	SkillName: 'Cursed Circle',
	MaxLv: 5,
	SpAmount: [40, 60, 80, 100, 120],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MO_BLADESTOP, 2]]
};
SkillInfo[SK.BD_RINGNIBELUNGEN] = {
	Name: 'BD_RINGNIBELUNGEN',
	SkillName: 'Harmonic Lick',
	MaxLv: 5,
	SpAmount: [64, 60, 56, 52, 48],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BD_DRUMBATTLEFIELD, 3]]
};
SkillInfo[SK.GN_SPORE_EXPLOSION] = {
	Name: 'GN_SPORE_EXPLOSION',
	SkillName: 'Spore Explosion',
	MaxLv: 10,
	SpAmount: [48, 52, 56, 60, 64, 68, 72, 76, 80, 84],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 4]]
};
SkillInfo[SK.BD_ROKISWEIL] = {
	Name: 'BD_ROKISWEIL',
	SkillName: 'Classical Pluck',
	MaxLv: 1,
	SpAmount: [180],
	bSeperateLv: false,
	AttackRange: [1],
	NeedSkillList: {
		[JobId.BARD]: [[SK.BA_ASSASSINCROSS, 10]],
		[JobId.DANCER]: [[SK.DC_DONTFORGETME, 10]]
	}
};
SkillInfo[SK.SR_RAMPAGEBLASTER] = {
	Name: 'SR_RAMPAGEBLASTER',
	SkillName: 'Rampage Blast',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SR_EARTHSHAKER, 2]]
};
SkillInfo[SK.BD_INTOABYSS] = {
	Name: 'BD_INTOABYSS',
	SkillName: 'Power Cord',
	MaxLv: 1,
	SpAmount: [70],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.BD_LULLABY, 1]]
};
SkillInfo[SK.GN_S_PHARMACY] = {
	Name: 'GN_S_PHARMACY',
	SkillName: 'Special Pharmacy',
	MaxLv: 10,
	SpAmount: [12, 12, 12, 12, 12, 12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.BD_SIEGFRIED] = {
	Name: 'BD_SIEGFRIED',
	SkillName: 'Acoustic Rhythm',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	NeedSkillList: { [JobId.BARD]: [[SK.BA_POEMBRAGI, 10]], [JobId.DANCER]: [[SK.DC_FORTUNEKISS, 10]] }
};
SkillInfo[SK.GD_RESTORE] = {
	Name: 'GD_RESTORE',
	SkillName: 'Restoration',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.BD_RAGNAROK] = {
	Name: 'BD_RAGNAROK',
	SkillName: 'Ragnarok',
	MaxLv: 0,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: []
};
SkillInfo[SK.LG_INSPIRATION] = {
	Name: 'LG_INSPIRATION',
	SkillName: 'Inspiration',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.LG_PIETY, 5],
		[SK.LG_RAYOFGENESIS, 4],
		[SK.LG_SHIELDSPELL, 3]
	]
};
SkillInfo[SK.BA_MUSICALLESSON] = {
	Name: 'BA_MUSICALLESSON',
	SkillName: 'Music Lessons',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.LG_PIETY] = {
	Name: 'LG_PIETY',
	SkillName: 'Piety',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.CR_TRUST, 3]]
};
SkillInfo[SK.BA_MUSICALSTRIKE] = {
	Name: 'BA_MUSICALSTRIKE',
	SkillName: 'Melody Strike',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.BA_MUSICALLESSON, 3]]
};
SkillInfo[SK.LG_PRESTIGE] = {
	Name: 'LG_PRESTIGE',
	SkillName: 'Prestige',
	MaxLv: 5,
	SpAmount: [75, 80, 85, 90, 95],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.LG_TRAMPLE, 3]]
};
SkillInfo[SK.BA_DISSONANCE] = {
	Name: 'BA_DISSONANCE',
	SkillName: 'Unchained Serenade',
	MaxLv: 5,
	SpAmount: [35, 38, 41, 44, 47],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BD_ADAPTATION, 1],
		[SK.BA_MUSICALLESSON, 1]
	]
};
SkillInfo[SK.ALL_ODINS_POWER] = {
	Name: 'ALL_ODINS_POWER',
	SkillName: 'Power of Odin',
	MaxLv: 2,
	SpAmount: [70, 100],
	bSeperateLv: false,
	AttackRange: [9, 9]
};
SkillInfo[SK.BA_FROSTJOKE] = {
	Name: 'BA_FROSTJOKE',
	SkillName: 'Unbarring Octave',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.BD_ENCORE, 1]]
};
SkillInfo[SK.LG_EXEEDBREAK] = {
	Name: 'LG_EXEEDBREAK',
	SkillName: 'Exceed Break',
	MaxLv: 5,
	SpAmount: [20, 32, 44, 56, 68],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.LG_BANISHINGPOINT, 3]]
};
SkillInfo[SK.BA_WHISTLE] = {
	Name: 'BA_WHISTLE',
	SkillName: 'Perfect Tablature',
	MaxLv: 10,
	SpAmount: [22, 24, 26, 28, 30, 32, 34, 36, 38, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BA_DISSONANCE, 3]]
};
SkillInfo[SK.MG_LIGHTNINGBOLT] = {
	Name: 'MG_LIGHTNINGBOLT',
	SkillName: 'Lightning Bolt',
	MaxLv: 10,
	SpAmount: [12, 14, 16, 18, 20, 22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9]
};
SkillInfo[SK.BA_ASSASSINCROSS] = {
	Name: 'BA_ASSASSINCROSS',
	SkillName: 'Impressive Riff',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BA_DISSONANCE, 3]]
};
SkillInfo[SK.LG_RAGEBURST] = {
	Name: 'LG_RAGEBURST',
	SkillName: 'Burst Attack',
	MaxLv: 1,
	SpAmount: [150],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.LG_FORCEOFVANGUARD, 1]]
};
SkillInfo[SK.BA_POEMBRAGI] = {
	Name: 'BA_POEMBRAGI',
	SkillName: 'Magic Strings',
	MaxLv: 10,
	SpAmount: [65, 70, 75, 80, 85, 90, 95, 100, 105, 110],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BA_DISSONANCE, 3]]
};
SkillInfo[SK.ML_DEVOTION] = {
	Name: 'ML_DEVOTION',
	SkillName: 'Sacrifice',
	MaxLv: 5,
	SpAmount: [25, 25, 25, 25, 25],
	bSeperateLv: false,
	AttackRange: [7, 8, 9, 10, 11]
};
SkillInfo[SK.BA_APPLEIDUN] = {
	Name: 'BA_APPLEIDUN',
	SkillName: 'Song of Lutie',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BA_DISSONANCE, 3]]
};
SkillInfo[SK.LG_TRAMPLE] = {
	Name: 'LG_TRAMPLE',
	SkillName: 'Trample',
	MaxLv: 3,
	SpAmount: [30, 45, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1]
};
SkillInfo[SK.DC_DANCINGLESSON] = {
	Name: 'DC_DANCINGLESSON',
	SkillName: 'Dance Lessons',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SC_MAELSTROM] = {
	Name: 'SC_MAELSTROM',
	SkillName: 'Maelstrom',
	MaxLv: 3,
	SpAmount: [50, 55, 60],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [
		[SK.SC_CHAOSPANIC, 3],
		[SK.SC_UNLUCKY, 3]
	]
};
SkillInfo[SK.DC_THROWARROW] = {
	Name: 'DC_THROWARROW',
	SkillName: 'Slinging Arrow',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.DC_DANCINGLESSON, 3]]
};
SkillInfo[SK.SC_CHAOSPANIC] = {
	Name: 'SC_CHAOSPANIC',
	SkillName: 'Chaos Panic ',
	MaxLv: 3,
	SpAmount: [30, 36, 42],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [[SK.SC_MANHOLE, 1]]
};
SkillInfo[SK.DC_UGLYDANCE] = {
	Name: 'DC_UGLYDANCE',
	SkillName: 'Hip Shaker',
	MaxLv: 5,
	SpAmount: [35, 38, 41, 44, 47],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BD_ADAPTATION, 1],
		[SK.DC_DANCINGLESSON, 1]
	]
};
SkillInfo[SK.SC_DIMENSIONDOOR] = {
	Name: 'SC_DIMENSIONDOOR',
	SkillName: 'Dimensional Door',
	MaxLv: 3,
	SpAmount: [30, 36, 42],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [[SK.SC_MANHOLE, 1]]
};
SkillInfo[SK.DC_SCREAM] = {
	Name: 'DC_SCREAM',
	SkillName: 'Dazzler',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BD_ENCORE, 1]]
};
SkillInfo[SK.SC_MANHOLE] = {
	Name: 'SC_MANHOLE',
	SkillName: 'Manhole ',
	MaxLv: 3,
	SpAmount: [20, 25, 30],
	bSeperateLv: true,
	AttackRange: [7, 7, 7],
	_NeedSkillList: [[SK.RG_FLAGGRAFFITI, 1]]
};
SkillInfo[SK.DC_HUMMING] = {
	Name: 'DC_HUMMING',
	SkillName: 'Focus Ballet',
	MaxLv: 10,
	SpAmount: [33, 36, 39, 42, 45, 48, 51, 54, 57, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DC_UGLYDANCE, 3]]
};
SkillInfo[SK.EL_PYROTECHNIC] = {
	Name: 'EL_PYROTECHNIC',
	SkillName: 'Pyrotechnic',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DC_DONTFORGETME] = {
	Name: 'DC_DONTFORGETME',
	SkillName: 'Slow Grace',
	MaxLv: 10,
	SpAmount: [38, 41, 44, 47, 50, 53, 56, 59, 62, 65],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DC_UGLYDANCE, 3]]
};
SkillInfo[SK.SC_WEAKNESS] = {
	Name: 'SC_WEAKNESS',
	SkillName: 'Masquerade-Weakness',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [
		[SK.SC_ENERVATION, 1],
		[SK.SC_GROOMY, 1],
		[SK.SC_IGNORANCE, 1]
	]
};
SkillInfo[SK.DC_FORTUNEKISS] = {
	Name: 'DC_FORTUNEKISS',
	SkillName: 'Lady Luck',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DC_UGLYDANCE, 3]]
};
SkillInfo[SK.SC_UNLUCKY] = {
	Name: 'SC_UNLUCKY',
	SkillName: 'Masquerade-Unlucky',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [
		[SK.SC_LAZINESS, 1],
		[SK.SC_WEAKNESS, 1]
	]
};
SkillInfo[SK.DC_SERVICEFORYOU] = {
	Name: 'DC_SERVICEFORYOU',
	SkillName: "Gypsy's Kiss",
	MaxLv: 10,
	SpAmount: [60, 63, 66, 69, 72, 75, 78, 81, 84, 87],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DC_UGLYDANCE, 3]]
};
SkillInfo[SK.SC_IGNORANCE] = {
	Name: 'SC_IGNORANCE',
	SkillName: 'Masquerade-Ignorance',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [[SK.SC_BODYPAINT, 1]]
};
SkillInfo[SK.SC_GROOMY] = {
	Name: 'SC_GROOMY',
	SkillName: 'Masquerade-Gloomy',
	MaxLv: 3,
	SpAmount: [30, 40, 50],
	bSeperateLv: true,
	AttackRange: [3, 3, 3],
	_NeedSkillList: [[SK.SC_BODYPAINT, 1]]
};
SkillInfo[SK.SC_INVISIBILITY] = {
	Name: 'SC_INVISIBILITY',
	SkillName: 'Invisibility',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SC_UNLUCKY, 3],
		[SK.SC_AUTOSHADOWSPELL, 7],
		[SK.SC_DEADLYINFECT, 5]
	]
};
SkillInfo[SK.SC_AUTOSHADOWSPELL] = {
	Name: 'SC_AUTOSHADOWSPELL',
	SkillName: 'Shadow Spell',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SC_REPRODUCE, 5]]
};
SkillInfo[SK.SC_REPRODUCE] = {
	Name: 'SC_REPRODUCE',
	SkillName: 'Reproduce',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_PLAGIARISM, 5]]
};
SkillInfo[SK.SC_FATALMENACE] = {
	Name: 'SC_FATALMENACE',
	SkillName: 'Fatal Menace',
	MaxLv: 10,
	SpAmount: [10, 14, 18, 22, 26, 30, 34, 38, 42, 46],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RG_INTIMIDATE, 5]]
};
SkillInfo[SK.NC_MAGICDECOY] = {
	Name: 'NC_MAGICDECOY',
	SkillName: 'FAW Magic Decoy',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.NC_SILVERSNIPER, 2]]
};
SkillInfo[SK.WE_MALE] = {
	Name: 'WE_MALE',
	SkillName: 'Loving Touch',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.NC_AXEBOOMERANG] = {
	Name: 'NC_AXEBOOMERANG',
	SkillName: 'Axe Boomerang',
	MaxLv: 5,
	SpAmount: [20, 22, 24, 26, 28],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [[SK.NC_TRAININGAXE, 1]]
};
SkillInfo[SK.WE_FEMALE] = {
	Name: 'WE_FEMALE',
	SkillName: 'Undying Love',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.MG_THUNDERSTORM] = {
	Name: 'MG_THUNDERSTORM',
	SkillName: 'Thunder Storm',
	MaxLv: 10,
	SpAmount: [29, 34, 39, 44, 49, 54, 59, 64, 69, 74],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MG_LIGHTNINGBOLT, 4]]
};
SkillInfo[SK.WE_CALLPARTNER] = {
	Name: 'WE_CALLPARTNER',
	SkillName: 'Romantic Rendeavous!!',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.NC_RESEARCHFE] = {
	Name: 'NC_RESEARCHFE',
	SkillName: 'Fire Earth Research ',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.ITM_TOMAHAWK] = {
	Name: 'ITM_TOMAHAWK',
	SkillName: 'Tomahawk Throwing',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [9]
};
SkillInfo[SK.NC_STEALTHFIELD] = {
	Name: 'NC_STEALTHFIELD',
	SkillName: 'Stealth Field',
	MaxLv: 3,
	SpAmount: [80, 100, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.NC_ANALYZE, 3],
		[SK.NC_NEUTRALBARRIER, 2]
	]
};
SkillInfo[SK.NC_INFRAREDSCAN] = {
	Name: 'NC_INFRAREDSCAN',
	SkillName: 'Infrared Scan',
	MaxLv: 1,
	SpAmount: [45],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NC_SHAPESHIFT, 2]]
};
SkillInfo[SK.NC_EMERGENCYCOOL] = {
	Name: 'NC_EMERGENCYCOOL',
	SkillName: 'Cooldown',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NC_SELFDESTRUCTION, 2]]
};
SkillInfo[SK.NC_SHAPESHIFT] = {
	Name: 'NC_SHAPESHIFT',
	SkillName: 'Elemental Shift',
	MaxLv: 4,
	SpAmount: [100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.NC_MAINFRAME, 2]]
};
SkillInfo[SK.NC_SELFDESTRUCTION] = {
	Name: 'NC_SELFDESTRUCTION',
	SkillName: 'Suicidal Destruction',
	MaxLv: 3,
	SpAmount: [200, 200, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.NC_MAINFRAME, 2]]
};
SkillInfo[SK.NC_MAINFRAME] = {
	Name: 'NC_MAINFRAME',
	SkillName: 'Remodel Mainframe',
	MaxLv: 4,
	SpAmount: [0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.NC_MADOLICENCE, 4]]
};
SkillInfo[SK.NC_ACCELERATION] = {
	Name: 'NC_ACCELERATION',
	SkillName: 'Acceleration',
	MaxLv: 3,
	SpAmount: [20, 40, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.NC_MADOLICENCE, 1]]
};
SkillInfo[SK.NC_ARMSCANNON] = {
	Name: 'NC_ARMSCANNON',
	SkillName: 'Arm Cannon',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.NC_FLAMELAUNCHER, 2],
		[SK.NC_COLDSLOWER, 2]
	]
};
SkillInfo[SK.NC_VULCANARM] = {
	Name: 'NC_VULCANARM',
	SkillName: 'Vulcan Arm',
	MaxLv: 3,
	SpAmount: [9, 12, 15],
	bSeperateLv: true,
	AttackRange: [13, 13, 13],
	_NeedSkillList: [[SK.NC_BOOSTKNUCKLE, 2]]
};
SkillInfo[SK.RA_ICEBOUNDTRAP] = {
	Name: 'RA_ICEBOUNDTRAP',
	SkillName: 'Ice Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.RA_DETONATOR, 1]]
};
SkillInfo[SK.RA_FIRINGTRAP] = {
	Name: 'RA_FIRINGTRAP',
	SkillName: 'Fire Trap',
	MaxLv: 5,
	SpAmount: [10, 10, 10, 10, 10],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.RA_DETONATOR, 1]]
};
SkillInfo[SK.RA_VERDURETRAP] = {
	Name: 'RA_VERDURETRAP',
	SkillName: 'Verdure Trap',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 1]]
};
SkillInfo[SK.RA_COBALTTRAP] = {
	Name: 'RA_COBALTTRAP',
	SkillName: 'Cobalt Trap',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [3],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 1]]
};
SkillInfo[SK.RA_SENSITIVEKEEN] = {
	Name: 'RA_SENSITIVEKEEN',
	SkillName: 'Keen Nose',
	MaxLv: 5,
	SpAmount: [12, 12, 12, 12, 12],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RA_TOOTHOFWUG, 3]]
};
SkillInfo[SK.RA_TOOTHOFWUG] = {
	Name: 'RA_TOOTHOFWUG',
	SkillName: 'Warg Teeth',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RA_WUGMASTERY, 1]]
};
SkillInfo[SK.RA_WUGDASH] = {
	Name: 'RA_WUGDASH',
	SkillName: 'Warg Dash',
	MaxLv: 1,
	SpAmount: [4],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.RA_WUGRIDER, 1]]
};
SkillInfo[SK.RA_ELECTRICSHOCKER] = {
	Name: 'RA_ELECTRICSHOCKER',
	SkillName: 'Electric Shock',
	MaxLv: 5,
	SpAmount: [35, 35, 35, 35, 35],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.HT_SHOCKWAVE, 5]]
};
SkillInfo[SK.RA_DETONATOR] = {
	Name: 'RA_DETONATOR',
	SkillName: 'Detonator',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.RA_CLUSTERBOMB, 3]]
};
SkillInfo[SK.RA_AIMEDBOLT] = {
	Name: 'RA_AIMEDBOLT',
	SkillName: 'Aimed Bolt',
	MaxLv: 10,
	SpAmount: [40, 40, 40, 40, 40, 40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.HT_ANKLESNARE, 5]]
};
SkillInfo[SK.RA_RANGERMAIN] = {
	Name: 'RA_RANGERMAIN',
	SkillName: 'Main Ranger',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.RA_FEARBREEZE] = {
	Name: 'RA_FEARBREEZE',
	SkillName: 'Fear Breeze',
	MaxLv: 5,
	SpAmount: [55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.RA_ARROWSTORM, 5],
		[SK.RA_CAMOUFLAGE, 1]
	]
};
SkillInfo[SK.WL_RELEASE] = {
	Name: 'WL_RELEASE',
	SkillName: 'Release',
	MaxLv: 2,
	SpAmount: [10, 10],
	bSeperateLv: true,
	AttackRange: [11, 11]
};
SkillInfo[SK.WL_SUMMONSTONE] = {
	Name: 'WL_SUMMONSTONE',
	SkillName: 'Summon Stone',
	MaxLv: 2,
	SpAmount: [10, 50],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.WZ_HEAVENDRIVE, 1]]
};
SkillInfo[SK.WL_SUMMONFB] = {
	Name: 'WL_SUMMONFB',
	SkillName: 'Summon Fire Ball',
	MaxLv: 2,
	SpAmount: [10, 50],
	bSeperateLv: true,
	AttackRange: [1, 1],
	_NeedSkillList: [[SK.WZ_METEOR, 1]]
};
SkillInfo[SK.WL_CHAINLIGHTNING] = {
	Name: 'WL_CHAINLIGHTNING',
	SkillName: 'Chain Lightning',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_SUMMONBL, 1]]
};
SkillInfo[SK.WL_COMET] = {
	Name: 'WL_COMET',
	SkillName: 'Comet',
	MaxLv: 5,
	SpAmount: [70, 90, 110, 130, 150],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_HELLINFERNO, 3]]
};
SkillInfo[SK.WL_DRAINLIFE] = {
	Name: 'WL_DRAINLIFE',
	SkillName: 'Drain Life',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_RADIUS, 1]]
};
SkillInfo[SK.WL_RECOGNIZEDSPELL] = {
	Name: 'WL_RECOGNIZEDSPELL',
	SkillName: 'Recognized Spell',
	MaxLv: 5,
	SpAmount: [100, 120, 140, 160, 180],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.WL_RELEASE, 2],
		[SK.WL_STASIS, 1],
		[SK.WL_WHITEIMPRISON, 1]
	]
};
SkillInfo[SK.AL_DP] = {
	Name: 'AL_DP',
	SkillName: 'Divine Protection',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	NeedSkillList: { [JobId.CRUSADER]: [[SK.AL_CURE, 1]] }
};
SkillInfo[SK.WL_MARSHOFABYSS] = {
	Name: 'WL_MARSHOFABYSS',
	SkillName: 'Marsh Of Abyss',
	MaxLv: 5,
	SpAmount: [40, 42, 44, 46, 48],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WZ_QUAGMIRE, 1]]
};
SkillInfo[SK.WL_JACKFROST] = {
	Name: 'WL_JACKFROST',
	SkillName: 'Jack Frost',
	MaxLv: 5,
	SpAmount: [50, 60, 70, 80, 90],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_FROSTMISTY, 2]]
};
SkillInfo[SK.WL_FROSTMISTY] = {
	Name: 'WL_FROSTMISTY',
	SkillName: 'Frost Misty',
	MaxLv: 5,
	SpAmount: [40, 48, 56, 64, 72],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_SUMMONWB, 1]]
};
SkillInfo[SK.WL_SOULEXPANSION] = {
	Name: 'WL_SOULEXPANSION',
	SkillName: 'Soul Expansion',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.WL_DRAINLIFE, 1]]
};
SkillInfo[SK.AB_EXPIATIO] = {
	Name: 'AB_EXPIATIO',
	SkillName: 'Expiatio',
	MaxLv: 5,
	SpAmount: [35, 40, 45, 50, 55],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.AB_DUPLELIGHT, 5],
		[SK.AB_ORATIO, 5]
	]
};
SkillInfo[SK.LK_AURABLADE] = {
	Name: 'LK_AURABLADE',
	SkillName: 'Aura Blade',
	MaxLv: 5,
	SpAmount: [18, 26, 34, 42, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SM_MAGNUM, 5],
		[SK.SM_TWOHAND, 5]
	]
};
SkillInfo[SK.AB_RENOVATIO] = {
	Name: 'AB_RENOVATIO',
	SkillName: 'Renovatio',
	MaxLv: 4,
	SpAmount: [240, 280, 320, 360],
	bSeperateLv: false,
	AttackRange: [11, 11, 11, 11],
	_NeedSkillList: [[SK.AB_CHEAL, 3]]
};
SkillInfo[SK.LK_PARRYING] = {
	Name: 'LK_PARRYING',
	SkillName: 'Parry',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SM_PROVOKE, 5],
		[SK.SM_TWOHAND, 10],
		[SK.KN_TWOHANDQUICKEN, 3]
	]
};
SkillInfo[SK.AB_LAUDAAGNUS] = {
	Name: 'AB_LAUDAAGNUS',
	SkillName: 'Lauda Agnus',
	MaxLv: 4,
	SpAmount: [50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11],
	_NeedSkillList: [[SK.PR_STRECOVERY, 1]]
};
SkillInfo[SK.LK_CONCENTRATION] = {
	Name: 'LK_CONCENTRATION',
	SkillName: 'Spear Dynamo',
	MaxLv: 5,
	SpAmount: [14, 18, 22, 26, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SM_RECOVERY, 5],
		[SK.KN_SPEARMASTERY, 5],
		[SK.KN_RIDING, 1]
	]
};
SkillInfo[SK.AB_ORATIO] = {
	Name: 'AB_ORATIO',
	SkillName: 'Oratio',
	MaxLv: 10,
	SpAmount: [35, 38, 41, 44, 47, 50, 53, 56, 59, 62],
	bSeperateLv: false,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.AB_PRAEFATIO, 5]]
};
SkillInfo[SK.LK_TENSIONRELAX] = {
	Name: 'LK_TENSIONRELAX',
	SkillName: 'Relax',
	MaxLv: 1,
	SpAmount: [15],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SM_PROVOKE, 5],
		[SK.SM_RECOVERY, 10],
		[SK.SM_ENDURE, 3]
	]
};
SkillInfo[SK.AB_PRAEFATIO] = {
	Name: 'AB_PRAEFATIO',
	SkillName: 'Praefatio',
	MaxLv: 10,
	SpAmount: [90, 100, 110, 120, 130, 140, 150, 160, 170, 180],
	bSeperateLv: false,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.PR_KYRIE, 1]]
};
SkillInfo[SK.LK_BERSERK] = {
	Name: 'LK_BERSERK',
	SkillName: 'Frenzy',
	MaxLv: 1,
	SpAmount: [200],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.AB_EPICLESIS] = {
	Name: 'AB_EPICLESIS',
	SkillName: 'Epiclesis',
	MaxLv: 5,
	SpAmount: [300, 300, 300, 300, 300],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.AB_ANCILLA, 1],
		[SK.AB_HIGHNESSHEAL, 1]
	]
};
SkillInfo[SK.AB_CHEAL] = {
	Name: 'AB_CHEAL',
	SkillName: 'Coluseo Heal',
	MaxLv: 3,
	SpAmount: [200, 220, 240],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.AL_HEAL, 1]]
};
SkillInfo[SK.AB_ANCILLA] = {
	Name: 'AB_ANCILLA',
	SkillName: 'Ancilla',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.AB_CLEMENTIA, 3]]
};
SkillInfo[SK.HP_ASSUMPTIO] = {
	Name: 'HP_ASSUMPTIO',
	SkillName: 'Assumptio',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AL_ANGELUS, 1],
		[SK.MG_SRECOVERY, 3],
		[SK.PR_IMPOSITIO, 3]
	]
};
SkillInfo[SK.GC_HALLUCINATIONWALK] = {
	Name: 'GC_HALLUCINATIONWALK',
	SkillName: 'Hallucination Walk',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GC_PHANTOMMENACE, 1]]
};
SkillInfo[SK.HP_BASILICA] = {
	Name: 'HP_BASILICA',
	SkillName: 'Basilica',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.PR_GLORIA, 2],
		[SK.MG_SRECOVERY, 1],
		[SK.PR_KYRIE, 3]
	]
};
SkillInfo[SK.GC_VENOMPRESSURE] = {
	Name: 'GC_VENOMPRESSURE',
	SkillName: 'Venom Pressure',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.GC_WEAPONBLOCKING, 1],
		[SK.GC_POISONINGWEAPON, 3]
	]
};
SkillInfo[SK.HP_MEDITATIO] = {
	Name: 'HP_MEDITATIO',
	SkillName: 'Meditation',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MG_SRECOVERY, 5],
		[SK.PR_LEXDIVINA, 5],
		[SK.PR_ASPERSIO, 3]
	]
};
SkillInfo[SK.GC_WEAPONCRUSH] = {
	Name: 'GC_WEAPONCRUSH',
	SkillName: 'Weapon Crush',
	MaxLv: 5,
	SpAmount: [20, 20, 20, 20, 20],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GC_WEAPONBLOCKING, 1]]
};
SkillInfo[SK.HW_SOULDRAIN] = {
	Name: 'HW_SOULDRAIN',
	SkillName: 'Soul Drain',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MG_SRECOVERY, 5],
		[SK.MG_SOULSTRIKE, 7]
	]
};
SkillInfo[SK.GC_POISONINGWEAPON] = {
	Name: 'GC_POISONINGWEAPON',
	SkillName: 'Poisonous Weapon',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GC_CREATENEWPOISON, 1]]
};
SkillInfo[SK.HW_MAGICCRASHER] = {
	Name: 'HW_MAGICCRASHER',
	SkillName: 'Stave Crasher',
	MaxLv: 1,
	SpAmount: [8],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.MG_SRECOVERY, 1]]
};
SkillInfo[SK.GC_DARKILLUSION] = {
	Name: 'GC_DARKILLUSION',
	SkillName: 'Dark Illusion',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [[SK.GC_CROSSIMPACT, 3]]
};
SkillInfo[SK.HW_MAGICPOWER] = {
	Name: 'HW_MAGICPOWER',
	SkillName: 'Mystical Amplification',
	MaxLv: 10,
	SpAmount: [35, 40, 45, 50, 55, 60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.RK_ABUNDANCE] = {
	Name: 'RK_ABUNDANCE',
	SkillName: 'Abundance',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.PA_PRESSURE] = {
	Name: 'PA_PRESSURE',
	SkillName: 'Gloria Domini',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SM_ENDURE, 5],
		[SK.CR_TRUST, 5],
		[SK.CR_SHIELDCHARGE, 2]
	]
};
SkillInfo[SK.AL_DEMONBANE] = {
	Name: 'AL_DEMONBANE',
	SkillName: 'Demon Bane',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AL_DP, 3]]
};
SkillInfo[SK.PA_SACRIFICE] = {
	Name: 'PA_SACRIFICE',
	SkillName: "Martyr's Reckoning",
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: false,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SM_ENDURE, 1],
		[SK.CR_DEVOTION, 3]
	]
};
SkillInfo[SK.RK_STONEHARDSKIN] = {
	Name: 'RK_STONEHARDSKIN',
	SkillName: 'Skin of Stone',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.PA_GOSPEL] = {
	Name: 'PA_GOSPEL',
	SkillName: 'Battle Chant',
	MaxLv: 10,
	SpAmount: [80, 80, 80, 80, 80, 100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.CR_TRUST, 8],
		[SK.AL_DP, 3],
		[SK.AL_DEMONBANE, 5]
	]
};
SkillInfo[SK.RK_GIANTGROWTH] = {
	Name: 'RK_GIANTGROWTH',
	SkillName: 'Giant Growth',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.CH_PALMSTRIKE] = {
	Name: 'CH_PALMSTRIKE',
	SkillName: 'Raging Palm Strike',
	MaxLv: 5,
	SpAmount: [2, 4, 6, 8, 10],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.MO_IRONHAND, 7],
		[SK.MO_CALLSPIRITS, 5]
	]
};
SkillInfo[SK.RK_MILLENNIUMSHIELD] = {
	Name: 'RK_MILLENNIUMSHIELD',
	SkillName: 'Millenium Shield',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.CH_TIGERFIST] = {
	Name: 'CH_TIGERFIST',
	SkillName: 'Glacier Fist',
	MaxLv: 5,
	SpAmount: [4, 6, 8, 10, 12],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.MO_IRONHAND, 5],
		[SK.MO_TRIPLEATTACK, 5],
		[SK.MO_COMBOFINISH, 3]
	]
};
SkillInfo[SK.RK_DRAGONTRAINING] = {
	Name: 'RK_DRAGONTRAINING',
	SkillName: 'Dragon Training',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KN_CAVALIERMASTERY, 1]]
};
SkillInfo[SK.CH_CHAINCRUSH] = {
	Name: 'CH_CHAINCRUSH',
	SkillName: 'Chain Crush Combo',
	MaxLv: 10,
	SpAmount: [4, 6, 8, 10, 12, 14, 16, 18, 20, 22],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.MO_IRONHAND, 5],
		[SK.MO_CALLSPIRITS, 5],
		[SK.CH_TIGERFIST, 2]
	]
};
SkillInfo[SK.RK_DEATHBOUND] = {
	Name: 'RK_DEATHBOUND',
	SkillName: 'Death Bound',
	MaxLv: 10,
	SpAmount: [50, 60, 65, 70, 75, 80, 85, 90, 95, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.KN_AUTOCOUNTER, 1],
		[SK.RK_ENCHANTBLADE, 2]
	]
};
SkillInfo[SK.PF_HPCONVERSION] = {
	Name: 'PF_HPCONVERSION',
	SkillName: 'Indulge',
	MaxLv: 5,
	SpAmount: [1, 2, 3, 4, 5],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MG_SRECOVERY, 1],
		[SK.SA_MAGICROD, 1]
	]
};
SkillInfo[SK.HVAN_INSTRUCT] = {
	Name: 'HVAN_INSTRUCT',
	SkillName: 'Instruction Change',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.PF_SOULCHANGE] = {
	Name: 'PF_SOULCHANGE',
	SkillName: 'Soul Exhale',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [
		[SK.SA_MAGICROD, 3],
		[SK.SA_SPELLBREAKER, 2]
	]
};
SkillInfo[SK.MH_STAHL_HORN] = {
	Name: 'MH_STAHL_HORN',
	SkillName: 'Stahl Horn',
	MaxLv: 10,
	SpAmount: [43, 46, 49, 52, 55, 58, 61, 64, 67, 70],
	bSeperateLv: true,
	AttackRange: [5, 5, 6, 6, 7, 7, 8, 8, 9, 9]
};
SkillInfo[SK.PF_SOULBURN] = {
	Name: 'PF_SOULBURN',
	SkillName: 'Soul Siphon',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SA_CASTCANCEL, 5],
		[SK.SA_MAGICROD, 3],
		[SK.SA_DISPELL, 3]
	]
};
SkillInfo[SK.NPC_MAGICMIRROR] = {
	Name: 'NPC_MAGICMIRROR',
	SkillName: 'Magic Mirror',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ASC_KATAR] = {
	Name: 'ASC_KATAR',
	SkillName: 'Advanced Katar Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.TF_DOUBLE, 5],
		[SK.AS_KATAR, 7]
	]
};
SkillInfo[SK.DA_DREAM] = {
	Name: 'DA_DREAM',
	SkillName: 'Dream',
	MaxLv: 5,
	SpAmount: [600, 500, 400, 300, 200],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DA_SPACE] = {
	Name: 'DA_SPACE',
	SkillName: 'Space',
	MaxLv: 5,
	SpAmount: [120, 100, 80, 60, 40],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.GD_EMERGENCYCALL] = {
	Name: 'GD_EMERGENCYCALL',
	SkillName: 'Urgent Call',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ASC_EDP] = {
	Name: 'ASC_EDP',
	SkillName: 'Enchant Deadly Poison',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.ASC_CDP, 1]]
};
SkillInfo[SK.DE_NIGHTMARE] = {
	Name: 'DE_NIGHTMARE',
	SkillName: 'Nightmare',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [4]
};
SkillInfo[SK.ASC_BREAKER] = {
	Name: 'ASC_BREAKER',
	SkillName: 'Soul Destroyer',
	MaxLv: 10,
	SpAmount: [60, 60, 60, 60, 60, 60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
	_NeedSkillList: [
		[SK.TF_DOUBLE, 5],
		[SK.TF_POISON, 5],
		[SK.AS_CLOAKING, 3],
		[SK.AS_ENCHANTPOISON, 6]
	]
};
SkillInfo[SK.SL_GUNNER] = {
	Name: 'SL_GUNNER',
	SkillName: 'Gunslinger Spirit',
	MaxLv: 5,
	SpAmount: [460, 360, 260, 160, 60],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.SN_SIGHT] = {
	Name: 'SN_SIGHT',
	SkillName: 'Falcon Eyes',
	MaxLv: 10,
	SpAmount: [20, 20, 25, 25, 30, 30, 35, 35, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AC_OWL, 10],
		[SK.AC_VULTURE, 10],
		[SK.AC_CONCENTRATION, 10],
		[SK.HT_FALCON, 1]
	]
};
SkillInfo[SK.MB_MUNAKKNOWLEDGE] = {
	Name: 'MB_MUNAKKNOWLEDGE',
	SkillName:
		'ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã¢â‚¬ÂÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÆ’Ã…â€™Ãƒâ€šÃ‚Â¹ÃƒÆ’Ã¢â‚¬â€œÃƒâ€šÃ‚Â ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¸ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¶ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â½ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [4]
};
SkillInfo[SK.SN_FALCONASSAULT] = {
	Name: 'SN_FALCONASSAULT',
	SkillName: 'Falcon Assault',
	MaxLv: 5,
	SpAmount: [30, 34, 38, 42, 46],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AC_VULTURE, 5],
		[SK.HT_FALCON, 1],
		[SK.HT_BLITZBEAT, 5],
		[SK.HT_STEELCROW, 3]
	]
};
SkillInfo[SK.NJ_NEN] = {
	Name: 'NJ_NEN',
	SkillName: 'Ninja Aura',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_NINPOU, 5]]
};
SkillInfo[SK.SN_SHARPSHOOTING] = {
	Name: 'SN_SHARPSHOOTING',
	SkillName: 'Focused Arrow Strike',
	MaxLv: 5,
	SpAmount: [16, 18, 20, 22, 24],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.AC_DOUBLE, 5],
		[SK.AC_CONCENTRATION, 10]
	]
};
SkillInfo[SK.NJ_TATAMIGAESHI] = {
	Name: 'NJ_TATAMIGAESHI',
	SkillName: 'Flip Tatami',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.GS_CHAINACTION] = {
	Name: 'GS_CHAINACTION',
	SkillName: 'Chain Action',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GS_SINGLEACTION, 1]]
};
SkillInfo[SK.KO_YAMIKUMO] = {
	Name: 'KO_YAMIKUMO',
	SkillName: 'Shadow Hiding',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.NJ_KIRIKAGE, 5]]
};
SkillInfo[SK.KO_RIGHT] = {
	Name: 'KO_RIGHT',
	SkillName: 'Righthand Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.KO_LEFT] = {
	Name: 'KO_LEFT',
	SkillName: 'Lefthand Mastery',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.KO_JYUMONJIKIRI] = {
	Name: 'KO_JYUMONJIKIRI',
	SkillName: 'Cross Slash',
	MaxLv: 10,
	SpAmount: [10, 12, 14, 16, 18, 20, 22, 24, 26, 28],
	bSeperateLv: true,
	AttackRange: [4, 4, 4, 5, 5, 5, 6, 6, 6, 7],
	_NeedSkillList: [[SK.KO_YAMIKUMO, 1]]
};
SkillInfo[SK.KO_SETSUDAN] = {
	Name: 'KO_SETSUDAN',
	SkillName: 'Soul Cutter',
	MaxLv: 5,
	SpAmount: [12, 16, 20, 24, 28],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.KO_JYUMONJIKIRI, 2]]
};
SkillInfo[SK.KO_BAKURETSU] = {
	Name: 'KO_BAKURETSU',
	SkillName: 'Kunai Explosion',
	MaxLv: 5,
	SpAmount: [5, 6, 7, 8, 9],
	bSeperateLv: true,
	AttackRange: [7, 8, 9, 10, 11],
	_NeedSkillList: [[SK.NJ_KUNAI, 5]]
};
SkillInfo[SK.KO_HAPPOKUNAI] = {
	Name: 'KO_HAPPOKUNAI',
	SkillName: 'Kunai Splash',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KO_BAKURETSU, 1]]
};
SkillInfo[SK.KO_MUCHANAGE] = {
	Name: 'KO_MUCHANAGE',
	SkillName: 'Rapid Throw',
	MaxLv: 10,
	SpAmount: [50, 50, 50, 50, 50, 50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.KO_MAKIBISHI, 3]]
};
SkillInfo[SK.KO_HUUMARANKA] = {
	Name: 'KO_HUUMARANKA',
	SkillName: 'Swirling Petal',
	MaxLv: 10,
	SpAmount: [22, 24, 26, 28, 30, 32, 34, 36, 38, 40],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.NJ_HUUMA, 5]]
};
SkillInfo[SK.KO_MAKIBISHI] = {
	Name: 'KO_MAKIBISHI',
	SkillName: 'Makibishi',
	MaxLv: 5,
	SpAmount: [9, 12, 15, 18, 21],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.NJ_ZENYNAGE, 1]]
};
SkillInfo[SK.KO_MEIKYOUSISUI] = {
	Name: 'KO_MEIKYOUSISUI',
	SkillName: 'Pure Soul',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_NINPOU, 10]]
};
SkillInfo[SK.KO_ZANZOU] = {
	Name: 'KO_ZANZOU',
	SkillName: 'Illusion - Shadow',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_UTSUSEMI, 1]]
};
SkillInfo[SK.KO_KYOUGAKU] = {
	Name: 'KO_KYOUGAKU',
	SkillName: 'Illusion - Shock',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.KO_GENWAKU, 2]]
};
SkillInfo[SK.KO_JYUSATSU] = {
	Name: 'KO_JYUSATSU',
	SkillName: 'Illusion - Death',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [5, 5, 5, 5, 5],
	_NeedSkillList: [[SK.KO_KYOUGAKU, 3]]
};
SkillInfo[SK.KO_KAHU_ENTEN] = {
	Name: 'KO_KAHU_ENTEN',
	SkillName: 'Fire Charm',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KO_HYOUHU_HUBUKI] = {
	Name: 'KO_HYOUHU_HUBUKI',
	SkillName: 'Ice Charm',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KO_KAZEHU_SEIRAN] = {
	Name: 'KO_KAZEHU_SEIRAN',
	SkillName: 'Wind Charm',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KO_DOHU_KOUKAI] = {
	Name: 'KO_DOHU_KOUKAI',
	SkillName: 'Earth Charm',
	MaxLv: 1,
	SpAmount: [20],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.KO_KAIHOU] = {
	Name: 'KO_KAIHOU',
	SkillName: 'Release Ninja Spell',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.KO_KAHU_ENTEN, 1],
		[SK.KO_HYOUHU_HUBUKI, 1],
		[SK.KO_KAZEHU_SEIRAN, 1],
		[SK.KO_DOHU_KOUKAI, 1]
	]
};
SkillInfo[SK.KO_ZENKAI] = {
	Name: 'KO_ZENKAI',
	SkillName: 'Cast Ninja Spell',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.KO_KAIHOU, 1],
		[SK.KO_IZAYOI, 1]
	]
};
SkillInfo[SK.KO_GENWAKU] = {
	Name: 'KO_GENWAKU',
	SkillName: 'Illusion - Bewitch',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [5, 6, 7, 8, 9],
	_NeedSkillList: [[SK.NJ_UTSUSEMI, 1]]
};
SkillInfo[SK.KO_IZAYOI] = {
	Name: 'KO_IZAYOI',
	SkillName: '16th Night',
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NJ_NINPOU, 5]]
};
SkillInfo[SK.KG_KAGEHUMI] = {
	Name: 'KG_KAGEHUMI',
	SkillName: 'Shadow Trampling',
	MaxLv: 5,
	SpAmount: [25, 30, 35, 40, 45],
	bSeperateLv: true,
	AttackRange: [5, 7, 9, 11, 13],
	_NeedSkillList: [[SK.KO_ZANZOU, 1]]
};
SkillInfo[SK.KG_KYOMU] = {
	Name: 'KG_KYOMU',
	SkillName: 'Empty Shadow',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KG_KAGEHUMI, 2]]
};
SkillInfo[SK.KG_KAGEMUSYA] = {
	Name: 'KG_KAGEMUSYA',
	SkillName: 'Shadow Warrior',
	MaxLv: 5,
	SpAmount: [60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.KG_KYOMU, 3]]
};
SkillInfo[SK.OB_ZANGETSU] = {
	Name: 'OB_ZANGETSU',
	SkillName: 'Distorted Crescent',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.KO_GENWAKU, 1]]
};
SkillInfo[SK.OB_OBOROGENSOU] = {
	Name: 'OB_OBOROGENSOU',
	SkillName: 'Moonlight Fantasy',
	MaxLv: 5,
	SpAmount: [55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.OB_AKAITSUKI, 3]]
};
SkillInfo[SK.OB_AKAITSUKI] = {
	Name: 'OB_AKAITSUKI',
	SkillName: 'Ominous Moonlight',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.OB_ZANGETSU, 2]]
};
SkillInfo[SK.ECLAGE_RECALL] = {
	Name: 'ECLAGE_RECALL',
	SkillName: 'Return to Eclage',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.ECL_SNOWFLIP] = {
	Name: 'ECL_SNOWFLIP',
	SkillName: 'Snow Flip',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.ECL_PEONYMAMY] = {
	Name: 'ECL_PEONYMAMY',
	SkillName: 'Peony Mommy',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.ECL_SADAGUI] = {
	Name: 'ECL_SADAGUI',
	SkillName: 'Slapping Herb',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.ECL_SEQUOIADUST] = {
	Name: 'ECL_SEQUOIADUST',
	SkillName: 'Yggdrasil Dust',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [7]
};
SkillInfo[SK.ALL_RAY_OF_PROTECTION] = {
	Name: 'ALL_RAY_OF_PROTECTION',
	SkillName:
		'ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¼ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¶ÃƒÆ’Ã‹â€ Ãƒâ€šÃ‚Â£ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÆ’Ã¢â‚¬Â¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚ÂºÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â» ',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.MER_INVINCIBLEOFF2] = {
	Name: 'MER_INVINCIBLEOFF2',
	SkillName: 'Mind Blaster',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [2]
};
SkillInfo[SK.GC_DARKCROW] = {
	Name: 'GC_DARKCROW',
	SkillName: 'Dark Claw',
	MaxLv: 5,
	SpAmount: [22, 34, 46, 58, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GC_DARKILLUSION, 5]]
};
SkillInfo[SK.RA_UNLIMIT] = {
	Name: 'RA_UNLIMIT',
	SkillName: 'No Limits',
	MaxLv: 5,
	SpAmount: [100, 120, 140, 160, 180],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RA_FEARBREEZE, 5]]
};
SkillInfo[SK.LG_KINGS_GRACE] = {
	Name: 'LG_KINGS_GRACE',
	SkillName: "King's Grace",
	MaxLv: 5,
	SpAmount: [200, 180, 160, 140, 120],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.LG_REFLECTDAMAGE, 5]]
};
SkillInfo[SK.RK_DRAGONBREATH_WATER] = {
	Name: 'RK_DRAGONBREATH_WATER',
	SkillName: "Dragon's Water Breath",
	MaxLv: 10,
	SpAmount: [30, 35, 40, 45, 50, 55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RK_DRAGONTRAINING, 2]]
};
SkillInfo[SK.NC_MAGMA_ERUPTION] = {
	Name: 'NC_MAGMA_ERUPTION',
	SkillName: 'Lava Flow',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.NC_RESEARCHFE, 1]]
};
SkillInfo[SK.WM_FRIGG_SONG] = {
	Name: 'WM_FRIGG_SONG',
	SkillName: "Frigg's Song",
	MaxLv: 5,
	SpAmount: [200, 230, 260, 290, 320],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WM_LESSON, 2]]
};
SkillInfo[SK.SO_ELEMENTAL_SHIELD] = {
	Name: 'SO_ELEMENTAL_SHIELD',
	SkillName: 'Elemental Shield',
	MaxLv: 5,
	SpAmount: [120, 120, 120, 120, 120],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SO_EL_CONTROL, 3]]
};
SkillInfo[SK.SR_FLASHCOMBO] = {
	Name: 'SR_FLASHCOMBO',
	SkillName: 'Flash Combo',
	MaxLv: 5,
	SpAmount: [65, 65, 65, 65, 65],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SR_DRAGONCOMBO, 3],
		[SK.SR_FALLENEMPIRE, 3],
		[SK.SR_SKYNETBLOW, 1],
		[SK.SR_TIGERCANNON, 5]
	]
};
SkillInfo[SK.SC_ESCAPE] = {
	Name: 'SC_ESCAPE',
	SkillName: 'Urgent Escape',
	MaxLv: 5,
	SpAmount: [30, 26, 22, 18, 14],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SC_TRIANGLESHOT, 2]]
};
SkillInfo[SK.AB_OFFERTORIUM] = {
	Name: 'AB_OFFERTORIUM',
	SkillName: 'Offertorium',
	MaxLv: 5,
	SpAmount: [30, 60, 90, 120, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AB_HIGHNESSHEAL, 2]]
};
SkillInfo[SK.WL_TELEKINESIS_INTENSE] = {
	Name: 'WL_TELEKINESIS_INTENSE',
	SkillName: 'Intensification',
	MaxLv: 5,
	SpAmount: [100, 150, 200, 250, 300],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WL_SOULEXPANSION, 5]]
};
SkillInfo[SK.ALL_FULL_THROTTLE] = {
	Name: 'ALL_FULL_THROTTLE',
	SkillName: 'Full Throttle',
	MaxLv: 5,
	SpAmount: [1, 1, 1, 1, 1],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: []
};
SkillInfo[SK.GN_ILLUSIONDOPING] = {
	Name: 'GN_ILLUSIONDOPING',
	SkillName: 'Hallucination Drug',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 1]]
};
SkillInfo[SK.GM_ITEM_ATKMAX] = {
	Name: 'GM_ITEM_ATKMAX',
	SkillName: 'UNKNOW NAME',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GM_ITEM_ATKMIN] = {
	Name: 'GM_ITEM_ATKMIN',
	SkillName: 'Max Physical item attack rate',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GM_ITEM_MATKMAX] = {
	Name: 'GM_ITEM_MATKMAX',
	SkillName: 'Minimize Physical item attack rate',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GM_ITEM_MATKMIN] = {
	Name: 'GM_ITEM_MATKMIN',
	SkillName: 'Minimize Magic item attack rate',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.RL_D_TAIL] = {
	Name: 'RL_D_TAIL',
	SkillName: 'Dragon Tail',
	MaxLv: 10,
	SpAmount: [55, 60, 65, 70, 75, 80, 85, 90, 95, 100],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.RL_H_MINE, 3],
		[SK.RL_C_MARKER, 1]
	]
};
SkillInfo[SK.RL_R_TRIP] = {
	Name: 'RL_R_TRIP',
	SkillName: 'Round Trip',
	MaxLv: 10,
	SpAmount: [43, 46, 49, 52, 55, 58, 61, 64, 67, 70],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	_NeedSkillList: [[SK.RL_FIRE_RAIN, 1]]
};
SkillInfo[SK.RL_RICHS_COIN] = {
	Name: 'RL_RICHS_COIN',
	SkillName: "Rich's Coin",
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [0],
	_NeedSkillList: [[SK.GS_GLITTERING, 5]]
};
SkillInfo[SK.RL_MASS_SPIRAL] = {
	Name: 'RL_MASS_SPIRAL',
	SkillName: 'Mass Spiral',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [15, 15, 15, 15, 15],
	_NeedSkillList: [[SK.GS_PIERCINGSHOT, 1]]
};
SkillInfo[SK.RL_B_TRAP] = {
	Name: 'RL_B_TRAP',
	SkillName: 'Binding Trap',
	MaxLv: 5,
	SpAmount: [30, 32, 34, 36, 38],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0],
	_NeedSkillList: [[SK.RL_FLICKER, 1]]
};
SkillInfo[SK.RL_BANISHING_BUSTER] = {
	Name: 'RL_BANISHING_BUSTER',
	SkillName: 'Vanishing Buster',
	MaxLv: 10,
	SpAmount: [55, 57, 59, 61, 63, 65, 67, 69, 71, 73],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RL_S_STORM, 1]]
};
SkillInfo[SK.RL_S_STORM] = {
	Name: 'RL_S_STORM',
	SkillName: 'Shattering Storm',
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.GS_DISARM, 1],
		[SK.GS_DUST, 1]
	]
};
SkillInfo[SK.RL_SLUGSHOT] = {
	Name: 'RL_SLUGSHOT',
	SkillName: 'Slug Shot',
	MaxLv: 5,
	SpAmount: [80, 84, 88, 92, 96],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RL_BANISHING_BUSTER, 3]]
};
SkillInfo[SK.RL_AM_BLAST] = {
	Name: 'RL_AM_BLAST',
	SkillName: 'Anti Material Blast',
	MaxLv: 5,
	SpAmount: [80, 84, 88, 92, 96],
	bSeperateLv: true,
	AttackRange: [15, 15, 15, 15, 15],
	_NeedSkillList: [[SK.RL_MASS_SPIRAL, 1]]
};
SkillInfo[SK.RL_E_CHAIN] = {
	Name: 'RL_E_CHAIN',
	SkillName: 'Eternal Chain',
	MaxLv: 10,
	SpAmount: [45, 45, 45, 45, 45, 45, 45, 45, 45, 45],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	_NeedSkillList: [
		[SK.GS_GLITTERING, 1],
		[SK.GS_CHAINACTION, 10]
	]
};
SkillInfo[SK.RL_QD_SHOT] = {
	Name: 'RL_QD_SHOT',
	SkillName: 'Quick Draw Shot',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [0],
	_NeedSkillList: [[SK.GS_CHAINACTION, 1]]
};
SkillInfo[SK.RL_C_MARKER] = {
	Name: 'RL_C_MARKER',
	SkillName: 'Crimson Marker',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [11],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.RL_FIREDANCE] = {
	Name: 'RL_FIREDANCE',
	SkillName: 'Fire Dance',
	MaxLv: 10,
	SpAmount: [13, 16, 19, 22, 25, 28, 31, 34, 37, 40],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	_NeedSkillList: [[SK.GS_DESPERADO, 1]]
};
SkillInfo[SK.RL_FIRE_RAIN] = {
	Name: 'RL_FIRE_RAIN',
	SkillName: 'Fire Rain',
	MaxLv: 5,
	SpAmount: [70, 70, 70, 70, 70],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.GS_GATLINGFEVER, 1]]
};
SkillInfo[SK.RL_FALLEN_ANGEL] = {
	Name: 'RL_FALLEN_ANGEL',
	SkillName: 'Fallen Angel',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.GS_DESPERADO, 10]]
};
SkillInfo[SK.RL_P_ALTER] = {
	Name: 'RL_P_ALTER',
	SkillName: 'Platinum Altar',
	MaxLv: 5,
	SpAmount: [20, 24, 28, 32, 36],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0],
	_NeedSkillList: [[SK.RL_RICHS_COIN, 1]]
};
SkillInfo[SK.RL_FLICKER] = {
	Name: 'RL_FLICKER',
	SkillName: 'Flicker',
	MaxLv: 1,
	SpAmount: [2],
	bSeperateLv: false,
	AttackRange: [0],
	_NeedSkillList: [[SK.GS_GLITTERING, 1]]
};
SkillInfo[SK.RL_H_MINE] = {
	Name: 'RL_H_MINE',
	SkillName: 'Howling Mine',
	MaxLv: 5,
	SpAmount: [45, 50, 55, 60, 65],
	bSeperateLv: true,
	AttackRange: [7, 8, 9, 10, 11],
	_NeedSkillList: [[SK.GS_GROUNDDRIFT, 1]]
};
SkillInfo[SK.RL_HAMMER_OF_GOD] = {
	Name: 'RL_HAMMER_OF_GOD',
	SkillName: "God's Hammer",
	MaxLv: 10,
	SpAmount: [37, 39, 41, 43, 45, 47, 49, 51, 53, 55],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [
		[SK.RL_RICHS_COIN, 1],
		[SK.RL_AM_BLAST, 3]
	]
};
SkillInfo[SK.RL_HEAT_BARREL] = {
	Name: 'RL_HEAT_BARREL',
	SkillName: 'Hit Barrel',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [0, 0, 0, 0, 0],
	_NeedSkillList: [[SK.RL_RICHS_COIN, 1]]
};
SkillInfo[SK.MC_CARTDECORATE] = {
	Name: 'MC_CARTDECORATE',
	SkillName: 'Cart Decoration',
	MaxLv: 1,
	SpAmount: [40],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SU_BASIC_SKILL] = {
	Name: 'SU_BASIC_SKILL',
	SkillName: 'New Basic Skill',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.SU_BITE] = {
	Name: 'SU_BITE',
	SkillName: 'Bite',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [2],
	_NeedSkillList: [[SK.SU_BASIC_SKILL, 1]]
};
SkillInfo[SK.SU_HIDE] = {
	Name: 'SU_HIDE',
	SkillName: 'Hide',
	MaxLv: 1,
	SpAmount: [30],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_BITE, 1]]
};
SkillInfo[SK.SU_SCRATCH] = {
	Name: 'SU_SCRATCH',
	SkillName: 'Scratch',
	MaxLv: 3,
	SpAmount: [20, 25, 30],
	bSeperateLv: true,
	AttackRange: [2, 2, 2],
	_NeedSkillList: [[SK.SU_HIDE, 1]]
};
SkillInfo[SK.SU_STOOP] = {
	Name: 'SU_STOOP',
	SkillName: 'Stoop',
	MaxLv: 1,
	SpAmount: [10],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_SCRATCH, 3]]
};
SkillInfo[SK.SU_LOPE] = {
	Name: 'SU_LOPE',
	SkillName: 'Lope',
	MaxLv: 3,
	SpAmount: [30, 30, 30],
	bSeperateLv: false,
	AttackRange: [6, 10, 14],
	_NeedSkillList: [[SK.SU_STOOP, 1]]
};
SkillInfo[SK.SU_SPRITEMABLE] = {
	Name: 'SU_SPRITEMABLE',
	SkillName: 'Sprite Mable',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_LOPE, 3]]
};
SkillInfo[SK.SU_FRESHSHRIMP] = {
	Name: 'SU_FRESHSHRIMP',
	SkillName: 'Fresh Shrimp',
	MaxLv: 5,
	SpAmount: [22, 24, 26, 28, 30],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SPRITEMABLE, 1]]
};
SkillInfo[SK.SU_BUNCHOFSHRIMP] = {
	Name: 'SU_BUNCHOFSHRIMP',
	SkillName: 'Bunch of Shrimp',
	MaxLv: 5,
	SpAmount: [44, 48, 52, 56, 60],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SU_FRESHSHRIMP, 3]]
};
SkillInfo[SK.SU_TUNABELLY] = {
	Name: 'SU_TUNABELLY',
	SkillName: 'Tuna Belly',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_BUNCHOFSHRIMP, 3]]
};
SkillInfo[SK.SU_TUNAPARTY] = {
	Name: 'SU_TUNAPARTY',
	SkillName: 'Tuna Party',
	MaxLv: 5,
	SpAmount: [20, 30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_TUNABELLY, 3]]
};
SkillInfo[SK.SU_SV_STEMSPEAR] = {
	Name: 'SU_SV_STEMSPEAR',
	SkillName: 'SV Stem Spear',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SPRITEMABLE, 1]]
};
SkillInfo[SK.SU_SV_ROOTTWIST] = {
	Name: 'SU_SV_ROOTTWIST',
	SkillName: 'SV Root Twist',
	MaxLv: 5,
	SpAmount: [10, 12, 14, 16, 18],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SV_STEMSPEAR, 3]]
};
SkillInfo[SK.SU_CN_METEOR] = {
	Name: 'SU_CN_METEOR',
	SkillName: 'CN Meteor',
	MaxLv: 5,
	SpAmount: [20, 35, 50, 65, 80],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SV_ROOTTWIST, 3]]
};
SkillInfo[SK.SU_CN_POWDERING] = {
	Name: 'SU_CN_POWDERING',
	SkillName: 'CN Powdering',
	MaxLv: 5,
	SpAmount: [40, 36, 32, 28, 24],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_CN_METEOR, 3]]
};
SkillInfo[SK.SU_PICKYPECK] = {
	Name: 'SU_PICKYPECK',
	SkillName: 'Picky Peck',
	MaxLv: 5,
	SpAmount: [10, 12, 14, 16, 18],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SPRITEMABLE, 1]]
};
SkillInfo[SK.SU_ARCLOUSEDASH] = {
	Name: 'SU_ARCLOUSEDASH',
	SkillName: 'Arclouze Dash',
	MaxLv: 5,
	SpAmount: [12, 14, 16, 18, 20],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_PICKYPECK, 3]]
};
SkillInfo[SK.SU_SCAROFTAROU] = {
	Name: 'SU_SCAROFTAROU',
	SkillName: 'Scar of Tarou',
	MaxLv: 5,
	SpAmount: [10, 12, 14, 16, 18],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_ARCLOUSEDASH, 3]]
};
SkillInfo[SK.SU_LUNATICCARROTBEAT] = {
	Name: 'SU_LUNATICCARROTBEAT',
	SkillName: 'Lunatic Carrot Beat',
	MaxLv: 5,
	SpAmount: [15, 20, 25, 30, 35],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_SCAROFTAROU, 3]]
};
SkillInfo[SK.SU_POWEROFSEA] = {
	Name: 'SU_POWEROFSEA',
	SkillName: 'Power of Sea',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_TUNAPARTY, 3]]
};
SkillInfo[SK.SU_POWEROFLAND] = {
	Name: 'SU_POWEROFLAND',
	SkillName: 'Power of Land',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_CN_POWDERING, 3]]
};
SkillInfo[SK.SU_POWEROFLIFE] = {
	Name: 'SU_POWEROFLIFE',
	SkillName: 'Power of Life',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SU_LUNATICCARROTBEAT, 3]]
};
SkillInfo[SK.SU_SOULATTACK] = {
	Name: 'SU_SOULATTACK',
	SkillName: 'Soul Attack',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [9],
	_NeedSkillList: [[SK.SU_SPRITEMABLE, 1]]
};
SkillInfo[SK.SU_POWEROFFLOCK] = {
	Name: 'SU_POWEROFFLOCK',
	SkillName: 'Power Of Lock',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_HISS, 5]]
};
SkillInfo[SK.SU_SVG_SPIRIT] = {
	Name: 'SU_SVG_SPIRIT',
	SkillName: 'Sprit Of Savage',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_POWEROFFLOCK, 5]]
};
SkillInfo[SK.SU_HISS] = {
	Name: 'SU_HISS',
	SkillName: 'Hiss',
	MaxLv: 5,
	SpAmount: [50, 46, 42, 38, 34],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_POWEROFLIFE, 1]]
};
SkillInfo[SK.SU_NYANGGRASS] = {
	Name: 'SU_NYANGGRASS',
	SkillName: 'Nyang Grass',
	MaxLv: 5,
	SpAmount: [50, 48, 46, 44, 42],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SU_MEOWMEOW, 5]]
};
SkillInfo[SK.SU_GROOMING] = {
	Name: 'SU_GROOMING',
	SkillName: 'Grooming',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_POWEROFSEA, 1]]
};
SkillInfo[SK.SU_PURRING] = {
	Name: 'SU_PURRING',
	SkillName: 'Purring',
	MaxLv: 5,
	SpAmount: [70, 65, 60, 55, 50],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_GROOMING, 5]]
};
SkillInfo[SK.SU_SHRIMPARTY] = {
	Name: 'SU_SHRIMPARTY',
	SkillName: 'Tasty Shrimp Party',
	MaxLv: 5,
	SpAmount: [100, 90, 80, 70, 60],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_PURRING, 5]]
};
SkillInfo[SK.SU_SPIRITOFLIFE] = {
	Name: 'SU_SPIRITOFLIFE',
	SkillName: 'Spirit Of Life',
	MaxLv: 1,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_SVG_SPIRIT, 5]]
};
SkillInfo[SK.SU_MEOWMEOW] = {
	Name: 'SU_MEOWMEOW',
	SkillName: 'Meow Meow',
	MaxLv: 5,
	SpAmount: [100, 90, 80, 70, 60],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_CHATTERING, 5]]
};
SkillInfo[SK.SU_SPIRITOFLAND] = {
	Name: 'SU_SPIRITOFLAND',
	SkillName: 'Spirit Of Land',
	MaxLv: 1,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_NYANGGRASS, 5]]
};
SkillInfo[SK.SU_CHATTERING] = {
	Name: 'SU_CHATTERING',
	SkillName: 'Chattering',
	MaxLv: 5,
	SpAmount: [50, 45, 40, 35, 30],
	bSeperateLv: true,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_POWEROFLAND, 1]]
};
SkillInfo[SK.SU_SPIRITOFSEA] = {
	Name: 'SU_SPIRITOFSEA',
	SkillName: 'Spirit Of Sea',
	MaxLv: 1,
	SpAmount: [],
	bSeperateLv: false,
	AttackRange: [],
	_NeedSkillList: [[SK.SU_SHRIMPARTY, 5]]
};
SkillInfo[SK.ALL_PRONTERA_RECALL] = {
	Name: 'ALL_PRONTERA_RECALL',
	SkillName: 'Prontera Recall',
	MaxLv: 2,
	SpAmount: [0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1]
};
SkillInfo[SK.NPC_IGNITIONBREAK] = {
	Name: 'NPC_IGNITIONBREAK',
	SkillName: 'Ignition Break',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11]
	]
};
SkillInfo[SK.NPC_MANDRAGORA] = {
	Name: 'NPC_MANDRAGORA',
	SkillName: 'Mandragora Howl',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[13, 13],
		[15, 15],
		[17, 17],
		[19, 19]
	]
};
SkillInfo[SK.NPC_FATALMENACE] = {
	Name: 'NPC_FATALMENACE',
	SkillName: 'Fatal Menace',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[3, 3],
		[5, 5],
		[7, 7],
		[9, 9],
		[11, 11]
	]
};
SkillInfo[SK.NPC_SR_CURSEDCIRCLE] = {
	Name: 'NPC_SR_CURSEDCIRCLE',
	SkillName: 'Cursed Circle',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[3, 3],
		[5, 5],
		[7, 7],
		[9, 9],
		[11, 11]
	]
};
SkillInfo[SK.NPC_JACKFROST] = {
	Name: 'NPC_JACKFROST',
	SkillName: 'Jack Frost',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[13, 13],
		[15, 15],
		[17, 17],
		[19, 19]
	]
};
SkillInfo[SK.NPC_VENOMFOG] = {
	Name: 'NPC_VENOMFOG',
	SkillName: 'Venom fog',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[27, 27]
	]
};
SkillInfo[SK.NPC_ASSASSINCROSS] = {
	Name: 'NPC_ASSASSINCROSS',
	SkillName: 'Impressive Riff',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_FLAMECROSS] = {
	Name: 'NPC_FLAMECROSS',
	SkillName: 'Flame cross',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_ICEMINE] = {
	Name: 'NPC_ICEMINE',
	SkillName: 'Ice mine',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_DISSONANCE] = {
	Name: 'NPC_DISSONANCE',
	SkillName: 'Unchained Serenade',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_UGLYDANCE] = {
	Name: 'NPC_UGLYDANCE',
	SkillName: 'Hip Shaker',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_WIDEHEALTHFEAR] = {
	Name: 'NPC_WIDEHEALTHFEAR',
	SkillName: 'Wide area fear',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDE_DEEP_SLEEP] = {
	Name: 'NPC_WIDE_DEEP_SLEEP',
	SkillName: 'Wide area deep sleep',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESIREN] = {
	Name: 'NPC_WIDESIREN',
	SkillName: 'Wide area fascination',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDEBODYBURNNING] = {
	Name: 'NPC_WIDEBODYBURNNING',
	SkillName: 'Wide area burnning',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDEFROSTMISTY] = {
	Name: 'NPC_WIDEFROSTMISTY',
	SkillName: 'Wide area frost misty',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDECOLD] = {
	Name: 'NPC_WIDECOLD',
	SkillName: 'Wide area freeze',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_CLOUD_KILL] = {
	Name: 'NPC_CLOUD_KILL',
	SkillName: 'Killing Cloud',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9],
	SkillScale: [
		[3, 3],
		[5, 5],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_RAYOFGENESIS] = {
	Name: 'NPC_RAYOFGENESIS',
	SkillName: 'Genesis Ray',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[11, 11],
		[11, 11],
		[17, 17],
		[17, 17],
		[23, 23],
		[23, 23],
		[27, 27],
		[27, 27],
		[27, 27],
		[27, 27]
	]
};
SkillInfo[SK.NPC_PSYCHIC_WAVE] = {
	Name: 'NPC_PSYCHIC_WAVE',
	SkillName: 'Psychic Wave',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	SkillScale: [
		[7, 7],
		[9, 9],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11]
	]
};
SkillInfo[SK.NPC_MAGMA_ERUPTION] = {
	Name: 'NPC_MAGMA_ERUPTION',
	SkillName: 'Lava Flow',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_COMET] = {
	Name: 'NPC_COMET',
	SkillName: 'Comet',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[19, 19],
		[19, 19],
		[19, 19],
		[19, 19],
		[19, 19]
	]
};
SkillInfo[SK.NPC_WIDEWEB] = {
	Name: 'NPC_WIDEWEB',
	SkillName: 'Wide web',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	SkillScale: [[15, 15]]
};
SkillInfo[SK.NPC_WIDESIGHT] = {
	Name: 'NPC_WIDESIGHT',
	SkillName: 'Wide sight',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	SkillScale: [[11, 11]]
};
SkillInfo[SK.NPC_WIDESUCK] = {
	Name: 'NPC_WIDESUCK',
	SkillName: 'Wide bloodsucking',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	SkillScale: [[27, 27]]
};
SkillInfo[SK.NPC_STORMGUST2] = {
	Name: 'NPC_STORMGUST2',
	SkillName: 'Storm Gust',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [9, 9, 9],
	SkillScale: [
		[11, 11],
		[11, 11],
		[11, 11]
	]
};
SkillInfo[SK.NPC_FIRESTORM] = {
	Name: 'NPC_FIRESTORM',
	SkillName: 'Fire storm',
	MaxLv: 3,
	SpAmount: [0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1],
	SkillScale: [
		[7, 7],
		[7, 7],
		[7, 7]
	]
};
SkillInfo[SK.NPC_DRAGONBREATH] = {
	Name: 'NPC_DRAGONBREATH',
	SkillName: "Dragon's Breath",
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [3, 3, 3, 3, 3, 3, 3, 3, 3, 3],
	SkillScale: [
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9],
		[9, 9]
	]
};
SkillInfo[SK.NPC_REVERBERATION] = {
	Name: 'NPC_REVERBERATION',
	SkillName: 'Reverberation',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[5, 5],
		[5, 5],
		[5, 5],
		[5, 5]
	]
};
SkillInfo[SK.NPC_LEX_AETERNA] = {
	Name: 'NPC_LEX_AETERNA',
	SkillName: 'Wide area Lex Aeterna',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.WE_CALLALLFAMILY] = {
	Name: 'WE_CALLALLFAMILY',
	SkillName: "Let's Go Family!",
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.WE_ONEFOREVER] = {
	Name: 'WE_ONEFOREVER',
	SkillName: 'Love Conquers Death',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: false,
	AttackRange: [3]
};
SkillInfo[SK.WE_CHEERUP] = {
	Name: 'WE_CHEERUP',
	SkillName: 'Go! Parents Go!',
	MaxLv: 1,
	SpAmount: [50],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.GD_GUILD_STORAGE] = {
	Name: 'GD_GUILD_STORAGE',
	SkillName: 'Guild Storage Extension',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.CG_SPECIALSINGER] = {
	Name: 'SK_CG_SPECIALSINGER',
	SkillName: 'Skilled Special Singer',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	NeedSkillList: {
		[JobId.BARD_H]: [
			[SK.CG_MARIONETTE, 1],
			[SK.BA_DISSONANCE, 3],
			[SK.BA_MUSICALLESSON, 10]
		],
		[JobId.DANCER_H]: [
			[SK.CG_MARIONETTE, 1],
			[SK.DC_UGLYDANCE, 3],
			[SK.DC_DANCINGLESSON, 10]
		]
	}
};
SkillInfo[SK.BA_POEMBRAGI2] = {
	Name: 'BA_POEMBRAGI2',
	SkillName: 'Magic Strings',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BA_DISSONANCE, 3]]
};
SkillInfo[SK.DC_FORTUNEKISS2] = {
	Name: 'DC_FORTUNEKISS2',
	SkillName: 'Lady Luck',
	MaxLv: 10,
	SpAmount: [43, 46, 49, 52, 55, 58, 61, 64, 67, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DC_UGLYDANCE, 3]]
};
SkillInfo[SK.SJ_LIGHTOFMOON] = {
	Name: 'SJ_LIGHTOFMOON',
	SkillName: 'Lunar Luminance',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_FULLMOONKICK, 3]]
};
SkillInfo[SK.SJ_LUNARSTANCE] = {
	Name: 'SJ_LUNARSTANCE',
	SkillName: 'Lunar Stance',
	MaxLv: 3,
	SpAmount: [10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SJ_DOCUMENT, 1]]
};
SkillInfo[SK.SJ_FULLMOONKICK] = {
	Name: 'SJ_FULLMOONKICK',
	SkillName: 'Full Moon Kick',
	MaxLv: 10,
	SpAmount: [30, 35, 40, 45, 50, 55, 60, 65, 70, 75],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_NEWMOONKICK, 7]]
};
SkillInfo[SK.SJ_NEWMOONKICK] = {
	Name: 'SJ_NEWMOONKICK',
	SkillName: 'New Moon Kick',
	MaxLv: 7,
	SpAmount: [20, 25, 30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_LUNARSTANCE, 1]]
};
SkillInfo[SK.SJ_LIGHTOFSTAR] = {
	Name: 'SJ_LIGHTOFSTAR',
	SkillName: 'Stellar Luminance',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_FALLINGSTAR, 3]]
};
SkillInfo[SK.SJ_STARSTANCE] = {
	Name: 'SJ_STARSTANCE',
	SkillName: 'Stellar Stance',
	MaxLv: 3,
	SpAmount: [10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SJ_DOCUMENT, 1]]
};
SkillInfo[SK.SJ_FLASHKICK] = {
	Name: 'SJ_FLASHKICK',
	SkillName: 'Flash Kick',
	MaxLv: 7,
	SpAmount: [45, 40, 35, 30, 25, 20, 15],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_STARSTANCE, 1]]
};
SkillInfo[SK.SJ_STAREMPEROR] = {
	Name: 'SJ_STAREMPEROR',
	SkillName: "Star Emperor's Descent",
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SJ_NOVAEXPLOSING, 5],
		[SK.SJ_UNIVERSESTANCE, 3]
	]
};
SkillInfo[SK.SJ_NOVAEXPLOSING] = {
	Name: 'SJ_NOVAEXPLOSING',
	SkillName: 'Nova Explosion',
	MaxLv: 5,
	SpAmount: [60, 65, 70, 75, 80],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.SJ_UNIVERSESTANCE, 1]]
};
SkillInfo[SK.SJ_UNIVERSESTANCE] = {
	Name: 'SJ_UNIVERSESTANCE',
	SkillName: 'Universal Stance',
	MaxLv: 3,
	SpAmount: [10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SJ_SUNSTANCE, 3],
		[SK.SJ_LUNARSTANCE, 3],
		[SK.SJ_STARSTANCE, 3]
	]
};
SkillInfo[SK.SJ_FALLINGSTAR] = {
	Name: 'SJ_FALLINGSTAR',
	SkillName: 'Falling Stars',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_FLASHKICK, 7]]
};
SkillInfo[SK.SJ_GRAVITYCONTROL] = {
	Name: 'SJ_GRAVITYCONTROL',
	SkillName: 'Gravity Control',
	MaxLv: 1,
	SpAmount: [80],
	bSeperateLv: true,
	AttackRange: [9],
	_NeedSkillList: [[SK.SJ_UNIVERSESTANCE, 1]]
};
SkillInfo[SK.SJ_BOOKOFDIMENSION] = {
	Name: 'SJ_BOOKOFDIMENSION',
	SkillName: 'Book of Dimensions',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SJ_STAREMPEROR, 3],
		[SK.SJ_DOCUMENT, 3]
	]
};
SkillInfo[SK.SJ_BOOKOFCREATINGSTAR] = {
	Name: 'SJ_BOOKOFCREATINGSTAR',
	SkillName: "Star Creator's Book",
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.SJ_STAREMPEROR, 3],
		[SK.SJ_DOCUMENT, 3]
	]
};
SkillInfo[SK.SJ_DOCUMENT] = {
	Name: 'SJ_DOCUMENT',
	SkillName: 'Solar, Lunar, and Stellar Record',
	MaxLv: 3,
	SpAmount: [60, 60, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [
		[SK.SG_FEEL, 3],
		[SK.SG_HATE, 3]
	]
};
SkillInfo[SK.SJ_PURIFY] = {
	Name: 'SJ_PURIFY',
	SkillName: 'Solar, Lunar, and Stellar Purification',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.SG_DEVIL, 10]]
};
SkillInfo[SK.SJ_LIGHTOFSUN] = {
	Name: 'SJ_LIGHTOFSUN',
	SkillName: 'Solar Luminance',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_SOLARBURST, 3]]
};
SkillInfo[SK.SJ_SUNSTANCE] = {
	Name: 'SJ_SUNSTANCE',
	SkillName: 'Solar Stance',
	MaxLv: 3,
	SpAmount: [10, 10, 10],
	bSeperateLv: true,
	AttackRange: [1, 1, 1],
	_NeedSkillList: [[SK.SJ_DOCUMENT, 1]]
};
SkillInfo[SK.SJ_SOLARBURST] = {
	Name: 'SJ_SOLARBURST',
	SkillName: 'Solar Explosion',
	MaxLv: 10,
	SpAmount: [34, 37, 40, 43, 46, 49, 52, 55, 58, 61],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_PROMINENCEKICK, 7]]
};
SkillInfo[SK.SJ_PROMINENCEKICK] = {
	Name: 'SJ_PROMINENCEKICK',
	SkillName: 'Blaze Kick',
	MaxLv: 7,
	SpAmount: [20, 20, 20, 20, 20, 20, 20],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SJ_SUNSTANCE, 1]]
};
SkillInfo[SK.SP_SOULGOLEM] = {
	Name: 'SP_SOULGOLEM',
	SkillName: 'Golem Soul',
	MaxLv: 5,
	SpAmount: [250, 200, 150, 100, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULREVOLVE, 2]]
};
SkillInfo[SK.SP_SOULSHADOW] = {
	Name: 'SP_SOULSHADOW',
	SkillName: 'Shadow Soul',
	MaxLv: 5,
	SpAmount: [250, 200, 150, 100, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULUNITY, 5]]
};
SkillInfo[SK.SP_SOULFALCON] = {
	Name: 'SP_SOULFALCON',
	SkillName: 'Falcon Soul',
	MaxLv: 5,
	SpAmount: [250, 200, 150, 100, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULREVOLVE, 2]]
};
SkillInfo[SK.SP_SOULFAIRY] = {
	Name: 'SP_SOULFAIRY',
	SkillName: 'Fairy Soul',
	MaxLv: 5,
	SpAmount: [250, 200, 150, 100, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULUNITY, 5]]
};
SkillInfo[SK.SP_CURSEEXPLOSION] = {
	Name: 'SP_CURSEEXPLOSION',
	SkillName: 'Curse Explosion',
	MaxLv: 10,
	SpAmount: [50, 55, 60, 65, 70, 75, 80, 85, 90, 95],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULCURSE, 3]]
};
SkillInfo[SK.SP_SOULCURSE] = {
	Name: 'SP_SOULCURSE',
	SkillName: 'Evil Soul Curse',
	MaxLv: 5,
	SpAmount: [70, 70, 70, 70, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULREAPER, 3]]
};
SkillInfo[SK.SP_SPA] = {
	Name: 'SP_SPA',
	SkillName: 'Espa',
	MaxLv: 10,
	SpAmount: [52, 56, 60, 64, 68, 72, 76, 80, 84, 88],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SHA, 1]]
};
SkillInfo[SK.SP_SHA] = {
	Name: 'SP_SHA',
	SkillName: 'Esha',
	MaxLv: 5,
	SpAmount: [18, 20, 22, 24, 26],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SOULREAPER, 3]]
};
SkillInfo[SK.SP_SWHOO] = {
	Name: 'SP_SWHOO',
	SkillName: 'Eswoo',
	MaxLv: 10,
	SpAmount: [66, 70, 74, 78, 82, 86, 90, 94, 98, 102],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.SP_SPA, 3]]
};
SkillInfo[SK.SP_SOULUNITY] = {
	Name: 'SP_SOULUNITY',
	SkillName: 'Soul Bind',
	MaxLv: 7,
	SpAmount: [44, 46, 48, 50, 52, 54, 56],
	bSeperateLv: true,
	AttackRange: [11, 11, 11, 11, 11, 11, 11],
	_NeedSkillList: [[SK.SP_SOULENERGY, 3]]
};
SkillInfo[SK.SP_SOULDIVISION] = {
	Name: 'SP_SOULDIVISION',
	SkillName: 'Soul Division',
	MaxLv: 5,
	SpAmount: [36, 40, 44, 48, 52],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SP_SPA, 5],
		[SK.SP_SHA, 5]
	]
};
SkillInfo[SK.SP_SOULREAPER] = {
	Name: 'SP_SOULREAPER',
	SkillName: 'Soul Harvest',
	MaxLv: 5,
	SpAmount: [42, 44, 46, 48, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SP_SOULCOLLECT, 1]]
};
SkillInfo[SK.SP_SOULCOLLECT] = {
	Name: 'SP_SOULCOLLECT',
	SkillName: 'Soul Collection',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.SP_SOULREVOLVE] = {
	Name: 'SP_SOULREVOLVE',
	SkillName: 'Soul Circulation',
	MaxLv: 3,
	SpAmount: [50, 100, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9],
	_NeedSkillList: [
		[SK.SP_SOULENERGY, 3],
		[SK.SP_KAUTE, 3]
	]
};
SkillInfo[SK.SP_SOULEXPLOSION] = {
	Name: 'SP_SOULEXPLOSION',
	SkillName: 'Soul Explosion',
	MaxLv: 5,
	SpAmount: [30, 60, 90, 120, 150],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.SP_SOULSHADOW, 1],
		[SK.SP_SOULFALCON, 1],
		[SK.SP_SOULFAIRY, 1],
		[SK.SP_SOULGOLEM, 1],
		[SK.SP_CURSEEXPLOSION, 2]
	]
};
SkillInfo[SK.SP_KAUTE] = {
	Name: 'SP_KAUTE',
	SkillName: 'Kaute',
	MaxLv: 5,
	SpAmount: [24, 30, 36, 42, 48],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.SP_SOULENERGY, 1]]
};
SkillInfo[SK.SP_SOULENERGY] = {
	Name: 'SP_SOULENERGY',
	SkillName: 'Soul Energy Research',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SP_SOULCOLLECT, 1]]
};
SkillInfo[SK.SJ_FALLINGSTAR_ATK2] = {
	Name: 'SJ_FALLINGSTAR_ATK2',
	SkillName: 'Falling Star',
	MaxLv: 1,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.SJ_FALLINGSTAR_ATK] = {
	Name: 'SJ_FALLINGSTAR_ATK',
	SkillName: 'Falling Star',
	MaxLv: 1,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.RK_LUXANIMA] = {
	Name: 'RK_LUXANIMA',
	SkillName: 'Lux Anima',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: []
};
SkillInfo[SK.NPC_WIDEBLEEDING2] = {
	Name: 'NPC_WIDEBLEEDING2',
	SkillName: 'Demonic Mass Bleeding',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESILENCE2] = {
	Name: 'NPC_WIDESILENCE2',
	SkillName: 'Demonic Mass Silence',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESTUN2] = {
	Name: 'NPC_WIDESTUN2',
	SkillName: 'Demonic Mass Stun',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESTONE2] = {
	Name: 'NPC_WIDESTONE2',
	SkillName: 'Demonic Mass Stone',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDESLEEP2] = {
	Name: 'NPC_WIDESLEEP2',
	SkillName: 'Demonic Mass Sleep',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDECURSE2] = {
	Name: 'NPC_WIDECURSE2',
	SkillName: 'Demonic Mass Curse',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDECONFUSE2] = {
	Name: 'NPC_WIDECONFUSE2',
	SkillName: 'Demonic Mass Confuse',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_WIDEFREEZE2] = {
	Name: 'NPC_WIDEFREEZE2',
	SkillName: 'Demonic Mass Freeze',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[11, 11],
		[17, 17],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_EVILLAND2] = {
	Name: 'NPC_EVILLAND2',
	SkillName: 'Demonic Evil Land',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	SkillScale: [
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[11, 11],
		[13, 13],
		[15, 15],
		[19, 19],
		[23, 23],
		[29, 29]
	]
};
SkillInfo[SK.NPC_HELLJUDGEMENT2] = {
	Name: 'NPC_HELLJUDGEMENT2',
	SkillName: 'Demonic Hell Judgment',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	SkillScale: [
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29],
		[29, 29]
	]
};
SkillInfo[SK.NV_BREAKTHROUGH] = {
	Name: 'NV_BREAKTHROUGH',
	SkillName: 'Breakthrough',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: []
};
SkillInfo[SK.NV_HELPANGEL] = {
	Name: 'NV_HELPANGEL',
	SkillName: 'Help, Angel!',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: []
};
SkillInfo[SK.NV_TRANSCENDENCE] = {
	Name: 'NV_TRANSCENDENCE',
	SkillName: 'Transcendence',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: []
};
SkillInfo[SK.ALL_NIFLHEIM_RECALL] = {
	Name: 'ALL_NIFLHEIM_RECALL',
	SkillName: 'The World of the Dead!',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: false,
	AttackRange: [1]
};
SkillInfo[SK.DK_SERVANTWEAPON] = {
	Name: 'DK_SERVANTWEAPON',
	SkillName: 'Servant Weapon',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.DK_SERVANT_W_SIGN] = {
	Name: 'DK_SERVANT_W_SIGN',
	SkillName: 'Servant Weapon - Sign',
	MaxLv: 5,
	SpAmount: [15, 15, 15, 15, 15],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.DK_SERVANTWEAPON, 3]]
};
SkillInfo[SK.DK_SERVANT_W_PHANTOM] = {
	Name: 'DK_SERVANT_W_PHANTOM',
	SkillName: 'Servant Weapon - Phantom',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.DK_SERVANTWEAPON, 5],
		[SK.DK_SERVANT_W_SIGN, 5]
	]
};
SkillInfo[SK.DK_SERVANT_W_DEMOL] = {
	Name: 'DK_SERVANT_W_DEMOL',
	SkillName: 'Servant Weapon - Demolition',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.DK_SERVANT_W_PHANTOM, 5]]
};
SkillInfo[SK.DK_CHARGINGPIERCE] = {
	Name: 'DK_CHARGINGPIERCE',
	SkillName: 'Charging Pierce',
	MaxLv: 10,
	SpAmount: [25, 30, 35, 40, 45, 50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RK_HUNDREDSPEAR, 5]]
};
SkillInfo[SK.DK_TWOHANDDEF] = {
	Name: 'DK_TWOHANDDEF',
	SkillName: 'Two-handed Defense',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.DK_HACKANDSLASHER] = {
	Name: 'DK_HACKANDSLASHER',
	SkillName: 'Hack and Slash',
	MaxLv: 10,
	SpAmount: [34, 38, 42, 46, 50, 54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.DK_TWOHANDDEF, 5]]
};
SkillInfo[SK.DK_DRAGONIC_AURA] = {
	Name: 'DK_DRAGONIC_AURA',
	SkillName: 'Dragonic Aura',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.DK_CHARGINGPIERCE, 10],
		[SK.RK_DRAGONBREATH, 10],
		[SK.RK_DRAGONBREATH_WATER, 10]
	]
};
SkillInfo[SK.DK_MADNESS_CRUSHER] = {
	Name: 'DK_MADNESS_CRUSHER',
	SkillName: 'Madness Crusher',
	MaxLv: 5,
	SpAmount: [34, 38, 42, 46, 50],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.DK_CHARGINGPIERCE, 5],
		[SK.DK_HACKANDSLASHER, 10]
	]
};
SkillInfo[SK.DK_VIGOR] = {
	Name: 'DK_VIGOR',
	SkillName: 'Vigor',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.DK_SERVANT_W_DEMOL, 3],
		[SK.DK_STORMSLASH, 5]
	]
};
SkillInfo[SK.DK_STORMSLASH] = {
	Name: 'DK_STORMSLASH',
	SkillName: 'Storm Slash',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.DK_TWOHANDDEF, 10],
		[SK.DK_HACKANDSLASHER, 5]
	]
};
SkillInfo[SK.AG_DEADLY_PROJECTION] = {
	Name: 'AG_DEADLY_PROJECTION',
	SkillName: 'Deadly Projection',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_MYSTERY_ILLUSION, 3]]
};
SkillInfo[SK.AG_DESTRUCTIVE_HURRICANE] = {
	Name: 'AG_DESTRUCTIVE_HURRICANE',
	SkillName: 'Destructive Hurricane',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AG_TORNADO_STORM, 3]]
};
SkillInfo[SK.AG_RAIN_OF_CRYSTAL] = {
	Name: 'AG_RAIN_OF_CRYSTAL',
	SkillName: 'Crystal Rain',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WL_FROSTMISTY, 3]]
};
SkillInfo[SK.AG_MYSTERY_ILLUSION] = {
	Name: 'AG_MYSTERY_ILLUSION',
	SkillName: 'Mystery Illusion',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AG_SOUL_VC_STRIKE, 3],
		[SK.WL_HELLINFERNO, 3]
	]
};
SkillInfo[SK.AG_VIOLENT_QUAKE] = {
	Name: 'AG_VIOLENT_QUAKE',
	SkillName: 'Violent Quake',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_STRANTUM_TREMOR, 3]]
};
SkillInfo[SK.AG_SOUL_VC_STRIKE] = {
	Name: 'AG_SOUL_VC_STRIKE',
	SkillName: 'Soul Vulcan Strike',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.WL_SOULEXPANSION, 5],
		[SK.AG_TWOHANDSTAFF, 3]
	]
};
SkillInfo[SK.AG_STRANTUM_TREMOR] = {
	Name: 'AG_STRANTUM_TREMOR',
	SkillName: 'Stratum Tremor',
	MaxLv: 5,
	SpAmount: [35, 45, 55, 65, 75],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WL_SIENNAEXECRATE, 3]]
};
SkillInfo[SK.AG_ALL_BLOOM] = {
	Name: 'AG_ALL_BLOOM',
	SkillName: 'All Bloom',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_FLORAL_FLARE_ROAD, 3]]
};
SkillInfo[SK.AG_CRYSTAL_IMPACT] = {
	Name: 'AG_CRYSTAL_IMPACT',
	SkillName: 'Crystal Impact',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AG_RAIN_OF_CRYSTAL, 3]]
};
SkillInfo[SK.AG_TORNADO_STORM] = {
	Name: 'AG_TORNADO_STORM',
	SkillName: 'Tornado Storm',
	MaxLv: 5,
	SpAmount: [45, 55, 65, 75, 85],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WL_CHAINLIGHTNING, 3]]
};
SkillInfo[SK.AG_FLORAL_FLARE_ROAD] = {
	Name: 'AG_FLORAL_FLARE_ROAD',
	SkillName: 'Floral Flare Road',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WL_CRIMSONROCK, 3]]
};
SkillInfo[SK.AG_CLIMAX] = {
	Name: 'AG_CLIMAX',
	SkillName: 'Climax',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	ApAmount: [200, 200, 200, 200, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.WL_TETRAVORTEX, 5],
		[SK.AG_TWOHANDSTAFF, 3]
	]
};
SkillInfo[SK.AG_ASTRAL_STRIKE] = {
	Name: 'AG_ASTRAL_STRIKE',
	SkillName: 'Astral Strike',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.WL_COMET, 5],
		[SK.AG_MYSTERY_ILLUSION, 3],
		[SK.AG_DEADLY_PROJECTION, 3]
	]
};
SkillInfo[SK.AG_ROCK_DOWN] = {
	Name: 'AG_ROCK_DOWN',
	SkillName: 'Rock Down',
	MaxLv: 5,
	SpAmount: [65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_STRANTUM_TREMOR, 1]]
};
SkillInfo[SK.AG_STORM_CANNON] = {
	Name: 'AG_STORM_CANNON',
	SkillName: 'Storm Cannon',
	MaxLv: 5,
	SpAmount: [60, 70, 80, 90, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_TORNADO_STORM, 1]]
};
SkillInfo[SK.AG_CRIMSON_ARROW] = {
	Name: 'AG_CRIMSON_ARROW',
	SkillName: 'Crimson Arrow',
	MaxLv: 5,
	SpAmount: [65, 75, 85, 95, 105],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.AG_FLORAL_FLARE_ROAD, 1]]
};
SkillInfo[SK.AG_FROZEN_SLASH] = {
	Name: 'AG_FROZEN_SLASH',
	SkillName: 'Frozen Slash',
	MaxLv: 5,
	SpAmount: [45, 55, 65, 75, 85],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.AG_RAIN_OF_CRYSTAL, 1]]
};
SkillInfo[SK.AG_TWOHANDSTAFF] = {
	Name: 'AG_TWOHANDSTAFF',
	SkillName: 'Two-handed Staff Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.IQ_POWERFUL_FAITH] = {
	Name: 'IQ_POWERFUL_FAITH',
	SkillName: 'Powerful Faith',
	MaxLv: 5,
	SpAmount: [54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_WILL_OF_FAITH, 1]]
};
SkillInfo[SK.IQ_FIRM_FAITH] = {
	Name: 'IQ_FIRM_FAITH',
	SkillName: 'Firm Faith',
	MaxLv: 5,
	SpAmount: [54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_WILL_OF_FAITH, 1]]
};
SkillInfo[SK.IQ_WILL_OF_FAITH] = {
	Name: 'IQ_WILL_OF_FAITH',
	SkillName: 'Will of Faith',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.IQ_OLEUM_SANCTUM] = {
	Name: 'IQ_OLEUM_SANCTUM',
	SkillName: 'Oleum Sanctum',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.AL_HOLYWATER, 1],
		[SK.IQ_WILL_OF_FAITH, 3]
	]
};
SkillInfo[SK.IQ_SINCERE_FAITH] = {
	Name: 'IQ_SINCERE_FAITH',
	SkillName: 'Sincere Faith',
	MaxLv: 5,
	SpAmount: [54, 58, 62, 66, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_WILL_OF_FAITH, 1]]
};
SkillInfo[SK.IQ_FIRST_BRAND] = {
	Name: 'IQ_FIRST_BRAND',
	SkillName: 'First Brand',
	MaxLv: 5,
	SpAmount: [22, 29, 36, 43, 50],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [[SK.IQ_WILL_OF_FAITH, 2]]
};
SkillInfo[SK.IQ_FIRST_FAITH_POWER] = {
	Name: 'IQ_FIRST_FAITH_POWER',
	SkillName: 'First Faith Power',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.IQ_WILL_OF_FAITH, 3],
		[SK.IQ_FIRST_BRAND, 1]
	]
};
SkillInfo[SK.IQ_THIRD_PUNISH] = {
	Name: 'IQ_THIRD_PUNISH',
	SkillName: 'Third Punishment',
	MaxLv: 5,
	SpAmount: [56, 62, 68, 74, 80],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_SECOND_FAITH, 2]]
};
SkillInfo[SK.IQ_THIRD_FLAME_BOMB] = {
	Name: 'IQ_THIRD_FLAME_BOMB',
	SkillName: 'Third Flame Bomb',
	MaxLv: 5,
	SpAmount: [74, 78, 82, 86, 90],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_SECOND_FLAME, 2]]
};
SkillInfo[SK.IQ_THIRD_CONSECRATION] = {
	Name: 'IQ_THIRD_CONSECRATION',
	SkillName: 'Third Consecration',
	MaxLv: 5,
	SpAmount: [65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_SECOND_JUDGEMENT, 2]]
};
SkillInfo[SK.IQ_SECOND_FLAME] = {
	Name: 'IQ_SECOND_FLAME',
	SkillName: 'Second Flame',
	MaxLv: 5,
	SpAmount: [46, 52, 58, 64, 70],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_THIRD_EXOR_FLAME, 1]]
};
SkillInfo[SK.IQ_SECOND_FAITH] = {
	Name: 'IQ_SECOND_FAITH',
	SkillName: 'Second Faith',
	MaxLv: 5,
	SpAmount: [36, 42, 48, 54, 60],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_FIRST_FAITH_POWER, 1]]
};
SkillInfo[SK.IQ_SECOND_JUDGEMENT] = {
	Name: 'IQ_SECOND_JUDGEMENT',
	SkillName: 'Second Judgment',
	MaxLv: 5,
	SpAmount: [45, 50, 55, 60, 65],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [[SK.IQ_JUDGE, 1]]
};
SkillInfo[SK.IQ_EXPOSION_BLASTER] = {
	Name: 'IQ_EXPOSION_BLASTER',
	SkillName: 'Explosion Blaster',
	MaxLv: 5,
	SpAmount: [80, 90, 100, 110, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_OLEUM_SANCTUM, 1]]
};
SkillInfo[SK.IQ_MASSIVE_F_BLASTER] = {
	Name: 'IQ_MASSIVE_F_BLASTER',
	SkillName: 'Massive Flame Blaster',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.IQ_OLEUM_SANCTUM, 3],
		[SK.IQ_EXPOSION_BLASTER, 3],
		[SK.IQ_WILL_OF_FAITH, 5]
	]
};
SkillInfo[SK.IQ_JUDGE] = {
	Name: 'IQ_JUDGE',
	SkillName: 'Judgment',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	ApAmount: [100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_FIRST_FAITH_POWER, 1]]
};
SkillInfo[SK.IQ_THIRD_EXOR_FLAME] = {
	Name: 'IQ_THIRD_EXOR_FLAME',
	SkillName: 'Third Exorcism Flame',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	ApAmount: [150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IQ_JUDGE, 1]]
};
SkillInfo[SK.IG_GUARD_STANCE] = {
	Name: 'IG_GUARD_STANCE',
	SkillName: 'Guard Stance',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IG_SHIELD_MASTERY, 3]]
};
SkillInfo[SK.IG_GUARDIAN_SHIELD] = {
	Name: 'IG_GUARDIAN_SHIELD',
	SkillName: 'Guardian Shield',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IG_GUARD_STANCE, 2]]
};
SkillInfo[SK.IG_REBOUND_SHIELD] = {
	Name: 'IG_REBOUND_SHIELD',
	SkillName: 'Rebound Shield',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IG_GUARD_STANCE, 4]]
};
SkillInfo[SK.IG_SHIELD_MASTERY] = {
	Name: 'IG_SHIELD_MASTERY',
	SkillName: 'Shield Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.IG_SPEAR_SWORD_M] = {
	Name: 'IG_SPEAR_SWORD_M',
	SkillName: 'Spear Sword Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.IG_ATTACK_STANCE] = {
	Name: 'IG_ATTACK_STANCE',
	SkillName: 'Attack Stance',
	MaxLv: 5,
	SpAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IG_SPEAR_SWORD_M, 3]]
};
SkillInfo[SK.IG_ULTIMATE_SACRIFICE] = {
	Name: 'IG_ULTIMATE_SACRIFICE',
	SkillName: 'Ultimate Sacrifice',
	MaxLv: 5,
	SpAmount: [120, 120, 120, 120, 120],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.IG_REBOUND_SHIELD, 3],
		[SK.IG_GUARDIAN_SHIELD, 3]
	]
};
SkillInfo[SK.IG_HOLY_SHIELD] = {
	Name: 'IG_HOLY_SHIELD',
	SkillName: 'Holy Shield',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.IG_SHIELD_MASTERY, 5],
		[SK.IG_CROSS_RAIN, 3]
	]
};
SkillInfo[SK.IG_GRAND_JUDGEMENT] = {
	Name: 'IG_GRAND_JUDGEMENT',
	SkillName: 'Grand Judgment',
	MaxLv: 10,
	SpAmount: [41, 44, 47, 50, 53, 56, 59, 62, 65, 68],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.IG_OVERSLASH, 5],
		[SK.IG_SPEAR_SWORD_M, 5]
	]
};
SkillInfo[SK.IG_JUDGEMENT_CROSS] = {
	Name: 'IG_JUDGEMENT_CROSS',
	SkillName: 'Judgement Cross',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.IG_CROSS_RAIN, 5],
		[SK.IG_HOLY_SHIELD, 3]
	]
};
SkillInfo[SK.IG_SHIELD_SHOOTING] = {
	Name: 'IG_SHIELD_SHOOTING',
	SkillName: 'Shield Shooting',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.IG_SHIELD_MASTERY, 5],
		[SK.IG_ATTACK_STANCE, 2]
	]
};
SkillInfo[SK.IG_OVERSLASH] = {
	Name: 'IG_OVERSLASH',
	SkillName: 'Overslash',
	MaxLv: 10,
	SpAmount: [41, 44, 47, 50, 53, 56, 59, 62, 65, 68],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.IG_ATTACK_STANCE, 3]]
};
SkillInfo[SK.IG_CROSS_RAIN] = {
	Name: 'IG_CROSS_RAIN',
	SkillName: 'Cross Rain',
	MaxLv: 10,
	SpAmount: [50, 54, 58, 62, 66, 70, 74, 78, 82, 86],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.IG_SHIELD_MASTERY, 1]]
};
SkillInfo[SK.SHC_DANCING_KNIFE] = {
	Name: 'SHC_DANCING_KNIFE',
	SkillName: 'Dancing Knife',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SHC_SHADOW_SENSE, 3]]
};
SkillInfo[SK.SHC_SAVAGE_IMPACT] = {
	Name: 'SHC_SAVAGE_IMPACT',
	SkillName: 'Savage Impact',
	MaxLv: 10,
	SpAmount: [28, 31, 34, 37, 40, 43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7, 7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 3],
		[SK.GC_CROSSIMPACT, 5]
	]
};
SkillInfo[SK.SHC_SHADOW_SENSE] = {
	Name: 'SHC_SHADOW_SENSE',
	SkillName: 'Shadow Sense',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.SHC_ETERNAL_SLASH] = {
	Name: 'SHC_ETERNAL_SLASH',
	SkillName: 'Eternal Slash',
	MaxLv: 5,
	SpAmount: [40, 40, 40, 40, 40],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 5],
		[SK.SHC_DANCING_KNIFE, 3],
		[SK.GC_WEAPONBLOCKING, 3]
	]
};
SkillInfo[SK.SHC_ENCHANTING_SHADOW] = {
	Name: 'SHC_ENCHANTING_SHADOW',
	SkillName: 'Enchanting Shadow',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 3],
		[SK.GC_POISONINGWEAPON, 5]
	]
};
SkillInfo[SK.SHC_POTENT_VENOM] = {
	Name: 'SHC_POTENT_VENOM',
	SkillName: 'Potent Venom',
	MaxLv: 10,
	SpAmount: [15, 20, 25, 30, 35, 40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 5],
		[SK.SHC_ENCHANTING_SHADOW, 3]
	]
};
SkillInfo[SK.SHC_SHADOW_EXCEED] = {
	Name: 'SHC_SHADOW_EXCEED',
	SkillName: 'Shadow Exceed',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 7],
		[SK.SHC_ENCHANTING_SHADOW, 5],
		[SK.SHC_POTENT_VENOM, 3]
	]
};
SkillInfo[SK.SHC_FATAL_SHADOW_CROW] = {
	Name: 'SHC_FATAL_SHADOW_CROW',
	SkillName: 'Fatal Shadow Claw',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SHC_SHADOW_STAB, 5],
		[SK.SHC_IMPACT_CRATER, 5]
	]
};
SkillInfo[SK.SHC_SHADOW_STAB] = {
	Name: 'SHC_SHADOW_STAB',
	SkillName: 'Shadow Stab',
	MaxLv: 5,
	SpAmount: [45, 50, 55, 60, 65],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 5],
		[SK.SHC_DANCING_KNIFE, 5],
		[SK.SHC_ETERNAL_SLASH, 3],
		[SK.GC_CLOAKINGEXCEED, 5]
	]
};
SkillInfo[SK.SHC_IMPACT_CRATER] = {
	Name: 'SHC_IMPACT_CRATER',
	SkillName: 'Impact Crater',
	MaxLv: 5,
	SpAmount: [43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SHC_SHADOW_SENSE, 5],
		[SK.SHC_SAVAGE_IMPACT, 5],
		[SK.GC_ROLLINGCUTTER, 5],
		[SK.GC_WEAPONBLOCKING, 3]
	]
};
SkillInfo[SK.CD_REPARATIO] = {
	Name: 'CD_REPARATIO',
	SkillName: 'Repatatio',
	MaxLv: 5,
	SpAmount: [120, 120, 120, 120, 120],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.CD_MEDIALE_VOTUM, 3]]
};
SkillInfo[SK.CD_MEDIALE_VOTUM] = {
	Name: 'CD_MEDIALE_VOTUM',
	SkillName: 'Mediale Votum',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.CD_DILECTIO_HEAL, 3]]
};
SkillInfo[SK.CD_MACE_BOOK_M] = {
	Name: 'CD_MACE_BOOK_M',
	SkillName: 'Mace Book Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.CD_ARGUTUS_VITA] = {
	Name: 'CD_ARGUTUS_VITA',
	SkillName: 'Argutus Vita',
	MaxLv: 5,
	SpAmount: [30, 45, 60, 75, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.CD_MEDIALE_VOTUM, 3],
		[SK.CD_REPARATIO, 3]
	]
};
SkillInfo[SK.CD_ARGUTUS_TELUM] = {
	Name: 'CD_ARGUTUS_TELUM',
	SkillName: 'Argutus Telum',
	MaxLv: 5,
	SpAmount: [30, 45, 60, 75, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.CD_MEDIALE_VOTUM, 3],
		[SK.CD_REPARATIO, 3]
	]
};
SkillInfo[SK.CD_ARBITRIUM] = {
	Name: 'CD_ARBITRIUM',
	SkillName: 'Arbitrium',
	MaxLv: 10,
	SpAmount: [50, 60, 70, 80, 90, 100, 110, 120, 130, 140],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AB_ADORAMUS, 5],
		[SK.CD_FRAMEN, 3]
	]
};
SkillInfo[SK.CD_PRESENS_ACIES] = {
	Name: 'CD_PRESENS_ACIES',
	SkillName: 'Presens Acies',
	MaxLv: 5,
	SpAmount: [30, 45, 60, 75, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.CD_MEDIALE_VOTUM, 3],
		[SK.CD_REPARATIO, 3]
	]
};
SkillInfo[SK.CD_FIDUS_ANIMUS] = {
	Name: 'CD_FIDUS_ANIMUS',
	SkillName: 'Fidus Animus',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.CD_EFFLIGO] = {
	Name: 'CD_EFFLIGO',
	SkillName: 'Effligo',
	MaxLv: 10,
	SpAmount: [60, 60, 60, 60, 60, 60, 60, 60, 60, 60],
	ApAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.AB_ORATIO, 5],
		[SK.CD_PETITIO, 10]
	]
};
SkillInfo[SK.CD_COMPETENTIA] = {
	Name: 'CD_COMPETENTIA',
	SkillName: 'Competentia',
	MaxLv: 5,
	SpAmount: [60, 60, 60, 60, 60],
	ApAmount: [200, 200, 200, 200, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.CD_PRESENS_ACIES, 2],
		[SK.CD_ARGUTUS_TELUM, 2],
		[SK.CD_ARGUTUS_VITA, 2]
	]
};
SkillInfo[SK.CD_PNEUMATICUS_PROCELLA] = {
	Name: 'CD_PNEUMATICUS_PROCELLA',
	SkillName: 'Pneumaticus Procella',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.CD_FRAMEN, 5],
		[SK.CD_ARBITRIUM, 10]
	]
};
SkillInfo[SK.CD_DILECTIO_HEAL] = {
	Name: 'CD_DILECTIO_HEAL',
	SkillName: 'Dilectio Heal',
	MaxLv: 5,
	SpAmount: [50, 55, 60, 65, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AB_CHEAL, 3],
		[SK.AB_HIGHNESSHEAL, 3]
	]
};
SkillInfo[SK.CD_RELIGIO] = {
	Name: 'CD_RELIGIO',
	SkillName: 'Religio',
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AB_CLEMENTIA, 3],
		[SK.CD_DILECTIO_HEAL, 2]
	]
};
SkillInfo[SK.CD_BENEDICTUM] = {
	Name: 'CD_BENEDICTUM',
	SkillName: 'Benedictum',
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AB_CANTO, 3],
		[SK.CD_DILECTIO_HEAL, 2]
	]
};
SkillInfo[SK.CD_PETITIO] = {
	Name: 'CD_PETITIO',
	SkillName: 'Petitio',
	MaxLv: 10,
	SpAmount: [32, 34, 36, 38, 40, 42, 44, 46, 48, 50],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.AB_DUPLELIGHT, 10],
		[SK.CD_MACE_BOOK_M, 5]
	]
};
SkillInfo[SK.CD_FRAMEN] = {
	Name: 'CD_FRAMEN',
	SkillName: 'Flamen',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.AB_JUDEX, 10],
		[SK.CD_FIDUS_ANIMUS, 5]
	]
};
SkillInfo[SK.BO_BIONIC_PHARMACY] = {
	Name: 'BO_BIONIC_PHARMACY',
	SkillName: 'Bionic Pharmacy',
	MaxLv: 5,
	SpAmount: [30, 30, 30, 30, 30],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.GN_S_PHARMACY, 5]]
};
SkillInfo[SK.BO_BIONICS_M] = {
	Name: 'BO_BIONICS_M',
	SkillName: 'Bionics Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.BO_THE_WHOLE_PROTECTION] = {
	Name: 'BO_THE_WHOLE_PROTECTION',
	SkillName: 'Group Protection',
	MaxLv: 5,
	SpAmount: [220, 260, 300, 340, 380],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BO_BIONIC_PHARMACY, 5]]
};
SkillInfo[SK.BO_ADVANCE_PROTECTION] = {
	Name: 'BO_ADVANCE_PROTECTION',
	SkillName: 'Full Shadow Protection',
	MaxLv: 4,
	SpAmount: [120, 130, 140, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.BO_BIONIC_PHARMACY, 5]]
};
SkillInfo[SK.BO_ACIDIFIED_ZONE_WATER] = {
	Name: 'BO_ACIDIFIED_ZONE_WATER',
	SkillName: 'Acidified Zone (Water)',
	MaxLv: 5,
	SpAmount: [40, 52, 64, 76, 88],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.BO_ACIDIFIED_ZONE_WIND, 1]]
};
SkillInfo[SK.BO_ACIDIFIED_ZONE_GROUND] = {
	Name: 'BO_ACIDIFIED_ZONE_GROUND',
	SkillName: 'Acidified Zone (Earth)',
	MaxLv: 5,
	SpAmount: [40, 52, 64, 76, 88],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.BO_BIONICS_M, 3],
		[SK.BO_BIONIC_PHARMACY, 5]
	]
};
SkillInfo[SK.BO_ACIDIFIED_ZONE_FIRE] = {
	Name: 'BO_ACIDIFIED_ZONE_FIRE',
	SkillName: 'Acidified Zone (Fire)',
	MaxLv: 5,
	SpAmount: [40, 52, 64, 76, 88],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [[SK.BO_ACIDIFIED_ZONE_GROUND, 1]]
};
SkillInfo[SK.BO_ACIDIFIED_ZONE_WIND] = {
	Name: 'BO_ACIDIFIED_ZONE_WIND',
	SkillName: 'Acidified Zone (Wind)',
	MaxLv: 5,
	SpAmount: [40, 52, 64, 76, 88],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.BO_BIONICS_M, 3],
		[SK.BO_BIONIC_PHARMACY, 5]
	]
};
SkillInfo[SK.BO_WOODENWARRIOR] = {
	Name: 'BO_WOODENWARRIOR',
	SkillName: 'Create Wooden Warrior',
	MaxLv: 5,
	SpAmount: [100, 120, 140, 160, 180],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BO_CREEPER, 3]]
};
SkillInfo[SK.BO_WOODEN_FAIRY] = {
	Name: 'BO_WOODEN_FAIRY',
	SkillName: 'Create Wooden Fairy',
	MaxLv: 5,
	SpAmount: [120, 155, 180, 205, 230],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BO_CREEPER, 3]]
};
SkillInfo[SK.BO_CREEPER] = {
	Name: 'BO_CREEPER',
	SkillName: 'Create Creeper',
	MaxLv: 5,
	SpAmount: [80, 96, 112, 128, 144],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.BO_BIONICS_M, 5]]
};
SkillInfo[SK.BO_RESEARCHREPORT] = {
	Name: 'BO_RESEARCHREPORT',
	SkillName: 'Research Report',
	MaxLv: 1,
	SpAmount: [60],
	ApAmount: [100],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.BO_ACIDIFIED_ZONE_FIRE, 3],
		[SK.BO_ACIDIFIED_ZONE_WATER, 3]
	]
};
SkillInfo[SK.BO_HELLTREE] = {
	Name: 'BO_HELLTREE',
	SkillName: 'Create Hell Tree',
	MaxLv: 5,
	SpAmount: [100, 100, 100, 100, 100],
	ApAmount: [100, 100, 100, 100, 100],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.BO_WOODENWARRIOR, 3],
		[SK.BO_WOODEN_FAIRY, 3]
	]
};
SkillInfo[SK.WH_ADVANCED_TRAP] = {
	Name: 'WH_ADVANCED_TRAP',
	SkillName: 'Advanced Trap',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.RA_RESEARCHTRAP, 3]]
};
SkillInfo[SK.WH_WIND_SIGN] = {
	Name: 'WH_WIND_SIGN',
	SkillName: 'Wind Sign',
	MaxLv: 5,
	SpAmount: [100, 90, 80, 70, 60],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WH_NATUREFRIENDLY, 5]]
};
SkillInfo[SK.WH_NATUREFRIENDLY] = {
	Name: 'WH_NATUREFRIENDLY',
	SkillName: "Nature's Friend",
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.WH_HAWKRUSH] = {
	Name: 'WH_HAWKRUSH',
	SkillName: 'Hawk Rush',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WH_HAWK_M, 1]]
};
SkillInfo[SK.WH_HAWK_M] = {
	Name: 'WH_HAWK_M',
	SkillName: 'Hawk Mastery',
	MaxLv: 1,
	SpAmount: [5],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.HT_STEELCROW, 1]]
};
SkillInfo[SK.WH_CALAMITYGALE] = {
	Name: 'WH_CALAMITYGALE',
	SkillName: 'Calamity Gale',
	MaxLv: 1,
	SpAmount: [300],
	ApAmount: [200],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.WH_GALESTORM, 5],
		[SK.WH_WIND_SIGN, 5]
	]
};
SkillInfo[SK.WH_HAWKBOOMERANG] = {
	Name: 'WH_HAWKBOOMERANG',
	SkillName: 'Hawk Boomerang',
	MaxLv: 5,
	SpAmount: [120, 120, 120, 120, 120],
	ApAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WH_HAWKRUSH, 5]]
};
SkillInfo[SK.WH_GALESTORM] = {
	Name: 'WH_GALESTORM',
	SkillName: 'Gale Storm',
	MaxLv: 10,
	SpAmount: [80, 91, 102, 113, 124, 135, 146, 157, 168, 179],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WH_CRESCIVE_BOLT, 3]]
};
SkillInfo[SK.WH_DEEPBLINDTRAP] = {
	Name: 'WH_DEEPBLINDTRAP',
	SkillName: 'Deep Blind Trap',
	MaxLv: 5,
	SpAmount: [50, 53, 56, 59, 62],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WH_ADVANCED_TRAP, 3]]
};
SkillInfo[SK.WH_SOLIDTRAP] = {
	Name: 'WH_SOLIDTRAP',
	SkillName: 'Solid Trap',
	MaxLv: 5,
	SpAmount: [70, 80, 90, 100, 110],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WH_ADVANCED_TRAP, 3]]
};
SkillInfo[SK.WH_SWIFTTRAP] = {
	Name: 'WH_SWIFTTRAP',
	SkillName: 'Swift Trap',
	MaxLv: 5,
	SpAmount: [60, 62, 64, 66, 68],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WH_DEEPBLINDTRAP, 1]]
};
SkillInfo[SK.WH_CRESCIVE_BOLT] = {
	Name: 'WH_CRESCIVE_BOLT',
	SkillName: 'Crescive Bolt',
	MaxLv: 10,
	SpAmount: [55, 60, 65, 70, 75, 80, 85, 90, 95, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.RA_AIMEDBOLT, 5]]
};
SkillInfo[SK.WH_FLAMETRAP] = {
	Name: 'WH_FLAMETRAP',
	SkillName: 'Flame Trap',
	MaxLv: 5,
	SpAmount: [40, 44, 48, 52, 56],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.WH_SOLIDTRAP, 1]]
};
SkillInfo[SK.TR_STAGE_MANNER] = {
	Name: 'TR_STAGE_MANNER',
	SkillName: 'Stage Etiquette',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.TR_RETROSPECTION] = {
	Name: 'TR_RETROSPECTION',
	SkillName: 'Retrospection',
	MaxLv: 1,
	SpAmount: [1],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [[SK.TR_STAGE_MANNER, 1]]
};
SkillInfo[SK.TR_MYSTIC_SYMPHONY] = {
	Name: 'TR_MYSTIC_SYMPHONY',
	SkillName: 'Mystic Symphony',
	MaxLv: 1,
	SpAmount: [250],
	ApAmount: [100],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.TR_METALIC_FURY, 1],
		[SK.TR_ROSEBLOSSOM, 5]
	]
};
SkillInfo[SK.TR_KVASIR_SONATA] = {
	Name: 'TR_KVASIR_SONATA',
	SkillName: 'Kvasir Sonata',
	MaxLv: 1,
	SpAmount: [300],
	ApAmount: [100],
	bSeperateLv: false,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.TR_ROKI_CAPRICCIO, 1],
		[SK.TR_NIPELHEIM_REQUIEM, 1]
	]
};
SkillInfo[SK.TR_ROSEBLOSSOM] = {
	Name: 'TR_ROSEBLOSSOM',
	SkillName: 'Rose Blossom',
	MaxLv: 5,
	SpAmount: [215, 230, 245, 260, 275],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.TR_RHYTHMSHOOTING, 3]]
};
SkillInfo[SK.TR_RHYTHMSHOOTING] = {
	Name: 'TR_RHYTHMSHOOTING',
	SkillName: 'Rhythm Shooting',
	MaxLv: 5,
	SpAmount: [80, 92, 104, 116, 128],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9]
};
SkillInfo[SK.TR_METALIC_FURY] = {
	Name: 'TR_METALIC_FURY',
	SkillName: 'Metallic Fury',
	MaxLv: 5,
	SpAmount: [120, 132, 144, 156, 168],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.TR_SOUNDBLEND, 1]]
};
SkillInfo[SK.TR_SOUNDBLEND] = {
	Name: 'TR_SOUNDBLEND',
	SkillName: 'Sound Blend',
	MaxLv: 5,
	SpAmount: [80, 92, 104, 116, 128],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.WM_METALICSOUND, 5]]
};
SkillInfo[SK.TR_GEF_NOCTURN] = {
	Name: 'TR_GEF_NOCTURN',
	SkillName: 'Geffenia Nocturne',
	MaxLv: 5,
	SpAmount: [120, 160, 200, 240, 280],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_STAGE_MANNER, 3]]
};
SkillInfo[SK.TR_ROKI_CAPRICCIO] = {
	Name: 'TR_ROKI_CAPRICCIO',
	SkillName: 'Loki Capriccio',
	MaxLv: 5,
	SpAmount: [120, 160, 200, 240, 280],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_JAWAII_SERENADE, 1]]
};
SkillInfo[SK.TR_AIN_RHAPSODY] = {
	Name: 'TR_AIN_RHAPSODY',
	SkillName: 'Miner Rhapsody',
	MaxLv: 5,
	SpAmount: [120, 160, 200, 240, 280],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_STAGE_MANNER, 3]]
};
SkillInfo[SK.TR_MUSICAL_INTERLUDE] = {
	Name: 'TR_MUSICAL_INTERLUDE',
	SkillName: 'Musical Interlude',
	MaxLv: 5,
	SpAmount: [171, 182, 193, 204, 215],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_AIN_RHAPSODY, 1]]
};
SkillInfo[SK.TR_JAWAII_SERENADE] = {
	Name: 'TR_JAWAII_SERENADE',
	SkillName: 'Jawaii Serenade',
	MaxLv: 5,
	SpAmount: [140, 150, 160, 170, 180],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_GEF_NOCTURN, 1]]
};
SkillInfo[SK.TR_NIPELHEIM_REQUIEM] = {
	Name: 'TR_NIPELHEIM_REQUIEM',
	SkillName: 'Nifflheim Requiem',
	MaxLv: 5,
	SpAmount: [120, 160, 200, 240, 280],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.TR_MUSICAL_INTERLUDE, 1],
		[SK.TR_PRON_MARCH, 1]
	]
};
SkillInfo[SK.TR_PRON_MARCH] = {
	Name: 'TR_PRON_MARCH',
	SkillName: 'Prontera March',
	MaxLv: 5,
	SpAmount: [140, 150, 160, 170, 180],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.TR_AIN_RHAPSODY, 1]]
};
SkillInfo[SK.ABC_DAGGER_AND_BOW_M] = {
	Name: 'ABC_DAGGER_AND_BOW_M',
	SkillName: 'Dagger Bow Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.ABC_MAGIC_SWORD_M] = {
	Name: 'ABC_MAGIC_SWORD_M',
	SkillName: 'Magic Sword Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SC_REPRODUCE, 5],
		[SK.SC_AUTOSHADOWSPELL, 5]
	]
};
SkillInfo[SK.ABC_STRIP_SHADOW] = {
	Name: 'ABC_STRIP_SHADOW',
	SkillName: 'Divest Shadow',
	MaxLv: 5,
	SpAmount: [29, 33, 37, 41, 45],
	bSeperateLv: true,
	AttackRange: [3, 3, 3, 3, 3],
	_NeedSkillList: [
		[SK.SC_STRIPACCESSARY, 1],
		[SK.ABC_DAGGER_AND_BOW_M, 7]
	]
};
SkillInfo[SK.ABC_ABYSS_DAGGER] = {
	Name: 'ABC_ABYSS_DAGGER',
	SkillName: 'Abyss Dagger',
	MaxLv: 5,
	SpAmount: [40, 45, 50, 55, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.SC_FATALMENACE, 5],
		[SK.ABC_DAGGER_AND_BOW_M, 3]
	]
};
SkillInfo[SK.ABC_UNLUCKY_RUSH] = {
	Name: 'ABC_UNLUCKY_RUSH',
	SkillName: 'Misfortune Rush',
	MaxLv: 5,
	SpAmount: [30, 35, 40, 45, 50],
	bSeperateLv: true,
	AttackRange: [7, 7, 7, 7, 7],
	_NeedSkillList: [
		[SK.ABC_ABYSS_DAGGER, 3],
		[SK.ABC_DAGGER_AND_BOW_M, 4]
	]
};
SkillInfo[SK.ABC_CHAIN_REACTION_SHOT] = {
	Name: 'ABC_CHAIN_REACTION_SHOT',
	SkillName: 'Chain Reaction Shot',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.SC_TRIANGLESHOT, 5],
		[SK.ABC_DAGGER_AND_BOW_M, 3]
	]
};
SkillInfo[SK.ABC_FROM_THE_ABYSS] = {
	Name: 'ABC_FROM_THE_ABYSS',
	SkillName: 'From the Abyss',
	MaxLv: 5,
	SpAmount: [40, 50, 60, 70, 80],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.ABC_MAGIC_SWORD_M, 3]]
};
SkillInfo[SK.ABC_ABYSS_SLAYER] = {
	Name: 'ABC_ABYSS_SLAYER',
	SkillName: 'Abyss Slayer',
	MaxLv: 10,
	SpAmount: [100, 100, 100, 100, 100, 100, 100, 100, 100, 100],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.ABC_ABYSS_DAGGER, 5],
		[SK.ABC_DEFT_STAB, 5]
	]
};
SkillInfo[SK.ABC_ABYSS_STRIKE] = {
	Name: 'ABC_ABYSS_STRIKE',
	SkillName: 'Omega Abyss Strike',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.ABC_FROM_THE_ABYSS, 3],
		[SK.ABC_ABYSS_SQUARE, 3]
	]
};
SkillInfo[SK.ABC_DEFT_STAB] = {
	Name: 'ABC_DEFT_STAB',
	SkillName: 'Deft Stab',
	MaxLv: 10,
	SpAmount: [28, 31, 34, 37, 40, 43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
	_NeedSkillList: [
		[SK.ABC_ABYSS_DAGGER, 3],
		[SK.ABC_DAGGER_AND_BOW_M, 5]
	]
};
SkillInfo[SK.ABC_ABYSS_SQUARE] = {
	Name: 'ABC_ABYSS_SQUARE',
	SkillName: 'Abyss Square',
	MaxLv: 5,
	SpAmount: [65, 75, 85, 95, 105],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.ABC_MAGIC_SWORD_M, 5],
		[SK.ABC_FROM_THE_ABYSS, 1]
	]
};
SkillInfo[SK.ABC_FRENZY_SHOT] = {
	Name: 'ABC_FRENZY_SHOT',
	SkillName: 'Frenzy Shot',
	MaxLv: 10,
	SpAmount: [40, 45, 50, 55, 60, 65, 70, 75, 80, 85],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [
		[SK.ABC_CHAIN_REACTION_SHOT, 3],
		[SK.ABC_DAGGER_AND_BOW_M, 5]
	]
};
SkillInfo[SK.MT_AXE_STOMP] = {
	Name: 'MT_AXE_STOMP',
	SkillName: 'Axe Stomp',
	MaxLv: 5,
	SpAmount: [25, 30, 35, 40, 45],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MT_TWOAXEDEF, 5]]
};
SkillInfo[SK.MT_RUSH_QUAKE] = {
	Name: 'MT_RUSH_QUAKE',
	SkillName: 'Rush Quake',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9, 9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.MT_AXE_STOMP, 5]]
};
SkillInfo[SK.MT_M_MACHINE] = {
	Name: 'MT_M_MACHINE',
	SkillName: 'Manufacture Machine',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1]
};
SkillInfo[SK.MT_A_MACHINE] = {
	Name: 'MT_A_MACHINE',
	SkillName: 'Activate Attack Device',
	MaxLv: 5,
	SpAmount: [43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MT_AXE_STOMP, 3],
		[SK.MT_M_MACHINE, 3]
	]
};
SkillInfo[SK.MT_D_MACHINE] = {
	Name: 'MT_D_MACHINE',
	SkillName: 'Activate Defense Device',
	MaxLv: 5,
	SpAmount: [43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MT_M_MACHINE, 1]]
};
SkillInfo[SK.MT_TWOAXEDEF] = {
	Name: 'MT_TWOAXEDEF',
	SkillName: 'Two-handed Axe Defense',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.MT_ABR_M] = {
	Name: 'MT_ABR_M',
	SkillName: 'ABR Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.MT_M_MACHINE, 1]]
};
SkillInfo[SK.MT_SUMMON_ABR_BATTLE_WARIOR] = {
	Name: 'MT_SUMMON_ABR_BATTLE_WARIOR',
	SkillName: 'ABR: Battle Warrior',
	MaxLv: 4,
	SpAmount: [30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [[SK.MT_ABR_M, 1]]
};
SkillInfo[SK.MT_SUMMON_ABR_DUAL_CANNON] = {
	Name: 'MT_SUMMON_ABR_DUAL_CANNON',
	SkillName: 'ABR: Dual Cannon',
	MaxLv: 4,
	SpAmount: [30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MT_ABR_M, 3],
		[SK.MT_SUMMON_ABR_BATTLE_WARIOR, 2]
	]
};
SkillInfo[SK.MT_SUMMON_ABR_MOTHER_NET] = {
	Name: 'MT_SUMMON_ABR_MOTHER_NET',
	SkillName: 'ABR: Mother Net',
	MaxLv: 4,
	SpAmount: [30, 40, 50, 60],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MT_ABR_M, 5],
		[SK.MT_SUMMON_ABR_BATTLE_WARIOR, 3],
		[SK.MT_SUMMON_ABR_DUAL_CANNON, 3]
	]
};
SkillInfo[SK.MT_SUMMON_ABR_INFINITY] = {
	Name: 'MT_SUMMON_ABR_INFINITY',
	SkillName: 'ABR: Infinity',
	MaxLv: 4,
	SpAmount: [30, 40, 50, 60],
	ApAmount: [200, 200, 200, 200],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1],
	_NeedSkillList: [
		[SK.MT_ABR_M, 10],
		[SK.MT_SUMMON_ABR_BATTLE_WARIOR, 4],
		[SK.MT_SUMMON_ABR_DUAL_CANNON, 4],
		[SK.MT_SUMMON_ABR_MOTHER_NET, 4]
	]
};
SkillInfo[SK.ABR_DUAL_CANNON_FIRE] = {
	Name: 'ABR_DUAL_CANNON_FIRE',
	SkillName: 'Dual Cannon Fire',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.ABR_BATTLE_BUSTER] = {
	Name: 'ABR_BATTLE_BUSTER',
	SkillName: 'Battle Buster',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.ABR_NET_REPAIR] = {
	Name: 'ABR_NET_REPAIR',
	SkillName: 'Net Repair',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.ABR_NET_SUPPORT] = {
	Name: 'ABR_NET_SUPPORT',
	SkillName: 'Net Support',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.ABR_INFINITY_BUSTER] = {
	Name: 'ABR_INFINITY_BUSTER',
	SkillName: 'Infinity Buster',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_MAGIC_BOOK_M] = {
	Name: 'EM_MAGIC_BOOK_M',
	SkillName: 'Magic Book Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
};
SkillInfo[SK.EM_SPELL_ENCHANTING] = {
	Name: 'EM_SPELL_ENCHANTING',
	SkillName: 'Spell Enchanting',
	MaxLv: 5,
	SpAmount: [43, 46, 49, 52, 55],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 5]]
};
SkillInfo[SK.EM_ACTIVITY_BURN] = {
	Name: 'EM_ACTIVITY_BURN',
	SkillName: 'AP Burn',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_SPELL_ENCHANTING, 3]]
};
SkillInfo[SK.EM_INCREASING_ACTIVITY] = {
	Name: 'EM_INCREASING_ACTIVITY',
	SkillName: 'Increase AP',
	MaxLv: 5,
	SpAmount: [30, 40, 50, 60, 70],
	ApAmount: [50, 50, 50, 50, 50],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_ACTIVITY_BURN, 5]]
};
SkillInfo[SK.EM_DIAMOND_STORM] = {
	Name: 'EM_DIAMOND_STORM',
	SkillName: 'Diamond Storm',
	MaxLv: 5,
	SpAmount: [84, 88, 92, 96, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 2]]
};
SkillInfo[SK.EM_LIGHTNING_LAND] = {
	Name: 'EM_LIGHTNING_LAND',
	SkillName: 'Lightning Land',
	MaxLv: 5,
	SpAmount: [65, 70, 80, 85, 95],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 2]]
};
SkillInfo[SK.EM_VENOM_SWAMP] = {
	Name: 'EM_VENOM_SWAMP',
	SkillName: 'Venom Swamp',
	MaxLv: 5,
	SpAmount: [84, 88, 92, 96, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 2]]
};
SkillInfo[SK.EM_CONFLAGRATION] = {
	Name: 'EM_CONFLAGRATION',
	SkillName: 'Conflagration',
	MaxLv: 5,
	SpAmount: [70, 80, 90, 100, 110],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 2]]
};
SkillInfo[SK.EM_TERRA_DRIVE] = {
	Name: 'EM_TERRA_DRIVE',
	SkillName: 'Terra Drive',
	MaxLv: 5,
	SpAmount: [84, 88, 92, 96, 100],
	bSeperateLv: true,
	AttackRange: [9, 9, 9, 9, 9],
	_NeedSkillList: [[SK.EM_MAGIC_BOOK_M, 2]]
};
SkillInfo[SK.EM_ELEMENTAL_SPIRIT_M] = {
	Name: 'EM_ELEMENTAL_SPIRIT_M',
	SkillName: 'Elemental Spirit Mastery',
	MaxLv: 10,
	SpAmount: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
	bSeperateLv: false,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.SO_EL_SYMPATHY, 1]]
};
SkillInfo[SK.EM_SUMMON_ELEMENTAL_ARDOR] = {
	Name: 'EM_SUMMON_ELEMENTAL_ARDOR',
	SkillName: 'Summon Elemental: Ador',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SO_SUMMON_AGNI, 3],
		[SK.EM_ELEMENTAL_SPIRIT_M, 1],
		[SK.EM_CONFLAGRATION, 1]
	]
};
SkillInfo[SK.EM_SUMMON_ELEMENTAL_DILUVIO] = {
	Name: 'EM_SUMMON_ELEMENTAL_DILUVIO',
	SkillName: 'Summon Elemental: Diluvio',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SO_SUMMON_AQUA, 3],
		[SK.EM_ELEMENTAL_SPIRIT_M, 1],
		[SK.EM_DIAMOND_STORM, 1]
	]
};
SkillInfo[SK.EM_SUMMON_ELEMENTAL_PROCELLA] = {
	Name: 'EM_SUMMON_ELEMENTAL_PROCELLA',
	SkillName: 'Summon Elemental: Procella',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SO_SUMMON_VENTUS, 3],
		[SK.EM_ELEMENTAL_SPIRIT_M, 1],
		[SK.EM_LIGHTNING_LAND, 1]
	]
};
SkillInfo[SK.EM_SUMMON_ELEMENTAL_TERREMOTUS] = {
	Name: 'EM_SUMMON_ELEMENTAL_TERREMOTUS',
	SkillName: 'Summon Elemental: Terremotus',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SO_SUMMON_TERA, 3],
		[SK.EM_ELEMENTAL_SPIRIT_M, 1],
		[SK.EM_TERRA_DRIVE, 1]
	]
};
SkillInfo[SK.EM_SUMMON_ELEMENTAL_SERPENS] = {
	Name: 'EM_SUMMON_ELEMENTAL_SERPENS',
	SkillName: 'Summon Elemental: Serpens',
	MaxLv: 1,
	SpAmount: [100],
	bSeperateLv: true,
	AttackRange: [1],
	_NeedSkillList: [
		[SK.SO_SUMMON_AGNI, 3],
		[SK.SO_SUMMON_AQUA, 3],
		[SK.SO_SUMMON_VENTUS, 3],
		[SK.SO_SUMMON_TERA, 3],
		[SK.EM_ELEMENTAL_SPIRIT_M, 1],
		[SK.EM_VENOM_SWAMP, 1]
	]
};
SkillInfo[SK.EM_ELEMENTAL_BUSTER] = {
	Name: 'EM_ELEMENTAL_BUSTER',
	SkillName: 'Elemental Buster',
	MaxLv: 10,
	SpAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	ApAmount: [150, 150, 150, 150, 150, 150, 150, 150, 150, 150],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
	_NeedSkillList: [
		[SK.EM_SUMMON_ELEMENTAL_SERPENS, 1],
		[SK.EM_SUMMON_ELEMENTAL_TERREMOTUS, 1],
		[SK.EM_SUMMON_ELEMENTAL_PROCELLA, 1],
		[SK.EM_SUMMON_ELEMENTAL_DILUVIO, 1],
		[SK.EM_ELEMENTAL_SPIRIT_M, 5],
		[SK.EM_SUMMON_ELEMENTAL_ARDOR, 1]
	]
};
SkillInfo[SK.EM_ELEMENTAL_VEIL] = {
	Name: 'EM_ELEMENTAL_VEIL',
	SkillName: 'Elemental Veil',
	MaxLv: 5,
	SpAmount: [70, 75, 80, 85, 90],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	_NeedSkillList: [[SK.EM_ELEMENTAL_SPIRIT_M, 3]]
};
SkillInfo[SK.EM_EL_FLAMETECHNIC] = {
	Name: 'EM_EL_FLAMETECHNIC',
	SkillName: 'Flame Technique',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_FLAMEARMOR] = {
	Name: 'EM_EL_FLAMEARMOR',
	SkillName: 'Flame Armor',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_FLAMEROCK] = {
	Name: 'EM_EL_FLAMEROCK',
	SkillName: 'Flame Rock',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [7]
};
SkillInfo[SK.EM_EL_COLD_FORCE] = {
	Name: 'EM_EL_COLD_FORCE',
	SkillName: 'Cold Force',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_CRYSTAL_ARMOR] = {
	Name: 'EM_EL_CRYSTAL_ARMOR',
	SkillName: 'Crystal Armor',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_AGE_OF_ICE] = {
	Name: 'EM_EL_AGE_OF_ICE',
	SkillName: 'Ice Age',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_GRACE_BREEZE] = {
	Name: 'EM_EL_GRACE_BREEZE',
	SkillName: 'Grace Breeze',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_EYES_OF_STORM] = {
	Name: 'EM_EL_EYES_OF_STORM',
	SkillName: 'Eye of the Storm',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_STORM_WIND] = {
	Name: 'EM_EL_STORM_WIND',
	SkillName: 'Storm Wind',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [7]
};
SkillInfo[SK.EM_EL_EARTH_CARE] = {
	Name: 'EM_EL_EARTH_CARE',
	SkillName: 'Earth Care',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_STRONG_PROTECTION] = {
	Name: 'EM_EL_STRONG_PROTECTION',
	SkillName: 'Strong Protection',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_AVALANCHE] = {
	Name: 'EM_EL_AVALANCHE',
	SkillName: 'Avalanche',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [7]
};
SkillInfo[SK.EM_EL_DEEP_POISONING] = {
	Name: 'EM_EL_DEEP_POISONING',
	SkillName: 'Deep Poisoning',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_POISON_SHIELD] = {
	Name: 'EM_EL_POISON_SHIELD',
	SkillName: 'Poison Shield',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [1]
};
SkillInfo[SK.EM_EL_DEADLY_POISON] = {
	Name: 'EM_EL_DEADLY_POISON',
	SkillName: 'Deadly Poison',
	MaxLv: 1,
	SpAmount: [0],
	bSeperateLv: true,
	AttackRange: [7]
};
SkillInfo[SK.NPC_DEADLYCURSE2] = {
	Name: 'NPC_DEADLYCURSE2',
	SkillName: 'Wide Deadly Curse',
	MaxLv: 5,
	SpAmount: [0, 0, 0, 0, 0],
	bSeperateLv: true,
	AttackRange: [1, 1, 1, 1, 1],
	SkillScale: [
		[5, 5],
		[7, 7],
		[9, 9],
		[11, 11],
		[13, 13]
	]
};

export default SkillInfo;
