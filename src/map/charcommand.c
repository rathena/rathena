// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "atcommand.h" // msg_txt()
#include "charcommand.h"
#include "battle.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "itemdb.h"
#include "log.h"
#include "map.h"
#include "pc.h"
#include "status.h"
#include "skill.h"
#include "mob.h"
#include "npc.h"
#include "pet.h"
#include "mercenary.h"
#include "party.h"
#include "guild.h"
#include "script.h"
#include "trade.h"
#include "unit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

char charcommand_symbol = '#';

extern char *msg_table[1000]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

#define CCMD_FUNC(x) int charcommand_ ## x (const int fd, struct map_session_data* sd, const char* command, const char* message)

typedef struct CharCommandInfo {
	const char* command;
	int level;
	CharCommandFunc func;
} CharCommandInfo;

static CharCommandInfo* get_charcommandinfo_byname(const char* name);
static CharCommandInfo* get_charcommandinfo_byfunc(const CharCommandFunc func);


/*==========================================
 * ëŒè€ÉLÉÉÉâÉNÉ^Å[Çì]êEÇ≥ÇπÇÈ upperéwíËÇ≈ì]ê∂Ç‚ó{éqÇ‡â¬î\
 *------------------------------------------*/
int charcommand_jobchange(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[100];
	struct map_session_data* pl_sd;
	int job = 0, upper = -1;

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &job, character) < 2) {
		clif_displaymessage(fd, "Please, enter a job and a player name (usage: #job/#jobchange <job ID> <player>).");
		clif_displaymessage(fd, "   0 Novice            7 Knight           14 Crusader         21 Peco Crusader");
		clif_displaymessage(fd, "   1 Swordman          8 Priest           15 Monk             22 N/A");
		clif_displaymessage(fd, "   2 Mage              9 Wizard           16 Sage             23 Super Novice");
		clif_displaymessage(fd, "   3 Archer           10 Blacksmith       17 Rogue            24 Gunslinger");
		clif_displaymessage(fd, "   4 Acolyte          11 Hunter           18 Alchemist        25 Ninja");
		clif_displaymessage(fd, "   5 Merchant         12 Assassin         19 Bard             26 N/A");
		clif_displaymessage(fd, "   6 Thief            13 Peco Knight      20 Dancer");
		clif_displaymessage(fd, "4001 Novice High    4008 Lord Knight      4015 Paladin        4022 Peco Paladin");
		clif_displaymessage(fd, "4002 Swordman High  4009 High Priest      4016 Champion");
		clif_displaymessage(fd, "4003 Mage High      4010 High Wizard      4017 Professor");
		clif_displaymessage(fd, "4004 Archer High    4011 Whitesmith       4018 Stalker");
		clif_displaymessage(fd, "4005 Acolyte High   4012 Sniper           4019 Creator");
		clif_displaymessage(fd, "4006 Merchant High  4013 Assassin Cross   4020 Clown");
		clif_displaymessage(fd, "4007 Thief High     4014 Peco Lord Knight 4021 Gypsy");
		clif_displaymessage(fd, "4023 Baby Novice    4030 Baby Knight      4037 Baby Crusader  4044 Baby Peco Crusader");
		clif_displaymessage(fd, "4024 Baby Swordsman 4031 Baby Priest      4038 Baby Monk      4045 Super Baby");
		clif_displaymessage(fd, "4025 Baby Mage      4032 Baby Wizard      4039 Baby Sage      4046 Taekwon Kid");
		clif_displaymessage(fd, "4026 Baby Archer    4033 Baby Blacksmith  4040 Baby Rogue     4047 Taekwon Master");
		clif_displaymessage(fd, "4027 Baby Acolyte   4034 Baby Hunter      4041 Baby Alchemist 4048 N/A");
		clif_displaymessage(fd, "4028 Baby Merchant  4035 Baby Assassin    4042 Baby Bard      4049 Soul Linker");
		clif_displaymessage(fd, "4029 Baby Thief     4036 Baby Peco-Knight 4043 Baby Dancer");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pcdb_checkid(job)) {
		clif_displaymessage(fd, msg_txt(49)); // Invalid job ID.
		return -1;
	}

	if (pc_jobchange(pl_sd, job, upper) != 0) {
		clif_displaymessage(fd, msg_txt(192)); // Impossible to change the character's job.
		return -1;
	}

	clif_displaymessage(fd, msg_txt(48)); // Character's job changed.
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int charcommand_petrename(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct pet_data *pd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #petrename <player>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (!pl_sd->status.pet_id || !pl_sd->pd) {
		clif_displaymessage(fd, msg_txt(191)); // Sorry, but this player has no pet.
		return -1;
	}

	pd = pl_sd->pd;

	if (pd->pet.rename_flag) {
		clif_displaymessage(fd, msg_txt(190)); // This player can already rename his/her pet.
		return -1;
	}
	pd->pet.rename_flag = 0;
	intif_save_petdata(pl_sd->status.account_id, &pd->pet);
	clif_send_petstatus(pl_sd);
	clif_displaymessage(fd, msg_txt(189)); // This player can now rename his/her pet.
	return 0;
}


/*==========================================
 * 
 *------------------------------------------*/
