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

struct s_homunculus *homun_pt;

int inter_homunculus_sql_init(void){
	//memory alloc
	homun_pt = (struct s_homunculus*)aCalloc(sizeof(struct s_homunculus), 1);
	return 0;
}
void inter_homunculus_sql_final(void){
	if (homun_pt) aFree(homun_pt);
	return;
}

static int mapif_saved_homunculus(int fd, int account_id, bool flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0) = 0x3892;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag; // 1:success, 0:failure
	WFIFOSET(fd, 7);
	return 0;
}
int mapif_info_homunculus(int fd, int account_id, struct s_homunculus *hd)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3891;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = 1; // account loaded with success

	memcpy(WFIFOP(fd,9), hd, sizeof(struct s_homunculus));
	WFIFOSET(fd, sizeof(struct s_homunculus)+9);
	return 0;
}

int mapif_noinfo_homunculus(int fd, int account_id)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3891;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = 0; // not found.
	memset(WFIFOP(fd,9), 0, sizeof(struct s_homunculus));
	WFIFOSET(fd, sizeof(struct s_homunculus)+9);
	return 0;
}

int mapif_homunculus_deleted(int fd, int flag)
{
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) = 0x3893;
	WFIFOB(fd,2) = flag; //Flag 1 = success
	WFIFOSET(fd, 3);
	return 0;

}
int mapif_homunculus_created(int fd, int account_id, struct s_homunculus *sh, unsigned char flag)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3890;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8)= flag;
	memcpy(WFIFOP(fd,9),sh,sizeof(struct s_homunculus));
	WFIFOSET(fd, WFIFOW(fd,2));
	return 0;
}

// Save/Update Homunculus Skills
static bool mapif_save_homunculus_skills(struct s_homunculus *hd)
{
	SqlStmt* stmt;
	int i;

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_Prepare(stmt, "REPLACE INTO `skill_homunculus` (`homun_id`, `id`, `lv`) VALUES (%d, ?, ?)", hd->hom_id) )
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
				return false;
			}
		}
	}
	SqlStmt_Free(stmt);

	return true;
}

static bool mapif_save_homunculus(int fd, int account_id, struct s_homunculus *hd)
{
	bool flag = true;
	char esc_name[NAME_LENGTH*2+1];

	Sql_EscapeStringLen(sql_handle, esc_name, hd->name, strnlen(hd->name, NAME_LENGTH));

	if( hd->hom_id == 0 )
	{// new homunculus
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `homunculus` "
			"(`char_id`, `class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`, `rename_flag`, `vaporize`) "
			"VALUES ('%d', '%d', '%s', '%d', '%u', '%u', '%d', '%d', %d, '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			hd->char_id, hd->class_, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
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
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `homunculus` SET `char_id`='%d', `class`='%d',`name`='%s',`level`='%d',`exp`='%u',`intimacy`='%u',`hunger`='%d', `str`='%d', `agi`='%d', `vit`='%d', `int`='%d', `dex`='%d', `luk`='%d', `hp`='%d',`max_hp`='%d',`sp`='%d',`max_sp`='%d',`skill_point`='%d', `rename_flag`='%d', `vaporize`='%d' WHERE `homun_id`='%d'",
			hd->char_id, hd->class_, esc_name, hd->level, hd->exp, hd->intimacy, hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp, hd->max_hp, hd->sp, hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize, hd->hom_id) )
		{
			Sql_ShowDebug(sql_handle);
			flag = false;
		}
		else
		{
			flag = mapif_save_homunculus_skills(hd);
			mapif_saved_homunculus(fd, account_id, flag);
		}
	}

	return flag;
}



