// (c) eAthena Dev Team - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/core.h"
#include "../common/strlib.h"
#include "../common/mmo.h"
#include "../common/showmsg.h"
#include "../common/mapindex.h"

#include <mysql.h>

//Txt Files used.
char char_txt[1024]="save/athena.txt";
char friends_txt[1024]="save/friends.txt"; // davidsiaw
char pet_txt[256]="save/pet.txt";
char storage_txt[256]="save/storage.txt";

//Databases used.
char char_db[256] = "char";
char cart_db[256] = "cart_inventory";
char inventory_db[256] = "inventory";
char storage_db[256] = "storage";
char reg_db[256] = "global_reg_value";
char skill_db[256] = "skill";
char memo_db[256] = "memo";
char pet_db[256] = "pet";
char friend_db[256] = "friends";
char guild_storage_db[256] = "guild_storage";

MYSQL mysql_handle;
MYSQL_RES* 	sql_res ;
MYSQL_ROW	sql_row ;
char tmp_sql[65535];

int sql_fields, sql_cnt;

int db_server_port = 3306;
char db_server_ip[16] = "127.0.0.1";
char db_server_id[32] = "ragnarok";
char db_server_pw[32] = "ragnarok";
char db_server_logindb[32] = "ragnarok";

char t_name[256];

int char_id_count=100000;

struct accreg {
	int reg_num;
	struct global_reg reg[ACCOUNT_REG_NUM];
};

struct character_data {
	struct mmo_charstatus status;
	struct accreg global;
} *char_dat;
int char_num, char_max;

//Required for sql saving, taken from src/char_sql/char.h
struct itemtmp {
	int flag;//checked = 1 else 0
	int id;
	short nameid;
	short amount;
	unsigned short equip;
	char identify;
	char refine;
	char attribute;
	short card[4];
};
enum {
	TABLE_INVENTORY,
	TABLE_CART,
	TABLE_STORAGE,
	TABLE_GUILD_STORAGE,
};

