/**
 * DB/Skills/SkillUnit.js
 *
 * Zone effects
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import SU from './SkillUnitConst.js';
import EC from 'DB/Effects/EffectConst.js';

const SkillUnit = {};

SkillUnit[SU.UNT_SAFETYWALL] = EC.EF_GLASSWALL2;
SkillUnit[SU.UNT_FIREWALL] = EC.EF_FIREWALL;
SkillUnit[SU.UNT_WARPPORTAL] = EC.EF_PORTAL2;
SkillUnit[SU.UNT_PRE_WARPPORTAL] = EC.EF_READYPORTAL2;
SkillUnit[SU.UNT_SANCTUARY] = EC.EF_BOTTOM_SANC;
SkillUnit[SU.UNT_MAGNUS] = EC.EF_BOTTOM_MAG;
SkillUnit[SU.UNT_PNEUMA] = EC.EF_PNEUMA;
SkillUnit[SU.UNT_FIREPILLAR_WAITING] = EC.EF_FIREPILLARON;
SkillUnit[SU.UNT_ICEWALL] = EC.EF_ICEWALL;
SkillUnit[SU.UNT_QUAGMIRE] = EC.EF_QUAGMIRE;
SkillUnit[SU.UNT_BLASTMINE] = 'ef_trap_03_3';
SkillUnit[SU.UNT_SKIDTRAP] = 'ef_trap_02';
SkillUnit[SU.UNT_ANKLESNARE] = 'ef_trap_01';
SkillUnit[SU.UNT_VENOMDUST] = EC.EF_VENOMDUST2;
SkillUnit[SU.UNT_LANDMINE] = 'ef_trap_03';
SkillUnit[SU.UNT_SHOCKWAVE] = 'ef_trap_03_6';
SkillUnit[SU.UNT_SANDMAN] = 'ef_trap_03_4';
SkillUnit[SU.UNT_FLASHER] = 'ef_trap_03_5';
SkillUnit[SU.UNT_FREEZINGTRAP] = 'ef_trap_03_2';
SkillUnit[SU.UNT_CLAYMORETRAP] = 'ef_trap_04';
SkillUnit[SU.UNT_TALKIEBOX] = 'ef_trap_05';
SkillUnit[SU.UNT_VOLCANO] = EC.EF_BOTTOM_VO;
SkillUnit[SU.UNT_DELUGE] = EC.EF_BOTTOM_DE;
SkillUnit[SU.UNT_VIOLENTGALE] = EC.EF_BOTTOM_VI;
SkillUnit[SU.UNT_LANDPROTECTOR] = EC.EF_BOTTOM_LA;
SkillUnit[SU.UNT_LULLABY] = '278_ground'; // Tofix
SkillUnit[SU.UNT_RICHMANKIM] = '279_ground'; // Tofix
SkillUnit[SU.UNT_ETERNALCHAOS] = '280_ground'; // Tofix
SkillUnit[SU.UNT_DRUMBATTLEFIELD] = '281_ground'; // Tofix
SkillUnit[SU.UNT_RINGNIBELUNGEN] = '282_ground'; // Tofix
SkillUnit[SU.UNT_ROKISWEIL] = '283_ground'; // Tofix
SkillUnit[SU.UNT_INTOABYSS] = '284_ground'; // Tofix
SkillUnit[SU.UNT_SIEGFRIED] = '285_ground'; // Tofix
SkillUnit[SU.UNT_DISSONANCE] = '277_ground'; // Tofix
SkillUnit[SU.UNT_WHISTLE] = '286_ground'; // Tofix
SkillUnit[SU.UNT_ASSASSINCROSS] = '287_ground'; // Tofix
SkillUnit[SU.UNT_POEMBRAGI] = '288_ground'; // Tofix
SkillUnit[SU.UNT_APPLEIDUN] = '289_ground'; // Tofix
SkillUnit[SU.UNT_UGLYDANCE] = '290_ground'; // Tofix
SkillUnit[SU.UNT_HUMMING] = '291_ground'; // Tofix
SkillUnit[SU.UNT_DONTFORGETME] = '292_ground'; // Tofix
SkillUnit[SU.UNT_FORTUNEKISS] = '293_ground'; // Tofix
SkillUnit[SU.UNT_SERVICEFORYOU] = '294_ground'; // Tofix
SkillUnit[SU.UNT_GRAFFITI] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_DEMONSTRATION] = EC.EF_DEMONSTRATION;
SkillUnit[SU.UNT_GOSPEL] = '370_ground'; // Tofix
SkillUnit[SU.UNT_BASILICA] = EC.EF_BOTTOM_BASILICA;
SkillUnit[SU.UNT_MOONLIT] = '394_ground'; // Tofix
SkillUnit[SU.UNT_FOGWALL] = '405_ground'; // Tofix
SkillUnit[SU.UNT_SPIDERWEB] = EC.EF_BOTTOM_SPIDER;
SkillUnit[SU.UNT_GRAVITATION] = '522_ground'; // Tofix
SkillUnit[SU.UNT_HERMODE] = EC.EF_BOTTOM_HERMODE;
SkillUnit[SU.UNT_SUITON] = EC.EF_BOTTOM_SUITON;
SkillUnit[SU.UNT_TATAMIGAESHI] = EC.EF_TATAMI;
SkillUnit[SU.UNT_KAEN] = EC.EF_KAEN;
SkillUnit[SU.UNT_GROUNDDRIFT_WIND] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_GROUNDDRIFT_WATER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_GROUNDDRIFT_FIRE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_DEATHWAVE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_WATERATTACK] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_EVILLAND] = EC.EF_BOTTOM_EVILLAND;
SkillUnit[SU.UNT_EPICLESIS] = EC.EF_GLASSWALL3;
SkillUnit[SU.UNT_EARTHSTRAIN] = EC.EF_EARTHWALL;
SkillUnit[SU.UNT_MANHOLE] = EC.EF_BOTTOM_MANHOLE;
SkillUnit[SU.UNT_DIMENSIONDOOR] = EC.EF_FORESTLIGHT6;
SkillUnit[SU.UNT_CHAOSPANIC] = EC.EF_BOTTOM_ANI;
SkillUnit[SU.UNT_MAELSTROM] = EC.EF_BOTTOM_MAELSTROM;
SkillUnit[SU.UNT_BLOODYLUST] = EC.EF_BOTTOM_BLOODYLUST;
SkillUnit[SU.UNT_FEINTBOMB] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_MAGENTATRAP] = 'ef_trap_3_magenta';
SkillUnit[SU.UNT_COBALTTRAP] = 'ef_trap_3_cobalt';
SkillUnit[SU.UNT_MAIZETRAP] = 'ef_trap_3_maze';
SkillUnit[SU.UNT_VERDURETRAP] = 'ef_trap_3_verdure';
SkillUnit[SU.UNT_FIRINGTRAP] = 'ef_trap_3_fire';
SkillUnit[SU.UNT_ICEBOUNDTRAP] = 'ef_trap_3_ice';
SkillUnit[SU.UNT_ELECTRICSHOCKER] = 'ef_trap_3_shock';
SkillUnit[SU.UNT_CLUSTERBOMB] = 'ef_trap_3_cluster';
SkillUnit[SU.UNT_REVERBERATION] = EC.EF_BOT_REVERB;
SkillUnit[SU.UNT_SEVERE_RAINSTORM] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_FIREWALK] = EC.EF_FIREWALL2;
SkillUnit[SU.UNT_ELECTRICWALK] = EC.EF_SHOCKWAVE2;
SkillUnit[SU.UNT_NETHERWORLD] = EC.EF_BOT_REVERB2;
SkillUnit[SU.UNT_PSYCHIC_WAVE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_CLOUD_KILL] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_POISONSMOKE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_NEUTRALBARRIER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_STEALTHFIELD] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_WARMER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_THORNS_TRAP] = 'ef_trap_3_thorn';
SkillUnit[SU.UNT_WALLOFTHORN] = EC.EF_WALLOFTHORN;
SkillUnit[SU.UNT_DEMONIC_FIRE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_FIRE_EXPANSION_SMOKE_POWDER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_FIRE_EXPANSION_TEAR_GAS] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_HELLS_PLANT] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_VACUUM_EXTREME] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_BANDING] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_FIRE_MANTLE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_WATER_BARRIER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_ZEPHYR] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_POWER_OF_GAIA] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_FIRE_INSIGNIA] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_WATER_INSIGNIA] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_WIND_INSIGNIA] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_EARTH_INSIGNIA] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_POISON_MIST] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_LAVA_SLIDE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_VOLCANIC_ASH] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_ZENKAI_WATER] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_MAKIBISHI] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_VENOMFOG] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_ICEMINE] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_MAGMA_ERUPTION] = EC.EF_NONE; // Todo
SkillUnit[SU.UNT_B_TRAP] = EC.EF_NONE; // Todo

export default SkillUnit;
