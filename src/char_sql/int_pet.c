//
// original code from athena
// SQL conversion by Jioh L. Jung
//
#include "char.h"
#include "strlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct s_pet *pet_pt;
static int pet_newid = 100;


//---------------------------------------------------------
int inter_pet_tosql(int pet_id, struct s_pet *p) {
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	char t_name[100];
	
	printf("request save pet: %d.......\n",pet_id);
	
	jstrescapecpy(t_name, p->name);
	
	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;
	sprintf(tmp_sql,"SELECT * FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) 
		//row reside -> updating
		sprintf(tmp_sql, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%d'",
			pet_db, p->class, t_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate, p->pet_id);
	else //no row -> insert
		sprintf(tmp_sql,"INSERT INTO `%s` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`) VALUES ('%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			pet_db, pet_id, p->class, t_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate);
	mysql_free_result(sql_res) ; //resource free
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error (inset/update `pet`)- %s\n", mysql_error(&mysql_handle) );
	}
	
	printf("pet save success.......\n");
	return 0;
}

int inter_pet_fromsql(int pet_id, struct s_pet *p){
	
	printf("request load pet: %d.......\n",pet_id);
	
	memset(p, 0, sizeof(struct s_pet));
	
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	
	sprintf(tmp_sql,"SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate` FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error (select `pet`)- %s\n", mysql_error(&mysql_handle) );
			return 0;
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0) {
		sql_row = mysql_fetch_row(sql_res);
		
		p->pet_id = pet_id;
		p->class = atoi(sql_row[1]);
		memcpy(p->name, sql_row[2],24);
		p->account_id = atoi(sql_row[3]);
		p->char_id = atoi(sql_row[4]);
		p->level = atoi(sql_row[5]);
		p->egg_id = atoi(sql_row[6]);
		p->equip = atoi(sql_row[7]);
		p->intimate = atoi(sql_row[8]);
		p->hungry = atoi(sql_row[9]);
		p->rename_flag = atoi(sql_row[10]);
		p->incuvate = atoi(sql_row[11]);
	}
	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;
	
	mysql_free_result(sql_res);
	
	printf("pet load success.......\n");
	return 0;
}
//----------------------------------------------
	