#define CHAR_CONF_NAME "conf/char_athena.conf"
#define INTER_CONF_NAME "conf/inter_athena.conf"
//==========================================================================================================
//NOTE: Update this function from the one in src/char/char.c as needed. [Skotlex]
int mmo_char_fromstr(char *str, struct mmo_charstatus *p, struct accreg *reg) {
	char tmp_str[3][128]; //To avoid deleting chars with too long names.
	int tmp_int[256];
	int set, next, len, i;

	// initilialise character
	memset(p, '\0', sizeof(struct mmo_charstatus));
	
	// If it's not char structure of version 1488 and after
	if ((set = sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
		"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
		"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d,%d%n",
		&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0],
		&tmp_int[3], &tmp_int[4], &tmp_int[5],
		&tmp_int[6], &tmp_int[7], &tmp_int[8],
		&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
		&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
		&tmp_int[19], &tmp_int[20],
		&tmp_int[21], &tmp_int[22], &tmp_int[23], //
		&tmp_int[24], &tmp_int[25], &tmp_int[26],
		&tmp_int[27], &tmp_int[28], &tmp_int[29],
		&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
		tmp_str[1], &tmp_int[35], &tmp_int[36],
		tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
		&tmp_int[40], &tmp_int[41], &tmp_int[42], &tmp_int[43], &next)) != 47)
	{
		tmp_int[43] = 0;	
		// If it's not char structure of version 1363 and after
		if ((set = sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
			"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
			"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d,%d,%d,%d%n",
			&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
			&tmp_int[3], &tmp_int[4], &tmp_int[5],
			&tmp_int[6], &tmp_int[7], &tmp_int[8],
			&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
			&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
			&tmp_int[19], &tmp_int[20],
			&tmp_int[21], &tmp_int[22], &tmp_int[23], //
			&tmp_int[24], &tmp_int[25], &tmp_int[26],
			&tmp_int[27], &tmp_int[28], &tmp_int[29],
			&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
			tmp_str[1], &tmp_int[35], &tmp_int[36], //
			tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], 
			&tmp_int[40], &tmp_int[41], &tmp_int[42], &next)) != 46)
		{
			tmp_int[40] = 0; // father
			tmp_int[41] = 0; // mother
			tmp_int[42] = 0; // child
			// If it's not char structure of version 1008 and before 1363
			if ((set = sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
				"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
				"\t%127[^,],%d,%d\t%127[^,],%d,%d,%d%n",
				&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
				&tmp_int[3], &tmp_int[4], &tmp_int[5],
				&tmp_int[6], &tmp_int[7], &tmp_int[8],
				&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
				&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
				&tmp_int[19], &tmp_int[20],
				&tmp_int[21], &tmp_int[22], &tmp_int[23], //
				&tmp_int[24], &tmp_int[25], &tmp_int[26],
				&tmp_int[27], &tmp_int[28], &tmp_int[29],
				&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
				tmp_str[1], &tmp_int[35], &tmp_int[36], //
				tmp_str[2], &tmp_int[37], &tmp_int[38], &tmp_int[39], &next)) != 43)
			{
				tmp_int[39] = 0; // partner id
				// If not char structure from version 384 to 1007
				if ((set = sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
					"\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
					"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
					&tmp_int[3], &tmp_int[4], &tmp_int[5],
					&tmp_int[6], &tmp_int[7], &tmp_int[8],
					&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
					&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
					&tmp_int[19], &tmp_int[20],
					&tmp_int[21], &tmp_int[22], &tmp_int[23], //
					&tmp_int[24], &tmp_int[25], &tmp_int[26],
					&tmp_int[27], &tmp_int[28], &tmp_int[29],
					&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
					tmp_str[1], &tmp_int[35], &tmp_int[36], //
					tmp_str[2], &tmp_int[37], &tmp_int[38], &next)) != 42)
				{
					// It's char structure of a version before 384
					tmp_int[26] = 0; // pet id
					set = sscanf(str, "%d\t%d,%d\t%127[^\t]\t%d,%d,%d\t%d,%d,%d\t%d,%d,%d,%d\t%d,%d,%d,%d,%d,%d\t%d,%d"
					"\t%d,%d,%d\t%d,%d\t%d,%d,%d\t%d,%d,%d,%d,%d"
					"\t%127[^,],%d,%d\t%127[^,],%d,%d%n",
					&tmp_int[0], &tmp_int[1], &tmp_int[2], tmp_str[0], //
					&tmp_int[3], &tmp_int[4], &tmp_int[5],
					&tmp_int[6], &tmp_int[7], &tmp_int[8],
					&tmp_int[9], &tmp_int[10], &tmp_int[11], &tmp_int[12],
					&tmp_int[13], &tmp_int[14], &tmp_int[15], &tmp_int[16], &tmp_int[17], &tmp_int[18],
					&tmp_int[19], &tmp_int[20],
					&tmp_int[21], &tmp_int[22], &tmp_int[23], //
					&tmp_int[24], &tmp_int[25], //
					&tmp_int[27], &tmp_int[28], &tmp_int[29],
					&tmp_int[30], &tmp_int[31], &tmp_int[32], &tmp_int[33], &tmp_int[34],
					tmp_str[1], &tmp_int[35], &tmp_int[36], //
					tmp_str[2], &tmp_int[37], &tmp_int[38], &next);
					set += 2;
					//printf("char: old char data ver.1\n");
				// Char structure of version 1007 or older
				} else {
					set++;
					//printf("char: old char data ver.2\n");
				}
			// Char structure of version 1008+
			} else {
				set += 3;
				//printf("char: new char data ver.3\n");
			}
		// Char structture of version 1363+
		} else {
			set++;
			//printf("char: new char data ver.4\n");
		}
	// Char structure of version 1488+
	} else {
		//printf("char: new char data ver.5\n");
	}
	if (set != 47)
		return 0;

	memcpy(p->name, tmp_str[0], NAME_LENGTH-1); //Overflow protection [Skotlex]
	p->char_id = tmp_int[0];
	p->account_id = tmp_int[1];
	p->char_num = tmp_int[2];
	p->class_ = tmp_int[3];
	p->base_level = tmp_int[4];
	p->job_level = tmp_int[5];
	p->base_exp = tmp_int[6];
	p->job_exp = tmp_int[7];
	p->zeny = tmp_int[8];
	p->hp = tmp_int[9];
	p->max_hp = tmp_int[10];
	p->sp = tmp_int[11];
	p->max_sp = tmp_int[12];
	p->str = tmp_int[13];
	p->agi = tmp_int[14];
	p->vit = tmp_int[15];
	p->int_ = tmp_int[16];
	p->dex = tmp_int[17];
	p->luk = tmp_int[18];
	p->status_point = tmp_int[19];
	p->skill_point = tmp_int[20];
	p->option = tmp_int[21];
	p->karma = tmp_int[22];
	p->manner = tmp_int[23];
	p->party_id = tmp_int[24];
	p->guild_id = tmp_int[25];
	p->pet_id = tmp_int[26];
	p->hair = tmp_int[27];
	p->hair_color = tmp_int[28];
	p->clothes_color = tmp_int[29];
	p->weapon = tmp_int[30];
	p->shield = tmp_int[31];
	p->head_top = tmp_int[32];
	p->head_mid = tmp_int[33];
	p->head_bottom = tmp_int[34];
	p->last_point.map = mapindex_name2id(tmp_str[1]);
	p->last_point.x = tmp_int[35];
	p->last_point.y = tmp_int[36];
	p->save_point.map = mapindex_name2id(tmp_str[2]);
	p->save_point.x = tmp_int[37];
	p->save_point.y = tmp_int[38];
	p->partner_id = tmp_int[39];
	p->father = tmp_int[40];
	p->mother = tmp_int[41];
	p->child = tmp_int[42];
	p->fame = tmp_int[43];

	// Some checks
	for(i = 0; i < char_num; i++) {
		if (char_dat[i].status.char_id == p->char_id) {
			ShowError("\033[1;31mmmo_auth_init: a character has an identical id to another.\n");
			ShowError("               character id #%d -> new character not readed.\n", p->char_id);
			ShowError("               Character saved in log file.\033[0m\n");
			return -1;
		} else if (strcmp(char_dat[i].status.name, p->name) == 0) {
			ShowError("\033[1;31mmmo_auth_init: a character name already exists.\n");
			ShowError("               character name '%s' -> new character not read.\n", p->name);
			ShowError("               Character saved in log file.\033[0m\n");
			return -2;
		}
	}

	if (str[next] == '\n' || str[next] == '\r')
		return 1;	// VKf[^

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str+next, "%[^,],%d,%d%n", tmp_str[0], &tmp_int[0], &tmp_int[1], &len) != 3)
			return -3;
		p->memo_point[i].map = mapindex_name2id(tmp_str[0]);
		p->memo_point[i].x = tmp_int[0];
		p->memo_point[i].y = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6],
		    &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12) {
			// do nothing, it's ok
		} else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		          &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		          &tmp_int[4], &tmp_int[5], &tmp_int[6],
		          &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
		} else // invalid structure
			return -4;
		p->inventory[i].id = tmp_int[0];
		p->inventory[i].nameid = tmp_int[1];
		p->inventory[i].amount = tmp_int[2];
		p->inventory[i].equip = tmp_int[3];
		p->inventory[i].identify = tmp_int[4];
		p->inventory[i].refine = tmp_int[5];
		p->inventory[i].attribute = tmp_int[6];
		p->inventory[i].card[0] = tmp_int[7];
		p->inventory[i].card[1] = tmp_int[8];
		p->inventory[i].card[2] = tmp_int[9];
		p->inventory[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		    &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		    &tmp_int[4], &tmp_int[5], &tmp_int[6],
		    &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12) {
			// do nothing, it's ok
		} else if (sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		           &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		           &tmp_int[4], &tmp_int[5], &tmp_int[6],
		           &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
		} else // invalid structure
			return -5;
		p->cart[i].id = tmp_int[0];
		p->cart[i].nameid = tmp_int[1];
		p->cart[i].amount = tmp_int[2];
		p->cart[i].equip = tmp_int[3];
		p->cart[i].identify = tmp_int[4];
		p->cart[i].refine = tmp_int[5];
		p->cart[i].attribute = tmp_int[6];
		p->cart[i].card[0] = tmp_int[7];
		p->cart[i].card[1] = tmp_int[8];
		p->cart[i].card[2] = tmp_int[9];
		p->cart[i].card[3] = tmp_int[10];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t'; i++) {
		if (sscanf(str + next, "%d,%d%n", &tmp_int[0], &tmp_int[1], &len) != 2)
			return -6;
		p->skill[tmp_int[0]].id = tmp_int[0];
		p->skill[tmp_int[0]].lv = tmp_int[1];
		next += len;
		if (str[next] == ' ')
			next++;
	}

	next++;

	for(i = 0; str[next] && str[next] != '\t' && str[next] != '\n' && str[next] != '\r'; i++) { // global_regÀÈOÌathena.txtÝ·Ì½ßê'\n'`FbN
		if (sscanf(str + next, "%[^,],%s%n", reg->reg[i].str, reg->reg[i].value, &len) != 2) {
			// because some scripts are not correct, the str can be "". So, we must check that.
			// If it's, we must not refuse the character, but just this REG value.
			// Character line will have something like: nov_2nd_cos,9 ,9 nov_1_2_cos_c,1 (here, ,9 is not good)
			if (str[next] == ',' && sscanf(str + next, ",%s%n", reg->reg[i].value, &len) == 1)
				i--;
			else
				return -7;
		}
		next += len;
		if (str[next] == ' ')
			next++;
	}
	reg->reg_num = i;

	return 1;
}
/* Because of the friend database update, it is no longer possible to convert friends.
//Note: Keep updated with the same function found in src/char/char.c [Skotlex]
int parse_friend_txt(struct mmo_charstatus *p)
{
	char line[1024];
	int i,cid=0,temp[20];
	FILE *fp;

	// Open the file and look for the ID
	fp = fopen(friends_txt, "r");

	if(fp == NULL)
		return 1;


	while(fgets(line, sizeof(line)-1, fp)) {

		if(line[0] == '/' && line[1] == '/')
			continue;
		//Character names must not exceed the 23+\0 limit. [Skotlex]
		sscanf(line, "%d,%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23[^,],%d,%23s",&cid,
		&temp[0],p->friends[0].name,
		&temp[1],p->friends[1].name,
		&temp[2],p->friends[2].name,
		&temp[3],p->friends[3].name,
		&temp[4],p->friends[4].name,
		&temp[5],p->friends[5].name,
		&temp[6],p->friends[6].name,
		&temp[7],p->friends[7].name,
		&temp[8],p->friends[8].name,
		&temp[9],p->friends[9].name,
		&temp[10],p->friends[10].name,
		&temp[11],p->friends[11].name,
		&temp[12],p->friends[12].name,
		&temp[13],p->friends[13].name,
		&temp[14],p->friends[14].name,
		&temp[15],p->friends[15].name,
		&temp[16],p->friends[16].name,
		&temp[17],p->friends[17].name,
		&temp[18],p->friends[18].name,
		&temp[19],p->friends[19].name);
		if (cid == p->char_id)
			break;
	}

	// No register of friends list
	if (cid == 0) {
		fclose(fp);
		return 0;
	}

	// Fill in the list

	for (i=0; i<20; i++)
		p->friends[i].char_id = temp[i];

	fclose(fp);
	return 0;
}
*/

