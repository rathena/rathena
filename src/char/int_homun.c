// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/socket.h"
#include "../common/db.h"
#include "../common/lock.h"
#include "../common/showmsg.h"
#include "char.h"
#include "inter.h"
#include "int_homun.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char homun_txt[1024]="save/homun.txt";

static DBMap* homun_db; // int hom_id -> struct s_homunculus*
static int homun_newid = 100;

int inter_homun_tostr(char *str,struct s_homunculus *p)
{
	int i;

	str+=sprintf(str,"%d,%d\t%s\t%d,%d,%d,%d,%d,"
		"%u,%d,%d,%d,"
		"%u,%d,%d,"
		"%d,%d,%d,%d,%d,%d\t",
		p->hom_id, p->class_, p->name,
		p->char_id, p->hp, p->max_hp, p->sp, p->max_sp,
	  	p->intimacy, p->hunger, p->skillpts, p->level,
		p->exp, p->rename_flag, p->vaporize,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk);

	for (i = 0; i < MAX_HOMUNSKILL; i++)
	{
		if (p->hskill[i].id && !p->hskill[i].flag)
			str+=sprintf(str,"%d,%d,", p->hskill[i].id, p->hskill[i].lv);
	}

	return 0;
}

int inter_homun_fromstr(char *str,struct s_homunculus *p)
{
	int i, next, len;
	int tmp_int[25];
	unsigned int tmp_uint[5];
	char tmp_str[256];

	memset(p,0,sizeof(struct s_homunculus));

	i=sscanf(str,"%d,%d\t%127[^\t]\t%d,%d,%d,%d,%d,"
		"%u,%d,%d,%d,"
		"%u,%d,%d,"
		"%d,%d,%d,%d,%d,%d\t%n",
		&tmp_int[0],&tmp_int[1],tmp_str,
		&tmp_int[2],&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],
		&tmp_uint[0],&tmp_int[7],&tmp_int[8],&tmp_int[9],
		&tmp_uint[1],&tmp_int[10],&tmp_int[11],
		&tmp_int[12],&tmp_int[13],&tmp_int[14],&tmp_int[15],&tmp_int[16],&tmp_int[17],
		&next);

	if(i!=21)
		return 1;

	p->hom_id = tmp_int[0];
	p->class_ = tmp_int[1];
	memcpy(p->name, tmp_str, NAME_LENGTH);

	p->char_id = tmp_int[2];
  	p->hp = tmp_int[3];
	p->max_hp = tmp_int[4];
	p->sp = tmp_int[5];
	p->max_sp = tmp_int[6];

	p->intimacy = tmp_uint[0];
	p->hunger = tmp_int[7];
	p->skillpts = tmp_int[8];
	p->level = tmp_int[9];

	p->exp = tmp_uint[1];
	p->rename_flag = tmp_int[10];
	p->vaporize = tmp_int[11];

	p->str = tmp_int[12];
	p->agi = tmp_int[13];
	p->vit = tmp_int[14];
	p->int_= tmp_int[15];
	p->dex = tmp_int[16];
	p->luk = tmp_int[17];

	//Read skills.
	while(str[next] && str[next] != '\n' && str[next] != '\r') {
		if (sscanf(str+next, "%d,%d,%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return 2;

		if (tmp_int[0] >= HM_SKILLBASE && tmp_int[0] < HM_SKILLBASE+MAX_HOMUNSKILL)
		{
			i = tmp_int[0] - HM_SKILLBASE;
			p->hskill[i].id = tmp_int[0];
			p->hskill[i].lv = tmp_int[1];
		} else
			ShowError("Read Homun: Unsupported Skill ID %d for homunculus (Homun ID=%d)\n", tmp_int[0], p->hom_id);
		next += len;
		if (str[next] == ' ')
			next++;
	}
	return 0;
}

int inter_homun_init()
{
	char line[8192];
	struct s_homunculus *p;
	FILE *fp;
	int c=0;

	homun_db= idb_alloc(DB_OPT_RELEASE_DATA);

	if( (fp=fopen(homun_txt,"r"))==NULL )
		return 1;
	while(fgets(line, sizeof(line), fp))
	{
		p = (struct s_homunculus*)aCalloc(sizeof(struct s_homunculus), 1);
		if(p==NULL){
			ShowFatalError("int_homun: out of memory!\n");
			exit(EXIT_FAILURE);
		}
		if(inter_homun_fromstr(line,p)==0 && p->hom_id>0){
			if( p->hom_id >= homun_newid)
				homun_newid=p->hom_id+1;
			idb_put(homun_db,p->hom_id,p);
		}else{
			ShowError("int_homun: broken data [%s] line %d\n",homun_txt,c);
			aFree(p);
		}
		c++;
	}
	fclose(fp);
	return 0;
}

void inter_homun_final()
{
	homun_db->destroy(homun_db, NULL);
	return;
}

int inter_homun_save_sub(DBKey key,void *data,va_list ap)
{
	char line[8192];
	FILE *fp;
	inter_homun_tostr(line,(struct s_homunculus *)data);
	fp=va_arg(ap,FILE *);
	fprintf(fp,"%s\n",line);
	return 0;
}

int inter_homun_save()
{
	FILE *fp;
	int lock;
	if( (fp=lock_fopen(homun_txt,&lock))==NULL ){
		ShowError("int_homun: can't write [%s] !!! data is lost !!!\n",homun_txt);
		return 1;
	}
	homun_db->foreach(homun_db,inter_homun_save_sub,fp);
	lock_fclose(fp,homun_txt,&lock);
	return 0;
}

int inter_homun_delete(int hom_id)
{
	struct s_homunculus *p;
	p = idb_get(homun_db,hom_id);
	if( p == NULL)
		return 0;
	idb_remove(homun_db,hom_id);
	ShowInfo("Deleted homun (hom_id: %d)\n",hom_id);
	return 1;
}

int mapif_homun_created(int fd,int account_id, struct s_homunculus *p)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd, 0) =0x3890;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8)= p->hom_id?1:0;
	memcpy(WFIFOP(fd,9), p, sizeof(struct s_homunculus));
	WFIFOSET(fd, WFIFOW(fd,2));
	return 0;
}

