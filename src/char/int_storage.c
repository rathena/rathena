// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "../common/utils.h"
#include "char.h"
#include "inter.h"
#include "int_storage.h"
#include "int_pet.h"
#include "int_guild.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ファイル名のデフォルト
// inter_config_read()で再設定される
char storage_txt[1024]="save/storage.txt";
char guild_storage_txt[1024]="save/g_storage.txt";

static DBMap* storage_db = NULL; // int account_id -> struct storage_data*
static DBMap* guild_storage_db = NULL; // int guild_id -> struct guild_storage*

// 倉庫データを文字列に変換
bool storage_tostr(char* str, int account_id, struct storage_data* p)
{
	int i,j;
	char *str_p = str;
	str_p += snprintf(str_p, sizeof str_p, "%d,%d\t", account_id, p->storage_amount);

	for( i = 0; i < MAX_STORAGE; i++ )
		if( p->items[i].nameid > 0 && p->items[i].amount > 0 )
		{
			str_p += snprintf(str_p, sizeof str_p, "%d,%d,%d,%d,%d,%d,%d",
				p->items[i].id,p->items[i].nameid,p->items[i].amount,p->items[i].equip,
				p->items[i].identify,p->items[i].refine,p->items[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += snprintf(str_p,sizeof str_p,",%d",p->items[i].card[j]);
			str_p += snprintf(str_p,sizeof str_p," ");
		}

	*(str_p++)='\t';

	*str_p='\0';

	return true;
}

// 文字列を倉庫データに変換
bool storage_fromstr(char* str, int* account_id, struct storage_data* p)
{
	int tmp_int[256];
	char tmp_str[256];
	int next,len,i,j;

	if( sscanf(str, "%d,%d%n", &tmp_int[0], &tmp_int[1], &next) != 2 )
		return false;

	*account_id = tmp_int[0];
	p->storage_amount = tmp_int[1]; //FIXME: limit to MAX_STORAGE?

	next++;
	for( i = 0; str[next] && str[next]!='\t' && i < MAX_STORAGE; i++ )
	{
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &len) != 8)
			  return false;

		p->items[i].id = tmp_int[0];
		p->items[i].nameid = tmp_int[1];
		p->items[i].amount = tmp_int[2];
		p->items[i].equip = tmp_int[3];
		p->items[i].identify = tmp_int[4];
		p->items[i].refine = tmp_int[5];
		p->items[i].attribute = tmp_int[6];
			
		for(j = 0; j < MAX_SLOTS && tmp_str[0] && sscanf(tmp_str, ",%d%[0-9,-]",&tmp_int[0], tmp_str) > 0; j++)
			p->items[i].card[j] = tmp_int[0];
			
		next += len;
		if (str[next] == ' ')
			next++;
	}

	if( i >= MAX_STORAGE && str[next] && str[next] != '\t' )
		ShowWarning("storage_fromstr: Found a storage line with more items than MAX_STORAGE (%d), remaining items have been discarded!\n", MAX_STORAGE);

	return true;
}

int guild_storage_tostr(char *str,struct guild_storage *p)
{
	int i,j,f=0;
	char *str_p = str;
	str_p+=snprintf(str,sizeof str,"%d,%d\t",p->guild_id,p->storage_amount);

	for(i=0;i<MAX_GUILD_STORAGE;i++)
		if( (p->storage_[i].nameid) && (p->storage_[i].amount) ){
			str_p += snprintf(str_p,sizeof str_p,"%d,%d,%d,%d,%d,%d,%d",
				p->storage_[i].id,p->storage_[i].nameid,p->storage_[i].amount,p->storage_[i].equip,
				p->storage_[i].identify,p->storage_[i].refine,p->storage_[i].attribute);
			for(j=0; j<MAX_SLOTS; j++)
				str_p += snprintf(str_p,sizeof str_p,",%d",p->storage_[i].card[j]);
			str_p += snprintf(str_p,sizeof str_p," ");
			f++;
		}

	*(str_p++)='\t';

	*str_p='\0';
	if(!f)
		str[0]=0;
	return 0;
}

int guild_storage_fromstr(char *str,struct guild_storage *p)
{
	int tmp_int[256];
	char tmp_str[256];
	int set,next,len,i,j;

	set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
	p->storage_amount=tmp_int[1];

	if(set!=2)
		return 1;
	if(str[next]=='\n' || str[next]=='\r')
		return 0;
	next++;
	for(i=0;str[next] && str[next]!='\t' && i < MAX_GUILD_STORAGE;i++){
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d%[0-9,-]%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
			&tmp_int[4], &tmp_int[5], &tmp_int[6], tmp_str, &len) == 8)
		{
			p->storage_[i].id = tmp_int[0];
			p->storage_[i].nameid = tmp_int[1];
			p->storage_[i].amount = tmp_int[2];
			p->storage_[i].equip = tmp_int[3];
			p->storage_[i].identify = tmp_int[4];
			p->storage_[i].refine = tmp_int[5];
			p->storage_[i].attribute = tmp_int[6];
			for(j = 0; j < MAX_SLOTS && tmp_str[0] && sscanf(tmp_str, ",%d%[0-9,-]",&tmp_int[0], tmp_str) > 0; j++)
				p->storage_[i].card[j] = tmp_int[0];
			next += len;
			if (str[next] == ' ')
				next++;
		}
		else return 1;
	}
	if (i >= MAX_GUILD_STORAGE && str[next] && str[next]!='\t')
		ShowWarning("guild_storage_fromstr: Found a storage line with more items than MAX_GUILD_STORAGE (%d), remaining items have been discarded!\n", MAX_GUILD_STORAGE);
	return 0;
}

