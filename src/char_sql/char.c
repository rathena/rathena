// $Id: char.c,v 1.16 2004/09/23 18:31:16 MouseJstr Exp $
// original : char2.c 2003/03/14 11:58:35 Rev.1.5
//
// original code from athena
// SQL conversion by Jioh L. Jung
// TXT 1.105
#include <sys/types.h>

#ifdef LCCWIN32
#include <winsock.h>
#pragma lib <libmysql.lib>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "../common/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdarg.h>

#include "char.h"
#include "strlib.h"
#include "itemdb.h"
#include "inter.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

char char_db[256] = "char";
char cart_db[256] = "cart_inventory";
char inventory_db[256] = "inventory";
char charlog_db[256] = "charlog";
char storage_db[256] = "storage";
char interlog_db[256] = "interlog";
char reg_db[256] = "global_reg_value";
char skill_db[256] = "skill";
char memo_db[256] = "memo";
char guild_db[256] = "guild";
char guild_alliance_db[256] = "guild_alliance";
char guild_castle_db[256] = "guild_castle";
char guild_expulsion_db[256] = "guild_expulsion";
char guild_member_db[256] = "guild_member";
char guild_position_db[256] = "guild_position";
char guild_skill_db[256] = "guild_skill";
char guild_storage_db[256] = "guild_storage";
char party_db[256] = "party";
char pet_db[256] = "pet";
char login_db[256] = "login";

char login_db_account_id[32] = "account_id";
char login_db_level[32] = "level";

int lowest_gm_level = 1;

unsigned char *SQL_CONF_NAME = "conf/inter_athena.conf";

struct mmo_map_server server[MAX_MAP_SERVERS];
int server_fd[MAX_MAP_SERVERS];
int server_freezeflag[MAX_MAP_SERVERS]; // Map-server anti-freeze system. Counter. 5 ok, 4...0 freezed

int anti_freeze_enable = 0;
int ANTI_FREEZE_INTERVAL = 6;

int login_fd, char_fd;
char userid[24];
char passwd[24];
char server_name[20];
char wisp_server_name[24] = "Server";
int login_ip_set_ = 0;
char login_ip_str[128];
int login_ip;
int login_port = 6900;
int char_ip_set_ = 0;
char char_ip_str[128];
int char_ip;
int char_port = 6121;
int char_maintenance;
int char_new;
int name_ignoring_case = 0; // Allow or not identical name for characters but with a different case by [Yor]
int char_name_option = 0; // Option to know which letters/symbols are authorised in the name of a character (0: all, 1: only those in char_name_letters, 2: all EXCEPT those in char_name_letters) by [Yor]
char char_name_letters[1024] = ""; // list of letters/symbols used to authorise or not a name of a character. by [Yor]

int log_char = 1;	// loggin char or not [devil]
int log_inter = 1;	// loggin inter or not [devil]

char lan_map_ip[128]; // Lan map ip added by kashy
int subnetmaski[4]; // Subnetmask added by kashy
char unknown_char_name[1024] = "Unknown";
char db_path[1024]="db";

//Added for Mugendai's I'm Alive mod
int imalive_on=0;
int imalive_time=60;
//Added by Mugendai for GUI
int flush_on=1;
int flush_time=100;

struct char_session_data{
	int account_id,login_id1,login_id2,sex;
	int found_char[9];
	char email[40]; // e-mail (default: a@a.com) by [Yor]
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
};

#define AUTH_FIFO_SIZE 256
struct {
	int account_id,char_id,login_id1,login_id2,ip,char_pos,delflag,sex;
	time_t connect_until_time; // # of seconds 1/1/1970 (timestamp): Validity limit of the account (0 = unlimited)
} auth_fifo[AUTH_FIFO_SIZE];
int auth_fifo_pos = 0;

int check_ip_flag = 1; // It's to check IP of a player between char-server and other servers (part of anti-hacking system)

int char_id_count = 150000;
struct mmo_charstatus *char_dat;
int char_num,char_max;
int max_connect_user = 0;
int autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
int start_zeny = 500;
int start_weapon = 1201;
int start_armor = 2301;

// check for exit signal
// 0 is saving complete
// other is char_id
unsigned int save_flag = 0;

// start point (you can reset point on conf file)
struct point start_point = {"new_1-1.gat", 53, 111};

struct gm_account *gm_account = NULL;
int GM_num = 0;

int console = 0;

//-----------------------------------------------------
// Function to suppress control characters in a string.
//-----------------------------------------------------
int remove_control_chars(unsigned char *str) {
	int i;
	int change = 0;

	for(i = 0; i < strlen(str); i++) {
		if (str[i] < 32) {
			str[i] = '_';
			change = 1;
		}
	}

	return change;
}

//----------------------------------------------------------------------
// Determine if an account (id) is a GM account
// and returns its level (or 0 if it isn't a GM account or if not found)
//----------------------------------------------------------------------
// Removed since nothing GM related goes on in the char server [CLOWNISIUS]
int isGM(int account_id) {
	int i;

	for(i = 0; i < GM_num; i++)
		if (gm_account[i].account_id == account_id)
			return gm_account[i].level;
	return 0;
}

void read_gm_account(void) {
	if (gm_account != NULL)
		free(gm_account);
	GM_num = 0;

	sprintf(tmp_lsql, "SELECT `%s`,`%s` FROM `%s` WHERE `%s`>='%d'",login_db_account_id,login_db_level,login_db,login_db_level,lowest_gm_level);
	if (mysql_query(&lmysql_handle, tmp_lsql)) {
		printf("DB server Error (select %s to Memory)- %s\n",login_db,mysql_error(&lmysql_handle));
	}
	lsql_res = mysql_store_result(&lmysql_handle);
	if (lsql_res) {
		gm_account = calloc(sizeof(struct gm_account) * mysql_num_rows(lsql_res), 1);
		while ((lsql_row = mysql_fetch_row(lsql_res))) {
			gm_account[GM_num].account_id = atoi(lsql_row[0]);
			gm_account[GM_num].level = atoi(lsql_row[1]);
			GM_num++;
		}
	}

	mysql_free_result(lsql_res);
}

//=====================================================================================================
int mmo_char_tosql(int char_id, struct mmo_charstatus *p){
	int i=0,party_exist,guild_exist;
	int eqcount=1;
	int noteqcount=1;
	char temp_str[32];

	struct itemtemp mapitem;
	if (char_id!=p->char_id) return 0;

	save_flag = p->char_id;
	printf("(\033[1;32m%d\033[0m) %s \trequest save char data - ",char_id,char_dat[0].name);



//for(testcount=1;testcount<50;testcount++){//---------------------------test count--------------------
//	printf("test count : %d\n", testcount);
//	eqcount=1;
//	noteqcount=1;
//	dbeqcount=1;
//	dbnoteqcount=1;
//-----------------------------------------------------------------------------------------------------

//=========================================map  inventory data > memory ===============================
	//map inventory data
	for(i=0;i<MAX_INVENTORY;i++){
		if(p->inventory[i].nameid>0){
			if(itemdb_isequip(p->inventory[i].nameid)==1){
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->inventory[i].id;
				mapitem.equip[eqcount].nameid=p->inventory[i].nameid;
				mapitem.equip[eqcount].amount = p->inventory[i].amount;
				mapitem.equip[eqcount].equip = p->inventory[i].equip;
				mapitem.equip[eqcount].identify = p->inventory[i].identify;
				mapitem.equip[eqcount].refine = p->inventory[i].refine;
				mapitem.equip[eqcount].attribute = p->inventory[i].attribute;
				mapitem.equip[eqcount].card[0] = p->inventory[i].card[0];
				mapitem.equip[eqcount].card[1] = p->inventory[i].card[1];
				mapitem.equip[eqcount].card[2] = p->inventory[i].card[2];
				mapitem.equip[eqcount].card[3] = p->inventory[i].card[3];
				eqcount++;
			}
			else if(itemdb_isequip(p->inventory[i].nameid)==0){
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->inventory[i].id;
				mapitem.notequip[noteqcount].nameid=p->inventory[i].nameid;
				mapitem.notequip[noteqcount].amount = p->inventory[i].amount;
				mapitem.notequip[noteqcount].equip = p->inventory[i].equip;
				mapitem.notequip[noteqcount].identify = p->inventory[i].identify;
				mapitem.notequip[noteqcount].refine = p->inventory[i].refine;
				mapitem.notequip[noteqcount].attribute = p->inventory[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->inventory[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->inventory[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->inventory[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->inventory[i].card[3];
				noteqcount++;
			}
		}
	}
	//printf("- Save item data to MySQL!\n");
	memitemdata_to_sql(mapitem, eqcount, noteqcount, p->char_id,TABLE_INVENTORY);

//=========================================map  cart data > memory ====================================
	eqcount=1;
	noteqcount=1;

	//map cart data
	for(i=0;i<MAX_CART;i++){
		if(p->cart[i].nameid>0){
			if(itemdb_isequip(p->cart[i].nameid)==1){
				mapitem.equip[eqcount].flag=0;
				mapitem.equip[eqcount].id = p->cart[i].id;
				mapitem.equip[eqcount].nameid=p->cart[i].nameid;
				mapitem.equip[eqcount].amount = p->cart[i].amount;
				mapitem.equip[eqcount].equip = p->cart[i].equip;
				mapitem.equip[eqcount].identify = p->cart[i].identify;
				mapitem.equip[eqcount].refine = p->cart[i].refine;
				mapitem.equip[eqcount].attribute = p->cart[i].attribute;
				mapitem.equip[eqcount].card[0] = p->cart[i].card[0];
				mapitem.equip[eqcount].card[1] = p->cart[i].card[1];
				mapitem.equip[eqcount].card[2] = p->cart[i].card[2];
				mapitem.equip[eqcount].card[3] = p->cart[i].card[3];
				eqcount++;
			}
			else if(itemdb_isequip(p->cart[i].nameid)==0){
				mapitem.notequip[noteqcount].flag=0;
				mapitem.notequip[noteqcount].id = p->cart[i].id;
				mapitem.notequip[noteqcount].nameid=p->cart[i].nameid;
				mapitem.notequip[noteqcount].amount = p->cart[i].amount;
				mapitem.notequip[noteqcount].equip = p->cart[i].equip;
				mapitem.notequip[noteqcount].identify = p->cart[i].identify;
				mapitem.notequip[noteqcount].refine = p->cart[i].refine;
				mapitem.notequip[noteqcount].attribute = p->cart[i].attribute;
				mapitem.notequip[noteqcount].card[0] = p->cart[i].card[0];
				mapitem.notequip[noteqcount].card[1] = p->cart[i].card[1];
				mapitem.notequip[noteqcount].card[2] = p->cart[i].card[2];
				mapitem.notequip[noteqcount].card[3] = p->cart[i].card[3];
				noteqcount++;
			}
		}
	}

	//printf("- Save cart data to MySQL!\n");
	memitemdata_to_sql(mapitem, eqcount, noteqcount, p->char_id,TABLE_CART);

//=====================================================================================================

//}//---------------------------test count------------------------------
	//check party_exist
	party_exist=0;
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `party_id` = '%d'",party_db, p->party_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	sql_row = mysql_fetch_row(sql_res);
	if (sql_row) party_exist = atoi(sql_row[0]);
	mysql_free_result(sql_res);

	//check guild_exist
	guild_exist=0;
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `guild_id` = '%d'",guild_db, p->guild_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	sql_row = mysql_fetch_row(sql_res);
	if (sql_row) guild_exist = atoi(sql_row[0]);
	mysql_free_result(sql_res);

	if (guild_exist==0) p->guild_id=0;
	if (party_exist==0) p->party_id=0;

	//sql query
	//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
	//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
	//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
	//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
	//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
	//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
	//printf("- Save char data to MySQL!\n");
	sprintf(tmp_sql ,"UPDATE `%s` SET `class`='%d', `base_level`='%d', `job_level`='%d',"
		"`base_exp`='%d', `job_exp`='%d', `zeny`='%d',"
		"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%d',`skill_point`='%d',"
		"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
		"`option`='%d',`karma`='%d',`manner`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',"
		"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
		"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d',`partner_id`='%d' WHERE  `account_id`='%d' AND `char_id` = '%d'",
		char_db, p->class, p->base_level, p->job_level,
		p->base_exp, p->job_exp, p->zeny,
		p->max_hp, p->hp, p->max_sp, p->sp, p->status_point, p->skill_point,
		p->str, p->agi, p->vit, p->int_, p->dex, p->luk,
		p->option, p->karma, p->manner, p->party_id, p->guild_id, p->pet_id,
		p->hair, p->hair_color, p->clothes_color,
		p->weapon, p->shield, p->head_top, p->head_mid, p->head_bottom,
		p->last_point.map, p->last_point.x, p->last_point.y,
		p->save_point.map, p->save_point.x, p->save_point.y, p->partner_id, p->account_id, p->char_id
	);

	if(mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle));
	}

	//printf("- Save memo data to MySQL!\n");
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",memo_db, p->char_id);
	if(mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (delete `memo`)- %s\n", mysql_error(&mysql_handle));
	}

	//insert here.
	for(i=0;i<10;i++){
		if(p->memo_point[i].map[0]){
			sprintf(tmp_sql,"INSERT INTO `%s`(`char_id`,`map`,`x`,`y`) VALUES ('%d', '%s', '%d', '%d')",
				memo_db, char_id, p->memo_point[i].map, p->memo_point[i].x, p->memo_point[i].y);
			if(mysql_query(&mysql_handle, tmp_sql))
				printf("DB server Error (insert `memo`)- %s\n", mysql_error(&mysql_handle));
		}
	}

	//printf("- Save skill data to MySQL!\n");
	//`skill` (`char_id`, `id`, `lv`)
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",skill_db, p->char_id);
	if(mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (delete `skill`)- %s\n", mysql_error(&mysql_handle));
	}
	//printf("- Insert skill \n");
	//insert here.
	for(i=0;i<MAX_SKILL;i++){
		if(p->skill[i].id){
			if (p->skill[i].id && p->skill[i].flag!=1) {
				sprintf(tmp_sql,"INSERT delayed INTO `%s`(`char_id`, `id`, `lv`) VALUES ('%d', '%d','%d')",
					skill_db, char_id, p->skill[i].id, (p->skill[i].flag==0)?p->skill[i].lv:p->skill[i].flag-2);
				if(mysql_query(&mysql_handle, tmp_sql)) {
					printf("DB server Error (insert `skill`)- %s\n", mysql_error(&mysql_handle));
				}
			}
		}
	}


	//printf("- Save global_reg_value data to MySQL!\n");
	//`global_reg_value` (`char_id`, `str`, `value`)
	sprintf(tmp_sql,"DELETE FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, p->char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (delete `global_reg_value`)- %s\n", mysql_error(&mysql_handle));
	}

	//insert here.
	for(i=0;i<p->global_reg_num;i++){
		if (p->global_reg[i].str) {
			if(p->global_reg[i].value !=0){
					sprintf(tmp_sql,"INSERT INTO `%s` (`char_id`, `str`, `value`) VALUES ('%d', '%s','%d')",
					         reg_db, char_id, jstrescapecpy(temp_str,(unsigned char*)p->global_reg[i].str), p->global_reg[i].value);
					if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error (insert `global_reg_value`)- %s\n", mysql_error(&mysql_handle));
					}
			}
		}
	}
	printf("saving char is done.\n");
	save_flag = 0;

	return 0;
}

