// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"

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
#include "charcommand.h"
#include "atcommand.h"

static char command_symbol = '#';

extern char *msg_table[1000]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

#define CCMD_FUNC(x) int charcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)
CCMD_FUNC(jobchange);
CCMD_FUNC(petrename);
CCMD_FUNC(petfriendly);
CCMD_FUNC(stats);
CCMD_FUNC(option);
CCMD_FUNC(save);
CCMD_FUNC(stats_all);
CCMD_FUNC(reset);
CCMD_FUNC(spiritball);
CCMD_FUNC(itemlist);
CCMD_FUNC(effect);
CCMD_FUNC(storagelist);
CCMD_FUNC(item);
CCMD_FUNC(warp);
CCMD_FUNC(zeny);
CCMD_FUNC(fakename);
CCMD_FUNC(baselevel);
CCMD_FUNC(joblevel);
CCMD_FUNC(questskill);
CCMD_FUNC(lostskill);
CCMD_FUNC(skreset);
CCMD_FUNC(streset);
CCMD_FUNC(model);
CCMD_FUNC(stpoint);
CCMD_FUNC(skpoint);
CCMD_FUNC(changesex);
CCMD_FUNC(feelreset);
CCMD_FUNC(help);


/*==========================================
 *CharCommandInfo charcommand_info[]構造体の定義
 *------------------------------------------
 */

// First char of commands is configured in charcommand_athena.conf. Leave # in this list for default value.
// to set default level, read charcommand_athena.conf first please.
static CharCommandInfo charcommand_info[] = {
	{ CharCommandJobChange,				"#job",					60, charcommand_jobchange },
	{ CharCommandJobChange,				"#jobchange",				60, charcommand_jobchange },
	{ CharCommandPetRename,				"#petrename",				50, charcommand_petrename },
	{ CharCommandPetFriendly,			"#petfriendly",				50, charcommand_petfriendly },
	{ CharCommandStats,				"#stats",					40, charcommand_stats },
	{ CharCommandOption,				"#option",					60, charcommand_option },
	{ CharCommandReset,				"#reset",					60, charcommand_reset },
	{ CharCommandSave,				"#save",					60, charcommand_save },
	{ CharCommandStatsAll,				"#statsall",				40, charcommand_stats_all },
	{ CharCommandSpiritball,			"#spiritball",				40, charcommand_spiritball },
	{ CharCommandItemList,				"#itemlist",				40, charcommand_itemlist },
	{ CharCommandEffect,				"#effect",					40, charcommand_effect },
	{ CharCommandStorageList,			"#storagelist",				40, charcommand_storagelist },
	{ CharCommandItem,				"#item",					60, charcommand_item },
	{ CharCommandWarp,				"#warp",					60, charcommand_warp },
	{ CharCommandWarp,				"#rura",					60, charcommand_warp },
	{ CharCommandWarp,				"#rura+",					60, charcommand_warp },
	{ CharCommandZeny,				"#zeny",					60, charcommand_zeny },
	{ CharCommandFakeName,				"#fakename",				20, charcommand_fakename},
	
	//*********************************Recently added commands*********************************************
	{ CharCommandBaseLevel,				"#baselvl",					20, charcommand_baselevel},
	{ CharCommandBaseLevel,				"#blvl",					60, charcommand_baselevel},
	{ CharCommandBaseLevel,				"#baselvlup",				60, charcommand_baselevel},
	{ CharCommandJobLevel,				"#joblvl",					60, charcommand_joblevel},
	{ CharCommandJobLevel,				"#jlvl",					60, charcommand_joblevel},
	{ CharCommandJobLevel,				"#joblvlup",				60, charcommand_joblevel},
	{ CharCommandQuestSkill,			"#questskill",				60, charcommand_questskill },
	{ CharCommandLostSkill,				"#lostskill",				60, charcommand_lostskill },
	{ CharCommandSkReset,				"#skreset",					60, charcommand_skreset },
	{ CharCommandStReset,				"#streset",					60, charcommand_streset },
	{ CharCommandModel,				"#model",					50, charcommand_model },
	{ CharCommandSKPoint,				"#skpoint",					60, charcommand_skpoint },
	{ CharCommandSTPoint,				"#stpoint",					60, charcommand_stpoint },
	{ CharCommandChangeSex,				"#changesex",				60, charcommand_changesex },
	{ CharCommandFeelReset,				"#feelreset",				60, charcommand_feelreset },
	{ CharCommandHelp,				"#help",				20, charcommand_help },
// add new commands before this line
	{ CharCommand_Unknown, 		            NULL, 				       1, NULL }
};

int get_charcommand_level(const CharCommandType type) {
	int i;

	for (i = 0; charcommand_info[i].type != CharCommand_None; i++)
		if (charcommand_info[i].type == type)
			return charcommand_info[i].level;

	return 100; // 100: command can not be used
}

/*==========================================
 *is_charcommand @コマンドに存在するかどうか確認する
 *------------------------------------------
 */