// Note: Remember to keep this function updated with the one in src/char_sql/char.c [Skotlex]
// Unneded code was left commented for easier merging of changes.
// Function by [Ilpalazzo-sama]
 
int memitemdata_to_sql(struct itemtmp mapitem[], int count, int char_id, int tableswitch)
{
	int i; //, flag, id;
	char *tablename;
	char selectoption[16];

	switch (tableswitch) {
	case TABLE_INVENTORY:
		tablename = inventory_db; // no need for sprintf here as *_db are char*.
		sprintf(selectoption,"char_id");
		break;
	case TABLE_CART:
		tablename = cart_db;
		sprintf(selectoption,"char_id");
		break;
	case TABLE_STORAGE:
		tablename = storage_db;
		sprintf(selectoption,"account_id");
		break;
	case TABLE_GUILD_STORAGE:
		tablename = guild_storage_db;
		sprintf(selectoption,"guild_id");
		break;
	default:
		ShowError("Invalid table name!\n");
		return 1;
	}

	//=======================================mysql database data > memory===============================================
/*
         sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
		"FROM `%s` WHERE `%s`='%d'", tablename, selectoption, char_id);

         if (mysql_query(&mysql_handle, tmp_sql)) {
		ShowSQL("DB server Error (select `%s` to Memory)- %s\n",tablename ,mysql_error(&mysql_handle));
		return 1;
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		while ((sql_row = mysql_fetch_row(sql_res))) {
			flag = 0;
			id = atoi(sql_row[0]);
			for(i = 0; i < count; i++) {
				if(mapitem[i].flag == 1)
					continue;
				if(mapitem[i].nameid == atoi(sql_row[1])) { // produced items fixup
					if((mapitem[i].equip == atoi(sql_row[3])) &&
						(mapitem[i].identify == atoi(sql_row[4])) &&
						(mapitem[i].amount == atoi(sql_row[2])) &&
						(mapitem[i].refine == atoi(sql_row[5])) &&
						(mapitem[i].attribute == atoi(sql_row[6])) &&
						(mapitem[i].card[0] == atoi(sql_row[7])) &&
						(mapitem[i].card[1] == atoi(sql_row[8])) &&
						(mapitem[i].card[2] == atoi(sql_row[9])) &&
						(mapitem[i].card[3] == atoi(sql_row[10]))) {
						//printf("the same item : %d , equip : %d , i : %d , flag :  %d\n", mapitem.equip[i].nameid,mapitem.equip[i].equip , i, mapitem.equip[i].flag); //DEBUG-STRING
					} else {
//==============================================Memory data > SQL ===============================
						if(itemdb_isequip(mapitem[i].nameid) || (mapitem[i].card[0] == atoi(sql_row[7]))) {
							sprintf(tmp_sql,"UPDATE `%s` SET `equip`='%d', `identify`='%d', `refine`='%d',"
								"`attribute`='%d', `card0`='%d', `card1`='%d', `card2`='%d', `card3`='%d', `amount`='%d' WHERE `id`='%d' LIMIT 1",
								tablename, mapitem[i].equip, mapitem[i].identify, mapitem[i].refine, mapitem[i].attribute, mapitem[i].card[0],
								mapitem[i].card[1], mapitem[i].card[2], mapitem[i].card[3], mapitem[i].amount, id);
							if(mysql_query(&mysql_handle, tmp_sql))
								ShowSQL("DB server Error (UPdate `equ %s`)- %s\n", tablename, mysql_error(&mysql_handle));
						}
						//printf("not the same item : %d ; i : %d ; flag :  %d\n", mapitem.equip[i].nameid, i, mapitem.equip[i].flag);
					}
					flag = mapitem[i].flag = 1;
					break;
				}
			}
			if(!flag) {
				sprintf(tmp_sql,"DELETE from `%s` where `id`='%d'", tablename, id);
					if(mysql_query(&mysql_handle, tmp_sql))
						ShowSQL("DB server Error (DELETE `equ %s`)- %s\n", tablename, mysql_error(&mysql_handle));
			}
		}
		mysql_free_result(sql_res);
	}
*/
	for(i = 0; i < count; i++) {
		if(mapitem[i].nameid) {
			sprintf(tmp_sql,"INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` )"
				" VALUES ( '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d' )",
				tablename, selectoption,  char_id, mapitem[i].nameid, mapitem[i].amount, mapitem[i].equip, mapitem[i].identify, mapitem[i].refine,
				mapitem[i].attribute, mapitem[i].card[0], mapitem[i].card[1], mapitem[i].card[2], mapitem[i].card[3]);
			if(mysql_query(&mysql_handle, tmp_sql))
				ShowSQL("DB server Error (INSERT `equ %s`)- %s\n", tablename, mysql_error(&mysql_handle));
		}
	}
	return 0;
}