int memitemdata_to_sql(struct itemtemp mapitem, int eqcount, int noteqcount, int char_id, int tableswitch){
	//equ
	int i, j;
	int dbeqcount = 1;
	int dbnoteqcount = 1;
	struct itemtemp dbitem;
	char tablename[16];
	char selectoption[16];

	switch (tableswitch){
	case TABLE_INVENTORY:
		sprintf(tablename,"%s",inventory_db);
		sprintf(selectoption,"char_id");
		break;
	case TABLE_CART:
		sprintf(tablename,"%s",cart_db);
		sprintf(selectoption,"char_id");
		break;
	case TABLE_STORAGE:
		sprintf(tablename,"%s",storage_db);
		sprintf(selectoption,"account_id");
		break;
	case TABLE_GUILD_STORAGE:
		sprintf(tablename,"%s",guild_storage_db);
		sprintf(selectoption,"guild_id");
		break;
	}
	//printf("Working Table : %s \n",tablename);

	//=======================================mysql database data > memory===============================================

	sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3` "
		"FROM `%s` WHERE `%s`='%d'",tablename ,selectoption ,char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `%s` to Memory)- %s\n",tablename ,mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			if (itemdb_isequip(atoi(sql_row[1]))==1){
				dbitem.equip[dbeqcount].flag=0;
				dbitem.equip[dbeqcount].id = atoi(sql_row[0]);
				dbitem.equip[dbeqcount].nameid = atoi(sql_row[1]);
				dbitem.equip[dbeqcount].amount = atoi(sql_row[2]);
				dbitem.equip[dbeqcount].equip = atoi(sql_row[3]);
				dbitem.equip[dbeqcount].identify = atoi(sql_row[4]);
				dbitem.equip[dbeqcount].refine = atoi(sql_row[5]);
				dbitem.equip[dbeqcount].attribute = atoi(sql_row[6]);
				dbitem.equip[dbeqcount].card[0] = atoi(sql_row[7]);
				dbitem.equip[dbeqcount].card[1] = atoi(sql_row[8]);
				dbitem.equip[dbeqcount].card[2] = atoi(sql_row[9]);
				dbitem.equip[dbeqcount].card[3] = atoi(sql_row[10]);
				dbeqcount++;
			}else {
				dbitem.notequip[dbnoteqcount].flag=0;
				dbitem.notequip[dbnoteqcount].id = atoi(sql_row[0]);
				dbitem.notequip[dbnoteqcount].nameid = atoi(sql_row[1]);
				dbitem.notequip[dbnoteqcount].amount = atoi(sql_row[2]);
				dbitem.notequip[dbnoteqcount].equip = atoi(sql_row[3]);
				dbitem.notequip[dbnoteqcount].identify = atoi(sql_row[4]);
				dbitem.notequip[dbnoteqcount].refine = atoi(sql_row[5]);
				dbitem.notequip[dbnoteqcount].attribute = atoi(sql_row[6]);
				dbitem.notequip[dbnoteqcount].card[0] = atoi(sql_row[7]);
				dbitem.notequip[dbnoteqcount].card[1] = atoi(sql_row[8]);
				dbitem.notequip[dbnoteqcount].card[2] = atoi(sql_row[9]);
				dbitem.notequip[dbnoteqcount].card[3] = atoi(sql_row[10]);
				dbnoteqcount++;
			}
		}
		mysql_free_result(sql_res);
	}

	//==============================================Memory data > SQL ===============================
	//======================================Equip ITEM=======================================
	if((eqcount==1) && (dbeqcount==1)){//printf("%s Equip Empty\n",tablename);
	//item empty
	} else {

		for(i=1;i<eqcount;i++){
			for(j=1;j<dbeqcount;j++){
				if(mapitem.equip[i].flag==1) break;
				if(!(dbitem.equip[j].flag==1)){
					if(mapitem.equip[i].nameid==dbitem.equip[j].nameid){
						if ((mapitem.equip[i].equip==dbitem.equip[j].equip) && (mapitem.equip[i].identify==dbitem.equip[j].identify) && (mapitem.equip[i].amount==dbitem.equip[j].amount) &&

						    (mapitem.equip[i].refine==dbitem.equip[j].refine) && (mapitem.equip[i].attribute==dbitem.equip[j].attribute) && (mapitem.equip[i].card[0]==dbitem.equip[j].card[0]) &&
						    (mapitem.equip[i].card[1]==dbitem.equip[j].card[1]) && (mapitem.equip[i].card[2]==dbitem.equip[j].card[2]) && (mapitem.equip[i].card[3]==dbitem.equip[j].card[3])) {
							mapitem.equip[i].flag = 1;
							dbitem.equip[j].flag = 1;
							//printf("the same item : %d , equip : %d , i : %d , flag :  %d\n", mapitem.equip[i].nameid,mapitem.equip[i].equip , i, mapitem.equip[i].flag); //DEBUG-STRING
						} else {
							sprintf(tmp_sql,"UPDATE `%s` SET `equip`='%d', `identify`='%d', `refine`='%d',"
							"`attribute`='%d', `card0`='%d', `card1`='%d', `card2`='%d', `card3`='%d', `amount`='%d' WHERE `id`='%d' LIMIT 1",
							tablename, mapitem.equip[i].equip, mapitem.equip[i].identify, mapitem.equip[i].refine,mapitem.equip[i].attribute, mapitem.equip[i].card[0],
							mapitem.equip[i].card[1], mapitem.equip[i].card[2], mapitem.equip[i].card[3], mapitem.equip[i].amount, dbitem.equip[j].id);
							//printf("%s\n",tmp_sql);
							if(mysql_query(&mysql_handle, tmp_sql))
								printf("DB server Error (UPdate `equ %s`)- %s\n", tablename, mysql_error(&mysql_handle));
							mapitem.equip[i].flag=1;
							dbitem.equip[j].flag=1;
							//printf("not the same item : %d ; i : %d ; flag :  %d\n", mapitem.equip[i].nameid, i, mapitem.equip[i].flag);
						}
					}
				}
			}
		}

		//printf("dbeqcount = %d\n",dbeqcount);

		for(i=1;i<dbeqcount;i++){
			//printf("dbitem.equip[i].flag = %d , dbitem.equip[i].id = %d\n",dbitem.equip[i].flag,dbitem.equip[i].id);
			if (!(dbitem.equip[i].flag == 1)) {
				sprintf(tmp_sql,"DELETE from `%s` where `id`='%d'",tablename , dbitem.equip[i].id);
				//printf("%s", tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql))
					printf("DB server Error (DELETE `equ %s`)- %s\n", tablename ,mysql_error(&mysql_handle));
			}
		}
		for(i=1;i<eqcount;i++){
			if(!(mapitem.equip[i].flag==1)){
				sprintf(tmp_sql,"INSERT INTO `%s`(`%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)"
				" VALUES ( '%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
				tablename, selectoption,  char_id, mapitem.equip[i].nameid, mapitem.equip[i].amount, mapitem.equip[i].equip, mapitem.equip[i].identify, mapitem.equip[i].refine,
				mapitem.equip[i].attribute, mapitem.equip[i].card[0], mapitem.equip[i].card[1], mapitem.equip[i].card[2], mapitem.equip[i].card[3]);
				//printf("%s", tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql))
					printf("DB server Error (INSERT `equ %s`)- %s\n",tablename ,mysql_error(&mysql_handle));
			}
		}

		//======================================DEBUG=================================================

