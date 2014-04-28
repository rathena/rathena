// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/utils.h"
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int inter_homunculus_sql_init(void)
{
	return 0;
}
void inter_homunculus_sql_final(void)
{
	return;
}

static void mapif_homunculus_created(int fd, int account_id, struct s_homunculus *sh, unsigned char flag)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3890;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8)= flag;
	memcpy(WFIFOP(fd,9),sh,sizeof(struct s_homunculus));
	WFIFOSET(fd, WFIFOW(fd,2));
}

static void mapif_homunculus_deleted(int fd, int flag)
{
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) = 0x3893;
	WFIFOB(fd,2) = flag; //Flag 1 = success
	WFIFOSET(fd, 3);
}

static void mapif_homunculus_loaded(int fd, int account_id, struct s_homunculus *hd)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3891;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	if( hd != NULL )
	{
		WFIFOB(fd,8) = 1; // success
		memcpy(WFIFOP(fd,9), hd, sizeof(struct s_homunculus));
	}
	else
	{
		WFIFOB(fd,8) = 0; // not found.
		memset(WFIFOP(fd,9), 0, sizeof(struct s_homunculus));
	}
	WFIFOSET(fd, sizeof(struct s_homunculus)+9);
}

static void mapif_homunculus_saved(int fd, int account_id, bool flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0) = 0x3892;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag; // 1:success, 0:failure
	WFIFOSET(fd, 7);
}

static void mapif_homunculus_renamed(int fd, int account_id, int char_id, unsigned char flag, char* name)
{
	WFIFOHEAD(fd, NAME_LENGTH+12);
	WFIFOW(fd, 0) = 0x3894;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = flag;
	safestrncpy((char*)WFIFOP(fd,11), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+12);
}

