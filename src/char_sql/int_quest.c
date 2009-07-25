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

	memcpy(&qd, RFIFOP(fd,8), RFIFOW(fd,2)-8);

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `mob1`, `count1`, `mob2`, `count2`, `mob3`, `count3`) VALUES ('%d', '%d', '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d')", quest_db, qd.quest_id, char_id, qd.state, qd.time, qd.mob[0], qd.count[0], qd.mob[1], qd.count[1], qd.mob[2], qd.count[2]);

	if ( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ) 
	{
		Sql_ShowDebug(sql_handle);
		success = false;
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

//Update a questlog
int mapif_parse_quest_update(int fd)
{

	StringBuf buf;
	bool success = true;
	int char_id = RFIFOL(fd,4);
	struct quest qd;

	memcpy(&qd, RFIFOP(fd,8), RFIFOW(fd,2)-8);

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' WHERE `quest_id` = '%d' AND `char_id` = '%d'", quest_db, qd.state, qd.count[0], qd.count[1], qd.count[2], qd.quest_id, char_id);

	if ( SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf)) ) 
	{
		Sql_ShowDebug(sql_handle);
		success = false;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x3863;
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
		case 0x3063: mapif_parse_quest_update(fd); break;
		default:
			return 0;
	}
	return 1;

}
