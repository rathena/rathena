// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/cbasetypes.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/malloc.h"
#include "../common/version.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"
#include "../common/strlib.h"
#include "../common/utils.h"

#include "map.h"
#include "chrif.h"
#include "pc.h"
#include "npc.h"
#include "itemdb.h"
#include "script.h"
#include "intif.h"
#include "battle.h"
#include "mob.h"
#include "party.h"
#include "unit.h"
#include "log.h"
#include "clif.h"
#include "quest.h"
#include "intif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

//Send quest info on login
int quest_pc_login(TBL_PC * sd)
{

	if(sd->num_quests == 0)
		return 1;

	clif_send_questlog(sd);
	clif_send_questlog_info(sd);
	return 0;
}

int quest_add(TBL_PC * sd, struct quest * qd)
{

	int i;

	//Search to see if this quest exists
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == qd->quest_id);

	//Already have this quest
	if(i!=MAX_QUEST)
		return 1;

	//Find empty quest log spot
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == 0);

	//Quest log is full
	if(i == MAX_QUEST)
		return -1;

	//Copy over quest data
	memcpy(&sd->quest_log[i], qd, sizeof(struct quest));
	sd->num_quests++;

	//Notify inter server
	intif_quest_add(sd->status.char_id, qd);

	return 0;

}

int quest_add_ack(int char_id, int quest_id, int success)
{
	int i;
	TBL_PC * sd = map_charid2sd(char_id);

	///Player no longer on map
	if(!sd)
		return -1;

	//Search for quest
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == quest_id);

	//Quest not found, shouldn't happen?
	if(i == MAX_QUEST)
		return -1;

	if(success)
	{
		//Notify client
		clif_send_quest_info(sd, &sd->quest_log[i]);
	}
	else
	{
		ShowError("Quest %d for character %d could not be added!\n", quest_id, char_id);

		//Zero quest
		memset(&sd->quest_log[i], 0, sizeof(struct quest));
		sd->num_quests--;
	}

	return 0;
}

int quest_delete(TBL_PC * sd, int quest_id)
{

	int i;

	//Search for quest
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == quest_id);

	//Quest not found
	if(i == MAX_QUEST)
		return -1;

	intif_quest_delete(sd->status.char_id, quest_id);

	return 0;

}

int quest_delete_ack(int char_id, int quest_id, int success)
{

	int i;
	TBL_PC * sd = map_charid2sd(char_id);

	///Player no longer on map
	if(!sd)
		return -1;

	//Search for quest
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == quest_id);

	//Quest not found
	if(i == MAX_QUEST)
		return -1;

	if(success)
	{

		//Zero quest
		memset(&sd->quest_log[i], 0, sizeof(struct quest));
		sd->num_quests--;

		//Notify client
		clif_send_quest_delete(sd, quest_id);

		return 1;

	}
	else
		ShowError("Quest %d for character %d could not be deleted!\n", quest_id, char_id);

	return 0;

}

int quest_update_objective(TBL_PC * sd, int quest_id, int objective_num, const char * name, int count)
{

	int i;

	//Search for quest
	ARR_FIND(0, MAX_QUEST, i, sd->quest_log[i].quest_id == quest_id);

	//Quest not found
	if(i == MAX_QUEST)
		return -1;

	memcpy(&sd->quest_log[i].objectives[objective_num].name, name, NAME_LENGTH);
	sd->quest_log[i].objectives[objective_num].count = count;

	//Notify client
	clif_send_quest_info(sd, &sd->quest_log[i]);

	return 0;

}

int quest_update_status(TBL_PC * sd, int quest_id, bool status)
{

	return 0;
}