int charcommand_petfriendly(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int friendly = 0;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct pet_data *pd;

	memset(character, '\0', sizeof(character));
	if (!message || !*message || sscanf(message,"%d %23s",&friendly,character) < 2) {
		clif_displaymessage(fd, "Please, enter a valid value (usage: #petfriendly <0-1000> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pl_sd->status.pet_id  || !pl_sd->pd) {
		clif_displaymessage(fd, msg_txt(191)); // Sorry, but this player has no pet.
		return -1;
	}

	friendly = cap_value(friendly, 0, 1000);

	pd = pl_sd->pd;
	if (friendly == pd->pet.intimate) {
		clif_displaymessage(fd, msg_txt(183)); // Pet friendly is already the good value.
		return -1;
	}
	
	pd->pet.intimate = friendly;
	clif_send_petstatus(pl_sd);
	clif_pet_emotion(pd,0);
	clif_displaymessage(pl_sd->fd, msg_txt(182)); // Pet friendly value changed!
	clif_displaymessage(sd->fd, msg_txt(182)); // Pet friendly value changed!
	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int charcommand_stats(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char job_jobname[100];
	char output[200];
	struct map_session_data *pl_sd;
	int i;
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
		{ "Free SK Points - %d", 0 },
		{ "JobChangeLvl - %d", 0 },
		{ NULL, 0 }
	};

	memset(character, '\0', sizeof(character));
	memset(job_jobname, '\0', sizeof(job_jobname));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #stats <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

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
	output_table[13].value = pl_sd->status.skill_point;
	output_table[14].value = pl_sd->change_level;

	sprintf(job_jobname, "Job - %s %s", job_name(pl_sd->status.class_), "(level %d)");
	sprintf(output, msg_txt(53), pl_sd->status.name); // '%s' stats:

	clif_displaymessage(fd, output);
	
	for (i = 0; output_table[i].format != NULL; i++) {
		sprintf(output, output_table[i].format, output_table[i].value);
		clif_displaymessage(fd, output);
	}

	return 0;
}

/*==========================================
 * Character Reset
 *------------------------------------------*/
int charcommand_reset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #reset <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	pc_resetstate(pl_sd);
	pc_resetskill(pl_sd,1);
	sprintf(output, msg_txt(208), character); // '%s' skill and stats points reseted!
	clif_displaymessage(fd, output);

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int charcommand_option(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	int opt1 = 0, opt2 = 0, opt3 = 0;
	struct map_session_data* pl_sd;

	memset(character, '\0', sizeof(character));

	if (!message || !*message ||
		sscanf(message, "%d %d %d %23[^\n]", &opt1, &opt2, &opt3, character) < 4 ||
		opt1 < 0 || opt2 < 0 || opt3 < 0) {
		clif_displaymessage(fd, "Please, enter valid options and a player name (usage: #option <param1> <param2> <param3> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	pl_sd->sc.opt1 = opt1;
	pl_sd->sc.opt2 = opt2;
	pc_setoption(pl_sd, opt3);
	clif_displaymessage(fd, msg_txt(58)); // Character's options changed.

	return 0;
}

/*==========================================
 *
 *------------------------------------------*/
int charcommand_save(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char map_name[MAP_NAME_LENGTH_EXT];
	char character[NAME_LENGTH];
	struct map_session_data* pl_sd;
	int x = 0, y = 0;
	int m;

	memset(map_name, '\0', sizeof(map_name));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%15s %d %d %23[^\n]", map_name, &x, &y, character) < 4 || x < 0 || y < 0) {
		clif_displaymessage(fd, "Please, enter a valid save point and a player name (usage: #save <map> <x> <y> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	m = map_mapname2mapid(map_name);
	if (m < 0 && !mapindex_name2id(map_name)) {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}

	//FIXME: What do you do if the map is in another map server with the nowarpto flag?
	if (m>=0 && map[m].flag.nosave && battle_config.any_warp_GM_min_level > pc_isGM(sd)) {
		clif_displaymessage(fd, "You are not authorised to set this map as a save map.");
		return -1;
	}

	if (m>=0)
		pc_setsavepoint(pl_sd, map[m].index, x, y);
	else
		pc_setsavepoint(pl_sd, mapindex_name2id(map_name), x, y);
	clif_displaymessage(fd, msg_txt(57)); // Character's respawn point changed.

	return 0;
}

/*==========================================
 * CharSpiritBall Function by PalasX
 *------------------------------------------*/
int charcommand_spiritball(const int fd, struct map_session_data* sd,const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[NAME_LENGTH];
	int spirit = 0;

	memset(character, '\0', sizeof(character));

	if(!message || !*message || sscanf(message, "%d %23[^\n]", &spirit, character) < 2 || spirit < 0 || spirit > 1000) {
		clif_displaymessage(fd, "Usage: @spiritball <number: 0-1000> <player>.");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if (spirit >= 0 && spirit <= 0x7FFF) {
		if (pl_sd->spiritball != spirit || spirit > 999) {
			if (pl_sd->spiritball > 0)
				pc_delspiritball(pl_sd, pl_sd->spiritball, 1);
		}
		pl_sd->spiritball = spirit;
		clif_spiritball(pl_sd);
		// no message, player can look the difference
		if (spirit > 1000)
			clif_displaymessage(fd, msg_txt(204)); // WARNING: more than 1000 spiritballs can CRASH your server and/or client!
	} else {
		clif_displaymessage(fd, msg_txt(205)); // You already have this number of spiritballs.
		return -1;
	}

	return 0;
}

/*==========================================
 * #itemlist <character>: Displays the list of a player's items.
 *------------------------------------------*/
int charcommand_itemlist(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, equip, count, counter, counter2;
	char character[NAME_LENGTH], output[200], equipstr[100], outputtmp[200];
	struct item *i_item; //Current inventory item.
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(equipstr, '\0', sizeof(equipstr));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

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
				memset(equipstr, '\0', sizeof(equipstr));
			if (i_item->refine)
				sprintf(output, "%d %s %+d (%s %+d, id: %d) %s", i_item->amount, item_data->name, i_item->refine, item_data->jname, i_item->refine, i_item->nameid, equipstr);
			else
				sprintf(output, "%d %s (%s, id: %d) %s", i_item->amount, item_data->name, item_data->jname, i_item->nameid, equipstr);
			clif_displaymessage(fd, output);
			memset(output, '\0', sizeof(output));
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
	{
		clif_displaymessage(fd, "No item found on this player.");
		return -1;
	}

	sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
	clif_displaymessage(fd, output);
	return 0;
}

/*==========================================
 * #effect by [MouseJstr]
 *
 * Create a effect localized on another character
 *------------------------------------------*/
int charcommand_effect(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd = NULL;
	char character[NAME_LENGTH];
	int type = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &type, character) != 2) {
		clif_displaymessage(fd, "usage: #effect <type+> <target>.");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	clif_specialeffect(&pl_sd->bl, type, AREA);
	clif_displaymessage(fd, msg_txt(229)); // Your effect has changed.

	return 0;
}

/*==========================================
 * #storagelist <character>: Displays the items list of a player's storage.
 *------------------------------------------*/
int charcommand_storagelist(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct storage *stor;
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count, counter, counter2;
	char character[NAME_LENGTH], output[200], outputtmp[200];
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemlist <char name>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

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
				memset(output, '\0', sizeof(output));
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

	return 0;
}

static void charcommand_giveitem_sub(struct map_session_data *sd,struct item_data *item_data,int number)
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
			memset(&item_tmp, 0, sizeof(item_tmp));
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
 *------------------------------------------*/
int charcommand_item(const int fd, struct map_session_data* sd, const char* command, const char* message)
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

	memset(item_name, '\0', sizeof(item_name));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %23[^\n]", item_name, &number, character) < 3 &&
		sscanf(message, "%99s %d %23[^\n]", item_name, &number, character) < 3
	)) {
		clif_displaymessage(fd, "Please, enter an item name/id (usage: #item <item name or ID> <quantity> <char name>).");
		return -1;
	}

	if (number <= 0)
		number = 1;

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
		(item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;

	if (item_id < 500) {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	get_count = number;
	// check pet egg
	pet_id = search_petDB_index(item_id, PET_EGG);
	if (item_data->type == 4 || item_data->type == 5 ||
		item_data->type == 7 || item_data->type == 8) {
		get_count = 1;
	}

	if ((pl_sd = map_nick2sd(character)) == NULL)
	{
		if (pc_isGM(sd) < pc_isGM(pl_sd))
		{// you can give items only to lower or same level
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
		else
		{
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
					memset(&item_tmp, 0, sizeof(item_tmp));
					item_tmp.nameid = item_id;
					item_tmp.identify = 1;

					if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
						clif_additem(pl_sd, 0, 0, flag);
				}
			}

			//Logs (A)dmins items [Lupus]
			if(log_config.enable_logs&0x400)
				log_pick_pc(sd, "A", item_tmp.nameid, number, &item_tmp);

			clif_displaymessage(fd, msg_txt(18)); // Item created.
		}
	} else
	if (strcmpi(character,"all")==0 || strcmpi(character,"everyone")==0) {
		struct s_mapiterator* iter = mapit_getallusers();
		for( pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter) )
		{
			charcommand_giveitem_sub(pl_sd,item_data,number);
			snprintf(tmp_cmdoutput, sizeof(tmp_cmdoutput), "You got %s %d.", item_name,number);
			clif_displaymessage(pl_sd->fd, tmp_cmdoutput);
		}
		mapit_free(iter);

		snprintf(tmp_cmdoutput, sizeof(tmp_cmdoutput), "%s received %s %d.","Everyone",item_name,number);
		clif_displaymessage(fd, tmp_cmdoutput);
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * #warp/#rura/#rura+ <mapname> <x> <y> <char name>
 *------------------------------------------*/
int charcommand_warp(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char map_name[MAP_NAME_LENGTH_EXT];
	char character[NAME_LENGTH];
	int x = 0, y = 0;
	struct map_session_data *pl_sd;
	int m;

	nullpo_retr(-1, sd);

	memset(map_name, '\0', sizeof(map_name));
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%15s %d %d %23[^\n]", map_name, &x, &y, character) < 4) {
		clif_displaymessage(fd, "Usage: #warp/#rura/#rura+ <mapname> <x> <y> <char name>");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	if (pc_isGM(sd) < pc_isGM(pl_sd)) { // you can rura+ only lower or same GM level
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	m = map_mapname2mapid(map_name);
	if (m < 0) {
		clif_displaymessage(fd, msg_txt(1)); // Map not found.
		return -1;
	}
	if ((x || y) && map_getcell(m, x, y, CELL_CHKNOREACH)) {
		clif_displaymessage(fd, msg_txt(2)); // Coordinates out of range.
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
		clif_displaymessage(pl_sd->fd, msg_txt(0)); // Warped.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(15)); // Player warped (message sends to player too).
		return 0;
	}
	//No error message specified...?
	return -1;
}

/*==========================================
 * #zeny <charname>
 *------------------------------------------*/
int charcommand_zeny(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[NAME_LENGTH];
	int zeny = 0, new_zeny;
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));

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
			clif_displaymessage(fd, msg_txt(211)); // Character's number of zenys changed!
		} else {
			if (zeny < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * #fakename <char name> <fake name>
 *------------------------------------------*/
int charcommand_fakename(const int fd, struct map_session_data* sd, const char* command, const char* message)
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

	if(!name[0]) {
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
	
	memcpy(pl_sd->fakename, name, NAME_LENGTH);
	clif_charnameack(0, &pl_sd->bl);
	clif_displaymessage(sd->fd,"Fake name enabled.");
	
	return 0;
}


/*==========================================
 * #baselvl <#> <nickname> 
 * Transferred by: Kevin
 *------------------------------------------*/
int charcommand_baselevel(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int level = 0, i, status_point=0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustment and a player name (usage: #baselvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) == NULL) {
		clif_displaymessage(fd, msg_table[3]); // Character not found.
		return -1;
	}
	if (pc_isGM(sd) < pc_isGM(pl_sd)) { // you can change base level only lower or same gm level
		clif_displaymessage(fd, msg_table[81]); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (level > 0) {
		if (pl_sd->status.base_level == pc_maxbaselv(pl_sd)) {	// check for max level by Valaris
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
		for (i = 0; i > -level; i--)
			status_point += (pl_sd->status.base_level +i + 14) / 5;
		if (pl_sd->status.status_point < status_point)
			pc_resetstate(pl_sd);
		if (pl_sd->status.status_point < status_point)
			pl_sd->status.status_point = 0;
		else
			pl_sd->status.status_point -= status_point;
		pl_sd->status.base_level -= (unsigned int)level;
		clif_displaymessage(fd, msg_table[66]); // Character's base level lowered.
	}
	clif_updatestatus(pl_sd, SP_BASELEVEL);
	clif_updatestatus(pl_sd, SP_NEXTBASEEXP);
	clif_updatestatus(pl_sd, SP_STATUSPOINT);
	status_calc_pc(pl_sd, 0);
	if(pl_sd->status.party_id) 
		party_send_levelup(pl_sd); 
	return 0; //ê≥èÌèIóπ
}

/*==========================================
 * #jlvl <#> <nickname> 
 * Transferred by: Kevin
 *------------------------------------------*/
int charcommand_joblevel(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int level = 0;
	//ì]ê∂Ç‚ó{éqÇÃèÍçáÇÃå≥ÇÃêEã∆ÇéZèoÇ∑ÇÈ
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, player) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a level adjustment and a player name (usage: #joblvl <#> <nickname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can change job level only lower or same gm level
			if (level > 0) {
				if (pl_sd->status.job_level == pc_maxjoblv(pl_sd)) {
					clif_displaymessage(fd, msg_txt(67)); // Character's job level can't go any higher.
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
				clif_displaymessage(fd, msg_txt(68)); // character's job level raised.
			} else {
				if (pl_sd->status.job_level == 1) {
					clif_displaymessage(fd, msg_txt(194)); // Character's job level can't go any lower.
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
				clif_displaymessage(fd, msg_txt(69)); // Character's job level lowered.
			}
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}


/*==========================================
 * #questskill <skill_#> <nickname>
 * Transferred by: Kevin
 *------------------------------------------*/
int charcommand_questskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int skill_id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: #questskill <#:0+> <nickname>).");
		return -1;
	}

	if (skill_id < 0 && skill_id >= MAX_SKILL_DB) {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}
	if (!(skill_get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return -1;
	}
	if ((pl_sd = map_nick2sd(player)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	if (pc_checkskill(pl_sd, skill_id) > 0) {
		clif_displaymessage(fd, msg_txt(200)); // This player already has this quest skill.
		return -1;
	}

	pc_skill(pl_sd, skill_id, 1, 0);
	clif_displaymessage(fd, msg_txt(199)); // This player has learned the skill.

	return 0;
}


/*==========================================
 * #lostskill <skill_#> <nickname>
 * Transferred by: Kevin
 *------------------------------------------*/
int charcommand_lostskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	int skill_id = 0;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &skill_id, player) < 2 || skill_id < 0) {
		clif_displaymessage(fd, "Please, enter a quest skill number and a player name (usage: @charlostskill <#:0+> <char_name>).");
		return -1;
	}

	if (skill_id < 0 && skill_id >= MAX_SKILL) {
		clif_displaymessage(fd, msg_txt(198)); // This skill number doesn't exist.
		return -1;
	}
	if (!(skill_get_inf2(skill_id) & INF2_QUEST_SKILL)) {
		clif_displaymessage(fd, msg_txt(197)); // This skill number doesn't exist or isn't a quest skill.
		return -1;
	}
	if ((pl_sd = map_nick2sd(player)) == NULL) {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	if (pc_checkskill(pl_sd, skill_id) == 0) {
		clif_displaymessage(fd, msg_txt(203)); // This player doesn't have this quest skill.
		return -1;
	}

	pl_sd->status.skill[skill_id].lv = 0;
	pl_sd->status.skill[skill_id].flag = 0;
	clif_skillinfoblock(pl_sd);
	clif_displaymessage(fd, msg_txt(202)); // This player has forgotten the skill.

	return 0;
}

/*==========================================
 * Character Skill Reset
 *------------------------------------------*/
int charcommand_skreset(const int fd, struct map_session_data* sd, const char* command, const char* message)
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
			sprintf(tmp_cmdoutput, msg_txt(206), player); // '%s' skill points reseted!
			clif_displaymessage(fd, tmp_cmdoutput);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Stat Reset
 *------------------------------------------*/
int charcommand_streset(const int fd, struct map_session_data* sd, const char* command, const char* message)
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
			sprintf(tmp_cmdoutput, msg_txt(207), player); // '%s' stats points reseted!
			clif_displaymessage(fd, tmp_cmdoutput);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Model by chbrules
 *------------------------------------------*/
int charcommand_model(const int fd, struct map_session_data* sd, const char* command, const char* message)
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
				pc_changelook(pl_sd, LOOK_HAIR, hair_style);
				pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
				pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
				clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
		} else {
			clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Skill Point (Rewritten by [Yor])
 *------------------------------------------*/
int charcommand_skpoint(const int fd, struct map_session_data* sd, const char* command, const char* message)
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
			clif_displaymessage(fd, msg_txt(209)); // Character's number of skill points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * Character Status Point (rewritten by [Yor])
 *------------------------------------------*/
int charcommand_stpoint(const int fd, struct map_session_data* sd, const char* command, const char* message)
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
			clif_displaymessage(fd, msg_txt(210)); // Character's number of status points changed!
		} else {
			if (point < 0)
				clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
			else
				clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * charchangesex command (usage: charchangesex <player_name>)
 *------------------------------------------*/
int charcommand_changesex(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char player[NAME_LENGTH];
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^\n]", player) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: @charchangesex <name>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(player)) == NULL)
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}
	if (pc_isGM(sd) < pc_isGM(pl_sd)) {
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	clif_displaymessage(fd, msg_table[88]); // Character name sent to char-server to ask it.
	chrif_changesex(pl_sd);
	return 0;
}

/*==========================================
 * Feel (SG save map) Reset
 *------------------------------------------*/
int charcommand_feelreset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #feelreset <charname>).");
		return -1;
	}

	if ((pl_sd = map_nick2sd(character)) != NULL) {
		if (pc_isGM(sd) >= pc_isGM(pl_sd)) { // you can reset a character only for lower or same GM level
			pc_resetfeel(pl_sd);
			sprintf(output, msg_txt(267), character); // '%s' designated maps reseted!
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
			return -1;
		}
	} else {
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	return 0;
}

/*==========================================
 * #help - Char commands [Kayla]
 *------------------------------------------*/
int charcommand_help(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char buf[2048], w1[2048], w2[2048];
	int i, gm_level;
	FILE* fp;
	nullpo_retr(-1, sd);

	memset(buf, '\0', sizeof(buf));

	if ((fp = fopen(charhelp_txt, "r")) != NULL) {
		clif_displaymessage(fd, msg_txt(26)); /* Help commands: */
		gm_level = pc_isGM(sd);
		while(fgets(buf, sizeof(buf), fp) != NULL)
		{
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
		clif_displaymessage(fd, msg_txt(27)); /*  File help.txt not found. */
		return -1;
	}

	return 0;
}

/*==========================================
 * Loads a character back to their save point [HiddenDragon]
 *------------------------------------------*/
int charcommand_load(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int m;
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #load <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	m = map_mapindex2mapid(pl_sd->status.save_point.map);
	if (m >= 0 && map[m].flag.nowarpto && battle_config.any_warp_GM_min_level > pc_isGM(pl_sd)) {
		clif_displaymessage(fd, "Not authorized to warp to target map.");
		return -1;
	}
	if (pl_sd->bl.m >= 0 && map[pl_sd->bl.m].flag.nowarp && battle_config.any_warp_GM_min_level > pc_isGM(pl_sd)) {
		clif_displaymessage(fd, "Not authorized to warp from current map.");
		return -1;
	}

	pc_setpos(pl_sd, pl_sd->status.save_point.map, pl_sd->status.save_point.x, pl_sd->status.save_point.y, 0);
	clif_displaymessage(pl_sd->fd, msg_txt(7)); // Warping to respawn point.
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, msg_txt(7)); // Warping to respawn point.
	
	return 0;
}

/*==========================================
 * Changes the targets speed [HiddenDragon]
 *------------------------------------------*/
int charcommand_speed(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int speed;
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &speed, character) < 2) {
		sprintf(output, "Please, enter proper values (usage: #speed <%d-%d> <player>).", MIN_WALK_SPEED, MAX_WALK_SPEED);
		clif_displaymessage(fd, output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	pl_sd->base_status.speed = cap_value(speed, MIN_WALK_SPEED, MAX_WALK_SPEED);
	status_calc_bl(&pl_sd->bl, SCB_SPEED);
	clif_displaymessage(pl_sd->fd, msg_txt(8)); // Speed changed.
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, msg_txt(8)); // Speed changed.
	return 0;
}

/*==========================================
 * Opens their storage [HiddenDragon]
 *------------------------------------------*/
int charcommand_storage(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #storage <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (pl_sd->npc_id || pl_sd->vender_id || pl_sd->state.trading || pl_sd->state.storage_flag)
		return -1;

	if (storage_storageopen(pl_sd) == 1)
  	{	//Already open.
		clif_displaymessage(fd, "Players storage already open.");
		return -1;
	}
	
	clif_displaymessage(pl_sd->fd, "Storage opened.");
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, "Player's storage opened.");

	return 0;
}


/*==========================================
 * Opens their guild storage
 *------------------------------------------*/
int charcommand_guildstorage(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct storage *stor; //changes from Freya/Yor
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;
	
	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #gstorage <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (pl_sd->npc_id || pl_sd->vender_id || pl_sd->state.trading || pl_sd->state.storage_flag)
		return -1;

	if (pl_sd->status.guild_id > 0) {
		if (pl_sd->state.storage_flag) {
			clif_displaymessage(fd, "Guild storage is currently in use.");
			return -1;
		}
		if ((stor = account2storage2(pl_sd->status.account_id)) != NULL && stor->storage_status == 1) {
			clif_displaymessage(fd, "Guild storage is currently in use.");
			return -1;
		}
		storage_guild_storageopen(pl_sd);
	} else {
		clif_displaymessage(fd, "Target player is not in a guild.");
		return -1;
	}

	clif_displaymessage(pl_sd->fd, "Guild storage opened.");
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, "Player's guild storage opened.");

	return 0;
}

/*==========================================
 * Applies GM Hide to a character [HiddenDragon]
 *------------------------------------------*/
int charcommand_hide(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;
	
	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #hide <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (pl_sd->sc.option & OPTION_INVISIBLE) {
		pl_sd->sc.option &= ~OPTION_INVISIBLE;
		if (pl_sd->disguise)
			status_set_viewdata(&pl_sd->bl, pl_sd->disguise);
		else
			status_set_viewdata(&pl_sd->bl, pl_sd->status.class_);
		clif_changeoption(&pl_sd->bl);
		clif_displaymessage(pl_sd->fd, msg_txt(10)); // Invisible: Off
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(10)); // Invisible: Off
	} else {
		pl_sd->sc.option |= OPTION_INVISIBLE;
		pl_sd->vd.class_ = INVISIBLE_CLASS;
		clif_changeoption(&pl_sd->bl);
		clif_displaymessage(pl_sd->fd, msg_txt(11)); // Invisible: On
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(11)); // Invisible: On
	}

	return 0;
}