//		gettimeofday(&tv,NULL);
//		strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
//		printf("\n\n");
//		printf("Working Table Name : EQU %s,  Count : map %3d | db %3d \n",tablename ,eqcount ,dbeqcount);
//		printf("*********************************************************************************\n");
//		printf("======================================MAP===================Char ID %10d===\n",char_id);
//		printf("==flag ===name ===equip===ident===amoun===attri===card0===card1===card2===card3==\n");
//		for(j=1;j<eqcount;j++)
//			printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", mapitem.equip[j].flag,mapitem.equip[j].nameid, mapitem.equip[j].equip, mapitem.equip[j].identify, mapitem.equip[j].refine,mapitem.equip[j].attribute, mapitem.equip[j].card[0], mapitem.equip[j].card[1], mapitem.equip[j].card[2], mapitem.equip[j].card[3]);
//		printf("======================================DB=========================================\n");
//		printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//		for(j=1;j<dbeqcount;j++)
//			printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", dbitem.equip[j].flag ,dbitem.equip[j].nameid, dbitem.equip[j].equip, dbitem.equip[j].identify, dbitem.equip[j].amount,dbitem.equip[j].attribute, dbitem.equip[j].card[0], dbitem.equip[j].card[1], dbitem.equip[j].card[2], dbitem.equip[j].card[3]);
//		printf("=================================================================================\n");
//		printf("=================================================Data Time %s===\n", tmpstr);
//		printf("=================================================================================\n");

	}

	//======================================DEBUG==================================================

		//=============================Not Equip ITEM==========================================
	if((noteqcount==1) && (dbnoteqcount==1)){
		//printf("%s Not Equip Empty\n",tablename);
	//item empty
	} else {

		for(i=1;i<noteqcount;i++){
			for(j=1;j<dbnoteqcount;j++){
				if(mapitem.notequip[i].flag==1) break;
				if(!(dbitem.notequip[j].flag==1)){
					if(mapitem.notequip[i].nameid==dbitem.notequip[j].nameid){
						if ((mapitem.notequip[i].amount==dbitem.notequip[j].amount) && (mapitem.notequip[i].equip==dbitem.notequip[j].equip) && (mapitem.notequip[i].identify==dbitem.notequip[j].identify)
						&& (mapitem.notequip[i].attribute==dbitem.notequip[j].attribute))
						{	mapitem.notequip[i].flag=1;
							dbitem.notequip[j].flag=1;
							//printf("the same item : %d ; i : %d ; flag :  %d\n", mapitem.notequip[i].nameid, i, mapitem.notequip[i].flag); //DEBUG-STRING
						}
						else{
							sprintf(tmp_sql,"UPDATE `%s` SET `amount`='%d', `equip`='%d', `identify`='%d',"
							"`attribute`='%d' WHERE `%s`='%d' AND `nameid`='%d'",
							tablename, mapitem.notequip[i].amount, mapitem.notequip[i].equip, mapitem.notequip[i].identify, mapitem.notequip[i].attribute,
							selectoption, char_id, mapitem.notequip[i].nameid);
							//printf("%s",tmp_sql);
							if(mysql_query(&mysql_handle, tmp_sql))
								printf("DB server Error (UPdate `notequ %s`)- %s\n",tablename ,mysql_error(&mysql_handle));

							mapitem.notequip[i].flag=1;
							dbitem.notequip[j].flag=1;
						}
					}
				}
			}
		}

		//printf("dbnoteqcount = %d\n",dbnoteqcount);

		for(i=1;i<dbnoteqcount;i++){
			//printf("dbitem.notequip[i].flag = %d , dbitem.notequip[i].id = %d\n",dbitem.notequip[i].flag,dbitem.notequip[i].id);
			if(!(dbitem.notequip[i].flag==1)){
				sprintf(tmp_sql,"DELETE from `%s` where `id`='%d'", tablename, dbitem.notequip[i].id);
				//printf("%s", tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql))
					printf("DB server Error (DELETE `notequ %s`)- %s\n", tablename ,mysql_error(&mysql_handle));
			}
		}
		for(i=1;i<noteqcount;i++){
			if(!(mapitem.notequip[i].flag==1)){
				sprintf(tmp_sql,"INSERT INTO `%s`( `%s`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)"
				" VALUES ('%d','%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
				tablename ,selectoption , char_id, mapitem.notequip[i].nameid, mapitem.notequip[i].amount, mapitem.notequip[i].equip, mapitem.notequip[i].identify, mapitem.notequip[i].refine,
				mapitem.notequip[i].attribute, mapitem.notequip[i].card[0], mapitem.notequip[i].card[1], mapitem.notequip[i].card[2], mapitem.notequip[i].card[3]);
				//printf("%s", tmp_sql);
				if(mysql_query(&mysql_handle, tmp_sql))
					printf("DB server Error (INSERT `notequ %s`)- %s\n", tablename, mysql_error(&mysql_handle));
			}
		}

		//======================================DEBUG=================================================

//		gettimeofday(&tv,NULL);
//		strftime(tmpstr,24,"%Y-%m-%d %H:%M:%S",localtime(&(tv.tv_sec)));
//		printf("\n\n");
//		printf("Working Table Name : Not EQU %s,  Count : map %3d | db %3d \n",tablename ,noteqcount ,dbnoteqcount);
//		printf("*********************************************************************************\n");
//		printf("======================================MAP===================Char ID %10d===\n",char_id);
//		printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//		for(j=1;j<noteqcount;j++)
//			printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", mapitem.notequip[j].flag,mapitem.notequip[j].nameid, mapitem.notequip[j].equip, mapitem.notequip[j].identify, mapitem.notequip[j].refine,mapitem.notequip[j].attribute, mapitem.notequip[j].card[0], mapitem.notequip[j].card[1], mapitem.notequip[j].card[2], mapitem.notequip[j].card[3]);
//		printf("======================================DB=========================================\n");
//		printf("==flag ===name ===equip===ident===refin===attri===card0===card1===card2===card3==\n");
//		for(j=1;j<dbnoteqcount;j++)
//			printf("| %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d | %5d |\n", dbitem.notequip[j].flag ,dbitem.notequip[j].nameid, dbitem.notequip[j].equip, dbitem.notequip[j].identify, dbitem.notequip[j].refine,dbitem.notequip[j].attribute, dbitem.notequip[j].card[0], dbitem.notequip[j].card[1], dbitem.notequip[j].card[2], dbitem.notequip[j].card[3]);
//		printf("=================================================================================\n");
//		printf("=================================================Data Time %s===\n", tmpstr);
//		printf("=================================================================================\n");
//
	}
	return 0;
}
//=====================================================================================================
int mmo_char_fromsql(int char_id, struct mmo_charstatus *p, int online){
	int i, n;

	memset(p, 0, sizeof(struct mmo_charstatus));

	p->char_id = char_id;
	printf("Loaded: ");
	//`char`( `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, //9
	//`str`,`agi`,`vit`,`int`,`dex`,`luk`, //15
	//`max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, //21
	//`option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`, //27
	//`hair`,`hair_color`,`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, //35
	//`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`)
	//splite 2 parts. cause veeeery long SQL syntax

	sprintf(tmp_sql, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`,"
		"`str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point` FROM `%s` WHERE `char_id` = '%d'",char_db, char_id);

	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
	}

	sql_res = mysql_store_result(&mysql_handle);

	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);

		p->char_id = char_id;
		p->account_id = atoi(sql_row[1]);
		p->char_num = atoi(sql_row[2]);
		strcpy(p->name, sql_row[3]);
		p->class = atoi(sql_row[4]);
		p->base_level = atoi(sql_row[5]);
		p->job_level = atoi(sql_row[6]);
		p->base_exp = atoi(sql_row[7]);
		p->job_exp = atoi(sql_row[8]);
		p->zeny = atoi(sql_row[9]);
		p->str = atoi(sql_row[10]);
		p->agi = atoi(sql_row[11]);
		p->vit = atoi(sql_row[12]);
		p->int_ = atoi(sql_row[13]);
		p->dex = atoi(sql_row[14]);
		p->luk = atoi(sql_row[15]);
		p->max_hp = atoi(sql_row[16]);
		p->hp = atoi(sql_row[17]);
		p->max_sp = atoi(sql_row[18]);
		p->sp = atoi(sql_row[19]);
		p->status_point = atoi(sql_row[20]);
		p->skill_point = atoi(sql_row[21]);
		//free mysql result.
		mysql_free_result(sql_res);
	} else
		printf("char1 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?
	printf("(\033[1;32m%d\033[0m)\033[1;32m%s\033[0m\t[",p->char_id,p->name);
	printf("char1 ");

	sprintf(tmp_sql, "SELECT `option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`,"
		"`clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`,"
		"`last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`, `partner_id` FROM `%s` WHERE `char_id` = '%d'",char_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `char2`)- %s\n", mysql_error(&mysql_handle));
	}

	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		sql_row = mysql_fetch_row(sql_res);


		p->option = atoi(sql_row[0]);	p->karma = atoi(sql_row[1]);	p->manner = atoi(sql_row[2]);
			p->party_id = atoi(sql_row[3]);	p->guild_id = atoi(sql_row[4]);	p->pet_id = atoi(sql_row[5]);

		p->hair = atoi(sql_row[6]);	p->hair_color = atoi(sql_row[7]);	p->clothes_color = atoi(sql_row[8]);
		p->weapon = atoi(sql_row[9]);	p->shield = atoi(sql_row[10]);
		p->head_top = atoi(sql_row[11]);	p->head_mid = atoi(sql_row[12]);	p->head_bottom = atoi(sql_row[13]);
		strcpy(p->last_point.map,sql_row[14]); p->last_point.x = atoi(sql_row[15]);	p->last_point.y = atoi(sql_row[16]);
		strcpy(p->save_point.map,sql_row[17]); p->save_point.x = atoi(sql_row[18]);	p->save_point.y = atoi(sql_row[19]);
		p->partner_id = atoi(sql_row[20]);

		//free mysql result.
		mysql_free_result(sql_res);
	} else
		printf("char2 - failed\n");	//Error?! ERRRRRR WHAT THAT SAY!?

	printf("char2 ");

	//read memo data
	//`memo` (`memo_id`,`char_id`,`map`,`x`,`y`)
	sprintf(tmp_sql, "SELECT `map`,`x`,`y` FROM `%s` WHERE `char_id`='%d'",memo_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `memo`)- %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);

	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			strcpy (p->memo_point[i].map,sql_row[0]);
			p->memo_point[i].x=atoi(sql_row[1]);
			p->memo_point[i].y=atoi(sql_row[2]);
			//i ++;
		}
		mysql_free_result(sql_res);
	}
	printf("memo ");

	//read inventory
	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`"
		"FROM `%s` WHERE `char_id`='%d'",inventory_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `inventory`)- %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			p->inventory[i].id = atoi(sql_row[0]);
			p->inventory[i].nameid = atoi(sql_row[1]);
			p->inventory[i].amount = atoi(sql_row[2]);
			p->inventory[i].equip = atoi(sql_row[3]);
			p->inventory[i].identify = atoi(sql_row[4]);
			p->inventory[i].refine = atoi(sql_row[5]);
			p->inventory[i].attribute = atoi(sql_row[6]);
			p->inventory[i].card[0] = atoi(sql_row[7]);
			p->inventory[i].card[1] = atoi(sql_row[8]);
			p->inventory[i].card[2] = atoi(sql_row[9]);
			p->inventory[i].card[3] = atoi(sql_row[10]);
		}
		mysql_free_result(sql_res);
	}
	printf("inventory ");


	//read cart.
	//`cart_inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf(tmp_sql, "SELECT `id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`"
		"FROM `%s` WHERE `char_id`='%d'",cart_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `cart_inventory`)- %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			p->cart[i].id = atoi(sql_row[0]);
			p->cart[i].nameid = atoi(sql_row[1]);
			p->cart[i].amount = atoi(sql_row[2]);
			p->cart[i].equip = atoi(sql_row[3]);
			p->cart[i].identify = atoi(sql_row[4]);
			p->cart[i].refine = atoi(sql_row[5]);
			p->cart[i].attribute = atoi(sql_row[6]);
			p->cart[i].card[0] = atoi(sql_row[7]);
			p->cart[i].card[1] = atoi(sql_row[8]);
			p->cart[i].card[2] = atoi(sql_row[9]);
			p->cart[i].card[3] = atoi(sql_row[10]);
		}
		mysql_free_result(sql_res);
	}
	printf("cart ");

	//read skill
	//`skill` (`char_id`, `id`, `lv`)
	sprintf(tmp_sql, "SELECT `id`, `lv` FROM `%s` WHERE `char_id`='%d'",skill_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `skill`)- %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			n = atoi(sql_row[0]);
			p->skill[n].id = n; //memory!? shit!.
			p->skill[n].lv = atoi(sql_row[1]);
		}
		mysql_free_result(sql_res);
	}
	printf("skill ");

	//global_reg
	//`global_reg_value` (`char_id`, `str`, `value`)
	sprintf(tmp_sql, "SELECT `str`, `value` FROM `%s` WHERE `type`=3 AND `char_id`='%d'",reg_db, char_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (select `global_reg_value`)- %s\n", mysql_error(&mysql_handle));
	}
	i = 0;
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		for(i=0;(sql_row = mysql_fetch_row(sql_res));i++){
			strcpy (p->global_reg[i].str, sql_row[0]);
			p->global_reg[i].value = atoi (sql_row[1]);
		}
		mysql_free_result(sql_res);
	}
	p->global_reg_num=i;

	if (online) {
		sprintf(tmp_sql, "UPDATE `%s` SET `online`='%d' WHERE `char_id`='%d'",char_db,online,char_id);
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error (set char online)- %s\n", mysql_error(&mysql_handle));
		}
	}

	printf("global_reg]\n");	//ok. all data load successfuly!

	//printf("char cloade");

	return 1;
}
//==========================================================================================================
int mmo_char_sql_init(void) {
	int i;

	printf("init start.......\n");
	// memory initialize
	// no need to set twice size in this routine. but some cause segmentation error. :P
	printf("initializing char memory...(%d byte)\n",sizeof(struct mmo_charstatus)*2);
	CREATE(char_dat, struct mmo_charstatus, 2);

	memset(char_dat, 0, sizeof(struct mmo_charstatus)*2);
/*	Initialized in inter.c already [Wizputer]
	// DB connection initialized
	// for char-server session only
	mysql_init(&mysql_handle);
	printf("Connect DB server....(char server)\n");
	if(!mysql_real_connect(&mysql_handle, char_server_ip, char_server_id, char_server_pw, char_server_db ,char_server_port, (char *)NULL, 0)) {
		// SQL connection pointer check
		printf("%s\n",mysql_error(&mysql_handle));
		exit(1);
	} else {
		printf("connect success! (char server)\n");
	}
*/
	sprintf(tmp_sql , "SELECT count(*) FROM `%s`", char_db);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle) ;
	sql_row = mysql_fetch_row(sql_res);
	printf("total char data -> '%s'.......\n",sql_row[0]);
	i = atoi (sql_row[0]);
	mysql_free_result(sql_res);

	if (i !=0) {
		sprintf(tmp_sql , "SELECT max(`char_id`) FROM `%s`", char_db);
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle));
		}
		sql_res = mysql_store_result(&mysql_handle) ;
		sql_row = mysql_fetch_row(sql_res);
		char_id_count = atoi (sql_row[0]);

		mysql_free_result(sql_res);
	} else
		printf("set char_id_count: %d.......\n",char_id_count);

	sprintf(tmp_sql,"UPDATE `%s` SET `online`='0' WHERE `online`='1'", char_db);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (reset_online `%s`)- %s\n", char_db, mysql_error(&mysql_handle));
	}
	printf("init end.......\n");

	return 0;
}

//==========================================================================================================

int make_new_char_sql(int fd, unsigned char *dat) {
	struct char_session_data *sd;
	char t_name[100];
	int i;
	//aphostropy error check! - fixed!
	jstrescapecpy(t_name, dat);
	printf("making new char -");

	sd = session[fd]->session_data;

	// Check Authorised letters/symbols in the name of the character
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < strlen(dat); i++)
			if (strchr(char_name_letters, dat[i]) == NULL)
				return -1;
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < strlen(dat); i++)
			if (strchr(char_name_letters, dat[i]) != NULL)
				return -1;
	} // else, all letters/symbols are authorised (except control char removed before)

	//check stat error
	if ((dat[24]+dat[25]+dat[26]+dat[27]+dat[28]+dat[29]!=5*6 ) ||
	    (dat[30] >= 9) ||
	    (dat[33] <= 0) || (dat[33] >= 20) ||
	    (dat[31] >= 9)) {

		// check individual stat value
		for(i = 24; i <= 29; i++) {
			if (dat[i] < 1 || dat[i] > 9) {
				return -1;
			}
		}

		// char.log to charlog
		sprintf(tmp_sql,"INSERT INTO `%s` (`time`, `char_msg`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
			"VALUES (NOW(), '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
			charlog_db,"make new char error", sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
		//query
		if (mysql_query(&mysql_handle, tmp_sql)) {
			printf("DB server Error - %s\n", mysql_error(&mysql_handle));
		}
		printf("make new char error %d-%d %s %d, %d, %d, %d, %d, %d %d, %d" RETCODE,
			fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
		return -1;
	}

	// char.log to charlog
	sprintf(tmp_sql,"INSERT INTO `%s`(`time`, `char_msg`,`account_id`,`char_num`,`name`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`hair`,`hair_color`)"
		"VALUES (NOW(), '%s', '%d', '%d', '%s', '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
		charlog_db,"make new char", sd->account_id, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);
	//query
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	printf("make new char %d-%d %s %d, %d, %d, %d, %d, %d - %d, %d" RETCODE,
		fd, dat[30], dat, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29], dat[33], dat[31]);

	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `name` = '%s'",char_db, t_name);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
		return -1;
	}
	sql_res = mysql_store_result(&mysql_handle);
	sql_row = mysql_fetch_row(sql_res);
	printf("\033[1;32m name check result : %s -\033[0m ",sql_row[0]);
	if (atoi(sql_row[0]) > 0) {
		mysql_free_result(sql_res);
		return -1;
	} else
		mysql_free_result(sql_res);

	// check char slot.
	sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `account_id` = '%d' AND `char_num` = '%d'",char_db, sd->account_id, dat[30]);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	sql_row = mysql_fetch_row(sql_res);

	//printf("slot check result : %s\n",sql_row[0]);
	if (atoi(sql_row[0]) > 0) {
		mysql_free_result(sql_res);
		return -1;
	} else
		mysql_free_result(sql_res);

	char_id_count++;

	// make new char.
	sprintf(tmp_sql,"INSERT INTO `%s` (`char_id`,`account_id`,`char_num`,`name`,`zeny`,`str`,`agi`,`vit`,`int`,`dex`,`luk`,`max_hp`,`hp`,`max_sp`,`sp`,`hair`,`hair_color`)"
		" VALUES ('%d', '%d', '%d', '%s', '%d',  '%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d','%d', '%d','%d', '%d')",
		 char_db, char_id_count, sd->account_id , dat[30] , t_name, start_zeny, dat[24], dat[25], dat[26], dat[27], dat[28], dat[29],
		(40 * (100 + dat[26])/100) , (40 * (100 + dat[26])/100 ),  (11 * (100 + dat[27])/100), (11 * (100 + dat[27])/100), dat[33], dat[31]);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `char`)- %s\n", mysql_error(&mysql_handle));
	}

	//`inventory` (`id`,`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`, `card0`, `card1`, `card2`, `card3`)
	sprintf(tmp_sql,"INSERT INTO `%s` (`char_id`,`nameid`, `amount`, `equip`, `identify`) VALUES ('%d', '%d', '%d', '%d', '%d')",
		inventory_db, char_id_count, 1201,1,0x02,1); //add Knife
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `inventory`)- %s\n", mysql_error(&mysql_handle));
	}

	sprintf(tmp_sql,"INSERT INTO `%s` (`char_id`,`nameid`, `amount`, `equip`, `identify`) VALUES ('%d', '%d', '%d', '%d', '%d')",
		inventory_db, char_id_count, 2301,1,0x10,1); //add Cotton Shirt
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `inventory`)- %s\n", mysql_error(&mysql_handle));
	}
	// respawn map and start point set
	sprintf(tmp_sql,"UPDATE `%s` SET `last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d'  WHERE  `char_id` = '%d'",
		char_db, start_point.map,start_point.x,start_point.y, start_point.map,start_point.x,start_point.y, char_id_count);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (update `char`)- %s\n", mysql_error(&mysql_handle));
	}
	printf("making new char success - id:(\033[1;32m%d\033[0m\tname:\033[1;32%s\033[0m\n", char_id_count, t_name);
	return char_id_count;
}

//==========================================================================================================

void mmo_char_sync(void){
	printf("mmo_char_sync() - nothing to do\n");
}

// to do
///////////////////////////

int mmo_char_sync_timer(int tid, unsigned int tick, int id, int data) {
	printf("mmo_char_sync_timer() tic - no works to do\n");
	return 0;
}

int count_users(void) {
	int i, users;

	if (login_fd > 0 && session[login_fd]){
		users = 0;
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (server_fd[i] >= 0) {
				users += server[i].users;
			}
		}
		return users;
	}
	return 0;
}

int mmo_char_send006b(int fd, struct char_session_data *sd) {
	int i, j, found_num = 0;
	struct mmo_charstatus *p = NULL;
// hehe. commented other. anyway there's no need to use older version.
// if use older packet version just uncomment that!
//#ifdef NEW_006b
	const int offset = 24;
//#else
//	int offset = 4;
//#endif

	printf("mmo_char_send006b start.. (account:%d)\n",sd->account_id);
//	printf("offset -> %d...\n",offset);

	//search char.
	sprintf(tmp_sql, "SELECT `char_id` FROM `%s` WHERE `account_id` = '%d'",char_db, sd->account_id);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error - %s\n", mysql_error(&mysql_handle));
	}
	sql_res = mysql_store_result(&mysql_handle);
	if (sql_res) {
		found_num = mysql_num_rows(sql_res);
		printf("number of chars: %d\n", found_num);
		i = 0;
		while((sql_row = mysql_fetch_row(sql_res))) {
			sd->found_char[i] = atoi(sql_row[0]);
			i++;
		}
		mysql_free_result(sql_res);
	}

