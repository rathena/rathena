// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_pet.hpp"

#include <cstdlib>
#include <cstring>

#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>
#include <common/utils.hpp>

#include "char.hpp"
#include "inter.hpp"

struct s_pet *pet_pt;
int32 mapif_load_pet(int32 fd, uint32 account_id, uint32 char_id, int32 pet_id);

//---------------------------------------------------------
int32 inter_pet_tosql(int32 pet_id, struct s_pet* p)
{
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`,`autofeed`)
	char esc_name[NAME_LENGTH*2+1];// escaped pet name

	Sql_EscapeStringLen(sql_handle, esc_name, p->name, strnlen(p->name, NAME_LENGTH));
	p->hungry = cap_value(p->hungry, 0, 100);
	p->intimate = cap_value(p->intimate, 0, 1000);

	if( pet_id == -1 )
	{// New pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` "
			"(`class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`,`autofeed`) "
			"VALUES ('%d', '%s', '%d', '%d', '%d', '%u', '%u', '%d', '%d', '%d', '%d', '%d')",
			schema_config.pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incubate, p->autofeed) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
		p->pet_id = (int32)Sql_LastInsertId(sql_handle);
	}
	else
	{// Update pet.
		if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%u',`equip`='%u',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incubate`='%d',`autofeed`='%d' WHERE `pet_id`='%d'",
			schema_config.pet_db, p->class_, esc_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incubate, p->autofeed, p->pet_id) )
		{
			Sql_ShowDebug(sql_handle);
			return 0;
		}
	}

	if (charserv_config.save_log)
		ShowInfo("Pet saved %d - %s.\n", pet_id, p->name);
	return 1;
}

int32 inter_pet_fromsql(int32 pet_id, struct s_pet* p)
{
	char* data;
	size_t len;

#ifdef NOISY
	ShowInfo("Loading pet (%d)...\n",pet_id);
#endif
	memset(p, 0, sizeof(struct s_pet));

	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`,`autofeed`)

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incubate`,`autofeed` FROM `%s` WHERE `pet_id`='%d'", schema_config.pet_db, pet_id) )
	{
		Sql_ShowDebug(sql_handle);
		return 0;
	}

	if( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		p->pet_id = pet_id;
		Sql_GetData(sql_handle,  1, &data, nullptr); p->class_ = atoi(data);
		Sql_GetData(sql_handle,  2, &data, &len); memcpy(p->name, data, zmin(len, NAME_LENGTH));
		Sql_GetData(sql_handle,  3, &data, nullptr); p->account_id = atoi(data);
		Sql_GetData(sql_handle,  4, &data, nullptr); p->char_id = atoi(data);
		Sql_GetData(sql_handle,  5, &data, nullptr); p->level = atoi(data);
		Sql_GetData(sql_handle,  6, &data, nullptr); p->egg_id = strtoul(data, nullptr, 10);
		Sql_GetData(sql_handle,  7, &data, nullptr); p->equip = strtoul(data, nullptr, 10);
		Sql_GetData(sql_handle,  8, &data, nullptr); p->intimate = atoi(data);
		Sql_GetData(sql_handle,  9, &data, nullptr); p->hungry = atoi(data);
		Sql_GetData(sql_handle, 10, &data, nullptr); p->rename_flag = atoi(data);
		Sql_GetData(sql_handle, 11, &data, nullptr); p->incubate = atoi(data);
		Sql_GetData(sql_handle, 12, &data, nullptr); p->autofeed = atoi(data) != 0;

		Sql_FreeResult(sql_handle);

		p->hungry = cap_value(p->hungry, 0, 100);
		p->intimate = cap_value(p->intimate, 0, 1000);

		if( charserv_config.save_log )
			ShowInfo("Pet loaded (%d - %s).\n", pet_id, p->name);
	}
	return 0;
}
//----------------------------------------------

int32 inter_pet_sql_init(void){
	//memory alloc
	pet_pt = (struct s_pet*)aCalloc(sizeof(struct s_pet), 1);
	return 0;
}
void inter_pet_sql_final(void){
	if (pet_pt) aFree(pet_pt);
	return;
}
//----------------------------------
int32 inter_pet_delete(int32 pet_id){
	ShowInfo("delete pet request: %d...\n",pet_id);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `pet_id`='%d'", schema_config.pet_db, pet_id) )
		Sql_ShowDebug(sql_handle);
	return 0;
}
//------------------------------------------------------
int32 mapif_pet_created(int32 fd, uint32 account_id, struct s_pet *p)
{
	WFIFOHEAD(fd, 12);
	WFIFOW(fd, 0) = 0x3880;
	WFIFOL(fd, 2) = account_id;
	if(p!=nullptr){
		WFIFOW(fd, 6) = p->class_;
		WFIFOL(fd, 8) = p->pet_id;
		ShowInfo("int_pet: created pet %d - %s\n", p->pet_id, p->name);
	}else{
		WFIFOB(fd, 6) = 0;
		WFIFOL(fd, 8) = 0;
	}
	WFIFOSET(fd, 12);

	return 0;
}