/*==========================================
 * Resurrects a dead character [HiddenDragon]
 *------------------------------------------*/
int charcommand_alive(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;
	
	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #alive <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (!status_revive(&pl_sd->bl, 100, 100))
	{
		clif_displaymessage(fd, "Target player is not dead.");
		return -1;
	}
	clif_skill_nodamage(&pl_sd->bl,&pl_sd->bl,ALL_RESURRECTION,4,1);
	clif_displaymessage(pl_sd->fd, msg_txt(16)); // You've been revived! It's a miracle!
	return 0;
}

/*==========================================
 * Heals someone's HP and SP [HiddenDragon]
 *------------------------------------------*/
int charcommand_heal(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hp = 0, sp = 0; // [Valaris] thanks to fov
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;
	
	if (!message || !*message || sscanf(message, "%d %d %23[^\n]", &hp, &sp, character) < 3) {
		clif_displaymessage(fd, "Please, enter proper values (usage: #heal <hp> <sp> <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (hp == 0 && sp == 0) {
		if (!status_percent_heal(&pl_sd->bl, 100, 100))
			clif_displaymessage(fd, msg_txt(157)); // HP and SP are already with the good value.
		else
		{
			clif_displaymessage(pl_sd->fd, msg_txt(17)); // HP, SP recovered.
			if (pl_sd->fd != fd)
				clif_displaymessage(fd, msg_txt(17)); // HP, SP recovered.
		}
		return 0;
	}
	
	if(hp > 0 && sp >= 0) {
		if(!status_heal(&pl_sd->bl, hp, sp, 2))
			clif_displaymessage(fd, msg_txt(157)); // HP and SP are already with the good value.
		else
		{
			clif_displaymessage(pl_sd->fd, msg_txt(17)); // HP, SP recovered.
			if (pl_sd->fd != fd)
				clif_displaymessage(fd, msg_txt(17)); // HP, SP recovered.
		}
		return 0;
	}

	if(hp < 0 && sp <= 0) {
		status_damage(NULL, &pl_sd->bl, -hp, -sp, 0, 0);
		clif_damage(&pl_sd->bl,&pl_sd->bl, gettick(), 0, 0, -hp, 0 , 4, 0);
		clif_displaymessage(pl_sd->fd, msg_txt(156)); // HP or/and SP modified.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(156)); // HP or/and SP modified.
		return 0;
	}

	//Opposing signs.
	if (hp) {
		if (hp > 0)
			status_heal(&pl_sd->bl, hp, 0, 2);
		else {
			status_damage(NULL, &pl_sd->bl, -hp, 0, 0, 0);
			clif_damage(&pl_sd->bl,&pl_sd->bl, gettick(), 0, 0, -hp, 0 , 4, 0);
		}
	}

	if (sp) {
		if (sp > 0)
			status_heal(&pl_sd->bl, 0, sp, 2);
		else
			status_damage(NULL, &pl_sd->bl, 0, -sp, 0, 0);
	}

	clif_displaymessage(pl_sd->fd, msg_txt(156)); // HP or/and SP modified.
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, msg_txt(156)); // HP or/and SP modified.
	return 0;
}