int mapif_homun_info(int fd,int account_id,struct s_homunculus *p)
{
	WFIFOHEAD(fd, sizeof(struct s_homunculus)+9);
	WFIFOW(fd,0) = 0x3891;
	WFIFOW(fd,2) = sizeof(struct s_homunculus)+9;
	WFIFOL(fd,4) = account_id;
	WFIFOB(fd,8) = 1; // account loaded with success

	memcpy(WFIFOP(fd,9), p, sizeof(struct s_homunculus));
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

int mapif_homun_noinfo(int fd,int account_id)
{
	WFIFOHEAD(fd,sizeof(struct s_homunculus) + 9);
	WFIFOW(fd,0)=0x3891;
	WFIFOW(fd,2)=sizeof(struct s_homunculus) + 9;
	WFIFOL(fd,4)=account_id;
	WFIFOB(fd,8)=0;
	memset(WFIFOP(fd,9),0,sizeof(struct s_homunculus));
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int mapif_save_homun_ack(int fd,int account_id,int flag)
{
	WFIFOHEAD(fd, 7);
	WFIFOW(fd,0)=0x3892;
	WFIFOL(fd,2)=account_id;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,7);
	return 0;
}

int mapif_delete_homun_ack(int fd,int flag)
{
	WFIFOHEAD(fd, 3);
	WFIFOW(fd,0)=0x3893;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,3);
	return 0;
}

int mapif_rename_homun_ack(int fd, int account_id, int char_id, int flag, char *name){
	WFIFOHEAD(fd, NAME_LENGTH+12);
	WFIFOW(fd, 0) =0x3894;
	WFIFOL(fd, 2) =account_id;
	WFIFOL(fd, 6) =char_id;
	WFIFOB(fd, 10) =flag;
	memcpy(WFIFOP(fd, 11), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+12);

	return 0;
}

int mapif_create_homun(int fd)
{
	struct s_homunculus *p;
	RFIFOHEAD(fd);
	p= (struct s_homunculus *) aCalloc(sizeof(struct s_homunculus), 1);
	if(p==NULL){
		ShowFatalError("int_homun: out of memory !\n");
		//Sending the received data will pass hom_id == 0 <- fail.
		mapif_homun_created(fd,RFIFOL(fd,4),(struct s_homunculus*)RFIFOP(fd,8));
		return 0;
	}
	memcpy(p, RFIFOP(fd,8), sizeof(struct s_homunculus));
	p->hom_id = homun_newid++; //New ID
	idb_put(homun_db,p->hom_id,p);
	mapif_homun_created(fd,RFIFOL(fd,4),p);
	return 0;
}

int mapif_load_homun(int fd)
{
	struct s_homunculus *p;
	int account_id;
	RFIFOHEAD(fd);
	account_id = RFIFOL(fd,2);

	p= idb_get(homun_db,RFIFOL(fd,6));
	if(p==NULL) {
		mapif_homun_noinfo(fd,account_id);
		return 0;
	}
	mapif_homun_info(fd,account_id,p);
	return 0;
}

static void* create_homun(DBKey key, va_list args) {
	struct s_homunculus *p;
	p=(struct s_homunculus *)aCalloc(sizeof(struct s_homunculus),1);
	p->hom_id = key.i;
	return p;
}
int mapif_save_homun(int fd,int account_id,struct s_homunculus *data)
{
	struct s_homunculus *p;
	int hom_id;
	
	if (data->hom_id == 0)
		data->hom_id = homun_newid++;
	hom_id = data->hom_id;
	p= idb_ensure(homun_db,hom_id,create_homun);
	memcpy(p,data,sizeof(struct s_homunculus));
	mapif_save_homun_ack(fd,account_id,1);
	return 0;
}

int mapif_delete_homun(int fd,int hom_id)
{
	mapif_delete_homun_ack(fd,inter_homun_delete(hom_id));
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

int mapif_parse_SaveHomun(int fd)
{
	RFIFOHEAD(fd);
	mapif_save_homun(fd,RFIFOL(fd,4),(struct s_homunculus *)RFIFOP(fd,8));
	return 0;
}

int mapif_parse_DeleteHomun(int fd)
{
	RFIFOHEAD(fd);
	mapif_delete_homun(fd,RFIFOL(fd,2));
	return 0;
}

int mapif_parse_RenameHomun(int fd){
	RFIFOHEAD(fd);
	mapif_rename_homun(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOP(fd, 10));
	return 0;
}

int inter_homun_parse_frommap(int fd)
{
	RFIFOHEAD(fd);
	switch(RFIFOW(fd,0)){
	case 0x3090: mapif_create_homun(fd); break;
	case 0x3091: mapif_load_homun(fd); break;
	case 0x3092: mapif_parse_SaveHomun(fd); break;
	case 0x3093: mapif_parse_DeleteHomun(fd); break;
	case 0x3094: mapif_parse_RenameHomun(fd); break;
	default:
		return 0;
	}
	return 1;
}
