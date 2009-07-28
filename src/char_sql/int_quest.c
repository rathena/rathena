// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "../common/timer.h"

#include "char.h"
#include "inter.h"
#include "int_quest.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Load entire questlog for a character
int mapif_quests_fromsql(int char_id, struct quest questlog[])
{
	int i;
	struct quest tmp_quest;
	SqlStmt * stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if( stmt == NULL )
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	memset(&tmp_quest, 0, sizeof(struct quest));

	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `quest_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3` FROM `%s` WHERE `char_id`=? LIMIT %d", quest_db, MAX_QUEST_DB)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_quest.quest_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT,    &tmp_quest.state, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_UINT,   &tmp_quest.time, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 3, SQLDT_INT,    &tmp_quest.mob[0], 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 4, SQLDT_INT,    &tmp_quest.count[0], 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 5, SQLDT_INT,    &tmp_quest.mob[1], 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 6, SQLDT_INT,    &tmp_quest.count[1], 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 7, SQLDT_INT,    &tmp_quest.mob[2], 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 8, SQLDT_INT,    &tmp_quest.count[2], 0, NULL, NULL) )
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_QUEST_DB && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		memcpy(&questlog[i], &tmp_quest, sizeof(tmp_quest));
		questlog[i].num_objectives = (!questlog[i].mob[0] ? 0 : !questlog[i].mob[1] ? 1 : !questlog[i].mob[2] ? 2 : 3);
	}

	SqlStmt_Free(stmt);
	return i;
}

//Save quests
int mapif_parse_quest_save(int fd)
{
	int i, j, num2, num1 = (RFIFOW(fd,2)-8)/sizeof(struct quest);
	int char_id = RFIFOL(fd,4);
	struct quest qd1[MAX_QUEST_DB],qd2[MAX_QUEST_DB];
	uint8 buf[MAX_QUEST_DB];
	int count = 0;

	memset(qd1, 0, sizeof(qd1));
	memset(qd2, 0, sizeof(qd2));
	memcpy(&qd1, RFIFOP(fd,8), RFIFOW(fd,2)-8);
	num2 = mapif_quests_fromsql(char_id, qd2);

	for( i = 0; i < num1; i++ )
	{
		ARR_FIND( 0, num2, j, qd1[i].quest_id == qd2[j].quest_id );
		if( j < num2 ) // Update existed quests
		{	// Only states and counts are changable.
			if( qd1[i].state != qd2[j].state || qd1[i].count[0] != qd2[j].count[0] || qd1[i].count[1] != qd2[j].count[1] || qd1[i].count[2] != qd2[j].count[2] )
				mapif_quest_update(char_id, qd1[i]);

			if( j < (--num2) )
			{
				memmove(&qd2[j],&qd2[j+1],sizeof(struct quest)*(num2-j));
				memset(&qd2[num2], 0, sizeof(struct quest));
			}

		}
		else // Add new quests
		{
			mapif_quest_add(char_id, qd1[i]);

			WBUFL(buf,count*4) = qd1[i].quest_id;
			count++;
		}
	}

	for( i = 0; i < num2; i++ ) // Quests not in qd1 but in qd2 are to be erased.
		mapif_quest_delete(char_id, qd2[i].quest_id);

	WFIFOHEAD(fd,8+4*count);
	WFIFOW(fd,0) = 0x3861;
	WFIFOW(fd,2) = 8+4*count;
	WFIFOL(fd,4) = char_id;
	memcpy(WFIFOP(fd,8), buf, count*4);
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

//Delete a quest
int mapif_quest_delete(int char_id, int quest_id)
{
	if ( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return -1;
	}

	return 1;
}

//Add a quest to a questlog
int mapif_quest_add(int char_id, struct quest qd)
{
	StringBuf buf;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3`) VALUES ('%d', '%d', '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d')", quest_db, qd.quest_id, char_id, qd.state, qd.time, qd.mob[0], qd.count[0], qd.mob[1], qd.count[1], qd.mob[2], qd.count[2]);

	if ( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ) 
	{
		Sql_ShowDebug(sql_handle);
		return -1;
	}

	StringBuf_Destroy(&buf);

	return 1;
}

//Update a questlog
int mapif_quest_update(int char_id, struct quest qd)
{
	StringBuf buf;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, qd.state, qd.count[0], qd.count[1], qd.count[2], qd.quest_id, char_id);

	if ( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ) 
	{
		Sql_ShowDebug(sql_handle);
		return -1;
	}

	StringBuf_Destroy(&buf);

	return 1;
}

//Send questlog to map server
int mapif_parse_quest_load(int fd)
{
	int char_id = RFIFOL(fd,2);
	struct quest tmp_questlog[MAX_QUEST_DB];
	int num_quests, i, num_complete = 0;
	int complete[MAX_QUEST_DB];

	memset(tmp_questlog, 0, sizeof(tmp_questlog));
	memset(complete, 0, sizeof(complete));

	num_quests = mapif_quests_fromsql(char_id, tmp_questlog);

	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	//Active and inactive quests
	for( i = 0; i < num_quests; i++ )
	{
		if( tmp_questlog[i].state == Q_COMPLETE )
		{
			complete[num_complete++] = i;
			continue;
		}
		memcpy(WFIFOP(fd,(i-num_complete)*sizeof(struct quest)+8), &tmp_questlog[i], sizeof(struct quest));
	}

	// Completed quests
	for( i = num_quests - num_complete; i < num_quests; i++ )
		memcpy(WFIFOP(fd,i*sizeof(struct quest)+8), &tmp_questlog[complete[i-num_quests+num_complete]], sizeof(struct quest));

	WFIFOSET(fd,num_quests*sizeof(struct quest)+8);

	return 0;
}

int inter_quest_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3060: mapif_parse_quest_load(fd); break;
		case 0x3061: mapif_parse_quest_save(fd); break;
		default:
			return 0;
	}
	return 1;
}