CharCommandType
is_charcommand(const int fd, struct map_session_data* sd, const char* message, int gmlvl) {
	const char* str = message;
	int s_flag = 0;
	CharCommandInfo info;
	CharCommandType type;

	nullpo_retr(CharCommand_None, sd);

	if (!message || !*message)
		return CharCommand_None;

	malloc_set(&info, 0, sizeof(info));
	str += strlen(sd->status.name);
	while (*str && (isspace(*str) || (s_flag == 0 && *str == ':'))) {
		if (*str == ':')
			s_flag = 1;
		str++;
	}
	if (!*str)
		return CharCommand_None;

	if (!gmlvl) gmlvl = pc_isGM(sd);
	type = charcommand(sd, gmlvl, str, &info);
	if (type != CharCommand_None) {
		char command[100];
		char output[200];
		const char* p = str;

		if (map[sd->bl.m].nocommand &&
			gmlvl < map[sd->bl.m].nocommand)
		{	//Command not allowed on this map.
			sprintf(output, msg_txt(143)); 
			clif_displaymessage(fd, output);
			return AtCommand_None;
		}

		malloc_tsetdword(command, '\0', sizeof(command));
		malloc_tsetdword(output, '\0', sizeof(output));
		while (*p && !isspace(*p))
			p++;
		if (p - str >= sizeof(command)) // too long
			return CharCommand_Unknown;
		strncpy(command, str, p - str);
		while (isspace(*p))
			p++;

		if (type == CharCommand_Unknown || info.proc == NULL) {
			snprintf(output, sizeof(output),msg_txt(153), command); // %s is Unknown Command.
			clif_displaymessage(fd, output);
		} else {
			if (info.proc(fd, sd, command, p) != 0) {
				// Command can not be executed
				snprintf(output, sizeof(output), msg_txt(154), command); // %s failed.
				clif_displaymessage(fd, output);
			}
		}

		return info.type;
	}

	return CharCommand_None;
}

/*==========================================
 *
 *------------------------------------------
 */
CharCommandType charcommand(struct map_session_data* sd, const int level, const char* message, CharCommandInfo* info) {
	char* p = (char *)message; 

	if (!info)
		return CharCommand_None;
	if (battle_config.atc_gmonly != 0 && !level) // level = pc_isGM(sd)
		return CharCommand_None;
	if (!p || !*p) {
		ShowError("char command message is empty\n");
		return CharCommand_None;
	}

	if (*p == command_symbol) { // check first char.
		char command[101];
		int i = 0;
		malloc_set(info, 0, sizeof(CharCommandInfo));
		sscanf(p, "%100s", command);
		command[sizeof(command)-1] = '\0';

		while (charcommand_info[i].type != CharCommand_Unknown) {
			if (strcmpi(command+1, charcommand_info[i].command+1) == 0 && level >= charcommand_info[i].level) {
				p[0] = charcommand_info[i].command[0]; // set correct first symbol for after.
				break;
			}
			i++;
		}

		if (charcommand_info[i].type == CharCommand_Unknown) {
			// doesn't return Unknown if player is normal player (display the text, not display: unknown command)
			if (level == 0)
				return CharCommand_None;
			else
				return CharCommand_Unknown;
		} else if((log_config.gm) && (charcommand_info[i].level >= log_config.gm)) {
			log_atcommand(sd, message);
		}
		memcpy(info, &charcommand_info[i], sizeof charcommand_info[i]);
	} else {
		return CharCommand_None;
	}

	return info->type;
}


/*==========================================
 *
 *------------------------------------------
 */
static CharCommandInfo* get_charcommandinfo_byname(const char* name) {
	int i;

	for (i = 0; charcommand_info[i].type != CharCommand_Unknown; i++)
		if (strcmpi(charcommand_info[i].command + 1, name) == 0)
			return &charcommand_info[i];

	return NULL;
}

/*==========================================
 *
 *------------------------------------------
 */
int charcommand_config_read(const char *cfgName) {
	char line[1024], w1[1024], w2[1024];
	CharCommandInfo* p;
	FILE* fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		ShowError("CharCommands configuration file not found: %s\n", cfgName);
		return 1;
	}

	while (fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2)
			continue;
		p = get_charcommandinfo_byname(w1);
		if (p != NULL) {
			p->level = atoi(w2);
			if (p->level > 100)
				p->level = 100;
			else if (p->level < 0)
				p->level = 0;
		}

		if (strcmpi(w1, "import") == 0)
			charcommand_config_read(w2);
		else if (strcmpi(w1, "command_symbol") == 0 && w2[0] > 31 &&
				w2[0] != '/' && // symbol of standard ragnarok GM commands
				w2[0] != '%' && // symbol of party chat speaking
				w2[0] != '$' && // symbol of guild chat speaking
				w2[0] != '@')	// symbol of atcommand
			command_symbol = w2[0];
	}
	fclose(fp);

	return 0;
}

/*==========================================
 * 対象キャラクターを転職させる upper指定で転生や養子も可能
 *------------------------------------------
 */
int charcommand_jobchange(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[100];
	struct map_session_data* pl_sd;
	int job = 0, upper = -1;

	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message) {
		clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
		return -1;
	}

	if (sscanf(message, "%d %d %99[^\n]", &job, &upper, character) < 3) { //upper指定してある
		upper = -1;
		if (sscanf(message, "%d %99[^\n]", &job, character) < 2) { //upper指定してない上に何か足りない
			clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <char name>).");
			return -1;
		}
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		int j;
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job only to lower or same level
			if ((job >= 0 && job < MAX_PC_CLASS)) {
				for (j=0; j < MAX_INVENTORY; j++) {
					if(pl_sd->status.inventory[j].nameid>0 && pl_sd->status.inventory[j].equip!=0)
						pc_unequipitem(pl_sd, j, 3);
				}
				if (pc_jobchange(pl_sd, job, upper) == 0)
					clif_displaymessage(fd, msg_table[48]); // Character's job changed.
				else {
					clif_displaymessage(fd, msg_table[192]); // Impossible to change the character's job.
					return -1;
				}
			} else {
				clif_displaymessage(fd, msg_table[49]); // Invalid job ID.
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
 *
 *------------------------------------------
 */
int charcommand_petrename(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct pet_data *pd;

	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #petrename <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (!pl_sd->status.pet_id || !pl_sd->pd) {
		clif_displaymessage(fd, msg_table[191]); // Sorry, but this player has no pet.
		return -1;
	}

	pd = pl_sd->pd;

	if (pd->pet.rename_flag) {
		clif_displaymessage(fd, msg_table[190]); // This player can already rename his/her pet.
		return -1;
	}
	pd->pet.rename_flag = 0;
	intif_save_petdata(pl_sd->status.account_id, &pd->pet);
	clif_send_petstatus(pl_sd);
	clif_displaymessage(fd, msg_table[189]); // This player can now rename his/her pet.
	return 0;
}