//	printf("char fetching end (total: %d)....\n", found_num);

	for(i = found_num; i < 9; i++)
		sd->found_char[i] = -1;

	memset(WFIFOP(fd, 0), 0, offset + found_num * 106);
	WFIFOW(fd, 0) = 0x6b;
	WFIFOW(fd, 2) = offset + found_num * 106;

	printf("(\033[1;13m%d\033[0m) Request Char Data:\n",sd->account_id);

	for(i = 0; i < found_num; i++) {
		mmo_char_fromsql(sd->found_char[i], char_dat, 0);

		p = &char_dat[0];

		j = offset + (i * 106); // increase speed of code

		WFIFOL(fd,j) = p->char_id;
		WFIFOL(fd,j+4) = p->base_exp;
		WFIFOL(fd,j+8) = p->zeny;
		WFIFOL(fd,j+12) = p->job_exp;
		WFIFOL(fd,j+16) = p->job_level;

		WFIFOL(fd,j+20) = 0;
		WFIFOL(fd,j+24) = 0;
		WFIFOL(fd,j+28) = p->option;

		WFIFOL(fd,j+32) = p->karma;
		WFIFOL(fd,j+36) = p->manner;

		WFIFOW(fd,j+40) = p->status_point;
		WFIFOW(fd,j+42) = (p->hp > 0x7fff) ? 0x7fff : p->hp;
		WFIFOW(fd,j+44) = (p->max_hp > 0x7fff) ? 0x7fff : p->max_hp;
		WFIFOW(fd,j+46) = (p->sp > 0x7fff) ? 0x7fff : p->sp;
		WFIFOW(fd,j+48) = (p->max_sp > 0x7fff) ? 0x7fff : p->max_sp;
		WFIFOW(fd,j+50) = DEFAULT_WALK_SPEED; // p->speed;
		WFIFOW(fd,j+52) = p->class;
		WFIFOW(fd,j+54) = p->hair;
		WFIFOW(fd,j+56) = p->weapon;
		WFIFOW(fd,j+58) = p->base_level;
		WFIFOW(fd,j+60) = p->skill_point;
		WFIFOW(fd,j+62) = p->head_bottom;
		WFIFOW(fd,j+64) = p->shield;
		WFIFOW(fd,j+66) = p->head_top;
		WFIFOW(fd,j+68) = p->head_mid;
		WFIFOW(fd,j+70) = p->hair_color;
		WFIFOW(fd,j+72) = p->clothes_color;

		memcpy(WFIFOP(fd,j+74), p->name, 24);

		WFIFOB(fd,j+98) = (p->str > 255) ? 255 : p->str;
		WFIFOB(fd,j+99) = (p->agi > 255) ? 255 : p->agi;
		WFIFOB(fd,j+100) = (p->vit > 255) ? 255 : p->vit;
		WFIFOB(fd,j+101) = (p->int_ > 255) ? 255 : p->int_;
		WFIFOB(fd,j+102) = (p->dex > 255) ? 255 : p->dex;
		WFIFOB(fd,j+103) = (p->luk > 255) ? 255 : p->luk;
		WFIFOB(fd,j+104) = p->char_num;
	}

	WFIFOSET(fd,WFIFOW(fd,2));
//	printf("mmo_char_send006b end..\n");
	return 0;
}

int parse_tologin(int fd) {
	int i;
	struct char_session_data *sd;

	// only login-server can have an access to here.
	// so, if it isn't the login-server, we disconnect the session.
	//session eof check!
	if(fd != login_fd)
		session[fd]->eof = 1;
	if(session[fd]->eof) {
		if (fd == login_fd) {
			printf("Char-server can't connect to login-server (connection #%d).\n", fd);
			login_fd = -1;
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	sd = session[fd]->session_data;

	// hehe. no need to set user limite on SQL version. :P
	// but char limitation is good way to maintain server. :D
	while(RFIFOREST(fd) >= 2) {
//		printf("parse_tologin : %d %d %x\n", fd, RFIFOREST(fd), RFIFOW(fd, 0));

		switch(RFIFOW(fd, 0)){
		case 0x2711:
			if (RFIFOREST(fd) < 3)
				return 0;
			if (RFIFOB(fd, 2)) {
				//printf("connect login server error : %d\n", RFIFOB(fd, 2));
				printf("Can not connect to login-server.\n");
				printf("The server communication passwords (default s1/p1) is probably invalid.\n");
				printf("Also, please make sure your login db has the username/password present and the sex of the account is S.\n");
				printf("If you changed the communication passwords, change them back at map_athena.conf and char_athena.conf\n");
				return 0;
				//exit(1); //fixed for server shutdown.
			}else {
				printf("Connected to login-server (connection #%d).\n", fd);
				// if no map-server already connected, display a message...
				for(i = 0; i < MAX_MAP_SERVERS; i++)
					if (server_fd[i] >= 0 && server[i].map[0][0]) // if map-server online and at least 1 map
						break;
				if (i == MAX_MAP_SERVERS)
					printf("Awaiting maps from map-server.\n");
			}
			RFIFOSKIP(fd, 3);
			break;

		case 0x2713:
			if(RFIFOREST(fd)<51)
				return 0;
			for(i = 0; i < fd_max; i++) {
				if (session[i] && (sd = session[i]->session_data) && sd->account_id == RFIFOL(fd,2)) {
					if (RFIFOB(fd,6) != 0) {
						WFIFOW(i,0) = 0x6c;
						WFIFOB(i,2) = 0x42;
						WFIFOSET(i,3);
					} else if (max_connect_user == 0 || count_users() < max_connect_user) {
//						if (max_connect_user == 0)
//							printf("max_connect_user (unlimited) -> accepted.\n");
//						else
//							printf("count_users(): %d < max_connect_user (%d) -> accepted.\n", count_users(), max_connect_user);
						sd->connect_until_time = (time_t)RFIFOL(fd,47);
						// send characters to player
						mmo_char_send006b(i, sd);
					} else {
						// refuse connection: too much online players
//						printf("count_users(): %d < max_connect_use (%d) -> fail...\n", count_users(), max_connect_user);
						WFIFOW(i,0) = 0x6c;
						WFIFOW(i,2) = 0;
						WFIFOSET(i,3);
					}
				}
			}
			RFIFOSKIP(fd,51);
			break;

		case 0x2717:
			if (RFIFOREST(fd) < 50)
				return 0;
			for(i = 0; i < fd_max; i++) {
				if (session[i] && (sd = session[i]->session_data)) {
					if (sd->account_id == RFIFOL(fd,2)) {
					sd->connect_until_time = (time_t)RFIFOL(fd,46);
					break;
					}
				}
			}
			RFIFOSKIP(fd,50);
			break;

/*		case 0x2721:	// gm reply. I don't want to support this function.
			printf("0x2721:GM reply\n");
		  {
			int oldacc, newacc;
			unsigned char buf[64];
			if (RFIFOREST(fd) < 10)
				return 0;
			oldacc = RFIFOL(fd, 2);
			newacc = RFIFOL(fd, 6);
			RFIFOSKIP(fd, 10);
			if (newacc > 0) {
				for(i=0;i<char_num;i++){
					if(char_dat[i].account_id==oldacc)
						char_dat[i].account_id=newacc;
				}
			}
			WBUFW(buf,0)=0x2b0b;
			WBUFL(buf,2)=oldacc;
			WBUFL(buf,6)=newacc;
			mapif_sendall(buf,10);
//			printf("char -> map\n");
		  }
			break;
*/
		case 0x2723:	// changesex reply (modified by [Yor])
			if (RFIFOREST(fd) < 7)
				return 0;
		  {
			int acc, sex;
			unsigned char buf[16];

			acc = RFIFOL(fd,2);
			sex = RFIFOB(fd,6);
			RFIFOSKIP(fd, 7);
			if (acc > 0) {
				sprintf(tmp_sql, "SELECT `char_id`,`class`,`skill_point` FROM `%s` WHERE `account_id` = '%d'",char_db, acc);
				if (mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
				}
				sql_res = mysql_store_result(&mysql_handle);

				if (sql_res) {
						int char_id, jobclass, skill_point, class;
						sql_row = mysql_fetch_row(sql_res);
						char_id = atoi(sql_row[0]);
						jobclass = atoi(sql_row[1]);
						skill_point = atoi(sql_row[2]);
						class = jobclass;
						if (jobclass == 19 || jobclass == 20 ||
						    jobclass == 4020 || jobclass == 4021 ||
						    jobclass == 4042 || jobclass == 4043) {
							// job modification
							if (jobclass == 19 || jobclass == 20) {
								class = (sex) ? 19 : 20;
							} else if (jobclass == 4020 || jobclass == 4021) {
								class = (sex) ? 4020 : 4021;
							} else if (jobclass == 4042 || jobclass == 4043) {
								class = (sex) ? 4042 : 4043;
							}
							// remove specifical skills of classes 19,20 4020,4021 and 4042,4043
							sprintf(tmp_sql, "SELECT `lv` FROM `%s` WHERE `char_id` = '%d' AND `id` >= '315' AND `id` <= '330'",skill_db, char_id);
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
							}
							sql_res = mysql_store_result(&mysql_handle);
							if (sql_res) {
								while(( sql_row = mysql_fetch_row(sql_res))) {
									skill_point += atoi(sql_row[0]);
								}
							}
							sprintf(tmp_sql, "DELETE FROM `%s` WHERE `char_id` = '%d' AND `id` >= '315' AND `id` <= '330'",skill_db, char_id);
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
							}
						}
						// to avoid any problem with equipment and invalid sex, equipment is unequiped.
						sprintf(tmp_sql, "UPDATE `%s` SET `equip` = '0' WHERE `char_id` = '%d'",inventory_db, char_id);
						if (mysql_query(&mysql_handle, tmp_sql)) {
							printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
						}
						sprintf(tmp_sql, "UPDATE `%s` SET `class`='%d' , `skill_point`='%d' , `weapon`='0' , `shield='0' , `head_top`='0' , `head_mid`='0' , `head_bottom`='0' WHERE `char_id` = '%d'",char_db, class, skill_point, char_id);
						if (mysql_query(&mysql_handle, tmp_sql)) {
							printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
						}
					}
				}
				// disconnect player if online on char-server
				for(i = 0; i < fd_max; i++) {
					if (session[i] && (sd = session[i]->session_data)) {
						if (sd->account_id == acc) {
							session[i]->eof = 1;
							break;
						}
					}
				}

			WBUFW(buf,0) = 0x2b0d;
			WBUFL(buf,2) = acc;
			WBUFB(buf,6) = sex;

			mapif_sendall(buf, 7);
		  }
			break;

		// account_reg2変更通知
		case 0x2729:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			struct global_reg reg[ACCOUNT_REG2_NUM];
			unsigned char buf[4096];
			int j, p, acc;
			acc = RFIFOL(fd,4);
			for(p = 8, j = 0; p < RFIFOW(fd,2) && j < ACCOUNT_REG2_NUM; p += 36, j++) {
				memcpy(reg[j].str, RFIFOP(fd,p), 32);
				reg[j].value = RFIFOL(fd,p+32);
			}
			// set_account_reg2(acc,j,reg);
			// 同垢ログインを禁止していれば送る必要は無い
			memcpy(buf,RFIFOP(fd,0), RFIFOW(fd,2));
			WBUFW(buf,0) = 0x2b11;
			mapif_sendall(buf, WBUFW(buf,2));
			RFIFOSKIP(fd, RFIFOW(fd,2));
//			printf("char: save_account_reg_reply\n");
		  }
			break;

		// State change of account/ban notification (from login-server) by [Yor]
		case 0x2731:
			if (RFIFOREST(fd) < 11)
				return 0;
			// send to all map-servers to disconnect the player
		  {
			unsigned char buf[16];
			WBUFW(buf,0) = 0x2b14;
			WBUFL(buf,2) = RFIFOL(fd,2);
			WBUFB(buf,6) = RFIFOB(fd,6); // 0: change of statut, 1: ban
			WBUFL(buf,7) = RFIFOL(fd,7); // status or final date of a banishment
			mapif_sendall(buf, 11);
		  }
			// disconnect player if online on char-server
			for(i = 0; i < fd_max; i++) {
				if (session[i] && (sd = session[i]->session_data)) {
					if (sd->account_id == RFIFOL(fd,2)) {
						session[i]->eof = 1;
						break;
					}
				}
			}
			RFIFOSKIP(fd,11);
			break;

		default:
			printf("set eof.\n");
			session[fd]->eof = 1;
			return 0;
		}
	}

	RFIFOFLUSH(fd);

	return 0;
}

//--------------------------------
// Map-server anti-freeze system
//--------------------------------
int map_anti_freeze_system(int tid, unsigned int tick, int id, int data) {
	int i;

	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		if (server_fd[i] >= 0) {// if map-server is online
			printf("map_anti_freeze_system: server #%d, flag: %d.\n", i, server_freezeflag[i]);
			if (server_freezeflag[i]-- < 1) {// Map-server anti-freeze system. Counter. 5 ok, 4...0 freezed
				printf("Map-server anti-freeze system: char-server #%d is frozen -> disconnection.\n", i);
				session[server_fd[i]]->eof = 1;
				sprintf(tmp_sql, "DELETE FROM `ragsrvinfo` WHERE `index`='%d'", server_fd[i]);
				if (mysql_query(&mysql_handle, tmp_sql)) {
				    printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}
			}
		}
	}

	return 0;
}

