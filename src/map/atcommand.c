// $Id: atcommand.c 148 2004-09-30 14:05:37Z MouseJstr $
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/core.h"

#include "log.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "pet.h"
#include "battle.h"
#include "party.h"
#include "guild.h"
#include "atcommand.h"
#include "script.h"
#include "npc.h"
#include "trade.h"

#ifndef TXT_ONLY
#include "mail.h"
#endif

#define STATE_BLIND 0x10

static char command_symbol = '@'; // first char of the commands (by [Yor])

#define MAX_MSG 1000
char *msg_table[MAX_MSG]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

#define ACMD_FUNC(x) int atcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)
ACMD_FUNC(broadcast);
ACMD_FUNC(localbroadcast);
ACMD_FUNC(rura);
ACMD_FUNC(where);
ACMD_FUNC(jumpto);
ACMD_FUNC(jump);
ACMD_FUNC(who);
ACMD_FUNC(who2);
ACMD_FUNC(who3);
ACMD_FUNC(whomap);
ACMD_FUNC(whomap2);
ACMD_FUNC(whomap3);
ACMD_FUNC(whogm); // by Yor
ACMD_FUNC(whozeny); // [Valaris]
ACMD_FUNC(happyhappyjoyjoy); // [Valaris]
ACMD_FUNC(save);
ACMD_FUNC(load);
ACMD_FUNC(speed);
ACMD_FUNC(storage);
ACMD_FUNC(guildstorage);
ACMD_FUNC(option);
ACMD_FUNC(hide);
ACMD_FUNC(jobchange);
ACMD_FUNC(die);
ACMD_FUNC(kill);
ACMD_FUNC(alive);
ACMD_FUNC(kami);
ACMD_FUNC(heal);
ACMD_FUNC(item);
ACMD_FUNC(item2);
ACMD_FUNC(itemreset);
ACMD_FUNC(itemcheck);
ACMD_FUNC(baselevelup);
ACMD_FUNC(joblevelup);
ACMD_FUNC(help);
ACMD_FUNC(gm);
ACMD_FUNC(pvpoff);
ACMD_FUNC(pvpon);
ACMD_FUNC(gvgoff);
ACMD_FUNC(gvgon);
ACMD_FUNC(model);
ACMD_FUNC(go);
ACMD_FUNC(monster);
ACMD_FUNC(monstersmall);
ACMD_FUNC(monsterbig);
ACMD_FUNC(spawn);
ACMD_FUNC(killmonster);
ACMD_FUNC(killmonster2);
ACMD_FUNC(refine);
ACMD_FUNC(produce);
ACMD_FUNC(memo);
ACMD_FUNC(gat);
ACMD_FUNC(packet);
ACMD_FUNC(statuspoint);
ACMD_FUNC(skillpoint);
ACMD_FUNC(zeny);
ACMD_FUNC(param);
ACMD_FUNC(guildlevelup);
ACMD_FUNC(makeegg);
ACMD_FUNC(hatch);
ACMD_FUNC(petfriendly);
ACMD_FUNC(pethungry);
ACMD_FUNC(petrename);
ACMD_FUNC(recall);
ACMD_FUNC(recallall);
ACMD_FUNC(revive);
ACMD_FUNC(character_stats_all);
ACMD_FUNC(character_save);
ACMD_FUNC(night);
ACMD_FUNC(day);
ACMD_FUNC(doom);
ACMD_FUNC(doommap);
ACMD_FUNC(raise);
ACMD_FUNC(raisemap);
ACMD_FUNC(character_baselevel);
ACMD_FUNC(character_joblevel);
ACMD_FUNC(kick);
ACMD_FUNC(kickall);
ACMD_FUNC(allskill);
ACMD_FUNC(questskill);
ACMD_FUNC(charquestskill);
ACMD_FUNC(lostskill);
ACMD_FUNC(charlostskill);
ACMD_FUNC(spiritball);
ACMD_FUNC(party);
ACMD_FUNC(guild);
ACMD_FUNC(charskreset);
ACMD_FUNC(charstreset);
ACMD_FUNC(charreset);
ACMD_FUNC(charstpoint);
ACMD_FUNC(charmodel);
ACMD_FUNC(charskpoint);
ACMD_FUNC(agitstart);
ACMD_FUNC(agitend);
ACMD_FUNC(reloaditemdb);
ACMD_FUNC(reloadmobdb);
ACMD_FUNC(reloadskilldb);
ACMD_FUNC(reloadscript);
ACMD_FUNC(reloadgmdb); // by Yor
ACMD_FUNC(reloadatcommand);
ACMD_FUNC(reloadbattleconf);
ACMD_FUNC(reloadstatusdb);
ACMD_FUNC(reloadpcdb);
ACMD_FUNC(mapexit);
ACMD_FUNC(idsearch);
ACMD_FUNC(mapinfo);
ACMD_FUNC(dye); //** by fritz
ACMD_FUNC(hair_style); //** by fritz
ACMD_FUNC(hair_color); //** by fritz
ACMD_FUNC(stat_all); //** by fritz
ACMD_FUNC(char_change_sex); // by Yor
ACMD_FUNC(char_block); // by Yor
ACMD_FUNC(char_ban); // by Yor
ACMD_FUNC(char_unblock); // by Yor
ACMD_FUNC(char_unban); // by Yor
ACMD_FUNC(mount_peco); // by Valaris
ACMD_FUNC(char_mount_peco); // by Yor
ACMD_FUNC(guildspy); // [Syrus22]
ACMD_FUNC(partyspy); // [Syrus22]
ACMD_FUNC(repairall); // [Valaris]
ACMD_FUNC(guildrecall); // by Yor
ACMD_FUNC(partyrecall); // by Yor
ACMD_FUNC(nuke); // [Valaris]
ACMD_FUNC(enablenpc);
ACMD_FUNC(hidenpc);
ACMD_FUNC(disablenpc);
ACMD_FUNC(servertime); // by Yor
ACMD_FUNC(chardelitem); // by Yor
ACMD_FUNC(jail); // by Yor
ACMD_FUNC(unjail); // by Yor
ACMD_FUNC(disguise); // [Valaris]
ACMD_FUNC(undisguise); // by Yor
ACMD_FUNC(chardisguise); // Kalaspuff
ACMD_FUNC(charundisguise); // Kalaspuff
ACMD_FUNC(email); // by Yor
ACMD_FUNC(effect);//by Apple
ACMD_FUNC(character_cart_list); // by Yor
ACMD_FUNC(addwarp); // by MouseJstr
ACMD_FUNC(follow); // by MouseJstr
ACMD_FUNC(skillon); // by MouseJstr
ACMD_FUNC(skilloff); // by MouseJstr
ACMD_FUNC(killer); // by MouseJstr
ACMD_FUNC(npcmove); // by MouseJstr
ACMD_FUNC(killable); // by MouseJstr
ACMD_FUNC(charkillable); // by MouseJstr
ACMD_FUNC(dropall); // by MouseJstr
ACMD_FUNC(chardropall); // by MouseJstr
ACMD_FUNC(storeall); // by MouseJstr
ACMD_FUNC(charstoreall); // by MouseJstr
ACMD_FUNC(skillid); // by MouseJstr
ACMD_FUNC(useskill); // by MouseJstr
ACMD_FUNC(summon);
ACMD_FUNC(rain);
ACMD_FUNC(snow);
ACMD_FUNC(sakura);
ACMD_FUNC(fog);
ACMD_FUNC(leaves);
ACMD_FUNC(adjgmlvl); // by MouseJstr
ACMD_FUNC(adjcmdlvl); // by MouseJstr
ACMD_FUNC(trade); // by MouseJstr
ACMD_FUNC(send); // by davidsiaw
ACMD_FUNC(setbattleflag); // by MouseJstr
ACMD_FUNC(unmute); // [Valaris]
ACMD_FUNC(clearweather); // Dexity
ACMD_FUNC(uptime); // by MC Cameri
ACMD_FUNC(changesex); // by MC Cameri
ACMD_FUNC(mute); // celest
ACMD_FUNC(refresh); // by MC Cameri
ACMD_FUNC(petid); // by MC Cameri
ACMD_FUNC(identify); // by MC Cameri
ACMD_FUNC(gmotd); // Added by MC Cameri, created by davidsiaw
ACMD_FUNC(misceffect); // by MC Cameri
ACMD_FUNC(mobsearch);
ACMD_FUNC(cleanmap);
ACMD_FUNC(npctalk);
ACMD_FUNC(pettalk);
ACMD_FUNC(users);
ACMD_FUNC(autoloot);  // by Upa-Kun

#ifndef TXT_ONLY
ACMD_FUNC(checkmail); // [Valaris]
ACMD_FUNC(listmail); // [Valaris]
ACMD_FUNC(listnewmail); // [Valaris]
ACMD_FUNC(readmail); // [Valaris]
ACMD_FUNC(sendmail); // [Valaris]
ACMD_FUNC(sendprioritymail); // [Valaris]
ACMD_FUNC(deletemail); // [Valaris]
//ACMD_FUNC(sound); // [Valaris]
ACMD_FUNC(refreshonline); // [Valaris]
#endif /* TXT_ONLY */

ACMD_FUNC(skilltree); // by MouseJstr

ACMD_FUNC(marry); // by MouseJstr
ACMD_FUNC(divorce); // by MouseJstr
ACMD_FUNC(rings); // by MouseJstr

ACMD_FUNC(grind); // by MouseJstr
ACMD_FUNC(grind2); // by MouseJstr

#ifdef DMALLOC
ACMD_FUNC(dmstart); // by MouseJstr
ACMD_FUNC(dmtick); // by MouseJstr
#endif

ACMD_FUNC(jumptoid); // by Dino9021
ACMD_FUNC(jumptoid2); // by Dino9021
ACMD_FUNC(recallid); // by Dino9021
ACMD_FUNC(recallid2); // by Dino9021
ACMD_FUNC(kickid); // by Dino9021
ACMD_FUNC(kickid2); // by Dino9021
ACMD_FUNC(reviveid); // by Dino9021
ACMD_FUNC(reviveid2); // by Dino9021
ACMD_FUNC(killid); // by Dino9021
ACMD_FUNC(killid2); // by Dino9021
ACMD_FUNC(charkillableid); // by Dino9021
ACMD_FUNC(charkillableid2);  // by Dino9021
ACMD_FUNC(sound);
ACMD_FUNC(undisguiseall);
ACMD_FUNC(disguiseall);
ACMD_FUNC(changelook);
ACMD_FUNC(mobinfo);	//by Lupus
ACMD_FUNC(adopt); // by Veider

/*==========================================
 *AtCommandInfo atcommand_info[]ç\ë¢ëÃÇÃíËã`
 *------------------------------------------
 */

// First char of commands is configured in atcommand_athena.conf. Leave @ in this list for default value.
// to set default level, read atcommand_athena.conf first please.
static AtCommandInfo atcommand_info[] = {
	{ AtCommand_Rura,				"@rura",			40, atcommand_rura },
	{ AtCommand_Warp,				"@warp",			40, atcommand_rura },
	{ AtCommand_Where,				"@where",			 1, atcommand_where },
	{ AtCommand_JumpTo,				"@jumpto",			20, atcommand_jumpto }, // + /shift
	{ AtCommand_JumpTo,				"@warpto",			20, atcommand_jumpto },
	{ AtCommand_JumpTo,				"@goto",			20, atcommand_jumpto },
	{ AtCommand_Jump,				"@jump",			40, atcommand_jump },
	{ AtCommand_Who,				"@who",				20, atcommand_who },
	{ AtCommand_Who,				"@whois",			20, atcommand_who },
	{ AtCommand_Who2,				"@who2",			20, atcommand_who2 },
	{ AtCommand_Who3,				"@who3",			20, atcommand_who3 },
	{ AtCommand_WhoMap,				"@whomap",			20, atcommand_whomap },
	{ AtCommand_WhoMap2,			"@whomap2",			20, atcommand_whomap2 },
	{ AtCommand_WhoMap3,			"@whomap3",			20, atcommand_whomap3 },
	{ AtCommand_WhoGM,				"@whogm",			20, atcommand_whogm }, // by Yor
	{ AtCommand_Save,				"@save",			40, atcommand_save },
	{ AtCommand_Load,				"@return",			40, atcommand_load },
	{ AtCommand_Load,				"@load",			40, atcommand_load },
	{ AtCommand_Speed,				"@speed",			40, atcommand_speed },
	{ AtCommand_Storage,			"@storage",			 1, atcommand_storage },
	{ AtCommand_GuildStorage,		"@gstorage",		50, atcommand_guildstorage },
	{ AtCommand_Option,				"@option",			40, atcommand_option },
	{ AtCommand_Hide,				"@hide",			40, atcommand_hide }, // + /hide
	{ AtCommand_JobChange,			"@jobchange",		40, atcommand_jobchange },
	{ AtCommand_JobChange,			"@job",				40, atcommand_jobchange },
	{ AtCommand_Die,				"@die",				 1, atcommand_die },
	{ AtCommand_Kill,				"@kill",			60, atcommand_kill },
	{ AtCommand_Alive,				"@alive",			60, atcommand_alive },
	{ AtCommand_Kami,				"@kami",			40, atcommand_kami },
	{ AtCommand_KamiB,				"@kamib",			40, atcommand_kami },
	{ AtCommand_Heal,				"@heal",			40, atcommand_heal },
	{ AtCommand_Item,				"@item",			60, atcommand_item },
	{ AtCommand_Item2,				"@item2",			60, atcommand_item2 },
	{ AtCommand_ItemReset,			"@itemreset",		40, atcommand_itemreset },
	{ AtCommand_ItemCheck,			"@itemcheck",		60, atcommand_itemcheck },
	{ AtCommand_BaseLevelUp,		"@lvup",			60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,		"@blevel",			60, atcommand_baselevelup },
	{ AtCommand_BaseLevelUp,		"@baselvlup",		60, atcommand_baselevelup },
	{ AtCommand_JobLevelUp,			"@jlevel",			60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,			"@joblvup",			60, atcommand_joblevelup },
	{ AtCommand_JobLevelUp,			"@joblvlup",		60, atcommand_joblevelup },
	{ AtCommand_H,					"@h",				20, atcommand_help },
	{ AtCommand_Help,				"@help",			20, atcommand_help },
	{ AtCommand_GM,					"@gm",				100, atcommand_gm },
	{ AtCommand_PvPOff,				"@pvpoff",			40, atcommand_pvpoff },
	{ AtCommand_PvPOn,				"@pvpon",			40, atcommand_pvpon },
	{ AtCommand_GvGOff,				"@gvgoff",			40, atcommand_gvgoff },
	{ AtCommand_GvGOff,				"@gpvpoff",			40, atcommand_gvgoff },
	{ AtCommand_GvGOn,				"@gvgon",			40, atcommand_gvgon },
	{ AtCommand_GvGOn,				"@gpvpon",			40, atcommand_gvgon },
	{ AtCommand_Model,				"@model",			20, atcommand_model },
	{ AtCommand_Go,					"@go",				10, atcommand_go },
	{ AtCommand_Spawn,				"@monster",			50, atcommand_spawn },
	{ AtCommand_Spawn,				"@spawn",			50, atcommand_spawn },
//	{ AtCommand_Spawn,				"@summon",			50, atcommand_spawn },
	{ AtCommand_Monster,			"@monster2",		50, atcommand_monster },
	{ AtCommand_MonsterSmall,		"@monstersmall",	50, atcommand_monstersmall },
	{ AtCommand_MonsterBig,			"@monsterbig",		50, atcommand_monsterbig },
	{ AtCommand_KillMonster,		"@killmonster",		60, atcommand_killmonster },
	{ AtCommand_KillMonster2,		"@killmonster2",	40, atcommand_killmonster2 },
	{ AtCommand_Refine,				"@refine",			60, atcommand_refine },
	{ AtCommand_Produce,			"@produce",         60, atcommand_produce },
	{ AtCommand_Memo,				"@memo",			40, atcommand_memo },
	{ AtCommand_GAT,				"@gat",				99, atcommand_gat }, // debug function
	{ AtCommand_Packet,				"@packet",			99, atcommand_packet }, // debug function
	{ AtCommand_StatusPoint,		"@stpoint",			60, atcommand_statuspoint },
	{ AtCommand_SkillPoint,			"@skpoint",			60, atcommand_skillpoint },
	{ AtCommand_Zeny,				"@zeny",			60, atcommand_zeny },
	{ AtCommand_Strength,			"@str",				60, atcommand_param },
	{ AtCommand_Agility,			"@agi",				60, atcommand_param },
	{ AtCommand_Vitality,			"@vit",				60, atcommand_param },
	{ AtCommand_Intelligence,		"@int",				60, atcommand_param },
	{ AtCommand_Dexterity,			"@dex",				60, atcommand_param },
	{ AtCommand_Luck,				"@luk",				60, atcommand_param },
	{ AtCommand_GuildLevelUp,		"@guildlvup",		60, atcommand_guildlevelup },
	{ AtCommand_GuildLevelUp,		"@guildlvlup",		60, atcommand_guildlevelup },
	{ AtCommand_MakeEgg,			"@makeegg",			60, atcommand_makeegg },
	{ AtCommand_Hatch,				"@hatch",			60, atcommand_hatch },
	{ AtCommand_PetFriendly,		"@petfriendly",		40, atcommand_petfriendly },
	{ AtCommand_PetHungry,			"@pethungry",		40, atcommand_pethungry },
	{ AtCommand_PetRename,			"@petrename",		 1, atcommand_petrename },
	{ AtCommand_Recall,				"@recall",			60, atcommand_recall }, // + /recall
	{ AtCommand_Revive,				"@revive",			60, atcommand_revive },
	{ AtCommand_Night,				"@night",			80, atcommand_night },
	{ AtCommand_Day,				"@day",				80, atcommand_day },
	{ AtCommand_Doom,				"@doom",			80, atcommand_doom },
	{ AtCommand_DoomMap,			"@doommap",			80, atcommand_doommap },
	{ AtCommand_Raise,				"@raise",			80, atcommand_raise },
	{ AtCommand_RaiseMap,			"@raisemap",		80, atcommand_raisemap },
	{ AtCommand_CharacterBaseLevel,	"@charbaselvl",		60, atcommand_character_baselevel },
	{ AtCommand_CharacterJobLevel,	"@charjlvl",		60, atcommand_character_joblevel },
	{ AtCommand_Kick,				"@kick",			20, atcommand_kick }, // + right click menu for GM "(name) force to quit"
	{ AtCommand_KickAll,			"@kickall",			99, atcommand_kickall },
	{ AtCommand_AllSkill,			"@allskill",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@allskills",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@skillall",		60, atcommand_allskill },
	{ AtCommand_AllSkill,			"@skillsall",		60, atcommand_allskill },
	{ AtCommand_QuestSkill,			"@questskill",		40, atcommand_questskill },
	{ AtCommand_CharQuestSkill,		"@charquestskill",	60, atcommand_charquestskill },
	{ AtCommand_LostSkill,			"@lostskill",		40, atcommand_lostskill },
	{ AtCommand_CharLostSkill,		"@charlostskill",	60, atcommand_charlostskill },
	{ AtCommand_SpiritBall,			"@spiritball",		40, atcommand_spiritball },
	{ AtCommand_Party,				"@party",			 1, atcommand_party },
	{ AtCommand_Guild,				"@guild",			50, atcommand_guild },
	{ AtCommand_AgitStart,			"@agitstart",		60, atcommand_agitstart },
	{ AtCommand_AgitEnd,			"@agitend",			60, atcommand_agitend },
	{ AtCommand_MapExit,			"@mapexit",			99, atcommand_mapexit },
	{ AtCommand_IDSearch,			"@idsearch",		60, atcommand_idsearch },
	{ AtCommand_MapMove,			"@mapmove",			40, atcommand_rura }, // /mm command
	{ AtCommand_Broadcast,			"@broadcast",		40, atcommand_broadcast }, // /b and /nb command
	{ AtCommand_LocalBroadcast,		"@localbroadcast",	40, atcommand_localbroadcast }, // /lb and /nlb command
	{ AtCommand_RecallAll,			"@recallall",		80, atcommand_recallall },
	{ AtCommand_CharSkReset,		"@charskreset",		60, atcommand_charskreset },
	{ AtCommand_CharStReset,		"@charstreset",		60, atcommand_charstreset },
	{ AtCommand_ReloadItemDB,		"@reloaditemdb",	99, atcommand_reloaditemdb }, // admin command
	{ AtCommand_ReloadMobDB,		"@reloadmobdb",		99, atcommand_reloadmobdb }, // admin command
	{ AtCommand_ReloadSkillDB,		"@reloadskilldb",	99, atcommand_reloadskilldb }, // admin command
	{ AtCommand_ReloadScript,		"@reloadscript",	99, atcommand_reloadscript }, // admin command
	{ AtCommand_ReloadGMDB,			"@reloadgmdb",		99, atcommand_reloadgmdb }, // admin command
	{ AtCommand_ReloadAtcommand,	"@reloadatcommand",	99, atcommand_reloadatcommand },
	{ AtCommand_ReloadBattleConf,	"@reloadbattleconf",99, atcommand_reloadbattleconf },
	{ AtCommand_ReloadStatusDB,		"@reloadstatusdb",	99, atcommand_reloadstatusdb },
	{ AtCommand_ReloadPcDB,			"@reloadpcdb",		99, atcommand_reloadpcdb },
	{ AtCommand_CharModel,			"@charmodel",		50, atcommand_charmodel },
	{ AtCommand_CharSKPoint,		"@charskpoint",		60, atcommand_charskpoint },
	{ AtCommand_CharSTPoint,		"@charstpoint",		60, atcommand_charstpoint },
	{ AtCommand_MapInfo,			"@mapinfo",			99, atcommand_mapinfo },
	{ AtCommand_Dye,				"@dye",				40, atcommand_dye }, // by fritz
	{ AtCommand_Dye,				"@ccolor",			40, atcommand_dye }, // by fritz
	{ AtCommand_Hstyle,				"@hairstyle", 		40, atcommand_hair_style }, // by fritz
	{ AtCommand_Hstyle,				"@hstyle",			40, atcommand_hair_style }, // by fritz
	{ AtCommand_Hcolor,				"@haircolor",		40, atcommand_hair_color }, // by fritz
	{ AtCommand_Hcolor,				"@hcolor",			40, atcommand_hair_color }, // by fritz
	{ AtCommand_StatAll,			"@statall",			60, atcommand_stat_all }, // by fritz
	{ AtCommand_StatAll,			"@statsall",		60, atcommand_stat_all },
	{ AtCommand_StatAll,			"@allstats",		60, atcommand_stat_all }, // by fritz
	{ AtCommand_StatAll,			"@allstat",			60, atcommand_stat_all }, // by fritz
//	{ AtCommand_CharChangeSex,		"@charchangesex",	60, atcommand_char_change_sex }, // by Yor
	{ AtCommand_CharBlock,			"@block",			60, atcommand_char_block }, // by Yor
	{ AtCommand_CharBlock,			"@charblock",		60, atcommand_char_block }, // by Yor
	{ AtCommand_CharBan,			"@ban",				60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@banish",			60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@charban",			60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharBan,			"@charbanish",		60, atcommand_char_ban }, // by Yor
	{ AtCommand_CharUnBlock,		"@unblock",			60, atcommand_char_unblock }, // by Yor
	{ AtCommand_CharUnBlock,		"@charunblock",		60, atcommand_char_unblock }, // by Yor
	{ AtCommand_CharUnBan,			"@unban",			60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@unbanish",		60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@charunban",		60, atcommand_char_unban }, // by Yor
	{ AtCommand_CharUnBan,			"@charunbanish",	60, atcommand_char_unban }, // by Yor
	{ AtCommand_MountPeco,			"@mountpeco",		20, atcommand_mount_peco }, // by Valaris
	{ AtCommand_CharMountPeco,		"@charmountpeco",	50, atcommand_char_mount_peco }, // by Yor
	{ AtCommand_GuildSpy,			"@guildspy",		60, atcommand_guildspy }, // [Syrus22]
	{ AtCommand_PartySpy,			"@partyspy",		60, atcommand_partyspy }, // [Syrus22]
	{ AtCommand_RepairAll,			"@repairall",		60, atcommand_repairall }, // [Valaris]
	{ AtCommand_GuildRecall,		"@guildrecall",		60, atcommand_guildrecall }, // by Yor
	{ AtCommand_PartyRecall,		"@partyrecall",		60, atcommand_partyrecall }, // by Yor
	{ AtCommand_Nuke,				"@nuke",			60, atcommand_nuke }, // [Valaris]
	{ AtCommand_Enablenpc,			"@enablenpc",		80, atcommand_enablenpc }, // []
	{ AtCommand_Hidenpc,			"@hidenpc",			80, atcommand_hidenpc }, // []
	{ AtCommand_Disablenpc,			"@disablenpc",		80, atcommand_disablenpc }, // []
	{ AtCommand_ServerTime,			"@time",			 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@date",			 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@server_date",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@serverdate",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@server_time",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_ServerTime,			"@servertime",		 0, atcommand_servertime }, // by Yor
	{ AtCommand_CharDelItem,		"@chardelitem",		60, atcommand_chardelitem }, // by Yor
	{ AtCommand_Jail,				"@jail",			60, atcommand_jail }, // by Yor
	{ AtCommand_UnJail,				"@unjail",			60, atcommand_unjail }, // by Yor
	{ AtCommand_UnJail,				"@discharge",		60, atcommand_unjail }, // by Yor
	{ AtCommand_Disguise,			"@disguise",		20, atcommand_disguise }, // [Valaris]
	{ AtCommand_UnDisguise,			"@undisguise",		20, atcommand_undisguise }, // by Yor
	{ AtCommand_CharDisguise,		"@chardisguise",	60, atcommand_chardisguise }, // Kalaspuff
	{ AtCommand_CharUnDisguise,		"@charundisguise",	60, atcommand_charundisguise }, // Kalaspuff
	{ AtCommand_EMail,				"@email",			 0, atcommand_email }, // by Yor
	{ AtCommand_Effect,				"@effect",			40, atcommand_effect }, // by Apple
//	{ AtCommand_Char_Item_List,		"@charitemlist",	40, atcommand_character_item_list }, // by Yor, now #itemlist
//	{ AtCommand_Char_Storage_List,	"@charstoragelist",	40, atcommand_character_storage_list }, // by Yor, now #storagelist
	{ AtCommand_Char_Cart_List,		"@charcartlist",	40, atcommand_character_cart_list }, // by Yor
	{ AtCommand_Follow,				"@follow",			10, atcommand_follow }, // by MouseJstr
	{ AtCommand_AddWarp,			"@addwarp",			20, atcommand_addwarp }, // by MouseJstr
	{ AtCommand_SkillOn,			"@skillon",			20, atcommand_skillon }, // by MouseJstr
	{ AtCommand_SkillOff,			"@skilloff",		20, atcommand_skilloff }, // by MouseJstr
	{ AtCommand_Killer,				"@killer",			60, atcommand_killer }, // by MouseJstr
	{ AtCommand_NpcMove,			"@npcmove",			20, atcommand_npcmove }, // by MouseJstr
	{ AtCommand_Killable,			"@killable",		40, atcommand_killable }, // by MouseJstr
	{ AtCommand_CharKillable,		"@charkillable",	40, atcommand_charkillable }, // by MouseJstr
	{ AtCommand_Dropall,			"@dropall",			40, atcommand_dropall }, // MouseJstr
	{ AtCommand_Chardropall,		"@chardropall",		40, atcommand_chardropall }, // MouseJstr
	{ AtCommand_Storeall,			"@storeall",		40, atcommand_storeall }, // MouseJstr
	{ AtCommand_Charstoreall,		"@charstoreall",	40, atcommand_charstoreall }, // MouseJstr
	{ AtCommand_Skillid,			"@skillid",			40, atcommand_skillid }, // MouseJstr
	{ AtCommand_Useskill,			"@useskill",		40, atcommand_useskill }, // MouseJstr
	{ AtCommand_Rain,				"@rain",			99, atcommand_rain },
	{ AtCommand_Snow,				"@snow",			99, atcommand_snow },
	{ AtCommand_Sakura,				"@sakura",			99, atcommand_sakura },
	{ AtCommand_Fog,				"@fog",				99,	atcommand_fog },
	{ AtCommand_Leaves,				"@leaves",			99, atcommand_leaves },
/*
	{ AtCommand_Shuffle,			"@shuffle",			99, atcommand_shuffle },
	{ AtCommand_Maintenance,		"@maintenance",		99, atcommand_maintenance },
	{ AtCommand_Misceffect,			"@misceffect",		60, atcommand_misceffect },
*/
	{ AtCommand_Summon,				"@summon",			60, atcommand_summon },
	{ AtCommand_AdjGmLvl,			"@adjgmlvl",		99, atcommand_adjgmlvl },
	{ AtCommand_AdjCmdLvl,			"@adjcmdlvl",		99, atcommand_adjcmdlvl },
	{ AtCommand_Trade,				"@trade",			60, atcommand_trade },
	{ AtCommand_Send,				"@send",			60, atcommand_send },
	{ AtCommand_SetBattleFlag,		"@setbattleflag",	60, atcommand_setbattleflag },
	{ AtCommand_UnMute,				"@unmute",			60, atcommand_unmute }, // [Valaris]
	{ AtCommand_Clearweather,		"@clearweather",	99, atcommand_clearweather }, // Dexity
	{ AtCommand_UpTime,				"@uptime",			 0, atcommand_uptime }, // by MC Cameri
//	{ AtCommand_ChangeSex,			"@changesex",		 1, atcommand_changesex }, // by MC Cameri
	{ AtCommand_Mute,				"@mute",			99, atcommand_mute }, // [celest]
	{ AtCommand_Mute,				"@red",				99, atcommand_mute }, // [celest]
	{ AtCommand_WhoZeny,			"@whozeny",			20, atcommand_whozeny }, // [Valaris]
	{ AtCommand_HappyHappyJoyJoy,	"@happyhappyjoyjoy",40, atcommand_happyhappyjoyjoy }, // [Valaris]
	{ AtCommand_Refresh,	        "@refresh",			 0, atcommand_refresh }, // by MC Cameri
	{ AtCommand_PetId,	    	    "@petid",			40, atcommand_petid }, // by MC Cameri
	{ AtCommand_Identify,	   	    "@identify",		40, atcommand_identify }, // by MC Cameri
	{ AtCommand_Gmotd,				"@gmotd",			 0, atcommand_gmotd }, // Added by MC Cameri, created by davidsiaw
	{ AtCommand_MiscEffect,			"@misceffect",		50, atcommand_misceffect }, // by MC Cameri
	{ AtCommand_MobSearch,			"@mobsearch",		 0, atcommand_mobsearch },
	{ AtCommand_CleanMap,			"@cleanmap",		 0, atcommand_cleanmap },
	{ AtCommand_NpcTalk,			"@npctalk",			 0,	atcommand_npctalk },
	{ AtCommand_PetTalk,			"@pettalk",			 0,	atcommand_pettalk },
	{ AtCommand_Users,				"@users",		 0, atcommand_users },
	{ AtCommand_ResetState,			"/reset",			40,	NULL },

#ifndef TXT_ONLY // sql-only commands
	{ AtCommand_CheckMail,			"@checkmail",		 1, atcommand_listmail }, // [Valaris]
	{ AtCommand_ListMail,			"@listmail",		 1, atcommand_listmail }, // [Valaris]
	{ AtCommand_ListNewMail,		"@listnewmail",		 1, atcommand_listmail }, // [Valaris]
	{ AtCommand_ReadMail,			"@readmail",		 1, atcommand_readmail }, // [Valaris]
	{ AtCommand_DeleteMail,			"@deletemail",		 1, atcommand_readmail }, // [Valaris]
	{ AtCommand_SendMail,			"@sendmail",		 1, atcommand_sendmail }, // [Valaris]
	{ AtCommand_SendPriorityMail,	"@sendprioritymail",80, atcommand_sendmail }, // [Valaris]
	{ AtCommand_RefreshOnline,		"@refreshonline",	99, atcommand_refreshonline }, // [Valaris]

#endif /* TXT_ONLY */
	{ AtCommand_SkillTree,		"@skilltree",	40, atcommand_skilltree }, // [MouseJstr]
	{ AtCommand_Marry,		"@marry",	40, atcommand_marry }, // [MouseJstr]
	{ AtCommand_Divorce,		"@divorce",	40, atcommand_divorce }, // [MouseJstr]
	{ AtCommand_Rings,		"@rings",	40, atcommand_rings }, // [MouseJstr]
	{ AtCommand_Grind,		"@grind",	99, atcommand_grind }, // [MouseJstr]
	{ AtCommand_Grind2,		"@grind2",	99, atcommand_grind2 }, // [MouseJstr]

#ifdef DMALLOC
	{ AtCommand_DMStart,		"@dmstart",	99, atcommand_dmstart }, // [MouseJstr]
	{ AtCommand_DMTick,		"@dmtick",	99, atcommand_dmtick }, // [MouseJstr]
#endif

	{ AtCommand_JumpToId,       "@jumptoid",  20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId,       "@warptoid",  20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId,       "@gotoid",  20, atcommand_jumptoid }, // [Dino9021]
	{ AtCommand_JumpToId2,       "@jumptoid2",  20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_JumpToId2,       "@warptoid2",  20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_JumpToId2,       "@gotoid2",  20, atcommand_jumptoid2 }, // [Dino9021]
	{ AtCommand_RecallId,       "@recallid", 60, atcommand_recallid }, // [Dino9021]
	{ AtCommand_RecallId2,       "@recallid2", 60, atcommand_recallid2 }, // [Dino9021]
	{ AtCommand_KickId,                "@kickid", 99, atcommand_kickid }, // [Dino9021]
	{ AtCommand_KickId2,                "@kickid2", 99, atcommand_kickid2 }, // [Dino9021]
	{ AtCommand_ReviveId,              "@reviveid", 60, atcommand_reviveid }, // [Dino9021]
	{ AtCommand_ReviveId2,              "@reviveid2", 60, atcommand_reviveid2 }, // [Dino9021]
	{ AtCommand_KillId,                "@killid", 60, atcommand_killid }, // [Dino9021]
	{ AtCommand_KillId2,                "@killid2", 60, atcommand_killid2 }, // [Dino9021]
	{ AtCommand_CharKillableId,      "@charkillableid",    40, atcommand_charkillableid }, // [Dino9021]
	{ AtCommand_CharKillableId2,      "@charkillableid2",    40, atcommand_charkillableid2 }, // [Dino9021]
	{ AtCommand_Sound,		"@sound",	40,	atcommand_sound },
	{ AtCommand_UndisguiseAll,		"@undisguiseall",	99,	atcommand_undisguiseall },
	{ AtCommand_DisguiseAll,		"@disguiseall",	99,	atcommand_disguiseall },
	{ AtCommand_ChangeLook,		"@changelook",	99,	atcommand_changelook },
	{ AtCommand_AutoLoot,		"@autoloot",	10,	atcommand_autoloot }, // Upa-Kun
	{ AtCommand_MobInfo,		"@mobinfo",	1,	atcommand_mobinfo }, // [Lupus]
	{ AtCommand_MobInfo,		"@monsterinfo",	1,	atcommand_mobinfo }, // [Lupus]
	{ AtCommand_MobInfo,		"@mi",	1,	atcommand_mobinfo }, // [Lupus]
        { AtCommand_Adopt,              "@adopt",       40, atcommand_adopt }, // [Veider]
	
// add new commands before this line
	{ AtCommand_Unknown,             NULL,                1, NULL }
};

