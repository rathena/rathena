// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

// original code from athena
// SQL conversion by Jioh L. Jung

#include <string.h>
#include <stdlib.h>

#include "char.h"
#include "itemdb.h"
#include "../common/showmsg.h"

#define STORAGE_MEMINC	16

#ifndef TXT_SQL_CONVERT
// reset by inter_config_read()
struct storage *storage_pt=NULL;
struct guild_storage *guild_storage_pt=NULL;

#ifndef SQL_DEBUG

#define mysql_query(_x, _y) mysql_real_query(_x, _y, strlen(_y)) //supports ' in names and runs faster [Kevin]

#else 

#define mysql_query(_x, _y) debug_mysql_query(__FILE__, __LINE__, _x, _y)

#endif

#endif //TXT_SQL_CONVERT
// storage data -> DB conversion
int storage_tosql(int account_id,struct storage *p){
	int i,j;
//	int eqcount=1;
//	int noteqcount=1;
	int count=0;
	struct itemtmp mapitem[MAX_STORAGE];
	for(i=0;i<MAX_STORAGE;i++){
		if(p->storage_[i].nameid>0){
			mapitem[count].flag=0;
			mapitem[count].id = p->storage_[i].id;
			mapitem[count].nameid=p->storage_[i].nameid;
			mapitem[count].amount = p->storage_[i].amount;
			mapitem[count].equip = p->storage_[i].equip;
			mapitem[count].identify = p->storage_[i].identify;
			mapitem[count].refine = p->storage_[i].refine;
			mapitem[count].attribute = p->storage_[i].attribute;
			for(j=0; j<MAX_SLOTS; j++)
				mapitem[count].card[j] = p->storage_[i].card[j];
			count++;
		}
	}

	memitemdata_to_sql(mapitem, count, account_id,TABLE_STORAGE);

	//printf ("storage dump to DB - id: %d (total: %d)\n", account_id, j);
	return 0;
}
#ifndef TXT_SQL_CONVERT

// DB -> storage data conversion
int storage_fromsql(int account_id, struct storage *p){
	int i=0,j;
	char * str_p = tmp_sql;

	memset(p,0,sizeof(struct storage)); //clean up memory
	p->storage_amount = 0;
	p->account_id = account_id;

	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	str_p += sprintf(str_p,"SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`");
	
	for (j=0; j<MAX_SLOTS; j++)
		str_p += sprintf(str_p, ", `card%d`", j);
	
	str_p += sprintf(str_p," FROM `%s` WHERE `account_id`='%d' ORDER BY `nameid`", storage_db, account_id);
	
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle) ;

	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res)) && i<MAX_STORAGE) {	//start to fetch
			p->storage_[i].id= atoi(sql_row[0]);
			p->storage_[i].nameid= atoi(sql_row[1]);
			p->storage_[i].amount= atoi(sql_row[2]);
			p->storage_[i].equip= atoi(sql_row[3]);
			p->storage_[i].identify= atoi(sql_row[4]);
			p->storage_[i].refine= atoi(sql_row[5]);
			p->storage_[i].attribute= atoi(sql_row[6]);
			for (j=0; j<MAX_SLOTS; j++)
				p->storage_[i].card[j]= atoi(sql_row[7+j]);
			i++;
		}
		p->storage_amount = i;
		mysql_free_result(sql_res);
	}

	ShowInfo ("storage load complete from DB - id: %d (total: %d)\n", account_id, p->storage_amount);
	return 1;
}
#endif //TXT_SQL_CONVERT
// Save guild_storage data to sql
int guild_storage_tosql(int guild_id, struct guild_storage *p){
	int i,j;
//	int eqcount=1;
//	int noteqcount=1;
	int count=0;
	struct itemtmp mapitem[MAX_GUILD_STORAGE];
	for(i=0;i<MAX_GUILD_STORAGE;i++){
		if(p->storage_[i].nameid>0){
			mapitem[count].flag=0;
			mapitem[count].id = p->storage_[i].id;
			mapitem[count].nameid=p->storage_[i].nameid;
			mapitem[count].amount = p->storage_[i].amount;
			mapitem[count].equip = p->storage_[i].equip;
			mapitem[count].identify = p->storage_[i].identify;
			mapitem[count].refine = p->storage_[i].refine;
			mapitem[count].attribute = p->storage_[i].attribute;
			for (j=0; j<MAX_SLOTS; j++)
				mapitem[count].card[j] = p->storage_[i].card[j];
			count++;
		}
	}

	memitemdata_to_sql(mapitem, count, guild_id,TABLE_GUILD_STORAGE);

	ShowInfo ("guild storage save to DB - id: %d (total: %d)\n", guild_id,i);
	return 0;
}
#ifndef TXT_SQL_CONVERT
// Load guild_storage data to mem
int guild_storage_fromsql(int guild_id, struct guild_storage *p){
	int i=0,j;
	struct guild_storage *gs=guild_storage_pt;
	char * str_p = tmp_sql;
	p=gs;

	memset(p,0,sizeof(struct guild_storage)); //clean up memory
	p->storage_amount = 0;
	p->guild_id = guild_id;

	// storage {`guild_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	str_p += sprintf(str_p,"SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`");

	for (j=0; j<MAX_SLOTS; j++)
		str_p += sprintf(str_p, ", `card%d`",  j);
	
	str_p += sprintf(str_p," FROM `%s` WHERE `guild_id`='%d' ORDER BY `nameid`", guild_storage_db, guild_id);
	
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle) ;

	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res)) && i < MAX_GUILD_STORAGE) {	//start to fetch
			p->storage_[i].id= atoi(sql_row[0]);
			p->storage_[i].nameid= atoi(sql_row[1]);
			p->storage_[i].amount= atoi(sql_row[2]);
			p->storage_[i].equip= atoi(sql_row[3]);
			p->storage_[i].identify= atoi(sql_row[4]);
			p->storage_[i].refine= atoi(sql_row[5]);
			p->storage_[i].attribute= atoi(sql_row[6]);
			for (j=0; j<MAX_SLOTS; j++)
				p->storage_[i].card[j] = atoi(sql_row[7+j]);
			i++;
		}
		p->storage_amount = i;
		mysql_free_result(sql_res);
	}
	ShowInfo ("guild storage load complete from DB - id: %d (total: %d)\n", guild_id, p->storage_amount);
	return 0;
}