//Note: Remember to keep this function updated with src/char_sql/char.c [Skotlex]
//Unncessary code is left commented for easy merging.
int mmo_char_tosql(int char_id, struct mmo_charstatus *p){
	int i=0 ,party_exist=0;//,guild_exist;
	int count = 0;
	int diff = 1;
	char *tmp_ptr; //Building a single query should be more efficient than running
		//multiple queries for each thing about to be saved, right? [Skotlex]
	char save_status[100]; //For displaying save information. [Skotlex]
//	struct mmo_charstatus *cp;
	struct itemtmp mapitem[MAX_GUILD_STORAGE];

	if (char_id!=p->char_id) return 0;

/*
	cp = (struct mmo_charstatus*)idb_get(char_db_,char_id);

	if (cp == NULL) {
		cp = (struct mmo_charstatus *) aMalloc(sizeof(struct mmo_charstatus));
    		memset(cp, 0, sizeof(struct mmo_charstatus));
		idb_put(char_db_, char_id,cp);
	}
*/
	ShowInfo("Saving char "CL_WHITE"%d"CL_RESET" (%s)...\n",char_id,p->name);
	memset(save_status, 0, sizeof(save_status));

	count = 0;
//	diff = 0;
	//map inventory data
	for(i=0;i<MAX_INVENTORY;i++){
//		if (!compare_item(&p->inventory[i], &cp->inventory[i]))
//			diff = 1;
		if(p->inventory[i].nameid>0){
			mapitem[count].flag=0;
			mapitem[count].id = p->inventory[i].id;
			mapitem[count].nameid=p->inventory[i].nameid;
			mapitem[count].amount = p->inventory[i].amount;
			mapitem[count].equip = p->inventory[i].equip;
			mapitem[count].identify = p->inventory[i].identify;
			mapitem[count].refine = p->inventory[i].refine;
			mapitem[count].attribute = p->inventory[i].attribute;
			mapitem[count].card[0] = p->inventory[i].card[0];
			mapitem[count].card[1] = p->inventory[i].card[1];
			mapitem[count].card[2] = p->inventory[i].card[2];
			mapitem[count].card[3] = p->inventory[i].card[3];
			count++;
		}
	}
	//printf("- Save item data to MySQL!\n");
	if (diff)
		if (!memitemdata_to_sql(mapitem, count, p->char_id,TABLE_INVENTORY))
			strcat(save_status, " inventory");

	count = 0;

//	diff = 0;
	//map cart data
	for(i=0;i<MAX_CART;i++){
//		if (!compare_item(&p->cart[i], &cp->cart[i]))
//			diff = 1;
		if(p->cart[i].nameid>0){
			mapitem[count].flag=0;
			mapitem[count].id = p->cart[i].id;
			mapitem[count].nameid=p->cart[i].nameid;
			mapitem[count].amount = p->cart[i].amount;
			mapitem[count].equip = p->cart[i].equip;
			mapitem[count].identify = p->cart[i].identify;
			mapitem[count].refine = p->cart[i].refine;
			mapitem[count].attribute = p->cart[i].attribute;
			mapitem[count].card[0] = p->cart[i].card[0];
			mapitem[count].card[1] = p->cart[i].card[1];
			mapitem[count].card[2] = p->cart[i].card[2];
			mapitem[count].card[3] = p->cart[i].card[3];
			count++;
		}
	}

	if (diff)
		if (!memitemdata_to_sql(mapitem, count, p->char_id,TABLE_CART))
			strcat(save_status, " cart");
/*
	if ((p->base_exp != cp->base_exp) || (p->class_ != cp->class_) ||
	    (p->base_level != cp->base_level) || (p->job_level != cp->job_level) ||
	    (p->job_exp != cp->job_exp) || (p->zeny != cp->zeny) ||
	    (p->last_point.x != cp->last_point.x) || (p->last_point.y != cp->last_point.y) ||
	    (p->max_hp != cp->max_hp) || (p->hp != cp->hp) ||
	    (p->max_sp != cp->max_sp) || (p->sp != cp->sp) ||
	    (p->status_point != cp->status_point) || (p->skill_point != cp->skill_point) ||
	    (p->str != cp->str) || (p->agi != cp->agi) || (p->vit != cp->vit) ||
	    (p->int_ != cp->int_) || (p->dex != cp->dex) || (p->luk != cp->luk) ||
	    (p->option != cp->option) || (p->karma != cp->karma) || (p->manner != cp->manner) ||
	    (p->party_id != cp->party_id) || (p->guild_id != cp->guild_id) ||
	    (p->pet_id != cp->pet_id) || (p->hair != cp->hair) || (p->hair_color != cp->hair_color) ||
	    (p->clothes_color != cp->clothes_color) || (p->weapon != cp->weapon) ||
	    (p->shield != cp->shield) || (p->head_top != cp->head_top) ||
	    (p->head_mid != cp->head_mid) || (p->head_bottom != cp->head_bottom) ||
	    (p->partner_id != cp->partner_id) || (p->father != cp->father) ||
	    (p->mother != cp->mother) || (p->child != cp->child) || (p->fame != cp->fame))
	{	//Save status
	
		//Check for party
		party_exist=1;
		sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id` = '%d'",party_db, p->party_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			ShowSQL("DB Error - %s\n", mysql_error(&mysql_handle));
		} else { //In case of failure, don't touch the data. [Skotlex
			sql_res = mysql_store_result(&mysql_handle);
			sql_row = mysql_fetch_row(sql_res);
			if (sql_row)
				party_exist = atoi(sql_row[0]);
			mysql_free_result(sql_res);
		}

		//check guild_exist
		guild_exist=1;
		sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id` = '%d'",guild_db, p->guild_id); // TBR
		if (mysql_query(&mysql_handle, tmp_sql)) {
			ShowSQL("DB Error (select guild): %s\n", mysql_error(&mysql_handle));
		} else { //If we fail to confirm, don't touch the data.
			sql_res = mysql_store_result(&mysql_handle);
			sql_row = mysql_fetch_row(sql_res);
			if (sql_row)
				guild_exist = atoi(sql_row[0]);
			mysql_free_result(sql_res);
		}
		if (guild_exist==0) p->guild_id=0;
	*/	
		if (party_exist==0) p->party_id=0; //Parties are not converted. [Skotlex]

		//sql query
		//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
		//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
		//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
		//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
		//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
		//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)

	//Rather than converting the update to a insert, let's insert a blank char. [Skotlex]
		sprintf(tmp_sql, "INSERT INTO `%s` (`char_id`, `account_id`, `name`, `char_num`) VALUES ('%d','%d','%s', '%d')", char_db, char_id, p->account_id, p->name, p->char_num);
		if(mysql_query(&mysql_handle, tmp_sql))
			ShowSQL("DB Error (insert char): %s\n", mysql_error(&mysql_handle));
		//If there is an error, we could assume it already exists, so just update anyway.
		
		sprintf(tmp_sql ,"UPDATE `%s` SET `class`='%d', `base_level`='%d', `job_level`='%d',"
			"`base_exp`='%d', `job_exp`='%d', `zeny`='%d',"
			"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%d',`skill_point`='%d',"
			"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
			"`option`='%d',`karma`='%d',`manner`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',"
			"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
			"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d',"
			"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d', `fame`='%d'"
			"WHERE  `account_id`='%d' AND `char_id` = '%d'",
			char_db, p->class_, p->base_level, p->job_level,
			p->base_exp, p->job_exp, p->zeny,
			p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
			p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
			p->option, p->karma, p->manner, p->party_id, p->guild_id, p->pet_id,
			p->hair, p->hair_color, p->clothes_color,
			p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
			mapindex_id2name(p->last_point.map), p->last_point.x, p->last_point.y,
			mapindex_id2name(p->save_point.map), p->save_point.x, p->save_point.y, p->partner_id, p->father, p->mother,
			p->child, p->fame, p->account_id, p->char_id
		);

		if(mysql_query(&mysql_handle, tmp_sql))
			ShowSQL("DB Error (update char): %s\n", mysql_error(&mysql_handle));
		else
			strcat(save_status, " status");
/*
	}
	diff = 0;

	for(i=0;i<MAX_MEMOPOINTS;i++){
		if((strcmp(p->memo_point[i].map,cp->memo_point[i].map) == 0) && (p->memo_point[i].x == cp->memo_point[i].x) && (p->memo_point[i].y == cp->memo_point[i].y))
			continue;
		diff = 1;
		break;
	}
*/
	if (diff)
	{ //Save memo
		//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
		sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",memo_db, p->char_id);
		if(mysql_query(&mysql_handle, tmp_sql))
			ShowSQL("DB Error (delete memo): %s\n", mysql_error(&mysql_handle));

		//insert here.
		tmp_ptr = tmp_sql;
		tmp_ptr += sprintf(tmp_ptr, "INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ", memo_db);
		count = 0;
		for(i=0;i<MAX_MEMOPOINTS;i++){
			if(p->memo_point[i].map){
				tmp_ptr += sprintf(tmp_ptr,"('%d', '%s', '%d', '%d'),",
					char_id, mapindex_id2name(p->memo_point[i].map), p->memo_point[i].x, p->memo_point[i].y);
				count++;
			}
		}
		if (count)
		{	//Dangerous? Only if none of the above sprintf worked. [Skotlex]
			tmp_ptr[-1] = '\0'; //Remove the trailing comma.
			if(mysql_query(&mysql_handle, tmp_sql))
				ShowSQL("DB Error (insert memo): %s\n", mysql_error(&mysql_handle));
			else
				strcat(save_status, " memo");
		} else //Memo Points cleared (how is this possible?).
			strcat(save_status, " memo");
	}
/*
	diff = 0;
	for(i=0;i<MAX_SKILL;i++) {
		if ((p->skill[i].lv != 0) && (p->skill[i].id == 0))
			p->skill[i].id = i; // Fix skill tree

		if((p->skill[i].id != cp->skill[i].id) || (p->skill[i].lv != cp->skill[i].lv) ||
			(p->skill[i].flag != cp->skill[i].flag))
		{
			diff = 1;
			break;
		}
	}
*/
	if (diff)
	{	//Save skills
		
		//`skill` (`char_id`, `id`, `lv`)
		sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",skill_db, p->char_id);
		if(mysql_query(&mysql_handle, tmp_sql))
			ShowSQL("DB Error (delete skill): %s\n", mysql_error(&mysql_handle));

		tmp_ptr = tmp_sql;
		tmp_ptr += sprintf(tmp_ptr,"INSERT INTO `%s`(`char_id`,`id`,`lv`) VALUES ", skill_db);
		count = 0;
		//insert here.
		for(i=0;i<MAX_SKILL;i++){
			if(p->skill[i].id && p->skill[i].flag!=1)
			{
				tmp_ptr += sprintf(tmp_ptr,"('%d','%d','%d'),",
					char_id, p->skill[i].id, (p->skill[i].flag==0)?p->skill[i].lv:p->skill[i].flag-2);
				count++;
			}
		}
	
		if (count)
		{
			tmp_ptr[-1] = '\0';	//Remove trailing comma.
			if(mysql_query(&mysql_handle, tmp_sql))
				ShowSQL("DB Error (insert skill): %s\n", mysql_error(&mysql_handle));
			else
				strcat(save_status, " skills");
		} else //Skills removed (reset?)
			strcat(save_status, " skills");
	}
/*
	diff = 0;
	for(i=0;i<p->global_reg_num;i++) {
		if ((p->global_reg[i].str == NULL) && (cp->global_reg[i].str == NULL))
			continue;
		if (((p->global_reg[i].str == NULL) != (cp->global_reg[i].str == NULL)) ||
			(p->global_reg[i].value != cp->global_reg[i].value) ||
			strcmp(p->global_reg[i].str, cp->global_reg[i].str) != 0
		) {
			diff = 1;
			break;
		}
	}
*/
/*
	if (diff)
	{	//Save global registry.
		//`global_reg_value` (`char_id`, `str`, `value`)
		
		sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, p->char_id);
		if (mysql_query(&mysql_handle, tmp_sql))
			ShowSQL("DB Error (delete global_reg_value): %s\n", mysql_error(&mysql_handle));

		//insert here.
		tmp_ptr = tmp_sql;
		tmp_ptr += sprintf(tmp_ptr,"INSERT INTO `%s` (`char_id`, `str`, `value`) VALUES", reg_db);
		count = 0;
		for(i=0;i<p->global_reg_num;i++)
		{
			if (p->global_reg[i].str && p->global_reg[i].value !=0)
			{
				tmp_ptr += sprintf(tmp_ptr,"('%d','%s','%s'),",
					char_id, jstrescapecpy(temp_str,p->global_reg[i].str), p->global_reg[i].value);
				if (++count%100 == 0)
				{ //Save every X registers to avoid overflowing tmp_sql [Skotlex]
					tmp_ptr[-1] = '\0';
					if(mysql_query(&mysql_handle, tmp_sql))
						ShowSQL("DB Error (insert global_reg_value sub): %s\n", mysql_error(&mysql_handle));
					else
						strcat(save_status, " global_reg");
						
					tmp_ptr = tmp_sql;
					tmp_ptr += sprintf(tmp_ptr,"INSERT INTO `%s` (`char_id`, `str`, `value`) VALUES", reg_db);
					count = 0;
				}
			}
		}

		if (count)
		{
			tmp_ptr[-1] = '\0';
			if(mysql_query(&mysql_handle, tmp_sql))
				ShowSQL("DB Error (insert global_reg_value): %s\n", mysql_error(&mysql_handle));
			else
				strcat(save_status, " global_reg");
		} else //Values cleared.
			strcat(save_status, " global_reg");
	}
*/
/*	
	diff = 0;
	for(i = 0; i < MAX_FRIENDS; i++){
		if(p->friend_id[i] != cp->friend_id[i]){
			diff = 1;
			break;
		}
	}
	if(diff)
	{	//Save friends	
		sprintf(tmp_sql, "DELETE FROM `%s` WHERE `char_id`='%d'", friend_db, char_id);
		if(mysql_query(&mysql_handle, tmp_sql)){
			ShowSQL("DB Error (delete friend): %s\n", mysql_error(&mysql_handle));
		}
		  
		tmp_ptr = tmp_sql;
		tmp_ptr += sprintf(tmp_ptr, "INSERT INTO `%s` (`char_id`, `friend_id`) VALUES ", friend_db);
		count = 0;
		for(i = 0; i < MAX_FRIENDS; i++){
			if(p->friend_id[i] > 0)
			{
				tmp_ptr += sprintf(tmp_ptr, "('%d', '%d'),", char_id, p->friend_id[i]);
				count++;
			}
		}
		if (count)
		{
			tmp_ptr[-1] = '\0';	//Remove the last comma. [Skotlex]
			if(mysql_query(&mysql_handle, tmp_sql))
				ShowSQL("DB Error (insert friend): %s\n", mysql_error(&mysql_handle));
			else
				strcat(save_status, " friends");
		} else //Friend list cleared.
			strcat(save_status, " friends");

	}
*/
	ShowInfo("Saved char %d:%s.\n", char_id, save_status);
//	memcpy(cp, p, sizeof(struct mmo_charstatus));

	return 0;
}

