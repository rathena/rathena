// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdlib.h>

bool mercenary_owner_fromsql(uint32 char_id, struct mmo_charstatus *status)
{
	char* data;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith` FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_owner_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); status->mer_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); status->arch_calls = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); status->arch_faith = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); status->spear_calls = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); status->spear_faith = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); status->sword_calls = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); status->sword_faith = atoi(data);
	Sql_FreeResult(sql_handle);

	return true;
}

bool mercenary_owner_tosql(uint32 char_id, struct mmo_charstatus *status)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`char_id`, `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith`) VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
		schema_config.mercenary_owner_db, char_id, status->mer_id, status->arch_calls, status->arch_faith, status->spear_calls, status->spear_faith, status->sword_calls, status->sword_faith) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

bool mercenary_owner_delete(uint32 char_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_owner_db, char_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_db, char_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}

bool mapif_mercenary_save(struct s_mercenary* merc)
{
	bool flag = true;

	if( merc->mercenary_id == 0 )
	{ // Create new DB entry
		if( SQL_ERROR == Sql_Query(sql_handle,
			"INSERT INTO `%s` (`char_id`,`class`,`hp`,`sp`,`kill_counter`,`life_time`) VALUES ('%d','%d','%d','%d','%u','%u')",
			schema_config.mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
			merc->mercenary_id = (int)Sql_LastInsertId(sql_handle);
	}
	else if( SQL_ERROR == Sql_Query(sql_handle,
		"UPDATE `%s` SET `char_id` = '%d', `class` = '%d', `hp` = '%d', `sp` = '%d', `kill_counter` = '%u', `life_time` = '%u' WHERE `mer_id` = '%d'",
		schema_config.mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time, merc->mercenary_id) )
	{ // Update DB entry
		Sql_ShowDebug(sql_handle);
		flag = false;
	}

	return flag;
}

bool mapif_mercenary_load(int merc_id, uint32 char_id, struct s_mercenary *merc)
{
	char* data;

	memset(merc, 0, sizeof(struct s_mercenary));
	merc->mercenary_id = merc_id;
	merc->char_id = char_id;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `class`, `hp`, `sp`, `kill_counter`, `life_time` FROM `%s` WHERE `mer_id` = '%d' AND `char_id` = '%d'", schema_config.mercenary_db, merc_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); merc->class_ = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); merc->hp = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); merc->sp = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); merc->kill_count = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); merc->life_time = atoi(data);
	Sql_FreeResult(sql_handle);
	if( charserv_config.save_log )
		ShowInfo("Mercenary loaded (ID: %d / Class: %d / CID: %d).\n", merc->mercenary_id, merc->class_, merc->char_id);

	return true;
}

bool mapif_mercenary_delete(int merc_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `mer_id` = '%d'", schema_config.mercenary_db, merc_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

static void mapif_mercenary_send(int fd, struct s_mercenary *merc, unsigned char flag)
{
	int size = sizeof(struct s_mercenary) + 5;

	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x3870;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),merc,sizeof(struct s_mercenary));
	WFIFOSET(fd,size);
}

static void mapif_parse_mercenary_create(int fd, struct s_mercenary* merc)
{
	bool result = mapif_mercenary_save(merc);
	mapif_mercenary_send(fd, merc, result);
}

static void mapif_parse_mercenary_load(int fd, int merc_id, uint32 char_id)
{
	struct s_mercenary merc;
	bool result = mapif_mercenary_load(merc_id, char_id, &merc);
	mapif_mercenary_send(fd, &merc, result);
}

static void mapif_mercenary_deleted(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3871;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

static void mapif_parse_mercenary_delete(int fd, int merc_id)
{
	bool result = mapif_mercenary_delete(merc_id);
	mapif_mercenary_deleted(fd, result);
}

static void mapif_mercenary_saved(int fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3872;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

static void mapif_parse_mercenary_save(int fd, struct s_mercenary* merc)
{
	bool result = mapif_mercenary_save(merc);
	mapif_mercenary_saved(fd, result);
}

int inter_mercenary_sql_init(void)
{
	return 0;
}
void inter_mercenary_sql_final(void)
{
	return;
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_mercenary_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);

	switch( cmd )
	{
		case 0x3070: mapif_parse_mercenary_create(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		case 0x3071: mapif_parse_mercenary_load(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
		case 0x3072: mapif_parse_mercenary_delete(fd, (int)RFIFOL(fd,2)); break;
		case 0x3073: mapif_parse_mercenary_save(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}