/*==========================================
 * Creates items as specified [HiddenDragon]
 *------------------------------------------*/
int charcommand_item2(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct item item_tmp;
	struct item_data *item_data;
	char item_name[100];
	int item_id, number = 0;
	int identify = 0, refine = 0, attr = 0;
	int c1 = 0, c2 = 0, c3 = 0, c4 = 0;
	int flag;
	int loop, get_count, i;
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;
	
	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d %d %d %d %d %d %d %23[^\n]", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4, character) < 10 &&
		sscanf(message, "%99s %d %d %d %d %d %d %d %d %23[^\n]", item_name, &number, &identify, &refine, &attr, &c1, &c2, &c3, &c4, character) < 10
	)) {
		clif_displaymessage(fd, "Please, enter all informations (usage: @item2 <item name or ID> <quantity>");
		clif_displaymessage(fd, "  <Identify_flag[1|0]> <refine> <attribut> <Card1> <Card2> <Card3> <Card4> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
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
			if ((flag = pc_additem(pl_sd, &item_tmp, get_count)))
				clif_additem(pl_sd, 0, 0, flag);
		}

		//Logs (A)dmins items [Lupus]
		if(log_config.enable_logs&0x400)
			log_pick_pc(pl_sd, "A", item_tmp.nameid, number, &item_tmp);

		clif_displaymessage(pl_sd->fd, msg_txt(18)); // Item created.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(18)); // Item created.
	} else {
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * Reset a character's items [HiddenDragon]
 *------------------------------------------*/
int charcommand_itemreset(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i, count = 0;
	char character[NAME_LENGTH];
	char output[200];

	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #itemreset <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	for (i = 0; i < MAX_INVENTORY; i++) {
		if (pl_sd->status.inventory[i].amount && pl_sd->status.inventory[i].equip == 0) {

			//Logs (A)dmins items [Lupus]
			if(log_config.enable_logs&0x400)
				log_pick_pc(pl_sd, "A", pl_sd->status.inventory[i].nameid, -pl_sd->status.inventory[i].amount, &pl_sd->status.inventory[i]);

			pc_delitem(pl_sd, i, pl_sd->status.inventory[i].amount, 0);
			count++;
		}
	}
	
	sprintf(output, msg_txt(114), count);
	if (pl_sd->fd != fd)
		clif_displaymessage(pl_sd->fd, output); // %d item(s) removed from the player.
	clif_displaymessage(fd, msg_txt(20)); // All of your items have been removed.

	return 0;
}

/*==========================================
 * Refine their items [HiddenDragon]
 *------------------------------------------*/
int charcommand_refine(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i,j, position = 0, refine = 0, current_position, final_refine;
	int count;
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %d %23[^\n]", &position, &refine, character) < 3) {
		clif_displaymessage(fd, "Please, enter a position and a amount (usage: #refine <equip position> <+/- amount> <player>).");
		sprintf(output, "%d: Left Accessory", EQI_ACC_L);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Right Accessory", EQI_ACC_R);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Shoes", EQI_SHOES);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Garment", EQI_GARMENT);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Lower Headgear", EQI_HEAD_LOW);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Mid Headgear", EQI_HEAD_MID);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Top Headgear", EQI_HEAD_TOP);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Body Armor", EQI_ARMOR);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Left Hand", EQI_HAND_L);
		clif_displaymessage(fd, output);
		sprintf(output, "%d: Right Hand", EQI_HAND_R);
		clif_displaymessage(fd, output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (refine < -MAX_REFINE)
		refine = -MAX_REFINE;
	else if (refine > MAX_REFINE)
		refine = MAX_REFINE;
	else if (refine == 0)
		refine = 1;

	count = 0;
	for (j = 0; j < EQI_MAX-1; j++) {
		if ((i = pl_sd->equip_index[j]) < 0)
			continue;
		if(j == EQI_HAND_R && pl_sd->equip_index[EQI_HAND_L] == i)
			continue;
		if(j == EQI_HEAD_MID && pl_sd->equip_index[EQI_HEAD_LOW] == i)
			continue;
		if(j == EQI_HEAD_TOP && (pl_sd->equip_index[EQI_HEAD_MID] == i || pl_sd->equip_index[EQI_HEAD_LOW] == i))
			continue;

		if(position && !(pl_sd->status.inventory[i].equip & position))
			continue;

		final_refine = pl_sd->status.inventory[i].refine + refine;
		if (final_refine > MAX_REFINE)
			final_refine = MAX_REFINE;
		else if (final_refine < 0)
			final_refine = 0;
		if (pl_sd->status.inventory[i].refine != final_refine) {
			pl_sd->status.inventory[i].refine = final_refine;
			current_position = pl_sd->status.inventory[i].equip;
			pc_unequipitem(pl_sd, i, 3);
			clif_refine(fd, 0, i, pl_sd->status.inventory[i].refine);
			clif_delitem(pl_sd, i, 1);
			clif_additem(pl_sd, i, 1, 0);
			pc_equipitem(pl_sd, i, current_position);
			clif_misceffect(&pl_sd->bl, 3);
			count++;
		}
	}

	if (count == 0)
		clif_displaymessage(fd, msg_txt(166)); // No item has been refined!
	else if (count == 1)
	{
		clif_displaymessage(pl_sd->fd, msg_txt(167)); // 1 item has been refined!
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(167)); // 1 item has been refined!
	}
	else {
		sprintf(output, msg_txt(168), count); // %d items have been refined!
		clif_displaymessage(pl_sd->fd, output);
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, output);
	}

	return 0;
}

/*==========================================
 * Produce a manufactured item in their inventory [HiddenDragon]
 *------------------------------------------*/
int charcommand_produce(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char item_name[100];
	int item_id, attribute = 0, star = 0;
	int flag = 0;
	struct item_data *item_data;
	struct item tmp_item;
	char character[NAME_LENGTH];
	char output[200];

	struct map_session_data *pl_sd;

	nullpo_retr(-1, sd);

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %d %23[^\n]", item_name, &attribute, &star, character) < 4 &&
		sscanf(message, "%99s %d %d %23[^\n]", item_name, &attribute, &star, character) < 4
	)) {
		clif_displaymessage(fd, "Please, enter at least an item name/id (usage: #produce <equip name or equip ID> <element> <# of very's> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) == NULL &&
	    (item_data = itemdb_exists(atoi(item_name))) == NULL)
	{
		sprintf(output, msg_txt(170)); // This item is not an equipment.
		clif_displaymessage(fd, output);
		return -1;
	}
	item_id = item_data->nameid;
	if (itemdb_isequip2(item_data)) {
		if (attribute < MIN_ATTRIBUTE || attribute > MAX_ATTRIBUTE)
			attribute = ATTRIBUTE_NORMAL;
		if (star < MIN_STAR || star > MAX_STAR)
			star = 0;
		memset(&tmp_item, 0, sizeof tmp_item);
		tmp_item.nameid = item_id;
		tmp_item.amount = 1;
		tmp_item.identify = 1;
		tmp_item.card[0] = CARD0_FORGE;
		tmp_item.card[1] = item_data->type==IT_WEAPON?
			((star*5) << 8) + attribute:0;
		tmp_item.card[2] = GetWord(pl_sd->status.char_id, 0);
		tmp_item.card[3] = GetWord(pl_sd->status.char_id, 1);
		clif_produceeffect(pl_sd, 0, item_id);
		clif_misceffect(&pl_sd->bl, 3);

		//Logs (A)dmins items [Lupus]
		if(log_config.enable_logs&0x400)
			log_pick_pc(pl_sd, "A", tmp_item.nameid, 1, &tmp_item);

		if ((flag = pc_additem(pl_sd, &tmp_item, 1)))
		{
			clif_additem(pl_sd, 0, 0, flag);
			clif_displaymessage(fd, msg_txt(18));
		}
	} else {
		sprintf(output, msg_txt(169), item_id, item_data->name); // This item (%d: '%s') is not an equipment.
		clif_displaymessage(fd, output);
		return -1;
	}

	return 0;
}

/*==========================================
 * Changes a character's stats [HiddenDragon
 *------------------------------------------*/