/*=========================================
 * Generic variables
 *-----------------------------------------
 */
char atcmd_output[200];
char atcmd_player_name[100];

/*====================================================
 * This function return the name of the job (by [Yor])
 *----------------------------------------------------
 */
char * job_name(int class_) {
	switch (class_) {
	case 0:    return "Novice";
	case 1:    return "Swordsman";
	case 2:    return "Mage";
	case 3:    return "Archer";
	case 4:    return "Acolyte";
	case 5:    return "Merchant";
	case 6:    return "Thief";
	case 7:    return "Knight";
	case 8:    return "Priest";
	case 9:    return "Wizard";
	case 10:   return "Blacksmith";
	case 11:   return "Hunter";
	case 12:   return "Assassin";
	case 13:   return "Knight 2";
	case 14:   return "Crusader";
	case 15:   return "Monk";
	case 16:   return "Sage";
	case 17:   return "Rogue";
	case 18:   return "Alchemist";
	case 19:   return "Bard";
	case 20:   return "Dancer";
	case 21:   return "Crusader 2";
	case 22:   return "Wedding";
	case 23:   return "Super Novice";
	case 4001: return "Novice High";
	case 4002: return "Swordsman High";
	case 4003: return "Mage High";
	case 4004: return "Archer High";
	case 4005: return "Acolyte High";
	case 4006: return "Merchant High";
	case 4007: return "Thief High";
	case 4008: return "Lord Knight";
	case 4009: return "High Priest";
	case 4010: return "High Wizard";
	case 4011: return "Whitesmith";
	case 4012: return "Sniper";
	case 4013: return "Assassin Cross";
	case 4014: return "Peko Knight";
	case 4015: return "Paladin";
	case 4016: return "Champion";
	case 4017: return "Professor";
	case 4018: return "Stalker";
	case 4019: return "Creator";
	case 4020: return "Clown";
	case 4021: return "Gypsy";
	case 4022: return "Peko Paladin";
	case 4023: return "Baby Novice";
	case 4024: return "Baby Swordsman";
	case 4025: return "Baby Mage";
	case 4026: return "Baby Archer";
	case 4027: return "Baby Acolyte";
	case 4028: return "Baby Merchant";
	case 4029: return "Baby Thief";
	case 4030: return "Baby Knight";
	case 4031: return "Baby Priest";
	case 4032: return "Baby Wizard";
	case 4033: return "Baby Blacksmith";
	case 4034: return "Baby Hunter";
	case 4035: return "Baby Assassin";
	case 4036: return "Baby Peco Knight";
	case 4037: return "Baby Crusader";
	case 4038: return "Baby Monk";
	case 4039: return "Baby Sage";
	case 4040: return "Baby Rogue";
	case 4041: return "Baby Alchemist";
	case 4042: return "Baby Bard";
	case 4043: return "Baby Dancer";
	case 4044: return "Baby Peco Crusader";
	case 4045: return "Super Baby";
	}
	return "Unknown Job";
}

/*==========================================
 * estr_lower (replace strlwr, non ANSI function that doesn't exist in all C compilator)
 *------------------------------------------
 */
char *estr_lower(char *str)
{
	int i;

	for (i=0; str[i]; i++)
		if ((str[i] >= 65) && (str[i] <= 90))
			str[i] += 32;
	return str;
}

// compare function for sorting high to lowest
int hightolow_compare (const void * a, const void * b)
{
  return ( *(int*)b - *(int*)a );
}

// compare function for sorting lowest to highest
int lowtohigh_compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

//-----------------------------------------------------------
// Return the message string of the specified number by [Yor]
//-----------------------------------------------------------
char * msg_txt(int msg_number) {
	if (msg_number >= 0 && msg_number < MAX_MSG &&
	    msg_table[msg_number] != NULL && msg_table[msg_number][0] != '\0')
		return msg_table[msg_number];

	return "??";
}

//------------------------------------------------------------
// E-mail check: return 0 (not correct) or 1 (valid). by [Yor]
//------------------------------------------------------------
int e_mail_check(char *email) {
	char ch;
	char* last_arobas;

	// athena limits
	if (strlen(email) < 3 || strlen(email) > 39)
		return 0;

	// part of RFC limits (official reference of e-mail description)
	if (strchr(email, '@') == NULL || email[strlen(email)-1] == '@')
		return 0;

	if (email[strlen(email)-1] == '.')
		return 0;

	last_arobas = strrchr(email, '@');

	if (strstr(last_arobas, "@.") != NULL ||
	    strstr(last_arobas, "..") != NULL)
		return 0;

	for(ch = 1; ch < 32; ch++) {
		if (strchr(last_arobas, ch) != NULL) {
			return 0;
			break;
		}
	}

	if (strchr(last_arobas, ' ') != NULL ||
	    strchr(last_arobas, ';') != NULL)
		return 0;

	// all correct
	return 1;
}

/*==========================================
 * get_atcommand_level @ÉRÉ}ÉìÉhÇÃïKóvÉåÉxÉãÇéÊìæ
 *------------------------------------------
 */
int get_atcommand_level(const AtCommandType type) {
	int i;

	for (i = 0; atcommand_info[i].type != AtCommand_None; i++)
		if (atcommand_info[i].type == type)
			return atcommand_info[i].level;

	return 100; // 100: command can not be used
}

/*==========================================
 *is_atcommand @ÉRÉ}ÉìÉhÇ…ë∂ç›Ç∑ÇÈÇ©Ç«Ç§Ç©ämîFÇ∑ÇÈ
 *------------------------------------------
 */
AtCommandType
is_atcommand(const int fd, struct map_session_data* sd, const char* message, int gmlvl) {
	const char* str = message;
	int s_flag = 0;
	AtCommandInfo info;
	AtCommandType type;

	nullpo_retr(AtCommand_None, sd);

	if (!battle_config.allow_atcommand_when_mute &&
		sd->sc_count && sd->sc_data[SC_NOCHAT].timer != -1) {
		return AtCommand_Unknown;
	}

	if (!message || !*message)
		return AtCommand_None;

	memset(&info, 0, sizeof(info));
	str += strlen(sd->status.name);
	while (*str && (isspace(*str) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return AtCommand_None;

	type = atcommand(sd, gmlvl > 0 ? gmlvl : pc_isGM(sd), str, &info);
	if (type != AtCommand_None) {
		char command[100];
		const char* p = str;
		memset(command, '\0', sizeof(command));
		memset(atcmd_output, '\0', sizeof(atcmd_output));
		while (*p && !isspace(*p))
			p++;
		if (p - str >= sizeof(command)) // too long
			return AtCommand_Unknown;
		strncpy(command, str, p - str);
		while (isspace(*p))
			p++;

		if (type == AtCommand_Unknown || info.proc == NULL) {
			sprintf(atcmd_output, msg_table[153], command); // %s is Unknown Command.
			clif_displaymessage(fd, atcmd_output);
		} else {
			if (info.proc(fd, sd, command, p) != 0) {
				// Command can not be executed
				sprintf(atcmd_output, msg_table[154], command); // %s failed.
				clif_displaymessage(fd, atcmd_output);
			}
		}

		return info.type;
	}

	return AtCommand_None;
}

/*==========================================
 *
 *------------------------------------------
 */
AtCommandType atcommand(struct map_session_data* sd, const int level, const char* message, struct AtCommandInfo* info) {
	char* p = (char *)message; // it's 'char' and not 'const char' to have possibility to modify the first character if necessary

	if (!info)
		return AtCommand_None;
	if (battle_config.atc_gmonly != 0 && !level) // level = pc_isGM(sd)
		return AtCommand_None;
	if (!p || !*p) {
		fprintf(stderr, "at command message is empty\n");
		return AtCommand_None;
	}

	if (*p == command_symbol) { // check first char.
		char command[101];
		int i = 0;
		memset(info, 0, sizeof(AtCommandInfo));
		sscanf(p, "%100s", command);
		command[sizeof(command)-1] = '\0';

		while (atcommand_info[i].type != AtCommand_Unknown) {
			if (strcmpi(command+1, atcommand_info[i].command+1) == 0 && level >= atcommand_info[i].level) {
				p[0] = atcommand_info[i].command[0]; // set correct first symbol for after.
				break;
			}
			i++;
		}

		if (atcommand_info[i].type == AtCommand_Unknown) {
			// doesn't return Unknown if player is normal player (display the text, not display: unknown command)
			if (level == 0)
				return AtCommand_None;
			else
				return AtCommand_Unknown;
		} else if((log_config.gm) && (atcommand_info[i].level >= log_config.gm)) {
			log_atcommand(sd, message);
		}
		memcpy(info, &atcommand_info[i], sizeof atcommand_info[i]);
	} else {
		return AtCommand_None;
	}

	return info->type;
}

/*==========================================
 *
 *------------------------------------------
 */
static int atkillmonster_sub(struct block_list *bl, va_list ap) {
	struct mob_data *md;
	int flag;
	
	nullpo_retr(0, ap);
	nullpo_retr(0, md=(struct mob_data *)bl);
	flag = va_arg(ap, int);

	if (flag)
		mob_damage(NULL, md, md->hp, 2);
	else
		mob_delete(md);
	
	return 0;
}
/*==========================================
 * Mob search
 *------------------------------------------
 */
static int atmobsearch_sub(struct block_list *bl,va_list ap)
{
	int mob_id,fd;
	static int number=0;
	struct mob_data *md;

	nullpo_retr(0, bl);

	if(!ap){
		number=0;
		return 0;
	}
	mob_id = va_arg(ap,int);
	fd = va_arg(ap,int);

	md = (struct mob_data *)bl;

	if(md && fd && (mob_id==-1 || (md->class_==mob_id))){
		snprintf(atcmd_output, sizeof atcmd_output, "%2d[%3d:%3d] %s",
				++number,bl->x, bl->y,md->name);
		clif_displaymessage(fd, atcmd_output);
	}
	return 0;
}
/*==========================================
 * cleanmap
 *------------------------------------------
 */
static int atcommand_cleanmap_sub(struct block_list *bl,va_list ap)
{
	struct flooritem_data *fitem;

	nullpo_retr(0, bl);

	fitem = (struct flooritem_data *)bl;
	if(fitem==NULL || fitem->bl.type!=BL_ITEM){
		if(battle_config.error_log)
			printf("map_clearflooritem_timer : error\n");
		return 1;
	}
	delete_timer(fitem->cleartimer,map_clearflooritem_timer);
	if(fitem->item_data.card[0] == (short)0xff00)
		intif_delete_petdata(*((long *)(&fitem->item_data.card[1])));
	clif_clearflooritem(fitem,0);
	map_delobject(fitem->bl.id);

	return 0;
}

/*==========================================
 * Read Message Data
 *------------------------------------------
 */
int msg_config_read(const char *cfgName) {
	int msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int called = 1;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		printf("Messages file not found: %s\n", cfgName);
		return 1;
	}

	if ((--called) == 0)
		memset(&msg_table[0], 0, sizeof(msg_table[0]) * MAX_MSG);
	while(fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) == 2) {
			if (strcmpi(w1, "import") == 0) {
				msg_config_read(w2);
			} else {
				msg_number = atoi(w1);
				if (msg_number >= 0 && msg_number < MAX_MSG) {
					if (msg_table[msg_number] != NULL)
						aFree(msg_table[msg_number]);
					msg_table[msg_number] = (char *)aCalloc(strlen(w2) + 1, sizeof (char));
					strcpy(msg_table[msg_number],w2);
				//	printf("message #%d: '%s'.\n", msg_number, msg_table[msg_number]);
				}
			}
		}
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------
 */
void do_final_msg () {
	int i;
	for (i = 0; i < MAX_MSG; i++)
		aFree(msg_table[i]);
	return;
}

/*==========================================
 *
 *------------------------------------------
 */
static AtCommandInfo* get_atcommandinfo_byname(const char* name) {
	int i;

	for (i = 0; atcommand_info[i].type != AtCommand_Unknown; i++)
		if (strcmpi(atcommand_info[i].command + 1, name) == 0)
			return &atcommand_info[i];

	return NULL;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	AtCommandInfo* p;
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		printf("At commands configuration file not found: %s\n", cfgName);
		return 1;
	}

	while (fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
			continue;
		p = get_atcommandinfo_byname(w1);
		if (p != NULL) {
			p->level = atoi(w2);
			if (p->level > 100)
				p->level = 100;
			else if (p->level < 0)
				p->level = 0;
		}

		if (strcmpi(w1, "import") == 0)
			atcommand_config_read(w2);
		else if (strcmpi(w1, "command_symbol") == 0 && w2[0] > 31 &&
		         w2[0] != '/' && // symbol of standard ragnarok GM commands
		         w2[0] != '%' && // symbol of party chat speaking
				 w2[0] != '#')	// symbol of charcommand
			command_symbol = w2[0];
	}
	fclose(fp);

	return 0;
}



/*==========================================
// @ command processing functions
 *------------------------------------------
 */

/*==========================================
 * @send (used for testing packet sends from the client)
 *------------------------------------------
 */
int atcommand_send(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int a = atoi(message);
	if (a)
	{
		unsigned char buf[1024];
		switch(a)
		{
		case 1:
			WBUFW(buf,0)=0x18d;
		case 2:
			WBUFW(buf,0)=0x18e;
		case 3:
			WBUFW(buf,0)=0x18f;
		case 4:
			WBUFW(buf,0)=0x190;
		}
	}
	return 0;
}

// @rura
/*==========================================
 *
 *------------------------------------------
 */
