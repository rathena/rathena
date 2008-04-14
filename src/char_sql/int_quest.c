// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
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

	int count, i, j, num;
	struct quest tmp_quest;
	struct quest_objective tmp_quest_objective;
	SqlStmt * stmt;

	stmt = SqlStmt_Malloc(sql_handle);
	if( stmt == NULL )
	{
		SqlStmt_ShowDebug(stmt);
		return 0;
	}

	memset(&tmp_quest, 0, sizeof(struct quest));
	memset(&tmp_quest_objective, 0, sizeof(struct quest_objective));

	if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `quest_id`, `state` FROM `%s` WHERE `char_id`=? LIMIT %d", quest_db, MAX_QUEST)
	||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
	||	SQL_ERROR == SqlStmt_Execute(stmt)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &tmp_quest.quest_id, 0, NULL, NULL)
	||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_INT,    &tmp_quest.state, 0, NULL, NULL) )
	//||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_INT,    &tmp_quest.time, 0, NULL, NULL)
		SqlStmt_ShowDebug(stmt);

	for( i = 0; i < MAX_QUEST && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++i )
	{
		memcpy(&questlog[i], &tmp_quest, sizeof(tmp_quest));
	}
	count = i;

	for( i = 0; i < count; ++i )
	{

		if( SQL_ERROR == SqlStmt_Prepare(stmt, "SELECT `num`, `name`, `count` FROM `%s` WHERE `char_id`=? AND `quest_id`=? LIMIT %d", quest_obj_db, MAX_QUEST_OBJECTIVES)
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 0, SQLDT_INT, &char_id, 0)
		||	SQL_ERROR == SqlStmt_BindParam(stmt, 1, SQLDT_INT, &questlog[i].quest_id, 0)
		||	SQL_ERROR == SqlStmt_Execute(stmt)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 0, SQLDT_INT,    &num, 0, NULL, NULL)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 1, SQLDT_STRING, &tmp_quest_objective.name, NAME_LENGTH, NULL, NULL)
		||	SQL_ERROR == SqlStmt_BindColumn(stmt, 2, SQLDT_INT,    &tmp_quest_objective.count, 0, NULL, NULL) )
			SqlStmt_ShowDebug(stmt);

		for( j = 0; j < MAX_QUEST && SQL_SUCCESS == SqlStmt_NextRow(stmt); ++j )
		{
			memcpy(&questlog[i].objectives[num], &tmp_quest_objective, sizeof(struct quest_objective));
		}
		questlog[i].num_objectives = j;

	}
		

	return count;
}

//Delete a quest
int mapif_parse_quest_delete(int fd)
{

	bool success = true;
	int char_id = RFIFOL(fd,2);
	int quest_id = RFIFOL(fd,6);

	if ( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		success = false;
	}

	if ( success && SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_obj_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		success = false;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3862;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = quest_id;
	WFIFOB(fd,10) = success?1:0;
	WFIFOSET(fd,11);

	return 0;

}

//Add a quest to a questlog
int mapif_parse_quest_add(int fd)
{

	StringBuf buf;
	bool success = true;
	int char_id = RFIFOL(fd,4);
	struct quest qd;
	int i;

	memcpy(&qd, RFIFOP(fd,8), RFIFOW(fd,2)-8);

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`) VALUES ('%d', '%d', '%d')", quest_db, qd.quest_id, char_id, qd.state);


	if ( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ) 
	{
		Sql_ShowDebug(sql_handle);
		success = false;
	}

	for(i=0; i<qd.num_objectives && success; i++)
	{

		StringBuf_Clear(&buf);
		StringBuf_Printf(&buf, "INSERT INTO `%s`(`quest_id`, `char_id`, `num`, `name`, `count`) VALUES ('%d', '%d', '%d', '%s', '%d')",
					quest_obj_db, qd.quest_id, char_id, i, qd.objectives[i].name, qd.objectives[i].count);

		if ( success && SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) )
		{
			Sql_ShowDebug(sql_handle);
			success = false;
		}
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = qd.quest_id;
	WFIFOB(fd,10) = success?1:0;
	WFIFOSET(fd,11);

	StringBuf_Destroy(&buf);

	return 0;

}

//Send questlog to map server
int mapif_send_quests(int fd, int char_id)
{

	struct quest tmp_questlog[MAX_QUEST];
	int num_quests, i;

	for(i=0; i<MAX_QUEST; i++)
	{
		memset(&tmp_questlog[i], 0, sizeof(struct quest));
	}

	num_quests = mapif_quests_fromsql(char_id, tmp_questlog);

	WFIFOHEAD(fd,num_quests*sizeof(struct quest)+8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = num_quests*sizeof(struct quest)+8;
	WFIFOL(fd,4) = char_id;

	for(i=0; i<num_quests; i++)
	{
		memcpy(WFIFOP(fd,i*sizeof(struct quest)+8), &tmp_questlog[i], sizeof(struct quest));
	}

	WFIFOSET(fd,num_quests*sizeof(struct quest)+8);

	return 0;
}

//Map server requesting a character's quest log
int mapif_parse_loadquestrequest(int fd)
{
	mapif_send_quests(fd, RFIFOL(fd,2));
	return 0;
}

int inter_quest_parse_frommap(int fd)
{

	switch(RFIFOW(fd,0))
	{
		case 0x3060: mapif_parse_loadquestrequest(fd); break;
		case 0x3061: mapif_parse_quest_add(fd); break;
		case 0x3062: mapif_parse_quest_delete(fd); break;
		default:
			return 0;
	}
	return 1;

}