int charcommand_param(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int index, value = 0, new_value;
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;
	const char* param[] = { "#str", "#agi", "#vit", "#int", "#dex", "#luk", NULL };
	short* status[6];

	
	if (!message || !*message || sscanf(message, "%d %s", &value, character) < 2 || value == 0) {
		sprintf(output, "Please, enter a valid value (usage: #str,#agi,#vit,#int,#dex,#luk <+/-adjustment> <player>).");
		clif_displaymessage(fd, output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	status[0] = &pl_sd->status.str;
	status[1] = &pl_sd->status.agi;
	status[2] = &pl_sd->status.vit;
	status[3] = &pl_sd->status.int_;
	status[4] = &pl_sd->status.dex;
	status[5] = &pl_sd->status.luk;

	index = -1;
	for (index = 0; index < ARRAYLENGTH(param); index++) {
		if (strcmpi(command, param[index]) == 0)
			break;
	}
	if (index == ARRAYLENGTH(param) || index > MAX_STATUS_TYPE) {
		// normaly impossible...
		sprintf(output, "Please, enter a valid value (usage: #str,#agi,#vit,#int,#dex,#luk <+/-adjustment> <player>).");
		clif_displaymessage(fd, output);
		return -1;
	}

	if (value > 0 && *status[index] > 999 - value)
		new_value = 999;
	else if (value < 0 && *status[index] <= -value)
		new_value = 1;
	else
		new_value = *status[index] + value;
	
	if (new_value != (int)*status[index]) {
		*status[index] = new_value;
		clif_updatestatus(pl_sd, SP_STR + index);
		clif_updatestatus(pl_sd, SP_USTR + index);
		status_calc_pc(pl_sd, 0);
		clif_displaymessage(pl_sd->fd, msg_txt(42)); // Stat changed.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(42)); // Stat changed.
	} else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(41)); // Impossible to decrease the number/value.
		else
			clif_displaymessage(fd, msg_txt(149)); // Impossible to increase the number/value.
		return -1;
	}

	return 0;
}

/*==========================================
 * Levels up a character's guild [HiddenDragon]
 *------------------------------------------*/
int charcommand_guildlevelup(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int level = 0;
	short added_level;
	struct guild *guild_info;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %s", &level, character) < 2 || level == 0) {
		clif_displaymessage(fd, "Please, enter a valid level and player (usage: #guildlvup/@guildlvlup <# of levels> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (pl_sd->status.guild_id <= 0 || (guild_info = guild_search(pl_sd->status.guild_id)) == NULL) {
		clif_displaymessage(fd, "Target player is not in a guild"); // You're not in a guild.
		return -1;
	}
	//if (strcmp(sd->status.name, guild_info->master) != 0) {
	//	clif_displaymessage(fd, msg_txt(44)); // You're not the master of your guild.
	//	return -1;
	//}

	added_level = (short)level;
	if (level > 0 && (level > MAX_GUILDLEVEL || added_level > ((short)MAX_GUILDLEVEL - guild_info->guild_lv))) // fix positiv overflow
		added_level = (short)MAX_GUILDLEVEL - guild_info->guild_lv;
	else if (level < 0 && (level < -MAX_GUILDLEVEL || added_level < (1 - guild_info->guild_lv))) // fix negativ overflow
		added_level = 1 - guild_info->guild_lv;

	if (added_level != 0) {
		intif_guild_change_basicinfo(guild_info->guild_id, GBI_GUILDLV, &added_level, 2);
		clif_displaymessage(pl_sd->fd, msg_txt(179)); // Guild level changed.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(179));
	} else {
		clif_displaymessage(fd, msg_txt(45)); // Guild level change failed.
		return -1;
	}

	return 0;
}

/*==========================================
 * Opens a hatch window for them [HiddenDragon]
 *------------------------------------------*/
int charcommand_hatch(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #hatch <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (pl_sd->status.pet_id <= 0)
		clif_sendegg(sd);
	else {
		clif_displaymessage(fd, "Target player already has a pet"); // You already have a pet.
		return -1;
	}

	return 0;
}

/*==========================================
 * Change target pet's hunger [HiddenDragon]
 *------------------------------------------*/
int charcommand_pethungry(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hungry;
	struct pet_data *pd;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &hungry, character) < 2) {
		clif_displaymessage(fd, "Please, enter a valid number and player (usage: #pethungry <0-100> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	pd = pl_sd->pd;
	if (!pl_sd->status.pet_id || !pd) {
		clif_displaymessage(fd, "Target has no pet"); // Sorry, but you have no pet.
		return -1;
	}
	if (hungry < 0 || hungry > 100) {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}
	if (hungry == pd->pet.hungry) {
		clif_displaymessage(fd, msg_txt(186)); // Pet hungry is already the good value.
		return -1;
	}

	pd->pet.hungry = hungry;
	clif_send_petstatus(pl_sd);
	clif_displaymessage(pl_sd->fd, msg_txt(185)); // Pet hungry value changed!
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, msg_txt(185)); // Pet hungry value changed!

	return 0;
}

/*==========================================
 * Give all skills to target [HiddenDragon]
 *------------------------------------------*/
int charcommand_allskill(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];

	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #allskill <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	pc_allskillup(pl_sd); // all skills
	pl_sd->status.skill_point = 0; // 0 skill points
	clif_updatestatus(pl_sd, SP_SKILLPOINT); // update
	clif_displaymessage(pl_sd->fd, msg_txt(76)); // You have received all skills.
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, "Player has received all skills for his/her class.");

	return 0;
}


/*==========================================
 * Change target's clothing color [HiddenDragon]
 *------------------------------------------*/