//--------------------------------------------------------
// Save registry to sql
int inter_accreg_tosql(int account_id, int char_id, struct accreg *reg, int type){

	int j;
	char temp_str[64]; //Needs be twice the source to ensure it fits [Skotlex]
	char temp_str2[512];
	if (account_id<=0) return 0;

	switch (type) {
		case 3: //Char Reg
		//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, char_id);
			break;
		case 2: //Account Reg
		//`global_reg_value` (`type`, `account_id`, `char_id`, `str`, `value`)
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=2 AND `account_id`='%d'",reg_db, account_id);
			break;
		case 1: //Account2 Reg
			ShowError("inter_accreg_tosql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
			return 0;
		default:
			ShowError("inter_accreg_tosql: Invalid type %d\n", type);
			return 0;
			
	}	
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}

	if (reg->reg_num<=0) return 0;

	for(j=0;j<reg->reg_num;j++){
		if(reg->reg[j].str != NULL){
			sprintf(tmp_sql,"INSERT INTO `%s` (`type`, `account_id`, `char_id`, `str`, `value`) VALUES ('%d','%d','%d','%s','%s')",
				reg_db, type, type!=3?account_id:0, type==3?char_id:0,
				jstrescapecpy(temp_str,reg->reg[j].str), jstrescapecpy(temp_str2,reg->reg[j].value));
			if(mysql_query(&mysql_handle, tmp_sql) ) {
				ShowSQL("DB error - %s\n",mysql_error(&mysql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
		}
	}
	return 1;
}
//Note: Keep this function updated with the one in src/char/int_storage.c [Skotlex]
int storage_fromstr(char *str,struct storage *p)
{
	int tmp_int[256];
	int set,next,len,i;

	set=sscanf(str,"%d,%d%n",&tmp_int[0],&tmp_int[1],&next);
	p->storage_amount=tmp_int[1];

	if(set!=2)
		return 1;
	if(str[next]=='\n' || str[next]=='\r')
		return 0;
	next++;
	for(i=0;str[next] && str[next]!='\t';i++){
		if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6],
		      &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &tmp_int[10], &len) == 12) {
			p->storage_[i].id = tmp_int[0];
			p->storage_[i].nameid = tmp_int[1];
			p->storage_[i].amount = tmp_int[2];
			p->storage_[i].equip = tmp_int[3];
			p->storage_[i].identify = tmp_int[4];
			p->storage_[i].refine = tmp_int[5];
			p->storage_[i].attribute = tmp_int[6];
			p->storage_[i].card[0] = tmp_int[7];
			p->storage_[i].card[1] = tmp_int[8];
			p->storage_[i].card[2] = tmp_int[9];
			p->storage_[i].card[3] = tmp_int[10];
			next += len;
			if (str[next] == ' ')
				next++;
		}

		else if(sscanf(str + next, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d%n",
		      &tmp_int[0], &tmp_int[1], &tmp_int[2], &tmp_int[3],
		      &tmp_int[4], &tmp_int[5], &tmp_int[6],
		      &tmp_int[7], &tmp_int[8], &tmp_int[9], &tmp_int[10], &len) == 11) {
			p->storage_[i].id = tmp_int[0];
			p->storage_[i].nameid = tmp_int[1];
			p->storage_[i].amount = tmp_int[2];
			p->storage_[i].equip = tmp_int[3];
			p->storage_[i].identify = tmp_int[4];
			p->storage_[i].refine = tmp_int[5];
			p->storage_[i].attribute = tmp_int[6];
			p->storage_[i].card[0] = tmp_int[7];
			p->storage_[i].card[1] = tmp_int[8];
			p->storage_[i].card[2] = tmp_int[9];
			p->storage_[i].card[3] = tmp_int[10];
			next += len;
			if (str[next] == ' ')
				next++;
		}

		else return 1;
	}
	return 0;
}

