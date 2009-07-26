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

struct s_quest_db {
	int id;
	unsigned int time;
	int mob[MAX_QUEST_OBJECTIVES];
	int count[MAX_QUEST_OBJECTIVES];
	//char name[NAME_LENGTH];
};
struct s_quest_db quest_db[MAX_QUEST_DB];

//Send quest info on login
int quest_pc_login(TBL_PC * sd)
{
	if(sd->num_quests == 0)
		return 1;

	clif_send_questlog(sd);
	clif_send_questlog_info(sd);

	return 0;
}

int quest_add(TBL_PC * sd, int quest_id)
{

	int i, j, count;

	if( quest_check_quest(sd, quest_id, HAVEQUEST) >= 0 )
	{
		ShowError("quest_add: you already have quest %d.\n",quest_id);
		return -1;
	}

	if( (j = quest_search_db(quest_id)) < 0 )
	{
		ShowError("quest_add: quest %d not found in DB.\n",quest_id);
		return -1;
	}

	if( sd->num_quests >= MAX_QUEST_DB )
	{
		ShowError("quest_add: your quest log is full.(max quests: %d)\n", MAX_QUEST_DB);
		return 1;
	}

	i = sd->avail_quests;
	memmove(&sd->quest_log[i+1], &sd->quest_log[i], sizeof(struct quest)*(sd->num_quests-sd->avail_quests));
	memset(&sd->quest_log[i], 0, sizeof(struct quest));
	sd->quest_log[i].quest_id = quest_db[j].id;
	if( quest_db[j].time )
		sd->quest_log[i].time = (unsigned int)(time(NULL) + quest_db[j].time);
	sd->quest_log[i].state = Q_ACTIVE;
	for( count = 0; count < MAX_QUEST_OBJECTIVES && quest_db[j].mob[count]; count++ )
		sd->quest_log[i].mob[count] = quest_db[j].mob[count];
	sd->quest_log[i].num_objectives = count;

	sd->num_quests++;
	sd->avail_quests++;

	//Notify inter server
	intif_quest_add(sd->status.char_id, &sd->quest_log[i]);

	return 0;
}

int quest_add_ack(int char_id, int quest_id, int success)
{
	int i;
	TBL_PC * sd = map_charid2sd(char_id);

	///Player no longer on map
	if(!sd)
		return -1;
	
	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id);
	if( i == sd->avail_quests )
		return -1; //Shouldn't happen!

	if( success )
		clif_send_quest_info(sd, &sd->quest_log[i]); //Notify client
	else
	{
		sd->avail_quests--;
		if( sd->num_quests < MAX_QUEST_DB && sd->quest_log[i+1].quest_id )
			memmove(&sd->quest_log[i], &sd->quest_log[i+1], sizeof(struct quest)*(sd->num_quests-i));
		memset(&sd->quest_log[--sd->num_quests], 0, sizeof(struct quest));
		ShowError("Quest %d for character %d could not be added!\n", quest_id, char_id);
		return 1;
	}

	return 0;
}

int quest_delete(TBL_PC * sd, int quest_id)
{
	//Search for quest
	if( quest_check_quest(sd, quest_id, HAVEQUEST) < 0 ) //Quest not found
	{
		ShowError("quest_delete: quest %d not found in your quest log.\n",quest_id);
		return -1;
	}

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
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if(i == sd->num_quests) //Shouldn't happen!
		return -1;

	if(success)
	{
		if( sd->quest_log[i].state != Q_COMPLETE )
			sd->avail_quests--;
		if( sd->num_quests < MAX_QUEST_DB && sd->quest_log[i+1].quest_id )
			memmove(&sd->quest_log[i], &sd->quest_log[i+1], sizeof(struct quest)*(sd->num_quests-i));
		memset(&sd->quest_log[--sd->num_quests], 0, sizeof(struct quest));
		clif_send_quest_delete(sd, quest_id);
	}
	else
	{
		ShowError("Quest %d for character %d could not be deleted!\n", quest_id, char_id);
		return 1;
	}

	return 0;
}

void quest_update_objective(TBL_PC * sd, int mob)
{
	int i,j;

	for( i = 0; i < sd->avail_quests; i++ )
	{
		if( sd->quest_log[i].state != Q_ACTIVE )
			continue;

		for( j = 0; j < MAX_QUEST_OBJECTIVES; j++ )
			if( sd->quest_log[i].mob[j] == mob )
			{
				sd->quest_log[i].count[j]++;
				sd->quest_log[i].save_quest = true;
				//clif_send_quest_info(sd, &sd->quest_log[i]); //TODO: Figure out the real packet [Inkfish]
				//break;
			}
	}
}