// Load an homunculus
int mapif_load_homunculus(int fd)
{
	int i;
	char* data;
	size_t len;

	memset(homun_pt, 0, sizeof(struct s_homunculus));
	RFIFOHEAD(fd);

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `homun_id`,`char_id`,`class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`, `vaporize` FROM `homunculus` WHERE `homun_id`='%u'", RFIFOL(fd,6)) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( !Sql_NumRows(sql_handle) )
	{	//No homunculus found.
		mapif_noinfo_homunculus(fd, RFIFOL(fd,2));
		Sql_FreeResult(sql_handle);
		return 0;
	}
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_ShowDebug(sql_handle);
		Sql_FreeResult(sql_handle);
		return 0;
	}
	homun_pt->hom_id = RFIFOL(fd,6);
	Sql_GetData(sql_handle,  1, &data, NULL); homun_pt->char_id = atoi(data);
	Sql_GetData(sql_handle,  2, &data, NULL); homun_pt->class_ = atoi(data);
	Sql_GetData(sql_handle,  3, &data, &len); memcpy(homun_pt->name, data, min(len, NAME_LENGTH));
	Sql_GetData(sql_handle,  4, &data, NULL); homun_pt->level = atoi(data);
	Sql_GetData(sql_handle,  5, &data, NULL); homun_pt->exp = atoi(data);
	Sql_GetData(sql_handle,  6, &data, NULL); homun_pt->intimacy = (unsigned int)strtoul(data, NULL, 10);
	homun_pt->intimacy = cap_value(homun_pt->intimacy, 0, 100000);
	Sql_GetData(sql_handle,  7, &data, NULL); homun_pt->hunger = atoi(data);
	homun_pt->hunger = cap_value(homun_pt->hunger, 0, 100);
	Sql_GetData(sql_handle,  8, &data, NULL); homun_pt->str = atoi(data);
	Sql_GetData(sql_handle,  9, &data, NULL); homun_pt->agi = atoi(data);
	Sql_GetData(sql_handle, 10, &data, NULL); homun_pt->vit = atoi(data);
	Sql_GetData(sql_handle, 11, &data, NULL); homun_pt->int_ = atoi(data);
	Sql_GetData(sql_handle, 12, &data, NULL); homun_pt->dex = atoi(data);
	Sql_GetData(sql_handle, 13, &data, NULL); homun_pt->luk = atoi(data);
	Sql_GetData(sql_handle, 14, &data, NULL); homun_pt->hp = atoi(data);
	Sql_GetData(sql_handle, 15, &data, NULL); homun_pt->max_hp = atoi(data);
	Sql_GetData(sql_handle, 16, &data, NULL); homun_pt->sp = atoi(data);
	Sql_GetData(sql_handle, 17, &data, NULL); homun_pt->max_sp = atoi(data);
	Sql_GetData(sql_handle, 18, &data, NULL); homun_pt->skillpts = atoi(data);
	Sql_GetData(sql_handle, 19, &data, NULL); homun_pt->rename_flag = atoi(data);
	Sql_GetData(sql_handle, 20, &data, NULL); homun_pt->vaporize = atoi(data);
	Sql_FreeResult(sql_handle);

	// Load Homunculus Skill
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `id`,`lv` FROM `skill_homunculus` WHERE `homun_id`=%d", homun_pt->hom_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}
	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		// id
		Sql_GetData(sql_handle, 0, &data, NULL);
		i = atoi(data);
		if( i < HM_SKILLBASE || i >= HM_SKILLBASE + MAX_HOMUNSKILL )
			continue;// invalid guild skill
		i = i - HM_SKILLBASE;
		homun_pt->hskill[i].id = (unsigned short)atoi(data);
		// lv
		Sql_GetData(sql_handle, 1, &data, NULL);
		homun_pt->hskill[i].lv = (unsigned short)atoi(data);
	}
	Sql_FreeResult(sql_handle);

	if( save_log )
		ShowInfo("Homunculus loaded (%d - %s).\n", homun_pt->hom_id, homun_pt->name);
	return mapif_info_homunculus(fd, RFIFOL(fd,2), homun_pt);
}

int inter_delete_homunculus(int hom_id)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `homunculus` WHERE `homun_id` = '%u'", hom_id) ||
		SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `skill_homunculus` WHERE `homun_id` = '%u'", hom_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}
	return 1;
}

int mapif_delete_homunculus(int fd)
{
	RFIFOHEAD(fd);
	mapif_homunculus_deleted(fd, inter_delete_homunculus(RFIFOL(fd,2)));
	return 1;
}

int mapif_rename_homun_ack(int fd, int account_id, int char_id, unsigned char flag, char *name){
	WFIFOHEAD(fd, NAME_LENGTH+12);
	WFIFOW(fd, 0) =0x3894;
	WFIFOL(fd, 2) =account_id;
	WFIFOL(fd, 6) =char_id;
	WFIFOB(fd, 10) =flag;
	memcpy(WFIFOP(fd, 11), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+12);

	return 0;
}

int mapif_rename_homun(int fd, int account_id, int char_id, char *name){
	int i;

	// Check Authorised letters/symbols in the name of the homun
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) == NULL) {
			mapif_rename_homun_ack(fd, account_id, char_id, 0, name);
			return 0;
		}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) != NULL) {
			mapif_rename_homun_ack(fd, account_id, char_id, 0, name);
			return 0;
		}
	}

	mapif_rename_homun_ack(fd, account_id, char_id, 1, name);
	return 0;
}

int mapif_parse_CreateHomunculus(int fd)
{
	RFIFOHEAD(fd);
	memcpy(homun_pt, RFIFOP(fd,8), sizeof(struct s_homunculus));
	// Save in sql db
	if(mapif_save_homunculus(fd,RFIFOL(fd,4), homun_pt))
		return mapif_homunculus_created(fd, RFIFOL(fd,4), homun_pt, 1); // send homun_id
	return mapif_homunculus_created(fd, RFIFOL(fd,4), homun_pt, 0); // fail
}

int inter_homunculus_parse_frommap(int fd){
	RFIFOHEAD(fd);
	switch(RFIFOW(fd, 0)){
	case 0x3090: mapif_parse_CreateHomunculus(fd); break;
	case 0x3091: mapif_load_homunculus(fd); break;
	case 0x3092: mapif_save_homunculus(fd, RFIFOW(fd,4), (struct s_homunculus*) RFIFOP(fd, 8)); break;
	case 0x3093: mapif_delete_homunculus(fd); break;  // doesn't need to be parse, very simple packet...
	case 0x3094: mapif_rename_homun(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOP(fd, 10)); break;
	default:
		return 0;
	}
	return 1;
}