int charcommand_dye(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int cloth_color = 0;
	char character[NAME_LENGTH];
	char output[200];
	
	struct map_session_data *pl_sd;

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &cloth_color, character) < 2) {
		sprintf(output, "Please, enter a clothes color (usage: #dye/@ccolor <clothes color: %d-%d> <player>).", MIN_CLOTH_COLOR, MAX_CLOTH_COLOR);
		clif_displaymessage(fd,output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if (cloth_color >= MIN_CLOTH_COLOR && cloth_color <= MAX_CLOTH_COLOR) {
		pc_changelook(pl_sd, LOOK_CLOTHES_COLOR, cloth_color);
		clif_displaymessage(pl_sd->fd, msg_txt(36)); // Appearence changed.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * Change target's hair style [HiddenDragon]
 *------------------------------------------*/
int charcommand_hair_style(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_style = 0;
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &hair_style, character) < 2) {
		sprintf(output, "Please, enter a hair style (usage: #hairstyle/#hstyle <hair ID: %d-%d> <player>.", MIN_HAIR_STYLE, MAX_HAIR_STYLE);
		clif_displaymessage(fd, output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (hair_style >= MIN_HAIR_STYLE && hair_style <= MAX_HAIR_STYLE) {
			pc_changelook(pl_sd, LOOK_HAIR, hair_style);
			clif_displaymessage(pl_sd->fd, msg_txt(36)); // Appearence changed.
			if (pl_sd->fd != fd)
				clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * Change target's hair color [HiddenDragon]
 *------------------------------------------*/
int charcommand_hair_color(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hair_color = 0;
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;
	nullpo_retr(-1, sd);

	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &hair_color, character) < 2) {
		sprintf(output, "Please, enter a hair color (usage: #haircolor/#hcolor <hair color: %d-%d> <player>).", MIN_HAIR_COLOR, MAX_HAIR_COLOR);
		clif_displaymessage(fd, output);
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (hair_color >= MIN_HAIR_COLOR && hair_color <= MAX_HAIR_COLOR) {
			pc_changelook(pl_sd, LOOK_HAIR_COLOR, hair_color);
			clif_displaymessage(pl_sd->fd, msg_txt(36)); // Appearence changed.
			if (pl_sd->fd != fd)
				clif_displaymessage(fd, msg_txt(36)); // Appearence changed.
	} else {
		clif_displaymessage(fd, msg_txt(37)); // An invalid number was specified.
		return -1;
	}

	return 0;
}

/*==========================================
 * Change all target's stats [HiddenDragon]
 *------------------------------------------*/
int charcommand_allstats(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int index, count, value = 0, max, new_value;
	short* status[6];
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;

 	//we don't use direct initialization because it isn't part of the c standard.
	nullpo_retr(-1, sd);
	memset(character, '\0', sizeof(character));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #statall/#statsall/#allstat/#allstats <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	status[0] = &pl_sd->status.str;
	status[1] = &pl_sd->status.agi;
	status[2] = &pl_sd->status.vit;
	status[3] = &pl_sd->status.int_;
	status[4] = &pl_sd->status.dex;
	status[5] = &pl_sd->status.luk;

	count = 0;
	max = pc_maxparameter(pl_sd);
	for (index = 0; index < ARRAYLENGTH(status); index++) {

		if (value > 0 && *status[index] > max - value)
			new_value = max;
		else if (value < 0 && *status[index] <= -value)
			new_value = 1;
		else
			new_value = *status[index] +value;
		
		if (new_value != (int)*status[index]) {
			*status[index] = new_value;
			clif_updatestatus(pl_sd, SP_STR + index);
			clif_updatestatus(pl_sd, SP_USTR + index);
			status_calc_pc(pl_sd, 0);
			count++;
		}
	}

	if (count > 0) // if at least 1 stat modified
	{
		clif_displaymessage(pl_sd->fd, msg_txt(84)); // All stats changed!
		if (fd != pl_sd->fd)
			clif_displaymessage(fd, "Player's stats changed"); // All stats changed!
	}
	else {
		if (value < 0)
			clif_displaymessage(fd, msg_txt(177)); // Impossible to decrease a stat.
		else
			clif_displaymessage(fd, msg_txt(178)); // Impossible to increase a stat.
		return -1;
	}

	return 0;
}

/*==========================================
 * Gives/Removes a peco from a player [HiddenDragon]
 *------------------------------------------*/
int charcommand_mount_peco(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char character[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #mount/#mountpeco <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pc_isriding(pl_sd))
	{ // if actually no peco
		if (!pc_checkskill(pl_sd, KN_RIDING))
		{
			clif_displaymessage(fd, msg_txt(217));	// Player cannot mount a PecoePeco with his/her job.
			return -1;
		}

		if (pl_sd->disguise)
		{
			clif_displaymessage(fd, msg_txt(215)); // Player cannot mount a PecoPeco while in disguise.
			return -1;
		}

		pc_setoption(pl_sd, pl_sd->sc.option | OPTION_RIDING);
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, "Player mounted a peco.");
		clif_displaymessage(pl_sd->fd, msg_txt(216));	// Mounted Peco.
	}
	else
	{	//Dismount
		pc_setoption(pl_sd, pl_sd->sc.option & ~OPTION_RIDING);
		clif_displaymessage(pl_sd->fd, msg_txt(218)); // Unmounted Peco.
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, "Player unmounted a peco.");
	}

	return 0;
}

/*==========================================
 * Remove items from a player (now a char command) [HiddenDragon]
 *------------------------------------------*/
int charcommand_delitem(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct map_session_data *pl_sd;
	char item_name[100];
	int i, number = 0, item_id, item_position, count;
	struct item_data *item_data;
	char character[NAME_LENGTH];
	char output[200];

	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(item_name, '\0', sizeof(item_name));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || (
		sscanf(message, "\"%99[^\"]\" %d %23[^\n]", item_name, &number, character) < 3 &&
		sscanf(message, "%s %d %23[^\n]", item_name, &number, character) < 3
	) || number < 1) {
		clif_displaymessage(fd, "Please, enter an item name/id, a quantity and a player name (usage: #delitem <item_name_or_ID> <quantity> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	item_id = 0;
	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(atoi(item_name))) != NULL)
		item_id = item_data->nameid;
	
	if (item_id > 500) {
		item_position = pc_search_inventory(pl_sd, item_id);
		if (item_position >= 0) {
			count = 0;
			for(i = 0; i < number && item_position >= 0; i++) {

				//Logs (A)dmins items [Lupus]
				if(log_config.enable_logs&0x400)
					log_pick_pc(pl_sd, "A", pl_sd->status.inventory[item_position].nameid, -1, &pl_sd->status.inventory[item_position]);

				pc_delitem(pl_sd, item_position, 1, 0);
				count++;
				item_position = pc_search_inventory(pl_sd, item_id); // for next loop
			}
			sprintf(output, msg_txt(113), count); // %d item(s) removed by a GM.
			clif_displaymessage(pl_sd->fd, output);
			if (number == count)
				sprintf(output, msg_txt(114), count); // %d item(s) removed from the player.
			else
				sprintf(output, msg_txt(115), count, count, number); // %d item(s) removed. Player had only %d on %d items.
			clif_displaymessage(fd, output);
		} else {
			clif_displaymessage(fd, msg_txt(116)); // Character does not have the item.
			return -1;
		}
	}
	else
	{
		clif_displaymessage(fd, msg_txt(19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

//Added by Coltaro
//We're using this function here instead of using time_t so that it only counts player's jail time when he/she's online (and since the idea is to reduce the amount of minutes one by one in status_change_timer...).
//Well, using time_t could still work but for some reason that looks like more coding x_x
static void get_jail_time(int jailtime, int* year, int* month, int* day, int* hour, int* minute) {
	const int factor_year = 518400; //12*30*24*60 = 518400
	const int factor_month = 43200; //30*24*60 = 43200
	const int factor_day = 1440; //24*60 = 1440
	const int factor_hour = 60;

	*year = jailtime/factor_year;
	jailtime -= *year*factor_year;
	*month = jailtime/factor_month;
	jailtime -= *month*factor_month;
	*day = jailtime/factor_day;
	jailtime -= *day*factor_day;
	*hour = jailtime/factor_hour;
	jailtime -= *hour*factor_hour;
	*minute = jailtime;

	*year = *year > 0? *year : 0;
	*month = *month > 0? *month : 0;
	*day = *day > 0? *day : 0;
	*hour = *hour > 0? *hour : 0;
	*minute = *minute > 0? *minute : 0;
	return;
}

/*==========================================
 * Jail a player for a certain amount of time [Coltaro]
 *------------------------------------------*/
int charcommand_jailtime(const int fd, struct map_session_data* sd, const char* command, const char* message)
{  
	struct map_session_data* pl_sd;
	int year, month, day, hour, minute;
	char character[NAME_LENGTH];
	char output[200];
	memset(character, '\0', sizeof(character));

	nullpo_retr(-1, sd);

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #jailtime <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pl_sd->sc.data[SC_JAILED]) {
		clif_displaymessage(fd, "This player is not in jail.");
		return -1;
	}

	if (pl_sd->sc.data[SC_JAILED]->val1 == INT_MAX) {
		clif_displaymessage(fd, "This player has been jailed indefinitely.");
		return 0;
	}

	if (pl_sd->sc.data[SC_JAILED]->val1 <= 0) { // Was not jailed with @jailfor (maybe @jail? or warped there? or got recalled?)
		clif_displaymessage(fd, "This player has been jailed for an unknown amount of time.");
		return -1;
	}
	//Get remaining jail time
	get_jail_time(pl_sd->sc.data[SC_JAILED]->val1,&year,&month,&day,&hour,&minute);
	sprintf(output,msg_txt(402),"This player will remain",year,month,day,hour,minute); 
	clif_displaymessage(fd, output);

	return 0;
}

/*==========================================
 * Disguises a player [HiddenDragon]
 *------------------------------------------*/
int charcommand_disguise(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int mob_id;
	char mob_name[NAME_LENGTH];
	struct map_session_data* pl_sd;
	char character[NAME_LENGTH];
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(mob_name, '\0', sizeof(mob_name));

	if (!message || !*message || sscanf(message, "%s %23[^\n]", mob_name, character) < 2) {
		clif_displaymessage(fd, "Please, enter a Monster/NPC name/id and a player name (usage: #disguise <monster_name_or_monster_ID> <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if ((mob_id = atoi(mob_name)) > 0)
	{	//Acquired an ID
		if (!mobdb_checkid(mob_id) && !npcdb_checkid(mob_id))
			mob_id = 0; //Invalid id for either mobs or npcs.
	}	else	{ //Acquired a Name
		if ((mob_id = mobdb_searchname(mob_name)) == 0)
		{
			struct npc_data* nd = npc_name2id(mob_name);
			if (nd != NULL)
				mob_id = nd->class_;
		}
	}

	if (mob_id == 0)
	{
		clif_displaymessage(fd, msg_txt(123)); // Monster/NPC name/id hasn't been found.
		return -1;
	}

	if(pc_isriding(pl_sd))
	{
		//FIXME: wrong message [ultramage]
		//clif_displaymessage(fd, msg_txt(228)); 	// Character cannot wear disguise while riding a PecoPeco.
		return -1;
	}

	pc_disguise(pl_sd, mob_id);
	clif_displaymessage(pl_sd->fd, msg_txt(122));	// Disguise applied.
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, msg_txt(140)); // Character's disguise applied.
	
	
	return 0;
}

/*==========================================
 * Undisguises a player [HiddenDragon]
 *------------------------------------------*/
int charcommand_undisguise(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data* pl_sd;
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #undisguise <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (pl_sd->disguise)
	{
		pc_disguise(pl_sd, 0);
		clif_displaymessage(pl_sd->fd, msg_txt(124));	// Disguise removed from character
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(141));	// Undisguise applied
	}
	else {
		clif_displaymessage(fd, msg_txt(142)); // Character is not disguised.
		return -1;
	}

	return 0;
}

/*==========================================
 * Displays contents of target's cart [HiddenDragon]
 *------------------------------------------*/
int charcommand_cart_list(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char outputtmp[200];
	char output[200];
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	struct item_data *item_data, *item_temp;
	int i, j, count = 0, counter = 0, counter2;
	nullpo_retr(-1, sd);

	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	memset(outputtmp, '\0', sizeof(outputtmp));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #cartlist <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	for (i = 0; i < MAX_CART; i++) {
		if (pl_sd->status.cart[i].nameid > 0 && (item_data = itemdb_search(pl_sd->status.cart[i].nameid)) != NULL) {
			counter = counter + pl_sd->status.cart[i].amount;
			count++;
			if (count == 1) {
				sprintf(output, "------ Cart items list of '%s' ------", pl_sd->status.name);
				clif_displaymessage(fd, output);
			}
			if (pl_sd->status.cart[i].refine)
				sprintf(output, "%d %s %+d (%s %+d, id: %d)", pl_sd->status.cart[i].amount, item_data->name, pl_sd->status.cart[i].refine, item_data->jname, pl_sd->status.cart[i].refine, pl_sd->status.cart[i].nameid);
			else
				sprintf(output, "%d %s (%s, id: %d)", pl_sd->status.cart[i].amount, item_data->name, item_data->jname, pl_sd->status.cart[i].nameid);
			clif_displaymessage(fd, output);
			memset(output, '\0', sizeof(output));
			counter2 = 0;
			for (j = 0; j < item_data->slot; j++) {
				if (pl_sd->status.cart[i].card[j]) {
					if ( (item_temp = itemdb_search(pl_sd->status.cart[i].card[j])) != NULL) {
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
		clif_displaymessage(fd, "No item found in the cart of this player.");
	else {
		sprintf(output, "%d item(s) found in %d kind(s) of items.", counter, count);
		clif_displaymessage(fd, output);
	}

	return 0;
}

static int charcommand_stopattack(struct block_list *bl,va_list ap)
{
	struct unit_data *ud = unit_bl2ud(bl);
	int id = va_arg(ap, int);
	if (ud && ud->attacktimer != INVALID_TIMER && (!id || id == ud->target))
	{
		unit_stop_attack(bl);
		return 1;
	}
	return 0;
}

/*==========================================
 * Enable a player to kill players even when not in pvp [HiddenDragon]
 *------------------------------------------*/
int charcommand_killer(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #killer <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	pl_sd->state.killer = !pl_sd->state.killer;

	if(pl_sd->state.killer)
	{
		clif_displaymessage(pl_sd->fd, msg_txt(241));
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, "Target player can now kill anybody.");
	}
	else
	{
		clif_displaymessage(pl_sd->fd, msg_txt(287));
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(287));
		pc_stop_attack(pl_sd);
	}

	return 0;
}

/*==========================================
 * Enable other players to kill target even when not in pvp [HiddenDragon]
 *------------------------------------------*/
int charcommand_killable(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #killable <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	pl_sd->state.killable = !pl_sd->state.killable;

	if(pl_sd->state.killable)
	{
		clif_displaymessage(pl_sd->fd, msg_txt(242));
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(289));
	}
	else
	{
		clif_displaymessage(pl_sd->fd, msg_txt(288));
		if (pl_sd->fd != fd)
			clif_displaymessage(fd, msg_txt(290));
		map_foreachinrange(charcommand_stopattack,&pl_sd->bl, AREA_SIZE, BL_CHAR, pl_sd->bl.id);
	}

	return 0;
}

/*==========================================
 * Drop all of the target's possessions on the ground
 *------------------------------------------*/
int charcommand_dropall(const int fd, struct map_session_data* sd, const char* command, const char* message)
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

	clif_displaymessage(pl_sd->fd, "All items dropped");
	clif_displaymessage(fd, "It is done");

	return 0;
}

/*==========================================
 * Put all of the target's possessions into storage
 *------------------------------------------*/
int charcommand_storeall(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int i;
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1, sd);

	if (!message || !*message)
		return -1;
	if((pl_sd=map_nick2sd((char *) message)) == NULL)
		return -1;

	if (pl_sd->state.storage_flag != 1)
  	{	//Open storage.
		switch (storage_storageopen(pl_sd)) {
		case 2: //Try again
			clif_displaymessage(fd, "Had to open the characters storage window...");
			clif_displaymessage(fd, "run this command again..");
			return 0;
		case 1: //Failure
			clif_displaymessage(fd, "The character currently can't use the storage.");
			return 1;
		}
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
 * Refreshes target [HiddenDragon]
 *------------------------------------------*/
int charcommand_refresh(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #refresh <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	clif_refresh(pl_sd);
	return 0;
}


/*==========================================
 * View target's exp [HiddenDragon]
 *------------------------------------------*/
int charcommand_exp(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char output[200];
	double nextb, nextj;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	
	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));
	

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #exp <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	nextb = pc_nextbaseexp(pl_sd);
	if (nextb)
		nextb = pl_sd->status.base_exp*100.0/nextb;
	
	nextj = pc_nextjobexp(pl_sd);
	if (nextj)
		nextj = pl_sd->status.job_exp*100.0/nextj;
	
	sprintf(output, "Base Level: %d (%.3f%%) | Job Level: %d (%.3f%%)", pl_sd->status.base_level, nextb, pl_sd->status.job_level, nextj);
	clif_displaymessage(fd, output);
	return 0;
}

/*==========================================
 * Makes monsters ignore target [HiddenDragon]
 *------------------------------------------*/ 
int charcommand_monsterignore(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #monsterignore/#battleignore <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (!pl_sd->state.monster_ignore) {
		pl_sd->state.monster_ignore = 1;
		clif_displaymessage(pl_sd->fd, "You are now immune to attacks.");
		if (fd != pl_sd->fd)
			clif_displaymessage(pl_sd->fd, "Target player is now immune to attacks.");
	} else {
		sd->state.monster_ignore = 0;
		clif_displaymessage(sd->fd, "You are no longer immune to attacks.");
		if (fd != pl_sd->fd)
			clif_displaymessage(pl_sd->fd, "Target player is no longer immune to attacks.");

	}

	return 0;
}

/*==========================================
 * Change target's size [HiddenDragon]
 *------------------------------------------*/
int charcommand_size(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int size=0;
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &size, character) < 2) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #size <0-2> <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if(pl_sd->state.size) {
		pl_sd->state.size=0;
		pc_setpos(pl_sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, 3);
	}

	if(size==1) {
		pl_sd->state.size=1;
		clif_specialeffect(&pl_sd->bl,420,AREA);
		clif_displaymessage(fd, "Size changed."); 
		if (fd != pl_sd->fd)
			clif_displaymessage(pl_sd->fd, "Size changed."); 
	} else if(size==2) {
		pl_sd->state.size=2;
		clif_specialeffect(&pl_sd->bl,422,AREA);
		clif_displaymessage(fd, "Size changed."); 
		if (fd != pl_sd->fd)
			clif_displaymessage(pl_sd->fd, "Size changed."); 
	}
	else
	{
		clif_displaymessage(fd, "Size restored."); 
		if (fd != pl_sd->fd)
			clif_displaymessage(pl_sd->fd, "Size restored."); 
	}


	return 0;
}