//Note: Keep this function synched with the one in src/char_sql/int_storage.c
int storage_tosql(int account_id,struct storage *p){
	int i;
//	int eqcount=1;
//	int noteqcount=1;
	int count=0;
	struct itemtmp mapitem[MAX_GUILD_STORAGE];
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
			mapitem[count].card[0] = p->storage_[i].card[0];
			mapitem[count].card[1] = p->storage_[i].card[1];
			mapitem[count].card[2] = p->storage_[i].card[2];
			mapitem[count].card[3] = p->storage_[i].card[3];
			count++;
		}
	}

	memitemdata_to_sql(mapitem, count, account_id,TABLE_STORAGE);

	//printf ("storage dump to DB - id: %d (total: %d)\n", account_id, j);
	return 0;
}

//Note: Keep updated with the function in src/char/int_pet.c [Skotlex]
int inter_pet_fromstr(char *str,struct s_pet *p)
{
	int s;
	int tmp_int[16];
	char tmp_str[256];

	memset(p,0,sizeof(struct s_pet));

//	printf("sscanf pet main info\n");
	s=sscanf(str,"%d,%d,%[^\t]\t%d,%d,%d,%d,%d,%d,%d,%d,%d",&tmp_int[0],&tmp_int[1],tmp_str,&tmp_int[2],
		&tmp_int[3],&tmp_int[4],&tmp_int[5],&tmp_int[6],&tmp_int[7],&tmp_int[8],&tmp_int[9],&tmp_int[10]);

	if(s!=12)
		return 1;

	p->pet_id = tmp_int[0];
	p->class_ = tmp_int[1];
	memcpy(p->name,tmp_str,NAME_LENGTH-1);
	p->account_id = tmp_int[2];
	p->char_id = tmp_int[3];
	p->level = tmp_int[4];
	p->egg_id = tmp_int[5];
	p->equip = tmp_int[6];
	p->intimate = tmp_int[7];
	p->hungry = tmp_int[8];
	p->rename_flag = tmp_int[9];
	p->incuvate = tmp_int[10];

	if(p->hungry < 0)
		p->hungry = 0;
	else if(p->hungry > 100)
		p->hungry = 100;
	if(p->intimate < 0)
		p->intimate = 0;
	else if(p->intimate > 1000)
		p->intimate = 1000;

	return 0;
}

