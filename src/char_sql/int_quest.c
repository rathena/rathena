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

int inter_quest_parse_frommap(int fd)
{

	switch(RFIFOW(fd,0))
	{
		case 0x3061: mapif_parse_quest_add(fd); break;
		case 0x3062: mapif_parse_quest_delete(fd); break;
		default:
			return 0;
	}
	return 1;

}