/*==========================================
 * Level up target's homunculus [HiddenDragon]
 *------------------------------------------*/
int charcommand_homlevel(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	TBL_HOM * hd;
	int level = 0, i = 0;

	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &level, character) < 2) {
		clif_displaymessage(fd, "Please, enter a player name and level (usage: #homlevel <# of levels> <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
		
	if ( !merc_is_hom_active(pl_sd->hd) )
	{
		clif_displaymessage(fd, "Target player does not have a homunculus."); 
		return 1;
	}

	hd = pl_sd->hd;
	
	for (i = 1; i <= level && hd->exp_next; i++){
		hd->homunculus.exp += hd->exp_next;
		merc_hom_levelup(hd);
	}
	status_calc_homunculus(hd,0);
	status_percent_heal(&hd->bl, 100, 100);
	clif_misceffect2(&hd->bl,568);
	clif_displaymessage(pl_sd->fd, "Homunculus level changed."); 
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, "Player's homunculus level changed.");

	return 0;
}

/*==========================================
 * Evolve target's homunculus [HiddenDragon]
 *------------------------------------------*/
int charcommand_homevolution(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #homevolution <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if ( !merc_is_hom_active(pl_sd->hd) ) {
		clif_displaymessage(fd, "Target player does not have a homunculus."); 
		return -1;
	}

	if ( !merc_hom_evolution(pl_sd->hd) ) {
		clif_displaymessage(fd, "Target homunculus cannot evolve."); 
		return -1;
	}

	clif_displaymessage(pl_sd->fd, "Homunculus evolution initiated."); 
	if (pl_sd->fd != fd)
		clif_displaymessage(fd, "Homunculus evolution initiated."); 
	return 0;
}

/*==========================================
 * Create a homunculus for target [HiddenDragon]
 *------------------------------------------*/
int charcommand_makehomun(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int homunid;
	
	char character[NAME_LENGTH];
	char output[200];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &homunid, character) < 2) {
		sprintf(output, "Please, enter a player name (usage: #makehomun <homunculus id: %d-%d> <player>).", HM_CLASS_BASE, HM_CLASS_BASE + MAX_HOMUNCULUS_CLASS - 1);
		clif_displaymessage(fd, output);
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}
	
	if( homunid < HM_CLASS_BASE || homunid > HM_CLASS_BASE + MAX_HOMUNCULUS_CLASS - 1 )
	{
		clif_displaymessage(fd, "Invalid homunculus ID."); 
		return -1;
	}
	
	if(pl_sd->status.hom_id == 0)
	{
		merc_create_homunculus_request(pl_sd,homunid);
	}
	else
	{
		clif_displaymessage(fd, "Target player already has a homunculus.");
	}
	return 0;
}

/*==========================================
 * Change Target's homunculus' friendlyness [HiddenDragon]
 *------------------------------------------*/
int charcommand_homfriendly(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int friendly = 0;

	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &friendly, character) < 2) {
		clif_displaymessage(fd, "Please, enter a player name and friendly value (usage: #homfriendly <0-1000> <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (merc_is_hom_active(pl_sd->hd)) {
		if (friendly > 0 && friendly <= 1000) {
			pl_sd->hd->homunculus.intimacy = friendly * 100 ;
			clif_send_homdata(pl_sd,SP_INTIMATE,friendly);
			clif_displaymessage(pl_sd->fd, "Homunculus friendly value changed."); 
			if (fd != pl_sd->fd)
				clif_displaymessage(fd, "Homunculus friendly value changed."); 
		} else {
			clif_displaymessage(fd, "Invalid friendly value."); 
		}
	}
	else
	{
		clif_displaymessage(fd, "Target player's homunculus does not exist."); 
		return -1;
	}

	return 0;
}

/*==========================================
 * Modify target's homunculus hunger [HiddenDragon]
 *------------------------------------------*/
int charcommand_homhungry(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	int hungry = 0;

	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));

	if (!message || !*message || sscanf(message, "%d %23[^\n]", &hungry, character) < 2) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #homhungry <0-100> <player>).");
		return -1;
	}
	
	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if (pl_sd->status.hom_id > 0 && pl_sd->hd) {
		struct homun_data *hd = pl_sd->hd;
		if (hungry >= 0 && hungry <= 100) {
			hd->homunculus.hunger = hungry;
			clif_send_homdata(pl_sd,SP_HUNGRY,hd->homunculus.hunger);
			clif_displaymessage(pl_sd->fd, "Homunculus hungry value changed."); 
			if (fd != pl_sd->fd)
				clif_displaymessage(fd, "Homunculus hungry value changed."); 
		} else {
			clif_displaymessage(fd, "Invalid hungry value."); 
			return -1;
		}
	}
	else
	{
		clif_displaymessage(fd, "Target player does not have a homunculus."); 
		return -1;
	}
	

	return 0;
}

/*==========================================
 * Show target's homunculus stats [HiddenDragon]
 *------------------------------------------*/
int charcommand_hominfo(const int fd, struct map_session_data* sd, const char* command, const char* message)
{
	struct homun_data *hd;
	struct status_data *status;
	char output[200];
	char character[NAME_LENGTH];
	struct map_session_data *pl_sd;
	memset(character, '\0', sizeof(character));
	memset(output, '\0', sizeof(output));

	if (!message || !*message || sscanf(message, "%23[^\n]", character) < 1) {
		clif_displaymessage(fd, "Please, enter a player name (usage: #hominfo <player>).");
		return -1;
	}

	if ( (pl_sd = map_nick2sd(character)) == NULL )
	{
		clif_displaymessage(fd, msg_txt(3)); // Character not found.
		return -1;
	}

	if ( pc_isGM(sd) < pc_isGM(pl_sd) )
	{
		clif_displaymessage(fd, msg_txt(81)); // Your GM level don't authorise you to do this action on this player.
		return -1;
	}

	if(!merc_is_hom_active(pl_sd->hd))
		return -1;

	hd = pl_sd->hd;
	status = status_get_status_data(&hd->bl);
	clif_displaymessage(fd, "Homunculus stats :");

	snprintf(output, sizeof(output), "HP : %d/%d - SP : %d/%d", status->hp, status->max_hp, status->sp, status->max_sp);
	clif_displaymessage(fd, output);

	snprintf(output, sizeof(output), "ATK : %d - MATK : %d~%d", status->rhw.atk2 +status->batk, status->matk_min, status->matk_max);
	clif_displaymessage(fd, output);

	snprintf(output, sizeof(output), "Hungry : %d - Intimacy : %u", hd->homunculus.hunger, hd->homunculus.intimacy/100);
	clif_displaymessage(fd, output);

	snprintf(output, sizeof(output), "Stats: Str %d / Agi %d / Vit %d / Int %d / Dex %d / Luk %d", status->str, status->agi, status->vit, status->int_, status->dex, status->luk);
	clif_displaymessage(fd, output);

	return 0;
}