#ifndef TXT_SQL_CONVERT

static void* create_storage(DBKey key, va_list args)
{
	return (struct storage_data *) aCalloc(sizeof(struct storage_data), 1);
}

static void* create_guildstorage(DBKey key, va_list args)
{
	struct guild_storage* gs = NULL;
	gs = (struct guild_storage *) aCalloc(sizeof(struct guild_storage), 1);
	gs->guild_id=key.i;
	return gs;
}

/// Loads storage data into the provided data structure.
/// If data doesn't exist, the destination is zeroed and false is returned.
bool storage_load(int account_id, struct storage_data* storage)
{
	struct storage_data* s = (struct storage_data*)idb_get(storage_db, account_id);

	if( s != NULL )
		memcpy(storage, s, sizeof(*storage));
	else
		memset(storage, 0x00, sizeof(*storage));

	return( s != NULL );
}

/// Writes provided data into storage cache.
/// If data contains 0 items, any existing entry in cache is destroyed.
/// If data contains 1+ items and no cache entry exists, a new one is created.
bool storage_save(int account_id, struct storage_data* storage)
{
	if( storage->storage_amount > 0 )
	{
		struct storage_data* s = (struct storage_data*)idb_ensure(storage_db, account_id, create_storage);
		memcpy(s, storage, sizeof(*storage));
	}
	else
	{
		idb_remove(storage_db, account_id);
	}

	return true;
}

//---------------------------------------------------------
// 倉庫データを読み込む
int inter_storage_init()
{
	char line[65536];
	int c = 0;
	FILE *fp;

	storage_db = idb_alloc(DB_OPT_RELEASE_DATA);

	fp=fopen(storage_txt,"r");
	if(fp==NULL){
		ShowError("can't read : %s\n",storage_txt);
		return 1;
	}
	while( fgets(line, sizeof(line), fp) )
	{
		int account_id;
		struct storage_data *s;

		s = (struct storage_data*)aCalloc(sizeof(struct storage_data), 1);
		if( s == NULL )
		{
			ShowFatalError("int_storage: out of memory!\n");
			exit(EXIT_FAILURE);
		}

		if( storage_fromstr(line,&account_id,s) )
		{
			idb_put(storage_db,account_id,s);
		}
		else{
			ShowError("int_storage: broken data in [%s] line %d\n",storage_txt,c);
			aFree(s);
		}
		c++;
	}
	fclose(fp);

	c = 0;
	guild_storage_db = idb_alloc(DB_OPT_RELEASE_DATA);

	fp=fopen(guild_storage_txt,"r");
	if(fp==NULL){
		ShowError("can't read : %s\n",guild_storage_txt);
		return 1;
	}
	while(fgets(line, sizeof(line), fp))
	{
		int tmp_int;
		struct guild_storage *gs;

		sscanf(line,"%d",&tmp_int);
		gs = (struct guild_storage*)aCalloc(sizeof(struct guild_storage), 1);
		if(gs==NULL){
			ShowFatalError("int_storage: out of memory!\n");
			exit(EXIT_FAILURE);
		}
//		memset(gs,0,sizeof(struct guild_storage)); aCalloc...
		gs->guild_id=tmp_int;
		if(gs->guild_id > 0 && guild_storage_fromstr(line,gs) == 0) {
			idb_put(guild_storage_db,gs->guild_id,gs);
		}
		else{
			ShowError("int_storage: broken data [%s] line %d\n",guild_storage_txt,c);
			aFree(gs);
		}
		c++;
	}
	fclose(fp);

	return 0;
}

void inter_storage_final() {
	if(storage_db)
	{
		storage_db->destroy(storage_db, NULL);
	}
	if(guild_storage_db)
	{
		guild_storage_db->destroy(guild_storage_db, NULL);
	}
	return;
}