int parse_frommap(int fd) {
	int i = 0, j = 0;
	int id;

	// Sometimes fd=0, and it will cause server crash. Don't know why. :(
	if (fd <= 0) {
		printf("parse_frommap error fd=0\n");
		return 0;
	}

	for(id = 0; id < MAX_MAP_SERVERS; id++)
		if (server_fd[id] == fd)
			break;
	if(id == MAX_MAP_SERVERS)
		session[fd]->eof = 1;
	if(session[fd]->eof) {
		if (id < MAX_MAP_SERVERS) {
			memset(&server[id], 0, sizeof(struct mmo_map_server));
			printf("Map-server %d (session #%d) has disconnected.\n", id, fd);
			sprintf(tmp_sql, "DELETE FROM `ragsrvinfo` WHERE `index`='%d'", server_fd[id]);
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			server_fd[id] = -1;
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	while(RFIFOREST(fd) >= 2) {
//		printf("parse_frommap : %d %d %x\n", fd, RFIFOREST(fd), RFIFOW(fd,0));

		switch(RFIFOW(fd, 0)) {
		case 0x2af7:
			RFIFOSKIP(fd,2);
			read_gm_account();
			break;

		// mapserver -> map names recv.
		case 0x2afa:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			memset(server[id].map, 0, sizeof(server[id].map));
			j = 0;
			for(i = 4; i < RFIFOW(fd,2); i += 16) {
				memcpy(server[id].map[j], RFIFOP(fd,i), 16);
//				printf("set map %d.%d : %s\n", id, j, server[id].map[j]);
				j++;
			}
			i = server[id].ip;
			{
				unsigned char *p = (unsigned char *)&server[id].ip;
				printf("Map-Server %d connected: %d maps, from IP %d.%d.%d.%d port %d.\n",
				       id, j, p[0], p[1], p[2], p[3], server[id].port);
				printf("Map-server %d loading complete.\n", id);
			}
			WFIFOW(fd,0) = 0x2afb;
			WFIFOB(fd,2) = 0;
			memcpy(WFIFOP(fd,3), wisp_server_name, 24); // name for wisp to player
			WFIFOSET(fd,27);
			{
				unsigned char buf[16384];
				int x;
				if (j == 0) {
					printf("WARNING: Map-Server %d have NO maps.\n", id);
				// Transmitting maps information to the other map-servers
				} else {
					WBUFW(buf,0) = 0x2b04;
					WBUFW(buf,2) = j * 16 + 10;
					WBUFL(buf,4) = server[id].ip;
					WBUFW(buf,8) = server[id].port;
					memcpy(WBUFP(buf,10), RFIFOP(fd,4), j * 16);
					mapif_sendallwos(fd, buf, WBUFW(buf,2));
				}
				// Transmitting the maps of the other map-servers to the new map-server
				for(x = 0; x < MAX_MAP_SERVERS; x++) {
					if (server_fd[x] >= 0 && x != id) {
						WFIFOW(fd,0) = 0x2b04;
						WFIFOL(fd,4) = server[x].ip;
						WFIFOW(fd,8) = server[x].port;
						j = 0;
						for(i = 0; i < MAX_MAP_PER_SERVER; i++)
							if (server[x].map[i][0])
								memcpy(WFIFOP(fd,10+(j++)*16), server[x].map[i], 16);
						if (j > 0) {
							WFIFOW(fd,2) = j * 16 + 10;
							WFIFOSET(fd,WFIFOW(fd,2));
						}
					}
				}
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// auth request
		case 0x2afc:
			if (RFIFOREST(fd) < 22)
				return 0;
//			printf("(AUTH request) auth_fifo search %d %d %d\n", RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
			for(i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == RFIFOL(fd,2) &&
				    auth_fifo[i].char_id == RFIFOL(fd,6) &&
				    auth_fifo[i].login_id1 == RFIFOL(fd,10) &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				    // here, it's the only area where it's possible that we doesn't know login_id2 (map-server asks just after 0x72 packet, that doesn't given the value)
				    (auth_fifo[i].login_id2 == RFIFOL(fd,14) || RFIFOL(fd,14) == 0) && // relate to the versions higher than 18
#endif
				    (!check_ip_flag || auth_fifo[i].ip == RFIFOL(fd,18)) &&
				    !auth_fifo[i].delflag) {
					auth_fifo[i].delflag = 1;
					WFIFOW(fd,0) = 0x2afd;
					WFIFOW(fd,2) = 16 + sizeof(struct mmo_charstatus);
					WFIFOL(fd,4) = RFIFOL(fd,2);
					WFIFOL(fd,8) = auth_fifo[i].login_id2;
					WFIFOL(fd,12) = (unsigned long)auth_fifo[i].connect_until_time;
					mmo_char_fromsql(auth_fifo[i].char_id, char_dat, 1);
					char_dat[0].sex = auth_fifo[i].sex;
					memcpy(WFIFOP(fd,16), &char_dat[0], sizeof(struct mmo_charstatus));
					WFIFOSET(fd, WFIFOW(fd,2));
					//printf("auth_fifo search success (auth #%d, account %d, character: %d).\n", i, RFIFOL(fd,2), RFIFOL(fd,6));
					break;
				}
			}
			if (i == AUTH_FIFO_SIZE) {
				WFIFOW(fd,0) = 0x2afe;
				WFIFOL(fd,2) = RFIFOL(fd,2);
				WFIFOSET(fd,6);
//				printf("(AUTH request) auth_fifo search error!\n");
			}
			RFIFOSKIP(fd,22);
			break;

		// set MAP user
		case 0x2aff:
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			if (RFIFOW(fd,4) != server[id].users)
				printf("map user: %d\n", RFIFOW(fd,4));
			server[id].users = RFIFOW(fd,4);
			if(anti_freeze_enable)
				server_freezeflag[id] = 5; // Map anti-freeze system. Counter. 5 ok, 4...0 freezed
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// char saving
		case 0x2b01:
			i = 0;
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
			//check account
			sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'",char_db, RFIFOL(fd,4),RFIFOL(fd,8));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			sql_res = mysql_store_result(&mysql_handle);
			if (sql_res) {
				sql_row = mysql_fetch_row(sql_res);
				if (sql_row)
					i = atoi(sql_row[0]);
			}
			mysql_free_result(sql_res);

			if (i == 1) {
				memcpy(&char_dat[0], RFIFOP(fd,12), sizeof(struct mmo_charstatus));
				mmo_char_tosql(RFIFOL(fd,8), char_dat);
				//save to DB
			}
			RFIFOSKIP(fd,RFIFOW(fd,2));
			break;

		// req char selection
		case 0x2b02:
			if (RFIFOREST(fd) < 18)
				return 0;

			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;

//			printf("(charselect) auth_fifo set %d - account_id:%08x login_id1:%08x\n", auth_fifo_pos, RFIFOL(fd, 2), RFIFOL(fd, 6));
			auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd, 2);
			auth_fifo[auth_fifo_pos].char_id = 0;
			auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd, 6);
			auth_fifo[auth_fifo_pos].login_id2 = RFIFOL(fd,10);
			auth_fifo[auth_fifo_pos].delflag = 2;
			auth_fifo[auth_fifo_pos].char_pos = 0;
			auth_fifo[auth_fifo_pos].connect_until_time = 0; // unlimited/unknown time by default (not display in map-server)
			auth_fifo[auth_fifo_pos].ip = RFIFOL(fd,14);
			auth_fifo_pos++;

			WFIFOW(fd, 0) = 0x2b03;
			WFIFOL(fd, 2) = RFIFOL(fd, 2);
			WFIFOB(fd, 6) = 0;
			WFIFOSET(fd, 7);

			RFIFOSKIP(fd, 18);
			break;

		// request "change map server"
		case 0x2b05:
			if (RFIFOREST(fd) < 49)
				return 0;

			if (auth_fifo_pos >= AUTH_FIFO_SIZE)
				auth_fifo_pos = 0;

			WFIFOW(fd, 0) = 0x2b06;
			memcpy(WFIFOP(fd,2), RFIFOP(fd,2), 42);
//			printf("(map change) auth_fifo set %d - account_id:%08x login_id1:%08x\n", auth_fifo_pos, RFIFOL(fd, 2), RFIFOL(fd, 6));
			auth_fifo[auth_fifo_pos].account_id = RFIFOL(fd, 2);
			auth_fifo[auth_fifo_pos].login_id1 = RFIFOL(fd, 6);
			auth_fifo[auth_fifo_pos].login_id2 = RFIFOL(fd,10);
			auth_fifo[auth_fifo_pos].char_id = RFIFOL(fd,14);
			auth_fifo[auth_fifo_pos].delflag = 0;
			auth_fifo[auth_fifo_pos].sex = RFIFOB(fd,44);
			auth_fifo[auth_fifo_pos].ip = RFIFOL(fd,45);

			sprintf(tmp_sql, "SELECT count(*) FROM `%s` WHERE `account_id` = '%d' AND `char_id`='%d'", char_db, RFIFOL(fd,2), RFIFOL(fd,14));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			sql_res = mysql_store_result(&mysql_handle);

			if (( sql_row = mysql_fetch_row(sql_res))) {
				i = atoi(sql_row[0]);
				mysql_free_result(sql_res);

				auth_fifo[auth_fifo_pos].char_pos = auth_fifo[auth_fifo_pos].char_id;
				auth_fifo_pos++;

				WFIFOL(fd,6) = 0;
				break;
			}
			if (i == 0)
				WFIFOW(fd,6) = 1;

			WFIFOSET(fd,44);
			RFIFOSKIP(fd,49);
			break;

		// char name check
		case 0x2b08:
			if (RFIFOREST(fd) < 6)
				return 0;

			sprintf(tmp_sql, "SELECT `name` FROM `%s` WHERE `char_id`='%d'", char_db, RFIFOL(fd,2));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			sql_res = mysql_store_result(&mysql_handle);

			sql_row = mysql_fetch_row(sql_res);

			WFIFOW(fd,0) = 0x2b09;
			WFIFOL(fd,2) = RFIFOL(fd,2);

			if (sql_row)
				memcpy(WFIFOP(fd,6), sql_row[0], 24);
			else
				memcpy(WFIFOP(fd,6), unknown_char_name, 24);
			mysql_free_result(sql_res);

			WFIFOSET(fd,30);

			RFIFOSKIP(fd,6);
			break;

/*		// I want become GM - fuck!
		case 0x2b0a:
			if(RFIFOREST(fd)<4)
				return 0;
			if(RFIFOREST(fd)<RFIFOW(fd,2))
				return 0;
			memcpy(WFIFOP(login_fd,2),RFIFOP(fd,2),RFIFOW(fd,2)-2);
			WFIFOW(login_fd,0)=0x2720;
			WFIFOSET(login_fd,RFIFOW(fd,2));
//			printf("char : change gm -> login %d %s %d\n", RFIFOL(fd, 4), RFIFOP(fd, 8), RFIFOW(fd, 2));
			RFIFOSKIP(fd, RFIFOW(fd, 2));
			break;
		*/

		// account_reg保存要求
		case 0x2b10:
			if (RFIFOREST(fd) < 4 || RFIFOREST(fd) < RFIFOW(fd,2))
				return 0;
		  {
			struct global_reg reg[ACCOUNT_REG2_NUM];
			int j,p,acc;
			acc=RFIFOL(fd,4);
			for(p=8,j=0;p<RFIFOW(fd,2) && j<ACCOUNT_REG2_NUM;p+=36,j++){
				memcpy(reg[j].str,RFIFOP(fd,p),32);
				reg[j].value=RFIFOL(fd,p+32);
			}
			// set_account_reg2(acc,j,reg);
			// loginサーバーへ送る
			if (login_fd > 0) { // don't send request if no login-server
				WFIFOW(login_fd, 0) = 0x2728;
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), RFIFOW(fd,2));
				WFIFOSET(login_fd, WFIFOW(login_fd,2));
			}
			// ワールドへの同垢ログインがなければmapサーバーに送る必要はない
			//memcpy(buf,RFIFOP(fd,0),RFIFOW(fd,2));
			//WBUFW(buf,0)=0x2b11;
			//mapif_sendall(buf,WBUFW(buf,2));
			RFIFOSKIP(fd,RFIFOW(fd,2));
//			printf("char: save_account_reg (from map)\n");
		  }
			break;

		// Map server send information to change an email of an account -> login-server
		case 0x2b0c:
			if (RFIFOREST(fd) < 86)
				return 0;
			if (login_fd > 0) { // don't send request if no login-server
				memcpy(WFIFOP(login_fd,0), RFIFOP(fd,0), 86); // 0x2722 <account_id>.L <actual_e-mail>.40B <new_e-mail>.40B
				WFIFOW(login_fd,0) = 0x2722;
				WFIFOSET(login_fd, 86);
			}
			RFIFOSKIP(fd, 86);
			break;

		// Receiving from map-server a status change resquest. Transmission to login-server (by Yor)
		case 0x2b0e:
			if (RFIFOREST(fd) < 44)
				return 0;
		  {
			char character_name[24];
			int acc = RFIFOL(fd,2); // account_id of who ask (-1 if nobody)
			memcpy(character_name, RFIFOP(fd,6), 24);
			character_name[sizeof(character_name) -1] = '\0';
			// prepare answer
			WFIFOW(fd,0) = 0x2b0f; // answer
			WFIFOL(fd,2) = acc; // who want do operation
			WFIFOW(fd,30) = RFIFOW(fd, 30); // type of operation: 1-block, 2-ban, 3-unblock, 4-unban
			sprintf(tmp_sql, "SELECT `account_id`,`name` FROM `%s` WHERE `name` = '%s'",char_db, character_name);
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error (select `char`)- %s\n", mysql_error(&mysql_handle));
			}

			sql_res = mysql_store_result(&mysql_handle);

			if (sql_res) {
				if (mysql_num_rows(sql_res)) {
					sql_row = mysql_fetch_row(sql_res);
					memcpy(WFIFOP(fd,6), sql_row[1], 24); // put correct name if found
					WFIFOW(fd,32) = 0; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
					switch(RFIFOW(fd, 30)) {
					case 1: // block
						if (acc == -1 || isGM(acc) >= isGM(atoi(sql_row[0]))) {
							if (login_fd > 0) { // don't send request if no login-server
								WFIFOW(login_fd,0) = 0x2724;
								WFIFOL(login_fd,2) = atoi(sql_row[0]); // account value
								WFIFOL(login_fd,6) = 5; // status of the account
								WFIFOSET(login_fd, 10);
//								printf("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 5);
							} else
								WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						} else
							WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						break;
					case 2: // ban
						if (acc == -1 || isGM(acc) >= isGM(atoi(sql_row[0]))) {
							if (login_fd > 0) { // don't send request if no login-server
								WFIFOW(login_fd, 0) = 0x2725;
								WFIFOL(login_fd, 2) = atoi(sql_row[0]); // account value
								WFIFOW(login_fd, 6) = RFIFOW(fd,32); // year
								WFIFOW(login_fd, 8) = RFIFOW(fd,34); // month
								WFIFOW(login_fd,10) = RFIFOW(fd,36); // day
								WFIFOW(login_fd,12) = RFIFOW(fd,38); // hour
								WFIFOW(login_fd,14) = RFIFOW(fd,40); // minute
								WFIFOW(login_fd,16) = RFIFOW(fd,42); // second
								WFIFOSET(login_fd,18);
//								printf("char : status -> login: account %d, ban: %dy %dm %dd %dh %dmn %ds\n",
//								       char_dat[i].account_id, (short)RFIFOW(fd,32), (short)RFIFOW(fd,34), (short)RFIFOW(fd,36), (short)RFIFOW(fd,38), (short)RFIFOW(fd,40), (short)RFIFOW(fd,42));
							} else
								WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						} else
							WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						break;
					case 3: // unblock
						if (acc == -1 || isGM(acc) >= isGM(atoi(sql_row[0]))) {
							if (login_fd > 0) { // don't send request if no login-server
								WFIFOW(login_fd,0) = 0x2724;
								WFIFOL(login_fd,2) = atoi(sql_row[0]); // account value
								WFIFOL(login_fd,6) = 0; // status of the account
								WFIFOSET(login_fd, 10);
//								printf("char : status -> login: account %d, status: %d \n", char_dat[i].account_id, 0);
							} else
								WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						} else
							WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						break;
					case 4: // unban
						if (acc == -1 || isGM(acc) >= isGM(atoi(sql_row[0]))) {
							if (login_fd > 0) { // don't send request if no login-server
								WFIFOW(login_fd, 0) = 0x272a;
								WFIFOL(login_fd, 2) = atoi(sql_row[0]); // account value
								WFIFOSET(login_fd, 6);
//								printf("char : status -> login: account %d, unban request\n", char_dat[i].account_id);
							} else
								WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						} else
							WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						break;
					case 5: // changesex
						if (acc == -1 || isGM(acc) >= isGM(atoi(sql_row[0]))) {
							if (login_fd > 0) { // don't send request if no login-server
								WFIFOW(login_fd, 0) = 0x2727;
								WFIFOL(login_fd, 2) = atoi(sql_row[0]); // account value
								WFIFOSET(login_fd, 6);
//								printf("char : status -> login: account %d, change sex request\n", char_dat[i].account_id);
							} else
								WFIFOW(fd,32) = 3; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						} else
							WFIFOW(fd,32) = 2; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
						break;
					}
				} else {
					// character name not found
					memcpy(WFIFOP(fd,6), character_name, 24);
					WFIFOW(fd,32) = 1; // answer: 0-login-server resquest done, 1-player not found, 2-gm level too low, 3-login-server offline
				}
				// send answer if a player ask, not if the server ask
				if (acc != -1) {
					WFIFOSET(fd, 34);
				}
			}
		  }
			RFIFOSKIP(fd, 44);
			break;

		// Recieve rates [Wizputer]
		case 0x2b16:
			if (RFIFOREST(fd) < 6 || RFIFOREST(fd) < RFIFOW(fd,8))
				return 0;
			sprintf(tmp_sql, "INSERT INTO `ragsrvinfo` SET `index`='%d',`name`='%s',`exp`='%d',`jexp`='%d',`drop`='%d',`motd`='%s'",
			        fd, server_name, RFIFOW(fd,2), RFIFOW(fd,4), RFIFOW(fd,6), RFIFOP(fd,10));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			RFIFOSKIP(fd,RFIFOW(fd,8));
			break;

		// Character disconnected set online 0 [Wizputer]
		case 0x2b17:
			if (RFIFOREST(fd) < 6 )
				return 0;
			//printf("Setting %d char offline\n",RFIFOL(fd,2));
			sprintf(tmp_sql,"UPDATE `%s` SET `online`='0' WHERE `char_id`='%d'", char_db, RFIFOL(fd,2));
			if (mysql_query(&mysql_handle, tmp_sql))
				printf("DB server Error (update online `%s`)- %s\n", char_db, mysql_error(&mysql_handle));
			RFIFOSKIP(fd,6);
			break;
		default:
			// inter server - packet
			{
				int r = inter_parse_frommap(fd);
				if (r == 1) break;		// processed
				if (r == 2) return 0;	// need more packet
			}

			// no inter server packet. no char server packet -> disconnect
			printf("parse_frommap: unknown packet %x! \n", RFIFOW(fd,0));
			session[fd]->eof = 1;
			return 0;
		}
	}
	return 0;
}