/*==========================================
 * charcommand_info[] structure definition
 *------------------------------------------*/

CharCommandInfo charcommand_info[] = {
	{ "job",               60,     charcommand_jobchange },
	{ "jobchange",         60,     charcommand_jobchange },
	{ "petrename",         50,     charcommand_petrename },
	{ "petfriendly",       50,     charcommand_petfriendly },
	{ "stats",             40,     charcommand_stats },
	{ "option",            60,     charcommand_option },
	{ "reset",             60,     charcommand_reset },
	{ "save",              60,     charcommand_save },
	{ "spiritball",        40,     charcommand_spiritball },
	{ "itemlist",          40,     charcommand_itemlist },
	{ "effect",            40,     charcommand_effect },
	{ "storagelist",       40,     charcommand_storagelist },
	{ "item",              60,     charcommand_item },
	{ "warp",              60,     charcommand_warp },
	{ "rura",              60,     charcommand_warp },
	{ "rura+",             60,     charcommand_warp },
	{ "zeny",              60,     charcommand_zeny },
	{ "fakename",          50,     charcommand_fakename },
	{ "baselvl",           60,     charcommand_baselevel },
	{ "baselevel",         60,     charcommand_baselevel },
	{ "blvl",              60,     charcommand_baselevel },
	{ "blevel",            60,     charcommand_baselevel },
	{ "joblvl",            60,     charcommand_joblevel },
	{ "joblevel",          60,     charcommand_joblevel },
	{ "jlvl",              60,     charcommand_joblevel },
	{ "jlevel",            60,     charcommand_joblevel },
	{ "questskill",        60,     charcommand_questskill },
	{ "lostskill",         60,     charcommand_lostskill },
	{ "skreset",           60,     charcommand_skreset },
	{ "streset",           60,     charcommand_streset },
	{ "model",             50,     charcommand_model },
	{ "skpoint",           60,     charcommand_skpoint },
	{ "stpoint",           60,     charcommand_stpoint },
	{ "changesex",         60,     charcommand_changesex },
	{ "feelreset",         60,     charcommand_feelreset },
	{ "help",              20,     charcommand_help },
	{ "load",              60,     charcommand_load },
	{ "speed",             60,     charcommand_speed },
	{ "storage",           60,     charcommand_storage },
	{ "gstorage",          60,     charcommand_guildstorage },
	{ "hide",              60,     charcommand_hide },
	{ "alive",             60,     charcommand_alive },
	{ "revive",            60,     charcommand_alive },
	{ "heal",              60,     charcommand_heal },
	{ "item2",             60,     charcommand_item2 },
	{ "itemreset",         60,     charcommand_itemreset },
	{ "refine",            60,     charcommand_refine },
	{ "produce",           60,     charcommand_produce },
	{ "str",               60,     charcommand_param },
	{ "agi",               60,     charcommand_param },
	{ "vit",               60,     charcommand_param },
	{ "int",               60,     charcommand_param },
	{ "dex",               60,     charcommand_param },
	{ "luk",               60,     charcommand_param },
	{ "glvl",              60,     charcommand_guildlevelup },
	{ "glevel",            60,     charcommand_guildlevelup },
	{ "guildlvl",          60,     charcommand_guildlevelup },
	{ "guildlevel",        60,     charcommand_guildlevelup },
	{ "hatch",             50,     charcommand_hatch },
	{ "pethungry",         60,     charcommand_pethungry },
	{ "allskill",          60,     charcommand_allskill },
	{ "allskills",         60,     charcommand_allskill },
	{ "skillall",          60,     charcommand_allskill },
	{ "skillsall",         60,     charcommand_allskill },
	{ "dye",               50,     charcommand_dye },
	{ "hairstyle",         50,     charcommand_hair_style },
	{ "hstyle",            50,     charcommand_hair_style },
	{ "haircolor",         50,     charcommand_hair_color },
	{ "hcolor",            50,     charcommand_hair_color },
	{ "allstat",           60,     charcommand_allstats },
	{ "allstats",          60,     charcommand_allstats },
	{ "statall",           60,     charcommand_allstats },
	{ "statsall",          60,     charcommand_allstats },
	{ "mount",             50,     charcommand_mount_peco },
	{ "mountpeco",         50,     charcommand_mount_peco },
	{ "delitem",           60,     charcommand_delitem },
	{ "jailtime",          40,     charcommand_jailtime },
	{ "disguise",          60,     charcommand_disguise },
	{ "undisguise",        60,     charcommand_undisguise },
	{ "cartlist",          40,     charcommand_cart_list },
	{ "killer",            60,     charcommand_killer },
	{ "killable",          60,     charcommand_killable },
	{ "dropall",           60,     charcommand_dropall },
	{ "storeall",          60,     charcommand_storeall },
	{ "refresh",           40,     charcommand_refresh },
	{ "exp",                1,     charcommand_exp },
	{ "monsterignore",     60,     charcommand_monsterignore },
	{ "size",              50,     charcommand_size },
	{ "hlvl",              60,     charcommand_homlevel },
	{ "hlevel",            60,     charcommand_homlevel },
	{ "homlvl",            60,     charcommand_homlevel },
	{ "homlevel",          60,     charcommand_homlevel },
	{ "homevolve",         60,     charcommand_homevolution },
	{ "homevolution",      60,     charcommand_homevolution },
	{ "homfriendly",       60,     charcommand_homfriendly },
	{ "homhungry",         60,     charcommand_homhungry },
	{ "hominfo",           40,     charcommand_hominfo },
};


/*==========================================
 * Command lookup functions
 *------------------------------------------*/
static CharCommandInfo* get_charcommandinfo_byname(const char* name)
{
	int i;
	if( *name == charcommand_symbol ) name++; // for backwards compatibility
	ARR_FIND( 0, ARRAYLENGTH(charcommand_info), i, strcmpi(charcommand_info[i].command, name) == 0 );
	return ( i != ARRAYLENGTH(charcommand_info) ) ? &charcommand_info[i] : NULL;
}

static CharCommandInfo* get_charcommandinfo_byfunc(const CharCommandFunc func)
{
	int i;
	ARR_FIND( 0, ARRAYLENGTH(charcommand_info), i, charcommand_info[i].func == func );
	return ( i != ARRAYLENGTH(charcommand_info) ) ? &charcommand_info[i] : NULL;
}


/*==========================================
 * Retrieve the command's required gm level
 *------------------------------------------*/
int get_charcommand_level(const CharCommandFunc func)
{
	CharCommandInfo* info = get_charcommandinfo_byfunc(func);
	return ( info != NULL ) ? info->level : 100; // 100: command can not be used
}


/// Executes a char-command.
/// To be called by internal server code (bypasses various restrictions).
bool is_charcommand_sub(const int fd, struct map_session_data* sd, const char* str, int gmlvl)
{
	CharCommandInfo* info;
	char command[100];
	char args[100];
	char output[200];
	
	if( !str || !*str )
		return false;

	if( *str != charcommand_symbol ) // check first char
		return false;

	if( sscanf(str, "%99s %99[^\n]", command, args) < 2 )
		args[0] = '\0';

	info = get_charcommandinfo_byname(command);
	if( info == NULL || info->func == NULL || gmlvl < info->level )
	{
		if( gmlvl == 0 )
			return false; // will just display as normal text
		else
		{
			sprintf(output, msg_txt(153), command); // "%s is Unknown Command."
			clif_displaymessage(fd, output);
			return true;
		}
	}

	if( log_config.gm && info->level >= log_config.gm )
		log_atcommand(sd, str);

	if( info->func(fd, sd, command, args) != 0 )
	{
		sprintf(output, msg_txt(154), command); // "%s failed."
		clif_displaymessage(fd, output);
	}
	
	return true;
}

/// Executes a char-command.
/// To be used by player-invoked code (restrictions will be applied)
bool is_charcommand(const int fd, struct map_session_data* sd, const char* message)
{
	int gmlvl = pc_isGM(sd);

	nullpo_retr(false, sd);

	if( !message || !*message )
		return false;

	if( sd->sc.data[SC_NOCHAT] && sd->sc.data[SC_NOCHAT]->val1&MANNER_NOCOMMAND )
		return true; // so that it won't display as normal message

	if( battle_config.atc_gmonly != 0 && gmlvl == 0 )
		return false;

	if( map[sd->bl.m].nocommand && gmlvl < map[sd->bl.m].nocommand )
	{
		clif_displaymessage(fd, msg_txt(143)); // "Commands are disabled on this map."
		return false;
	}
	
	// skip 10/11-langtype's codepage indicator, if detected
	if( message[0] == '|' && strlen(message) >= 4 && message[3] == charcommand_symbol )
		message += 3;

	return is_charcommand_sub(fd,sd,message,gmlvl);
}


/*==========================================
 *
 *------------------------------------------*/
int charcommand_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	CharCommandInfo* p;
	FILE* fp;

	if( (fp = fopen(cfgName, "r")) == NULL )
	{
		ShowError("CharCommand configuration file not found: %s\n", cfgName);
		return 1;
	}

	while( fgets(line, sizeof(line), fp) )
	{
		if( line[0] == '/' && line[1] == '/' )
			continue;
		
		if( sscanf(line, "%1023[^:]:%1023s", w1, w2) != 2 )
			continue;
		
		p = get_charcommandinfo_byname(w1);
		if( p != NULL )
		{
			p->level = atoi(w2);
			p->level = cap_value(p->level, 0, 100);
		}
		else
		if( strcmpi(w1, "import") == 0 )
			charcommand_config_read(w2);
		else
		if( strcmpi(w1, "command_symbol") == 0 &&
			w2[0] > 31   && // control characters
			w2[0] != '/' && // symbol of standard ragnarok GM commands
			w2[0] != '%' && // symbol of party chat speaking
			w2[0] != '$' && // symbol of guild chat speaking
			w2[0] != '@' )  // symbol of atcommand
			charcommand_symbol = w2[0];
		else
			ShowWarning("Unknown setting '%s' in file %s\n", w1, cfgName);
	}
	fclose(fp);

	return 0;
}