//Note: Keep updated with the function in src/char_sql/int_pet.c [Skotlex]
int inter_pet_tosql(int pet_id, struct s_pet *p) {
	//`pet` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`)
	char t_name[NAME_LENGTH*2];

	ShowInfo("Saving pet (%d)...\n",pet_id);

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
			ShowSQL("DB server Error - %s\n", mysql_error(&mysql_handle) );
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	if (sql_res!=NULL && mysql_num_rows(sql_res)>0)
		//row reside -> updating
		sprintf(tmp_sql, "UPDATE `%s` SET `class`='%d',`name`='%s',`account_id`='%d',`char_id`='%d',`level`='%d',`egg_id`='%d',`equip`='%d',`intimate`='%d',`hungry`='%d',`rename_flag`='%d',`incuvate`='%d' WHERE `pet_id`='%d'",
			pet_db, p->class_, t_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate, p->pet_id);
	else //no row -> insert
		sprintf(tmp_sql,"INSERT INTO `%s` (`pet_id`, `class`,`name`,`account_id`,`char_id`,`level`,`egg_id`,`equip`,`intimate`,`hungry`,`rename_flag`,`incuvate`) VALUES ('%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			pet_db, pet_id, p->class_, t_name, p->account_id, p->char_id, p->level, p->egg_id,
			p->equip, p->intimate, p->hungry, p->rename_flag, p->incuvate);
	mysql_free_result(sql_res) ; //resource free
	if(mysql_query(&mysql_handle, tmp_sql) ) {
		ShowSQL("DB server Error (inset/update `pet`)- %s\n", mysql_error(&mysql_handle) );
	}

	ShowInfo("Pet saved (%d). \n", pet_id);
	return 0;
}