int search_mapserver(char *map) {
	int i, j;
	char temp_map[16];
	int temp_map_len;

//	printf("Searching the map-server for map '%s'... ", map);
	strncpy(temp_map, map, sizeof(temp_map));
	temp_map[sizeof(temp_map)-1] = '\0';
	if (strchr(temp_map, '.') != NULL)
		temp_map[strchr(temp_map, '.') - temp_map + 1] = '\0'; // suppress the '.gat', but conserve the '.' to be sure of the name of the map

	temp_map_len = strlen(temp_map);
	for(i = 0; i < MAX_MAP_SERVERS; i++)
		if (server_fd[i] >= 0)
			for (j = 0; server[i].map[j][0]; j++)
				//printf("%s : %s = %d\n", server[i].map[j], map, strncmp(server[i].map[j], temp_map, temp_map_len));
				if (strncmp(server[i].map[j], temp_map, temp_map_len) == 0) {
//					printf("found -> server #%d.\n", i);
					return i;
				}

//	printf("not found.\n");
	return -1;
}

int char_mapif_init(int fd) {
	return inter_mapif_init(fd);
}

//-----------------------------------------------------
// Test to know if an IP come from LAN or WAN. by [Yor]
//-----------------------------------------------------
int lan_ip_check(unsigned char *p){
	int i;
	int lancheck = 1;
	int subneti[4];
	unsigned int k0, k1, k2, k3;

	sscanf(lan_map_ip, "%d.%d.%d.%d", &k0, &k1, &k2, &k3);
	subneti[0] = k0; subneti[1] = k1; subneti[2] = k2; subneti[3] = k3;

//	printf("lan_ip_check: to compare: %d.%d.%d.%d, network: %d.%d.%d.%d/%d.%d.%d.%d\n",
//	       p[0], p[1], p[2], p[3],
//	       subneti[0], subneti[1], subneti[2], subneti[3],
//	       subnetmaski[0], subnetmaski[1], subnetmaski[2], subnetmaski[3]);
	for(i = 0; i < 4; i++) {
		if ((subneti[i] & subnetmaski[i]) != (p[i] & subnetmaski[i])) {
			lancheck = 0;
			break;
		}
	}
//	printf("LAN test (result): %s source\033[0m.\n", (lancheck) ? "\033[1;36mLAN" : "\033[1;32mWAN");
	return lancheck;
}