//---------------------------------------------------------
// storage data initialize
int inter_storage_sql_init(void){

	//memory alloc
	ShowDebug("interserver storage memory initialize....(%d byte)\n",sizeof(struct storage));
	storage_pt = (struct storage*)aCalloc(sizeof(struct storage), 1);
	guild_storage_pt = (struct guild_storage*)aCalloc(sizeof(struct guild_storage), 1);
//	memset(storage_pt,0,sizeof(struct storage)); //Calloc sets stuff to 0 already. [Skotlex]
//	memset(guild_storage_pt,0,sizeof(struct guild_storage));

	return 1;
}
// storage data finalize
void inter_storage_sql_final(void)
{
	if (storage_pt) aFree(storage_pt);
	if (guild_storage_pt) aFree(guild_storage_pt);	 
	return;
}
// q?f[^?
int inter_storage_delete(int account_id)
{
		sprintf(tmp_sql, "DELETE FROM `%s` WHERE `account_id`='%d'",storage_db, account_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	return 0;
}
int inter_guild_storage_delete(int guild_id)
{
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_storage_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	return 0;
}

//---------------------------------------------------------
// packet from map server

// recive packet about storage data
int mapif_load_storage(int fd,int account_id){
	//load from DB
	WFIFOHEAD(fd, sizeof(struct storage)+8);
	storage_fromsql(account_id, storage_pt);
	WFIFOW(fd,0)=0x3810;
	WFIFOW(fd,2)=sizeof(struct storage)+8;
	WFIFOL(fd,4)=account_id;
	memcpy(WFIFOP(fd,8),storage_pt,sizeof(struct storage));
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
// send ack to map server which is "storage data save ok."
int mapif_save_storage_ack(int fd,int account_id){
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0)=0x3811;
	WFIFOL(fd,2)=account_id;
	WFIFOB(fd,6)=0;
	WFIFOSET(fd,7);
	return 0;
}

int mapif_load_guild_storage(int fd,int account_id,int guild_id)
{
	int guild_exist=1;
	WFIFOHEAD(fd, sizeof(struct guild_storage)+12);
	WFIFOW(fd,0)=0x3818;

#if 0	// innodb guilds should render this check unnecessary [Aru]
	// Check if guild exists, I may write a function for this later, coz I use it several times.
	//printf("- Check if guild %d exists\n",g->guild_id);
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		guild_exist =  atoi (sql_row[0]);
		//printf("- Check if guild %d exists : %s\n",g->guild_id,((guild_exist==0)?"No":"Yes"));
	}
	mysql_free_result(sql_res) ; //resource free
#endif
	if(guild_exist==1) {
		guild_storage_fromsql(guild_id,guild_storage_pt);
		WFIFOW(fd,2)=sizeof(struct guild_storage)+12;
		WFIFOL(fd,4)=account_id;
		WFIFOL(fd,8)=guild_id;
		memcpy(WFIFOP(fd,12),guild_storage_pt,sizeof(struct guild_storage));
	}
	else {
		WFIFOW(fd,2)=12;
		WFIFOL(fd,4)=account_id;
		WFIFOL(fd,8)=0;
	}
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}
int mapif_save_guild_storage_ack(int fd,int account_id,int guild_id,int fail)
{
	WFIFOHEAD(fd,11);
	WFIFOW(fd,0)=0x3819;
	WFIFOL(fd,2)=account_id;
	WFIFOL(fd,6)=guild_id;
	WFIFOB(fd,10)=fail;
	WFIFOSET(fd,11);
	return 0;
}

