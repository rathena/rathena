//
// original code from athena
// SQL conversion by Jioh L. Jung
//
#include "char.h"
#include "itemdb.h"
#include <string.h>
#include <stdlib.h>

#define STORAGE_MEMINC	16

// reset by inter_config_read()
struct storage *storage_pt=NULL;
struct guild_storage *guild_storage_pt=NULL;


// storage data -> DB conversion
int storage_tosql(int account_id,struct storage *p){
	int i;
	int eqcount=1;
	int noteqcount=1;
	struct itemtemp mapitem;
	for(i=0;i<MAX_STORAGE;i++){
		if(p->storage[i].nameid>0){
			if(itemdb_isequip(p->storage[i].nameid)==1){				
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->storage[i].id;
				mapitem.equip[eqcount].nameid=p->storage[i].nameid;
				mapitem.equip[eqcount].amount = p->storage[i].amount;
				mapitem.equip[eqcount].equip = p->storage[i].equip;
				mapitem.equip[eqcount].identify = p->storage[i].identify;
				mapitem.equip[eqcount].refine = p->storage[i].refine;
				mapitem.equip[eqcount].attribute = p->storage[i].attribute;
				mapitem.equip[eqcount].card[0] = p->storage[i].card[0];
				mapitem.equip[eqcount].card[1] = p->storage[i].card[1];
				mapitem.equip[eqcount].card[2] = p->storage[i].card[2];
				mapitem.equip[eqcount].card[3] = p->storage[i].card[3];
				eqcount++;
			}
			else if(itemdb_isequip(p->storage[i].nameid)==0){				
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->storage[i].id;
				mapitem.notequip[noteqcount].nameid=p->storage[i].nameid;
				mapitem.notequip[noteqcount].amount = p->storage[i].amount;
				mapitem.notequip[noteqcount].equip = p->storage[i].equip;
				mapitem.notequip[noteqcount].identify = p->storage[i].identify;
				mapitem.notequip[noteqcount].refine = p->storage[i].refine;
				mapitem.notequip[noteqcount].attribute = p->storage[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->storage[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->storage[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->storage[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->storage[i].card[3];
				noteqcount++;
			}
		}
	}

	memitemdata_to_sql(mapitem, eqcount, noteqcount, account_id,TABLE_STORAGE);

	//printf ("storage dump to DB - id: %d (total: %d)\n", account_id, j);
	return 0;
}

// DB -> storage data conversion
int storage_fromsql(int account_id, struct storage *p){
	int i=0;
	
	memset(p,0,sizeof(struct storage)); //clean up memory
	p->storage_amount = 0;
	p->account_id = account_id;
	
	// storage {`account_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	sprintf(tmp_sql,"SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3` FROM `%s` WHERE `account_id`='%d'",storage_db, account_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	
	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res))) {	//start to fetch
			p->storage[i].id= atoi(sql_row[0]);
			p->storage[i].nameid= atoi(sql_row[1]);
			p->storage[i].amount= atoi(sql_row[2]);
			p->storage[i].equip= atoi(sql_row[3]);
			p->storage[i].identify= atoi(sql_row[4]);
			p->storage[i].refine= atoi(sql_row[5]);
			p->storage[i].attribute= atoi(sql_row[6]);
			p->storage[i].card[0]= atoi(sql_row[7]);
			p->storage[i].card[1]= atoi(sql_row[8]);
			p->storage[i].card[2]= atoi(sql_row[9]);
			p->storage[i].card[3]= atoi(sql_row[10]);
			p->storage_amount = ++i;
		}
		mysql_free_result(sql_res);
	}
	
	printf ("storage load complete from DB - id: %d (total: %d)\n", account_id, p->storage_amount);
	return 1;
}

