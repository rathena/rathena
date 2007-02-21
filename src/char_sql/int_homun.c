// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"

struct s_homunculus *homun_pt;

#ifndef SQL_DEBUG

#define mysql_query(_x, _y) mysql_real_query(_x, _y, strlen(_y)) //supports ' in names and runs faster [Kevin]

#else

#define mysql_query(_x, _y) debug_mysql_query(__FILE__, __LINE__, _x, _y)

#endif


int inter_homunculus_sql_init(void){
	//memory alloc
	homun_pt = (struct s_homunculus*)aCalloc(sizeof(struct s_homunculus), 1);
	return 0;
}
void inter_homunculus_sql_final(void){
	if (homun_pt) aFree(homun_pt);
	return;
}

int mapif_saved_homunculus(int fd, int account_id, unsigned char flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0) = 0x3892;
	WFIFOL(fd,2) = account_id;
	WFIFOB(fd,6) = flag;
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
int mapif_save_homunculus_skills(struct s_homunculus *hd)
{
	int i;

	for(i=0; i<MAX_HOMUNSKILL; i++)
	{
		if(hd->hskill[i].id != 0 && hd->hskill[i].lv != 0 )
		{
			sprintf(tmp_sql,"REPLACE INTO `skill_homunculus` (`homun_id`, `id`, `lv`) VALUES (%d, %d, %d)",
				hd->hom_id, hd->hskill[i].id, hd->hskill[i].lv);

			if(mysql_query(&mysql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				return 0;
			}
		}
	}

	return 1;
}

int mapif_save_homunculus(int fd, int account_id, struct s_homunculus *hd)
{
	int flag =1;
	char t_name[NAME_LENGTH*2];
	jstrescapecpy(t_name, hd->name);

	if(hd->hom_id==0) // new homunculus
	{
		sprintf(tmp_sql, "INSERT INTO `homunculus` "
			"(`char_id`, `class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`, `rename_flag`, `vaporize`) "
			"VALUES ('%d', '%d', '%s', '%d', '%u', '%u', '%d', '%d', %d, '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			hd->char_id, hd->class_,t_name,hd->level,hd->exp,hd->intimacy,hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp,hd->max_hp,hd->sp,hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize);

	}
	else
	{
		sprintf(tmp_sql, "UPDATE `homunculus` SET `char_id`='%d', `class`='%d',`name`='%s',`level`='%d',`exp`='%u',`intimacy`='%u',`hunger`='%d', `str`='%d', `agi`='%d', `vit`='%d', `int`='%d', `dex`='%d', `luk`='%d', `hp`='%d',`max_hp`='%d',`sp`='%d',`max_sp`='%d',`skill_point`='%d', `rename_flag`='%d', `vaporize`='%d' WHERE `homun_id`='%d'",
			hd->char_id, hd->class_,t_name,hd->level,hd->exp,hd->intimacy,hd->hunger, hd->str, hd->agi, hd->vit, hd->int_, hd->dex, hd->luk,
			hd->hp,hd->max_hp,hd->sp,hd->max_sp, hd->skillpts, hd->rename_flag, hd->vaporize, hd->hom_id);
	}

	if(mysql_query(&mysql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		flag =  0;
	}

	if(hd->hom_id==0 && flag!=0)
		hd->hom_id = (int)mysql_insert_id(&mysql_handle); // new homunculus
	else
	{
		flag = mapif_save_homunculus_skills(hd);
		mapif_saved_homunculus(fd, account_id, flag);
	}
	return flag;
}



// Load an homunculus
int mapif_load_homunculus(int fd){
	int i;
	RFIFOHEAD(fd);
	memset(homun_pt, 0, sizeof(struct s_homunculus));

	sprintf(tmp_sql,"SELECT `homun_id`,`char_id`,`class`,`name`,`level`,`exp`,`intimacy`,`hunger`, `str`, `agi`, `vit`, `int`, `dex`, `luk`, `hp`,`max_hp`,`sp`,`max_sp`,`skill_point`,`rename_flag`, `vaporize` FROM `homunculus` WHERE `homun_id`='%u'", RFIFOL(fd,6));
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);

		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);

		homun_pt->hom_id = RFIFOL(fd,6) ; //RFIFOL(fd,2);
		homun_pt->class_ = atoi(sql_row[2]);
		memcpy(homun_pt->name, sql_row[3],NAME_LENGTH-1);
		homun_pt->char_id = atoi(sql_row[1]);
		homun_pt->level = atoi(sql_row[4]);
		homun_pt->exp = atoi(sql_row[5]);
		homun_pt->intimacy = atoi(sql_row[6]);
		homun_pt->hunger = atoi(sql_row[7]);
		homun_pt->str = atoi(sql_row[8]);
		homun_pt->agi = atoi(sql_row[9]);
		homun_pt->vit = atoi(sql_row[10]);
		homun_pt->int_ = atoi(sql_row[11]);
		homun_pt->dex = atoi(sql_row[12]);
		homun_pt->luk = atoi(sql_row[13]);
		homun_pt->hp = atoi(sql_row[14]);
		homun_pt->max_hp = atoi(sql_row[15]);
		homun_pt->sp = atoi(sql_row[16]);
		homun_pt->max_sp = atoi(sql_row[17]);
		homun_pt->skillpts = atoi(sql_row[18]);
		homun_pt->rename_flag = atoi(sql_row[19]);
		homun_pt->vaporize = atoi(sql_row[20]);
	}
	if(homun_pt->hunger < 0)
		homun_pt->hunger = 0;
	else if(homun_pt->hunger > 100)
		homun_pt->hunger = 100;
	if(homun_pt->intimacy > 100000)
		homun_pt->intimacy = 100000;

	mysql_free_result(sql_res);

	// Load Homunculus Skill
	memset(homun_pt->hskill, 0, sizeof(homun_pt->hskill));

	sprintf(tmp_sql,"SELECT `id`,`lv` FROM `skill_homunculus` WHERE `homun_id`=%d",homun_pt->hom_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
	}
	sql_res = mysql_store_result(&mysql_handle);
         if(sql_res){
         	while((sql_row = mysql_fetch_row(sql_res))){
						i = (atoi(sql_row[0])-HM_SKILLBASE-1);
						homun_pt->hskill[i].id = atoi(sql_row[0]);
                        homun_pt->hskill[i].lv = atoi(sql_row[1]);
                 }
         }

	mysql_free_result(sql_res);

	if (save_log)
		ShowInfo("Homunculus loaded (%d - %s).\n", homun_pt->hom_id, homun_pt->name);
	return mapif_info_homunculus(fd, RFIFOL(fd,2), homun_pt);
}

int inter_delete_homunculus(int hom_id)
{
	sprintf(tmp_sql, "DELETE FROM `homunculus` WHERE `homun_id` = '%u'", hom_id);
	if(mysql_query(&mysql_handle, tmp_sql))
	{
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
	}
	
	sprintf(tmp_sql, "DELETE FROM `skill_homunculus` WHERE `homun_id` = '%u'", hom_id);
	if(mysql_query(&mysql_handle, tmp_sql))
	{
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
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
