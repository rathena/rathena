// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_elemental.hpp"

#include <stdlib.h>
#include <string.h>

#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"

#include "char.hpp"
#include "inter.hpp"

bool mapif_elemental_save(struct s_elemental* ele) {
	bool flag = true;

	if( ele->elemental_id == 0 ) { // Create new DB entry
		if( SQL_ERROR == Sql_Query(sql_handle,
								   "INSERT INTO `%s` (`char_id`,`class`,`mode`,`hp`,`sp`,`max_hp`,`max_sp`,`atk1`,`atk2`,`matk`,`aspd`,`def`,`mdef`,`flee`,`hit`,`life_time`)"
								   "VALUES ('%d','%d','%d','%u','%u','%u','%u','%d','%d','%d','%d','%d','%d','%d','%d','%u')",
								   schema_config.elemental_db, ele->char_id, ele->class_, ele->mode, ele->hp, ele->sp, ele->max_hp, ele->max_sp, ele->atk, ele->atk2, ele->matk, ele->amotion, ele->def, ele->mdef, ele->flee, ele->hit, ele->life_time) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
			ele->elemental_id = (int)Sql_LastInsertId(sql_handle);
	} else if( SQL_ERROR == Sql_Query(sql_handle,
									"UPDATE `%s` SET `char_id` = '%d', `class` = '%d', `mode` = '%d', `hp` = '%u', `sp` = '%u',"
									"`max_hp` = '%u', `max_sp` = '%u', `atk1` = '%d', `atk2` = '%d', `matk` = '%d', `aspd` = '%d', `def` = '%d',"
									"`mdef` = '%d', `flee` = '%d', `hit` = '%d', `life_time` = '%u' WHERE `ele_id` = '%d'", schema_config.elemental_db,
									ele->char_id, ele->class_, ele->mode, ele->hp, ele->sp, ele->max_hp, ele->max_sp, ele->atk, ele->atk2,
									ele->matk, ele->amotion, ele->def, ele->mdef, ele->flee, ele->hit, ele->life_time, ele->elemental_id) )
	{ // Update DB entry
		Sql_ShowDebug(sql_handle);
		flag = false;
	}
	return flag;
}

bool mapif_elemental_load(int ele_id, uint32 char_id, struct s_elemental *ele) {
	char* data;

	memset(ele, 0, sizeof(struct s_elemental));
	ele->elemental_id = ele_id;
	ele->char_id = char_id;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `class`, `mode`, `hp`, `sp`, `max_hp`, `max_sp`, `atk1`, `atk2`, `matk`, `aspd`,"
							   "`def`, `mdef`, `flee`, `hit`, `life_time` FROM `%s` WHERE `ele_id` = '%d' AND `char_id` = '%d'",
							   schema_config.elemental_db, ele_id, char_id) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) ) {
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, NULL); ele->class_ = atoi(data);
	Sql_GetData(sql_handle,  1, &data, NULL); ele->mode = (e_mode)atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); ele->hp = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); ele->sp = atoi(data);
	Sql_GetData(sql_handle,  4, &data, NULL); ele->max_hp = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); ele->max_sp = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); ele->atk = atoi(data);
	Sql_GetData(sql_handle,  7, &data, NULL); ele->atk2 = atoi(data);
	Sql_GetData(sql_handle,  8, &data, NULL); ele->matk = atoi(data);
	Sql_GetData(sql_handle,  9, &data, NULL); ele->amotion = atoi(data);
	Sql_GetData(sql_handle, 10, &data, NULL); ele->def = atoi(data);
	Sql_GetData(sql_handle, 11, &data, NULL); ele->mdef = atoi(data);
	Sql_GetData(sql_handle, 12, &data, NULL); ele->flee = atoi(data);
	Sql_GetData(sql_handle, 13, &data, NULL); ele->hit = atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); ele->life_time = atoi(data);
	Sql_FreeResult(sql_handle);
	if( charserv_config.save_log )
		ShowInfo("Elemental loaded (ID: %d / Class: %d / CID: %d).\n", ele->elemental_id, ele->class_, ele->char_id);

	return true;
}

bool mapif_elemental_delete(int ele_id) {
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `ele_id` = '%d'", schema_config.elemental_db, ele_id) ) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

void mapif_elemental_send(int fd, struct s_elemental *ele, unsigned char flag) {
	int size = sizeof(struct s_elemental) + 5;

	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x387c;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),ele,sizeof(struct s_elemental));
	WFIFOSET(fd,size);
}

void mapif_parse_elemental_create(int fd, struct s_elemental* ele) {
	bool result = mapif_elemental_save(ele);
	mapif_elemental_send(fd, ele, result);
}

void mapif_parse_elemental_load(int fd, int ele_id, uint32 char_id) {
	struct s_elemental ele;
	bool result = mapif_elemental_load(ele_id, char_id, &ele);
	mapif_elemental_send(fd, &ele, result);
}

void mapif_elemental_deleted(int fd, unsigned char flag) {
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x387d;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_elemental_delete(int fd, int ele_id) {
	bool result = mapif_elemental_delete(ele_id);
	mapif_elemental_deleted(fd, result);
}

void mapif_elemental_saved(int fd, unsigned char flag) {
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x387e;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_elemental_save(int fd, struct s_elemental* ele) {
	bool result = mapif_elemental_save(ele);
	mapif_elemental_saved(fd, result);
}

void inter_elemental_sql_init(void) {
	return;
}
void inter_elemental_sql_final(void) {
	return;
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_elemental_parse_frommap(int fd) {
	unsigned short cmd = RFIFOW(fd,0);

	switch( cmd ) {
		case 0x307c: mapif_parse_elemental_create(fd, (struct s_elemental*)RFIFOP(fd,4)); break;
		case 0x307d: mapif_parse_elemental_load(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
		case 0x307e: mapif_parse_elemental_delete(fd, (int)RFIFOL(fd,2)); break;
		case 0x307f: mapif_parse_elemental_save(fd, (struct s_elemental*)RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}
