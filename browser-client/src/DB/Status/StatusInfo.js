/**
 * DB/Status/StatusInfo.js
 *
 * Icons of status
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import SC from './StatusConst.js';

// Hardcoded color
const COLOR_TITLE_BUFF = 'rgb(155, 202, 155)';
const COLOR_TITLE_DEBUFF = 'rgb(250, 100, 100)';
const COLOR_TITLE_TOGGLE = 'rgb(190, 190, 250)';
const COLOR_SYSTEM = 'rgb(255, 255,   0)';
const COLOR_TIME = 'rgb(255, 176,  98)';

// TODO: find icon status: 40-49 | 60 | 63-64 | 66-67 | 70-86 | 88-89 | 64-621 | ???

const StatusInfo = {};

StatusInfo[SC.OVERTHRUSTMAX] = {
	icon: '\x69\x5f\xbf\xc0\xb9\xf6\xb8\xc6\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Maximum Over Thrust', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases weapon damage.'],
		['Increases the possibility of breaking the weapon.']
	]
};

StatusInfo[SC.SUFFRAGIUM] = {
	icon: '\xbc\xf6\xc1\xdd\xc0\xba\xc7\xcf\xb7\xe7\xc0\xc7\xbf\xec\xbf\xef.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Suffragium', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces cast time.']]
};

StatusInfo[SC.OVERTHRUST] = {
	icon: '\xbf\xc0\xb9\xf6\xc6\xae\xb7\xaf\xbd\xba\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Over Thrust', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases weapon damage.'],
		['Increases the possibility of breaking the weapon.']
	]
};

StatusInfo[SC.AUTOBERSERK] = {
	icon: '\xb1\xdd\xb0\xad\xba\xd2\xb1\xab.tga',
	descript: [['Auto Berserk', COLOR_TITLE_BUFF], ['Rage when close to death']]
};

StatusInfo[SC.BEYOND_OF_WARCRY] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Warcry of Beyond', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases physical attack power'],
		['Decreases magic attack power']
	]
};

StatusInfo[SC.SWORDREJECT] = {
	icon: 'icon04.tga',
	descript: [
		['Sword Reject', COLOR_TITLE_BUFF],
		['Reflects damage back to attacking monsters'],
		['(for all monster attacks)'],
		['Damage received is reduced by 1/2'],
		['You receive the other 1/2 of damage']
	]
};

StatusInfo[SC.MANU_DEF] = {
	icon: 'efst_def.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Manuk's Will", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Decrease physical/magical damage'],
		['from the monsters in Manuk field']
	]
};

StatusInfo[SC.CONCENTRATION] = {
	icon: '\xc1\xfd\xc1\xdf\xb7\xc2\xc7\xe2\xbb\xf3.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Attention Concentration', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases DEX, AGI'],
		['Reveals nearby hidden enemies']
	]
};

StatusInfo[SC.GRIFFON] = {
	icon: '\xb6\xf3\xc0\xcc\xb5\xf9.tga',
	descript: [['Riding Griffon', COLOR_TITLE_TOGGLE]]
};

StatusInfo[SC.GS_MADNESSCANCEL] = {
	icon: 'i_madness.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Madness Canceller (Last Stand)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK'],
		['Increases Attack Speed'],
		['Immobilized']
	]
};

StatusInfo[SC.GS_ACCURACY] = {
	icon: 'i_accuracy',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Increasing Accuracy (Increase Accuracy)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Accuracy'],
		['Increases DEX'],
		['Increases AGI']
	]
};

StatusInfo[SC.FOOD_STR] = {
	icon: 'str_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.HALLUCINATIONWALK] = {
	icon: '\xc7\xd2\xb7\xe7\xbd\xc3\xb3\xd7\xc0\xcc\xbc\xc7\xbf\xf6\xc5\xa9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Hallucination Walk', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Improves Evasion Rate'],
		['Chance to evade Magical Damage.']
	]
};

StatusInfo[SC.STORMKICK_ON] = {
	icon: 'i_stormkick.tga',
	descript: [
		['Whirlwind Kick (Tornado Kick)', COLOR_TITLE_BUFF],
		['When attacking an enemy'],
		['there is a chance to prepare a Whirlwind Kick']
	]
};

StatusInfo[SC.KAUPE] = {
	icon: 'i_kaupe.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Kaupe', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Chance to evade an enemy attack.']]
};

StatusInfo[SC.SHIELDSPELL_DEF] = {
	icon: 'lg_shieldspell_¹æ.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Shield Spell (DEF)', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Invokes a magical spell based on DEF']]
};

StatusInfo[SC.WARMER] = {
	icon: '\xbf\xf6\xb8\xd3.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Warmer', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Removes Frozen/Freezing status'],
		['Immunity to Frozen/Freezing status'],
		['Recovers HP every 3 seconds']
	]
};

StatusInfo[SC.PROTECT_MDEF] = {
	icon: '\xb8\xb6\xb9\xfd\xb9\xe6\xbe\xee\xc6\xf7\xbc\xc7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Magic Armor Potions', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases resistance to magical attacks']
	]
};

StatusInfo[SC.STAR_COMFORT] = {
	icon: 'i_starcomfort.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Comfort of the Stars', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases ASPD']]
};

StatusInfo[SC.FOOD_CRITICALSUCCESSVALUE] = {
	icon: 'gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Improves CRIT']]
};

StatusInfo[SC.PROPERTYTELEKINESIS] = {
	icon: 'i_p_tele.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['Warm Wind', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Ghost Property']]
};

StatusInfo[SC.GLOOMYDAY] = {
	icon: '\xbc\xf6\xc1\xdd\xc0\xba\xc7\xcf\xb7\xe7\xc0\xc7\xbf\xec\xbf\xef.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Gloomy Day', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases the damage of specific skills'],
		['Reduces FLEE, ASPD']
	]
};

StatusInfo[SC.SIRCLEOFNATURE] = {
	icon: '\xbc\xf8\xc8\xaf\xc7\xcf\xb4\xc2\xc0\xda\xbf\xac\xc0\xc7\xbc\xd2\xb8\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [["Circle of Nature's Sound", COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Consumes SP and recovers HP']]
};

StatusInfo[SC.DEADLYINFECT] = {
	icon: '\xb5\xa5\xb5\xe9\xb8\xae\xc0\xce\xc6\xe5\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Deadly Infect', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When attacking'],
		['or being attacked'],
		['your status effects are applies to them']
	]
};

StatusInfo[SC.SYMPHONY_LOVE] = {
	icon: '\xbf\xac\xc0\xce\xb5\xe9\xc0\xbb\xc0\xa7\xc7\xd1\xbd\xc9\xc6\xf7\xb4\xcf.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Symphony of Love', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases MDEF']]
};

StatusInfo[SC.BANDING] = {
	icon: 'lg_banding.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Banding', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Banding status']]
};

StatusInfo[SC.NJ_BUNSINJYUTSU] = {
	icon: 'i_bunsin.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Illusionary Shadow', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Avoids a certain number of physical attacks'],
		['Magical attacks cannot be avoided']
	]
};

StatusInfo[SC.WUGRIDER] = {
	icon: 'wolfmount.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Warg Rider', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Bows are Disabled'],
		['Warg Skills are only allowed']
	]
};

StatusInfo[SC.ATKER_BLOOD] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['SP Consumption Reduction Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduced SP Consumption of skills by 15%']
	]
};

StatusInfo[SC.BODYPAINT] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Body Painting', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reveals hidden enemies'],
		['Chance to inflict Blind to enemies'],
		['Reduces Enemy ASPD']
	]
};

StatusInfo[SC.NJ_UTSUSEMI] = {
	icon: 'i_maemi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Cicada Skin Shedding', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Avoids a certain number of physical attacks'],
		['When avoiding, move in the opposite direction of the attacker']
	]
};

StatusInfo[SC.POISONINGWEAPON] = {
	icon: '\xc6\xf7\xc0\xcc\xc1\xee\xb4\xd7\xbf\xfe\xc6\xf9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Poisoning Weapon', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Applies the poison coated on your weapon to the target']
	]
};

StatusInfo[SC.CASH_DEATHPENALTY] = {
	icon: 'death.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['Life Insurance', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['No EXP loss upon death']]
};

StatusInfo[SC.GS_ADJUSTMENT] = {
	icon: 'i_adjustment.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Adjustment', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduces HIT'],
		['Increases FLEE'],
		['Reduces damage of incoming ranged physical attacks']
	]
};

StatusInfo[SC.AUTOSPELL] = {
	icon: '\xbf\xc0\xc5\xe4\xbd\xba\xc6\xe7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Auto Spell', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When physically attacking'],
		['the selected skill will automatically cast without casting time.'],
		['SP consumed is 2/3 the regular amount'],
		['Skill will not cast without sufficient SP']
	]
};

StatusInfo[SC.DEC_AGI] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xb0\xa8\xbc\xd2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Decrease Agility', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces Movement Speed'],
		['Reduces ASPD']
	]
};

StatusInfo[SC.NOEQUIPWEAPON] = {
	icon: '\xbd\xba\xc6\xae\xb8\xb3\xbf\xfe\xc6\xf9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Off Status', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Weapons cannot be worn']]
};

StatusInfo[SC.SHIELDSPELL_MDEF] = {
	icon: 'lg_shieldspell_¸¶.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Shield Spell (MDEF)', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Invokes a magical spell based on MDEF']]
};

StatusInfo[SC.AUTOGUARD] = {
	icon: '\xbf\xc0\xc5\xe4\xb0\xa1\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Auto Guard', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Has a chance to block physical attacks']]
};

StatusInfo[SC.TAROTCARD] = {
	icon: '\x69\x5f\xc5\xb8\xb7\xce\xc4\xab\xb5\xe5.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Tarot Card of Fate', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Applies one of 14 cards and their effects']
	]
};

StatusInfo[SC.FEARBREEZE] = {
	icon: 'ra_fearbreeze.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Fear Breeze', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When attacking with a bow'],
		['there is a chance to cause additional attacks']
	]
};

StatusInfo[SC.GN_CARTBOOST] = {
	icon: 'icon07.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Cart Boost', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Movement Speed']]
};

StatusInfo[SC.SHIELDSPELL_REF] = {
	icon: 'lg_shieldspell_¿¬.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Shield Spell (Refine)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Invokes a magical effect based on refine']
	]
};

StatusInfo[SC.FOOD_INT_CASH] = {
	icon: 'int_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases INT']]
};

StatusInfo[SC.NOEQUIPSHIELD] = {
	icon: '\xbd\xba\xc6\xae\xb8\xb3\xbd\xaf\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Shield Off Status', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Shields cannot be worn']]
};

StatusInfo[SC.MELTDOWN] = {
	icon: 'icon03.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Meltdown', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When attacking a player'],
		['there is a chance to destroy his/her weapon/armor'],
		['When attacking a monster'],
		["the monster's attack and defense are reduced"]
	]
};

StatusInfo[SC.QUAGMIRE] = {
	icon: '\xc4\xe2\xb1\xd7\xb8\xb6\xc0\xcc\xbe\xee.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Quagmire', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Reduces Movement Speed'], ['Reduces AGI/DEX']]
};

StatusInfo[SC.KAIZEL] = {
	icon: 'i_kaizel.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Kaizel', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Cast time not affected by DEX'],
		['Upon death, you will revive with Kyrie Eleison for 2 seconds']
	]
};

StatusInfo[SC.CR_SHRINK] = {
	icon: 'i_shrink.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Shrink', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When Autoguarding attacks'],
		['there is a chance to push the attack back']
	]
};

StatusInfo[SC.FOOD_VIT] = {
	icon: 'vit_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases VIT']]
};

StatusInfo[SC.PARRYING] = {
	icon: 'icon10.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Parrying', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Blocks physical attacks by chance']]
};

StatusInfo[SC.PROTECTWEAPON] = {
	icon: '\xc4\xc9\xb9\xcc\xc4\xc3\xc7\xc1\xb7\xce\xc5\xd8\xbc\xc7\x5b\xbf\xfe\xc6\xf9\x5d.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Chemical Protection (Weapon)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Prevents weapon from being stripped/broken']
	]
};

StatusInfo[SC.FOOD_AGI] = {
	icon: 'agi_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases AGI']]
};

StatusInfo[SC.INC_AGI] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xc1\xf5\xb0\xa1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Increase agility', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Movement Speed'],
		['Increases Attack Speed']
	]
};

StatusInfo[SC.SHOUT] = {
	icon: '\xb0\xed\xbc\xba\xc1\xf6\xb8\xa3\xb1\xe2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Loud Exclamation (Crazy Uproar)', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.CASH_RECEIVEITEM] = {
	icon: 'item.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['When killing monsters'], ['the drop chance is doubled']]
};

StatusInfo[SC.SPL_DEF] = {
	icon: 'efst_def.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Honey Jam', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Decrease Physical, Magical damage received'],
		['from the monsters in Splendid field']
	]
};

StatusInfo[SC.ILLUSION] = {
	icon: '\xc8\xaf\xb0\xa2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Illusion', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Screen distortion'],
		['Shows more damage'],
		['Randomly interrupts casting']
	]
};

StatusInfo[SC.HOVERING] = {
	icon: '\xc8\xa3\xb9\xf6\xb8\xb5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Hovering', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Traps and some ground skills do not have any effect']
	]
};

StatusInfo[SC.BENEDICTIO] = {
	icon: '\xbc\xba\xc3\xbc\xb0\xad\xba\xb9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Enchants Armor with Holy Property']]
};

StatusInfo[SC.WEAPONBLOCKING] = {
	icon: '\xbf\xfe\xc6\xf9\xba\xed\xb7\xce\xc5\xb7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Weapon Blocking', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When hit by close range physical attacks'],
		['there is a chance to nullify the damage']
	]
};

StatusInfo[SC.ANGELUS] = {
	icon: '\xbe\xc8\xc1\xa9\xb7\xe7\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Angelus', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases VIT DEF']]
};

StatusInfo[SC.MARSHOFABYSS] = {
	icon: 'marshofabyss.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Marsh of Abyss', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Slows Movement'], ['Reduces DEF, Flee']]
};

StatusInfo[SC.STEALTHFIELD] = {
	icon: '\xbd\xba\xc5\xda\xbd\xba\xc7\xca\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Stealth Field', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Cloaks everyone in a radius around caster'],
		['Consumes SP while active'],
		['Reduces Movement Speed']
	]
};

StatusInfo[SC.ADRENALINE2] = {
	icon: '\xbe\xc6\xb5\xe5\xb7\xb9\xb3\xaf\xb8\xb0\xb7\xaf\xbd\xac.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Full Adrenaline Rush', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Weapon ASPD except Bows']]
};

StatusInfo[SC.MANU_MATK] = {
	icon: 'efst_matk.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Manuk's Faith", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increase Magical attack against'],
		['all monsters in Manuk Field']
	]
};

StatusInfo[SC.NOEQUIPARMOR] = {
	icon: '\xbd\xba\xc6\xae\xb8\xb3\xbe\xc6\xb8\xd3.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Armor Off Status', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Armor cannot be worn']]
};

StatusInfo[SC.RENOVATIO] = {
	icon: '\xb7\xb9\xb3\xeb\xb9\xd9\xc6\xbc\xbf\xc0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Renovatio', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recovers HP every 5 seconds'],
		['When used on Undead monsters'],
		['it deals high damage according to skill level']
	]
};

StatusInfo[SC.HIDING] = {
	icon: '\xc7\xcf\xc0\xcc\xb5\xf9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Hiding', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Avoids enemy attacks by hiding in the ground'],
		['Can be discovered with detection skills']
	]
};

StatusInfo[SC.WEIGHTOVER50] = {
	icon: '\xb9\xab\xb0\xd4\x35\x30\xc0\xcc\xbb\xf3.tga',
	descript: [['Overweight 50%', COLOR_TITLE_DEBUFF], ['HP/SP will not be restored']]
};

StatusInfo[SC.STRUP] = {
	icon: 'i_run.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Spurt', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases STR'],
		['if the user is unequipped'],
		['and the skill level is high enough']
	]
};

StatusInfo[SC.NOEQUIPHELM] = {
	icon: '\xbd\xba\xc6\xae\xb8\xb3\xc7\xef\xb8\xa7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Headgear Off Status', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Headgear cannot be worn']]
};

StatusInfo[SC.ATTHASTE_POTION3] = {
	icon: '\xb0\xf8\xbc\xd3\xb9\xb0\xbe\xe0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['Berserk Potion', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases ASPD']]
};

StatusInfo[SC.ENDURE] = {
	icon: '\xc0\xce\xb5\xe0\xbe\xee.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Endure', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Enables attacking and movement'],
		['while receiving damage']
	]
};

StatusInfo[SC.TURNKICK_ON] = {
	icon: 'i_turnkick.tga',
	descript: [['Ready Turn Kick', COLOR_TITLE_BUFF], ['When attacking,'], ["there's a chance to prepare a Turn Kick"]]
};

StatusInfo[SC.ENCHANTPOISON] = {
	icon: '\xc0\xce\xc2\xf9\xc6\xae\xc6\xf7\xc0\xcc\xc1\xf0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Enchant Poison', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Poison Property']]
};

StatusInfo[SC.SPL_ATK] = {
	icon: 'efst_atk.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Pinguicula's Pickled Fruit", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increase physical ATK against'],
		['all monsters in Splendid fields']
	]
};

StatusInfo[SC.BLESSING] = {
	icon: '\xba\xed\xb7\xb9\xbd\xcc.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Blessing', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases DEX, INT and STR'],
		['Recovers from a few status effects']
	]
};

StatusInfo[SC.ONEHANDQUICKEN] = {
	icon: 'i_onehand.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['One-hand Quicken', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Using One-handed Weapon'],
		['Increases ASPD']
	]
};

StatusInfo[SC.SPEARQUICKEN] = {
	icon: '\xbd\xba\xc7\xc7\xbe\xee\xc4\xfb\xc5\xab.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Spear Quicken', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],

		['Increase ASPD when using Spear'],
		['Increase Critical rate'],

		['Increase Flee']
	]
};

StatusInfo[SC.BROKENWEAPON] = {
	icon: '\xb9\xab\xb1\xe2\xc6\xc4\xb1\xab.tga',
	descript: [['Weapon is damaged.', COLOR_TITLE_DEBUFF]]
};

StatusInfo[SC.ASSUMPTIO] = {
	icon: 'icon05.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Assumptio', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces damage taken']]
};

StatusInfo[SC.MAXIMIZE] = {
	icon: '\xb8\xc6\xbd\xc3\xb8\xb6\xc0\xcc\xc1\xee\xc6\xc4\xbf\xf6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Maximize Power', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases damage to the maximum'],
		['Drains SP over time']
	]
};

StatusInfo[SC.PROTECTSHIELD] = {
	icon: '\xc4\xc9\xb9\xcc\xc4\xc3\xc7\xc1\xb7\xce\xc5\xd8\xbc\xc7\x5b\xbd\xaf\xb5\xe5\x5d.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Chemical Protection (Shield)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Prevents shield from being stripped/broken']
	]
};

StatusInfo[SC.MAGNIFICAT] = {
	icon: '\xb8\xb6\xb4\xcf\xc7\xc7\xc4\xb1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Magnificat', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases natural SP Recovery Speed']]
};

StatusInfo[SC.ATTHASTE_POTION1] = {
	icon: '\xb0\xf8\xbc\xd3\xb9\xb0\xbe\xe0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Concentration Potion', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases ASPD']]
};

StatusInfo[SC.POISONREACT] = {
	icon: '\xc6\xf7\xc0\xcc\xc1\xf0\xb8\xae\xbe\xd7\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Poison React', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Counters a Poison attack with a one-time attack'],
		['When hit by a physical non-poison attack,'],
		['there is a chance to cast Envenom on target']
	]
};

StatusInfo[SC.MOVHASTE_HORSE] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xc1\xf5\xb0\xa1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases Movement Speed']]
};

StatusInfo[SC.CRESCENTELBOW] = {
	icon: '\xb1\xe2\xb8\xae\xbf\xa1\xbf\xa4\xb7\xb9\xc0\xcc\xbc\xd5.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Crescent Elbow', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		["Attempts to counter opponent's attack"],
		['Knocks back opponent and deals damage'],
		['You still take some of the damage'],
		['Does not affect boss monsters', COLOR_SYSTEM]
	]
};

StatusInfo[SC.SONG_OF_MANA] = {
	icon: '\xb8\xb6\xb3\xaa\xc0\xc7\xb3\xeb\xb7\xa1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Song of Mana', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Recovers SP every 5 seconds']]
};

StatusInfo[SC.KAAHI] = {
	icon: 'i_kaahi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Kaahi', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Whenever you are hit by non-skills'],
		['SP is consumed and HP is recovered']
	]
};

StatusInfo[SC.ECHOSONG] = {
	icon: '\xb8\xb6\xbd\xba\xc4\xbf\xb7\xb9\xc0\xcc\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Echo Song', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases DEF']]
};

StatusInfo[SC.PRESERVE] = {
	icon: 'i_\xc7\xc1\xb8\xae\xc0\xfa\xba\xea.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Preserve', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Allows you to keep copied skill']]
};

StatusInfo[SC.WEAPONPERFECT] = {
	icon: '\xbf\xfe\xc6\xf9\xc6\xdb\xc6\xe5\xbc\xc7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Weapon Perfection', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Applies 100% damage to'],
		['small, medium and large monsters']
	]
};

StatusInfo[SC.PROVOKE] = {
	icon: '\xc7\xc1\xb7\xce\xba\xb8\xc5\xa9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Provoke', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces VIT DEF'], ['Increases ATK']]
};

StatusInfo[SC.MOVHASTE_POTION] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xc1\xf5\xb0\xa1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases Movement Speed']]
};

StatusInfo[SC.EDP] = {
	icon: 'i_\xc4\xa1\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Enchant Deadly Poison', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Applies a deadly poison to weapon'],
		['Damage increase does not apply to boss monsters', COLOR_SYSTEM]
	]
};

StatusInfo[SC.JOINTBEAT] = {
	icon: '\xb0\xfc\xc0\xfd\xb0\xf8\xb0\xdd.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Joint Beat', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Applies various status effects'],
		['due to joint damage.']
	]
};

StatusInfo[SC.PROVIDENCE] = {
	icon: '\xbd\xc5\xc0\xc7\xb6\xe6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Providence', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increased resistance'],
		['to undead and demon monsters']
	]
};

StatusInfo[SC.FIGHTINGSPIRIT] = {
	icon: 'rk_eisir.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rune Stone: Fighting Spirit', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK'],
		['Increases ASPD of caster']
	]
};

StatusInfo[SC.FOOD_VIT_CASH] = {
	icon: 'vit_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases VIT']]
};

StatusInfo[SC.SATURDAY_NIGHT_FEVER] = {
	icon: '\xbb\xf5\xc5\xcd\xb5\xa5\xc0\xcc\xb3\xaa\xc0\xcc\xc6\xae\xc7\xc7\xb9\xf6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [
		['Wild', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Decreases HP/SP per 3 seconds'],
		['Damage increased, Defense and Evasion dropped'],
		['Skills and items cannot be used.']
	]
};

StatusInfo[SC.TRUESIGHT] = {
	icon: 'icon09.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['True Sight', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases all stats'],
		['Increases ATK, HIT, CRIT']
	]
};

StatusInfo[SC.CASH_PLUSONLYJOBEXP] = {
	icon: 'job.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases Job EXP acquired.']]
};

StatusInfo[SC.ARMOR_PROPERTY] = {
	icon: '\xc4\xb3\xbd\xc3\xb0\xa9\xbf\xca\xbc\xd3\xbc\xba\xba\xce\xbf\xa9.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Armor Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Changes your Elemental Property']]
};

StatusInfo[SC.TENSIONRELAX] = {
	icon: 'icon08.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Tension Relax', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases natural HP recovery']]
};

StatusInfo[SC.DEATHHURT] = {
	icon: '\xbb\xf3\xc3\xb3\xbf\xc0\xbf\xb0\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Death Hurt (Contaminated Wound Poison)', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces effects of healing skills']
	]
};

StatusInfo[SC.IMPOSITIO] = {
	icon: '\xc0\xd3\xc6\xf7\xbd\xc3\xc6\xbc\xbf\xc0\xb8\xb6\xb4\xa9\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Impositio Manus', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Weapon damage']]
};

StatusInfo[SC.LEECHESEND] = {
	icon: '\xb0\xc5\xb8\xd3\xb8\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Leech End', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Drains HP every second']]
};

StatusInfo[SC.REPRODUCE] = {
	icon: '\xb8\xae\xc7\xc1\xb7\xce\xb5\xe0\xbd\xba.tga',
	descript: [
		['Reproduce', COLOR_TITLE_BUFF],
		['Activates when targetted by a skill'],
		['Only one skill can be learnt']
	]
};

StatusInfo[SC.ACCELERATION] = {
	icon: '\xbf\xa2\xbc\xbf\xb7\xb9\xc0\xcc\xbc\xc7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Acceleration', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases movement speed of the Magic Gear']]
};

StatusInfo[SC.NJ_NEN] = {
	icon: 'i_nen.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Soul', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases INT, STR']]
};

StatusInfo[SC.FORCEOFVANGUARD] = {
	icon: '\xc6\xf7\xbd\xba\xbf\xc0\xba\xea\xb9\xf0\xb0\xa1\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Force of Vanguard', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Max HP, Defense increased'],
		['When physicalled attacked there is a chance to earn a rage counter'],
		['SP consumed while active']
	]
};

StatusInfo[SC.RG_CCONFINE_M] = {
	icon: 'i_closeconfine.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Close Confine', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Both Players cannot Move'],
		['Increases FLEE'],
		['Does not affect boss monsters', COLOR_SYSTEM]
	]
};

StatusInfo[SC.TRICKDEAD] = {
	icon: '\xc1\xd7\xc0\xba\xc3\xb4\xc7\xcf\xb1\xe2.tga',
	descript: [['Trick Dead (Play Dead)', COLOR_TITLE_TOGGLE], ['Pretend Dead Status']]
};

StatusInfo[SC.PROPERTYWATER] = {
	icon: '\xc7\xc1\xb7\xce\xbd\xba\xc6\xae\xbf\xfe\xc6\xf9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Frost Weapon (Endow Tsunami)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Enchants Weapon with Water Property']
	]
};

StatusInfo[SC.ADORAMUS] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xb0\xa8\xbc\xd2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Adoramus', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Blinds and reduces Movement Speed']]
};

StatusInfo[SC.GENTLETOUCH_ENERGYGAIN] = {
	icon: '\xc1\xa1\xc7\xf7\xb1\xb8.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Gentle Touch - Energy Gain', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When attacked or when attacking, there is a chance to'],
		['earn a Spirit Sphere']
	]
};

StatusInfo[SC.NEUTRALBARRIER] = {
	icon: '\xb4\xba\xc6\xae\xb7\xb2\xb9\xe8\xb8\xae\xbe\xee.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Neutral Barrier', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['DEF/MDEF up'],
		['Neutralizes ranged attacks']
	]
};

StatusInfo[SC.EARTHSCROLL] = {
	icon: '\xb1\xdb\xb7\xce\xb8\xae\xbe\xc6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Happy Break (Enjoyable Rest)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When sitting with another Taekwon'],
		['A certain amount of SP is regained'],
		['Chance for Earth Spike scrolls to not be destroyed when used']
	]
};

StatusInfo[SC.FALCON] = {
	icon: '\xc6\xc8\xc4\xdc.tga',
	descript: [['Falconry Mastery', COLOR_TITLE_TOGGLE], ['Falcon Rental']]
};

StatusInfo[SC.TWOHANDQUICKEN] = {
	icon: '\xc5\xf5\xc7\xda\xb5\xe5\xc4\xfb\xc5\xab.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Two Hand Quicken', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When using two handed weapons,'],
		['increases ASPD']
	]
};

StatusInfo[SC.SUN_COMFORT] = {
	icon: 'i_suncomfort.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Comfort of the Sun', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Improves VIT DEF']]
};

StatusInfo[SC.KYRIE] = {
	icon: '\xb1\xe2\xb8\xae\xbf\xa1\xbf\xa4\xb7\xb9\xc0\xcc\xbc\xd5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Kyrie Eleison', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['A defensive barrier that blocks a certain number of attacks']
	]
};

StatusInfo[SC.PROTECTARMOR] = {
	icon: '\xc4\xc9\xb9\xcc\xc4\xc3\xc7\xc1\xb7\xce\xc5\xd8\xbc\xc7\x5b\xbe\xc6\xb8\xd3\x5d.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Chemical Protection (Armor)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Prevents body Armor from being stripped/broken']
	]
};

StatusInfo[SC.GIANTGROWTH] = {
	icon: 'rk_turisuss.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rune Stone: Giant Growth', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases STR'],
		['There is a chance to vastly increase damage'],
		['of close range physical attacks'],
		['Chance to destroy weapon with each hit']
	]
};

StatusInfo[SC.STR_SCROLL] = {
	icon: 'str_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.AB_SECRAMENT] = {
	icon: '\xbc\xbc\xc5\xa9\xb6\xf3\xb8\xe0\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Sacrament', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces fixed casting time']]
};

StatusInfo[SC.PARALYSE] = {
	icon: '\xb0\xa8\xb0\xa2\xb8\xb6\xba\xf1\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Paralyze', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduces Attack Speed'],
		['Reduces FLEE'],
		['Reduces Movement Speed']
	]
};

StatusInfo[SC.PROPERTYGROUND] = {
	icon: '\xbb\xe7\xc0\xcc\xc1\xee\xb9\xcd\xbf\xfe\xc6\xf9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Seismic Weapon', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Earth Property']]
};

StatusInfo[SC.DOUBLECASTING] = {
	icon: 'i_\xb4\xf5\xba\xed\xc4\xb3\xbd\xba\xc6\xc3.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Double Casting', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When casting a Bolt Skill'],
		['there is a chance to cast another automatically']
	]
};

StatusInfo[SC.RG_CCONFINE_S] = {
	icon: 'i_closeconfine.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Close Confine', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Both Players cannot Move'],
		['Increases FLEE'],
		['Does not affect Boss']
	]
};

StatusInfo[SC.OVERHEAT] = {
	icon: '\xbf\xc0\xb9\xf6\xc8\xf7\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['Over Heat', COLOR_TITLE_BUFF], ['Heating caused by skill use'], ['Drains HP every second']]
};

StatusInfo[SC.SPL_MATK] = {
	icon: 'efst_matk.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Cornus's Tear", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],

		['Increase Magical attack against'],
		['all monsters in Splendide fields']
	]
};

StatusInfo[SC.DEEP_SLEEP] = {
	icon: '\xbe\xc8\xbd\xc4\xc0\xc7\xc0\xda\xc0\xe5\xb0\xa1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Deep Sleep Status', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Damage Received by 1.5 times'],
		['Recovers HP/SP every 2 seconds']
	]
};

StatusInfo[SC.RECOGNIZEDSPELL] = {
	icon: 'recognizespell.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Recognized Spell', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Magic Skills deals Max Damage'],
		['All skills consumes more SP']
	]
};

StatusInfo[SC.TARGET_ASPD] = {
	icon: 'plusmagicpower.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Max SP increased, SP consumption reduced']]
};

StatusInfo[SC.FOOD_BASICAVOIDANCE] = {
	icon: 'gogi.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Improves FLEE']]
};

StatusInfo[SC.DEFENDER] = {
	icon: '\xb5\xf0\xc6\xe6\xb4\xf5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Defender (Defending Aura)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduce Damage from Ranged Physical Attack'],
		['Reduces Movement Speed and Attack Speed']
	]
};

StatusInfo[SC.WEAPONPROPERTY] = {
	icon: '\xbf\xac\xb8\xb6\xc1\xa6.tga', //CUSTOM
	haveTimeLimit: 0,
	descript: [['Granted a weapon property']]
};

StatusInfo[SC.S_LIFEPOTION] = {
	icon: '\xbc\xd2\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Small Life Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recoverys HP every 5 seconds'],
		['No effect if Berserk State is active']
	]
};

StatusInfo[SC.FOOD_LUK] = {
	icon: 'luk_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases LUK']]
};

StatusInfo[SC.BLOODING] = {
	icon: '\xc3\xe2\xc7\xf7\xbb\xf3\xc5\xc2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Bleeding', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['HP, SP recovery disabled'],
		['HP lost every 10 seconds']
	]
};

StatusInfo[SC.REFRESH] = {
	icon: 'rk_nauthiz.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rune Stone: Refresh', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Removes all debuffs when cast'],
		['Grants immunity to debuffs'],
		['Recovers a certain amount of HP']
	]
};

StatusInfo[SC.FOOD_LUK_CASH] = {
	icon: 'luk_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases LUK']]
};

StatusInfo[SC.BROKENARMOR] = {
	icon: '\xb0\xa9\xbf\xca\xc6\xc4\xb1\xab.tga',
	descript: [['Armor is damaged', COLOR_TITLE_DEBUFF]]
};

StatusInfo[SC.DODGE_ON] = {
	icon: 'i_dodge.tga',
	descript: [
		['Dodge', COLOR_TITLE_BUFF],
		['Allows Flying Kick to be used as a counter'],
		['When receiving enemy magic attack'],
		['there is a chance of completely avoiding it'],
		['If Spurt is also active'],
		['chance of avoiding physical attacks as well']
	]
};

StatusInfo[SC.TARGET_BLOOD] = {
	icon: '\xbc\xd2\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resistance Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases resistance to'],
		['Stun, Frozen, Stone, Sleep, Silence'],
		['Blind, Curse, Poison, Bleeding, Confusion']
	]
};

StatusInfo[SC.MELODYOFSINK] = {
	icon: '\xb8\xe1\xb7\xce\xb5\xf0\xbf\xc0\xba\xea\xbd\xcc\xc5\xa9.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Melody of Sink', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Magical Damage'],
		['Decreases Physical Damage']
	]
};

StatusInfo[SC.CRUCIS] = {
	icon: '\xbd\xc3\xb1\xd7\xb3\xd1\xc5\xa9\xb7\xe7\xbd\xc3\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Signum Crucis', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces Undead and Demon monsters DEF']]
};

StatusInfo[SC.SLOWCAST] = {
	icon: 'slowcast.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Slow Cast', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Increases Casting Time']]
};

StatusInfo[SC.PROPERTYWIND] = {
	icon: '\xb6\xf3\xc0\xcc\xc6\xae\xb4\xd7\xb7\xce\xb4\xf5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Lightning Loader', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Wind Property']]
};

StatusInfo[SC.ENCHANTBLADE] = {
	icon: 'rk_enchant.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Enchant Blade', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Adds magic damage to physical attacks']]
};

StatusInfo[SC.ADRENALINE] = {
	icon: '\xbe\xc6\xb5\xe5\xb7\xb9\xb3\xaf\xb8\xb0\xb7\xaf\xbd\xac.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Adrenaline Rush', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Attack Speed of'],
		['Axes and Mace weapons']
	]
};

StatusInfo[SC.MAGICMUSHROOM] = {
	icon: '\xbf\xf4\xc0\xbd\xb9\xf6\xbc\xb8.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Magic Mushroom (Laughing Poison Mushroom)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Uses Smile Emoticon'],
		['Casts random spells every 4 seconds'],
		['Drains HP every 4 seconds']
	]
};

StatusInfo[SC.CASH_PLUSEXP] = {
	icon: 'exp.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases acquired EXP']]
};

StatusInfo[SC.ATTHASTE_POTION2] = {
	icon: '\xb0\xf8\xbc\xd3\xb9\xb0\xbe\xe0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Awakening Potion', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases ASPD']]
};

StatusInfo[SC.TOXIN] = {
	icon: '\xbd\xc5\xb0\xe6\xb8\xb6\xba\xf1\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Toxin', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Interferes with skills every 10 seconds'],
		['Phen card is ignored'],
		['Every 10 seconds, SP is consumed']
	]
};

StatusInfo[SC.RAISINGDRAGON] = {
	icon: '\xc0\xe1\xb7\xe6\xbd\xc2\xc3\xb5.tga',
	descript: [
		['Rising Dragon', COLOR_TITLE_BUFF],
		['Maximum Spheres Increased'],
		['Increases Maximum HP/SP'],
		['Increases Attack Speed'],
		['Maintains Fury State'],
		['Slowly Drains HP per seconds']
	]
};

StatusInfo[SC.HARMONIZE] = {
	icon: '\xc7\xcf\xb8\xf0\xb3\xaa\xc0\xcc\xc1\xee.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Harmonize', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases all Stats']]
};

StatusInfo[SC.CHASEWALK2] = {
	icon: 'i_\xc3\xbc\xc0\xcc\xbd\xba\xc8\xfb.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.FOOD_STR_CASH] = {
	icon: 'str_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.CLOAKINGEXCEED] = {
	icon: '\xc6\xf7\xc0\xcc\xc1\xee\xb4\xd7\xbf\xfe\xc6\xf9.tga',
	descript: [
		['Cloaking Exceed', COLOR_TITLE_BUFF],
		['Hides from Insects and Demon types too.'],
		['Remains hidden until a certain number of hits received.'],
		['Increases Movement Speed']
	]
};

StatusInfo[SC.ASSUMPTIO2] = {
	icon: 'icon05.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Assumptio', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Defense']]
};

StatusInfo[SC.THORNS_TRAP] = {
	icon: '\xb0\xa1\xbd\xc3\xb3\xaa\xb9\xab\xb5\xa3.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Thorn Trap', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Periodically applies damage']]
};

StatusInfo[SC.SLOWPOISON] = {
	icon: '\xbd\xbd\xb7\xce\xbf\xec\xc6\xf7\xc0\xcc\xc1\xf0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Slow Poison', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Temporarily stops Poison Damage']]
};

StatusInfo[SC.CLOAKING] = {
	icon: '\xc5\xac\xb7\xce\xc5\xb7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Cloaking', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Invisible']]
};

StatusInfo[SC.PARTYFLEE] = {
	icon: 'icon06.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases Flee Rate']]
};

StatusInfo[SC.CRITICALPERCENT] = {
	icon: '\xbd\xba\xc6\xae\xb6\xf3\xc0\xcc\xc5\xb7.tga', //CUSTOM?
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Varnish', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Critical']]
};

StatusInfo[SC.INSPIRATION] = {
	icon: 'lg_inspiration.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Inspiration', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Hit rate, Stats, Damage, Max HP increased'],
		['All buffs and status effects removed'],
		['Drains HP, SP over time'],
		['Cannot receive status effects'],
		['Lose a percentage of your EXP']
	]
};

StatusInfo[SC.UNLIMITED_HUMMING_VOICE] = {
	icon: '\xbe\xf0\xb8\xae\xb9\xcc\xc6\xbc\xb5\xe5\xc7\xe3\xb9\xd6\xba\xb8\xc0\xcc\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Unlimited Humming Voice', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		["Affected target's skills"],
		['increase their SP consumption']
	]
};

StatusInfo[SC.FOOD_DEX] = {
	icon: 'dex_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases DEX']]
};

StatusInfo[SC.ANALYZE] = {
	icon: '\xbe\xd6\xb3\xce\xb6\xf3\xc0\xcc\xc1\xee.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Analyze', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reduces Physical and Magical Defense']]
};

StatusInfo[SC.GENTLETOUCH_REVITALIZE] = {
	icon: '\xc1\xa1\xc7\xf7\xc8\xb0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Gentle Touch - Revitalize', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases VIT, Max HP'],
		['Increases DEF'],
		['Increases natural HP Recovery'],
		['Movement speed increased']
	]
};

StatusInfo[SC.COUNTER_ON] = {
	icon: 'i_counter.tga',
	descript: [['Prepare Counter Kick', COLOR_TITLE_BUFF], ['Hit an enemy'], ['to be ready for a counter kick']]
};

StatusInfo[SC.GLORIA] = {
	icon: '\xb1\xdb\xb7\xce\xb8\xae\xbe\xc6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Gloria', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases LUK']]
};

StatusInfo[SC.RUSH_WINDMILL] = {
	icon: '\xc7\xb3\xc2\xf7\xb8\xa6\xc7\xe2\xc7\xd8\xb5\xb9\xb0\xdd.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Rush Windmill Attack', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Damage']]
};

StatusInfo[SC.PYREXIA] = {
	icon: '\xbf\xad\xba\xb4\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Pyrexia', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Dark and Hallucinating state']]
};

StatusInfo[SC.DANCE_WITH_WUG] = {
	icon: '\xbf\xf6\xb1\xd7\xbf\xcd\xc7\xd4\xb2\xb2\xc3\xe3\xc0\xbb.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Dance With Warg', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ASPD'],
		['Reduces Fixed casting time']
	]
};

StatusInfo[SC.SWING] = {
	icon: '\xbd\xba\xc0\xae\xb4\xed\xbd\xba.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Swing Dance', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Movement Speed'], ['Increases ASPD']]
};

StatusInfo[SC.MOON_COMFORT] = {
	icon: 'i_mooncomfort.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Comfort of the Moon', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Improves FLEE']]
};

StatusInfo[SC.MOONLIT_SERENADE] = {
	icon: '\xb4\xde\xba\xfb\xc0\xc7\xbc\xbc\xb7\xb9\xb3\xaa\xb5\xa5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Moonlit Serenade', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases damage of magic skills']]
};

StatusInfo[SC.GENTLETOUCH_CHANGE] = {
	icon: '\xc1\xa1\xc7\xf7\xb9\xdd.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Gentle Touch - Change', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduces DEF and MDEF'],
		['Increases Damage and ASPD']
	]
};

StatusInfo[SC.STRIPACCESSARY] = {
	icon: '\xbd\xba\xc6\xae\xb8\xb3\xbe\xd7\xbc\xbc\xbc\xad\xb8\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Accessory Off Status', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Cannot Equip Accessories']]
};

StatusInfo[SC.PROPERTYUNDEAD] = {
	icon: 'bd_undead.tga', //CUSTOM TGA
	haveTimeLimit: 1,
	descript: [['Enchants Armor with Undead Property']]
};

StatusInfo[SC.INVISIBILITY] = {
	icon: '\xc0\xce\xba\xf1\xc1\xf6\xba\xf4\xb8\xae\xc6\xbc.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Invisibility', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Conceals yourself from view'],
		['All attacks become Ghost Lvl 1 property'],
		['Drains SP'],
		['Skills and items cannot be used']
	]
};

StatusInfo[SC.ABUNDANCE] = {
	icon: 'rk_uruz.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Rune Stone: Abundance', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Recovers SP every 10 seconds']]
};

StatusInfo[SC.FOOD_BASICHIT] = {
	icon: 'gogi.tga', //CUSTOM?
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Improves HIT']]
};

StatusInfo[SC.FOOD_AGI_CASH] = {
	icon: 'agi_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases AGI']]
};

StatusInfo[SC.SHADOWFORM] = {
	icon: '\xbd\xa6\xb5\xb5\xbf\xec\xc6\xfb.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Shadow Form', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['For a certain number of hits'],
		['have a target take the damage instead']
	]
};

StatusInfo[SC.AUTOSHADOWSPELL] = {
	icon: '\xbf\xc0\xc5\xe4\xbd\xa6\xb5\xb5\xbf\xec\xbd\xba\xc6\xe7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Auto Shadow Spell', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Automatically casts'],
		['an available magic spell']
	]
};

StatusInfo[SC.SHAPESHIFT] = {
	icon: '\xbc\xce\xc0\xcc\xc7\xc1\xbd\xac\xc7\xc1\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Shape Shift', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Changes your Mado Gear elemental property']]
};

StatusInfo[SC.MANU_ATK] = {
	icon: 'efst_atk.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Manuk's Golden Chance", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increase physical attack against'],
		['all monsters in Manuk fields']
	]
};

StatusInfo[SC.MARIONETTE_MASTER] = {
	icon: 'icon01.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Marionette Control (caster)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Passes the stats'],
		['to a Player']
	]
};

StatusInfo[SC.MARIONETTE] = {
	icon: 'icon01.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Marionette Control (target)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Target Player'],
		['who receives the stats']
	]
};

StatusInfo[SC.WZ_SIGHTBLASTER] = {
	icon: 'i_sightblaster.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Sight Blaster', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Attacks an enemy with a single attack'],
		['that ventures too close']
	]
};

StatusInfo[SC.LEXAETERNA] = {
	icon: '\xb7\xba\xbd\xba\xbf\xa1\xc5\xd7\xb8\xa3\xb3\xaa.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Lex Aeterna', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Doubles damage of the next attack']]
};

StatusInfo[SC.INFRAREDSCAN] = {
	icon: '\xc0\xce\xc7\xc1\xb6\xf3\xb7\xb9\xb5\xe5\xbd\xba\xc4\xb5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Infrared Scan', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Discovers targets in hiding'],
		['Chance to Reduce FLEE of nearby enemy']
	]
};

StatusInfo[SC.INT_SCROLL] = {
	icon: 'int_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases INT']]
};

StatusInfo[SC.ASPERSIO] = {
	icon: '\xbe\xc6\xbd\xba\xc6\xe4\xb8\xa3\xbd\xc3\xbf\xc0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Aspersio', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Holy Property']]
};

StatusInfo[SC.MOVHASTE_INFINITY] = {
	icon: '\xb9\xce\xc3\xb8\xbc\xba\xc1\xf5\xb0\xa1.tga',
	descript: [['Increases Movement Speed']]
};

StatusInfo[SC.LERADS_DEW] = {
	icon: '\xb7\xb9\xb6\xf3\xb5\xe5\xc0\xc7\xc0\xcc\xbd\xbd.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [["Lerad's Dew", COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Max HP']]
};

StatusInfo[SC.FOOD_INT] = {
	icon: 'int_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases INT']]
};

StatusInfo[SC.VENOMBLEED] = {
	icon: '\xbc\xe8\xbe\xe0\xb5\xb6.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Venom Bleed', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Reduces Max HP']]
};

StatusInfo[SC.GS_GATLINGFEVER] = {
	icon: 'i_fever.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Gatling Fever', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Damage and ASPD'],
		['Reduces Movement Speed']
	]
};

StatusInfo[SC.VITALITYACTIVATION] = {
	icon: 'rk_isha.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rune Stone: Vitality Activation', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['To the caster,'],
		['Increases Healing skills and Item effects'],
		['Stops SP regeneration'],
		['Reduces SP recovery item effects']
	]
};

StatusInfo[SC.STONEHARDSKIN] = {
	icon: 'rk_hagalaz.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rune Stone: Stone Hard Skin', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Resists damage equal to the HP consumed when casting'],
		['Players that attack you with weapons'],
		['will break them by chance.'],
		['On monsters, they will have reduced ATK for 10 seconds']
	]
};

StatusInfo[SC.WEIGHTOVER90] = {
	icon: '\xb9\xab\xb0\xd4\x39\x30\xc0\xcc\xbb\xf3.tga',
	descript: [['Overweight 90%', COLOR_TITLE_DEBUFF], ['HP/SP will not be restored'], ['Attacks/Skills are disabled']]
};

StatusInfo[SC.PROTECTHELM] = {
	icon: '\xc4\xc9\xb9\xcc\xc4\xc3\xc7\xc1\xb7\xce\xc5\xd8\xbc\xc7\x5b\xc7\xef\xb8\xa7\x5d.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Chemical Protection Helm (Biochemical Helm)', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Protect helm from being destroyed']
	]
};

StatusInfo[SC.PLUSAVOIDVALUE] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Cup of Welcome', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increase Perfect Dodge by 20']]
};

StatusInfo[SC.OBLIVIONCURSE] = {
	icon: '\xb8\xc1\xb0\xa2\xc0\xc7\xc0\xe7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Oblivion Curse', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Oblivion status']]
};

StatusInfo[SC.HEALPLUS] = {
	icon: '\xc1\xdf\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Enhanced Healing Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When healing via recovery items'],
		['the healing effect is increased']
	]
};

StatusInfo[SC.PROTECT_DEF] = {
	icon: 'gogi.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Defense Protection', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases physical DEF']]
};

StatusInfo[SC.CRITICALWOUND] = {
	icon: 'criticalwound.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Critical Wounds', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['Reduces effects of healing skills']]
};

StatusInfo[SC.PRESTIGE] = {
	icon: 'lg_prestige.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Prestige', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Chance to evade Magical Attacks'], ['Defense Up']]
};

StatusInfo[SC.FOOD_DEX_CASH] = {
	icon: 'dex_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases DEX']]
};

StatusInfo[SC.CARTBOOST] = {
	icon: 'icon07.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Cart Boost', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Movement Speed']]
};

StatusInfo[SC.L_LIFEPOTION] = {
	icon: '\xc1\xdf\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Medium Life Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recovers HP every 4 seconds'],
		['No effect if Berserk State is active']
	]
};

StatusInfo[SC.WINDWALK] = {
	icon: 'icon06.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Wind Walk', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Movement Speed/Evasion']]
};

StatusInfo[SC.PROPERTYFIRE] = {
	icon: '\xc7\xc1\xb7\xb9\xc0\xd3\xb7\xb1\xc3\xc4.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Flame Launcher', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Fire Property']]
};

StatusInfo[SC.STRIKING] = {
	icon: '\xbd\xba\xc6\xae\xb6\xf3\xc0\xcc\xc5\xb7.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Striking', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Weapon damage and chance of critical']]
};

StatusInfo[SC.DOWNKICK_ON] = {
	icon: 'i_downkick.tga',
	descript: [['Prepare Down Kick', COLOR_TITLE_BUFF], ['Hit an enemy'], ['for a chance to preform a kick']]
};

StatusInfo[SC.PROPERTYDARK] = {
	icon: '\xc4\xb3\xbd\xc3\xb0\xa9\xbf\xca\xbc\xd3\xbc\xba\xba\xce\xbf\xa9.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Enchants Armor with Shadow Property']]
};

StatusInfo[SC.REFLECTSHIELD] = {
	icon: '\xb8\xae\xc7\xc3\xb7\xba\xc6\xae\xbd\xaf\xb5\xe5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Reflect Shield', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When attacked with physical short range attacks'],
		['reflect a portion of the damage']
	]
};

StatusInfo[SC.DEVOTION] = {
	icon: '\xc7\xe5\xbd\xc5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Devotion', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['A Crusader is taking damage for you']]
};

StatusInfo[SC.RIDING] = {
	icon: '\xb6\xf3\xc0\xcc\xb5\xf9.tga',
	descript: [['Riding Vehicle', COLOR_TITLE_TOGGLE]]
};

StatusInfo[SC.LIGHTNINGWALK] = {
	icon: 'icon02.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Lightning Walk', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When targetted by a magic attack,'],
		['after a chance to avoid'],
		['move straight to the caster']
	]
};

StatusInfo[SC.FROSTMISTY] = {
	icon: 'frostmisty.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Freezing Status', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Decreases Defense, ASPD and Movement speed'],
		['Increases Fixed Cast time.']
	]
};

StatusInfo[SC.COLD] = {
	icon: '\xb4\xd9\xc0\xcc\xbe\xc6\xb8\xf3\xb5\xe5\xb4\xf5\xbd\xba\xc6\xae.tga', //CUSTOM?
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Frozen', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Movement, Attack, Skill and Items are not available'],
		['Drains SP and HP continuously'],
		['Increases the damage taken caused by Maces, Axes and 2H Axes'],
		['Increases the damage taken caused by Wind Property spells'],
		['Reduces the damage taken caused by Daggers, Swords, 2H Swords and Arrows']
	]
};

StatusInfo[SC.GROUNDMAGIC] = {
	icon: 'i_\xc0\xe5\xc6\xc7.tga', //CUSTOM?
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Ground skill effect']]
};

StatusInfo[SC.HELLPOWER] = {
	icon: 'npc_hellpower.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Hell Power', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Can not be revived'],
		['Sacrifice is Disabled'],
		['Token of Siegfried disabled']
	]
};

StatusInfo[SC.SAVAGE_STEAK] = {
	icon: 'str_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Savage Roast', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases STR']]
};

StatusInfo[SC.COCKTAIL_WARG_BLOOD] = {
	icon: 'int_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Wolf Blood Cocktail', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases INT']]
};

StatusInfo[SC.MINOR_BBQ] = {
	icon: 'vit_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Minorous Beef Stew', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases VIT']]
};

StatusInfo[SC.SIROMA_ICE_TEA] = {
	icon: 'dex_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Siroma Iced Tea', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases DEX']]
};

StatusInfo[SC.DROCERA_HERB_STEAMED] = {
	icon: 'agi_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Drosera Herb Salad', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases AGI']]
};

StatusInfo[SC.PUTTI_TAILS_NOODLES] = {
	icon: 'luk_gogi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Petite Tail Noodle', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases LUK']]
};

StatusInfo[SC.STOMACHACHE] = {
	icon: '\xbe\xc8\xc6\xbc\xb8\xc5\xc5\xcd\xb8\xae\xbe\xf3.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Stomachache', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduces All Stats'],
		['Reduces Movement Speed'],
		['Forced to sit down once in every 10 sec'],
		['Consume certain amount of SP for 10 sec']
	]
};

StatusInfo[SC.PROTECTEXP] = {
	icon: '\xbe\xf6\xb8\xb6\xbe\xc6\xba\xfc\xbb\xe7\xb6\xfb\xc7\xd8\xbf\xe4.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Mom and Dad I Love You', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['No EXP loss upon death']]
};

StatusInfo[SC.ANGEL_PROTECT] = {
	icon: 'death.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Guardian Angel', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['No EXP loss upon death']]
};

StatusInfo[SC.MORA_BUFF] = {
	icon: 'gogi.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Mora Berry', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases Resistance to every monsters'],
		['in the fields near the town of Mora.']
	]
};

StatusInfo[SC.POPECOOKIE] = {
	icon: 'gogi.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Pope Cookie', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK and MATK'],
		['Increases Resistance to all property.']
	]
};

StatusInfo[SC.VITALIZE_POTION] = {
	icon: '\xbf\xa1\xc0\xcc\xb6\xf3\x31.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Vitalize Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK and MATK'],
		['Increase effects of heal and healing items']
	]
};

StatusInfo[SC.G_LIFEPOTION] = {
	icon: '\xc1\xdf\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Rapid Life-giving Water', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recovers HP every 3 seconds'],
		['No effect if Berserk State is active']
	]
};

StatusInfo[SC.ODINS_POWER] = {
	icon: 'all_odins_power.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Odin's Power", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK and MATK'],
		['Decreases DEF and MDEF']
	]
};

StatusInfo[SC.MAGIC_CANDY] = {
	icon: 'plusmagicpower.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Magic Candy', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases MATK'],
		['Reduced fixed casting time.'],
		['Casting cannot be interrupted.'],
		['Drains SP every 10 seconds']
	]
};

StatusInfo[SC.ENERGYCOAT] = {
	icon: '\xbf\xa1\xb3\xca\xc1\xf6\xc4\xda\xc6\xae.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Energy Coat', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduces damage in proportion'],
		['to the amount of SP remaining']
	]
};

StatusInfo[SC.PAIN_KILLER] = {
	icon: '\xbc\xbc\xb6\xf34.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Pain Killer', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['No movement delay in receiving damage'],
		['Reduced damage taken']
	]
};

StatusInfo[SC.LIGHT_OF_REGENE] = {
	icon: '\xbf\xa1\xc0\xcc\xb6\xf31.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Light Of Regeneration', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When the summoner died'],
		['Homunculus will sacrifice to revive the summoner']
	]
};

StatusInfo[SC.OVERED_BOOST] = {
	icon: '\xbf\xa1\xc0\xcc\xb6\xf32.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Overed Boost', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ASPD and Evasion'],
		['to a fixed amount']
	]
};

StatusInfo[SC.STYLE_CHANGE] = {
	icon: 'efst_atk.tga', //CUSTOM
	haveTimeLimit: 0,
	descript: [['Style Change', COLOR_TITLE_TOGGLE], ['Homunculus in Fighter Style']]
};

StatusInfo[SC.MAGMA_FLOW] = {
	icon: '\xb5\xf0\xc0\xcc\xc5\xcd1.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Magma Flow', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When receiving damage'],
		['there is a chance to eject magma around it']
	]
};

StatusInfo[SC.GRANITIC_ARMOR] = {
	icon: '\xb5\xf0\xc0\xcc\xc5\xcd2.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Granitic Armor', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Reduced damage taken'],
		['Lose some HP when the status ends.']
	]
};

StatusInfo[SC.PYROCLASTIC] = {
	icon: '\xb5\xf0\xc0\xcc\xc5\xcd4.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Pyroclastic', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Weapon of Deeter and Owner'],
		['Change to Fire Property'],
		['Increased weapon damage']
	]
};

StatusInfo[SC.VOLCANIC_ASH] = {
	icon: '\xb5\xf0\xc0\xcc\xc5\xcd5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Volcanic Ash', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduced hit rate'],
		['Skill has a chance of failing'],
		['Increases fire damage taken']
	]
};

StatusInfo[SC.ATKER_ASPD] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Great Phantom', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Max HP'], ['Increases HP recovery']]
};

StatusInfo[SC.ATKER_MOVESPEED] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['ÅÂÃ»´Ü', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Max SP'], ['Increases SP recovery']]
};

StatusInfo[SC.OVERLAPEXPUP] = {
	icon: 'item.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Maldango Canned Cat', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['When killing monsters in Maldango'],
		['Increases Base and Job EXP'],
		['Increases Item drop rate']
	]
};

StatusInfo[SC.PLUSATTACKPOWER] = {
	icon: 'plusattackpower.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases ATK']]
};

StatusInfo[SC.PLUSMAGICPOWER] = {
	icon: 'plusmagicpower.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases MATK']]
};

StatusInfo[SC.MACRO_PERMIT] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Using Macros', COLOR_SYSTEM], ['%s', COLOR_TIME], ['Macro is activated']]
};

StatusInfo[SC.MACRO_POSTDELAY] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Macros Disabled', COLOR_SYSTEM], ['%s', COLOR_TIME], ['Macro is deactivated.']]
};

StatusInfo[SC.MONSTER_TRANSFORM] = {
	icon: '\xba\xaf\xbd\xc5.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Monster Transformation', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Transformed into monster.']]
};

StatusInfo[SC.SIT] = {
	icon: '\xbe\xc9\xb1\xe2.tga',
	descript: [['Sit', COLOR_TITLE_TOGGLE]]
};

StatusInfo[SC.ALL_RIDING] = {
	icon: '\xb6\xf3\xc0\xcc\xb5\xf9.tga',
	descript: [['Riding', COLOR_TITLE_TOGGLE]]
};

StatusInfo[SC.SKF_MATK] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases MATK']]
};

StatusInfo[SC.SKF_ATK] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases ATK']]
};

StatusInfo[SC.SKF_ASPD] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases ASPD']]
};

StatusInfo[SC.SKF_CAST] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Reduces casting time']]
};

StatusInfo[SC.REWARD_PLUSONLYJOBEXP] = {
	icon: 'job.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['Increases gained Job experience']]
};

StatusInfo[SC.ENERVATION] = {
	icon: 'masquerade_enervation.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Enervation', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces ATK'],
		['Removes Spirit Spheres']
	]
};

StatusInfo[SC.GROOMY] = {
	icon: 'masquerade_groomy.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Gloomy', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Decreases ASPD and HIT'],
		['Forced to release mounts and any related animals.'],
		['Mounts and any related animals are disabled.']
	]
};

StatusInfo[SC.IGNORANCE] = {
	icon: 'masquerade_ignorance.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Ignorance', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Lost a certain amount of SP'],
		['Skills and Magics are disabled']
	]
};

StatusInfo[SC.LAZINESS] = {
	icon: 'masquerade_laziness.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Laziness', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces Movement Speed and FLEE'],
		['Increases casting time'],
		['Adds a certain amount of SP when using a skill']
	]
};

StatusInfo[SC.UNLUCKY] = {
	icon: 'masquerade_unlucky.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Unlucky', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces critical rate'],
		['Reduces perfect dodge'],
		['Using skills costs zeny'],
		['Damage over time causes a certain status ailments.']
	]
};

StatusInfo[SC.WEAKNESS] = {
	icon: 'masquerade_weakness.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Masquerade: Weakness', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reduces Max HP'],
		['When evaded, weapon, shield is taken off.'],
		['Cannot equip weapons and shield']
	]
};

StatusInfo[SC.STEELBODY] = {
	icon: '\x73\x74\x65\x65\x6c\x62\x6f\x64\x79.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Steel Body', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Sets DEF and MDEF to a fixed amount.'],
		['Reduces Movement Speed and ASPD'],
		['Skills are disabled']
	]
};

StatusInfo[SC.LG_REFLECTDAMAGE] = {
	icon: 'lg_reflectdamage.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Reflect Damage', COLOR_TITLE_TOGGLE],
		['%s', COLOR_TIME],
		['Reflects some of the damages received to all enemies in an area'],
		['Consumes SP every second']
	]
};

StatusInfo[SC.MVPCARD_TAOGUNKA] = {
	icon: 'mvpcard_taogunka.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Tao Gunka Scroll', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases Max HP'], ['Reduces DEF/MDEF']]
};

StatusInfo[SC.MVPCARD_MISTRESS] = {
	icon: 'mvpcard_mistress.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Mistress Scroll', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Nullify the Gemstone requirement of certain spells'],
		['Increases SP consumption']
	]
};

StatusInfo[SC.MVPCARD_ORCHERO] = {
	icon: 'mvpcard_orchero.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Orc Hero Scroll', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Immunity to stun status']]
};

StatusInfo[SC.MVPCARD_ORCLORD] = {
	icon: 'mvpcard_orclord.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Orc Lord Scroll', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Reflects a part of physical damage taken']]
};

StatusInfo[SC.HANDICAPSTATE_NORECOVER] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Recovery disabled status', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['HP and SP recovery are disabled']
	]
};

StatusInfo[SC.SET_NUM_DEF] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['DEF amount is set.']]
};

StatusInfo[SC.SET_NUM_MDEF] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 1,
	descript: [['%s', COLOR_TIME], ['MDEF amount is set.']]
};

StatusInfo[SC.SET_PER_DEF] = {
	descript: [['DEF amount is fixed at certain percentage.']]
};

StatusInfo[SC.SET_PER_MDEF] = {
	descript: [['MDEF amount is fixed at certain percentage.']]
};

StatusInfo[SC.EXTREMITYFIST] = {
	icon: 'extremityfist.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Asura Strike (Guillotine Fist)', COLOR_TITLE_DEBUFF], ['%s', COLOR_TIME], ['SP recovery is disabled']]
};

StatusInfo[SC.ATTHASTE_CASH] = {
	icon: 'aspdcash.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['ASPD Reinforce Potion', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increase Attack Speed']]
};

StatusInfo[SC.RWC2011] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Firecracker', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases all stats'], ['Increases ATK, MATK']]
};

StatusInfo[SC.PHI_DEMON] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		["Godfather's Ancient Spirit", COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Gives Demon type monster'],
		['Increases Physical and Magic damage']
	]
};

StatusInfo[SC.GM_BATTLE] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['ÀüÅõ¾à', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases ATK and MATK'], ['Reduced MHP and MSP']]
};

StatusInfo[SC.GM_BATTLE2] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['°í±ÞÀüÅõ¾à', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK and MATK'],
		['Reduces MHP and MSP']
	]
};

StatusInfo[SC.RWC_SCROLL2011] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Red Booster', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases ATK and MATK'],
		['Increases ASPD'],
		['Reduced variable casting time'],
		['When receiving Physical and Magical damage'],
		['there is a chance to cast Improve Concentration skill']
	]
};

StatusInfo[SC.MEIKYOUSISUI] = {
	icon: 'meikyousisui.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Pure Soul', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recovers a certain amount of HP'],
		['Recovers a certain amount of SP'],
		['Unable to move'],
		['Chance to ignore damage when attacked'],
		['Effect wears off by chance when hit.']
	]
};

StatusInfo[SC.IZAYOI] = {
	icon: 'izayoi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['16th Night', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Removes Fixed casting time'],
		['Reduced variable casting time'],
		['Increases MATK'],
		['Drains SP per second']
	]
};

StatusInfo[SC.KG_KAGEHUMI] = {
	icon: 'kg_kagehumi.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Shadow Step', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Unable to move'],
		['Cannot use certain skills or item'],
		['Stealth or teleport skills and items are disabled.'],
		['Unable to use Emergency Call skill']
	]
};

StatusInfo[SC.KYOMU] = {
	icon: 'kyomu.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Kyomu', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['Reflects wont take effect when hit by Physical or Magical Attacks'],
		['Chance the skill will fail when casting']
	]
};

StatusInfo[SC.KAGEMUSYA] = {
	icon: 'kagemusya.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Shadow Warrior', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Enchanted with Double Attack effect'],
		['Drains SP per second'],
		['Status ends when received a certain number of hits.']
	]
};

StatusInfo[SC.ZANGETSU] = {
	icon: 'zangetsu.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Distorted Crescent Moon', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['ATK and MATK changed, depending on HP and SP']
	]
};

StatusInfo[SC.GENSOU] = {
	icon: 'gensou.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Oboro Gensou', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Randomly increase/decrease HP and SP'],
		['When hit by Magical Attacks half of the damage'],
		['will be distributed around the area']
	]
};

StatusInfo[SC.AKAITSUKI] = {
	icon: 'akaitsuki.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Ominous Crimson Moonlight', COLOR_TITLE_DEBUFF],
		['%s', COLOR_TIME],
		['When receiving recovery skills'],
		['healed amount will be converted to damage.']
	]
};

StatusInfo[SC.MYSTICPOWDER] = {
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Mystic Powder', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases FLEE and LUK']]
};

StatusInfo[SC.ACARAJE] = {
	icon: '\xb0\xf8\xbc\xd3\xb9\xb0\xbe\xe0.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Acaraje', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases HIT and ASPD']]
};

StatusInfo[SC.M_LIFEPOTION] = {
	icon: '\xc1\xdf\xc7\xfc\xbb\xfd\xb8\xed\xbc\xf6.tga', //CUSTOM
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Mysterious Life Potion', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Recovers a certain amount of HP every 3 seconds'],
		['No effect on Berserk status.']
	]
};

StatusInfo[SC.FLOWER_LEAF] = {
	icon: 'flower_leaf.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Ç³¼ºÇÑ ²É°¡Áö', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Increases FLEE'], ['Increases perfect dodge']]
};

StatusInfo[SC.BDPLAYING] = {
	icon: '\xb9\xd9\xb5\xe5\xb3\xeb\xb7\xa1.tga'
};

StatusInfo[SC.RUN] = {
	icon: 'i_run.tga'
};

StatusInfo[SC.CLIENT_ONLY_EQUIP_ARROW] = {
	icon: 'ArrowN.tga'
};

StatusInfo[SC.RAY_OF_PROTECTION] = {
	icon: 'all_ray_of_protection.tga'
};

StatusInfo[SC.DARKCROW] = {
	icon: 'darkcrow.tga'
};

StatusInfo[SC.FRIGG_SONG] = {
	icon: 'frigg_song.tga'
};

StatusInfo[SC.FULL_THROTTLE] = {
	icon: 'full_throttle.tga'
};

StatusInfo[SC.GLASTHEIM_ATK] = {
	icon: 'glastheim_atk.tga'
};

StatusInfo[SC.GLASTHEIM_DEF] = {
	icon: 'glastheim_def.tga'
};

StatusInfo[SC.GLASTHEIM_HEAL] = {
	icon: 'glastheim_heal.tga'
};

StatusInfo[SC.ATTACK_PROPERTY_NOTHING] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Neutral Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_WATER] = {
	icon: 'weapon_property_water.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Water Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_GROUND] = {
	icon: 'weapon_property_ground.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Earth Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_FIRE] = {
	icon: 'weapon_property_fire.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Fire Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_WIND] = {
	icon: 'weapon_property_wind.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Wind Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_POISON] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Poison Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_SAINT] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Holy Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_DARKNESS] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Dark Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_TELEKINESIS] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Ghost Property']]
};

StatusInfo[SC.ATTACK_PROPERTY_UNDEAD] = {
	icon: 'weapon_property.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [['Weapon Property', COLOR_TITLE_BUFF], ['%s', COLOR_TIME], ['Enchants Weapon with Undead Property']]
};

StatusInfo[SC.RESIST_PROPERTY_NOTHING] = {
	icon: 'resist_elemental_nothing.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Neutral Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_WATER] = {
	icon: 'resist_elemental_water.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Water Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_GROUND] = {
	icon: 'resist_elemental_ground.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Earth Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_FIRE] = {
	icon: 'resist_elemental_fire.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Fire Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_WIND] = {
	icon: 'resist_elemental_wind.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Wind Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_POISON] = {
	icon: 'resist_elemental_poison.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Poison Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_SAINT] = {
	icon: 'resist_elemental_saint.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Holy Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_DARKNESS] = {
	icon: 'resist_elemental_darkness.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Dark Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_TELEKINESIS] = {
	icon: 'resist_elemental_telekinesis.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Ghost Property']
	]
};

StatusInfo[SC.RESIST_PROPERTY_UNDEAD] = {
	icon: 'resist_elemental_undead.tga',
	haveTimeLimit: 1,
	posTimeLimitStr: 2,
	descript: [
		['Resist Property', COLOR_TITLE_BUFF],
		['%s', COLOR_TIME],
		['Increases your resistance to the Undead Property']
	]
};

export default StatusInfo;