int inter_pet_sql_init(){
	int i;
		
	//memory alloc
	printf("interserver pet memory initialize.... (%d byte)\n",sizeof(struct s_pet));
	pet_pt = calloc(sizeof(struct s_pet), 1);

	sprintf (tmp_sql , "SELECT count(*) FROM `%s`", pet_db);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		exit(0);
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);
	printf("total pet data -> '%s'.......\n",sql_row[0]);
	i = atoi (sql_row[0]);
	mysql_free_result(sql_res);

	if (i > 0) {
		//set pet_newid
		sprintf (tmp_sql , "SELECT max(`pet_id`) FROM `%s`",pet_db );
		if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
		}
		
		sql_res = mysql_store_result(&mysql_handle) ;
		
		sql_row = mysql_fetch_row(sql_res);
		pet_newid = atoi (sql_row[0])+1; //should SET MAX existing PET ID + 1 [Lupus]
		mysql_free_result(sql_res);
	}
	
	printf("set pet_newid: %d.......\n",pet_newid);
	
	return 0;
}
//----------------------------------
int inter_pet_delete(int pet_id){
	printf("request delete pet: %d.......\n",pet_id);
	
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `pet_id`='%d'",pet_db, pet_id);
	if(mysql_query(&mysql_handle, tmp_sql) ) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	return 0;
}
//------------------------------------------------------
int mapif_pet_created(int fd, int account_id, struct s_pet *p)
{
	WFIFOW(fd, 0) =0x3880;
	WFIFOL(fd, 2) =account_id;
	if(p!=NULL){
		WFIFOB(fd, 6)=0;
		WFIFOL(fd, 7) =p->pet_id;
		printf("int_pet: created! %d %s\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd, 6)=1;
		WFIFOL(fd, 7)=0;
	}
	WFIFOSET(fd, 11);

	return 0;
}

int mapif_pet_info(int fd, int account_id, struct s_pet *p){
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=0;
	memcpy(WFIFOP(fd, 9), p, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int mapif_pet_noinfo(int fd, int account_id){
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=1;
	memset(WFIFOP(fd, 9), 0, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int mapif_save_pet_ack(int fd, int account_id, int flag){
	WFIFOW(fd, 0) =0x3882;
	WFIFOL(fd, 2) =account_id;
	WFIFOB(fd, 6) =flag;
	WFIFOSET(fd, 7);

	return 0;
}

int mapif_delete_pet_ack(int fd, int flag){
	WFIFOW(fd, 0) =0x3883;
	WFIFOB(fd, 2) =flag;
	WFIFOSET(fd, 3);

	return 0;
}

int mapif_create_pet(int fd, int account_id, int char_id, short pet_class, short pet_lv, short pet_egg_id,
	short pet_equip, short intimate, short hungry, char rename_flag, char incuvate, char *pet_name){

	memset(pet_pt, 0, sizeof(struct s_pet));
	pet_pt->pet_id = pet_newid++;
	memcpy(pet_pt->name, pet_name, 24);
	if(incuvate == 1)
		pet_pt->account_id = pet_pt->char_id = 0;
	else {
		pet_pt->account_id = account_id;
		pet_pt->char_id = char_id;
	}
	pet_pt->class = pet_class;
	pet_pt->level = pet_lv;
	pet_pt->egg_id = pet_egg_id;
	pet_pt->equip = pet_equip;
	pet_pt->intimate = intimate;
	pet_pt->hungry = hungry;
	pet_pt->rename_flag = rename_flag;
	pet_pt->incuvate = incuvate;

	if(pet_pt->hungry < 0)
		pet_pt->hungry = 0;
	else if(pet_pt->hungry > 100)
		pet_pt->hungry = 100;
	if(pet_pt->intimate < 0)
		pet_pt->intimate = 0;
	else if(pet_pt->intimate > 1000)
		pet_pt->intimate = 1000;
	
	inter_pet_tosql(pet_pt->pet_id,pet_pt);
	
	mapif_pet_created(fd, account_id, pet_pt);
	
	return 0;
}

int mapif_load_pet(int fd, int account_id, int char_id, int pet_id){
	memset(pet_pt, 0, sizeof(struct s_pet));
	
	inter_pet_fromsql(pet_id, pet_pt);
	
	if(pet_pt!=NULL) {
		if(pet_pt->incuvate == 1) {
			pet_pt->account_id = pet_pt->char_id = 0;
			mapif_pet_info(fd, account_id, pet_pt);
		}
		else if(account_id == pet_pt->account_id && char_id == pet_pt->char_id)
			mapif_pet_info(fd, account_id, pet_pt);
		else
			mapif_pet_noinfo(fd, account_id);
	}
	else
		mapif_pet_noinfo(fd, account_id);

	return 0;
}

int mapif_save_pet(int fd, int account_id, struct s_pet *data) {
	//here process pet save request.
	int len=RFIFOW(fd, 2);
	if(sizeof(struct s_pet)!=len-8) {
		printf("inter pet: data size error %d %d\n", sizeof(struct s_pet), len-8);
	}
	
	else{
		if(data->hungry < 0)
			data->hungry = 0;
		else if(data->hungry > 100)
			data->hungry = 100;
		if(data->intimate < 0)
			data->intimate = 0;
		else if(data->intimate > 1000)
			data->intimate = 1000;
		inter_pet_tosql(data->pet_id,data);
		mapif_save_pet_ack(fd, account_id, 0);
	}

	return 0;
}

int mapif_delete_pet(int fd, int pet_id){
	mapif_delete_pet_ack(fd, inter_pet_delete(pet_id));

	return 0;
}

int mapif_parse_CreatePet(int fd){
	mapif_create_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOW(fd, 10), RFIFOW(fd, 12), RFIFOW(fd, 14), RFIFOW(fd, 16), RFIFOL(fd, 18),
		RFIFOL(fd, 20), RFIFOB(fd, 22), RFIFOB(fd, 23), RFIFOP(fd, 24));
	return 0;
}

int mapif_parse_LoadPet(int fd){
	mapif_load_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
	return 0;
}

int mapif_parse_SavePet(int fd){
	mapif_save_pet(fd, RFIFOL(fd, 4), (struct s_pet *) RFIFOP(fd, 8));
	return 0;
}

int mapif_parse_DeletePet(int fd){
	mapif_delete_pet(fd, RFIFOL(fd, 2));
	return 0;
}

int inter_pet_parse_frommap(int fd){
	switch(RFIFOW(fd, 0)){
	case 0x3080: mapif_parse_CreatePet(fd); break;
	case 0x3081: mapif_parse_LoadPet(fd); break;
	case 0x3082: mapif_parse_SavePet(fd); break;
	case 0x3083: mapif_parse_DeletePet(fd); break;
	default:
		return 0;
	}
	return 1;
}