/*==========================================
 * 
 *------------------------------------------
 */
int charcommand_petfriendly(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int friendly = 0;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct pet_data *pd;

	malloc_tsetdword(character, '\0', sizeof(character));
	if (!message || !*message || sscanf(message,"%d %23s",&friendly,character) < 2) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: "
			"#petfriendly <0-1000> <player>).");
		return -1;
	}

	if (((pl_sd = map_nick2sd(character)) == NULL) ||
		pc_isGM(sd)<pc_isGM(pl_sd)) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (!pl_sd->status.pet_id  || !pl_sd->pd) {
		clif_displaymessage(fd, msg_table[191]); // Sorry, but this player has no pet.
		return -1;
	}

	if (friendly < 0 || friendly > 1000) {
		clif_displaymessage(fd, msg_table[37]); // An invalid number was specified.
		return -1;
	}

	pd = pl_sd->pd;
	if (friendly == pd->pet.intimate) {
		clif_displaymessage(fd, msg_table[183]); // Pet friendly is already the good value.
		return -1;
	}
	
	pd->pet.intimate = friendly;
	clif_send_petstatus(pl_sd);
	clif_pet_emotion(pd,0);
	clif_displaymessage(pl_sd->fd, msg_table[182]); // Pet friendly value changed!
	clif_displaymessage(sd->fd, msg_table[182]); // Pet friendly value changed!
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int charcommand_stats(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char job_jobname[100];
	char output[200];
	struct map_session_data *pl_sd;
	int i;

	malloc_tsetdword(character, '\0', sizeof(character));
	malloc_tsetdword(job_jobname, '\0', sizeof(job_jobname));
	malloc_tsetdword(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #stats <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		struct {
			const char* format;
			int value;
		} output_table[] = {
			{ "Base Level - %d", 0 },
			{ NULL, 0 },
			{ "Hp - %d", 0 },
			{ "MaxHp - %d", 0 },
			{ "Sp - %d", 0 },
			{ "MaxSp - %d", 0 },
			{ "Str - %3d", 0 },
			{ "Agi - %3d", 0 },
			{ "Vit - %3d", 0 },
			{ "Int - %3d", 0 },
			{ "Dex - %3d", 0 },
			{ "Luk - %3d", 0 },
			{ "Zeny - %d", 0 },
			{ NULL, 0 }
		};
		//direct array initialization with variables is not standard C compliant.
		output_table[0].value = pl_sd->status.base_level;
		output_table[1].format = job_jobname;
		output_table[1].value = pl_sd->status.job_level;
		output_table[2].value = pl_sd->status.hp;
		output_table[3].value = pl_sd->status.max_hp;
		output_table[4].value = pl_sd->status.sp;
		output_table[5].value = pl_sd->status.max_sp;
		output_table[6].value = pl_sd->status.str;
		output_table[7].value = pl_sd->status.agi;
		output_table[8].value = pl_sd->status.vit;
		output_table[9].value = pl_sd->status.int_;
		output_table[10].value = pl_sd->status.dex;
		output_table[11].value = pl_sd->status.luk;
		output_table[12].value = pl_sd->status.zeny;
		sprintf(job_jobname, "Job - %s %s", job_name(pl_sd->status.class_), "(level %d)");
		sprintf(output, msg_table[53], pl_sd->status.name); // '%s' stats:
		clif_displaymessage(fd, output);
		for (i = 0; output_table[i].format != NULL; i++) {
			sprintf(output, output_table[i].format, output_table[i].value);
			clif_displaymessage(fd, output);
		}
	} else {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Reset
 *------------------------------------------
 */
int charcommand_reset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	malloc_tsetdword(character, '\0', sizeof(character));
	malloc_tsetdword(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #reset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset a character only for lower or same GM level
			pc_resetstate(pl_sd);
			pc_resetskill(pl_sd,1);
			sprintf(output, msg_table[208], character); // '%s' skill and stats points reseted!
			clif_displaymessage(fd, output);
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
int charcommand_option(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[NAME_LENGTH];
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message ||
		sscanf(message, "%d %d %d %23[^\n]", &opt1, &opt2, &opt3, character) < 4 ||
		opt1 < 0 || opt2 < 0 || opt3 < 0) {
		clif_displaymessage(fd, "Please, enter valid options and a player name (usage: #option <param1> <param2> <param3> <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change option only to lower or same level
			pl_sd->sc.opt1 = opt1;
			pl_sd->sc.opt2 = opt2;
			pc_setoption(pl_sd, opt3);
			clif_displaymessage(fd, msg_table[58]); // Character's options changed.
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
int charcommand_save(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[MAP_NAME_LENGTH];
	char character[NAME_LENGTH];
	struct map_session_data* pl_sd;
	int x = 0, y = 0;
	int m;

	malloc_tsetdword(map_name, '\0', sizeof(map_name));
	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%15s %d %d %23[^\n]", map_name, &x, &y, character) < 4 || x < 0 || y < 0) {
		clif_displaymessage(fd, "Please, enter a valid save point and a player name (usage: #save <map> <x> <y> <charname>).");
		return -1;
	}

	if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < MAP_NAME_LENGTH-4) // 16 - 4 (.gat)
		strcat(map_name, ".gat");

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change save point only to lower or same gm level
			m = map_mapname2mapid(map_name);
			if (m < 0 && !mapindex_name2id(map_name)) {
				clif_displaymessage(fd, msg_table[1]); // Map not found.
				return -1;
			} else {
				//FIXME: What do you do if the map is in another map server with the nowarpto flag?
				if (m>=0 && map[m].flag.nosave && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
					clif_displaymessage(fd, "You are not authorised to set this map as a save map.");
					return -1;
				}
				if (m>=0)
					pc_setsavepoint(pl_sd, map[m].index, x, y);
				else
					pc_setsavepoint(pl_sd, mapindex_name2id(map_name), x, y);
				clif_displaymessage(fd, msg_table[57]); // Character's respawn point changed.
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
//** Character Stats All by fritz
int charcommand_stats_all(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[1024], gmlevel[1024];
	int i;
	int count, users;
	struct map_session_data *pl_sd, **pl_allsd;

	malloc_tsetdword(output, '\0', sizeof(output));
	malloc_tsetdword(gmlevel, '\0', sizeof(gmlevel));

	count = 0;
	pl_allsd = map_getallusers(&users);
	for(i = 0; i < users; i++) {
		if ((pl_sd = pl_allsd[i]))
		{
			if (pc_isGM(pl_sd) > 0)
				sprintf(gmlevel, "| GM Lvl: %d", pc_isGM(pl_sd));
			else
				sprintf(gmlevel, " ");

			sprintf(output, "Name: %s | BLvl: %d | Job: %s (Lvl: %d) | HP: %d/%d | SP: %d/%d", pl_sd->status.name, pl_sd->status.base_level, job_name(pl_sd->status.class_), pl_sd->status.job_level, pl_sd->status.hp, pl_sd->status.max_hp, pl_sd->status.sp, pl_sd->status.max_sp);
			clif_displaymessage(fd, output);
			sprintf(output, "STR: %d | AGI: %d | VIT: %d | INT: %d | DEX: %d | LUK: %d | Zeny: %d %s", pl_sd->status.str, pl_sd->status.agi, pl_sd->status.vit, pl_sd->status.int_, pl_sd->status.dex, pl_sd->status.luk, pl_sd->status.zeny, gmlevel);
			clif_displaymessage(fd, output);
			clif_displaymessage(fd, "--------");
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_table[28]); // No player found.
	else if (count == 1)
		clif_displaymessage(fd, msg_table[29]); // 1 player found.
	else {
		sprintf(output, msg_table[30], count); // %d players found.
		clif_displaymessage(fd, output);
	}

	return 0;
}

/*==========================================
 * CharSpiritBall Function by PalasX
 *------------------------------------------
 */
int charcommand_spiritball(const int fd, struct map_session_data* sd,const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[NAME_LENGTH];
	int spirit = 0;

	malloc_tsetdword(character, '\0', sizeof(character));

	if(!message || !*message || sscanf(message, "%d %23[^\n]", &spirit, character) < 2 || spirit < 0 || spirit > 1000) {
		clif_displaymessage(fd, "Usage: @spiritball <number: 0-1000>) <CHARACTER_NAME>.");
		return -1;
	}

	if((pl_sd = map_nick2sd(character)) != NULL) {
		if (spirit >= 0 && spirit <= 0x7FFF) {
			if (pl_sd->spiritball != spirit || spirit > 999) {
				if (pl_sd->spiritball > 0)
					pc_delspiritball(pl_sd, pl_sd->spiritball, 1);
				pl_sd->spiritball = spirit;
				clif_spiritball(pl_sd);
				// no message, player can look the difference
				if (spirit > 1000)
					clif_displaymessage(fd, msg_table[204]); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
			} else {
				clif_displaymessage(fd, msg_table[205]); // You already have this number of spiritballs.
				return -1;
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
 * #itemlist <character>: Displays the list of a player's items.
 *------------------------------------------
 */
int
charcommand_itemlist(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, equip, count, counter, counter2;
	char character[NAME_LENGTH], output[200], equipstr[100], outputtmp[200];
	struct item *i_item; //Current inventory item.
	nullpo_retr(-1, sd);

	malloc_tsetdword(character, '\0', sizeof(character));
	malloc_tsetdword(output, '\0', sizeof(output));
	malloc_tsetdword(equipstr, '\0', sizeof(equipstr));
	malloc_tsetdword(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
			counter = 0;
			count = 0;
			for (i = 0; i < MAX_INVENTORY; i++) {
				i_item = &pl_sd->status.inventory[i];
				if (pl_sd->status.inventory[i].nameid > 0 && (item_data = itemdb_exists(i_item->nameid)) != NULL) {
					counter = counter + i_item->amount;
					count++;
					if (count == 1) {
						sprintf(output, "------ Items list of '%s' ------", pl_sd->status.name);
						clif_displaymessage(fd, output);
					}
					if ((equip = i_item->equip)) {
						strcpy(equipstr, "| equiped: ");
						if (equip & EQP_GARMENT)
							strcat(equipstr, "robe/gargment, ");
						if (equip & EQP_ACC_L)
							strcat(equipstr, "left accessory, ");
						if (equip & EQP_ARMOR)
							strcat(equipstr, "body/armor, ");
						if ((equip & EQP_ARMS) == EQP_HAND_R)
							strcat(equipstr, "right hand, ");
						if ((equip & EQP_ARMS) == EQP_HAND_L)
							strcat(equipstr, "left hand, ");
						if ((equip & EQP_ARMS) == EQP_ARMS)
							strcat(equipstr, "both hands, ");
						if (equip & EQP_SHOES)
							strcat(equipstr, "feet, ");
						if (equip & EQP_ACC_R)
							strcat(equipstr, "right accessory, ");
						if ((equip & EQP_HELM) == EQP_HEAD_LOW)
							strcat(equipstr, "lower head, ");
						if ((equip & EQP_HELM) == EQP_HEAD_TOP)
							strcat(equipstr, "top head, ");
						if ((equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_TOP))
							strcat(equipstr, "lower/top head, ");
						if ((equip & EQP_HELM) == EQP_HEAD_MID)
							strcat(equipstr, "mid head, ");
						if ((equip & EQP_HELM) == (EQP_HEAD_LOW|EQP_HEAD_MID))
							strcat(equipstr, "lower/mid head, ");
						if ((equip & EQP_HELM) == EQP_HELM)
							strcat(equipstr, "lower/mid/top head, ");
						// remove final ', '
						equipstr[strlen(equipstr) - 2] = '\0';
					} else
						malloc_tsetdword(equipstr, '\0', sizeof(equipstr));
					if (i_item->refine)
						sprintf(output, "%d %s %+d (%s %+d, id: %d) %s", i_item->amount, item_data->name, i_item->refine, item_data->jname, i_item->refine, i_item->nameid, equipstr);
					else
						sprintf(output, "%d %s (%s, id: %d) %s", i_item->amount, item_data->name, item_data->jname, i_item->nameid, equipstr);
					clif_displaymessage(fd, output);
					malloc_tsetdword(output, '\0', sizeof(output));
					counter2 = 0;

					if(i_item->card[0]==CARD0_PET) { //pet eggs
						if (i_item->card[3])
							sprintf(outputtmp, " -> (pet egg, pet id: %u, named)", (unsigned int)MakeDWord(i_item->card[1], i_item->card[2]));
						else
							sprintf(outputtmp, " -> (pet egg, pet id: %u, unnamed)", (unsigned int)MakeDWord(i_item->card[1], i_item->card[2]));
						strcat(output, outputtmp);
					} else
					if(i_item->card[0]==CARD0_FORGE) { //forged items.
						sprintf(outputtmp, " -> (crafted item, creator id: %u, star crumbs %d, element %d)", (unsigned int)MakeDWord(i_item->card[2], i_item->card[3]), i_item->card[1]>>8, i_item->card[1]&0x0f);
					} else
					if(i_item->card[0]==CARD0_CREATE) { //created items.
						sprintf(outputtmp, " -> (produced item, creator id: %u)", (unsigned int)MakeDWord(i_item->card[2], i_item->card[3]));
						strcat(output, outputtmp);
					} else //Normal slots
					for (j = 0; j < item_data->slot; j++) {
						if (pl_sd->status.inventory[i].card[j]) {
							if ((item_temp = itemdb_exists(i_item->card[j])) != NULL) {
								if (output[0] == '\0')
									sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								else
									sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
								strcat(output, outputtmp);
							}
						}
					}
					if (output[0] != '\0') {
						output[strlen(output) - 2] = ')';
						output[strlen(output) - 1] = '\0';
						clif_displaymessage(fd, output);
					}
				}
			}
			if (count == 0)
				clif_displaymessage(fd, "No item found on this player.");
			else {
				sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
				clif_displaymessage(fd, output);
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
 * #effect by [MouseJstr]
 *
 * Create a effect localized on another character
 *------------------------------------------
 */
int
charcommand_effect(const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char target[255];
	int type = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %s", &type, target) != 2) {
		clif_displaymessage(fd, "usage: #effect <type+> <target>.");
		return -1;
	}

	if((pl_sd=map_nick2sd((char *) target)) == NULL)
		return -1;

	clif_specialeffect(&pl_sd->bl, type, AREA);
	clif_displaymessage(fd, msg_table[229]); // Your effect has changed.

	return 0;
}

/*==========================================
 * #storagelist <character>: Displays the items list of a player's storage.
 *------------------------------------------
 */
int
charcommand_storagelist(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	char character[NAME_LENGTH], output[200], outputtmp[200];
	nullpo_retr(-1, sd);

	malloc_tsetdword(character, '\0', sizeof(character));
	malloc_tsetdword(output, '\0', sizeof(output));
	malloc_tsetdword(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
			if((stor = account2storage2(pl_sd->status.account_id)) != NULL) {
				counter = 0;
				count = 0;
				for (i = 0; i < MAX_STORAGE; i++) {
					if (stor->storage_[i].nameid > 0 && (item_data = itemdb_search(stor->storage_[i].nameid)) != NULL) {
						counter = counter + stor->storage_[i].amount;
						count++;
						if (count == 1) {
							sprintf(output, "------ Storage items list of '%s' ------", pl_sd->status.name);
							clif_displaymessage(fd, output);
						}
						if (stor->storage_[i].refine)
							sprintf(output, "%d %s %+d (%s %+d, id: %d)", stor->storage_[i].amount, item_data->name, stor->storage_[i].refine, item_data->jname, stor->storage_[i].refine, stor->storage_[i].nameid);
						else
							sprintf(output, "%d %s (%s, id: %d)", stor->storage_[i].amount, item_data->name, item_data->jname, stor->storage_[i].nameid);
						clif_displaymessage(fd, output);
						malloc_tsetdword(output, '\0', sizeof(output));
						counter2 = 0;
						for (j = 0; j < item_data->slot; j++) {
							if (stor->storage_[i].card[j]) {
								if ((item_temp = itemdb_search(stor->storage_[i].card[j])) != NULL) {
									if (output[0] == '\0')
										sprintf(outputtmp, " -> (card(s): #%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									else
										sprintf(outputtmp, "#%d %s (%s), ", ++counter2, item_temp->name, item_temp->jname);
									strcat(output, outputtmp);
								}
							}
						}
						if (output[0] != '\0') {
							output[strlen(output) - 2] = ')';
							output[strlen(output) - 1] = '\0';
							clif_displaymessage(fd, output);
						}
					}
				}
				if (count == 0)
					clif_displaymessage(fd, "No item found in the storage of this player.");
				else {
					sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
					clif_displaymessage(fd, output);
				}
			} else {
				clif_displaymessage(fd, "This player has no storage.");
				return 0;
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

static void 
charcommand_giveitem_sub(struct map_session_data *sd,struct item_data *item_data,int number)
{
	int flag = 0;
	int loop = 1, get_count = number,i;
	struct item item_tmp;

	if(sd && item_data){
		if (item_data->type == 4 || item_data->type == 5 ||
			item_data->type == 7 || item_data->type == 8) {
			loop = number;
			get_count = 1;
		}
		for (i = 0; i < loop; i++) {
			malloc_set(&item_tmp, 0, sizeof(item_tmp));
			item_tmp.nameid = item_data->nameid;
			item_tmp.identify = 1;

			if ((flag = pc_additem((struct map_session_data*)sd,
					&item_tmp, get_count)))
				clif_additem((struct map_session_data*)sd, 0, 0, flag);
		}
		//Logs (A)dmins items [Lupus]
		if(log_config.enable_logs&0x400)
			log_pick_pc(sd, "A", item_tmp.nameid, number, &item_tmp);

	}
}
/*==========================================
 * #item command (usage: #item <name/id_of_item> <quantity> <player>)
 * by MC Cameri
 *------------------------------------------
 */
int charcommand_item(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char item_name[100];
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	int number = 0, item_id, flag;
	struct item item_tmp;
	struct item_data *item_data;
	int get_count, i, pet_id;
	char tmp_cmdoutput[1024];
	nullpo_retr(-1, sd);

	malloc_tsetdword(item_name, '\0', sizeof(item_name));

	if (!message || !*message || sscanf(message, "%99s %d %23[^\n]", item_name, &number, character) < 3) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: #item <item name or ID> <quantity> <char name>).");
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
		if ((pl_sd = map_nick2sd(character)) != NULL) {
			if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can look items only lower or same level
				for (i = 0; i < number; i += get_count) {
					// if pet egg
					if (pet_id >= 0) {
						pl_sd->catch_target_class = pet_db[pet_id].class_;
						intif_create_pet(pl_sd->status.account_id, pl_sd->status.char_id,
										 (short)pet_db[pet_id].class_, (short)mob_db(pet_db[pet_id].class_)->lv,
										 (short)pet_db[pet_id].EggID, 0, (short)pet_db[pet_id].intimate,
										 100, 0, 1, pet_db[pet_id].jname);
					// if not pet egg
					} else {
						malloc_set(&item_tmp, 0, sizeof(item_tmp));
						item_tmp.nameid = item_id;
						item_tmp.identify = 1;

						if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
							clif_additem(pl_sd, 0, 0, flag);
					}
				}

				//Logs (A)dmins items [Lupus]
				if(log_config.enable_logs&0x400)
					log_pick_pc(sd, "A", item_tmp.nameid, number, &item_tmp);

				clif_displaymessage(fd, msg_table[18]); // Item created.
			} else {
				clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
				return -1;
			}
		} else if(/* from jA's @giveitem */strcmpi(character,"all")==0 || strcmpi(character,"everyone")==0){
			struct map_session_data **pl_allsd;
			int users;
			pl_allsd = map_getallusers(&users);
			for (i = 0; i < users; i++) {
				if ((pl_sd = pl_allsd[i])) {
					charcommand_giveitem_sub(pl_sd,item_data,number);
					snprintf(tmp_cmdoutput, sizeof(tmp_cmdoutput), "You got %s %d.", item_name,number);
					clif_displaymessage(pl_sd->fd, tmp_cmdoutput);
				}
			}
			snprintf(tmp_cmdoutput, sizeof(tmp_cmdoutput), "%s received %s %d.","Everyone",item_name,number);
			clif_displaymessage(fd, tmp_cmdoutput);
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
 * #warp/#rura/#rura+ <mapname> <x> <y> <char name>
 *------------------------------------------
 */
int charcommand_warp(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char map_name[MAP_NAME_LENGTH];
	char character[NAME_LENGTH];
	int x = 0, y = 0;
	struct map_session_data *pl_sd;
	int m;

	nullpo_retr(-1, sd);

	malloc_tsetdword(map_name, '\0', sizeof(map_name));
	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%15s %d %d %23[^\n]", map_name, &x, &y, character) < 4) {
		clif_displaymessage(fd, "Usage: #warp/#rura/#rura+ <mapname> <x> <y> <char name>");
		return -1;
	}

	if (strstr(map_name, ".gat") == NULL && strstr(map_name, ".afm") == NULL && strlen(map_name) < MAP_NAME_LENGTH-4) // 16 - 4 (.gat)
		strcat(map_name, ".gat");

	if ((pl_sd = map_nick2sd(character)) == NULL) {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}
	if (pc_isGM(sd) < pc_isGM(pl_sd)) { // you can rura+ only lower or same GM level
		clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	m = map_mapname2mapid(map_name);
	if (m < 0) {
		clif_displaymessage(fd, msg_table[1]); // Map not found.
		return -1;
	}
	if ((x || y) && map_getcell(m, x, y, CELL_CHKNOREACH)) {
		clif_displaymessage(fd, msg_table[2]); // Coordinates out of range.
		x = y = 0;
	}
	if (map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp someone to this map.");
		return -1;
	}
	if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to warp this player from its actual map.");
		return -1;
	}
	if (pc_setpos(pl_sd, map[m].index, x, y, 3) == 0) {
		clif_displaymessage(pl_sd->fd, msg_table[0]); // Warped.
		clif_displaymessage(fd, msg_table[15]); // Player warped (message sends to player too).
		return 0;
	}
	//No error message specified...?
	return -1;
}

/*==========================================
 * #zeny <charname>
 *------------------------------------------
 */
int charcommand_zeny(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[NAME_LENGTH];
	int zeny = 0, new_zeny;
	nullpo_retr(-1, sd);

	malloc_tsetdword(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &zeny, character) < 2 || zeny == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: #zeny <zeny> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		new_zeny = pl_sd->status.zeny + zeny;
		if (zeny > 0 && (zeny > MAX_ZENY || new_zeny > MAX_ZENY)) // fix positiv overflow
			new_zeny = MAX_ZENY;
		else if (zeny < 0 && (zeny < -MAX_ZENY || new_zeny < 0)) // fix negativ overflow
			new_zeny = 0;
		if (new_zeny != pl_sd->status.zeny) {
			pl_sd->status.zeny = new_zeny;
			clif_updatestatus(pl_sd, SP_ZENY);
			clif_displaymessage(fd, msg_table[211]); // Character's number of zenys changed!
		} else {
			if (zeny < 0)
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
 * #fakename <char name> <fake name>
 *------------------------------------------
 */

int charcommand_fakename(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char name[NAME_LENGTH];
	char char_name[NAME_LENGTH];
	
	nullpo_retr(-1, sd);

	name[0] = '\0'; //If you don't pass a second word, name is left as garbage, most definitely not a blank name! [Skotlex]
	if (!message || !*message || sscanf(message, "%23s %23[^\n]", char_name, name) < 1) {
		clif_displaymessage(sd->fd,"Usage: #fakename <char name> <fake name>.");
		clif_displaymessage(sd->fd,"Or: #fakename <char name> to disable.");
		return 0;
	}

	if(!(pl_sd = map_nick2sd(char_name))) {
		clif_displaymessage(sd->fd,"Character not found.");
		return -1;
	}

	if(strlen(name) < 1 || !name) {
		if(strlen(pl_sd->fakename) > 1) {
			pl_sd->fakename[0]='\0';
			clif_charnameack(0, &pl_sd->bl);
			clif_displaymessage(sd->fd,"Returned to real name.");
		} else {
			clif_displaymessage(sd->fd,"Character does not has a fake name.");
		}
		return 0;
	}

	if(strlen(name) < 2) {
		clif_displaymessage(sd->fd,"Fake name must be at least two characters.");
		return 0;
	}
	
	memcpy(pl_sd->fakename,name, NAME_LENGTH-1);
	clif_charnameack(0, &pl_sd->bl);
	clif_displaymessage(sd->fd,"Fake name enabled.");
	
	return 0;
}


/*==========================================
 * #baselvl <#> <nickname> 
 * Transferred by: Kevin
 *------------------------------------------
*/
int charcommand_baselevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int level = 0, i, status_point=0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: #baselvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change base level only lower or same gm level

			if (level > 0) {
				if (pl_sd->status.base_level == pc_maxbaselv(sd)) {	// check for max level by Valaris
					clif_displaymessage(fd, msg_table[91]); // Character's base level can't go any higher.
					return 0;
				}	// End Addition
				if ((unsigned int)level > pc_maxbaselv(pl_sd) ||
					pl_sd->status.base_level > pc_maxbaselv(pl_sd) -level)
					level = pc_maxbaselv(pl_sd) - pl_sd->status.base_level;
				for (i = 1; i <= level; i++)
					status_point += (pl_sd->status.base_level + i + 14) / 5;
				if (pl_sd->status.status_point > USHRT_MAX -  status_point)
					pl_sd->status.status_point = USHRT_MAX;
				else
					pl_sd->status.status_point += status_point;
				pl_sd->status.base_level += (unsigned int)level;
				clif_updatestatus(pl_sd, SP_BASELEVEL);
				clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
				clif_updatestatus(pl_sd, SP_STATUSPOINT);
				status_calc_pc(pl_sd, 0);
				status_percent_heal(&pl_sd->bl, 100, 100);
				clif_misceffect(&pl_sd->bl, 0);
				clif_displaymessage(fd, msg_table[65]); // Character's base level raised.
			} else {
				if (pl_sd->status.base_level == 1) {
					clif_displaymessage(fd, msg_table[193]); // Character's base level can't go any lower.
					return -1;
				}
				level *= -1;
				if ((unsigned int)level >= pl_sd->status.base_level)
					level = pl_sd->status.base_level -1;
				if (pl_sd->status.status_point > 0) {
					for (i = 0; i > -level; i--)
						status_point += (pl_sd->status.base_level +i + 14) / 5;
					if (pl_sd->status.status_point < status_point)
						pc_resetstate(pl_sd);
					if (pl_sd->status.status_point < status_point)
						pl_sd->status.status_point = 0;
					else
						pl_sd->status.status_point -= status_point;
					clif_updatestatus(pl_sd, SP_STATUSPOINT);
				} // to add: remove status points from stats
				pl_sd->status.base_level -= (unsigned int)level;
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

	return 0; //正常終了
}

/*==========================================
 * #jlvl <#> <nickname> 
 * Transferred by: Kevin
 *------------------------------------------
 */
int charcommand_joblevel(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int level = 0;
	//転生や養子の場合の元の職業を算出する
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustement and a player name (usage: #joblvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job level only lower or same gm level
			if (level > 0) {
				if (pl_sd->status.job_level == pc_maxjoblv(pl_sd)) {
					clif_displaymessage(fd, msg_table[67]); // Character's job level can't go any higher.
					return -1;
				}
				if ((unsigned int)level > pc_maxjoblv(pl_sd) ||
					pl_sd->status.job_level > pc_maxjoblv(pl_sd) -level)
					level = pc_maxjoblv(pl_sd) - pl_sd->status.job_level;
				pl_sd->status.job_level += (unsigned int)level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);

				if (pl_sd->status.skill_point > USHRT_MAX - level)
					pl_sd->status.skill_point = USHRT_MAX;
				else
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
				level*=-1;
				if ((unsigned int)level >= pl_sd->status.job_level)
					level = pl_sd->status.job_level-1;
				pl_sd->status.job_level -= (unsigned int)level;
				clif_updatestatus(pl_sd, SP_JOBLEVEL);
				clif_updatestatus(pl_sd, SP_NEXTJOBEXP);
				if (pl_sd->status.skill_point < level)
					pc_resetskill(pl_sd, 0); //Need more skill points to substract
				if (pl_sd->status.skill_point < level)
					pl_sd->status.skill_point = 0;
				else
					pl_sd->status.skill_point -= level;
				clif_updatestatus(pl_sd, SP_SKILLPOINT);
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
 * #questskill <skill_#> <nickname>
 * Transferred by: Kevin
 *------------------------------------------
 */
int charcommand_questskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int skill_id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: #questskill <#:0+> <nickname>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL_DB) {
		if (skill_get_inf2(skill_id) & INF2_QUEST_SKILL) {
			if ((pl_sd = map_nick2sd(player)) != NULL) {
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
 * #lostskill <skill_#> <nickname>
 * Transferred by: Kevin
 *------------------------------------------
 */
int charcommand_lostskill(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int skill_id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charlostskill <#:0+> <char_name>).");
		return -1;
	}

	if (skill_id >= 0 && skill_id < MAX_SKILL) {
		if (skill_get_inf2(skill_id) & INF2_QUEST_SKILL) {
			if ((pl_sd = map_nick2sd(player)) != NULL) {
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
 * Character Skill Reset
 *------------------------------------------
 */
int charcommand_skreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	char tmp_cmdoutput[1024];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charskreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset skill points only lower or same gm level
			pc_resetskill(pl_sd,1);
			sprintf(tmp_cmdoutput, msg_table[206], player); // '%s' skill points reseted!
			clif_displaymessage(fd, tmp_cmdoutput);
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
int charcommand_streset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	char tmp_cmdoutput[1024];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charstreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset stats points only lower or same gm level
			pc_resetstate(pl_sd);
			sprintf(tmp_cmdoutput, msg_table[207], player); // '%s' stats points reseted!
			clif_displaymessage(fd, tmp_cmdoutput);
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
int charcommand_model(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	int hair_style = 0, hair_color = 0, cloth_color = 0;
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	char tmp_cmdoutput[1024];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %d %d %23[^\n]", &hair_style, &hair_color, &cloth_color, player) < 4 || hair_style < 0 || hair_color < 0 || cloth_color < 0) {
		sprintf(tmp_cmdoutput, "Please, enter a valid model and a player name (usage: @charmodel <hair ID: %d-%d> <hair color: %d-%d> <clothes color: %d-%d> <name>).",
				MIN_HAIR_STYLE, MAX_HAIR_STYLE, MIN_HAIR_COLOR, MAX_HAIR_COLOR, MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd, tmp_cmdoutput);
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE &&
			hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR &&
			cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
			/* Removed this check for being too strange. [Skotlex]
			if (cloth_color != 0 &&
				pl_sd->status.sex == 1 &&
				(pl_sd->status.class_ == JOB_ASSASSIN ||  pl_sd->status.class_ == JOB_ROGUE)) {
				clif_displaymessage(fd, msg_table[35]); // You can't use this command with this class.
				return -1;
			} else {
			*/
				pc_changelook(pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_table[36]); // Appearence changed.
//			}
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
int charcommand_skpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int new_skill_point;
	int point = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &point, player) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charskpoint <amount> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (point > 0 && pl_sd->status.skill_point > USHRT_MAX - point)
			new_skill_point = USHRT_MAX;
		else if (point < 0 && pl_sd->status.skill_point < -point)
			new_skill_point = 0;
		else
			new_skill_point = pl_sd->status.skill_point + point;
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
int charcommand_stpoint(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int new_status_point;
	int point = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &point, player) < 2 || point == 0) {
		clif_displaymessage(fd, "Please, enter a number and a player name (usage: @charstpoint <amount> <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (point > 0 && pl_sd->status.status_point > USHRT_MAX - point)
			new_status_point = USHRT_MAX;
		else if (point < 0 && pl_sd->status.status_point < -point)
			new_status_point = 0;
		else
			new_status_point = pl_sd->status.status_point + point;
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
 * charchangesex command (usage: charchangesex <player_name>)
 *------------------------------------------
 */
int charcommand_changesex(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char player[NAME_LENGTH];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charchangesex <name>).");
		return -1;
	}

	// check player name
	if (strlen(player) < 4) {
		clif_displaymessage(fd, msg_table[86]); // Sorry, but a player name have at least 4 characters.
		return -1;
	} else if (strlen(player) > 23) {
		clif_displaymessage(fd, msg_table[87]); // Sorry, but a player name have 23 characters maximum.
		return -1;
	} else {
		chrif_char_ask_name(sd->status.account_id, player, 5, 0, 0, 0, 0, 0, 0); // type: 5 - changesex
		clif_displaymessage(fd, msg_table[88]); // Character name sends to char-server to ask it.
	}

	return 0;
}

/*==========================================
 * Feel (SG save map) Reset
 *------------------------------------------
 */
int charcommand_feelreset(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	malloc_tsetdword(character, '\0', sizeof(character));
	malloc_tsetdword(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #feelreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset a character only for lower or same GM level
			pc_resetfeel(pl_sd);
			sprintf(output, msg_table[267], character); // '%s' designated maps reseted!
			clif_displaymessage(fd, output);
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
 * #help - Char commands [Kayla]
 *------------------------------------------
 */
int charcommand_help(
	const int fd, struct map_session_data* sd,
	const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;
	nullpo_retr(-1, sd);

	malloc_tsetdword(buf, '\0', sizeof(buf));

	if ((fp = fopen(charhelp_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_table[26]); /* Help commands: */
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
		clif_displaymessage(fd, msg_table[27]); /*  File help.txt not found. */
		return -1;
	}

	return 0;
}