// Save guild_storage data to sql
int guild_storage_tosql(int guild_id, struct guild_storage *p){
	int i;
	int eqcount=1;
	int noteqcount=1;
	struct itemtemp mapitem;
	for(i=0;i<MAX_GUILD_STORAGE;i++){
		if(p->storage[i].nameid>0){
			if(itemdb_isequip(p->storage[i].nameid)==1){				
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->storage[i].id;
				mapitem.equip[eqcount].nameid=p->storage[i].nameid;
				mapitem.equip[eqcount].amount = p->storage[i].amount;
				mapitem.equip[eqcount].equip = p->storage[i].equip;
				mapitem.equip[eqcount].identify = p->storage[i].identify;
				mapitem.equip[eqcount].refine = p->storage[i].refine;
				mapitem.equip[eqcount].attribute = p->storage[i].attribute;
				mapitem.equip[eqcount].card[0] = p->storage[i].card[0];
				mapitem.equip[eqcount].card[1] = p->storage[i].card[1];
				mapitem.equip[eqcount].card[2] = p->storage[i].card[2];
				mapitem.equip[eqcount].card[3] = p->storage[i].card[3];
				eqcount++;
			}
			else if(itemdb_isequip(p->storage[i].nameid)==0){				
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->storage[i].id;
				mapitem.notequip[noteqcount].nameid=p->storage[i].nameid;
				mapitem.notequip[noteqcount].amount = p->storage[i].amount;
				mapitem.notequip[noteqcount].equip = p->storage[i].equip;
				mapitem.notequip[noteqcount].identify = p->storage[i].identify;
				mapitem.notequip[noteqcount].refine = p->storage[i].refine;
				mapitem.notequip[noteqcount].attribute = p->storage[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->storage[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->storage[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->storage[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->storage[i].card[3];
				noteqcount++;
			}
		}
	}

	memitemdata_to_sql(mapitem, eqcount, noteqcount, guild_id,TABLE_GUILD_STORAGE);

	printf ("guild storage save to DB - id: %d (total: %d)\n", guild_id,i);
	return 0;
}

// Load guild_storage data to mem
int guild_storage_fromsql(int guild_id, struct guild_storage *p){
	int i=0;
	struct guild_storage *gs=guild_storage_pt;
	p=gs;

	memset(p,0,sizeof(struct guild_storage)); //clean up memory
	p->storage_amount = 0;
	p->guild_id = guild_id;

	// storage {`guild_id`/`id`/`nameid`/`amount`/`equip`/`identify`/`refine`/`attribute`/`card0`/`card1`/`card2`/`card3`}
	sprintf(tmp_sql,"SELECT `id`,`nameid`,`amount`,`equip`,`identify`,`refine`,`attribute`,`card0`,`card1`,`card2`,`card3` FROM `%s` WHERE `guild_id`='%d'",guild_storage_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	
	if (sql_res) {
		while((sql_row = mysql_fetch_row(sql_res))) {	//start to fetch
			p->storage[i].id= atoi(sql_row[0]);
			p->storage[i].nameid= atoi(sql_row[1]);
			p->storage[i].amount= atoi(sql_row[2]);
			p->storage[i].equip= atoi(sql_row[3]);
			p->storage[i].identify= atoi(sql_row[4]);
			p->storage[i].refine= atoi(sql_row[5]);
			p->storage[i].attribute= atoi(sql_row[6]);
			p->storage[i].card[0]= atoi(sql_row[7]);
			p->storage[i].card[1]= atoi(sql_row[8]);
			p->storage[i].card[2]= atoi(sql_row[9]);
			p->storage[i].card[3]= atoi(sql_row[10]);
			p->storage_amount = ++i;
		}
		mysql_free_result(sql_res);
	}
	printf ("guild storage load complete from DB - id: %d (total: %d)\n", guild_id, p->storage_amount);
	return 0;
}

//---------------------------------------------------------
// storage data initialize
int inter_storage_sql_init(){
	
	//memory alloc
	printf("interserver storage memory initialize....(%d byte)\n",sizeof(struct storage));
	storage_pt=calloc(sizeof(struct storage), 1);
	guild_storage_pt=calloc(sizeof(struct guild_storage), 1);
	memset(storage_pt,0,sizeof(struct storage));
	memset(guild_storage_pt,0,sizeof(struct guild_storage));
	
	return 1;
}
// 倉庫データ削除
int inter_storage_delete(int account_id)
{
		sprintf(tmp_sql, "DELETE FROM `%s` WHERE `account_id`='%d'",storage_db, account_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `storage`)- %s\n", mysql_error(&mysql_handle) );
	}
	return 0;
}
int inter_guild_storage_delete(int guild_id)
{
	sprintf(tmp_sql, "DELETE FROM `%s` WHERE `guild_id`='%d'",guild_storage_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild_storage`)- %s\n", mysql_error(&mysql_handle) );
	}
	return 0;
}

//---------------------------------------------------------
// packet from map server

// recive packet about storage data
int mapif_load_storage(int fd,int account_id){
	//load from DB
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
	WFIFOW(fd,0)=0x3811;
	WFIFOL(fd,2)=account_id;
	WFIFOB(fd,6)=0;
	WFIFOSET(fd,7);
	return 0;
}

int mapif_load_guild_storage(int fd,int account_id,int guild_id)
{
	int guild_exist=0;
	WFIFOW(fd,0)=0x3818;
	
	// Check if guild exists, I may write a function for this later, coz I use it several times.
	//printf("- Check if guild %d exists\n",g->guild_id);
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (delete `guild`)- %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		guild_exist =  atoi (sql_row[0]);
		//printf("- Check if guild %d exists : %s\n",g->guild_id,((guild_exist==0)?"No":"Yes"));
	}
	mysql_free_result(sql_res) ; //resource free
	
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
	mapif_load_storage(fd,RFIFOL(fd,2));
	return 0;
}
// storage data recive and save
int mapif_parse_SaveStorage(int fd){
	int account_id=RFIFOL(fd,4);
	int len=RFIFOW(fd,2);
	
	if(sizeof(struct storage)!=len-8){
		printf("inter storage: data size error %d %d\n",sizeof(struct storage),len-8);
	}else{
		memcpy(&storage_pt[0],RFIFOP(fd,8),sizeof(struct storage));
		storage_tosql(account_id, storage_pt);
		mapif_save_storage_ack(fd,account_id);
	}
	return 0;
}

int mapif_parse_LoadGuildStorage(int fd)
{
	mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}

int mapif_parse_SaveGuildStorage(int fd)
{
	int guild_exist=0;
	int guild_id=RFIFOL(fd,8);
	int len=RFIFOW(fd,2);
	if(sizeof(struct guild_storage)!=len-12){
		printf("inter storage: data size error %d %d\n",sizeof(struct guild_storage),len-12);
	}
	else {
		// Check if guild exists, I may write a function for this later, coz I use it several times.
		//printf("- Check if guild %d exists\n",g->guild_id);
		sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id`='%d'",guild_db, guild_id);
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error (delete `guild`)- %s\n", mysql_error(&mysql_handle) );
		}
		sql_res = mysql_store_result(&mysql_handle) ;
		if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
			sql_row = mysql_fetch_row(sql_res);
			guild_exist =  atoi (sql_row[0]);
			//printf("- Check if guild %d exists : %s\n",g->guild_id,((guild_exist==0)?"No":"Yes"));
		}
		mysql_free_result(sql_res) ; //resource free
		
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