int mmo_char_init(void){
	char line[65536];
	struct storage *storage_ = NULL;
	struct s_pet *p;
	int ret;
	int i=0,set,tmp_int[2], c= 0;
	char input;
	FILE *fp;

	mysql_init(&mysql_handle);
	ShowInfo("Connect DB server.... (inter server)\n");
	if(!mysql_real_connect(&mysql_handle, db_server_ip, db_server_id, db_server_pw,
		db_server_logindb ,db_server_port, (char *)NULL, 0)) {
			//pointer check
			ShowFatalError("%s\n",mysql_error(&mysql_handle));
			exit(1);
	}
	else {
		ShowStatus ("connect success! (inter server)\n");
	}

	ShowWarning("Make sure you backup your databases before continuing!\n");
	printf("\n");
	ShowNotice("Do you wish to convert your Character Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y'){
		ShowStatus("Converting Character Database...\n");
		fp=fopen("save/athena.txt","r");
		char_dat = (struct character_data*)malloc(sizeof(struct character_data)*256);
		char_max=256;
		if(fp==NULL)
			return 0;
		while(fgets(line, 65535, fp)){
			if(char_num>=char_max){
				char_max+=256;
				char_dat = (struct character_data*)realloc(char_dat, sizeof(char_dat[0]) *char_max);
			}
			memset(&char_dat[char_num], 0, sizeof(char_dat[0]));
			ret=mmo_char_fromstr(line, &char_dat[char_num].status, &char_dat[char_num].global);
			if(ret){
				mmo_char_tosql(char_dat[char_num].status.char_id , &char_dat[char_num].status);
				inter_accreg_tosql(char_dat[char_num].status.account_id, char_dat[char_num].status.char_id, &char_dat[char_num].global, 3); //Type 3: Character regs
				if(char_dat[char_num].status.char_id>=char_id_count)
					char_id_count=char_dat[char_num].status.char_id+1;
				char_num++;
			}
		}
		ShowStatus("char data convert end\n");
		fclose(fp);
	}
	
	while(getchar() != '\n');
	printf("\n");
	ShowNotice("Do you wish to convert your Storage Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y') {
		printf("\n");
		ShowStatus("Converting Storage Database...\n");
		fp=fopen(storage_txt,"r");
		if(fp==NULL){
			ShowError("cant't read : %s\n",storage_txt);
			return 0;
		}

		while(fgets(line,65535,fp)){
			set=sscanf(line,"%d,%d",&tmp_int[0],&tmp_int[1]);
			if(set==2) {
				if(i==0){
					storage_ = (struct storage*)malloc(sizeof(struct storage));
				}else{
					storage_ = (struct storage*)realloc(storage_,sizeof(struct storage)*(i+1));
				}
				memset(&storage_[i],0,sizeof(struct storage));
				storage_[i].account_id=tmp_int[0];
				storage_fromstr(line,&storage_[i]);
				storage_tosql(tmp_int[0],&storage_[i]); //to sql. (dump)
				i++;
			}
		}
		fclose(fp);
	}

	while(getchar() != '\n');
	printf("\n");
	ShowNotice("Do you wish to convert your Pet Database to SQL? (y/n) : ");
	input=getchar();
	if(input == 'y' || input == 'Y') {
		printf("\n");
		ShowStatus("Converting Pet Database...\n");
		if( (fp=fopen(pet_txt,"r")) ==NULL )
			return 1;
		p = (struct s_pet*)malloc(sizeof(struct s_pet));
		while(fgets(line, sizeof(line), fp)){
			if(p==NULL){
				ShowFatalError("int_pet: out of memory!\n");
				exit(0);
			}
			if(inter_pet_fromstr(line, p)==0 && p->pet_id>0){
				//pet dump
				inter_pet_tosql(p->pet_id,p);
			}else{
				ShowError("int_pet: broken data [%s] line %d\n", pet_txt, c);
			}
			c++;
		}
		fclose(fp);
	}
	return 0;
}

int inter_config_read(const char *cfgName) {
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowStatus("Start reading interserver configuration: %s\n",cfgName);

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}
	while(fgets(line, 1020, fp)){
		i=sscanf(line,"%[^:]:%s", w1, w2);
		if(i!=2)
			continue;
		if(strcmpi(w1,"storage_txt")==0){
			ShowInfo ("set storage_txt : %s\n",w2);
			strncpy(storage_txt, w2, sizeof(storage_txt));
		} else if(strcmpi(w1,"pet_txt")==0){
			ShowInfo ("set pet_txt : %s\n",w2);
			strncpy(pet_txt, w2, sizeof(pet_txt));
		}
		//add for DB connection
		else if(strcmpi(w1,"db_server_ip")==0){
			strcpy(db_server_ip, w2);
			ShowInfo ("set db_server_ip : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_port")==0){
			db_server_port=atoi(w2);
			ShowInfo ("set db_server_port : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_id")==0){
			strcpy(db_server_id, w2);
			ShowInfo ("set db_server_id : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_pw")==0){
			strcpy(db_server_pw, w2);
			ShowInfo ("set db_server_pw : %s\n",w2);
		}
		else if(strcmpi(w1,"db_server_logindb")==0){
			strcpy(db_server_logindb, w2);
			ShowInfo ("set db_server_logindb : %s\n",w2);
		}
		else if(strcmpi(w1,"char_db")==0){
			strcpy(char_db, w2);
			ShowInfo ("set char_db : %s\n",w2);
		}
		else if(strcmpi(w1,"cart_db")==0){
			strcpy(cart_db, w2);
			ShowInfo ("set cart_db : %s\n",w2);
		}
		else if(strcmpi(w1,"inventory_db")==0){
			strcpy(inventory_db, w2);
			ShowInfo ("set inventory_db : %s\n",w2);
		}
		else if(strcmpi(w1,"storage_db")==0){
			strcpy(storage_db, w2);
			ShowInfo ("set storage_db : %s\n",w2);
		}
		else if(strcmpi(w1,"reg_db")==0){
			strcpy(reg_db, w2);
			ShowInfo ("set reg_db : %s\n",w2);
		}
		else if(strcmpi(w1,"skill_db")==0){
			strcpy(skill_db, w2);
			ShowInfo ("set skill_db : %s\n",w2);
		}
		else if(strcmpi(w1,"memo_db")==0){
			strcpy(memo_db, w2);
			ShowInfo ("set memo_db : %s\n",w2);
		}
		else if(strcmpi(w1,"pet_db")==0){
			strcpy(pet_db, w2);
			ShowInfo ("set pet_db : %s\n",w2);
		}
		else if(strcmpi(w1,"friend_db")==0){
			strcpy(friend_db, w2);
			ShowInfo ("set friend_db : %s\n",w2);
		//support the import command, just like any other config
		}else if(strcmpi(w1,"import")==0){
			inter_config_read(w2);
		}
	}
	fclose(fp);

	ShowStatus("Done reading %s.\n", cfgName);

	return 0;
}

int char_config_read(const char *cfgName) {
	int i;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	ShowStatus ("Start reading char-server configuration: %s\n",cfgName);

	fp=fopen(cfgName,"r");
	if(fp==NULL){
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

		while(fgets(line, 1020, fp)){
			if(line[0] == '/' && line[1] == '/')
				continue;

			i=sscanf(line,"%[^:]:%s", w1, w2);
			if(i!=2)
				continue;
			if(strcmpi(w1,"char_txt")==0){
				ShowInfo ("set char_txt : %s\n",w2);
				strcpy(char_txt, w2);
			} else
			if(strcmpi(w1,"friends_txt")==0){
				ShowInfo ("set friends_txt : %s\n",w2);
				strcpy(friends_txt, w2);
			}
		}
		fclose(fp);
		ShowStatus("Done reading %s.\n", cfgName);

	return 0;
}

int do_init(int argc, char **argv){

	char_config_read((argc>1)?argv[1]:CHAR_CONF_NAME);
	inter_config_read((argc>2)?argv[2]:INTER_CONF_NAME);
	mapindex_init();

	mmo_char_init();
	ShowStatus("Everything's been converted!\n");
	mapindex_final();
	exit (0);
}

void do_final () {}