bool mapif_homunculus_save(struct s_homunculus* hd)
{
	bool flag = true;
	char esc_name[NAME_LENGTH*2+1];

	Sql_EscapeStringLen(sql_handle, esc_name, hd->name, strnlen(hd->name, NAME_LENGTH));

	if( hd->hom_id == 0 )
	{// new homunculus
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` "
			"(`char_id`, `class`,`prev_class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`, `rename_flag`, `vaporize`) "
			"VALUES ('%d', '%d', '%d', '%s', '%d', '%u', '%u', '%d', '%d', %d, '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			schema_config.homunculus_db, hd->char_id, hd->class_, hd->prev_class, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp, hd->max_hp, hd->sp, hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
		{
			hd->hom_id = (int)Sql_LastInsertId(sql_handle);
		}
	}
	else
	{
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `char_id`='%d', `class`='%d',`prev_class`='%d',`name`='%s',`level`='%d',`exp`='%u',`intimacy`='%u',`hunger`='%d', `str`='%d', `agi`='%d', `vit`='%d', `int`='%d', `dex`='%d', `luk`='%d', `hp`='%d',`max_hp`='%d',`sp`='%d',`max_sp`='%d',`skill_point`='%d', `rename_flag`='%d', `vaporize`='%d' WHERE `homun_id`='%d'",
			schema_config.homunculus_db, hd->char_id, hd->class_, hd->prev_class, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp, hd->max_hp, hd->sp, hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize, hd->hom_id) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
		{
			SqlStmt* stmt;
			int i;

			stmt = SqlStmt_Malloc(sql_handle);
			if( SQL_ERROR == SqlStmt_Prepare(stmt, "REPLACE INTO `%s` (`homun_id`, `id`, `lv`) VALUES (%d, ?, ?)", schema_config.skill_homunculus_db, hd->hom_id) )
				SqlStmt_ShowDebug(stmt);
			for( i = 0; i < MAX_HOMUNSKILL; ++i )
			{
				if( hd->hskill[i].id > 0 && hd->hskill[i].lv != 0 )
				{
					SqlStmt_BindParam(stmt, 0, SQLDT_USHORT, &hd->hskill[i].id, 0);
					SqlStmt_BindParam(stmt, 1, SQLDT_USHORT, &hd->hskill[i].lv, 0);
					if( SQL_ERROR == SqlStmt_Execute(stmt) )
					{
						SqlStmt_ShowDebug(stmt);
						SqlStmt_Free(stmt);
						flag = false;
						break;
					}
				}
			}
			SqlStmt_Free(stmt);
		}
	}

	return flag;
}



// Load an homunculus
bool mapif_homunculus_load(int homun_id, struct s_homunculus* hd)
{
	char* data;
	size_t len;

	memset(hd, 0, sizeof(*hd));

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `homun_id`,`char_id`,`class`,`prev_class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`, `vaporize` FROM `%s` WHERE `homun_id`='%u'",
		schema_config.homunculus_db, homun_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( !Sql_NumRows(sql_handle) )
	{	//No homunculus found.
		Sql_FreeResult(sql_handle);
		return false;
	}
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return false;
	}

	hd->hom_id = homun_id;
	Sql_GetData(sql_handle,  1, &data, NULL); hd->char_id = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); hd->class_ = atoi(data);
	Sql_GetData(sql_handle,  3, &data, NULL); hd->prev_class = atoi(data);
	Sql_GetData(sql_handle,  4, &data, &len); safestrncpy(hd->name, data, sizeof(hd->name));
	Sql_GetData(sql_handle,  5, &data, NULL); hd->level = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); hd->exp = atoi(data);
	Sql_GetData(sql_handle,  7, &data, NULL); hd->intimacy = (unsigned int)strtoul(data, NULL, 10);
	Sql_GetData(sql_handle,  8, &data, NULL); hd->hunger = atoi(data);
	Sql_GetData(sql_handle,  9, &data, NULL); hd->str = atoi(data);
	Sql_GetData(sql_handle, 10, &data, NULL); hd->agi = atoi(data);
	Sql_GetData(sql_handle, 11, &data, NULL); hd->vit = atoi(data);
	Sql_GetData(sql_handle, 12, &data, NULL); hd->int_ = atoi(data);
	Sql_GetData(sql_handle, 13, &data, NULL); hd->dex = atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); hd->luk = atoi(data);
	Sql_GetData(sql_handle, 15, &data, NULL); hd->hp = atoi(data);
	Sql_GetData(sql_handle, 16, &data, NULL); hd->max_hp = atoi(data);
	Sql_GetData(sql_handle, 17, &data, NULL); hd->sp = atoi(data);
	Sql_GetData(sql_handle, 18, &data, NULL); hd->max_sp = atoi(data);
	Sql_GetData(sql_handle, 19, &data, NULL); hd->skillpts = atoi(data);
	Sql_GetData(sql_handle, 20, &data, NULL); hd->rename_flag = atoi(data);
	Sql_GetData(sql_handle, 21, &data, NULL); hd->vaporize = atoi(data);
	Sql_FreeResult(sql_handle);

	hd->intimacy = min(hd->intimacy,100000);
	hd->hunger = cap_value(hd->hunger, 0, 100);

	// Load Homunculus Skill
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`lv` FROM `%s` WHERE `homun_id`=%d", schema_config.skill_homunculus_db, homun_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}
	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		int i;
		// id
		Sql_GetData(sql_handle, 0, &data, NULL);
		i = atoi(data);
		if( i < HM_SKILLBASE || i >= HM_SKILLBASE + MAX_HOMUNSKILL )
			continue;// invalid skill id
		i = i - HM_SKILLBASE;
		hd->hskill[i].id = (unsigned short)atoi(data);

		// lv
		Sql_GetData(sql_handle, 1, &data, NULL);
		hd->hskill[i].lv = (unsigned char)atoi(data);
	}
	Sql_FreeResult(sql_handle);

	if( charserv_config.save_log )
		ShowInfo("Homunculus loaded (%d - %s).\n", hd->hom_id, hd->name);

	return true;
}

bool mapif_homunculus_delete(int homun_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `homun_id` = '%u'", schema_config.homunculus_db, homun_id)
	||	SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `homun_id` = '%u'", schema_config.skill_homunculus_db, homun_id)
	) {
		Sql_ShowDebug(sql_handle);
		return false;
	}
	return true;
}

bool mapif_homunculus_rename(char *name)
{
	int i;

	// Check Authorised letters/symbols in the name of the homun
	if( charserv_config.char_config.char_name_option == 1 )
	{// only letters/symbols in char_name_letters are authorised
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(charserv_config.char_config.char_name_letters, name[i]) == NULL )
				return false;
	} else
	if( charserv_config.char_config.char_name_option == 2 )
	{// letters/symbols in char_name_letters are forbidden
		for( i = 0; i < NAME_LENGTH && name[i]; i++ )
			if( strchr(charserv_config.char_config.char_name_letters, name[i]) != NULL )
				return false;
	}

	return true;
}


static void mapif_parse_homunculus_create(int fd, int len, int account_id, struct s_homunculus* phd)
{
	bool result = mapif_homunculus_save(phd);
	mapif_homunculus_created(fd, account_id, phd, result);
}

static void mapif_parse_homunculus_delete(int fd, int homun_id)
{
	bool result = mapif_homunculus_delete(homun_id);
	mapif_homunculus_deleted(fd, result);
}

static void mapif_parse_homunculus_load(int fd, int account_id, int homun_id)
{
	struct s_homunculus hd;
	bool result = mapif_homunculus_load(homun_id, &hd);
	mapif_homunculus_loaded(fd, account_id, ( result ? &hd : NULL ));
}

static void mapif_parse_homunculus_save(int fd, int len, int account_id, struct s_homunculus* phd)
{
	bool result = mapif_homunculus_save(phd);
	mapif_homunculus_saved(fd, account_id, result);
}

static void mapif_parse_homunculus_rename(int fd, int account_id, int char_id, char* name)
{
	bool result = mapif_homunculus_rename(name);
	mapif_homunculus_renamed(fd, account_id, char_id, result, name);
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int inter_homunculus_parse_frommap(int fd)
{
	unsigned short cmd = RFIFOW(fd,0);

	switch( cmd )
	{
		case 0x3090: mapif_parse_homunculus_create(fd, (int)RFIFOW(fd,2), (int)RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,8)); break;
		case 0x3091: mapif_parse_homunculus_load  (fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6)); break;
		case 0x3092: mapif_parse_homunculus_save  (fd, (int)RFIFOW(fd,2), (int)RFIFOL(fd,4), (struct s_homunculus*)RFIFOP(fd,8)); break;
		case 0x3093: mapif_parse_homunculus_delete(fd, (int)RFIFOL(fd,2)); break;
		case 0x3094: mapif_parse_homunculus_rename(fd, (int)RFIFOL(fd,2), (int)RFIFOL(fd,6), (char*)RFIFOP(fd,10)); break;
		default:
			return 0;
	}
	return 1;
}