//---------------------------------------------------------
// packet from map server

// recive request about storage data
int mapif_parse_LoadStorage(int fd){
	RFIFOHEAD(fd);
	mapif_load_storage(fd,RFIFOL(fd,2));
	return 0;
}
// storage data recive and save
int mapif_parse_SaveStorage(int fd){
	int account_id;
	int len;
	RFIFOHEAD(fd);
	account_id=RFIFOL(fd,4);
	len=RFIFOW(fd,2);
	if(sizeof(struct storage)!=len-8){
		ShowError("inter storage: data size error %d %d\n",sizeof(struct storage),len-8);
	}else{
		memcpy(&storage_pt[0],RFIFOP(fd,8),sizeof(struct storage));
		storage_tosql(account_id, storage_pt);
		mapif_save_storage_ack(fd,account_id);
	}
	return 0;
}

int mapif_parse_LoadGuildStorage(int fd)
{
	RFIFOHEAD(fd);
	mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}

int mapif_parse_SaveGuildStorage(int fd)
{
	int guild_exist=1;
	int guild_id;
	int len;
	RFIFOHEAD(fd);
	guild_id=RFIFOL(fd,8);
	len=RFIFOW(fd,2);
	if(sizeof(struct guild_storage)!=len-12){
		ShowError("inter storage: data size error %d %d\n",sizeof(struct guild_storage),len-12);
	}
	else {
#if 0	// Again, innodb key checks make the check pointless
		// Check if guild exists, I may write a function for this later, coz I use it several times.
		//printf("- Check if guild %d exists\n",g->guild_id);
		sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
		sql_res = mysql_store_result(&mysql_handle) ;
		if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
			sql_row = mysql_fetch_row(sql_res);
			guild_exist =  atoi (sql_row[0]);
			//printf("- Check if guild %d exists : %s\n",g->guild_id,((guild_exist==0)?"No":"Yes"));
		}
		mysql_free_result(sql_res) ; //resource free
#endif
		if(guild_exist==1) {
			memcpy(guild_storage_pt,RFIFOP(fd,12),sizeof(struct guild_storage));
			guild_storage_tosql(guild_id,guild_storage_pt);
			mapif_save_guild_storage_ack(fd,RFIFOL(fd,4),guild_id,0);
		}
		else
			mapif_save_guild_storage_ack(fd,RFIFOL(fd,4),guild_id,1);
	}
	return 0;
}


int inter_storage_parse_frommap(int fd){
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)){
	case 0x3010: mapif_parse_LoadStorage(fd); break;
	case 0x3011: mapif_parse_SaveStorage(fd); break;
	case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
	case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
	default:
		return 0;
	}
	return 1;
}
#endif //TXT_SQL_CONVERT