//---------------------------------------------------------
// 倉庫データを書き込む
int inter_storage_save()
{
	struct DBIterator* iter;
	DBKey key;
	struct storage_data* data;
	FILE *fp;
	int lock;
	if( (fp=lock_fopen(storage_txt,&lock))==NULL ){
		ShowError("int_storage: can't write [%s] !!! data is lost !!!\n",storage_txt);
		return 1;
	}

	iter = storage_db->iterator(storage_db);
	for( data = (struct storage_data*)iter->first(iter,&key); iter->exists(iter); data = (struct storage_data*)iter->next(iter,&key) )
	{
		int account_id = key.i;
		char line[65536];
		storage_tostr(line,account_id,data);
		fprintf(fp,"%s\n",line);
 	}
	iter->destroy(iter);

	lock_fclose(fp,storage_txt,&lock);
	return 0;
}

//---------------------------------------------------------
// 倉庫データを書き込む
int inter_guild_storage_save()
{
	struct DBIterator* iter;
	struct guild_storage* data;
	FILE *fp;
	int  lock;
	if( (fp=lock_fopen(guild_storage_txt,&lock))==NULL ){
		ShowError("int_storage: can't write [%s] !!! data is lost !!!\n",guild_storage_txt);
		return 1;
	}

	iter = guild_storage_db->iterator(guild_storage_db);
	for( data = (struct guild_storage*)iter->first(iter,NULL); iter->exists(iter); data = (struct guild_storage*)iter->next(iter,NULL) )
	{
		char line[65536];
		if(inter_guild_search(data->guild_id) != NULL)
		{
			guild_storage_tostr(line,data);
			if(*line)
				fprintf(fp,"%s\n",line);
		}
	}
	iter->destroy(iter);

	lock_fclose(fp,guild_storage_txt,&lock);
	return 0;
}

// 倉庫データ削除
int inter_storage_delete(int account_id)
{
	struct storage_data *s = (struct storage_data*)idb_get(storage_db,account_id);
	if(s) {
		int i;
		for(i=0;i<s->storage_amount;i++){
			if(s->items[i].card[0] == (short)0xff00)
				inter_pet_delete( MakeDWord(s->items[i].card[1],s->items[i].card[2]) );
		}
		idb_remove(storage_db,account_id);
	}
	return 0;
}

// ギルド倉庫データ削除
int inter_guild_storage_delete(int guild_id)
{
	struct guild_storage *gs = (struct guild_storage*)idb_get(guild_storage_db,guild_id);
	if(gs) {
		int i;
		for(i=0;i<gs->storage_amount;i++){
			if(gs->storage_[i].card[0] == (short)0xff00)
				inter_pet_delete( MakeDWord(gs->storage_[i].card[1],gs->storage_[i].card[2]) );
		}
		idb_remove(guild_storage_db,guild_id);
	}
	return 0;
}

struct guild_storage *guild2storage(int guild_id)
{
	struct guild_storage* gs = NULL;
	if(inter_guild_search(guild_id) != NULL)
		gs = (struct guild_storage*)idb_ensure(guild_storage_db, guild_id, create_guildstorage);
	return gs;
}

//---------------------------------------------------------
// map serverへの通信

int mapif_load_guild_storage(int fd,int account_id,int guild_id)
{
	struct guild_storage *gs=guild2storage(guild_id);
	WFIFOHEAD(fd, sizeof(struct guild_storage)+12);
	WFIFOW(fd,0)=0x3818;
	if(gs) {
		WFIFOW(fd,2)=sizeof(struct guild_storage)+12;
		WFIFOL(fd,4)=account_id;
		WFIFOL(fd,8)=guild_id;
		memcpy(WFIFOP(fd,12),gs,sizeof(struct guild_storage));
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
// map serverからの通信

int mapif_parse_LoadGuildStorage(int fd)
{
	RFIFOHEAD(fd);
	mapif_load_guild_storage(fd,RFIFOL(fd,2),RFIFOL(fd,6));
	return 0;
}

int mapif_parse_SaveGuildStorage(int fd)
{
	struct guild_storage *gs;
	int guild_id, len;
	RFIFOHEAD(fd);
	guild_id=RFIFOL(fd,8);
	len=RFIFOW(fd,2);
	if(sizeof(struct guild_storage)!=len-12){
		ShowError("inter storage: data size error %d %d\n",sizeof(struct guild_storage),len-12);
	}
	else {
		gs=guild2storage(guild_id);
		if(gs) {
			memcpy(gs,RFIFOP(fd,12),sizeof(struct guild_storage));
			mapif_save_guild_storage_ack(fd,RFIFOL(fd,4),guild_id,0);
		}
		else
			mapif_save_guild_storage_ack(fd,RFIFOL(fd,4),guild_id,1);
	}
	return 0;
}

// map server からの通信
// ・１パケットのみ解析すること
// ・パケット長データはinter.cにセットしておくこと
// ・パケット長チェックや、RFIFOSKIPは呼び出し元で行われるので行ってはならない
// ・エラーなら0(false)、そうでないなら1(true)をかえさなければならない
int inter_storage_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)){
	case 0x3018: mapif_parse_LoadGuildStorage(fd); break;
	case 0x3019: mapif_parse_SaveGuildStorage(fd); break;
	default:
		return 0;
	}
	return 1;
}
#endif //TXT_SQL_CONVERT