int quest_update_status(TBL_PC * sd, int quest_id, int status)
{
	int i;

	//Search for quest
	ARR_FIND(0, sd->avail_quests, i, sd->quest_log[i].quest_id == quest_id); // Only status of active and inactive quests can't be updated and completed quests can't [Inkfish]
	if(i == sd->avail_quests) //Quest not found
	{
		ShowError("Quest %d not found in your quest log!\n", quest_id);
		return -1;
	}

	sd->quest_log[i].state = (quest_state)status;
	if( status != Q_COMPLETE )
	{
		clif_send_quest_status(sd, quest_id, (bool)status);
		sd->quest_log[i].save_quest = true;
	}
	else
	{
		struct quest tmp_quest;

		clif_send_quest_delete(sd, quest_id);
		sd->avail_quests--;

		memcpy(&tmp_quest, &sd->quest_log[i],sizeof(struct quest));
		memcpy(&sd->quest_log[i], &sd->quest_log[sd->avail_quests],sizeof(struct quest));
		memcpy(&sd->quest_log[sd->avail_quests], &tmp_quest,sizeof(struct quest));

		sd->quest_log[sd->avail_quests].save_quest = true;
	}

	return 0;
}

int quest_save(TBL_PC * sd)
{
	int i;

	for( i = 0; i < sd->num_quests; i++ )
		if( sd->quest_log[i].save_quest )
			intif_quest_save(sd->status.char_id, &sd->quest_log[i]);

	return 0;
}

int quest_save_ack(int char_id, int quest_id, int success)
{

	int i;
	TBL_PC * sd = map_charid2sd(char_id);

	///Player no longer on map
	if(!sd)
		return -1;

	//Search for quest
	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if(i == sd->num_quests) //Quest not found
		return -1;

	if(success)
		sd->quest_log[i].save_quest = false;
	else
	{
		ShowError("Quest %d for character %d could not be saved!\n", quest_id, char_id);
		return 1;
	}

	return 0;

}

int quest_check_quest(TBL_PC * sd, int quest_id, quest_check_type type)
{
	int i;

	ARR_FIND(0, sd->num_quests, i, sd->quest_log[i].quest_id == quest_id);
	if(i == sd->num_quests)
		return -1;

	switch( type )
	{
	case HAVEQUEST:		
		return sd->quest_log[i].state;
	case PLAYTIME:
		return (sd->quest_log[i].time < (unsigned int)time(NULL) ? 2 : sd->quest_log[i].state == Q_COMPLETE ? 1 : 0);
	case HUNTING:
		{
			int j = quest_search_db(quest_id);

			if( j < 0 )
			{
				ShowError("quest_check_quest: quest not found in DB\n");
				return -1;
			}

			if( sd->quest_log[i].count[0] < quest_db[j].count[0] || sd->quest_log[i].count[1] < quest_db[j].count[1] || sd->quest_log[i].count[2] < quest_db[j].count[2] )
			{
				if( sd->quest_log[i].time < (unsigned int)time(NULL) )
					return 1;

				return 0;
			}
			
			return 2;
		}
	default:
		ShowError("quest_check_quest: Unknown parameter %d",type);
		break;
	}

	return -1;
}

int quest_search_db(int quest_id)
{
	int i;

	ARR_FIND(0, MAX_QUEST_DB,i,quest_id == quest_db[i].id);
	if( i == MAX_QUEST_DB )
		return -1;

	return i;
}

int quest_read_db(void)
{
	FILE *fp;
	char line[1024];
	int j,k = 0;
	char *str[20],*p,*np;

	sprintf(line, "%s/quest_db.txt", db_path);
	if( (fp=fopen(line,"r"))==NULL ){
		ShowError("can't read %s\n", line);
		return -1;
	}
	
	while(fgets(line, sizeof(line), fp))
	{
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));

		for( j = 0, p = line; j < 8;j++ )
		{
			if((np=strchr(p,','))!=NULL)
			{
				str[j] = p;
				*np = 0;
				p = np + 1;
			}
			else
			{
				ShowError("quest_read_db: insufficient columes in line %s\n", line);
				continue;
			}
		}
		if(str[0]==NULL)
			continue;

		memset(&quest_db[k], 0, sizeof(quest_db[0]));

		quest_db[k].id = atoi(str[0]);
		quest_db[k].time = atoi(str[1]);
		quest_db[k].mob[0] = atoi(str[2]);
		quest_db[k].count[0] = atoi(str[3]);
		quest_db[k].mob[1] = atoi(str[4]);
		quest_db[k].count[1] = atoi(str[5]);
		quest_db[k].mob[2] = atoi(str[6]);
		quest_db[k].count[2] = atoi(str[7]);
		//memcpy(quest_db[k].name, str[8], sizeof(str[8]));
		k++;
	}
	fclose(fp);
	ShowStatus("Done reading '"CL_WHITE"%s"CL_RESET"'.\n","quest_db.txt");
	return 0;
}

void do_init_quest(void)
{
	quest_read_db();
}