int atcommand_rura(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[100];
	int x = 0, y = 0;
	int m;

	nullpo_retr(-1, sd);

	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message || sscanf(message, "%99s %d %d", map_name, &x, &y) < 1) {
		clif_displaymessage(fd, "Please, enter a map (usage: @warp/@rura/@mapmove <mapname> <x> <y>).");
		return -1;
	}

	if (x <= 0)
		x = rand() % 399 + 1;
	if (y <= 0)
		y = rand() % 399 + 1;

	if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
		strcat(map_name, ".gat");

	if (x > 0 && x < 400 && y > 0 && y < 400) {
		m = map_mapname2mapid(map_name);
		if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_table[247]);
			return -1;
		}
		if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_table[248]);
			return -1;
		}
		if (pc_setpos(sd, map_name, x, y, 3) == 0)
			clif_displaymessage(fd, msg_table[0]); // Warped.
		else {
			clif_displaymessage(fd, msg_table[1]); // Map not found.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_where(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);
	if (sscanf(message, "%99[^\n]", atcmd_player_name) < 1)
		return -1;
	if(strncmp(sd->status.name,atcmd_player_name,24)==0)
		return -1;

	if ((pl_sd = map_nick2sd(atcmd_player_name)) == NULL) {
		snprintf(atcmd_output, sizeof atcmd_output, "%s %d %d",
			sd->mapname, sd->bl.x, sd->bl.y);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}
	snprintf(atcmd_output, sizeof atcmd_output, "%s %s %d %d",
		atcmd_player_name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_jumpto(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jumpto/@warpto/@goto <char name>).");
		return -1;
	}

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);
	if (sscanf(message, "%99[^\n]", atcmd_player_name) < 1)
		return -1;
	if(strncmp(sd->status.name,atcmd_player_name,24)==0) //Yourself mate? Tsk tsk tsk.
		return -1;

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_table[247]);
			return -1;
		}
		if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_table[248]);
			return -1;
		}
		pc_setpos(sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
		sprintf(atcmd_output, msg_table[4], atcmd_player_name); // Jump to %s
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_jump(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int x = 0, y = 0;

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	sscanf(message, "%d %d", &x, &y);

	if (x <= 0)
		x = rand() % 399 + 1;
	if (y <= 0)
		y = rand() % 399 + 1;
	if (x > 0 && x < 400 && y > 0 && y < 400) {
		if (sd->bl.m >= 0 && (map[sd->bl.m].flag.nowarp || map[sd->bl.m].flag.nowarpto) && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
			clif_displaymessage(fd, msg_table[248]);
			return -1;
		}
		pc_setpos(sd, sd->mapname, x, y, 3);
		sprintf(atcmd_output, msg_table[5], x, y); // Jump to %d %d
		clif_displaymessage(fd, atcmd_output);
	} else {
		clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_who(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i, j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[24];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				memcpy(player_name, pl_sd->status.name, 24);
				for (j = 0; player_name[j]; j++)
					player_name[j] = tolower(player_name[j]);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "(CID:%d/AID:%d) Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.char_id, pl_sd->status.account_id, pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					else
						sprintf(atcmd_output, "(CID:%d/AID:%d) Name: %s | Location: %s %d %d", pl_sd->status.char_id, pl_sd->status.account_id, pl_sd->status.name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(atcmd_output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_who2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i, j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[24];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				memcpy(player_name, pl_sd->status.name, 24);
				for (j = 0; player_name[j]; j++)
					player_name[j] = tolower(player_name[j]);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					else
						sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(atcmd_output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_who3(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char temp0[100];
	char temp1[100];
	struct map_session_data *pl_sd;
	int i, j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[24];
	struct guild *g;
	struct party *p;

	nullpo_retr(-1, sd);

	memset(temp0, '\0', sizeof(temp0));
	memset(temp1, '\0', sizeof(temp1));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				memcpy(player_name, pl_sd->status.name, 24);
				for (j = 0; player_name[j]; j++)
					player_name[j] = tolower(player_name[j]);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					g = guild_search(pl_sd->status.guild_id);
					if (g == NULL)
						sprintf(temp1, "None");
					else
						sprintf(temp1, "%s", g->name);
					p = party_search(pl_sd->status.party_id);
					if (p == NULL)
						sprintf(temp0, "None");
					else
						sprintf(temp0, "%s", p->name);
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_GM_level, temp0, temp1);
					else
						sprintf(atcmd_output, "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, temp0, temp1);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(atcmd_output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_whomap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i, count;
	int pl_GM_level, GM_level;
	int map_id;
	char map_name[100];

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%99s", map_name);
		if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
			strcat(map_name, ".gat");
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->bl.m == map_id) {
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					else
						sprintf(atcmd_output, "Name: %s | Location: %s %d %d", pl_sd->status.name, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_table[54], map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_table[55], map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_table[56], count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_whomap2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i, count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char map_name[100];

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%99s", map_name);
		if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
			strcat(map_name, ".gat");
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->bl.m == map_id) {
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_GM_level, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					else
						sprintf(atcmd_output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_table[54], map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_table[55], map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_table[56], count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_whomap3(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char temp0[100];
	char temp1[100];
	struct map_session_data *pl_sd;
	int i, count;
	int pl_GM_level, GM_level;
	int map_id = 0;
	char map_name[100];
	struct guild *g;
	struct party *p;

	nullpo_retr(-1, sd);

	memset(temp0, '\0', sizeof(temp0));
	memset(temp1, '\0', sizeof(temp1));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message)
		map_id = sd->bl.m;
	else {
		sscanf(message, "%99s", map_name);
		if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
			strcat(map_name, ".gat");
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
				if (pl_sd->bl.m == map_id) {
					g = guild_search(pl_sd->status.guild_id);
					if (g == NULL)
						sprintf(temp1, "None");
					else
						sprintf(temp1, "%s", g->name);
					p = party_search(pl_sd->status.party_id);
					if (p == NULL)
						sprintf(temp0, "None");
					else
						sprintf(temp0, "%s", p->name);
					if (pl_GM_level > 0)
						sprintf(atcmd_output, "Name: %s (GM:%d) | Party: '%s' | Guild: '%s'", pl_sd->status.name, pl_GM_level, temp0, temp1);
					else
						sprintf(atcmd_output, "Name: %s | Party: '%s' | Guild: '%s'", pl_sd->status.name, temp0, temp1);
					clif_displaymessage(fd, atcmd_output);
					count++;
				}
			}
		}
	}

	if (count == 0)
		sprintf(atcmd_output, msg_table[54], map[map_id].name); // No player found in map '%s'.
	else if (count == 1)
		sprintf(atcmd_output, msg_table[55], map[map_id].name); // 1 player found in map '%s'.
	else {
		sprintf(atcmd_output, msg_table[56], count, map[map_id].name); // %d players found in map '%s'.
	}
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_whogm(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char temp0[100];
	char temp1[100];
	struct map_session_data *pl_sd;
	int i, j, count;
	int pl_GM_level, GM_level;
	char match_text[100];
	char player_name[24];
	struct guild *g;
	struct party *p;

	nullpo_retr(-1, sd);

	memset(temp0, '\0', sizeof(temp0));
	memset(temp1, '\0', sizeof(temp1));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower(match_text[j]);

	count = 0;
	GM_level = pc_isGM(sd);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			pl_GM_level = pc_isGM(pl_sd);
			if (pl_GM_level > 0) {
				if (!((battle_config.hide_GM_session || (pl_sd->status.option & OPTION_HIDE)) && (pl_GM_level > GM_level))) { // you can look only lower or same level
					memcpy(player_name, pl_sd->status.name, 24);
					for (j = 0; player_name[j]; j++)
						player_name[j] = tolower(player_name[j]);
					if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
						sprintf(atcmd_output, "Name: %s (GM:%d) | Location: %s %d %d", pl_sd->status.name, pl_GM_level, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y);
						clif_displaymessage(fd, atcmd_output);
						sprintf(atcmd_output, "       BLvl: %d | Job: %s (Lvl: %d)", pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level);
						clif_displaymessage(fd, atcmd_output);
						g = guild_search(pl_sd->status.guild_id);
						if (g == NULL)
							sprintf(temp1, "None");
						else
							sprintf(temp1, "%s", g->name);
						p = party_search(pl_sd->status.party_id);
						if (p == NULL)
							sprintf(temp0, "None");
						else
							sprintf(temp0, "%s", p->name);
						sprintf(atcmd_output, "       Party: '%s' | Guild: '%s'", temp0, temp1);
						clif_displaymessage(fd, atcmd_output);
						count++;
					}
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[150]); // No GM found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[151]); // 1 GM found.
	else {
		sprintf(atcmd_output, msg_table[152], count); // %d GMs found.
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

int atcommand_whozeny(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i, j, count,c;
	char match_text[100];
	char player_name[24];
	//int zeny[clif_countusers()];
	//int counted[clif_countusers()];
	int *zeny = (int *)aCallocA(clif_countusers(), sizeof(int));
	int *counted = (int *)aCallocA(clif_countusers(), sizeof(int));

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(match_text, '\0', sizeof(match_text));
	memset(player_name, '\0', sizeof(player_name));

	if (sscanf(message, "%99[^\n]", match_text) < 1)
		strcpy(match_text, "");
	for (j = 0; match_text[j]; j++)
		match_text[j] = tolower(match_text[j]);

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				memcpy(player_name, pl_sd->status.name, 24);
				for (j = 0; player_name[j]; j++)
					player_name[j] = tolower(player_name[j]);
				if (strstr(player_name, match_text) != NULL) { // search with no case sensitive
					zeny[count]=pl_sd->status.zeny;
					counted[i]=0;
					count++;
				}
		}
	}

	qsort(zeny, count, sizeof(int), hightolow_compare);
	for (c = 0; c < count && c < 50; c++) {
		if(!zeny[c])
			continue;
		for (i = 0; i < fd_max; i++) {
			if(!zeny[c])
				continue;
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && zeny[c] && counted[i]==0) {
				if(pl_sd->status.zeny==zeny[c]) {
					sprintf(atcmd_output, "Name: %s | Zeny: %d", pl_sd->status.name, pl_sd->status.zeny);
					clif_displaymessage(fd, atcmd_output);
					zeny[c]=0;
					counted[i]=1;
				}
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(atcmd_output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, atcmd_output);
	}

	aFree(zeny);
	aFree(counted);

	return 0;
}


// cause random emote on all online players [Valaris]
int atcommand_happyhappyjoyjoy(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i,e;

	nullpo_retr(-1, sd);

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			e=rand()%40;
			if(e==34)
				e = 0;
			clif_emotion(&pl_sd->bl,e);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_save(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	pc_setsavepoint(sd, sd->mapname, sd->bl.x, sd->bl.y);
	if (sd->status.pet_id > 0 && sd->pd)
		intif_save_petdata(sd->status.account_id, &sd->pet);
	pc_makesavestatus(sd);
	chrif_save(sd);
	storage_storage_save(sd);
	clif_displaymessage(fd, msg_table[6]); // Character data respawn point saved.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_load(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int m;

	nullpo_retr(-1, sd);

	m = map_mapname2mapid(sd->status.save_point.map);
	if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_table[249]);
		return -1;
	}
	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, msg_table[248]);
		return -1;
	}

	pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, 0);
	clif_displaymessage(fd, msg_table[7]); // Warping to respawn point.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_speed(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int speed;

	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		sprintf(atcmd_output, "Please, enter a speed value (usage: @speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	speed = atoi(message);
	if (speed >= MIN_WALK_SPEED && speed <= MAX_WALK_SPEED) {
		sd->speed = speed;
		//sd->walktimer = x;
		//Ç±ÇÃï∂Çí«â¡ by ÇÍÇ
		clif_updatestatus(sd, SP_SPEED);
		clif_displaymessage(fd, msg_table[8]); // Speed changed.
	} else {
		sprintf(atcmd_output, "Please, enter a valid speed value (usage: @speed <%d-%d>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_storage(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct storage *stor; //changes from Freya/Yor
	nullpo_retr(-1, sd);

	if (sd->state.storage_flag == 1) {
		clif_displaymessage(fd, msg_table[250]);
		return -1;
	}

	if ((stor = account2storage2(sd->status.account_id)) != NULL && stor->storage_status == 1) {
		clif_displaymessage(fd, msg_table[250]);
		return -1;
	}

	storage_storageopen(sd);

	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int atcommand_guildstorage(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct storage *stor; //changes from Freya/Yor
	nullpo_retr(-1, sd);

	if (sd->status.guild_id > 0) {
		if (sd->state.storage_flag == 1) {
			clif_displaymessage(fd, msg_table[251]);
			return -1;
		}
		if ((stor = account2storage2(sd->status.account_id)) != NULL && stor->storage_status == 1) {
			clif_displaymessage(fd, msg_table[251]);
			return -1;
		}
		storage_guild_storageopen(sd);
	} else {
		clif_displaymessage(fd, msg_table[252]);
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_option(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int param1 = 0, param2 = 0, param3 = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d %d", &param1, &param2, &param3) < 1 || param1 < 0 || param2 < 0 || param3 < 0) {
		clif_displaymessage(fd, "Please, enter at least a option (usage: @option <param1:0+> <param2:0+> <param3:0+>).");
		return -1;
	}

	sd->opt1 = param1;
	sd->opt2 = param2;
	if (!(sd->status.option & CART_MASK) && param3 & CART_MASK) {
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd, SP_CARTINFO);
	}
	sd->status.option = param3;
	// fix pecopeco display
	if (sd->status.class_ == 13 || sd->status.class_ == 21 || sd->status.class_ == 4014 || sd->status.class_ == 4022) {
		if (!pc_isriding(sd)) { // sd have the new value...
			if (sd->status.class_ == 13)
				sd->status.class_ = sd->view_class = 7;
			else if (sd->status.class_ == 21)
				sd->status.class_ = sd->view_class = 14;
			else if (sd->status.class_ == 4014)
				sd->status.class_ = sd->view_class = 4008;
			else if (sd->status.class_ == 4022)
				sd->status.class_ = sd->view_class = 4015;
		}
	} else {
		if (pc_isriding(sd)) { // sd have the new value...
			if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris] (code added by [Yor])
				sd->status.option &= ~0x0020;
			} else {
				if (sd->status.class_ == 7)
					sd->status.class_ = sd->view_class = 13;
				else if (sd->status.class_ == 14)
					sd->status.class_ = sd->view_class = 21;
				else if (sd->status.class_ == 4008)
					sd->status.class_ = sd->view_class = 4014;
				else if (sd->status.class_ == 4015)
					sd->status.class_ = sd->view_class = 4022;
				else
					sd->status.option &= ~0x0020;
			}
		}
	}

	clif_changeoption(&sd->bl);
	status_calc_pc(sd, 0);
	clif_displaymessage(fd, msg_table[9]); // Options changed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_hide(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->status.option & OPTION_HIDE) {
		sd->status.option &= ~OPTION_HIDE;
		clif_displaymessage(fd, msg_table[10]); // Invisible: Off
	} else {
		sd->status.option |= OPTION_HIDE;
		clif_displaymessage(fd, msg_table[11]); // Invisible: On
	}
	clif_changeoption(&sd->bl);

	return 0;
}

/*==========================================
 * ì]êEÇ∑ÇÈ upperÇéwíËÇ∑ÇÈÇ∆ì]ê∂Ç‚ó{éqÇ…Ç‡Ç»ÇÍÇÈ
 *------------------------------------------
 */
int atcommand_jobchange(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int job = 0, upper = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d", &job, &upper) < 1) {

		int i, found = 0;
		const struct { char name[16]; int id; } jobs[] = {
			{ "novice",		0 },
			{ "swordsman",	1 },
			{ "mage",		2 },
			{ "archer",		3 },
			{ "acolyte",	4 },
			{ "merchant",	5 },
			{ "thief",		6 },
			{ "knight",		7 },
			{ "priest",		8 },
			{ "priestess",	8 },
			{ "wizard",		9 },
			{ "blacksmith",	10 },
			{ "hunter",		11 },
			{ "assassin",	12 },
			{ "crusader",	14 },
			{ "monk",		15 },
			{ "sage",		16 },
			{ "rogue",		17 },
			{ "alchemist",	18 },
			{ "bard",		19 },
			{ "dancer",		20 },
			{ "super novice",	23 },
			{ "supernovice",	23 },
			{ "high novice",	4001 },
			{ "swordsman high",	4002 },
			{ "mage high",		4003 },
			{ "archer high",	4004 },
			{ "acolyte high",	4005 },
			{ "merchant high",	4006 },
			{ "thief high",		4007 },
			{ "lord knight",	4008 },
			{ "high priest",	4009 },
			{ "high priestess",	4009 },
			{ "high wizard",	4010 },
			{ "whitesmith",		4011 },
			{ "sniper",		4012 },
			{ "assassin cross",	4013 },
			{ "paladin",	4015 },
			{ "champion",	4016 },
			{ "professor",	4017 },
			{ "stalker",	4018 },
			{ "creator",	4019 },
			{ "clown",		4020 },
			{ "gypsy",		4021 },
			{ "baby novice",	4023 },
			{ "baby swordsman",	4024 },
			{ "baby mage",		4025 },
			{ "baby archer",	4026 },
			{ "baby acolyte",	4027 },
			{ "baby merchant",	4028 },
			{ "baby thief",		4029 },
			{ "baby knight",	4030 },
			{ "baby priest",	4031 },
			{ "baby priestess",	4031 },
			{ "baby wizard",	4032 },
			{ "baby blacksmith",4033 },
			{ "baby hunter",	4034 },
			{ "baby assassin",	4035 },
			{ "baby crusader",	4037 },
			{ "baby monk",		4038 },
			{ "baby sage",		4039 },
			{ "baby rogue",		4040 },
			{ "baby alchemist",	4041 },
			{ "baby bard",		4042 },
			{ "baby dancer",	4043 },
			{ "super baby",		4045 },
		};

		for (i=0; i < (int)(sizeof(jobs) / sizeof(jobs[0])); i++) {
			if (strncmpi(message, jobs[i].name, 16) == 0) {
				job = jobs[i].id;
				upper = 0;
				found = 1;
				break;
			}
		}

		if (!found) {
			clif_displaymessage(fd, "Please, enter job ID (usage: @job/@jobchange <job ID>).");
			return -1;
		}
	}

	if (job == 37 ||job == 45)
		return 0;

	if ((job >= 0 && job < MAX_PC_CLASS)) {
		int j;

		// fix pecopeco display
		if ((job != 13 && job != 21 && job != 4014 && job != 4022)) {
			if (pc_isriding(sd)) {
				if (sd->status.class_ == 13)
					sd->status.class_ = sd->view_class = 7;
				if (sd->status.class_ == 21)
					sd->status.class_ = sd->view_class = 14;
				if (sd->status.class_ == 4014)
					sd->status.class_ = sd->view_class = 4008;
				if (sd->status.class_ == 4022)
					sd->status.class_ = sd->view_class = 4015;
				sd->status.option &= ~0x0020;
				clif_changeoption(&sd->bl);
				status_calc_pc(sd, 0);
			}
		} else {
			if (!pc_isriding(sd)) {
				if (job == 13)
					job = 7;
				if (job == 21)
					job = 14;
				if (job == 4014)
					job = 4008;
				if (job == 4022)
					job = 4015;
			}
		}
		for (j=0; j < MAX_INVENTORY; j++) {
			if(sd->status.inventory[j].nameid>0 && sd->status.inventory[j].equip!=0)
				pc_unequipitem(sd, j, 3);
		}
		if (pc_jobchange(sd, job, upper) == 0)
			clif_displaymessage(fd, msg_table[12]); // Your job has been changed.
		else {
			clif_displaymessage(fd, msg_table[155]); // Impossible to change your job.
			return -1;
		}
	} else {
		clif_displaymessage(fd, "Please, enter a valid job ID (usage: @job/@jobchange <job ID>).");
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_die(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,1);
	pc_damage(NULL, sd, sd->status.hp + 1);
	clif_displaymessage(fd, msg_table[13]); // A pity! You've died.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_kill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kill <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(fd, msg_table[14]); // Character killed.
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_alive(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (pc_isdead(sd)) {
	sd->status.hp = sd->status.max_hp;
	sd->status.sp = sd->status.max_sp;
	clif_skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
	pc_setstand(sd);
	if (battle_config.pc_invincible_time > 0)
		pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
	clif_updatestatus(sd, SP_HP);
	clif_updatestatus(sd, SP_SP);
	clif_resurrection(&sd->bl, 1);
	clif_displaymessage(fd, msg_table[16]); // You've been revived! It's a miracle!
	return 0;
	}
	return -1;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_kami(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @kami <message>).");
		return -1;
	}

	sscanf(message, "%199[^\n]", atcmd_output);
	intif_GMmessage(atcmd_output, strlen(atcmd_output) + 1, (*(command + 5) == 'b') ? 0x10 : 0);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_heal(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hp = 0, sp = 0; // [Valaris] thanks to fov
	nullpo_retr(-1, sd);

	sscanf(message, "%d %d", &hp, &sp);

	if (hp == 0 && sp == 0) {
		hp = sd->status.max_hp - sd->status.hp;
		sp = sd->status.max_sp - sd->status.sp;
	} else {
		if (hp > 0 && (hp > sd->status.max_hp || hp > (sd->status.max_hp - sd->status.hp))) // fix positiv overflow
			hp = sd->status.max_hp - sd->status.hp;
		else if (hp < 0 && (hp < -sd->status.max_hp || hp < (1 - sd->status.hp))) // fix negativ overflow
			hp = 1 - sd->status.hp;
		if (sp > 0 && (sp > sd->status.max_sp || sp > (sd->status.max_sp - sd->status.sp))) // fix positiv overflow
			sp = sd->status.max_sp - sd->status.sp;
		else if (sp < 0 && (sp < -sd->status.max_sp || sp < (1 - sd->status.sp))) // fix negativ overflow
			sp = 1 - sd->status.sp;
	}

	if (hp > 0) // display like heal
		clif_heal(fd, SP_HP, hp);
	else if (hp < 0) // display like damage
		clif_damage(&sd->bl,&sd->bl, gettick(), 0, 0, -hp, 0 , 4, 0);
	if (sp > 0) // no display when we lost SP
		clif_heal(fd, SP_SP, sp);

	if (hp != 0 || sp != 0) {
		pc_heal(sd, hp, sp);
		if (hp >= 0 && sp >= 0)
			clif_displaymessage(fd, msg_table[17]); // HP, SP recovered.
		else
			clif_displaymessage(fd, msg_table[156]); // HP or/and SP modified.
	} else {
		clif_displaymessage(fd, msg_table[157]); // HP and SP are already with the good value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @item command (usage: @item <name/id_of_item> <quantity>) (modified by [Yor] for pet_egg)
 *------------------------------------------
 */
int atcommand_item(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d", item_name, &number) < 1) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: @item <item name or ID> [quantity]).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id >= 500) {
		get_count = number;
		// check pet egg
		pet_id = search_petDB_index(item_id, PET_EGG);
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			get_count = 1;
		}
		for (i = 0; i < number; i += get_count) {
			// if pet egg
			if (pet_id >= 0) {
				sd->catch_target_class = pet_db[pet_id].class_;
				intif_create_pet(sd->status.account_id, sd->status.char_id,
				                 (short)pet_db[pet_id].class_, (short)mob_db[pet_db[pet_id].class_].lv,
				                 (short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
				                 100, 0, 1, pet_db[pet_id].jname);
			// if not pet egg
			} else {
				memset(&item_tmp, 0, sizeof(item_tmp));
				item_tmp.nameid = item_id;
				item_tmp.identify = 1;
				if ((flag = pc_additem((struct map_session_data*)sd, &item_tmp, get_count)))
					clif_additem((struct map_session_data*)sd, 0, 0, flag);
			}
		}
		clif_displaymessage(fd, msg_table[18]); // Item created.
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_item2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag;
	int loop, get_count, i;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d %d %d %d %d %d %d %d", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4) < 9) {
		clif_displaymessage(fd, "Please, enter all informations (usage: @item2 <item name or ID> <quantity>");
		clif_displaymessage(fd, "  <Identify_flag> <refine> <attribut> <Card1> <Card2> <Card3> <Card4>).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		loop = 1;
		get_count = number;
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
			if (item_data->type == 7) {
				identify = 1;
				refine = 0;
			}
			if (item_data->type == 8)
				refine = 0;
			if (refine > 10)
				refine = 10;
		} else {
			identify = 1;
			refine = attr = 0;
		}
		for (i = 0; i < loop; i++) {
			memset(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_id;
			item_tmp.identify = identify;
			item_tmp.refine = refine;
			item_tmp.attribute = attr;
			item_tmp.card[0] = c1;
			item_tmp.card[1] = c2;
			item_tmp.card[2] = c3;
			item_tmp.card[3] = c4;
			if ((flag = pc_additem(sd, &item_tmp, get_count)))
				clif_additem(sd, 0, 0, flag);
		}
		clif_displaymessage(fd, msg_table[18]); // Item created.
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_itemreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);

	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount && sd->status.inventory[i].equip == 0)
			pc_delitem(sd, i, sd->status.inventory[i].amount, 0);
	}
	clif_displaymessage(fd, msg_table[20]); // All of your items have been removed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_itemcheck(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	pc_checkitem(sd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_baselevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int level, i;
	nullpo_retr(-1, sd);

	if (!message || !*message || (level = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: @lvup/@blevel/@baselvlup <number of levels>).");
		return -1;
	}

	if (level > 0) {
		if (sd->status.base_level == battle_config.maximum_level) {	// check for max level by Valaris
			clif_displaymessage(fd, msg_table[47]); // Base level can't go any higher.
			return -1;
		}	// End Addition
		if ((unsigned int)level > battle_config.maximum_level || (unsigned int)level > (battle_config.maximum_level - sd->status.base_level)) // fix positiv overflow
			level = battle_config.maximum_level - sd->status.base_level;
		for (i = 1; i <= level; i++)
			sd->status.status_point += (sd->status.base_level + i + 14) / 5;
		sd->status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		clif_updatestatus(sd, SP_STATUSPOINT);
		status_calc_pc(sd, 0);
		pc_heal(sd, sd->status.max_hp, sd->status.max_sp);
		clif_misceffect(&sd->bl, 0);
		clif_displaymessage(fd, msg_table[21]); // Base level raised.
	} else {
		if (sd->status.base_level == 1) {
			clif_displaymessage(fd, msg_table[158]); // Base level can't go any lower.
			return -1;
		}
		if (level < -(int)battle_config.maximum_level || level < (1 - (int)sd->status.base_level)) // fix negativ overflow
			level = 1 - sd->status.base_level;
		if (sd->status.status_point > 0) {
			for (i = 0; i > level; i--)
				sd->status.status_point -= (sd->status.base_level + i + 14) / 5;
			if (sd->status.status_point < 0)
				sd->status.status_point = 0;
			clif_updatestatus(sd, SP_STATUSPOINT);
		} // to add: remove status points from stats
		sd->status.base_level += level;
		clif_updatestatus(sd, SP_BASELEVEL);
		clif_updatestatus(sd, SP_NEXTBASEEXP);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_table[22]); // Base level lowered.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_joblevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	unsigned int up_level = 50;
	int level;
	struct pc_base_job s_class;
	nullpo_retr(-1, sd);
	s_class = pc_calc_base_job(sd->status.class_);

	if (!message || !*message || (level = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement (usage: @joblvup/@jlevel/@joblvlup <number of levels>).");
		return -1;
	}

	if (s_class.job == 0)
		up_level -= 40;
	// super novices can go up to 99 [celest]
	else if (s_class.job == 23)
		up_level += 49;
	else if (sd->status.class_ > 4007 && sd->status.class_ < 4023)
		up_level += 20;

	if (level > 0) {
		if (sd->status.job_level == up_level) {
			clif_displaymessage(fd, msg_table[23]); // Job level can't go any higher.
			return -1;
		}
		if ((unsigned int)level > up_level || (unsigned int)level > (up_level - sd->status.job_level)) // fix positiv overflow
			level = up_level - sd->status.job_level;
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		sd->status.skill_point += level;
		clif_updatestatus(sd, SP_SKILLPOINT);
		status_calc_pc(sd, 0);
		clif_misceffect(&sd->bl, 1);
		clif_displaymessage(fd, msg_table[24]); // Job level raised.
	} else {
		if (sd->status.job_level == 1) {
			clif_displaymessage(fd, msg_table[159]); // Job level can't go any lower.
			return -1;
		}
		if (level < -(int)up_level || level < (1 - (int)sd->status.job_level)) // fix negativ overflow
			level = 1 - sd->status.job_level;
		sd->status.job_level += level;
		clif_updatestatus(sd, SP_JOBLEVEL);
		clif_updatestatus(sd, SP_NEXTJOBEXP);
		if (sd->status.skill_point > 0) {
			sd->status.skill_point += level;
			if (sd->status.skill_point < 0)
				sd->status.skill_point = 0;
			clif_updatestatus(sd, SP_SKILLPOINT);
		} // to add: remove status points from skills
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_table[25]); // Job level lowered.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_help(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;
	nullpo_retr(-1, sd);

	memset(buf, '\0', sizeof(buf));

	if ((fp = fopen(help_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_table[26]); // Help commands:
		gm_level = pc_isGM(sd);
		while(fgets(buf, sizeof(buf) - 1, fp) != NULL) {
			if (buf[0] == '/' && buf[1] == '/')
				continue;
			for (i = 0; buf[i] != '\0'; i++) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					buf[i] = '\0';
					break;
				}
			}
			if (sscanf(buf, "%2047[^:]:%2047[^\n]", w1, w2) < 2)
				clif_displaymessage(fd, buf);
			else if (gm_level >= atoi(w1))
				clif_displaymessage(fd, w2);
		}
		fclose(fp);
	} else {
		clif_displaymessage(fd, msg_table[27]); // File help.txt not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_gm(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char password[100];
	nullpo_retr(-1, sd);

	memset(password, '\0', sizeof(password));

	if (!message || !*message || sscanf(message, "%99[^\n]", password) < 1) {
		clif_displaymessage(fd, "Please, enter a password (usage: @gm <password>).");
		return -1;
	}

	if (pc_isGM(sd)) { // a GM can not use this function. only a normal player (become gm is not for gm!)
		clif_displaymessage(fd, msg_table[50]); // You already have some GM powers.
		return -1;
	} else
		chrif_changegm(sd->status.account_id, password, strlen(password) + 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_pvpoff(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	if (battle_config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_table[52]); // This option cannot be used in PK Mode.
		return -1;
	}

	if (map[sd->bl.m].flag.pvp) {
		map[sd->bl.m].flag.pvp = 0;
		clif_send0199(sd->bl.m, 0);
		for (i = 0; i < fd_max; i++) {	//êlêîï™ÉãÅ[Év
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m) {
					clif_pvpset(pl_sd, 0, 0, 2);
					if (pl_sd->pvp_timer != -1) {
						delete_timer(pl_sd->pvp_timer, pc_calc_pvprank_timer);
						pl_sd->pvp_timer = -1;
					}
				}
			}
		}
		clif_displaymessage(fd, msg_table[31]); // PvP: Off.
	} else {
		clif_displaymessage(fd, msg_table[160]); // PvP is already Off.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_pvpon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	if (battle_config.pk_mode) { //disable command if server is in PK mode [Valaris]
		clif_displaymessage(fd, msg_table[52]); // This option cannot be used in PK Mode.
		return -1;
	}

	if (!map[sd->bl.m].flag.pvp && !map[sd->bl.m].flag.nopvp) {
		map[sd->bl.m].flag.pvp = 1;
		clif_send0199(sd->bl.m, 1);
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				if (sd->bl.m == pl_sd->bl.m && pl_sd->pvp_timer == -1) {
					pl_sd->pvp_timer = add_timer(gettick() + 200,
						pc_calc_pvprank_timer, pl_sd->bl.id, 0);
					pl_sd->pvp_rank = 0;
					pl_sd->pvp_lastusers = 0;
					pl_sd->pvp_point = 5;
				}
			}
		}
		clif_displaymessage(fd, msg_table[32]); // PvP: On.
	} else {
		clif_displaymessage(fd, msg_table[161]); // PvP is already On.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_gvgoff(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.gvg) {
		map[sd->bl.m].flag.gvg = 0;
		clif_send0199(sd->bl.m, 0);
		clif_displaymessage(fd, msg_table[33]); // GvG: Off.
	} else {
		clif_displaymessage(fd, msg_table[162]); // GvG is already Off.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_gvgon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (!map[sd->bl.m].flag.gvg) {
		map[sd->bl.m].flag.gvg = 1;
		clif_send0199(sd->bl.m, 3);
		clif_displaymessage(fd, msg_table[34]); // GvG: On.
	} else {
		clif_displaymessage(fd, msg_table[163]); // GvG is already On.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_model(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d %d %d", &hair_style, &hair_color, &cloth_color) < 1) {
		sprintf(atcmd_output, "Please, enter at least a value (usage: @model <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d>).",
		        MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		//ïûÇÃêFïœçX
		if (cloth_color != 0 && sd->status.sex == 1 && (sd->status.class_ == 12 ||  sd->status.class_ == 17)) {
			//ïûÇÃêFñ¢é¿ëïêEÇÃîªíË
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class.
			return -1;
		} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @dye && @ccolor
 *------------------------------------------
 */
int atcommand_dye(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int cloth_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &cloth_color) < 1) {
		sprintf(atcmd_output, "Please, enter a clothes color (usage: @dye/@ccolor <clothes color: %d-%d>).", MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc_changelook(sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(fd, msg_table[36]); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @hairstyle && @hstyle
 *------------------------------------------
 */
int atcommand_hair_style(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_style = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &hair_style) < 1) {
		sprintf(atcmd_output, "Please, enter a hair style (usage: @hairstyle/@hstyle <hair ID: %d-%d>).", MIN_HAIR_STYLE, MAX_HAIR_STYLE);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
		if (hair_style != 0 && sd->status.sex == 1 && (sd->status.class_ == 12 || sd->status.class_ == 17)) {
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class.
			return -1;
		} else {
			pc_changelook(sd, LOOK_HAIR, hair_style);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charhairstyle by [MouseJstr]
 *------------------------------------------
 */
int
atcommand_charhairstyle(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
    return 0;
}

/*==========================================
 * @haircolor && @hcolor
 *------------------------------------------
 */
int atcommand_hair_color(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_color = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &hair_color) < 1) {
		sprintf(atcmd_output, "Please, enter a hair color (usage: @haircolor/@hcolor <hair color: %d-%d>).", MIN_HAIR_COLOR, MAX_HAIR_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
		if (hair_color != 0 && sd->status.sex == 1 && (sd->status.class_ == 12 || sd->status.class_ == 17)) {
			clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class.
			return -1;
		} else {
			pc_changelook(sd, LOOK_HAIR_COLOR, hair_color);
			clif_displaymessage(fd, msg_table[36]); // Appearence changed.
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * @go [city_number/city_name]: improved by [yor] to add city names and help
 *------------------------------------------
 */
int atcommand_go(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	int town;
	char map_name[100];
	int m;

	const struct { char map[16]; int x,   y; } data[] = {
	       { "prontera.gat", 156, 191  },	//	 0=Prontera
	       { "morocc.gat",   156,  93  },	//	 1=Morroc
	       { "geffen.gat",   119,  59  },	//	 2=Geffen
	       { "payon.gat",    162, 233  },	//	 3=Payon
	       { "alberta.gat",  192, 147  },	//	 4=Alberta
	       { "izlude.gat",   128, 114  },	//	 5=Izlude
	       { "aldebaran.gat",140, 131  },	//	 6=Al de Baran
	       { "xmas.gat",     147, 134  },	//	 7=Lutie
	       { "comodo.gat",   209, 143  },	//	 8=Comodo
	       { "yuno.gat",     157,  51  },	//	 9=Yuno
	       { "amatsu.gat",   198,  84  },	//	10=Amatsu
	       { "gonryun.gat",  160, 120  },	//	11=Gon Ryun
	       { "umbala.gat",    89, 157  },	//	12=Umbala
	       { "niflheim.gat",  21, 153  },	//	13=Niflheim
	       { "louyang.gat",  217,  40  },	//	14=Lou Yang
	       { "new_1-1.gat",   53, 111  },	//	15=Start point
	       { "sec_pri.gat",   23,  61  },	//	16=Prison
           { "jawaii.gat",   249, 127  },	//  17=Jawaii
		   { "ayothaya.gat", 151, 117  },	//  18=Ayothaya
	};

	nullpo_retr(-1, sd);

	if(map[sd->bl.m].flag.nogo) {
		clif_displaymessage(sd->fd,"You can not use @go on this map.");
		return 0;
	}

	memset(map_name, '\0', sizeof(map_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	// get the number
	town = atoi(message);

	// if no value, display all value
	if (!message || !*message || sscanf(message, "%99s", map_name) < 1 || town < -3 || town >= (int)(sizeof(data) / sizeof(data[0]))) {
		clif_displaymessage(fd, msg_table[38]); // Invalid location number or name.
		clif_displaymessage(fd, msg_table[82]); // Please, use one of this number/name:
		clif_displaymessage(fd, "-3=(Memo point 2)   4=Alberta       11=Gon Ryun");
		clif_displaymessage(fd, "-2=(Memo point 1)   5=Izlude        12=Umbala");
		clif_displaymessage(fd, "-1=(Memo point 0)   6=Al de Baran   13=Niflheim");
		clif_displaymessage(fd, " 0=Prontera         7=Lutie         14=Lou Yang");
		clif_displaymessage(fd, " 1=Morroc           8=Comodo        15=Start point");
		clif_displaymessage(fd, " 2=Geffen           9=Yuno          16=Prison");
		clif_displaymessage(fd, " 3=Payon           10=Amatsu        17=Jawaii");
		clif_displaymessage(fd, "                                    18=Ayothaya");
		return -1;
	} else {
		// get possible name of the city and add .gat if not in the name
		map_name[sizeof(map_name)-1] = '\0';
		for (i = 0; map_name[i]; i++)
			map_name[i] = tolower(map_name[i]);
		if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
			strcat(map_name, ".gat");
		// try to see if it's a name, and not a number (try a lot of possibilities, write errors and abbreviations too)
		if (strncmp(map_name, "prontera.gat", 3) == 0) { // 3 first characters
			town = 0;
		} else if (strncmp(map_name, "morocc.gat", 3) == 0) { // 3 first characters
			town = 1;
		} else if (strncmp(map_name, "geffen.gat", 3) == 0) { // 3 first characters
			town = 2;
		} else if (strncmp(map_name, "payon.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "paion.gat", 3) == 0) { // writing error (3 first characters)
			town = 3;
		} else if (strncmp(map_name, "alberta.gat", 3) == 0) { // 3 first characters
			town = 4;
		} else if (strncmp(map_name, "izlude.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "islude.gat", 3) == 0) { // writing error (3 first characters)
			town = 5;
		} else if (strncmp(map_name, "aldebaran.gat", 3) == 0 || // 3 first characters
		           strcmp(map_name,  "al.gat") == 0) { // al (de baran)
			town = 6;
		} else if (strncmp(map_name, "lutie.gat", 3) == 0 || // name of the city, not name of the map (3 first characters)
		           strcmp(map_name,  "christmas.gat") == 0 || // name of the symbol
		           strncmp(map_name, "xmas.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "x-mas.gat", 3) == 0) { // writing error (3 first characters)
			town = 7;
		} else if (strncmp(map_name, "comodo.gat", 3) == 0) { // 3 first characters
			town = 8;
		} else if (strncmp(map_name, "yuno.gat", 3) == 0) { // 3 first characters
			town = 9;
		} else if (strncmp(map_name, "amatsu.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "ammatsu.gat", 3) == 0) { // writing error (3 first characters)
			town = 10;
		} else if (strncmp(map_name, "gonryun.gat", 3) == 0) { // 3 first characters
			town = 11;
		} else if (strncmp(map_name, "umbala.gat", 3) == 0) { // 3 first characters
			town = 12;
		} else if (strncmp(map_name, "niflheim.gat", 3) == 0) { // 3 first characters
			town = 13;
		} else if (strncmp(map_name, "louyang.gat", 3) == 0) { // 3 first characters
			town = 14;
		} else if (strncmp(map_name, "new_1-1.gat", 3) == 0 || // 3 first characters (or "newbies")
		           strncmp(map_name, "startpoint.gat", 3) == 0 || // name of the position (3 first characters)
		           strncmp(map_name, "begining.gat", 3) == 0) { // name of the position (3 first characters)
			town = 15;
		} else if (strncmp(map_name, "sec_pri.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "prison.gat", 3) == 0 || // name of the position (3 first characters)
		           strncmp(map_name, "jails.gat", 3) == 0) { // name of the position
			town = 16;
		} else if (strncmp(map_name, "jawaii.gat", 3) == 0 || // 3 first characters
		           strncmp(map_name, "jawai.gat", 3) == 0) { // writing error (3 first characters)
			town = 17;
		} else if (strncmp(map_name, "ayothaya.gat", 4) == 0 || // 3 first characters
		           strncmp(map_name, "ayotaya.gat", 4) == 0) { // writing error (3 first characters)
			town = 18;
		}

		if (town >= -3 && town <= -1) {
			if (sd->status.memo_point[-town-1].map[0]) {
				m = map_mapname2mapid(sd->status.memo_point[-town-1].map);
				if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, msg_table[247]);
					return -1;
				}
				if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, msg_table[248]);
					return -1;
				}
				if (pc_setpos(sd, sd->status.memo_point[-town-1].map, sd->status.memo_point[-town-1].x, sd->status.memo_point[-town-1].y, 3) == 0) {
					clif_displaymessage(fd, msg_table[0]); // Warped.
				} else {
					clif_displaymessage(fd, msg_table[1]); // Map not found.
					return -1;
				}
			} else {
				sprintf(atcmd_output, msg_table[164], -town-1); // Your memo point #%d doesn't exist.
				clif_displaymessage(fd, atcmd_output);
				return -1;
			}
		} else if (town >= 0 && town < (int)(sizeof(data) / sizeof(data[0]))) {
			m = map_mapname2mapid((char *)data[town].map);
			if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, msg_table[247]);
				return -1;
			}
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, msg_table[248]);
				return -1;
			}
			if (pc_setpos(sd, (char *)data[town].map, data[town].x, data[town].y, 3) == 0) {
				clif_displaymessage(fd, msg_table[0]); // Warped.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return -1;
			}
		} else { // if you arrive here, you have an error in town variable when reading of names
			clif_displaymessage(fd, msg_table[38]); // Invalid location number or name.
			return -1;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_monster(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int x = 0, y = 0;
	int count;
	int i, j, k;
	int mx, my, range;
	nullpo_retr(-1, sd);

	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s %99s %d %d %d", name, monster, &number, &x, &y) < 2)) {
		clif_displaymessage(fd, msg_table[80]); // Give a display name and monster name/id please.
		return -1;
	}

	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_table[83]); // Cannot spawn emperium.
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.etc_log)
		printf("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, x, y);

	count = 0;
	range = (int)sqrt(number) / 2;
	range = range * 2 + 5; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		j = 0;
		k = 0;
		while(j++ < 8 && k == 0) { // try 8 times to spawn the monster (needed for close area)
			if (x <= 0)
				mx = sd->bl.x + (rand() % range - (range / 2));
			else
				mx = x;
			if (y <= 0)
				my = sd->bl.y + (rand() % range - (range / 2));
			else
				my = y;
			k = mob_once_spawn((struct map_session_data*)sd, "this", mx, my, name, mob_id, 1, "");
		}
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
		if (number == count)
			clif_displaymessage(fd, msg_table[39]); // All monster summoned!
		else {
			sprintf(atcmd_output, msg_table[240], count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
	else {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_spawn(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message) {
	char name[100];
	char monster[100];
	int mob_id;
	int number = 0;
	int x = 0, y = 0;
	int count;
	int i, j, k;
	int mx, my, range;

	nullpo_retr(-1, sd);
	memset(name, '\0', sizeof(name));
	memset(monster, '\0', sizeof(monster));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message ||
	    (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	     sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1)) {
		clif_displaymessage(fd, msg_table[143]); // Give a monster name/id please.
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(monster));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return -1;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, msg_table[83]); // Cannot spawn emperium.
		return -1;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	if (battle_config.etc_log)
		printf("%s monster='%s' name='%s' id=%d count=%d (%d,%d)\n", command, monster, name, mob_id, number, x, y);

	count = 0;
	range = (int)sqrt(number) / 2;
	range = range * 2 + 5; // calculation of an odd number (+ 4 area around)
	for (i = 0; i < number; i++) {
		j = 0;
		k = 0;
		while(j++ < 8 && k == 0) { // try 8 times to spawn the monster (needed for close area)
			if (x <= 0)
				mx = sd->bl.x + (rand() % range - (range / 2));
			else
				mx = x;
			if (y <= 0)
				my = sd->bl.y + (rand() % range - (range / 2));
			else
				my = y;
			k = mob_once_spawn((struct map_session_data*)sd, "this", mx, my, name, mob_id, 1, "");
		}
		count += (k != 0) ? 1 : 0;
	}

	if (count != 0)
		if (number == count)
			clif_displaymessage(fd, msg_table[39]); // All monster summoned!
		else {
			sprintf(atcmd_output, msg_table[240], count); // %d monster(s) summoned!
			clif_displaymessage(fd, atcmd_output);
		}
	else {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return -1;
	}

	return 0;
}
// small monster spawning [Valaris]
int atcommand_monstersmall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message) {
	char name[100] = "";
	char monster[100] = "";
	int mob_id = 0;
	int number = 0;
	int x = 0;
	int y = 0;
	int count;
	int i;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return 0;
	}

	if (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return 0;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, "Monster name hasn't been found.");
		return 0;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, "Cannot spawn emperium.");
		return 0;
	}

	if (mob_id > 2000) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return 0;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; i++) {
		int mx, my;
		if (x <= 0)
			mx = sd->bl.x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd->bl.y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn((struct map_session_data*)sd, "this", mx, my, name, mob_id+2000, 1, "") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_table[39]); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_table[40]); // Invalid Monster ID.

	return 0;
}
// big monster spawning [Valaris]
int atcommand_monsterbig(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message) {
	char name[100] = "";
	char monster[100] = "";
	int mob_id = 0;
	int number = 0;
	int x = 0;
	int y = 0;
	int count;
	int i;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return 0;
	}

	if (sscanf(message, "\"%[^\"]\" %99s %d %d %d", name, monster, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s \"%[^\"]\" %d %d %d", monster, name, &number, &x, &y) < 2 &&
	    sscanf(message, "%99s %d %99s %d %d", monster, &number, name, &x, &y) < 1) {
		clif_displaymessage(fd, "Give a monster name/id please.");
		return 0;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(monster)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(monster);

	if (mob_id == 0) {
		clif_displaymessage(fd, "Monster name hasn't been found.");
		return 0;
	}

	if (mob_id == 1288) {
		clif_displaymessage(fd, "Cannot spawn emperium.");
		return 0;
	}

	if (mob_id > 2000) {
		clif_displaymessage(fd, "Invalid monster ID"); // Invalid Monster ID.
		return 0;
	}

	if (number <= 0)
		number = 1;

	if (strlen(name) < 1)
		strcpy(name, "--ja--");

	// If value of atcommand_spawn_quantity_limit directive is greater than or equal to 1 and quantity of monsters is greater than value of the directive
	if (battle_config.atc_spawn_quantity_limit >= 1 && number > battle_config.atc_spawn_quantity_limit)
		number = battle_config.atc_spawn_quantity_limit;

	count = 0;
	for (i = 0; i < number; i++) {
		int mx, my;
		if (x <= 0)
			mx = sd->bl.x + (rand() % 11 - 5);
		else
			mx = x;
		if (y <= 0)
			my = sd->bl.y + (rand() % 11 - 5);
		else
			my = y;
		count += (mob_once_spawn((struct map_session_data*)sd, "this", mx, my, name, mob_id+4000, 1, "") != 0) ? 1 : 0;
	}

	if (count != 0)
		clif_displaymessage(fd, msg_table[39]); // Monster Summoned!!
	else
		clif_displaymessage(fd, msg_table[40]); // Invalid Monster ID.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void atcommand_killmonster_sub(
	const int fd, struct map_session_data* sd, const char* message,
	const int drop)
{
	int map_id;
	char map_name[100];

	if (!sd) return;

	memset(map_name, '\0', sizeof(map_name));

	if (!message || !*message || sscanf(message, "%99s", map_name) < 1)
		map_id = sd->bl.m;
	else {
		if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < 13) // 16 - 4 (.gat)
			strcat(map_name, ".gat");
		if ((map_id = map_mapname2mapid(map_name)) < 0)
			map_id = sd->bl.m;
	}

	map_foreachinarea(atkillmonster_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_MOB, drop);

	clif_displaymessage(fd, msg_table[165]); // All monsters killed!

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_killmonster(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (!sd) return 0;
	atcommand_killmonster_sub(fd, sd, message, 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_killmonster2(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if (!sd) return 0;
	atcommand_killmonster_sub(fd, sd, message, 0);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_refine(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i, position = 0, refine = 0, current_position, final_refine;
	int count;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d %d", &position, &refine) < 2) {
		clif_displaymessage(fd, "Please, enter a position and a amount (usage: @refine <equip position> <+/- amount>).");
		return -1;
	}

	if (refine < -10)
		refine = -10;
	else if (refine > 10)
		refine = 10;
	else if (refine == 0)
		refine = 1;

	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid &&	// äYìñå¬èäÇÃëïîıÇê∏òBÇ∑ÇÈ
		    (sd->status.inventory[i].equip & position ||
			(sd->status.inventory[i].equip && !position))) {
			final_refine = sd->status.inventory[i].refine + refine;
			if (final_refine > 10)
				final_refine = 10;
			else if (final_refine < 0)
				final_refine = 0;
			if (sd->status.inventory[i].refine != final_refine) {
				sd->status.inventory[i].refine = final_refine;
				current_position = sd->status.inventory[i].equip;
				pc_unequipitem(sd, i, 3);
				clif_refine(fd, sd, 0, i, sd->status.inventory[i].refine);
				clif_delitem(sd, i, 1);
				clif_additem(sd, i, 1, 0);
				pc_equipitem(sd, i, current_position);
				clif_misceffect((struct block_list*)&sd->bl, 3);
				count++;
			}
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[166]); // No item has been refined!
	else if (count == 1)
		clif_displaymessage(fd, msg_table[167]); // 1 item has been refined!
	else {
		sprintf(atcmd_output, msg_table[168], count); // %d items have been refined!
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_produce(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	int item_id, attribute = 0, star = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d %d", item_name, &attribute, &star) < 1) {
		clif_displaymessage(fd, "Please, enter at least an item name/id (usage: @produce <equip name or equip ID> <element> <# of very's>).");
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (itemdb_exists(item_id) &&
	    (item_id <= 500 || item_id > 1099) &&
	    (item_id < 4001 || item_id > 4148) &&
	    (item_id < 7001 || item_id > 10019) &&
	    itemdb_isequip(item_id)) {
		if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
			attribute = ATTRIBUTE_NORMAL;
		if (star < MIN_STAR || star > MAX_STAR)
			star = 0;
		memset(&tmp_item, 0, sizeof tmp_item);
		tmp_item.nameid = item_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		tmp_item.card[0] = 0x00ff;
		tmp_item.card[1] = ((star * 5) << 8) + attribute;
		*((unsigned long *)(&tmp_item.card[2])) = sd->char_id;
		clif_produceeffect(sd, 0, item_id); // êªë¢ÉGÉtÉFÉNÉgÉpÉPÉbÉg
		clif_misceffect(&sd->bl, 3); // ëºêlÇ…Ç‡ê¨å˜Çí ím
		if ((flag = pc_additem(sd, &tmp_item, 1)))
			clif_additem(sd, 0, 0, flag);
	} else {
		if (battle_config.error_log)
			printf("@produce NOT WEAPON [%d]\n", item_id);
		if (item_id != 0 && itemdb_exists(item_id))
			sprintf(atcmd_output, msg_table[169], item_id, item_data->name); // This item (%d: '%s') is not an equipment.
		else
			sprintf(atcmd_output, msg_table[170]); // This item is not an equipment.
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	return 0;
}

/*==========================================
 * Sub-function to display actual memo points
 *------------------------------------------
 */
void atcommand_memo_sub(struct map_session_data* sd) {
	int i;

	if (!sd) return;

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	clif_displaymessage(sd->fd,  "Your actual memo positions are (except respawn point):");
	for (i = MIN_PORTAL_MEMO; i <= MAX_PORTAL_MEMO; i++) {
		if (sd->status.memo_point[i].map[0])
			sprintf(atcmd_output, "%d - %s (%d,%d)", i, sd->status.memo_point[i].map, sd->status.memo_point[i].x, sd->status.memo_point[i].y);
		else
			sprintf(atcmd_output, msg_table[171], i); // %d - void
		clif_displaymessage(sd->fd, atcmd_output);
	}

	return;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_memo(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int position = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &position) < 1)
		atcommand_memo_sub(sd);
	else {
		if (position >= MIN_PORTAL_MEMO && position <= MAX_PORTAL_MEMO) {
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, msg_table[253]);
				return -1;
			}
			if (sd->status.memo_point[position].map[0]) {
				sprintf(atcmd_output, msg_table[172], position, sd->status.memo_point[position].map, sd->status.memo_point[position].x, sd->status.memo_point[position].y); // You replace previous memo position %d - %s (%d,%d).
				clif_displaymessage(fd, atcmd_output);
			}
			memcpy(sd->status.memo_point[position].map, map[sd->bl.m].name, 24);
			sd->status.memo_point[position].x = sd->bl.x;
			sd->status.memo_point[position].y = sd->bl.y;
			clif_skill_memo(sd, 0);
			if (pc_checkskill(sd, AL_WARP) <= (position + 1))
				clif_displaymessage(fd, msg_table[173]); // Note: you don't have the 'Warp' skill level to use it.
			atcommand_memo_sub(sd);
		} else {
			sprintf(atcmd_output, "Please, enter a valid position (usage: @memo <memo_position:%d-%d>).", MIN_PORTAL_MEMO, MAX_PORTAL_MEMO);
			clif_displaymessage(fd, atcmd_output);
			atcommand_memo_sub(sd);
			return -1;
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_gat(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int y;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	for (y = 2; y >= -2; y--) {
		sprintf(atcmd_output, "%s (x= %d, y= %d) %02X %02X %02X %02X %02X",
			map[sd->bl.m].name,   sd->bl.x - 2, sd->bl.y + y,
 			map_getcell(sd->bl.m, sd->bl.x - 2, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x - 1, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x,     sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x + 1, sd->bl.y + y, CELL_GETTYPE),
 			map_getcell(sd->bl.m, sd->bl.x + 2, sd->bl.y + y, CELL_GETTYPE));

		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_packet(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int x = 0, y = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d", &x, &y) < 2) {
		clif_displaymessage(fd, "Please, enter a status type/flag (usage: @packet <status type> <flag>).");
		return -1;
	}

	clif_status_change(&sd->bl, x, y);

	return 0;
}

/*==========================================
 * @stpoint (Rewritten by [Yor])
 *------------------------------------------
 */
int atcommand_statuspoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int point, new_status_point;

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @stpoint <number of points>).");
		return -1;
	}

	new_status_point = (int)sd->status.status_point + point;
	if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
		new_status_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
		new_status_point = 0;

	if (new_status_point != (int)sd->status.status_point) {
		sd->status.status_point = (short)new_status_point;
		clif_updatestatus(sd, SP_STATUSPOINT);
		clif_displaymessage(fd, msg_table[174]); // Number of status points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @skpoint (Rewritten by [Yor])
 *------------------------------------------
 */
int atcommand_skillpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int point, new_skill_point;
	nullpo_retr(-1, sd);

	if (!message || !*message || (point = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter a number (usage: @skpoint <number of points>).");
		return -1;
	}

	new_skill_point = (int)sd->status.skill_point + point;
	if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
		new_skill_point = 0x7FFF;
	else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
		new_skill_point = 0;

	if (new_skill_point != (int)sd->status.skill_point) {
		sd->status.skill_point = (short)new_skill_point;
		clif_updatestatus(sd, SP_SKILLPOINT);
		clif_displaymessage(fd, msg_table[175]); // Number of skill points changed!
	} else {
		if (point < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * @zeny (Rewritten by [Yor])
 *------------------------------------------
 */
int atcommand_zeny(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int zeny, new_zeny;
	nullpo_retr(-1, sd);

	if (!message || !*message || (zeny = atoi(message)) == 0) {
		clif_displaymessage(fd, "Please, enter an amount (usage: @zeny <amount>).");
		return -1;
	}

	new_zeny = sd->status.zeny + zeny;
	if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
		new_zeny = MAX_ZENY;
	else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
		new_zeny = 0;

	if (new_zeny != sd->status.zeny) {
		sd->status.zeny = new_zeny;
		clif_updatestatus(sd, SP_ZENY);
		clif_displaymessage(fd, msg_table[176]); // Number of zenys changed!
	} else {
		if (zeny < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_param(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i, index, value = 0, new_value;
	const char* param[] = { "@str", "@agi", "@vit", "@int", "@dex", "@luk", NULL };
	short* status[] = {
		&sd->status.str,  &sd->status.agi, &sd->status.vit,
		&sd->status.int_, &sd->status.dex, &sd->status.luk
	};
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0) {
		sprintf(atcmd_output, "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustement>).");
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	index = -1;
	for (i = 0; param[i] != NULL; i++) {
		if (strcmpi(command, param[i]) == 0) {
			index = i;
			break;
		}
	}
	if (index < 0 || index > MAX_STATUS_TYPE) { // normaly impossible...
		sprintf(atcmd_output, "Please, enter a valid value (usage: @str,@agi,@vit,@int,@dex,@luk <+/-adjustement>).");
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	new_value = (int)*status[index] + value;
	if (value > 0 && ((unsigned int)value > battle_config.max_parameter || (unsigned int)new_value > battle_config.max_parameter)) // fix positiv overflow
		new_value = battle_config.max_parameter;
	else if (value < 0 && (value < -(int)battle_config.max_parameter || new_value < 1)) // fix negativ overflow
		new_value = 1;

	if (new_value != (int)*status[index]) {
		*status[index] = new_value;
		clif_updatestatus(sd, SP_STR + index);
		clif_updatestatus(sd, SP_USTR + index);
		status_calc_pc(sd, 0);
		clif_displaymessage(fd, msg_table[42]); // Stat changed.
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
//** Stat all by fritz (rewritten by [Yor])
int atcommand_stat_all(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int index, count, value = 0, new_value;
	short* status[] = {
		&sd->status.str,  &sd->status.agi, &sd->status.vit,
		&sd->status.int_, &sd->status.dex, &sd->status.luk
	};
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d", &value) < 1 || value == 0)
		value = battle_config.max_parameter;

	count = 0;
	for (index = 0; index < (int)(sizeof(status) / sizeof(status[0])); index++) {

		new_value = (int)*status[index] + value;
		if (value > 0 && ((unsigned int)value > battle_config.max_parameter || (unsigned int)new_value > battle_config.max_parameter)) // fix positiv overflow
			new_value = battle_config.max_parameter;
		else if (value < 0 && (value < -(int)battle_config.max_parameter || new_value < 1)) // fix negativ overflow
			new_value = 1;

		if (new_value != (int)*status[index]) {
			*status[index] = new_value;
			clif_updatestatus(sd, SP_STR + index);
			clif_updatestatus(sd, SP_USTR + index);
			status_calc_pc(sd, 0);
			count++;
		}
	}

	if (count > 0) // if at least 1 stat modified
		clif_displaymessage(fd, msg_table[84]); // All stats changed!
	else {
		if (value < 0)
			clif_displaymessage(fd, msg_table[177]); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_table[178]); // Impossible to increase a stat.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_guildlevelup(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int level = 0;
	short added_level;
	struct guild *guild_info;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d", &level) < 1 || level == 0) {
		clif_displaymessage(fd, "Please, enter a valid level (usage: @guildlvup/@guildlvlup <# of levels>).");
		return -1;
	}

	if (sd->status.guild_id <= 0 || (guild_info = guild_search(sd->status.guild_id)) == NULL) {
		clif_displaymessage(fd, msg_table[43]); // You're not in a guild.
		return -1;
	}
	if (strcmp(sd->status.name, guild_info->master) != 0) {
		clif_displaymessage(fd, msg_table[44]); // You're not the master of your guild.
		return -1;
	}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, 2);
		clif_displaymessage(fd, msg_table[179]); // Guild level changed.
	} else {
		clif_displaymessage(fd, msg_table[45]); // Guild level change failed.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_makeegg(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct item_data *item_data;
	int id, pet_id;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a monter/egg name/id (usage: @makeegg <pet_id>).");
		return -1;
	}

	if ((item_data = itemdb_searchname(message)) != NULL) // for egg name
		id = item_data->nameid;
	else if ((id = mobdb_searchname(message)) == 0) // for monster name
		id = atoi(message);

	pet_id = search_petDB_index(id, PET_CLASS);
	if (pet_id < 0)
		pet_id = search_petDB_index(id, PET_EGG);
	if (pet_id >= 0) {
		sd->catch_target_class = pet_db[pet_id].class_;
		intif_create_pet(
			sd->status.account_id, sd->status.char_id,
			(short)pet_db[pet_id].class_, (short)mob_db[pet_db[pet_id].class_].lv,
			(short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
			100, 0, 1, pet_db[pet_id].jname);
	} else {
		clif_displaymessage(fd, msg_table[180]); // The monter/egg name/id doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_hatch(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->status.pet_id <= 0)
		clif_sendegg(sd);
	else {
		clif_displaymessage(fd, msg_table[181]); // You already have a pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_petfriendly(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int friendly;
	int t;
	nullpo_retr(-1, sd);

	if (!message || !*message || (friendly = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: @petfriendly <0-1000>).");
		return -1;
	}

	if (sd->status.pet_id > 0 && sd->pd) {
		if (friendly >= 0 && friendly <= 1000) {
			if (friendly != sd->pet.intimate) {
				t = sd->pet.intimate;
				sd->pet.intimate = friendly;
				clif_send_petstatus(sd);
				if (battle_config.pet_status_support) {
					if ((sd->pet.intimate > 0 && t <= 0) ||
					    (sd->pet.intimate <= 0 && t > 0)) {
						if (sd->bl.prev != NULL)
							status_calc_pc(sd, 0);
						else
							status_calc_pc(sd, 2);
					}
				}
				clif_displaymessage(fd, msg_table[182]); // Pet friendly value changed!
			} else {
				clif_displaymessage(fd, msg_table[183]); // Pet friendly is already the good value.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_pethungry(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hungry;
	nullpo_retr(-1, sd);

	if (!message || !*message || (hungry = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a valid number (usage: @pethungry <0-100>).");
		return -1;
	}

	if (sd->status.pet_id > 0 && sd->pd) {
		if (hungry >= 0 && hungry <= 100) {
			if (hungry != sd->pet.hungry) {
				sd->pet.hungry = hungry;
				clif_send_petstatus(sd);
				clif_displaymessage(fd, msg_table[185]); // Pet hungry value changed!
			} else {
				clif_displaymessage(fd, msg_table[186]); // Pet hungry is already the good value.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_petrename(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->status.pet_id > 0 && sd->pd) {
		if (sd->pet.rename_flag != 0) {
			sd->pet.rename_flag = 0;
			intif_save_petdata(sd->status.account_id, &sd->pet);
			clif_send_petstatus(sd);
			clif_displaymessage(fd, msg_table[187]); // You can now rename your pet.
		} else {
			clif_displaymessage(fd, msg_table[188]); // You can already rename your pet.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[184]); // Sorry, but you have no pet.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int
atcommand_recall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @recall <char name>).");
		return -1;
	}

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);
	if(sscanf(message, "%99[^\n]", atcmd_player_name) < 1)
		return -1;
	if(strncmp(sd->status.name,atcmd_player_name,24)==0)
		return -1;

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can recall only lower or same level
			if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
				return -1;
			}
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
				return -1;
			}
			pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			sprintf(atcmd_output, msg_table[46], atcmd_player_name); // %s recalled!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_revive(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @revive <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isdead(pl_sd)) {
			pl_sd->status.hp = pl_sd->status.max_hp;
			clif_skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
			pc_setstand(pl_sd);
			if (battle_config.pc_invincible_time > 0)
				pc_setinvincibletimer(pl_sd, battle_config.pc_invincible_time);
			clif_updatestatus(pl_sd, SP_HP);
			clif_updatestatus(pl_sd, SP_SP);
			clif_resurrection(&pl_sd->bl, 1);
			clif_displaymessage(fd, msg_table[51]); // Character revived.
			return 0;
		}
		return -1;
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * charchangesex command (usage: charchangesex <player_name>)
 *------------------------------------------
 */
int atcommand_char_change_sex(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charchangesex <name>).");
		return -1;
	}

	// check player name
	if (strlen(atcmd_player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(atcmd_player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charblock command (usage: charblock <player_name>)
 * This command do a definitiv ban on a player
 *------------------------------------------
 */
int atcommand_char_block(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charblock/@block <name>).");
		return -1;
	}

	// check player name
	if (strlen(atcmd_player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(atcmd_player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 1, 0, 0, 0, 0, 0, 0); // type: 1 - block
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charban command (usage: charban <time> <player_name>)
 * This command do a limited ban on a player
 * Time is done as follows:
 *   Adjustment value (-1, 1, +1, etc...)
 *   Modified element:
 *     a or y: year
 *     m:  month
 *     j or d: day
 *     h:  hour
 *     mn: minute
 *     s:  second
 * <example> @ban +1m-2mn1s-6y test_player
 *           this example adds 1 month and 1 second, and substracts 2 minutes and 6 years at the same time.
 *------------------------------------------
 */
int atcommand_char_ban(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char * modif_p;
	int year, month, day, hour, minute, second, value;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%s %99[^\n]", atcmd_output, atcmd_player_name) < 2) {
		clif_displaymessage(fd, "Please, enter ban time and a player name (usage: @charban/@ban/@banish/@charbanish <time> <name>).");
		return -1;
	}

	atcmd_output[sizeof(atcmd_output)-1] = '\0';

	modif_p = atcmd_output;
	year = month = day = hour = minute = second = 0;
	while (modif_p[0] != '\0') {
		value = atoi(modif_p);
		if (value == 0)
			modif_p++;
		else {
			if (modif_p[0] == '-' || modif_p[0] == '+')
				modif_p++;
			while (modif_p[0] >= '0' && modif_p[0] <= '9')
				modif_p++;
			if (modif_p[0] == 's') {
				second = value;
				modif_p++;
			} else if (modif_p[0] == 'm' && modif_p[1] == 'n') {
				minute = value;
				modif_p = modif_p + 2;
			} else if (modif_p[0] == 'h') {
				hour = value;
				modif_p++;
			} else if (modif_p[0] == 'd' || modif_p[0] == 'j') {
				day = value;
				modif_p++;
			} else if (modif_p[0] == 'm') {
				month = value;
				modif_p++;
			} else if (modif_p[0] == 'y' || modif_p[0] == 'a') {
				year = value;
				modif_p++;
			} else if (modif_p[0] != '\0') {
				modif_p++;
			}
		}
	}
	if (year == 0 && month == 0 && day == 0 && hour == 0 && minute == 0 && second == 0) {
		clif_displaymessage(fd, msg_table[85]); // Invalid time for ban command.
		return -1;
	}

	// check player name
	if (strlen(atcmd_player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(atcmd_player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 2, year, month, day, hour, minute, second); // type: 2 - ban
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charunblock command (usage: charunblock <player_name>)
 *------------------------------------------
 */
int atcommand_char_unblock(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunblock <player_name>).");
		return -1;
	}

	// check player name
	if (strlen(atcmd_player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(atcmd_player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 3, 0, 0, 0, 0, 0, 0); // type: 3 - unblock
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * charunban command (usage: charunban <player_name>)
 *------------------------------------------
 */
int atcommand_char_unban(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charunban <player_name>).");
		return -1;
	}

	// check player name
	if (strlen(atcmd_player_name) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(atcmd_player_name) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		// send answer to login server via char-server
		chrif_char_ask_name(sd->status.account_id, atcmd_player_name, 4, 0, 0, 0, 0, 0, 0); // type: 4 - unban
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_night(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	if (night_flag != 1) {
		night_flag = 1; // 0=day, 1=night [Yor]
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && !map[sd->bl.m].flag.indoors) {
				//pl_sd->opt2 |= STATE_BLIND;
				//clif_changeoption(&pl_sd->bl);
				if (battle_config.night_darkness_level > 0)
					clif_specialeffect(&pl_sd->bl, 474 + battle_config.night_darkness_level, 0);
				else {
					//clif_specialeffect(&pl_sd->bl, 483, 0); // default darkness level
					pl_sd->opt2 |= STATE_BLIND;
					clif_changeoption(&pl_sd->bl);
				}
				clif_displaymessage(pl_sd->fd, msg_table[59]); // Night has fallen.
			}
		}
	} else {
		clif_displaymessage(fd, msg_table[89]); // Sorry, it's already the night. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_day(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	if (night_flag != 0) {
		night_flag = 0; // 0=day, 1=night [Yor]
		for(i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				if (battle_config.night_darkness_level > 0)
					clif_refresh (pl_sd);
				else {
					pl_sd->opt2 &= ~STATE_BLIND;
					clif_changeoption(&pl_sd->bl);
				}
				clif_displaymessage(pl_sd->fd, msg_table[60]); // Day has arrived.
			}
		}
	} else {
		clif_displaymessage(fd, msg_table[90]); // Sorry, it's already the day. Impossible to execute the command.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_doom(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,2);
	for(i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && i != fd &&
		    pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can doom only lower or same gm level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
			clif_displaymessage(pl_sd->fd, msg_table[61]); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_table[62]); // Judgement was made.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_doommap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);
	clif_specialeffect(&sd->bl,450,3);
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && i != fd && sd->bl.m == pl_sd->bl.m &&
		    pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can doom only lower or same gm level
			pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
//			clif_specialeffect(&pl_sd->bl,450,1);
			clif_displaymessage(pl_sd->fd, msg_table[61]); // The holy messenger has given judgement.
		}
	}
	clif_displaymessage(fd, msg_table[62]); // Judgement was made.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static void atcommand_raise_sub(struct map_session_data* sd)
{
	if (sd && sd->state.auth && pc_isdead(sd)) {
		clif_skill_nodamage(&sd->bl,&sd->bl,ALL_RESURRECTION,4,1);
		sd->status.hp = sd->status.max_hp;
		sd->status.sp = sd->status.max_sp;
		pc_setstand(sd);
		clif_updatestatus(sd, SP_HP);
		clif_updatestatus(sd, SP_SP);
		clif_resurrection(&sd->bl, 1);
		if (battle_config.pc_invincible_time > 0)
			pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
		clif_displaymessage(sd->fd, msg_table[63]); // Mercy has been shown.
	}
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_raise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);

	for (i = 0; i < fd_max; i++) {
		if (session[i])
			atcommand_raise_sub((struct map_session_data *) session[i]->session_data);
	}
	clif_displaymessage(fd, msg_table[64]); // Mercy has been granted.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_raisemap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;

	nullpo_retr(-1, sd);

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && sd->bl.m == pl_sd->bl.m)
			atcommand_raise_sub(pl_sd);
	}
	clif_displaymessage(fd, msg_table[64]); // Mercy has been granted.

	return 0;
}

/*==========================================
 * atcommand_character_baselevel @charbaselvlÇ≈ëŒè€ÉLÉÉÉâÇÃÉåÉxÉãÇè„Ç∞ÇÈ
 *------------------------------------------
*/
int atcommand_character_baselevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int level = 0, i;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &level, atcmd_player_name) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: @charbaselvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change base level only lower or same gm level

			if (level > 0) {
				if (pl_sd->status.base_level == battle_config.maximum_level) {	// check for max level by Valaris
					clif_displaymessage(fd, msg_table[91]); // Character's base level can't go any higher.
					return 0;
				}	// End Addition
				if ((unsigned int)level > battle_config.maximum_level || (unsigned int)level > (battle_config.maximum_level - pl_sd->status.base_level)) // fix positiv overflow
					level = battle_config.maximum_level - pl_sd->status.base_level;
				for (i = 1; i <= level; i++)
					pl_sd->status.status_point += (pl_sd->status.base_level + i + 14) / 5;
				pl_sd->status.base_level += level;
				clif_updatestatus(pl_sd, SP_BASELEVEL);
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				status_calc_pc(pl_sd, 0);
				pc_heal(pl_sd, pl_sd->status.max_hp, pl_sd->status.max_sp);
				clif_misceffect(&pl_sd->bl, 0);
				clif_displaymessage(fd, msg_table[65]); // Character's base level raised.
			} else {
				if (pl_sd->status.base_level == 1) {
					clif_displaymessage(fd, msg_table[193]); // Character's base level can't go any lower.
					return -1;
				}
				if (level < -(int)battle_config.maximum_level || level < (1 - (int)pl_sd->status.base_level)) // fix negativ overflow
					level = 1 - pl_sd->status.base_level;
				if (pl_sd->status.status_point > 0) {
					for (i = 0; i > level; i--)
						pl_sd->status.status_point -= (pl_sd->status.base_level + i + 14) / 5;
					if (pl_sd->status.status_point < 0)
						pl_sd->status.status_point = 0;
					clif_updatestatus(pl_sd, SP_STATUSPOINT);
				} // to add: remove status points from stats
				pl_sd->status.base_level += level;
				clif_updatestatus(pl_sd, SP_BASELEVEL);
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
				status_calc_pc(pl_sd, 0);
				clif_displaymessage(fd, msg_table[66]); // Character's base level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0; //ê≥èÌèIóπ
}

/*==========================================
 * atcommand_character_joblevel @charjoblvlÇ≈ëŒè€ÉLÉÉÉâÇÃJobÉåÉxÉãÇè„Ç∞ÇÈ
 *------------------------------------------
 */
int atcommand_character_joblevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	unsigned int max_level = 50;
	int level = 0;
	//ì]ê∂Ç‚ó{éqÇÃèÍçáÇÃå≥ÇÃêEã∆ÇéZèoÇ∑ÇÈ
	struct pc_base_job pl_s_class;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &level, atcmd_player_name) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: @charjlvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		pl_s_class = pc_calc_base_job(pl_sd->status.class_);
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job level only lower or same gm level
			if (pl_s_class.job == 0)
				max_level -= 40;
			// super novices can go up to 99 [celest]
			else if (pl_s_class.job == 23)
				max_level += 49;
			else if (pl_sd->status.class_ > 4007 && pl_sd->status.class_ < 4023)
				max_level += 20;

			if (level > 0) {
				if (pl_sd->status.job_level == max_level) {
					clif_displaymessage(fd, msg_table[67]); // Character's job level can't go any higher.
					return -1;
				}
				if (pl_sd->status.job_level + level > max_level)
					level = max_level - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				pl_sd->status.skill_point += level;
				clif_updatestatus(pl_sd, SP_SKILLPOINT);
				status_calc_pc(pl_sd, 0);
				clif_misceffect(&pl_sd->bl, 1);
				clif_displaymessage(fd, msg_table[68]); // character's job level raised.
			} else {
				if (pl_sd->status.job_level == 1) {
					clif_displaymessage(fd, msg_table[194]); // Character's job level can't go any lower.
					return -1;
				}
				if (pl_sd->status.job_level + level < 1)
					level = 1 - pl_sd->status.job_level;
				pl_sd->status.job_level += level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				if (pl_sd->status.skill_point > 0) {
					pl_sd->status.skill_point += level;
					if (pl_sd->status.skill_point < 0)
						pl_sd->status.skill_point = 0;
					clif_updatestatus(pl_sd, SP_SKILLPOINT);
				} // to add: remove status points from skills
				status_calc_pc(pl_sd, 0);
				clif_displaymessage(fd, msg_table[69]); // Character's job level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_kick(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @kick <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) // you can kick only lower or same gm level
			clif_GM_kick(sd, pl_sd, 1);
		else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_kickall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth &&
		    pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kick only lower or same gm level
			if (sd->status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, pl_sd, 0);
			}
		}

	clif_displaymessage(fd, msg_table[195]); // All players have been kicked!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_allskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	pc_allskillup(sd); // all skills
	sd->status.skill_point = 0; // 0 skill points
	clif_updatestatus(sd, SP_SKILLPOINT); // update
	clif_displaymessage(fd, msg_table[76]); // You have received all skills.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_questskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int skill_id;
	nullpo_retr(-1, sd);

	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @questskill <#:0+>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) == 0) {
				pc_skill(sd, skill_id, 1, 0);
				clif_displaymessage(fd, msg_table[70]); // You have learned the skill.
			} else {
				clif_displaymessage(fd, msg_table[196]); // You already have this quest skill.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_charquestskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int skill_id = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &skill_id, atcmd_player_name) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charquestskill <#:0+> <char_name>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
				if (pc_checkskill(pl_sd, skill_id) == 0) {
					pc_skill(pl_sd, skill_id, 1, 0);
					clif_displaymessage(fd, msg_table[199]); // This player has learned the skill.
				} else {
					clif_displaymessage(fd, msg_table[200]); // This player already has this quest skill.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[3]); // Character not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_lostskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int skill_id;
	nullpo_retr(-1, sd);

	if (!message || !*message || (skill_id = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number (usage: @lostskill <#:0+>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if (pc_checkskill(sd, skill_id) > 0) {
				sd->status.skill[skill_id].lv = 0;
				sd->status.skill[skill_id].flag = 0;
				clif_skillinfoblock(sd);
				clif_displaymessage(fd, msg_table[71]); // You have forgotten the skill.
			} else {
				clif_displaymessage(fd, msg_table[201]); // You don't have this quest skill.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_charlostskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int skill_id = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &skill_id, atcmd_player_name) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charlostskill <#:0+> <char_name>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & 0x01) {
			if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
				if (pc_checkskill(pl_sd, skill_id) > 0) {
					pl_sd->status.skill[skill_id].lv = 0;
					pl_sd->status.skill[skill_id].flag = 0;
					clif_skillinfoblock(pl_sd);
					clif_displaymessage(fd, msg_table[202]); // This player has forgotten the skill.
				} else {
					clif_displaymessage(fd, msg_table[203]); // This player doesn't have this quest skill.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[3]); // Character not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[197]); // This skill number doesn't exist or isn't a quest skill.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[198]); // This skill number doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_spiritball(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int number;
	nullpo_retr(-1, sd);

	if (!message || !*message || (number = atoi(message)) < 0) {
		clif_displaymessage(fd, "Please, enter a spirit ball number (usage: @spiritball <number: 0-1000>).");
		return -1;
	}

	// set max number to avoid server/client crash (500 create big balls of several balls: no visial difference with more)
	if (number > 500)
		number = 500;

	if (number >= 0 && number <= 0x7FFF) {
		if (sd->spiritball != number || number > 499) {
			if (sd->spiritball > 0)
				pc_delspiritball(sd, sd->spiritball, 1);
			sd->spiritball = number;
			clif_spiritball(sd);
			// no message, player can look the difference
			if (number > 1000)
				clif_displaymessage(fd, msg_table[204]); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
		} else {
			clif_displaymessage(fd, msg_table[205]); // You already have this number of spiritballs.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_party(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char party[100];
	nullpo_retr(-1, sd);

	memset(party, '\0', sizeof(party));

	if (!message || !*message || sscanf(message, "%99[^\n]", party) < 1) {
		clif_displaymessage(fd, "Please, enter a party name (usage: @party <party_name>).");
		return -1;
	}

	party_create(sd, party, 0, 0);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_guild(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char guild[100];
	int prev;
	nullpo_retr(-1, sd);

	memset(guild, '\0', sizeof(guild));

	if (!message || !*message || sscanf(message, "%99[^\n]", guild) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name (usage: @guild <guild_name>).");
		return -1;
	}

	prev = battle_config.guild_emperium_check;
	battle_config.guild_emperium_check = 0;
	guild_create(sd, guild);
	battle_config.guild_emperium_check = prev;

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_agitstart(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (agit_flag == 1) {
		clif_displaymessage(fd, msg_table[73]); // Already it has started siege warfare.
		return -1;
	}

	agit_flag = 1;
	guild_agit_start();
	clif_displaymessage(fd, msg_table[72]); // Guild siege warfare start!

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_agitend(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (agit_flag == 0) {
		clif_displaymessage(fd, msg_table[75]); // Siege warfare hasn't started yet.
		return -1;
	}

	agit_flag = 0;
	guild_agit_end();
	clif_displaymessage(fd, msg_table[74]); // Guild siege warfare end!

	return 0;
}

/*==========================================
 * @mapexitÇ≈É}ÉbÉvÉTÅ[ÉoÅ[ÇèIóπÇ≥ÇπÇÈ
 *------------------------------------------
 */
int atcommand_mapexit(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
			if (sd->status.account_id != pl_sd->status.account_id)
				clif_GM_kick(sd, pl_sd, 0);
		}
	}
	clif_GM_kick(sd, sd, 0);
	
	flush_fifos();

	runflag = 0;

        return 0;
}

/*==========================================
 * idsearch <part_of_name>: revrited by [Yor]
 *------------------------------------------
 */
int atcommand_idsearch(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	unsigned int i, match;
	struct item_data *item;
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99s", item_name) < 0) {
		clif_displaymessage(fd, "Please, enter a part of item name (usage: @idsearch <part_of_item_name>).");
		return -1;
	}

	sprintf(atcmd_output, msg_table[77], item_name); // The reference result of '%s' (name: id):
	clif_displaymessage(fd, atcmd_output);
	match = 0;
	for(i = 0; i < 20000; i++) {
		if ((item = itemdb_exists(i)) != NULL && strstr(item->jname, item_name) != NULL) {
			match++;
			sprintf(atcmd_output, msg_table[78], item->jname, item->nameid); // %s: %d
			clif_displaymessage(fd, atcmd_output);
		}
	}
	sprintf(atcmd_output, msg_table[79], match); // It is %d affair above.
	clif_displaymessage(fd, atcmd_output);

	return 0;
}

/*==========================================
 * Character Skill Reset
 *------------------------------------------
 */
int atcommand_charskreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charskreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset skill points only lower or same gm level
			pc_resetskill(pl_sd);
			sprintf(atcmd_output, msg_table[206], atcmd_player_name); // '%s' skill points reseted!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Stat Reset
 *------------------------------------------
 */
int atcommand_charstreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charstreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset stats points only lower or same gm level
			pc_resetstate(pl_sd);
			sprintf(atcmd_output, msg_table[207], atcmd_player_name); // '%s' stats points reseted!
			clif_displaymessage(fd, atcmd_output);
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Model by chbrules
 *------------------------------------------
 */
int atcommand_charmodel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%d %d %d %99[^\n]", &hair_style, &hair_color, &cloth_color, atcmd_player_name) < 4 || hair_style < 0 || hair_color < 0 || cloth_color < 0) {
		sprintf(atcmd_output, "Please, enter a valid model and a player name (usage: @charmodel <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d> <name>).",
		        MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
		    hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
		    cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {

			if (cloth_color != 0 &&
			    pl_sd->status.sex == 1 &&
			    (pl_sd->status.class_ == 12 ||  pl_sd->status.class_ == 17)) {
				clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class.
				return -1;
			} else {
				pc_changelook(pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_table[36]); // Appearence changed.
			}
		} else {
			clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Skill Point (Rewritten by [Yor])
 *------------------------------------------
 */
int atcommand_charskpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int new_skill_point;
	int point = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &point, atcmd_player_name) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charskpoint <amount> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		new_skill_point = (int)pl_sd->status.skill_point + point;
		if (point > 0 && (point > 0x7FFF || new_skill_point > 0x7FFF)) // fix positiv overflow
			new_skill_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_skill_point < 0)) // fix negativ overflow
			new_skill_point = 0;
		if (new_skill_point != (int)pl_sd->status.skill_point) {
			pl_sd->status.skill_point = new_skill_point;
			clif_updatestatus(pl_sd, SP_SKILLPOINT);
			clif_displaymessage(fd, msg_table[209]); // Character's number of skill points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Status Point (rewritten by [Yor])
 *------------------------------------------
 */
int atcommand_charstpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int new_status_point;
	int point = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &point, atcmd_player_name) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charstpoint <amount> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		new_status_point = (int)pl_sd->status.status_point + point;
		if (point > 0 && (point > 0x7FFF || new_status_point > 0x7FFF)) // fix positiv overflow
			new_status_point = 0x7FFF;
		else if (point < 0 && (point < -0x7FFF || new_status_point < 0)) // fix negativ overflow
			new_status_point = 0;
		if (new_status_point != (int)pl_sd->status.status_point) {
			pl_sd->status.status_point = new_status_point;
			clif_updatestatus(pl_sd, SP_STATUSPOINT);
			clif_displaymessage(fd, msg_table[210]); // Character's number of status points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_table[41]); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_table[149]); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Recall All Characters Online To Your Location
 *------------------------------------------
 */
int atcommand_recallall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	int count;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	count = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && sd->status.account_id != pl_sd->status.account_id &&
		    pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can recall only lower or same level
			if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
				count++;
			else
				pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
		}
	}

	clif_displaymessage(fd, msg_table[92]); // All characters recalled!
	if (count) {
		sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
		clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * Recall online characters of a guild to your location
 *------------------------------------------
 */
int atcommand_guildrecall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	char guild_name[100];
	struct guild *g;
	int count;
	nullpo_retr(-1, sd);

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildrecall <guild_name/id>).");
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		count = 0;
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth &&
			    sd->status.account_id != pl_sd->status.account_id &&
			    pl_sd->status.guild_id == g->guild_id) {
				if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
					count++;
				else
					pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			}
		}
		sprintf(atcmd_output, msg_table[93], g->name); // All online characters of the %s guild are near you.
		clif_displaymessage(fd, atcmd_output);
		if (count) {
			sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_table[94]); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	return 0;
}

/*==========================================
 * Recall online characters of a party to your location
 *------------------------------------------
 */
int atcommand_partyrecall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd;
	char party_name[100];
	struct party *p;
	int count;
	nullpo_retr(-1, sd);

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyrecall <party_name/id>).");
		return -1;
	}

	if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp somenone to your actual map.");
		return -1;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		count = 0;
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth &&
			    sd->status.account_id != pl_sd->status.account_id &&
			    pl_sd->status.party_id == p->party_id) {
				if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd))
					count++;
				else
					pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
			}
		}
		sprintf(atcmd_output, msg_table[95], p->name); // All online characters of the %s party are near you.
		clif_displaymessage(fd, atcmd_output);
		if (count) {
			sprintf(atcmd_output, "Because you are not authorised to warp from some maps, %d player(s) have not been recalled.", count);
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_table[96]); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_reloaditemdb(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	itemdb_reload();
	clif_displaymessage(fd, msg_table[97]); // Item database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_reloadmobdb(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	mob_reload();
	read_petdb();
	clif_displaymessage(fd, msg_table[98]); // Monster database reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_reloadskilldb(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	skill_reload();
	clif_displaymessage(fd, msg_table[99]); // Skill database reloaded.

	return 0;
}

/*==========================================
 * @reloadatcommand
 *   atcommand_athena.conf ÇÃÉäÉçÅ[Éh
 *------------------------------------------
 */
int
atcommand_reloadatcommand(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	atcommand_config_read(ATCOMMAND_CONF_FILENAME);
	clif_displaymessage(fd, msg_table[254]);
	return 0;
}
/*==========================================
 * @reloadbattleconf
 *   battle_athena.conf ÇÃÉäÉçÅ[Éh
 *------------------------------------------
 */
int
atcommand_reloadbattleconf(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	battle_config_read(BATTLE_CONF_FILENAME);
	clif_displaymessage(fd, msg_table[255]);
	return 0;
}
/*==========================================
 * @reloadstatusdb
 *   job_db1.txt job_db2.txt job_db2-2.txt 
 *   refine_db.txt size_fix.txt
 *   ÇÃÉäÉçÅ[Éh
 *------------------------------------------
 */
int
atcommand_reloadstatusdb(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	status_readdb();
	clif_displaymessage(fd, msg_table[256]);
	return 0;
}
/*==========================================
 * @reloadpcdb
 *   exp.txt skill_tree.txt attr_fix.txt 
 *   ÇÃÉäÉçÅ[Éh
 *------------------------------------------
 */
int
atcommand_reloadpcdb(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	pc_readdb();
	clif_displaymessage(fd, msg_table[257]);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void rehash(void)
{
	int map_id;

	for (map_id = 0; map_id < map_num; map_id++) {
		map_foreachinarea(cleanup_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_MOB);
		map_foreachinarea(cleanup_sub, map_id, 0, 0, map[map_id].xs, map[map_id].ys, BL_NPC);
	}
}
int atcommand_reloadscript(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	atcommand_broadcast( fd, sd, "@broadcast", "eAthena Server is Rehashing..." );
	atcommand_broadcast( fd, sd, "@broadcast", "You will feel a bit of lag at this point !" );

	rehash();

	atcommand_broadcast( fd, sd, "@broadcast", "Reloading NPCs..." );
	//do_init_npc();
	do_init_script();
	npc_reload();
	npc_event_do_oninit();

	clif_displaymessage(fd, msg_table[100]); // Scripts reloaded.

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_reloadgmdb( // by [Yor]
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	chrif_reloadGMdb();

	clif_displaymessage(fd, msg_table[101]); // Login-server asked to reload GM accounts and their level.

	return 0;
}

/*==========================================
 * @mapinfo <map name> [0-3] by MC_Cameri
 * => Shows information about the map [map name]
 * 0 = no additional information
 * 1 = Show users in that map and their location
 * 2 = Shows NPCs in that map
 * 3 = Shows the shops/chats in that map (not implemented)
 *------------------------------------------
 */
int atcommand_mapinfo(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct npc_data *nd = NULL;
	struct chat_data *cd = NULL;
	char direction[12];
	int m_id, i, chat_num, list = 0;
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(direction, '\0', sizeof(direction));

	sscanf(message, "%d %99[^\n]", &list, atcmd_player_name);

	if (list < 0 || list > 3) {
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return -1;
	}

	if (atcmd_player_name[0] == '\0')
		strcpy(atcmd_player_name, sd->mapname);
	if (strstr(atcmd_player_name, ".gat") == NULL && strstr(atcmd_player_name, ".afm") == NULL && strlen(atcmd_player_name) < 13) // 16 - 4 (.gat)
		strcat(atcmd_player_name, ".gat");

	if ((m_id = map_mapname2mapid(atcmd_player_name)) < 0) {
		clif_displaymessage(fd, msg_table[1]); // Map not found.
		return -1;
	}

	clif_displaymessage(fd, "------ Map Info ------");
	sprintf(atcmd_output, "Map Name: %s", atcmd_player_name);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Players In Map: %d", map[m_id].users);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "NPCs In Map: %d", map[m_id].npc_num);
	clif_displaymessage(fd, atcmd_output);
	chat_num = 0;
	for (i = 0; i < fd_max; i++) {
		if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth &&
		    (cd = (struct chat_data*)map_id2bl(pl_sd->chatID))) {
			chat_num++;
		}
	}
	sprintf(atcmd_output, "Chats In Map: %d", chat_num);
	clif_displaymessage(fd, atcmd_output);
	clif_displaymessage(fd, "------ Map Flags ------");
	sprintf(atcmd_output, "Player vs Player: %s | No Guild: %s | No Party: %s",
	        (map[m_id].flag.pvp) ? "True" : "False",
	        (map[m_id].flag.pvp_noguild) ? "True" : "False",
	        (map[m_id].flag.pvp_noparty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "Guild vs Guild: %s | No Party: %s", (map[m_id].flag.gvg) ? "True" : "False", (map[m_id].flag.gvg_noparty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Dead Branch: %s", (map[m_id].flag.nobranch) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Memo: %s", (map[m_id].flag.nomemo) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Penalty: %s", (map[m_id].flag.nopenalty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Return: %s", (map[m_id].flag.noreturn) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Save: %s", (map[m_id].flag.nosave) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Teleport: %s", (map[m_id].flag.noteleport) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Monster Teleport: %s", (map[m_id].flag.monster_noteleport) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, "No Zeny Penalty: %s", (map[m_id].flag.nozenypenalty) ? "True" : "False");
	clif_displaymessage(fd, atcmd_output);

	switch (list) {
	case 0:
		// Do nothing. It's list 0, no additional display.
		break;
	case 1:
		clif_displaymessage(fd, "----- Players in Map -----");
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && strcmp(pl_sd->mapname, atcmd_player_name) == 0) {
				sprintf(atcmd_output, "Player '%s' (session #%d) | Location: %d,%d",
				        pl_sd->status.name, i, pl_sd->bl.x, pl_sd->bl.y);
				clif_displaymessage(fd, atcmd_output);
			}
		}
		break;
	case 2:
		clif_displaymessage(fd, "----- NPCs in Map -----");
		for (i = 0; i < map[m_id].npc_num;) {
			nd = map[m_id].npc[i];
			switch(nd->dir) {
			case 0:  strcpy(direction, "North"); break;
			case 1:  strcpy(direction, "North West"); break;
			case 2:  strcpy(direction, "West"); break;
			case 3:  strcpy(direction, "South West"); break;
			case 4:  strcpy(direction, "South"); break;
			case 5:  strcpy(direction, "South East"); break;
			case 6:  strcpy(direction, "East"); break;
			case 7:  strcpy(direction, "North East"); break;
			case 9:  strcpy(direction, "North"); break;
			default: strcpy(direction, "Unknown"); break;
			}
			sprintf(atcmd_output, "NPC %d: %s | Direction: %s | Sprite: %d | Location: %d %d",
			        ++i, nd->name, direction, nd->class_, nd->bl.x, nd->bl.y);
			clif_displaymessage(fd, atcmd_output);
		}
		break;
	case 3:
		clif_displaymessage(fd, "----- Chats in Map -----");
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth &&
			    (cd = (struct chat_data*)map_id2bl(pl_sd->chatID)) &&
			    strcmp(pl_sd->mapname, atcmd_player_name) == 0 &&
			    cd->usersd[0] == pl_sd) {
				sprintf(atcmd_output, "Chat %d: %s | Player: %s | Location: %d %d",
				        i, cd->title, pl_sd->status.name, cd->bl.x, cd->bl.y);
				clif_displaymessage(fd, atcmd_output);
				sprintf(atcmd_output, "   Users: %d/%d | Password: %s | Public: %s",
				        cd->users, cd->limit, cd->pass, (cd->pub) ? "Yes" : "No");
				clif_displaymessage(fd, atcmd_output);
			}
		}
		break;
	default: // normally impossible to arrive here
		clif_displaymessage(fd, "Please, enter at least a valid list number (usage: @mapinfo <0-3> [map]).");
		return -1;
		break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_mount_peco(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
		clif_displaymessage(fd, msg_table[212]); // Cannot mount a Peco while in disguise.
		return -1;
	}

	if (!pc_isriding(sd)) { // if actually no peco
		if (sd->status.class_ == 7 || sd->status.class_ == 14 || sd->status.class_ == 4008 || sd->status.class_ == 4015) {
			if (sd->status.class_ == 7)
				sd->status.class_ = sd->view_class = 13;
			else if (sd->status.class_ == 14)
				sd->status.class_ = sd->view_class = 21;
			else if (sd->status.class_ == 4008)
				sd->status.class_ = sd->view_class = 4014;
			else if (sd->status.class_ == 4015)
				sd->status.class_ = sd->view_class = 4022;
			pc_setoption(sd, sd->status.option | 0x0020);
			clif_displaymessage(fd, msg_table[102]); // Mounted Peco.
		} else {
			clif_displaymessage(fd, msg_table[213]); // You can not mount a peco with your job.
			return -1;
		}
	} else {
		if (sd->status.class_ == 13)
			sd->status.class_ = sd->view_class = 7;
		else if (sd->status.class_ == 21)
			sd->status.class_ = sd->view_class = 14;
		else if (sd->status.class_ == 4014)
			sd->status.class_ = sd->view_class = 4008;
		else if (sd->status.class_ == 4022)
			sd->status.class_ = sd->view_class = 4015;
		pc_setoption(sd, sd->status.option & ~0x0020);
		clif_displaymessage(fd, msg_table[214]); // Unmounted Peco.
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_char_mount_peco(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charmountpeco <char_name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pl_sd->disguise > 0) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
			clif_displaymessage(fd, msg_table[215]); // This player cannot mount a Peco while in disguise.
			return -1;
		}

		if (!pc_isriding(pl_sd)) { // if actually no peco
			if (pl_sd->status.class_ == 7 || pl_sd->status.class_ == 14 || pl_sd->status.class_ == 4008 || pl_sd->status.class_ == 4015) {
				if (pl_sd->status.class_ == 7)
					pl_sd->status.class_ = pl_sd->view_class = 13;
				else if (pl_sd->status.class_ == 14)
					pl_sd->status.class_ = pl_sd->view_class = 21;
				else if (pl_sd->status.class_ == 4008)
					pl_sd->status.class_ = pl_sd->view_class = 4014;
				else if (pl_sd->status.class_ == 4015)
					pl_sd->status.class_ = pl_sd->view_class = 4022;
				pc_setoption(pl_sd, pl_sd->status.option | 0x0020);
				clif_displaymessage(fd, msg_table[216]); // Now, this player mounts a peco.
			} else {
				clif_displaymessage(fd, msg_table[217]); // This player can not mount a peco with his/her job.
				return -1;
			}
		} else {
			if (pl_sd->status.class_ == 13)
				pl_sd->status.class_ = pl_sd->view_class = 7;
			else if (pl_sd->status.class_ == 21)
				pl_sd->status.class_ = pl_sd->view_class = 14;
			else if (pl_sd->status.class_ == 4014)
				pl_sd->status.class_ = pl_sd->view_class = 4008;
			else if (pl_sd->status.class_ == 4022)
				pl_sd->status.class_ = pl_sd->view_class = 4015;
			pc_setoption(pl_sd, pl_sd->status.option & ~0x0020);
			clif_displaymessage(fd, msg_table[218]); // Now, this player has not more peco.
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 *Spy Commands by Syrus22
 *------------------------------------------
 */
int atcommand_guildspy(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char guild_name[100];
	struct guild *g;
	nullpo_retr(-1, sd);

	memset(guild_name, '\0', sizeof(guild_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", guild_name) < 1) {
		clif_displaymessage(fd, "Please, enter a guild name/id (usage: @guildspy <guild_name/id>).");
		return -1;
	}

	if ((g = guild_searchname(guild_name)) != NULL || // name first to avoid error when name begin with a number
	    (g = guild_search(atoi(message))) != NULL) {
		if (sd->guildspy == g->guild_id) {
			sd->guildspy = 0;
			sprintf(atcmd_output, msg_table[103], g->name); // No longer spying on the %s guild.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->guildspy = g->guild_id;
			sprintf(atcmd_output, msg_table[104], g->name); // Spying on the %s guild.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_table[94]); // Incorrect name/ID, or no one from the guild is online.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_partyspy(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char party_name[100];
	struct party *p;
	nullpo_retr(-1, sd);

	memset(party_name, '\0', sizeof(party_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%99[^\n]", party_name) < 1) {
		clif_displaymessage(fd, "Please, enter a party name/id (usage: @partyspy <party_name/id>).");
		return -1;
	}

	if ((p = party_searchname(party_name)) != NULL || // name first to avoid error when name begin with a number
	    (p = party_search(atoi(message))) != NULL) {
		if (sd->partyspy == p->party_id) {
			sd->partyspy = 0;
			sprintf(atcmd_output, msg_table[105], p->name); // No longer spying on the %s party.
			clif_displaymessage(fd, atcmd_output);
		} else {
			sd->partyspy = p->party_id;
			sprintf(atcmd_output, msg_table[106], p->name); // Spying on the %s party.
			clif_displaymessage(fd, atcmd_output);
		}
	} else {
		clif_displaymessage(fd, msg_table[96]); // Incorrect name or ID, or no one from the party is online.
		return -1;
	}

	return 0;
}

/*==========================================
 * @repairall [Valaris]
 *------------------------------------------
 */
int atcommand_repairall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int count, i;
	nullpo_retr(-1, sd);

	count = 0;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].nameid && sd->status.inventory[i].attribute == 1) {
			sd->status.inventory[i].attribute = 0;
			clif_produceeffect(sd, 0, sd->status.inventory[i].nameid);
			count++;
		}
	}

	if (count > 0) {
		clif_misceffect(&sd->bl, 3);
		clif_equiplist(sd);
		clif_displaymessage(fd, msg_table[107]); // All items have been repaired.
	} else {
		clif_displaymessage(fd, msg_table[108]); // No item need to be repaired.
		return -1;
	}

	return 0;
}

// Removed @nuke for now in favor of alchemist marine sphere skill [Valaris]
int atcommand_nuke(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @nuke <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same GM level
			skill_castend_damage_id(&pl_sd->bl, &pl_sd->bl, NPC_SELFDESTRUCTION, 99, gettick(), 0);
			clif_displaymessage(fd, msg_table[109]); // Player has been nuked!
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}


/*==========================================
 *
 *------------------------------------------
 */
int atcommand_enablenpc(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char NPCname[100];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @enablenpc <NPC_name>).");
		return -1;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 1);
		clif_displaymessage(fd, msg_table[110]); // Npc Enabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int atcommand_hidenpc(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char NPCname[100];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @npcoff <NPC_name>).");
		return -1;
	}

	if (npc_name2id(NPCname) != NULL) {
		npc_enable(NPCname, 0);
		clif_displaymessage(fd, msg_table[112]); // Npc Disabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

int atcommand_disablenpc(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct npc_data *nd;
	char NPCname[100];
	nullpo_retr(-1, sd);

	memset(NPCname, '\0', sizeof(NPCname));

	if (!message || !*message || sscanf(message, "%99[^\n]", NPCname) < 1) {
		clif_displaymessage(fd, "Please, enter a NPC name (usage: @npcoff <NPC_name>).");
		return -1;
	}

	if ((nd = npc_name2id(NPCname)) != NULL) {
		npc_unload(nd);
		clif_displaymessage(fd, msg_table[112]); // Npc Disabled.
	} else {
		clif_displaymessage(fd, msg_table[111]); // This NPC doesn't exist.
		return -1;
	}

	return 0;
}

/*==========================================
 * time in txt for time command (by [Yor])
 *------------------------------------------
 */
char * txt_time(unsigned int duration) {
	int days, hours, minutes, seconds;
	char temp[256];
	static char temp1[256];

	memset(temp, '\0', sizeof(temp));
	memset(temp1, '\0', sizeof(temp1));

	if (duration < 0)
		duration = 0;

	days = duration / (60 * 60 * 24);
	duration = duration - (60 * 60 * 24 * days);
	hours = duration / (60 * 60);
	duration = duration - (60 * 60 * hours);
	minutes = duration / 60;
	seconds = duration - (60 * minutes);

	if (days < 2)
		sprintf(temp, msg_table[219], days); // %d day
	else
		sprintf(temp, msg_table[220], days); // %d days
	if (hours < 2)
		sprintf(temp1, msg_table[221], temp, hours); // %s %d hour
	else
		sprintf(temp1, msg_table[222], temp, hours); // %s %d hours
	if (minutes < 2)
		sprintf(temp, msg_table[223], temp1, minutes); // %s %d minute
	else
		sprintf(temp, msg_table[224], temp1, minutes); // %s %d minutes
	if (seconds < 2)
		sprintf(temp1, msg_table[225], temp, seconds); // %s and %d second
	else
		sprintf(temp1, msg_table[226], temp, seconds); // %s and %d seconds

	return temp1;
}

/*==========================================
 * @time/@date/@server_date/@serverdate/@server_time/@servertime: Display the date/time of the server (by [Yor]
 * Calculation management of GM modification (@day/@night GM commands) is done
 *------------------------------------------
 */
int atcommand_servertime(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct TimerData * timer_data;
	struct TimerData * timer_data2;
	time_t time_server;  // variable for number of seconds (used with time() function)
	struct tm *datetime; // variable for time in structure ->tm_mday, ->tm_sec, ...
	char temp[256];
	nullpo_retr(-1, sd);

	memset(temp, '\0', sizeof(temp));

	time(&time_server);  // get time in seconds since 1/1/1970
	datetime = localtime(&time_server); // convert seconds in structure
	// like sprintf, but only for date/time (Sunday, November 02 2003 15:12:52)
	strftime(temp, sizeof(temp)-1, msg_table[230], datetime); // Server time (normal time): %A, %B %d %Y %X.
	clif_displaymessage(fd, temp);

	if (battle_config.night_duration == 0 && battle_config.day_duration == 0) {
		if (night_flag == 0)
			clif_displaymessage(fd, msg_table[231]); // Game time: The game is in permanent daylight.
		else
			clif_displaymessage(fd, msg_table[232]); // Game time: The game is in permanent night.
	} else if (battle_config.night_duration == 0)
		if (night_flag == 1) { // we start with night
			timer_data = get_timer(day_timer_tid);
			sprintf(temp, msg_table[233], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_table[234]); // Game time: After, the game will be in permanent daylight.
		} else
			clif_displaymessage(fd, msg_table[231]); // Game time: The game is in permanent daylight.
	else if (battle_config.day_duration == 0)
		if (night_flag == 0) { // we start with day
			timer_data = get_timer(night_timer_tid);
			sprintf(temp, msg_table[235], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			clif_displaymessage(fd, msg_table[236]); // Game time: After, the game will be in permanent night.
		} else
			clif_displaymessage(fd, msg_table[232]); // Game time: The game is in permanent night.
	else {
		if (night_flag == 0) {
			timer_data = get_timer(night_timer_tid);
			timer_data2 = get_timer(day_timer_tid);
			sprintf(temp, msg_table[235], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in daylight for %s.
			clif_displaymessage(fd, temp);
			if (timer_data->tick > timer_data2->tick)
				sprintf(temp, msg_table[237], txt_time((timer_data->interval - abs(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in night for %s.
			else
				sprintf(temp, msg_table[237], txt_time(abs(timer_data->tick - timer_data2->tick) / 1000)); // Game time: After, the game will be in night for %s.
			clif_displaymessage(fd, temp);
			sprintf(temp, msg_table[238], txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		} else {
			timer_data = get_timer(day_timer_tid);
			timer_data2 = get_timer(night_timer_tid);
			sprintf(temp, msg_table[233], txt_time((timer_data->tick - gettick()) / 1000)); // Game time: The game is actualy in night for %s.
			clif_displaymessage(fd, temp);
			if (timer_data->tick > timer_data2->tick)
				sprintf(temp, msg_table[239], txt_time((timer_data->interval - abs(timer_data->tick - timer_data2->tick)) / 1000)); // Game time: After, the game will be in daylight for %s.
			else
				sprintf(temp, msg_table[239], txt_time(abs(timer_data->tick - timer_data2->tick) / 1000)); // Game time: After, the game will be in daylight for %s.
			clif_displaymessage(fd, temp);
			sprintf(temp, msg_table[238], txt_time(timer_data->interval / 1000)); // Game time: A day cycle has a normal duration of %s.
			clif_displaymessage(fd, temp);
		}
	}

	return 0;
}

/*==========================================
 * @chardelitem <item_name_or_ID> <quantity> <player> (by [Yor]
 * removes <quantity> item from a character
 * item can be equiped or not.
 * Inspired from a old command created by RoVeRT
 *------------------------------------------
 */
int atcommand_chardelitem(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char item_name[100];
	int i, number = 0, item_id, item_position, count;
	struct item_data *item_data;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(item_name, '\0', sizeof(item_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message || sscanf(message, "%s %d %99[^\n]", item_name, &number, atcmd_player_name) < 3 || number < 1) {
		clif_displaymessage(fd, "Please, enter an item name/id, a quantity and a player name (usage: @chardelitem <item_name_or_ID> <quantity> <player>).");
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id > 500) {
		if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
			if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same level
				item_position = pc_search_inventory(pl_sd, item_id);
				if (item_position >= 0) {
					count = 0;
					for(i = 0; i < number && item_position >= 0; i++) {
						pc_delitem(pl_sd, item_position, 1, 0);
						count++;
						item_position = pc_search_inventory(pl_sd, item_id); // for next loop
					}
					sprintf(atcmd_output, msg_table[113], count); // %d item(s) removed by a GM.
					clif_displaymessage(pl_sd->fd, atcmd_output);
					if (number == count)
						sprintf(atcmd_output, msg_table[114], count); // %d item(s) removed from the player.
					else
						sprintf(atcmd_output, msg_table[115], count, count, number); // %d item(s) removed. Player had only %d on %d items.
					clif_displaymessage(fd, atcmd_output);
				} else {
					clif_displaymessage(fd, msg_table[116]); // Character does not have the item.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[3]); // Character not found.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[19]); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * @jail <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------
 */
int atcommand_jail(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int x, y;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @jail <char_name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can jail only lower or same GM
			switch(rand() % 2) {
			case 0:
				x = 24;
				y = 75;
				break;
			default:
				x = 49;
				y = 75;
				break;
			}
			if (pc_setpos(pl_sd, "sec_pri.gat", x, y, 3) == 0) {
				pc_setsavepoint(pl_sd, "sec_pri.gat", x, y); // Save Char Respawn Point in the jail room [Lupus]
				clif_displaymessage(pl_sd->fd, msg_table[117]); // GM has send you in jails.
				clif_displaymessage(fd, msg_table[118]); // Player warped in jails.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @unjail/@discharge <char_name> by [Yor]
 * Special warp! No check with nowarp and nowarpto flag
 *------------------------------------------
 */
int atcommand_unjail(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @unjail/@discharge <char_name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can jail only lower or same GM
			if (pl_sd->bl.m != map_mapname2mapid("sec_pri.gat")) {
				clif_displaymessage(fd, msg_table[119]); // This player is not in jails.
				return -1;
			} else if (pc_setpos(pl_sd, "prontera.gat", 0, 0, 3) == 0) { //old coords: 156,191
				pc_setsavepoint(pl_sd, "prontera.gat", 0, 0); // Save char respawn point in Prontera
				clif_displaymessage(pl_sd->fd, msg_table[120]); // GM has discharge you.
				clif_displaymessage(fd, msg_table[121]); // Player warped to Prontera.
			} else {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @disguise <mob_id> by [Valaris] (simplified by [Yor])
 *------------------------------------------
 */
int atcommand_disguise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int mob_id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguise <monster_name_or_monster_ID>).");
		return -1;
	}

	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

	if ((mob_id >=  46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 718) || // NPC
	    (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || // NPC
	    (mob_id >= 813 && mob_id <= 834) || // NPC
	    (mob_id > 1000 && mob_id < 1582)) { // monsters
		if (pc_isriding(sd)) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
			clif_displaymessage(fd, msg_table[227]); // Cannot wear disguise while riding a Peco.
			return -1;
		}
		sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
		sd->disguise = mob_id;
		pc_setpos(sd, sd->mapname, sd->bl.x, sd->bl.y, 3);
		clif_displaymessage(fd, msg_table[122]); // Disguise applied.
	} else {
		clif_displaymessage(fd, msg_table[123]); // Monster/NPC name/id hasn't been found.
		return -1;
	}

	return 0;
}

/*==========================================
 * DisguiseAll
 *------------------------------------------
 */

int atcommand_disguiseall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int mob_id=0, i=0;
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @disguiseall <monster_name_or_monster_ID>).");
		return -1;
	}

	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(message);

	if ((mob_id >=  46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 718) || // NPC
	    (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || // NPC
	    (mob_id >= 813 && mob_id <= 834) || // NPC
	    (mob_id > 1000 && mob_id < 1582)) { // monsters
		for(i=0; i < fd_max; i++) {
			if(session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				if(pc_isriding(pl_sd)) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
					clif_displaymessage(fd, msg_table[227]); // Cannot wear disguise while riding a Peco.
				} else {
					pl_sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
					pl_sd->disguise = mob_id;
					pc_setpos(pl_sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
				}
			}
		}
		clif_displaymessage(fd, msg_table[122]); // Disguise applied.
	} else {
		return -1;
	}

	return 0;
}

/*==========================================
 * @undisguise by [Yor]
 *------------------------------------------
 */
int atcommand_undisguise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->disguise) {
		clif_clearchar(&sd->bl, 9);
		sd->disguise = 0;
		pc_setpos(sd, sd->mapname, sd->bl.x, sd->bl.y, 3);
		clif_displaymessage(fd, msg_table[124]); // Undisguise applied.
	} else {
		clif_displaymessage(fd, msg_table[125]); // You're not disguised.
		return -1;
	}

	return 0;
}

/*==========================================
 * UndisguiseAll
 *------------------------------------------
 */
int atcommand_undisguiseall(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int i;
	nullpo_retr(-1, sd);

	for(i=0; i < fd_max; i++) {
		if(session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth && pl_sd->disguise) {
			clif_clearchar(&pl_sd->bl, 9);
			pl_sd->disguise = 0;
			pc_setpos(pl_sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
		}
	}
	clif_displaymessage(fd, msg_table[124]); // Undisguise applied.

	return 0;
}

/*==========================================
 * @broadcast by [Valaris]
 *------------------------------------------
 */
int atcommand_broadcast(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @broadcast <message>).");
		return -1;
	}

	sprintf(atcmd_output, "%s : %s", sd->status.name, message);
	intif_GMmessage(atcmd_output, strlen(atcmd_output) + 1, 0);

	return 0;
}

/*==========================================
 * @localbroadcast by [Valaris]
 *------------------------------------------
 */
int atcommand_localbroadcast(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	memset(atcmd_output, '\0', sizeof(atcmd_output));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a message (usage: @localbroadcast <message>).");
		return -1;
	}

	sprintf(atcmd_output, "%s : %s", sd->status.name, message);

	clif_GMmessage(&sd->bl, atcmd_output, strlen(atcmd_output) + 1, 1); // 1: ALL_SAMEMAP

	return 0;
}

/*==========================================
 * @chardisguise <mob_id> <character> by Kalaspuff (based off Valaris' and Yor's work)
 *------------------------------------------
 */
int atcommand_chardisguise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int mob_id;
	char mob_name[100];
	struct map_session_data* pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(mob_name, '\0', sizeof(mob_name));

	if (!message || !*message || sscanf(message, "%s %99[^\n]", mob_name, atcmd_player_name) < 2) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id and a player name (usage: @chardisguise <monster_name_or_monster_ID> <char name>).");
		return -1;
	}

	if ((mob_id = mobdb_searchname(mob_name)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = atoi(mob_name);

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can disguise only lower or same level
			if ((mob_id >=  46 && mob_id <= 125) || (mob_id >= 700 && mob_id <= 718) || // NPC
			    (mob_id >= 721 && mob_id <= 755) || (mob_id >= 757 && mob_id <= 811) || // NPC
			    (mob_id >= 813 && mob_id <= 834) || // NPC
			    (mob_id > 1000 && mob_id < 1521)) { // monsters
				if (pc_isriding(pl_sd)) { // temporary prevention of crash caused by peco + disguise, will look into a better solution [Valaris]
					clif_displaymessage(fd, msg_table[228]); // Character cannot wear disguise while riding a Peco.
					return -1;
				}
				pl_sd->disguiseflag = 1; // set to override items with disguise script [Valaris]
				pl_sd->disguise = mob_id;
				pc_setpos(pl_sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
				clif_displaymessage(fd, msg_table[140]); // Character's disguise applied.
			} else {
				clif_displaymessage(fd, msg_table[123]); // Monster/NPC name/id hasn't been found.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @charundisguise <character> by Kalaspuff (based off Yor's work)
 *------------------------------------------
 */
int atcommand_charundisguise(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data* pl_sd;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charundisguise <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can undisguise only lower or same level
			if (pl_sd->disguise) {
				clif_clearchar(&pl_sd->bl, 9);
				pl_sd->disguise = 0;
				pc_setpos(pl_sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
				clif_displaymessage(fd, msg_table[141]); // Character's undisguise applied.
			} else {
				clif_displaymessage(fd, msg_table[142]); // Character is not disguised.
				return -1;
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @email <actual@email> <new@email> by [Yor]
 *------------------------------------------
 */
int atcommand_email(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char actual_email[100];
	char new_email[100];
	nullpo_retr(-1, sd);

	memset(actual_email, '\0', sizeof(actual_email));
	memset(new_email, '\0', sizeof(new_email));

	if (!message || !*message || sscanf(message, "%99s %99s", actual_email, new_email) < 2) {
		clif_displaymessage(fd, "Please enter 2 emails (usage: @email <actual@email> <new@email>).");
		return -1;
	}

	if (e_mail_check(actual_email) == 0) {
		clif_displaymessage(fd, msg_table[144]); // Invalid actual email. If you have default e-mail, give a@a.com.
		return -1;
	} else if (e_mail_check(new_email) == 0) {
		clif_displaymessage(fd, msg_table[145]); // Invalid new email. Please enter a real e-mail.
		return -1;
	} else if (strcmpi(new_email, "a@a.com") == 0) {
		clif_displaymessage(fd, msg_table[146]); // New email must be a real e-mail.
		return -1;
	} else if (strcmpi(actual_email, new_email) == 0) {
		clif_displaymessage(fd, msg_table[147]); // New email must be different of the actual e-mail.
		return -1;
	} else {
		chrif_changeemail(sd->status.account_id, actual_email, new_email);
		clif_displaymessage(fd, msg_table[148]); // Information sended to login-server via char-server.
	}

	return 0;
}

/*==========================================
 *@effect
 *------------------------------------------
 */
int atcommand_effect(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	int type = 0, flag = 0, i;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d", &type,&flag) < 2) {
		clif_displaymessage(fd, "Please, enter at least a option (usage: @effect <type+>).");
		return -1;
	}
	if(flag <=0){
		clif_specialeffect(&sd->bl, type, flag);
		clif_displaymessage(fd, msg_table[229]); // Your effect has changed.
	}
	else{
		for (i = 0; i < fd_max; i++) {
			if (session[i] && (pl_sd = (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth) {
				clif_specialeffect(&pl_sd->bl, type, flag);
				clif_displaymessage(pl_sd->fd, msg_table[229]); // Your effect has changed.
			}
		}
	}

	return 0;
}

/*==========================================
 * @charcartlist <character>: Displays the items list of a player's cart.
 *------------------------------------------
 */
int
atcommand_character_cart_list(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char outputtmp[200];
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	nullpo_retr(-1, sd);

	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charitemlist <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_CART; i++) {
				if (pl_sd->status.cart[i].nameid > 0 && (item_data = itemdb_search(pl_sd->status.cart[i].nameid)) != NULL) {
					counter = counter + pl_sd->status.cart[i].amount;
					count++;
					if (count == 1) {
						sprintf(atcmd_output, "------ Cart items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, atcmd_output);
					}
					if (pl_sd->status.cart[i].refine)
						sprintf(atcmd_output, "%d %s %+d (%s %+d, id: %d)", pl_sd->status.cart[i].amount, item_data->name, pl_sd->status.cart[i].refine, item_data->jname, pl_sd->status.cart[i].refine, pl_sd->status.cart[i].nameid);
					else
						sprintf(atcmd_output, "%d %s (%s, id: %d)", pl_sd->status.cart[i].amount, item_data->name, item_data->jname, pl_sd->status.cart[i].nameid);
					clif_displaymessage(fd, atcmd_output);
					memset(atcmd_output, '\0', sizeof(atcmd_output));
					counter2 = 0;
					for (j = 0; j < item_data->slot; j++) {
						if (pl_sd->status.cart[i].card[j]) {
							if ( (item_temp = itemdb_search(pl_sd->status.cart[i].card[j])) != NULL) {
								if (atcmd_output[0] == '\0')
									sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								else
									sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								strcat(atcmd_output, outputtmp);
							}
						}
					}
					if (atcmd_output[0] != '\0') {
						atcmd_output[strlen(atcmd_output) - 2] = ')';
						atcmd_output[strlen(atcmd_output) - 1] = '\0';
						clif_displaymessage(fd, atcmd_output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found in the cart of this player.");
			else {
				sprintf(atcmd_output, "%d item(s) found in %d kind(s) of items.", counter, count);
				clif_displaymessage(fd, atcmd_output);
			}
		} else {
			clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @killer by MouseJstr
 * enable killing players even when not in pvp
 *------------------------------------------
 */
int
atcommand_killer(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	sd->special_state.killer = !sd->special_state.killer;

	if(sd->special_state.killer)
	  clif_displaymessage(fd, msg_table[241]);
        else
	  clif_displaymessage(fd, msg_table[242]);

	return 0;
}

/*==========================================
 * @killable by MouseJstr
 * enable other people killing you
 *------------------------------------------
 */
int
atcommand_killable(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	sd->special_state.killable = !sd->special_state.killable;

	if(sd->special_state.killable)
	  clif_displaymessage(fd, msg_table[242]);
        else
	  clif_displaymessage(fd, msg_table[241]);

	return 0;
}

/*==========================================
 * @charkillable by MouseJstr
 * enable another player to be killed
 *------------------------------------------
 */
int
atcommand_charkillable(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;

	if((pl_sd=map_nick2sd((char *) message)) == NULL)
                return -1;

	pl_sd->special_state.killable = !pl_sd->special_state.killable;

	if(pl_sd->special_state.killable)
	  clif_displaymessage(fd, "The player is now killable");
        else
	  clif_displaymessage(fd, "The player is no longer killable");

	return 0;
}


/*==========================================
 * @skillon by MouseJstr
 * turn skills on for the map
 *------------------------------------------
 */
int
atcommand_skillon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.noskill = 0;
	clif_displaymessage(fd, msg_table[244]);
	return 0;
}

/*==========================================
 * @skilloff by MouseJstr
 * Turn skills off on the map
 *------------------------------------------
 */
int
atcommand_skilloff(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.noskill = 1;
	clif_displaymessage(fd, msg_table[243]);
	return 0;
}

/*==========================================
 * @npcmove by MouseJstr
 *
 * move a npc
 *------------------------------------------
 */
int
atcommand_npcmove(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int x = 0, y = 0;
	struct npc_data *nd = 0;
	nullpo_retr(-1, sd);


	if (!message || !*message)
		return -1;

	memset(atcmd_player_name, '\0', sizeof atcmd_player_name);

	if (sscanf(message, "%d %d %99[^\n]", &x, &y, atcmd_player_name) < 3) {
		clif_displaymessage(fd, "Usage: @npcmove <X> <Y> <npc_name>");
		return -1;
	}

	nullpo_retr(-1, (nd = npc_name2id(atcmd_player_name)));

	npc_enable(atcmd_player_name, 0);
	nd->bl.x = x;
	nd->bl.y = y;
	npc_enable(atcmd_player_name, 1);

	return 0;
}

/*==========================================
 * @addwarp by MouseJstr
 *
 * Create a new static warp point.
 *------------------------------------------
 */
int
atcommand_addwarp(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char w1[64], w3[64], w4[64];
	int x,y,ret;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;

	if (sscanf(message, "%99s %d %d[^\n]", atcmd_player_name, &x, &y ) < 3)
		return -1;

	sprintf(w1,"%s,%d,%d", sd->mapname, sd->bl.x, sd->bl.y);
	sprintf(w3,"%s%d%d%d%d", atcmd_player_name,sd->bl.x, sd->bl.y, x, y);
	sprintf(w4,"1,1,%s.gat,%d,%d", atcmd_player_name, x, y);

	ret = npc_parse_warp(w1, "warp", w3, w4);

	sprintf(atcmd_output, "New warp NPC => %s",w3);

	clif_displaymessage(fd, atcmd_output);

	return ret;
}

/*==========================================
 * @follow by [MouseJstr]
 *
 * Follow a player .. staying no more then 5 spaces away
 *------------------------------------------
 */
int
atcommand_follow(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if((pl_sd=map_nick2sd((char *) message)) != NULL)
		pc_follow(sd, pl_sd->bl.id);
	else
		return 1;
	return 0;
}


/*==========================================
 * @dropall by [MouseJstr]
 *
 * Drop all your possession on the ground
 *------------------------------------------
 */
int
atcommand_dropall(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);
	for (i = 0; i < MAX_INVENTORY; i++) {
	if (sd->status.inventory[i].amount) {
		if(sd->status.inventory[i].equip != 0)
			pc_unequipitem(sd, i, 3);
		pc_dropitem(sd,  i, sd->status.inventory[i].amount);
		}
	}
	return 0;
}
/*==========================================
 * @chardropall by [MouseJstr]
 *
 * Throw all the characters possessions on the ground.  Normally
 * done in response to them being disrespectful of a GM
 *------------------------------------------
 */
int
atcommand_chardropall(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if((pl_sd=map_nick2sd((char *) message)) == NULL)
		return -1;
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (pl_sd->status.inventory[i].amount) {
			if(pl_sd->status.inventory[i].equip != 0)
				pc_unequipitem(pl_sd, i, 3);
			pc_dropitem(pl_sd,  i, pl_sd->status.inventory[i].amount);
		}
	}

	clif_displaymessage(pl_sd->fd, "Ever play 52 card pickup?");
	clif_displaymessage(fd, "It is done");
	//clif_displaymessage(fd, "It is offical.. your a jerk");

	return 0;
}
/*==========================================
 * @storeall by [MouseJstr]
 *
 * Put everything into storage to simplify your inventory to make
 * debugging easie
 *------------------------------------------
 */
int
atcommand_storeall(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	nullpo_retr(-1, sd);
	if (storage_storageopen(sd) == 1) {
		clif_displaymessage(fd, "run this command again..");
		return 0;
	}
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (sd->status.inventory[i].amount) {
			if(sd->status.inventory[i].equip != 0)
				pc_unequipitem(sd, i, 3);
			storage_storageadd(sd,  i, sd->status.inventory[i].amount);
		}
	}
	storage_storageclose(sd);

	clif_displaymessage(fd, "It is done");
	return 0;
}
/*==========================================
 * @charstoreall by [MouseJstr]
 *
 * A way to screw with players who piss you off
 *------------------------------------------
 */
int
atcommand_charstoreall(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if((pl_sd=map_nick2sd((char *) message)) == NULL)
		return -1;

	if (storage_storageopen(pl_sd) == 1) {
		clif_displaymessage(fd, "Had to open the characters storage window...");
		clif_displaymessage(fd, "run this command again..");
		return 0;
	}
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (pl_sd->status.inventory[i].amount) {
			if(pl_sd->status.inventory[i].equip != 0)
				pc_unequipitem(pl_sd, i, 3);
			storage_storageadd(pl_sd,  i, sd->status.inventory[i].amount);
		}
	}
	storage_storageclose(pl_sd);

	clif_displaymessage(pl_sd->fd, "Everything you own has been put away for safe keeping.");
	clif_displaymessage(pl_sd->fd, "go to the nearest kafka to retrieve it..");
	clif_displaymessage(pl_sd->fd, "   -- the management");

	clif_displaymessage(fd, "It is done");

	return 0;
}
/*==========================================
 * @skillid by [MouseJstr]
 *
 * lookup a skill by name
 *------------------------------------------
 */
int
atcommand_skillid(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
        int skillen = 0, idx = 0;
	nullpo_retr(-1, sd);
	if (!message || !*message)
		return -1;
        skillen = strlen(message);
        while (skill_names[idx].id != 0) {
            if ((strnicmp(skill_names[idx].name, message, skillen) == 0) ||
            (strnicmp(skill_names[idx].desc, message, skillen) == 0)) {
               sprintf(atcmd_output, "skill %d: %s", skill_names[idx].id, skill_names[idx].desc);
	       clif_displaymessage(fd, atcmd_output);
            }
            idx++;
        }
        return 0;
}

/*==========================================
 * @useskill by [MouseJstr]
 *
 * A way of using skills without having to find them in the skills menu
 *------------------------------------------
 */
int
atcommand_useskill(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum;
	int skilllv;
	int inf;
	char target[255];
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if(sscanf(message, "%d %d %99[^\n]", &skillnum, &skilllv, target) != 3) {
		clif_displaymessage(fd, "Usage: @useskill <skillnum> <skillv> <target>");
		return -1;
	}
	if((pl_sd=map_nick2sd(target)) == NULL) {
		return -1;
	}

	inf = skill_get_inf(skillnum);

	if ((inf == 2) || (inf == 1))
		skill_use_pos(sd, pl_sd->bl.x, pl_sd->bl.y, skillnum, skilllv);
	else
		skill_use_id(sd, pl_sd->bl.id, skillnum, skilllv);

	return 0;
}

/*==========================================
 * @skilltree by [MouseJstr]
 *
 * prints the skill tree for a player required to get to a skill
 *------------------------------------------
 */
int
atcommand_skilltree(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  struct map_session_data *pl_sd = NULL;
  int skillnum, skillidx = -1;
  int meets = 1, j, c=0, s=0;
  struct pc_base_job s_class;
  char target[255], *tbl;
  struct skill_tree_entry *ent;
	nullpo_retr(-1, sd);

  if (!message || !*message)
    return -1;

  if(sscanf(message, "%d %[^\r\n]", &skillnum, target) != 2) {
    clif_displaymessage(fd, "Usage: @skilltree <skillnum> <target>");
    return -1;
  }
  if((pl_sd=map_nick2sd(target)) == NULL)
    return -1;

  s_class = pc_calc_base_job(pl_sd->status.class_);
  c = s_class.job;
  s = s_class.upper;

  c = pc_calc_skilltree_normalize_job(c, pl_sd);

  tbl = job_name(c);

  sprintf(atcmd_output, "Player is using %s %s skill tree (%d basic points)",
	  s_class.upper ? "upper" : "lower",
	  tbl, pc_checkskill(pl_sd, 1));
  clif_displaymessage(fd, atcmd_output);

  for (j = 0; skill_tree[s][c][j].id != 0; j++) {
    if (skill_tree[s][c][j].id == skillnum) {
      skillidx = j;
      break;
    }
  }

  if (skillidx == -1) {
    sprintf(atcmd_output, "I do not believe the player can use that skill");
    clif_displaymessage(fd, atcmd_output);
    return 0;
  }

  ent = &skill_tree[s][c][skillidx];

  for(j=0;j<5;j++)
    if( ent->need[j].id &&
	pc_checkskill(sd,ent->need[j].id) < ent->need[j].lv)
      {
	int idx = 0;
	char *desc;
        while (skill_names[idx].id != 0 && skill_names[idx].id != ent->need[j].id)
		idx++;
	if (skill_names[idx].id == 0)
		desc = "Unknown skill";
	else
		desc = skill_names[idx].desc;
	sprintf(atcmd_output, "player requires level %d of skill %s",
		ent->need[j].lv,  desc);
	clif_displaymessage(fd, atcmd_output);
	meets = 0;
      }

  if (meets == 1) {
    sprintf(atcmd_output, "I believe the player meets all the requirements for that skill");
    clif_displaymessage(fd, atcmd_output);
  }

  return 0;
}

/*==========================================
 * @marry by [MouseJstr], fixed by Lupus
 *
 * Marry two players
 *------------------------------------------
 */
int
atcommand_marry(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  struct map_session_data *pl_sd1 = NULL;
  struct map_session_data *pl_sd2 = NULL;
  char player1[255], player2[255];

  nullpo_retr(-1, sd);

  if (!message || !*message || sscanf(message, "%[^,],%[^\r\n]", player1, player2) != 2) {
    clif_displaymessage(fd, "Usage: @marry <player1>,<player2>.");
    return -1;
  }

  if((pl_sd1=map_nick2sd((char *) player1)) == NULL) {
    sprintf(player2, "Cannot find player '%s' online", player1);
    clif_displaymessage(fd, player2);
    return -1;
  }

  if((pl_sd2=map_nick2sd((char *) player2)) == NULL) {
    sprintf(player1, "Cannot find player '%s' online", player2);
    clif_displaymessage(fd, player1);
    return -1;
  }

  if (pc_marriage(pl_sd1, pl_sd2) == 0) {
	clif_displaymessage(fd, "They are married.. wish them well");
	clif_wedding_effect(&sd->bl);	//wedding effect and music [Lupus]
	return 0;
  }
  return -1;
}

/*==========================================
 * @divorce by [MouseJstr], fixed by [Lupus]
 *
 * divorce two players 
 *------------------------------------------
 */
int
atcommand_divorce(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  struct map_session_data *pl_sd = NULL;

  nullpo_retr(-1, sd);

  if (!message || !*message || sscanf(message, "%[^\r\n]", atcmd_player_name) != 1) {
    clif_displaymessage(fd, "Usage: @divorce <player>.");
    return -1;
  }

  if((pl_sd=map_nick2sd((char *) atcmd_player_name)) != NULL) {
	if (pc_divorce(pl_sd) != 0) {
		sprintf(atcmd_output, "The divorce has failed.. Cannot find player '%s' or his(her) partner online.", atcmd_player_name);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	} else {
		sprintf(atcmd_output, "'%s' and his(her) partner are now divorced.", atcmd_player_name);
		clif_displaymessage(fd, atcmd_output);
		return 0;
	}
  }
  sprintf(atcmd_output, "Cannot find player '%s' online", atcmd_player_name);
  clif_displaymessage(fd, atcmd_output);
  return -1;
}

/*==========================================
 * @rings by [MouseJstr]
 *
 * Give two players rings
 *------------------------------------------
 */
int
atcommand_rings(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  struct item item_tmp;
  int flag;

  memset(&item_tmp, 0, sizeof(item_tmp));

  item_tmp.nameid = 2634;
  item_tmp.identify = 1;

  if ((flag = pc_additem((struct map_session_data*)sd, &item_tmp, 1)))
    clif_additem((struct map_session_data*)sd, 0, 0, flag);

  item_tmp.nameid = 2635;
  item_tmp.identify = 1;
  if ((flag = pc_additem((struct map_session_data*)sd, &item_tmp, 1)))
    clif_additem((struct map_session_data*)sd, 0, 0, flag);

  clif_displaymessage(fd, "You have rings!  Give them to the lovers.");

  return 0;
}

#ifdef DMALLOC
unsigned long dmark_;
int
atcommand_dmstart(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  dmark_ = dmalloc_mark();

  clif_displaymessage(fd, "debug mark set");

  return 0;
}

int
atcommand_dmtick(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
  dmalloc_log_changed ( dmark_, 1, 0, 1 ) ;
  dmark_ = dmalloc_mark();
  clif_displaymessage(fd, "malloc changes logged");

  return 0;
}
#endif

/*==========================================
 * @grind by [MouseJstr]
 *------------------------------------------
 */
int
atcommand_grind(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int skillnum;
	int inf;
	char target[255];
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if(sscanf(message, "%s", target) != 1) {
		clif_displaymessage(fd, "Usage: @grind <target>");
		return -1;
	}
	if((pl_sd=map_nick2sd(target)) == NULL)
		return -1;

	for (skillnum = 1; skillnum < 500; skillnum++) {
		sd->status.sp = sd->status.max_sp;
		atcommand_alive(fd, sd, command, message);

		inf = skill_get_inf(skillnum);

		if ((inf == 2) || (inf == 1))
			skill_use_pos(sd, pl_sd->bl.x+5, pl_sd->bl.y+5, skillnum, 1);
		else
			skill_use_id(sd, pl_sd->bl.id, skillnum, 1);
	}

	return 0;
}

/*==========================================
 * @grind2 by [MouseJstr]
 *------------------------------------------
 */
int
atcommand_grind2(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i, x, y, id;

	for (i =  1000; i <2000; i++) {
		x = sd->bl.x + (rand() % 10 - 5);
		y = sd->bl.y + (rand() % 10 - 5);
		id = mob_once_spawn(sd, "this", x, y, "--ja--", i, 1, "");
	}

	return 0;
}

/*==========================================
 * @changelook by [Celest]
 *------------------------------------------
 */
int
atcommand_changelook(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i, j = 0, k = 0;
	int pos[6] = { LOOK_HEAD_TOP,LOOK_HEAD_MID,LOOK_HEAD_BOTTOM,LOOK_WEAPON,LOOK_SHIELD,LOOK_SHOES };

	if((i = sscanf(message, "%d %d", &j, &k)) < 1) {
		clif_displaymessage(fd, "Usage: @changelook [<position>] <view id> -- [] = optional");
		clif_displaymessage(fd, "Position: 1-Top 2-Middle 3-Bottom 4-Weapon 5-Shield");
		return -1;
	} else if (i == 2) {
		if (j < 1) j = 1;
		else if (j > 6) j = 6;	// 6 = Shoes - for beta clients only perhaps
		j = pos[j - 1];
	} else if (i == 1) {	// position not defined, use HEAD_TOP as default
		k = j;	// swap
		j = LOOK_HEAD_TOP;
	}

	clif_changelook(&sd->bl,j,k);

	return 0;
}

/*==========================================
 *Turns on/off AutoLoot for a specific player
 *------------------------------------------
 *by Upa-Kun
 */
int
atcommand_autoloot(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	if (sd->autoloot) 
	{
		sd->autoloot = 0;
		clif_displaymessage(fd, "Autoloot is now off.");
	}
	else 
	{
		sd->autoloot = 1;
		clif_displaymessage(fd, "Autoloot is now on.");
	}
	return 0;  
}   


/*==========================================
 * It is made to rain.
 *------------------------------------------
 */
int
atcommand_rain(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effno = 0;
	nullpo_retr(-1, sd);
	effno = 161;
	if (map[sd->bl.m].flag.rain) {
		map[sd->bl.m].flag.rain=0;
		clif_displaymessage(fd, "The rain has stopped.");
	} else {
		map[sd->bl.m].flag.rain=1;
		clif_specialeffect(&sd->bl,effno,2);
		clif_displaymessage(fd, "It is made to rain.");
	}
	return 0;
}
/*==========================================
 * It is made to snow.
 *------------------------------------------
 */
int
atcommand_snow(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effno = 0;
	effno = 162;
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.snow) {
		map[sd->bl.m].flag.snow=0;
		clif_displaymessage(fd, "Snow has stopped falling.");
	} else {
		map[sd->bl.m].flag.snow=1;
		clif_specialeffect(&sd->bl,effno,2);
		clif_displaymessage(fd, "It is made to snow.");
	}

	return 0;
}

/*==========================================
 * Cherry tree snowstorm is made to fall. (Sakura)
 *------------------------------------------
 */
int
atcommand_sakura(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effno = 0;
	effno = 163;
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.sakura) {
		map[sd->bl.m].flag.sakura=0;
		clif_displaymessage(fd, "Cherry tree leaves is made to fall.");
	} else {
		map[sd->bl.m].flag.sakura=1;
		clif_specialeffect(&sd->bl,effno,2);
		clif_displaymessage(fd, "Cherry tree leaves is made to fall.");
	}
	return 0;
}

/*==========================================
 * Fog hangs over.
 *------------------------------------------
 */
int
atcommand_fog(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effno = 0;
	effno = 233;
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.fog) {
		map[sd->bl.m].flag.fog=0;
		clif_displaymessage(fd, "The fog has gone.");
	} else {
		map[sd->bl.m].flag.fog=1;
		clif_specialeffect(&sd->bl,effno,2);
		clif_displaymessage(fd, "Fog hangs over.");
	}

	return 0;
}

/*==========================================
 * Fallen leaves fall.
 *------------------------------------------
 */
int
atcommand_leaves(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effno = 0;
	effno = 333;
	nullpo_retr(-1, sd);
	if (map[sd->bl.m].flag.leaves) {
		map[sd->bl.m].flag.leaves=0;
		clif_displaymessage(fd, "Leaves no longer fall.");
	} else {
		map[sd->bl.m].flag.leaves=1;
		clif_specialeffect(&sd->bl,effno,2);
		clif_displaymessage(fd, "Fallen leaves fall.");
	}

	return 0;
}

/*==========================================
 * Clearing Weather Effects by Dexity
 *------------------------------------------
 */
int
atcommand_clearweather(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
	//int effno = 0;
	nullpo_retr(-1, sd);
	map[sd->bl.m].flag.rain=0;
	map[sd->bl.m].flag.snow=0;
	map[sd->bl.m].flag.sakura=0;
	map[sd->bl.m].flag.fog=0;
	map[sd->bl.m].flag.leaves=0;
	//clif_specialeffect(&sd->bl,effno,2); // not required. [celest]
	return 0;
}

/*===============================================================
 * Sound Command - plays a sound for everyone! [Codemaster]
 *---------------------------------------------------------------
 */
int
atcommand_sound(
	const int fd, struct map_session_data *sd,
	const char *command, const char *message)
{
	char sound_file[100];

	if(!message || !*message || sscanf(message, "%99[^\n]", sound_file) < 1) {
		clif_displaymessage(fd, "Please, enter a sound filename. (usage: @sound <filename>)");
		return -1;
	}

	memset(sound_file, '\0', sizeof(sound_file));
	if(sscanf(message, "%99[^\n]", sound_file) < 1)
		return -1;

	if(strstr(sound_file, ".wav") == NULL)
		strcat(sound_file, ".wav");

	clif_soundeffectall(&sd->bl, sound_file,0);

	return 0;
}

/*==========================================
 * 	MOB Search
 *------------------------------------------
 */
int
atcommand_mobsearch(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char mob_name[100];
	int mob_id,map_id = 0;

	nullpo_retr(-1, sd);

	if (sscanf(message, "%99[^\n]", mob_name) < 0)
		return -1;

	if ((mob_id = atoi(mob_name)) == 0)
		 mob_id = mobdb_searchname(mob_name);
	if(mob_id !=-1 && (mob_id <= 1000 || mob_id >= 2000)){
		snprintf(atcmd_output, sizeof atcmd_output, "Invalid mob id %s!",mob_name);
		clif_displaymessage(fd, atcmd_output);
		return 0;
	}
	if(mob_id == atoi(mob_name) && mob_db[mob_id].jname)
				strcpy(mob_name,mob_db[mob_id].jname);	// --ja--
//				strcpy(mob_name,mob_db[mob_id].name);	// --en--

	map_id = sd->bl.m;

	snprintf(atcmd_output, sizeof atcmd_output, "Mob Search... %s %s",
		mob_name, sd->mapname);
	clif_displaymessage(fd, atcmd_output);

	map_foreachinarea(atmobsearch_sub, map_id, 0, 0,
		map[map_id].xs, map[map_id].ys, BL_MOB, mob_id, fd);

	atmobsearch_sub(&sd->bl,0);		// î‘çÜÉäÉZÉbÉg

	return 0;
}
/*==========================================
 * ÉhÉçÉbÉvÉAÉCÉeÉÄÇÃë|èú
 *------------------------------------------
 */
int
atcommand_cleanmap(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i=0;
	map_foreachinarea(atcommand_cleanmap_sub,sd->bl.m,
					  sd->bl.x-AREA_SIZE*2,sd->bl.y-AREA_SIZE*2,
					  sd->bl.x+AREA_SIZE*2,sd->bl.y+AREA_SIZE*2,
					  BL_ITEM,sd,&i);
	clif_displaymessage(fd, "All dropped items have been cleaned up.");
	return 0;
}

/*==========================================
 * NPC/PETÇ…òbÇ≥ÇπÇÈ
 *------------------------------------------
 */
int
atcommand_npctalk(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char name[100],mes[100];
	struct npc_data *nd;

	if (sscanf(message, "%s %99[^\n]", name, mes) < 2)
		return -1;

	if (!(nd = npc_name2id(name)))
		return -1;
	
	clif_message(&nd->bl, mes);
	return 0;
}
int
atcommand_pettalk(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char mes[100],temp[100];
	struct pet_data *pd;

	nullpo_retr(-1, sd);

	if(!sd->status.pet_id || !(pd=sd->pd))
		return -1;

	if (sscanf(message, "%99[^\n]", mes) < 1)
		return -1;

	snprintf(temp, sizeof temp ,"%s : %s",sd->pet.name,mes);
	clif_message(&pd->bl, temp);

	return 0;
}

/*==========================================
 * @users
 * ÉTÅ[ÉoÅ[ì‡ÇÃêlêîÉ}ÉbÉvÇï\é¶Ç≥ÇπÇÈ
 * éËî≤Ç´ÇÃÇΩÇﬂâòÇ≠Ç»Ç¡ÇƒÇ¢ÇÈÇÃÇÕédólÇ≈Ç∑ÅB
 *------------------------------------------
 */

static struct dbt *users_db;
static int users_all;

static int atcommand_users_sub1(struct map_session_data* sd,va_list va) {
	int users = (int)strdb_search(users_db,sd->mapname) + 1;
	users_all++;
	strdb_insert(users_db,sd->mapname,(void *)users);
	return 0;
}

static int atcommand_users_sub2(void* key,void* val,va_list va) {
	char buf[256];
	struct map_session_data* sd = va_arg(va,struct map_session_data*);
	sprintf(buf,"%s : %d (%d%%)",(char *)key,(int)val,(int)val * 100 / users_all);
	clif_displaymessage(sd->fd,buf);
	return 0;
}

int
atcommand_users(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char buf[256];
	users_all = 0;
	users_db = strdb_init(24);
	clif_foreachclient(atcommand_users_sub1);
	strdb_foreach(users_db,atcommand_users_sub2,sd);
	sprintf(buf,"all : %d",users_all);
	clif_displaymessage(fd,buf);
	strdb_final(users_db,NULL);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int
atcommand_summon(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char name[100];
	int mob_id = 0;
	int x = 0;
	int y = 0;
	int id = 0;
	struct mob_data *md;
	unsigned int tick=gettick();

	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if (sscanf(message, "%99s", name) < 1)
		return -1;

	if ((mob_id = atoi(name)) == 0)
		mob_id = mobdb_searchname(name);
	if(mob_id == 0)
		return -1;

	x = sd->bl.x + (rand() % 10 - 5);
	y = sd->bl.y + (rand() % 10 - 5);

	id = mob_once_spawn(sd,"this", x, y, "--ja--", mob_id, 1, "");
	if((md=(struct mob_data *)map_id2bl(id))){
		md->master_id=sd->bl.id;
		md->state.special_mob_ai=1;
		md->mode=mob_db[md->class_].mode|0x04;
		md->deletetimer=add_timer(tick+60000,mob_timer_delete,id,0);
		clif_misceffect2(&md->bl,344);
	}
	clif_skill_poseffect(&sd->bl,AM_CALLHOMUN,1,x,y,tick);

	return 0;
}


/*==========================================
 * @adjcmdlvl by [MouseJstr]
 *
 * Temp adjust the GM level required to use a GM command
 *
 * Used during beta testing to allow players to use GM commands
 * for short periods of time
 *------------------------------------------
 */
int
atcommand_adjcmdlvl(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
    int i, newlev;
    char cmd[100];
	nullpo_retr(-1, sd);

    if (!message || !*message || sscanf(message, "%d %s", &newlev, cmd) != 2) {
        clif_displaymessage(fd, "Usage: @adjcmdlvl <lvl> <command>.");
        return -1;
    }

    for (i = 0; atcommand_info[i].type != AtCommand_None; i++)
        if (strcmpi(cmd, atcommand_info[i].command+1) == 0) {
            atcommand_info[i].level = newlev;
            clif_displaymessage(fd, "@command level changed.");
            return 0;
        }

    clif_displaymessage(fd, "@command not found.");
    return -1;
}

/*==========================================
 * @adjgmlvl by [MouseJstr]
 *
 * Create a temp GM
 *
 * Used during beta testing to allow players to use GM commands
 * for short periods of time
 *------------------------------------------
 */
int
atcommand_adjgmlvl(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
    int newlev;
    char user[100];
    struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

    if (!message || !*message || sscanf(message, "%d %[^\r\n]", &newlev, user) != 2) {
        clif_displaymessage(fd, "Usage: @adjgmlvl <lvl> <user>.");
        return -1;
    }

    if((pl_sd=map_nick2sd((char *) user)) == NULL)
        return -1;

    pc_set_gm_level(pl_sd->status.account_id, newlev);

    return 0;
}


/*==========================================
 * @trade by [MouseJstr]
 *
 * Open a trade window with a remote player
 *
 * If I have to jump to a remote player one more time, I am
 * gonna scream!
 *------------------------------------------
 */
int
atcommand_trade(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
    struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

    if (!message || !*message)
        return -1;
    if((pl_sd=map_nick2sd((char *) message)) != NULL) {
        trade_traderequest(sd, pl_sd->bl.id);
        return 0;
    }
    return -1;
}

/*==========================================
 * @setbattleflag by [MouseJstr]
 *
 * set a battle_config flag without having to reboot
 */
int
atcommand_setbattleflag(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char flag[128], value[128];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%s %s", flag, value) != 2) {
        	clif_displaymessage(fd, "Usage: @setbattleflag <flag> <value>.");
        	return -1;
    	}

	if (battle_set_value(flag, value) == 0)
        	clif_displaymessage(fd, "unknown battle_config flag");
	else
        	clif_displaymessage(fd, "battle_config set as requested");

	return 0;
}


/*===========================
 * @unmute [Valaris]
 *===========================
*/
int atcommand_unmute(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);
	if (!message || !*message)
        	return -1;

	if((pl_sd=map_nick2sd((char *) message)) != NULL) {
		if(pl_sd->sc_data[SC_NOCHAT].timer!=-1) {
			pl_sd->status.manner = 0; // have to set to 0 first [celest]
			status_change_end(&pl_sd->bl,SC_NOCHAT,-1);
			clif_displaymessage(sd->fd,"Player unmuted");
		}
		else
			clif_displaymessage(sd->fd,"Player is not muted");
	}

	return 0;
}

/*==========================================
 * @uptime by MC Cameri
 *------------------------------------------
 */
int
atcommand_uptime(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	long seconds = 0, day = 24*60*60, hour = 60*60,
		minute = 60, days = 0, hours = 0, minutes = 0;
	nullpo_retr(-1, sd);

	seconds = (gettick()-ticks)/CLOCKS_PER_SEC;
	days = seconds/day;
	seconds -= (seconds/day>0)?(seconds/day)*day:0;
	hours = seconds/hour;
	seconds -= (seconds/hour>0)?(seconds/hour)*hour:0;
	minutes = seconds/minute;
	seconds -= (seconds/minute>0)?(seconds/minute)*minute:0;

        snprintf(atcmd_output, sizeof(atcmd_output), msg_table[245], days, hours, minutes, seconds);

	clif_displaymessage(fd,atcmd_output);

	return 0;
}

/*==========================================
 * @changesex <sex>
 * => Changes one's sex. Argument sex can be
 * 0 or 1, m or f, male or female.
 *------------------------------------------
 */
int
atcommand_changesex(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	chrif_char_ask_name(sd->status.account_id,sd->status.name, 5,0,0,0,0,0,0);
	return 0;
}

/*================================================
 * @mute - Mutes a player for a set amount of time
 *------------------------------------------------
 */
int atcommand_mute(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	int manner;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %99[^\n]", &manner, atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Usage: @mute <time> <character name>.");
		return -1;
	}

	if ((pl_sd = map_nick2sd(atcmd_player_name)) != NULL) {
		pl_sd->status.manner -= manner;
		if(pl_sd->status.manner < 0)
			status_change_start(&pl_sd->bl,SC_NOCHAT,0,0,0,0,0,0);
	}
	else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * @refresh (like @jumpto <<yourself>>)
 *------------------------------------------
 */
int atcommand_refresh(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);
	//pc_setpos(sd, sd->mapname, sd->bl.x, sd->bl.y, 3);
	clif_refresh(sd);
	return 0;
}

/*==========================================
 * @petid <part of pet name>
 * => Displays a list of matching pets.
 *------------------------------------------
 */
int
atcommand_petid(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char searchtext[100];
	char temp0[100];
	char temp1[100];
	int cnt = 0, i = 0;

	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if (sscanf(message, "%99s", searchtext) < 1)
		return -1;
	estr_lower(searchtext);
	snprintf(temp0, sizeof(temp0), "Search results for: %s", searchtext);
	clif_displaymessage(fd,temp0);
	while (i < MAX_PET_DB) {
		strcpy(temp1,pet_db[i].name);
		strcpy(temp1, estr_lower(temp1));
		strcpy(temp0,pet_db[i].jname);
		strcpy(temp0, estr_lower(temp1));
		if (strstr(temp1, searchtext) || strstr(temp0, searchtext) ) {
  			snprintf(temp0, sizeof(temp0), "ID: %i -- Name: %s", pet_db[i].class_,
     			pet_db[i].jname);
  			if (cnt >= 100) { // Only if there are custom pets
	  			clif_displaymessage(fd, "Be more specific, can't send more than"
					" 100 results.");
			} else {
				clif_displaymessage(fd, temp0);
			}
    		cnt++;
		}
	i++;
	}
	snprintf(temp0, sizeof(temp0),"%i pets have '%s' in their name.", cnt, searchtext);
	clif_displaymessage(fd, temp0);
	return 0;
}

/*==========================================
 * @identify
 * => GM's magnifier.
 *------------------------------------------
 */
int
atcommand_identify(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int i,num;

	nullpo_retr(-1, sd);

	for(i=num=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify!=1){
			num++;
		}
	}
	if (num > 0) {
		clif_item_identify_list(sd);
	} else {
		clif_displaymessage(fd,"There are no items to appraise.");
	}
	return 0;
}

/*==========================================
 * @gmotd (Global MOTD)
 * by davidsiaw :P
 *------------------------------------------
 */
int
atcommand_gmotd(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
		char buf[256];
		FILE *fp;
	nullpo_retr(-1, sd);
		if(	(fp = fopen(motd_txt, "r"))!=NULL){
			while (fgets(buf, 250, fp) != NULL){
				int i;
				for( i=0; buf[i]; i++){
					if( buf[i]=='\r' || buf[i]=='\n'){
						buf[i]=0;
						break;
					}
				}
				intif_GMmessage(buf,strlen(buf)+1,8);
			}
			fclose(fp);
		}
		return 0;
}

int atcommand_misceffect(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int effect = 0;
	nullpo_retr(-1, sd);
	if (!message || !*message)
		return -1;
	if (sscanf(message, "%d", &effect) < 1)
		return -1;
	clif_misceffect(&sd->bl,effect);

	return 0;
}

int charid2sessionid(int charid)
{
   int i;
   int session_id=0;
   struct map_session_data *pl_sd = NULL;

   for(i=0;i<fd_max;i++){
      if(session[i] && (pl_sd= (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth){
         if (pl_sd->status.char_id==charid) { session_id = i; break; }
      }
   }

   return session_id;
}

int accountid2sessionid(int accountid)
{
   int i;
   int session_id=0;
   struct map_session_data *pl_sd = NULL;

   for(i=0;i<fd_max;i++){
      if(session[i] && (pl_sd= (struct map_session_data *) session[i]->session_data) && pl_sd->state.auth){
         if (pl_sd->status.account_id==accountid) { session_id = i; break; }
      }
   }

   return session_id;
}


/*==========================================
 * Jump to a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */

int atcommand_jumptoid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
   memset(atcmd_output, '\0', sizeof(atcmd_output));

   if (!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @jumptoid/@warptoid/@gotoid <char id>).");
      return -1;
   }

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
            clif_displaymessage(fd, msg_table[247]);
            return -1;
         }
         if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
            clif_displaymessage(fd, msg_table[248]);
            return -1;
         }
         pc_setpos(sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
         sprintf(atcmd_output, msg_table[4], pl_sd->status.name); // Jump to %s
         clif_displaymessage(fd, atcmd_output);
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return -1;
      }
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);

   return 0;
}

/*==========================================
 * Jump to a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */

int atcommand_jumptoid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
   memset(atcmd_output, '\0', sizeof(atcmd_output));

   if (!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @jumptoid/@warptoid/@gotoid <account id>).");
      return -1;
   }

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
            clif_displaymessage(fd, msg_table[247]);
            return -1;
         }
         if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
            clif_displaymessage(fd, msg_table[248]);
            return -1;
         }
         pc_setpos(sd, pl_sd->mapname, pl_sd->bl.x, pl_sd->bl.y, 3);
         sprintf(atcmd_output, msg_table[4], pl_sd->status.name); // Jump to %s
         clif_displaymessage(fd, atcmd_output);
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return -1;
      }
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);

   return 0;
}

/*==========================================
 * Recall a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_recallid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
   memset(atcmd_output, '\0', sizeof(atcmd_output));

   if (!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @recallid <char id>).");
      return -1;
   }

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can recall only lower or same level
            if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
               clif_displaymessage(fd, msg_table[247]);
               return -1;
            }
            if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
               clif_displaymessage(fd, msg_table[248]);
               return -1;
            }
            pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
            sprintf(atcmd_output, msg_table[46], pl_sd->status.name); // Jump to %s
            clif_displaymessage(fd, atcmd_output);
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return -1;
      }
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);

   return 0;
}

/*==========================================
 * Recall a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_recallid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
   memset(atcmd_output, '\0', sizeof(atcmd_output));

   if (!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @recallid2 <account id>).");
      return -1;
   }

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can recall only lower or same level
            if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
				clif_displaymessage(fd, msg_table[247]);
				return -1;
            }
            if (sd->bl.m >= 0 && map[sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
               clif_displaymessage(fd, msg_table[248]);
               return -1;
            }
            pc_setpos(pl_sd, sd->mapname, sd->bl.x, sd->bl.y, 2);
            sprintf(atcmd_output, msg_table[46], pl_sd->status.name); // Jump to %s
            clif_displaymessage(fd, atcmd_output);
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[154]); // Character not found.
         return -1;
      }
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);

   return 0;
}

/*==========================================
 * Kick a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_kickid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   struct map_session_data *pl_sd;
   int cid=0;
   int session_id=0;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @kickid <char id>).");
      return -1;
   }

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) // you can kick only lower or same gm level
            clif_GM_kick(sd, pl_sd, 1);
         else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);

   return 0;
}

/*==========================================
 * Kick a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_kickid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   struct map_session_data *pl_sd;
   int aid=0;
   int session_id=0;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @kickid2 <account id>).");
      return -1;
   }

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) // you can kick only lower or same gm level
            clif_GM_kick(sd, pl_sd, 1);
         else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);

   return 0;
}

/*==========================================
 * Revive a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_reviveid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @reviveid <char id>).");
      return -1;
   }

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         pl_sd->status.hp = pl_sd->status.max_hp;
         pc_setstand(pl_sd);
         if (battle_config.pc_invincible_time > 0)
            pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
         clif_updatestatus(pl_sd, SP_HP);
         clif_updatestatus(pl_sd, SP_SP);
         clif_resurrection(&pl_sd->bl, 1);
         clif_displaymessage(fd, msg_table[51]); // Character revived.
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);


   return 0;
}

/*==========================================
 * Revive a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_reviveid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @reviveid2 <account id>).");
      return -1;
   }

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         pl_sd->status.hp = pl_sd->status.max_hp;
         pc_setstand(pl_sd);
         if (battle_config.pc_invincible_time > 0)
            pc_setinvincibletimer(sd, battle_config.pc_invincible_time);
         clif_updatestatus(pl_sd, SP_HP);
         clif_updatestatus(pl_sd, SP_SP);
         clif_resurrection(&pl_sd->bl, 1);
         clif_displaymessage(fd, msg_table[51]); // Character revived.
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);


   return 0;
}

/*==========================================
 * Kill a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_killid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int cid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (cid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player CID (usage: @killid <char id>).");
      return -1;
   }

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same level
            pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
            clif_displaymessage(fd, msg_table[14]); // Character killed.
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);

   return 0;
}

/*==========================================
 * Kill a player by PID number
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int atcommand_killid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   int aid=0;
   int session_id=0;
   struct map_session_data *pl_sd;

   memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));

   if (!message || (aid = atoi(message)) == 0 || !*message || sscanf(message, "%99[^\n]", atcmd_player_name) < 1) {
      clif_displaymessage(fd, "Please, enter a player AID (usage: @killid2 <account id>).");
      return -1;
   }

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if ((pl_sd = (struct map_session_data *) session[session_id]->session_data) != NULL) {
         if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can kill only lower or same level
            pc_damage(NULL, pl_sd, pl_sd->status.hp + 1);
            clif_displaymessage(fd, msg_table[14]); // Character killed.
         } else {
            clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
            return -1;
         }
      } else {
         clif_displaymessage(fd, msg_table[3]); // Character not found.
         return -1;
      }

   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);

   return 0;
}

/*==========================================
 * Make a player killable, by PID
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int
atcommand_charkillableid(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   struct map_session_data *pl_sd = NULL;
   int cid=0;
   int session_id=0;

   if (!message || (cid = atoi(message)) == 0  || !*message)
      return -1;

   cid=atoi(message);

   if ((session_id=charid2sessionid(cid))!=0)
   {
      if((pl_sd= (struct map_session_data *) session[session_id]->session_data) == NULL)
                   return -1;

      pl_sd->special_state.killable = !pl_sd->special_state.killable;

      if(pl_sd->special_state.killable)
        clif_displaymessage(fd, "The player is now killable");
           else
        clif_displaymessage(fd, "The player is no longer killable");
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, cid = %d\n",session_id,cid);
   return 0;
}


/*==========================================
 * Make a player killable, by PID
 * Original by Dino9021
 * Added in by nsstrunks
 *------------------------------------------
 */
int
atcommand_charkillableid2(
   const int fd, struct map_session_data* sd,
   const char* command, const char* message)
{
   struct map_session_data *pl_sd = NULL;
   int aid=0;
   int session_id=0;

   if (!message || (aid = atoi(message)) == 0 || !*message)
      return -1;

   aid=atoi(message);

   if ((session_id=accountid2sessionid(aid))!=0)
   {
      if((pl_sd= (struct map_session_data *) session[session_id]->session_data) == NULL)
                   return -1;

      pl_sd->special_state.killable = !pl_sd->special_state.killable;

      if(pl_sd->special_state.killable)
        clif_displaymessage(fd, "The player is now killable");
           else
        clif_displaymessage(fd, "The player is no longer killable");
   }
   else
   {
      clif_displaymessage(fd,msg_table[3]);
   }
   //printf("Session_id = %d, aid = %d\n",session_id,aid);
   return 0;
}

#ifndef TXT_ONLY  /* Begin SQL-Only commands */

/*==========================================
 * Mail System commands by [Valaris]
 *------------------------------------------
 */
int atcommand_listmail(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if(!battle_config.mail_system)
		return 0;

	nullpo_retr(-1, sd);

	if(strlen(command)==12)
		mail_check(sd,3);
	else if(strlen(command)==9)
		mail_check(sd,2);
	else
		mail_check(sd,1);
	return 0;
}

int atcommand_readmail(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	if(!battle_config.mail_system)
		return 0;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(sd->fd,"You must specify a message number.");
		return 0;
	}

	if(strlen(command)==11)
		mail_delete(sd,atoi(message));
	else
		mail_read(sd,atoi(message));

	return 0;
}

int atcommand_sendmail(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char name[24],text[80];

	if(!battle_config.mail_system)
		return 0;

	nullpo_retr(-1, sd);

	if (!message || !*message) {
		clif_displaymessage(sd->fd,"You must specify a recipient and a message.");
		return 0;
	}

	if ((sscanf(message, "\"%[^\"]\" %79[^\n]", name, text) < 2) &&
		(sscanf(message, "%23s %79[^\n]", name, text) < 2)) {
		clif_displaymessage(sd->fd,"You must specify a recipient and a message.");
		return 0;
	}

	if(strlen(command)==17)
		mail_send(sd,name,text,1);
	else
		mail_send(sd,name,text,0);

	return 0;
}

/*==========================================
 * Refresh online command for SQL [Valaris]
 * Will refresh and check online column of
 * players and set correctly.
 *------------------------------------------
 */
int atcommand_refreshonline(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	nullpo_retr(-1, sd);

	char_online_check();

	return 0;
}

#endif /* end sql only */

/*==========================================
 * Show Monster DB Info   v 1.0
 * originally by [Lupus] eAthena
 *------------------------------------------
 */
int atcommand_mobinfo(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	unsigned char msize[3][7] = {"Small", "Medium", "Large"};
	unsigned char mrace[12][11] = {"Formless", "Undead", "Beast", "Plant", "Insect", "Fish", "Demon", "Demi-Human", "Angel", "Dragon", "Boss", "Non-Boss"};
	unsigned char melement[11][8] = {"None", "Neutral", "Water", "Earth", "Fire", "Wind", "Poison", "Holy", "Dark", "Ghost", "Undead"};
	char atcmd_output2[200];
	struct item_data *item_data;
	struct mob_db *mob;
	int mob_id;
	int i, j;

	memset(atcmd_output, '\0', sizeof(atcmd_output));
	memset(atcmd_output2, '\0', sizeof(atcmd_output2));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id (usage: @mobinfo <monster_name_or_monster_ID>).");
		return -1;
	}

	// If monster identifier/name argument is a name
	if ((mob_id = mobdb_searchname(message)) == 0) // check name first (to avoid possible name begining by a number)
		mob_id = mobdb_checkid(atoi(message));

	if (mob_id == 0) {
		clif_displaymessage(fd, msg_table[40]); // Invalid monster ID or name.
		return -1;
	}

	mob = &mob_db[mob_id];

	// stats
	if (mob->mexp)
		sprintf(atcmd_output, "Monster (MVP): '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	else
		sprintf(atcmd_output, "Monster: '%s'/'%s' (%d)", mob->name, mob->jname, mob_id);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, " Level:%d  HP:%d  SP:%d  Base EXP:%d  Job EXP:%d", mob->lv, mob->max_hp, mob->max_sp, mob->base_exp, mob->job_exp);
	clif_displaymessage(fd, atcmd_output);
	sprintf(atcmd_output, " DEF:%d  MDEF:%d  STR:%d  AGI:%d  VIT:%d  INT:%d  DEX:%d  LUK:%d", mob->def, mob->mdef, mob->str, mob->agi, mob->vit, mob->int_, mob->dex, mob->luk);
	clif_displaymessage(fd, atcmd_output);
	if (mob->element < 20) {
		//Element - None, Level 0
		i = 0;
		j = 0;
	} else {
		i = mob->element % 20 + 1;
		j = mob->element / 20;
	}
	sprintf(atcmd_output, " ATK:%d~%d  Range:%d~%d~%d  Size:%s  Race: %s  Element: %s (Lv:%d)", mob->atk1, mob->atk2, mob->range, mob->range2 , mob->range3, msize[mob->size], mrace[mob->race], melement[i], j);
	clif_displaymessage(fd, atcmd_output);
	// drops
	clif_displaymessage(fd, " Drops:");
	strcpy(atcmd_output, " ");
	j = 0;
	for (i = 0; i < 10; i++) {
		if (mob->dropitem[i].nameid <= 0 || (item_data = itemdb_search(mob->dropitem[i].nameid)) == NULL)
			continue;
		if (mob->dropitem[i].p > 0) {
			sprintf(atcmd_output2, " - %s  %02.02f%%", item_data->name, (float)mob->dropitem[i].p / 100);
			strcat(atcmd_output, atcmd_output2);
			if (++j % 3 == 0) {
				clif_displaymessage(fd, atcmd_output);
				strcpy(atcmd_output, " ");
			}
		}
	}
	if (j == 0)
		clif_displaymessage(fd, "This monster has no drop.");
	else if (j % 3 != 0)
		clif_displaymessage(fd, atcmd_output);
	// mvp
	if (mob->mexp) {
		sprintf(atcmd_output, " MVP Bonus EXP:%d  %02.02f%%", mob->mexp, (float)mob->mexpper / 100);
		clif_displaymessage(fd, atcmd_output);
		strcpy(atcmd_output, " MVP Items:");
		j = 0;
		for (i = 0; i < 3; i++) {
			if (mob->mvpitem[i].nameid <= 0 || (item_data = itemdb_search(mob->mvpitem[i].nameid)) == NULL)
				continue;
			if (mob->mvpitem[i].p > 0) {
				j++;
				if (j == 1)
					sprintf(atcmd_output2, " %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
				else
					sprintf(atcmd_output2, " - %s  %02.02f%%", item_data->name, (float)mob->mvpitem[i].p / 100);
				strcat(atcmd_output, atcmd_output2);
			}
		}
		if (j == 0)
			clif_displaymessage(fd, "This monster has no MVP drop.");
		else
			clif_displaymessage(fd, atcmd_output);
	}

	return 0;
}

/*==========================================
 * @adopt by [Veider]
 *
 * adopt a novice
 *------------------------------------------
 */
int
atcommand_adopt(const int fd, struct map_session_data* sd,
const char* command, const char* message)
{
        struct map_session_data *pl_sd1 = NULL;
        struct map_session_data *pl_sd2 = NULL;
        struct map_session_data *pl_sd3 = NULL;
        char player1[255], player2[255], player3[255];

        nullpo_retr(-1, sd);

        if (!message || !*message)
                return -1;

        if (sscanf(message, "%[^,],%[^,],%[^\r\n]", player1, player2, player3) != 3) {
                clif_displaymessage(fd, "usage: @adopt <player1> <player2> <player3>.");
                return -1;
        }

        printf("Adopting: --%s--%s--%s--\n",player1,player2,player3);

        if((pl_sd1=map_nick2sd((char *) player1)) == NULL) {
                sprintf(player2, "Cannot find player %s online", player1);
                clif_displaymessage(fd, player2);
                return -1;
        }

        if((pl_sd2=map_nick2sd((char *) player2)) == NULL) {
                sprintf(player1, "Cannot find player %s online", player2);
                clif_displaymessage(fd, player1);
                return -1;
	}
 
       if((pl_sd3=map_nick2sd((char *) player3)) == NULL) {
                sprintf(player1, "Cannot find player %s online", player3);
                clif_displaymessage(fd, player1);
                return -1;
        }

        if((pl_sd1->status.base_level < 70) || (pl_sd2->status.base_level < 70)){
                clif_displaymessage(fd, "They are too young to be parents!");
                return -1;
        }

        if (pc_adoption(pl_sd1, pl_sd2, pl_sd3) == 0) {
                clif_displaymessage(fd, "They are family.. wish them luck");
                return 0;
        }
        else
                return -1;
}