int32 mapif_pet_info(int32 fd, uint32 account_id, struct s_pet *p){
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=0;
	memcpy(WFIFOP(fd, 9), p, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int32 mapif_pet_noinfo(int32 fd, uint32 account_id){
	WFIFOHEAD(fd, sizeof(struct s_pet) + 9);
	WFIFOW(fd, 0) =0x3881;
	WFIFOW(fd, 2) =sizeof(struct s_pet) + 9;
	WFIFOL(fd, 4) =account_id;
	WFIFOB(fd, 8)=1;
	memset(WFIFOP(fd, 9), 0, sizeof(struct s_pet));
	WFIFOSET(fd, WFIFOW(fd, 2));

	return 0;
}

int32 mapif_save_pet_ack(int32 fd, uint32 account_id, int32 flag){
	WFIFOHEAD(fd, 7);
	WFIFOW(fd, 0) =0x3882;
	WFIFOL(fd, 2) =account_id;
	WFIFOB(fd, 6) =flag;
	WFIFOSET(fd, 7);

	return 0;
}

int32 mapif_delete_pet_ack(int32 fd, int32 flag){
	WFIFOHEAD(fd, 3);
	WFIFOW(fd, 0) =0x3883;
	WFIFOB(fd, 2) =flag;
	WFIFOSET(fd, 3);

	return 0;
}

int32 mapif_create_pet(int32 fd, uint32 account_id, uint32 char_id, int16 pet_class, int16 pet_lv, t_itemid pet_egg_id, t_itemid pet_equip, int16 intimate, int16 hungry, char rename_flag, char incubate, char *pet_name)
{
	memset(pet_pt, 0, sizeof(struct s_pet));
	safestrncpy(pet_pt->name, pet_name, NAME_LENGTH);
	if(incubate == 1)
		pet_pt->account_id = pet_pt->char_id = 0;
	else {
		pet_pt->account_id = account_id;
		pet_pt->char_id = char_id;
	}
	pet_pt->class_ = pet_class;
	pet_pt->level = pet_lv;
	pet_pt->egg_id = pet_egg_id;
	pet_pt->equip = pet_equip;
	pet_pt->intimate = intimate;
	pet_pt->hungry = hungry;
	pet_pt->rename_flag = rename_flag;
	pet_pt->incubate = incubate;
	pet_pt->autofeed = 0;

	if(pet_pt->hungry < 0)
		pet_pt->hungry = 0;
	else if(pet_pt->hungry > 100)
		pet_pt->hungry = 100;
	if(pet_pt->intimate < 0)
		pet_pt->intimate = 0;
	else if(pet_pt->intimate > 1000)
		pet_pt->intimate = 1000;

	pet_pt->pet_id = -1; //Signal NEW pet.
	if (inter_pet_tosql(pet_pt->pet_id,pet_pt)){
		if( pet_pt->incubate ){
			mapif_pet_created(fd, account_id, pet_pt);
		}else{
			mapif_load_pet(fd, account_id, char_id, pet_pt->pet_id);
		}
	}else	//Failed...
		mapif_pet_created(fd, account_id, nullptr);

	return 0;
}

int32 mapif_load_pet(int32 fd, uint32 account_id, uint32 char_id, int32 pet_id){
	memset(pet_pt, 0, sizeof(struct s_pet));

	inter_pet_fromsql(pet_id, pet_pt);

	if(pet_pt!=nullptr) {
		if(pet_pt->incubate == 1) {
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

int32 mapif_save_pet(int32 fd, uint32 account_id, struct s_pet *data) {
	//here process pet save request.
	int32 len;
	len=RFIFOW(fd, 2);
	if(sizeof(struct s_pet)!=len-8) {
		ShowError("inter pet: data size error %" PRIuPTR " %d\n", sizeof(struct s_pet), len-8);
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

int32 mapif_delete_pet(int32 fd, int32 pet_id){
	mapif_delete_pet_ack(fd, inter_pet_delete(pet_id));

	return 0;
}

int32 mapif_parse_CreatePet(int32 fd){
	mapif_create_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOW(fd, 10), RFIFOW(fd, 12), RFIFOL(fd, 14), RFIFOL(fd, 18), RFIFOW(fd, 22),
		RFIFOW(fd, 24), RFIFOB(fd, 26), RFIFOB(fd, 27), RFIFOCP(fd, 28));
	return 0;
}

int32 mapif_parse_LoadPet(int32 fd){
	mapif_load_pet(fd, RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
	return 0;
}

int32 mapif_parse_SavePet(int32 fd){
	mapif_save_pet(fd, RFIFOL(fd, 4), (struct s_pet *) RFIFOP(fd, 8));
	return 0;
}

int32 mapif_parse_DeletePet(int32 fd){
	mapif_delete_pet(fd, RFIFOL(fd, 2));
	return 0;
}

int32 inter_pet_parse_frommap(int32 fd){
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