int parse_char(int fd) {
	int i, ch = 0;
	char email[40];
	unsigned short cmd;
	struct char_session_data *sd;
	unsigned char *p = (unsigned char *) &session[fd]->client_addr.sin_addr;

	if(login_fd < 0)
		session[fd]->eof = 1;
	if(session[fd]->eof) {
		if (fd == login_fd)
			login_fd = -1;
		close(fd);
		delete_session(fd);
		return 0;
	}

	sd = session[fd]->session_data;

	while(RFIFOREST(fd) >= 2) {
		cmd = RFIFOW(fd,0);
		// crc32のスキップ用
		if(	sd==NULL			&&	// 未ログインor管理パケット
			RFIFOREST(fd)>=4	&&	// 最低バイト数制限 ＆ 0x7530,0x7532管理パケ除去
			RFIFOREST(fd)<=21	&&	// 最大バイト数制限 ＆ サーバーログイン除去
			cmd!=0x20b	&&	// md5通知パケット除去
			(RFIFOREST(fd)<6 || RFIFOW(fd,4)==0x65)	){	// 次に何かパケットが来てるなら、接続でないとだめ
			RFIFOSKIP(fd,4);
			cmd = RFIFOW(fd,0);
			printf("parse_char : %d crc32 skipped\n",fd);
			if(RFIFOREST(fd)==0)
				return 0;
		}

//		if(cmd<30000 && cmd!=0x187)
//			printf("parse_char : %d %d %d\n",fd,RFIFOREST(fd),cmd);

		// 不正パケットの処理
//		if (sd == NULL && cmd != 0x65 && cmd != 0x20b && cmd != 0x187 &&
//					 cmd != 0x2af8 && cmd != 0x7530 && cmd != 0x7532)
//			cmd = 0xffff;	// パケットダンプを表示させる
		
		switch(cmd){
		case 0x20b: //20040622 encryption ragexe correspondence
			if (RFIFOREST(fd) < 19)
				return 0;
			RFIFOSKIP(fd,19);
			break;

		case 0x65: // request to connect
			printf("request connect - account_id:%d/login_id1:%d/login_id2:%d\n", RFIFOL(fd, 2), RFIFOL(fd, 6), RFIFOL(fd, 10));
			if (RFIFOREST(fd) < 17)
				return 0;
		  {
/*removed from isGM setup
			if (isGM(RFIFOL(fd,2)))
				printf("Account Logged On; Account ID: %d (GM level %d).\n", RFIFOL(fd,2), isGM(RFIFOL(fd,2)));
			else
				printf("Account Logged On; Account ID: %d.\n", RFIFOL(fd,2));
*/
			if (sd == NULL) {
				CREATE(session[fd]->session_data, struct char_session_data, 1);
				sd = session[fd]->session_data;
				sd->connect_until_time = 0; // unknow or illimited (not displaying on map-server)
			}
			sd->account_id = RFIFOL(fd, 2);
			sd->login_id1 = RFIFOL(fd, 6);
			sd->login_id2 = RFIFOL(fd, 10);
			sd->sex = RFIFOB(fd, 16);

			WFIFOL(fd, 0) = RFIFOL(fd, 2);
			WFIFOSET(fd, 4);

			for(i = 0; i < AUTH_FIFO_SIZE; i++) {
				if (auth_fifo[i].account_id == sd->account_id &&
				    auth_fifo[i].login_id1 == sd->login_id1 &&
#if CMP_AUTHFIFO_LOGIN2 != 0
				    auth_fifo[i].login_id2 == sd->login_id2 && // relate to the versions higher than 18
#endif
				    (!check_ip_flag || auth_fifo[i].ip == session[fd]->client_addr.sin_addr.s_addr) &&
				    auth_fifo[i].delflag == 2) {
					auth_fifo[i].delflag = 1;

				if (max_connect_user == 0 || count_users() < max_connect_user) {
					if (login_fd > 0) { // don't send request if no login-server
						// request to login-server to obtain e-mail/time limit
						WFIFOW(login_fd,0) = 0x2716;
						WFIFOL(login_fd,2) = sd->account_id;
						WFIFOSET(login_fd,6);
					}
					// send characters to player
					mmo_char_send006b(fd, sd);
				} else {
					// refuse connection (over populated)
					WFIFOW(fd,0) = 0x6c;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,3);
				}
//				printf("connection request> set delflag 1(o:2)- account_id:%d/login_id1:%d(fifo_id:%d)\n", sd->account_id, sd->login_id1, i);
				break;
				}
			}
			if (i == AUTH_FIFO_SIZE) {
				if (login_fd > 0) { // don't send request if no login-server
					WFIFOW(login_fd,0) = 0x2712; // ask login-server to authentify an account
					WFIFOL(login_fd,2) = sd->account_id;
					WFIFOL(login_fd,6) = sd->login_id1;
					WFIFOL(login_fd,10) = sd->login_id2;
					WFIFOB(login_fd,14) = sd->sex;
					WFIFOL(login_fd,15) = session[fd]->client_addr.sin_addr.s_addr;
					WFIFOSET(login_fd,19);
				} else { // if no login-server, we must refuse connection
					WFIFOW(fd,0) = 0x6c;
					WFIFOW(fd,2) = 0;
					WFIFOSET(fd,3);
				}
			}
		  }
			RFIFOSKIP(fd, 17);
			break;

		case 0x66: // char select
//			printf("0x66> request connect - account_id:%d/char_num:%d\n",sd->account_id,RFIFOB(fd, 2));
			if (RFIFOREST(fd) < 3)
				return 0;

			sprintf(tmp_sql, "SELECT `char_id` FROM `%s` WHERE `account_id`='%d' AND `char_num`='%d'",char_db, sd->account_id, RFIFOB(fd, 2));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			sql_res = mysql_store_result(&mysql_handle);

			sql_row = mysql_fetch_row(sql_res);

			if (sql_row)
				mmo_char_fromsql(atoi(sql_row[0]), char_dat, 0);
			else {
				mysql_free_result(sql_res);
				RFIFOSKIP(fd, 3);
				break;
			}

			sprintf(tmp_sql,"INSERT INTO `%s`(`time`, `account_id`,`char_num`,`name`) VALUES (NOW(), '%d', '%d', '%s')",
				charlog_db, sd->account_id, RFIFOB(fd, 2), char_dat[0].name);
			//query
			if(mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			printf("(\033[1;64m%d\033[0m) char selected (\033[1;32m%d\033[0m) \033[1;32m%s\033[0m" RETCODE, sd->account_id, RFIFOB(fd, 2), char_dat[0].name);

			i = search_mapserver(char_dat[0].last_point.map);

			// if map is not found, we check major cities
			if (i < 0) {
				if ((i = search_mapserver("prontera.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "prontera.gat", 16);
					char_dat[0].last_point.x = 273; // savepoint coordonates
					char_dat[0].last_point.y = 354;
				} else if ((i = search_mapserver("geffen.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "geffen.gat", 16);
					char_dat[0].last_point.x = 120; // savepoint coordonates
					char_dat[0].last_point.y = 100;
				} else if ((i = search_mapserver("morocc.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "morocc.gat", 16);
					char_dat[0].last_point.x = 160; // savepoint coordonates
					char_dat[0].last_point.y = 94;
				} else if ((i = search_mapserver("alberta.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "alberta.gat", 16);
					char_dat[0].last_point.x = 116; // savepoint coordonates
					char_dat[0].last_point.y = 57;
				} else if ((i = search_mapserver("payon.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "payon.gat", 16);
					char_dat[0].last_point.x = 87; // savepoint coordonates
					char_dat[0].last_point.y = 117;
				} else if ((i = search_mapserver("izlude.gat")) >= 0) { // check is done without 'gat'.
					memcpy(char_dat[0].last_point.map, "izlude.gat", 16);
					char_dat[0].last_point.x = 94; // savepoint coordonates
					char_dat[0].last_point.y = 103;
				} else {
					int j;
					// get first online server
					i = 0;
					for(j = 0; j < MAX_MAP_SERVERS; j++)
						if (server_fd[j] >= 0 && server[j].map[0][0])  {
							i = j;
							printf("Map-server #%d found with a map: '%s'.\n", j, server[j].map[0]);
							break;
						}
					// if no map-servers are connected, we send: server closed
					if (j == MAX_MAP_SERVERS) {
						WFIFOW(fd,0) = 0x81;
						WFIFOL(fd,2) = 1; // 01 = Server closed
						WFIFOSET(fd,3);
						RFIFOSKIP(fd,3);
						break;
					}
				}
			}
			WFIFOW(fd, 0) =0x71;
			WFIFOL(fd, 2) =char_dat[0].char_id;
			memcpy(WFIFOP(fd, 6), char_dat[0].last_point.map, 16);
			//Lan check added by Kashy
			if (lan_ip_check(p))
				WFIFOL(fd, 22) = inet_addr(lan_map_ip);
			else
				WFIFOL(fd, 22) = server[i].ip;
			WFIFOW(fd, 26) = server[i].port;
			WFIFOSET(fd, 28);

			if (auth_fifo_pos >= AUTH_FIFO_SIZE) {
				auth_fifo_pos = 0;
			}
//			printf("auth_fifo set (auth_fifo_pos:%d) - account_id:%d char_id:%d login_id1:%d\n", auth_fifo_pos, sd->account_id, char_dat[0].char_id, sd->login_id1);
			auth_fifo[auth_fifo_pos].account_id = sd->account_id;
			auth_fifo[auth_fifo_pos].char_id = char_dat[0].char_id;
			auth_fifo[auth_fifo_pos].login_id1 = sd->login_id1;
			auth_fifo[auth_fifo_pos].login_id2 = sd->login_id2;
			auth_fifo[auth_fifo_pos].delflag = 0;
			//auth_fifo[auth_fifo_pos].char_pos = sd->found_char[ch];
			auth_fifo[auth_fifo_pos].char_pos = 0;
			auth_fifo[auth_fifo_pos].sex = sd->sex;
			auth_fifo[auth_fifo_pos].connect_until_time = sd->connect_until_time;
			auth_fifo[auth_fifo_pos].ip = session[fd]->client_addr.sin_addr.s_addr;
			auth_fifo_pos++;
//			printf("0x66> end\n");
			RFIFOSKIP(fd, 3);
			break;

		case 0x67:	// make new
//			printf("0x67>request make new char\n");
			if (RFIFOREST(fd) < 37)
				return 0;
			i = make_new_char_sql(fd, RFIFOP(fd, 2));
			if (i < 0) {
				WFIFOW(fd, 0) = 0x6e;
				WFIFOB(fd, 2) = 0x00;
				WFIFOSET(fd, 3);
				RFIFOSKIP(fd, 37);
				break;
			}

			WFIFOW(fd, 0) = 0x6d;
			memset(WFIFOP(fd, 2), 0x00, 106);

			mmo_char_fromsql(i, char_dat, 0);
			i = 0;
			WFIFOL(fd, 2) = char_dat[i].char_id;
			WFIFOL(fd,2+4) = char_dat[i].base_exp;
			WFIFOL(fd,2+8) = char_dat[i].zeny;
			WFIFOL(fd,2+12) = char_dat[i].job_exp;
			WFIFOL(fd,2+16) = char_dat[i].job_level;

			WFIFOL(fd,2+28) = char_dat[i].karma;
			WFIFOL(fd,2+32) = char_dat[i].manner;

			WFIFOW(fd,2+40) = 0x30;
			WFIFOW(fd,2+42) = (char_dat[i].hp > 0x7fff) ? 0x7fff : char_dat[i].hp;
			WFIFOW(fd,2+44) = (char_dat[i].max_hp > 0x7fff) ? 0x7fff : char_dat[i].max_hp;
			WFIFOW(fd,2+46) = (char_dat[i].sp > 0x7fff) ? 0x7fff : char_dat[i].sp;
			WFIFOW(fd,2+48) = (char_dat[i].max_sp > 0x7fff) ? 0x7fff : char_dat[i].max_sp;
			WFIFOW(fd,2+50) = DEFAULT_WALK_SPEED; // char_dat[i].speed;
			WFIFOW(fd,2+52) = char_dat[i].class;
			WFIFOW(fd,2+54) = char_dat[i].hair;

			WFIFOW(fd,2+58) = char_dat[i].base_level;
			WFIFOW(fd,2+60) = char_dat[i].skill_point;

			WFIFOW(fd,2+64) = char_dat[i].shield;
			WFIFOW(fd,2+66) = char_dat[i].head_top;
			WFIFOW(fd,2+68) = char_dat[i].head_mid;
			WFIFOW(fd,2+70) = char_dat[i].hair_color;

			memcpy(WFIFOP(fd,2+74), char_dat[i].name, 24);

			WFIFOB(fd,2+98) = char_dat[i].str;
			WFIFOB(fd,2+99) = char_dat[i].agi;
			WFIFOB(fd,2+100) = char_dat[i].vit;
			WFIFOB(fd,2+101) = char_dat[i].int_;
			WFIFOB(fd,2+102) = char_dat[i].dex;
			WFIFOB(fd,2+103) = char_dat[i].luk;
			WFIFOB(fd,2+104) = char_dat[i].char_num;

			WFIFOSET(fd, 108);
			RFIFOSKIP(fd, 37);
			//to do
			for(ch = 0; ch < 9; ch++) {
				if (sd->found_char[ch] == -1) {
					sd->found_char[ch] = char_dat[i].char_id;
					break;
				}
			}

		case 0x68: // delete
			if (RFIFOREST(fd) < 46)
				return 0;
			printf("\033[1;31m Request Char Del:\033[0m \033[1;32m%d\033[0m(\033[1;32m%d\033[0m)\n", sd->account_id, RFIFOL(fd, 2));
			memcpy(email, RFIFOP(fd,6), 40);
			sprintf(tmp_sql, "SELECT `email` FROM `%s` WHERE `%s`='%d'",login_db, login_db_account_id, sd->account_id);
			if (mysql_query(&lmysql_handle, tmp_sql)) {
				printf("\033[1;31m DB server Error Delete Char data - %s \033[0m  \n", mysql_error(&lmysql_handle));
			}
			sql_res = mysql_store_result(&lmysql_handle);
			if (sql_res) {
				sql_row = mysql_fetch_row(sql_res);
				if (strcmp(email,sql_row[0]) == 0) {
					mysql_free_result(sql_res);
				} else {
					WFIFOW(fd, 0) = 0x70;
					WFIFOB(fd, 2) = 0;
					WFIFOSET(fd, 3);
					RFIFOSKIP(fd, 46);
					mysql_free_result(sql_res);
					break;
				}
			} else {
				WFIFOW(fd, 0) = 0x70;
				WFIFOB(fd, 2) = 0;
				WFIFOSET(fd, 3);
				RFIFOSKIP(fd, 46);
				mysql_free_result(sql_res);
				break;
			}
			sprintf(tmp_sql, "SELECT `name`,`partner_id` FROM `%s` WHERE `char_id`='%d'",char_db, RFIFOL(fd,2));
			if (mysql_query(&mysql_handle, tmp_sql)) {
				printf("DB server Error - %s\n", mysql_error(&mysql_handle));
			}
			sql_res = mysql_store_result(&mysql_handle);
			if(sql_res)
				sql_row = mysql_fetch_row(sql_res);

			if (sql_res && sql_row[0]) {
				//delete char from SQL
				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",pet_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}
				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",inventory_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",cart_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",memo_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",skill_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",char_db, RFIFOL(fd, 2));
				if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				//Divorce [Wizputer]
				if (sql_row[1] != 0) {
					char buf[64];
					sprintf(tmp_sql,"UPDATE `%s` SET `partner_id`='0' WHERE `char_id`='%d'",char_db,atoi(sql_row[1]));
					if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
					}
					sprintf(tmp_sql,"DELETE FROM `%s` WHERE (`nameid`='%d' OR `nameid`='%d') AND `char_id`='%d'",inventory_db,WEDDING_RING_M,WEDDING_RING_F,atoi(sql_row[1]));
					if(mysql_query(&mysql_handle, tmp_sql)) {
						printf("DB server Error - %s\n", mysql_error(&mysql_handle));
					}
					WBUFW(buf,0) = 0x2b12;
					WBUFL(buf,2) = atoi(sql_row[0]);
					WBUFL(buf,6) = atoi(sql_row[1]);
					mapif_sendall(buf,10);
				}
				// Also delete info from guildtables.
				sprintf(tmp_sql,"DELETE FROM `%s` WHERE `char_id`='%d'",guild_member_db, RFIFOL(fd,2));
				if (mysql_query(&mysql_handle, tmp_sql)) {
					printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}

				sprintf(tmp_sql, "SELECT `guild_id` FROM `%s` WHERE `master` = '%s'", guild_db, sql_row[0]);

				if (mysql_query(&mysql_handle, tmp_sql) == 0) {
					sql_res = mysql_store_result(&mysql_handle);

					if (sql_res != NULL) {
						if (mysql_num_rows(sql_res) != 0) {
							sql_row = mysql_fetch_row(sql_res);

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_member_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_castle_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_storage_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d' OR `alliance_id` = '%d'", guild_alliance_db, atoi(sql_row[0]), atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_position_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_skill_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							sprintf(tmp_sql,"DELETE FROM `%s` WHERE `guild_id` = '%d'", guild_expulsion_db, atoi(sql_row[0]));
							if (mysql_query(&mysql_handle, tmp_sql)) {
								printf("DB server Error - %s\n", mysql_error(&mysql_handle));
							}

							mysql_free_result(sql_res);
						}
					} else {
						if (mysql_errno(&mysql_handle) != 0) {
							printf("Database server error: %s\n", mysql_error(&mysql_handle));
						}
					}
				} else {
					printf("DB server Error - %s\n", mysql_error(&mysql_handle));
				}
			}

			for(i = 0; i < 9; i++) {
				printf("char comp: %d - %d (%d)\n", sd->found_char[i], RFIFOL(fd, 2), sd->account_id);
				if (sd->found_char[i] == RFIFOL(fd, 2)) {
					for(ch = i; ch < 9-1; ch++)
						sd->found_char[ch] = sd->found_char[ch+1];
					sd->found_char[8] = -1;
					break;
				}
			}
			if (i == 9) { // reject
				WFIFOW(fd, 0) = 0x70;
				WFIFOB(fd, 2) = 0;
				WFIFOSET(fd, 3);
			} else { // deleted!
				WFIFOW(fd, 0) = 0x6f;
				WFIFOSET(fd, 2);
			}
			RFIFOSKIP(fd, 46);
			break;

		case 0x2af8: // login as map-server
			if (RFIFOREST(fd) < 60)
				return 0;
			WFIFOW(fd, 0) = 0x2af9;
			for(i = 0; i < MAX_MAP_SERVERS; i++) {
				if (server_fd[i] < 0)
					break;
			}
			if (i == MAX_MAP_SERVERS || strcmp(RFIFOP(fd,2), userid) || strcmp(RFIFOP(fd,26), passwd)) {
				WFIFOB(fd,2) = 3;
				WFIFOSET(fd, 3);
			} else {
//				int len;
				WFIFOB(fd,2) = 0;
				WFIFOSET(fd, 3);
				session[fd]->func_parse = parse_frommap;
				server_fd[i] = fd;
				if(anti_freeze_enable)
					server_freezeflag[i] = 5; // Map anti-freeze system. Counter. 5 ok, 4...0 freezed
				server[i].ip = RFIFOL(fd, 54);
				server[i].port = RFIFOW(fd, 58);
				server[i].users = 0;
				memset(server[i].map, 0, sizeof(server[i].map));
				realloc_fifo(fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
				char_mapif_init(fd);
				// send gm acccounts level to map-servers
/* removed by CLOWNISIUS due to isGM
				len = 4;
				WFIFOW(fd,0) = 0x2b15;
				for(i = 0; i < GM_num; i++) {
					WFIFOL(fd,len) = gm_account[i].account_id;
					WFIFOB(fd,len+4) = (unsigned char)gm_account[i].level;
					len += 5;
				}
				WFIFOW(fd,2) = len;
				WFIFOSET(fd,len);*/
			}
			RFIFOSKIP(fd,60);
			break;

		case 0x187:	// Alive?
			if (RFIFOREST(fd) < 6) {
				return 0;
			}
			RFIFOSKIP(fd, 6);
			break;

		case 0x7530:	// Athena info get
			WFIFOW(fd, 0) = 0x7531;
			WFIFOB(fd, 2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd, 3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd, 4) = ATHENA_REVISION;
			WFIFOB(fd, 5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd, 6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd, 7) = ATHENA_SERVER_INTER | ATHENA_SERVER_CHAR;
			WFIFOW(fd, 8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd, 10);
			RFIFOSKIP(fd, 2);
			return 0;

		case 0x7532:	// disconnect(default also disconnect)
		default:
			session[fd]->eof = 1;
			return 0;
		}
	}
	RFIFOFLUSH(fd);

	return 0;
}

// Console Command Parser [Wizputer]
int parse_console(char *buf) {
    char *type,*command;
    
    type = (char *)malloc(64);
    command = (char *)malloc(64);
    
    memset(type,0,64);
    memset(command,0,64);
    
    printf("Console: %s\n",buf);
    
    if ( sscanf(buf, "%[^:]:%[^\n]", type , command ) < 2 )
        sscanf(buf,"%[^\n]",type);
    
    printf("Type of command: %s || Command: %s \n",type,command);
    
    if(buf) free(buf);
    if(type) free(type);
    if(command) free(command);
    
    return 0;
}

// MAP send all
int mapif_sendall(unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;

	c = 0;
	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		if ((fd = server_fd[i]) >= 0) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd,len);
			c++;
		}
	}

	return c;
}

int mapif_sendallwos(int sfd, unsigned char *buf, unsigned int len) {
	int i, c;
	int fd;

	c = 0;
	for(i=0, c=0;i<MAX_MAP_SERVERS;i++){
		if ((fd = server_fd[i]) >= 0 && fd != sfd) {
			memcpy(WFIFOP(fd,0), buf, len);
			WFIFOSET(fd, len);
			c++;
		}
	}

	return c;
}

int mapif_send(int fd, unsigned char *buf, unsigned int len) {
	int i;

	if (fd >= 0) {
		for(i = 0; i < MAX_MAP_SERVERS; i++) {
			if (fd == server_fd[i]) {
				memcpy(WFIFOP(fd,0), buf, len);
				WFIFOSET(fd,len);
				return 1;
			}
		}
	}
	return 0;
}

int send_users_tologin(int tid, unsigned int tick, int id, int data) {
	int users = count_users();
	char buf[16];

	if (login_fd > 0 && session[login_fd]) {
		// send number of user to login server
		WFIFOW(login_fd,0) = 0x2714;
		WFIFOL(login_fd,2) = users;
		WFIFOSET(login_fd,6);
	}
	// send number of players to all map-servers
	WBUFW(buf,0) = 0x2b00;
	WBUFL(buf,2) = users;
	mapif_sendall(buf, 6);

	return 0;
}

int check_connect_login_server(int tid, unsigned int tick, int id, int data) {
	if (login_fd <= 0 || session[login_fd] == NULL) {
		printf("Attempt to connect to login-server...\n");
		login_fd = make_connection(login_ip, login_port);
		session[login_fd]->func_parse = parse_tologin;
		realloc_fifo(login_fd, FIFOSIZE_SERVERLINK, FIFOSIZE_SERVERLINK);
		WFIFOW(login_fd,0) = 0x2710;
		memset(WFIFOP(login_fd,2), 0, 24);
		memcpy(WFIFOP(login_fd,2), userid, strlen(userid) < 24 ? strlen(userid) : 24);
		memset(WFIFOP(login_fd,26), 0, 24);
		memcpy(WFIFOP(login_fd,26), passwd, strlen(passwd) < 24 ? strlen(passwd) : 24);
		WFIFOL(login_fd,50) = 0;
		WFIFOL(login_fd,54) = char_ip;
		WFIFOL(login_fd,58) = char_port;
		memset(WFIFOP(login_fd,60), 0, 20);
		memcpy(WFIFOP(login_fd,60), server_name, strlen(server_name) < 20 ? strlen(server_name) : 20);
		WFIFOW(login_fd,80) = 0;
		WFIFOW(login_fd,82) = char_maintenance;
		WFIFOW(login_fd,84) = char_new;
		WFIFOSET(login_fd,86);
	}
	return 0;
}

//----------------------------------------------------------
// Return numerical value of a switch configuration by [Yor]
// on/off, english, fran軋is, deutsch, espal
//----------------------------------------------------------
int config_switch(const char *str) {
	if (strcmpi(str, "on") == 0 || strcmpi(str, "yes") == 0 || strcmpi(str, "oui") == 0 || strcmpi(str, "ja") == 0 || strcmpi(str, "si") == 0)
		return 1;
	if (strcmpi(str, "off") == 0 || strcmpi(str, "no") == 0 || strcmpi(str, "non") == 0 || strcmpi(str, "nein") == 0)
		return 0;

	return atoi(str);
}

// Lan Support conf reading added by Kashy
int char_lan_config_read(const char *lancfgName){
	char subnetmask[128];
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	struct hostent * h = NULL;

	if ((fp = fopen(lancfgName, "r")) == NULL) {
		printf("file not found: %s\n", lancfgName);
		return 1;
	}

	printf("Start reading of Lan Support configuration file\n");

	while(fgets(line, sizeof(line)-1, fp)){
		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		else if (strcmpi(w1, "lan_map_ip") == 0) {
			h = gethostbyname(w2);
			if (h != NULL) {
				sprintf(lan_map_ip, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else {
				strncpy(lan_map_ip, w2, sizeof(lan_map_ip));
				lan_map_ip[sizeof(lan_map_ip)-1] = 0;
			}
			printf("set Lan_map_IP : %s\n", lan_map_ip);
		}

		else if (strcmpi(w1, "subnetmask") == 0) {
			unsigned int k0, k1, k2, k3;
			strcpy(subnetmask, w2);
			sscanf(subnetmask, "%d.%d.%d.%d", &k0, &k1, &k2, &k3);
			subnetmaski[0] = k0; subnetmaski[1] = k1; subnetmaski[2] = k2; subnetmaski[3] = k3;
			printf("set subnetmask : %s\n", w2);
		}
	}
	fclose(fp);

	printf("End reading of Lan Support configuration file\n");
	return 0;
}

void do_final(void) {
	printf("Doing final stage...\n");
	//mmo_char_sync();
	//inter_save();
	do_final_itemdb();
	//check SQL save progress.
	//wait until save char complete
	printf("waiting until char saving complete...\n");
	do {
		sleep (0);
	}while (save_flag != 0);

	sprintf(tmp_sql,"UPDATE `%s` SET `online`='0' WHERE `online`='1'", char_db);
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `char`)- %s\n", mysql_error(&mysql_handle));
	}

	sprintf(tmp_sql,"DELETE FROM `ragsrvinfo");
	if (mysql_query(&mysql_handle, tmp_sql)) {
		printf("DB server Error (insert `char`)- %s\n", mysql_error(&mysql_handle));
	}

	if(gm_account) free(gm_account);

	if(char_dat) free(char_dat);
	delete_session(login_fd);
	delete_session(char_fd);

	mysql_close(&mysql_handle);
	mysql_close(&lmysql_handle);

	printf("ok! all done...\n");
}

void sql_config_read(const char *cfgName){ /* Kalaspuff, to get login_db */
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	printf("reading configure: %s\n", cfgName);

	if ((fp = fopen(cfgName, "r")) == NULL) {
		printf("file not found: %s\n", cfgName);
		exit(1);
	}

	while(fgets(line, sizeof(line)-1, fp)){
		if(line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		if(strcmpi(w1, "login_db") == 0) {
			strcpy(login_db, w2);
		}else if(strcmpi(w1,"char_db")==0){
			strcpy(char_db,w2);
		}else if(strcmpi(w1,"cart_db")==0){
			strcpy(cart_db,w2);
		}else if(strcmpi(w1,"inventory_db")==0){
			strcpy(inventory_db,w2);
		}else if(strcmpi(w1,"charlog_db")==0){
			strcpy(charlog_db,w2);
		}else if(strcmpi(w1,"storage_db")==0){
			strcpy(storage_db,w2);
		}else if(strcmpi(w1,"reg_db")==0){
			strcpy(reg_db,w2);
		}else if(strcmpi(w1,"skill_db")==0){
			strcpy(skill_db,w2);
		}else if(strcmpi(w1,"interlog_db")==0){
			strcpy(interlog_db,w2);
		}else if(strcmpi(w1,"memo_db")==0){
			strcpy(memo_db,w2);
		}else if(strcmpi(w1,"guild_db")==0){
			strcpy(guild_db,w2);
		}else if(strcmpi(w1,"guild_alliance_db")==0){
			strcpy(guild_alliance_db,w2);
		}else if(strcmpi(w1,"guild_castle_db")==0){
			strcpy(guild_castle_db,w2);
		}else if(strcmpi(w1,"guild_expulsion_db")==0){
			strcpy(guild_expulsion_db,w2);
		}else if(strcmpi(w1,"guild_member_db")==0){
			strcpy(guild_member_db,w2);
		}else if(strcmpi(w1,"guild_skill_db")==0){
			strcpy(guild_skill_db,w2);
		}else if(strcmpi(w1,"guild_position_db")==0){
			strcpy(guild_position_db,w2);
		}else if(strcmpi(w1,"guild_storage_db")==0){
			strcpy(guild_storage_db,w2);
		}else if(strcmpi(w1,"party_db")==0){
			strcpy(party_db,w2);
		}else if(strcmpi(w1,"pet_db")==0){
			strcpy(pet_db,w2);
		}else if(strcmpi(w1,"db_path")==0){
			strcpy(db_path,w2);
		//Map server option to use SQL db or not
		}else if(strcmpi(w1,"use_sql_db")==0){ // added for sql item_db read for char server [Valaris]
			if (strcmpi(w2, "yes")) {
				db_use_sqldbs = 1;
			} else if (strcmpi(w2, "no")) {
				db_use_sqldbs = 0;
			}
			printf("Using SQL dbs: %s\n",w2);
		//custom columns for login database
		}else if(strcmpi(w1,"login_db_level")==0){
			strcpy(login_db_level,w2);
		}else if(strcmpi(w1,"login_db_account_id")==0){
			strcpy(login_db_account_id,w2);
		}else if(strcmpi(w1,"lowest_gm_level")==0){
			lowest_gm_level = atoi(w2);
			printf("set lowest_gm_level : %s\n",w2);
		}
	}
	fclose(fp);
	printf("reading configure done.....\n");
}

int char_config_read(const char *cfgName) {
	struct hostent *h = NULL;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;

	if ((fp = fopen(cfgName, "r")) == NULL) {
		printf("Configuration file not found: %s.\n", cfgName);
		exit(1);
	}

	while(fgets(line, sizeof(line)-1, fp)) {
		if (line[0] == '/' && line[1] == '/')
			continue;

		line[sizeof(line)-1] = '\0';
		if (sscanf(line,"%[^:]: %[^\r\n]", w1, w2) != 2)
			continue;

		remove_control_chars(w1);
		remove_control_chars(w2);
		if (strcmpi(w1, "userid") == 0) {
			memcpy(userid, w2, 24);
		} else if (strcmpi(w1, "passwd") == 0) {
			memcpy(passwd, w2, 24);
		} else if (strcmpi(w1, "server_name") == 0) {
			memcpy(server_name, w2, 16);
			printf("%s server has been intialized\n", w2);
		} else if (strcmpi(w1, "wisp_server_name") == 0) {
			if (strlen(w2) >= 4) {
				memcpy(wisp_server_name, w2, sizeof(wisp_server_name));
				wisp_server_name[sizeof(wisp_server_name) - 1] = '\0';
			}
		} else if (strcmpi(w1, "login_ip") == 0) {
			login_ip_set_ = 1;
			h = gethostbyname (w2);
			if (h != NULL) {
				printf("Login server IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				sprintf(login_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				memcpy(login_ip_str,w2,16);
		} else if (strcmpi(w1, "login_port") == 0) {
			login_port=atoi(w2);
		} else if (strcmpi(w1, "char_ip") == 0) {
			char_ip_set_ = 1;
			h = gethostbyname (w2);
			if(h != NULL) {
				printf("Character server IP address : %s -> %d.%d.%d.%d\n", w2, (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
				sprintf(char_ip_str, "%d.%d.%d.%d", (unsigned char)h->h_addr[0], (unsigned char)h->h_addr[1], (unsigned char)h->h_addr[2], (unsigned char)h->h_addr[3]);
			} else
				memcpy(char_ip_str, w2, 16);
		} else if (strcmpi(w1, "char_port") == 0) {
			char_port = atoi(w2);
		} else if (strcmpi(w1, "char_maintenance") == 0) {
			char_maintenance = atoi(w2);
		} else if (strcmpi(w1, "char_new")==0){
			char_new = atoi(w2);
		} else if (strcmpi(w1, "max_connect_user") == 0) {
			max_connect_user = atoi(w2);
			if (max_connect_user < 0)
				max_connect_user = 0; // unlimited online players
		} else if (strcmpi(w1, "check_ip_flag") == 0) {
			check_ip_flag = config_switch(w2);
		} else if (strcmpi(w1, "autosave_time") == 0) {
			autosave_interval = atoi(w2)*1000;
			if (autosave_interval <= 0)
				autosave_interval = DEFAULT_AUTOSAVE_INTERVAL;
		} else if (strcmpi(w1, "start_point") == 0) {
			char map[32];
			int x, y;
			if (sscanf(w2,"%[^,],%d,%d", map, &x, &y) < 3)
				continue;
			if (strstr(map, ".gat") != NULL) { // Verify at least if '.gat' is in the map name
				memcpy(start_point.map, map, 16);
				start_point.x = x;
				start_point.y = y;
			}
		} else if (strcmpi(w1, "start_zeny") == 0) {
			start_zeny = atoi(w2);
			if (start_zeny < 0)
				start_zeny = 0;
		} else if (strcmpi(w1, "start_weapon") == 0) {
			start_weapon = atoi(w2);
			if (start_weapon < 0)
				start_weapon = 0;
		} else if (strcmpi(w1, "start_armor") == 0) {
			start_armor = atoi(w2);
			if (start_armor < 0)
				start_armor = 0;
		} else if(strcmpi(w1,"imalive_on")==0){		//Added by Mugendai for I'm Alive mod
			imalive_on = atoi(w2);			//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"imalive_time")==0){	//Added by Mugendai for I'm Alive mod
			imalive_time = atoi(w2);		//Added by Mugendai for I'm Alive mod
		} else if(strcmpi(w1,"flush_on")==0){		//Added by Mugendai for GUI
			flush_on = atoi(w2);			//Added by Mugendai for GUI
		} else if(strcmpi(w1,"flush_time")==0){		//Added by Mugendai for GUI
			flush_time = atoi(w2);			//Added by Mugendai for GUI
		} else if(strcmpi(w1,"log_char")==0){		//log char or not [devil]
			log_char = atoi(w2);
		} else if (strcmpi(w1, "unknown_char_name") == 0) {
			strcpy(unknown_char_name, w2);
			unknown_char_name[24] = 0;
		} else if (strcmpi(w1, "name_ignoring_case") == 0) {
			name_ignoring_case = config_switch(w2);
		} else if (strcmpi(w1, "char_name_option") == 0) {
			char_name_option = atoi(w2);
		} else if (strcmpi(w1, "char_name_letters") == 0) {
			strcpy(char_name_letters, w2);
		} else if (strcmpi(w1, "check_ip_flag") == 0) {
			check_ip_flag = config_switch(w2);
		// anti-freeze options [Valaris]
		} else if(strcmpi(w1,"anti_freeze_enable")==0){
			anti_freeze_enable = config_switch(w2);
		} else if (strcmpi(w1, "anti_freeze_interval") == 0) {
			ANTI_FREEZE_INTERVAL = atoi(w2);
			if (ANTI_FREEZE_INTERVAL < 5)
				ANTI_FREEZE_INTERVAL = 5; // minimum 5 seconds
		} else if (strcmpi(w1, "import") == 0) {
			char_config_read(w2);
		} else if (strcmpi(w1, "console") == 0) {
			    if(strcmpi(w2,"on") == 0 || strcmpi(w2,"yes") == 0 )
			        console = 1;
        }
	}
	fclose(fp);

//Read ItemDB
	do_init_itemdb();

	return 0;
}

//-----------------------------------------------------
//I'm Alive Alert
//Used to output 'I'm Alive' every few seconds
//Intended to let frontends know if the app froze
//-----------------------------------------------------
int imalive_timer(int tid, unsigned int tick, int id, int data){
	printf("I'm Alive\n");
	return 0;
}

//-----------------------------------------------------
//Flush stdout
//stdout buffer needs flushed to be seen in GUI
//-----------------------------------------------------
int flush_timer(int tid, unsigned int tick, int id, int data){
	fflush(stdout);
	return 0;
}

int do_init(int argc, char **argv){
	int i;

	for(i = 0; i < MAX_MAP_SERVERS; i++) {
		memset(&server[i], 0, sizeof(struct mmo_map_server));
		server_fd[i] = -1;
	}

	char_config_read((argc < 2) ? CHAR_CONF_NAME : argv[1]);
	char_lan_config_read((argc > 1) ? argv[1] : LAN_CONF_NAME);
	sql_config_read(SQL_CONF_NAME);

	printf("charserver configuration reading done.....\n");

	inter_init((argc > 2) ? argv[2] : inter_cfgName); // inter server ﾃﾊｱ篳ｭ
	printf("interserver configuration reading done.....\n");

	printf("start char server initializing.....\n");
	mmo_char_sql_init();
	printf("char server initializing done.....\n");

	printf("set parser -> parse_char().....\n");
	set_defaultparse(parse_char);

	printf("set terminate function -> do_final().....\n");
	set_termfunc(do_final);

	printf("open port %d.....\n",char_port);
	char_fd = make_listen_port(char_port);

        if ((naddr_ != 0) && (login_ip_set_ == 0 || char_ip_set_ == 0)) { 
          // The char server should know what IP address it is running on
          //   - MouseJstr
          int localaddr = ntohl(addr_[0]);
          unsigned char *ptr = (unsigned char *) &localaddr;
          char buf[16];
          sprintf(buf, "%d.%d.%d.%d", ptr[0], ptr[1], ptr[2], ptr[3]);;
          if (naddr_ != 1) 
            printf("Multiple interfaces detected..  using %s as our IP address\n", buf);
          else
            printf("Defaulting to %s as our IP address\n", buf);
          if (login_ip_set_ == 0)
          	strcpy(login_ip_str, buf);
          if (char_ip_set_ == 0)
          	strcpy(char_ip_str, buf);

          if (ptr[0] == 192 && ptr[1] == 168)  
            printf("Firewall detected.. edit lan_support.conf and char_athena.conf");
        } 

	login_ip = inet_addr(login_ip_str);
	char_ip = inet_addr(char_ip_str);

	// send ALIVE PING to login server.
	printf("add interval tic (check_connect_login_server)....\n");
	i = add_timer_interval(gettick() + 10, check_connect_login_server, 0, 0, 10 * 1000);

	// send USER COUNT PING to login server.
	printf("add interval tic (send_users_tologin)....\n");
	i = add_timer_interval(gettick() + 10, send_users_tologin, 0, 0, 5 * 1000);

	//no need to set sync timer on SQL version.
	//printf("add interval tic (mmo_char_sync_timer)....\n");
	//i = add_timer_interval(gettick() + 10, mmo_char_sync_timer, 0, 0, autosave_interval);

	add_timer_func_list(map_anti_freeze_system, "map_anti_freeze_system");
	//Added for Mugendais I'm Alive mod
	if(imalive_on)
		add_timer_interval(gettick()+10, imalive_timer,0,0,imalive_time*1000);

	//Added by Mugendai for GUI support
	if(flush_on)
		add_timer_interval(gettick()+10, flush_timer,0,0,flush_time);

	if(anti_freeze_enable > 0) {
		add_timer_func_list(map_anti_freeze_system, "map_anti_freeze_system");
		i = add_timer_interval(gettick() + 1000, map_anti_freeze_system, 0, 0, ANTI_FREEZE_INTERVAL * 1000); // checks every X seconds user specifies
	}

	read_gm_account();
	
	if ( console ) {
	    set_defaultconsoleparse(parse_console);
	   	start_console();
	}
	
	printf("char server init func end (now unlimited loop start!)....\n");
	printf("The char-server is \033[1;32mready\033[0m (Server is listening on the port %d).\n\n", char_port);
	return 0;
}


