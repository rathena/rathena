// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "../common/core.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/mmo.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/malloc.h"

#include "charsave.h"
#include "map.h"

#ifndef TXT_ONLY

struct mmo_charstatus *charsave_loadchar(int charid){
	int i,j, friends;
	struct mmo_charstatus *c;
	char *str_p;
	friends = 0;

	c = (struct mmo_charstatus *)aCalloc(1,sizeof(struct mmo_charstatus));

         if(charid <= 0){
         	ShowError("charsave_loadchar() charid <= 0! (%d)", charid);
				aFree(c);
         	return NULL;
         }
    // add homun_id [albator]
	//Tested, Mysql 4.1.9+ has no problems with the long query, the buf is 65k big and the sql server needs for it 0.00009 secs on an athlon xp 2400+ WinXP (1GB Mem) ..  [Sirius]
		sprintf(tmp_sql, "SELECT `char_id`,`account_id`,`char_num`,`name`,`class`,`base_level`,`job_level`,`base_exp`,`job_exp`,`zeny`, `str`,`agi`,`vit`,`int`,`dex`,`luk`, `max_hp`,`hp`,`max_sp`,`sp`,`status_point`,`skill_point`, `option`,`karma`,`manner`,`party_id`,`guild_id`,`pet_id`,`hair`,`hair_color`, `clothes_color`,`weapon`,`shield`,`head_top`,`head_mid`,`head_bottom`, `last_map`,`last_x`,`last_y`,`save_map`,`save_x`,`save_y`, `partner_id`, `father`, `mother`, `child`, `fame`, `homun_id` FROM `char` WHERE `char_id` = '%d'", charid);
    	if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
         	return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(mysql_num_rows(charsql_res) <= 0){
         	ShowWarning("charsave_loadchar() -> CHARACTER NOT FOUND! (id: %d)\n", charid);
         	mysql_free_result(charsql_res);
				aFree(c);
         	return NULL;
         }

         //fetch data
         charsql_row = mysql_fetch_row(charsql_res);

         //fill with data
         c->char_id = charid;
         c->account_id = atoi(charsql_row[1]);
         c->char_num = atoi(charsql_row[2]);
         strcpy(c->name, charsql_row[3]);
         c->class_ = atoi(charsql_row[4]);
         c->base_level = atoi(charsql_row[5]);
         c->job_level = atoi(charsql_row[6]);
         c->base_exp = atoi(charsql_row[7]);
         c->job_exp = atoi(charsql_row[8]);
         c->zeny = atoi(charsql_row[9]);
         c->str = atoi(charsql_row[10]);
         c->agi = atoi(charsql_row[11]);
         c->vit = atoi(charsql_row[12]);
         c->int_ = atoi(charsql_row[13]);
         c->dex = atoi(charsql_row[14]);
         c->luk = atoi(charsql_row[15]);
         c->max_hp = atoi(charsql_row[16]);
         c->hp = atoi(charsql_row[17]);
         c->max_sp = atoi(charsql_row[18]);
         c->sp = atoi(charsql_row[19]);
         c->status_point = atoi(charsql_row[20]) > USHRT_MAX? USHRT_MAX : atoi(charsql_row[20]);
         c->skill_point = atoi(charsql_row[21]) > USHRT_MAX? USHRT_MAX : atoi(charsql_row[21]);
         c->option = atoi(charsql_row[22]);
         c->karma = atoi(charsql_row[23]);
         c->manner = atoi(charsql_row[24]);
         c->party_id = atoi(charsql_row[25]);
         c->guild_id = atoi(charsql_row[26]);
         c->pet_id = atoi(charsql_row[27]);
         c->hair = atoi(charsql_row[28]);
         c->hair_color = atoi(charsql_row[29]);
         c->clothes_color = atoi(charsql_row[30]);
         c->weapon = atoi(charsql_row[31]);
         c->shield = atoi(charsql_row[32]);
         c->head_top = atoi(charsql_row[33]);
         c->head_mid = atoi(charsql_row[34]);
         c->head_bottom = atoi(charsql_row[35]);
			c->last_point.map = mapindex_name2id(charsql_row[36]);
         c->last_point.x = atoi(charsql_row[37]);
         c->last_point.y = atoi(charsql_row[38]);
    		c->save_point.map = mapindex_name2id(charsql_row[39]);
         c->save_point.x = atoi(charsql_row[40]);
         c->save_point.y = atoi(charsql_row[41]);
         c->partner_id = atoi(charsql_row[42]);
         c->father = atoi(charsql_row[43]);
         c->mother = atoi(charsql_row[44]);
         c->child = atoi(charsql_row[45]);
         c->fame = atoi(charsql_row[46]);
			c->hom_id = atoi(charsql_row[47]); // albator

			mysql_free_result(charsql_res);

	//Check for '0' Savepoint / LastPoint
	if (c->last_point.x == 0 || c->last_point.y == 0 || c->last_point.map == 0){
		c->last_point.map = mapindex_name2id(MAP_PRONTERA);
		c->last_point.x = 100;
		c->last_point.y = 100;
	}

	if (c->save_point.x == 0 || c->save_point.y == 0 || c->save_point.map == 0){
		c->save_point.map = mapindex_name2id(MAP_PRONTERA);
		c->save_point.x = 100;
		c->save_point.y = 100;
	}


	//read the memo points
         sprintf(tmp_sql, "SELECT `memo_id`, `char_id`, `map`, `x`, `y` FROM `memo` WHERE `char_id` = '%d' ORDER BY `memo_id`", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
				return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 c->memo_point[i].map = mapindex_name2id(charsql_row[2]);
	                 c->memo_point[i].x = atoi(charsql_row[3]);
	                 c->memo_point[i].y = atoi(charsql_row[4]);
	         }
		mysql_free_result(charsql_res);
         }

         //read inventory...
			str_p = tmp_sql;
         str_p += sprintf(str_p, "SELECT `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
			for (i = 0; i < MAX_SLOTS; i++)
				str_p += sprintf(str_p, ", `card%d`", i);
			str_p += sprintf(str_p, " FROM `inventory` WHERE `char_id` = '%d'", charid);
			if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
				return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
	if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 //c->inventory[i].id = atoi(charsql_row[0]);
	                 c->inventory[i].nameid = atoi(charsql_row[0]);
	                 c->inventory[i].amount = atoi(charsql_row[1]);
	                 c->inventory[i].equip = atoi(charsql_row[2]);
	                 c->inventory[i].identify = atoi(charsql_row[3]);
	                 c->inventory[i].refine = atoi(charsql_row[4]);
	                 c->inventory[i].attribute = atoi(charsql_row[5]);
						  for (j = 0; j < MAX_SLOTS; j++)
							c->inventory[i].card[j] = atoi(charsql_row[6+j]);
	         }
	         mysql_free_result(charsql_res);
         }


         //cart inventory ..
			str_p = tmp_sql;
         str_p += sprintf(str_p, "SELECT `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
			for (i = 0; i < MAX_SLOTS; i++)
				str_p += sprintf(str_p, ", `card%d`", i);
			str_p += sprintf(str_p, " FROM `cart_inventory` WHERE `char_id` = '%d'", charid);

         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
				return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
	if(charsql_res){
	         for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
	                 //c->cart[i].id = atoi(charsql_row[0]);
	                 c->cart[i].nameid = atoi(charsql_row[0]);
	                 c->cart[i].amount = atoi(charsql_row[1]);
	                 c->cart[i].equip = atoi(charsql_row[2]);
	                 c->cart[i].identify = atoi(charsql_row[3]);
	                 c->cart[i].refine = atoi(charsql_row[4]);
	                 c->cart[i].attribute = atoi(charsql_row[5]);
						  for (j = 0; j < MAX_SLOTS; j++)
							c->cart[i].card[j] = atoi(charsql_row[6+j]);
	         }
	         mysql_free_result(charsql_res);
         }


         //Skills...
         sprintf(tmp_sql, "SELECT `char_id`, `id`, `lv` FROM `skill` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
				return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
         	while((charsql_row = mysql_fetch_row(charsql_res))){
                         i = atoi(charsql_row[1]);
                         c->skill[i].id = i;
                         c->skill[i].lv = atoi(charsql_row[2]);
                 }
                 mysql_free_result(charsql_res);
         }
/* Reg values are handled by the char server.
         //Global REG
         sprintf(tmp_sql, "SELECT `char_id`, `str`, `value` FROM `global_reg_value` WHERE `type` = '3' AND `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				aFree(c);
				return NULL;
         }

         charsql_res = mysql_store_result(&charsql_handle);
         if(charsql_res){
         	for(i = 0; (charsql_row = mysql_fetch_row(charsql_res)); i++){
                 	strcpy(c->global_reg[i].str, charsql_row[1]);
                 	strcpy(c->global_reg[i].value, charsql_row[2]);
                 }
                 mysql_free_result(charsql_res);
				c->global_reg_num = i;
         }
*/
	
	//Shamelessly stolen from its_sparky (ie: thanks) and then assimilated by [Skotlex]
	//Friend list
	sprintf(tmp_sql, "SELECT f.friend_account, f.friend_id, c.name FROM friends f LEFT JOIN `char` c ON f.friend_account=c.account_id AND f.friend_id=c.char_id WHERE f.char_id='%d'", charid);

	if(mysql_query(&charsql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		sql_res = NULL; //To avoid trying to read data.
	}
	else
		sql_res = mysql_store_result(&charsql_handle);

	if(sql_res)
	{
		for(i = 0; (sql_row = mysql_fetch_row(sql_res)) && i<MAX_FRIENDS; i++)
		{
			if (sql_row[2] != NULL)
			{
				c->friends[i].account_id = atoi(sql_row[0]);
				c->friends[i].char_id = atoi(sql_row[1]);
				strncpy(c->friends[i].name, sql_row[2], NAME_LENGTH-1); //The -1 is to avoid losing the ending \0 [Skotlex]
			}
		}
		mysql_free_result(sql_res);
	}

	ShowInfo("charsql_loadchar(): loading of '%d' (%s) complete.\n", charid, c->name);
	return c;
}

int charsave_savechar(int charid, struct mmo_charstatus *c){
	int i,j;
	char *str_p;
//	char tmp_str[64];
//	char tmp_str2[512];
         //First save the 'char'
	sprintf(tmp_sql ,"UPDATE `char` SET `class`='%d', `base_level`='%d', `job_level`='%d',"
		"`base_exp`='%d', `job_exp`='%d', `zeny`='%d',"
		"`max_hp`='%d',`hp`='%d',`max_sp`='%d',`sp`='%d',`status_point`='%d',`skill_point`='%d',"
		"`str`='%d',`agi`='%d',`vit`='%d',`int`='%d',`dex`='%d',`luk`='%d',"
		"`option`='%d',`karma`='%d',`manner`='%d',`party_id`='%d',`guild_id`='%d',`pet_id`='%d',"
		"`hair`='%d',`hair_color`='%d',`clothes_color`='%d',`weapon`='%d',`shield`='%d',`head_top`='%d',`head_mid`='%d',`head_bottom`='%d',"
		"`last_map`='%s',`last_x`='%d',`last_y`='%d',`save_map`='%s',`save_x`='%d',`save_y`='%d',"
		"`partner_id`='%d', `father`='%d', `mother`='%d', `child`='%d', `fame`='%d', `homun_id`='%d'"
		"WHERE  `account_id`='%d' AND `char_id` = '%d'",
		c->class_, c->base_level, c->job_level,
		c->base_exp, c->job_exp, c->zeny,
		c->max_hp, c->hp, c->max_sp, c->sp, c->status_point, c->skill_point,
		c->str, c->agi, c->vit, c->int_, c->dex, c->luk,
		c->option, c->karma, c->manner, c->party_id, c->guild_id, c->pet_id,
		c->hair, c->hair_color, c->clothes_color,
		c->weapon, c->shield, c->head_top, c->head_mid, c->head_bottom,
		mapindex_id2name(c->last_point.map), c->last_point.x, c->last_point.y,
		mapindex_id2name(c->save_point.map), c->save_point.x, c->save_point.y, c->partner_id, c->father, c->mother,
		c->child, c->fame, c->hom_id, c->account_id, c->char_id
	);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
         }


		//Save the inventory
		sprintf(tmp_sql, "DELETE FROM `inventory` WHERE `char_id` = '%d'", charid);
		if(mysql_query(&charsql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
		for(i = 0; i < MAX_INVENTORY; i++){
			if(c->inventory[i].nameid > 0){
				str_p = tmp_sql;
				str_p += sprintf(str_p, "INSERT INTO `inventory` (`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
			for (j = 0; j < MAX_SLOTS; j++)
				str_p += sprintf(str_p, ", `card%d`", j);

			str_p += sprintf(str_p, ") VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d'",
				charid, c->inventory[i].nameid, c->inventory[i].amount, c->inventory[i].equip,
				c->inventory[i].identify, c->inventory[i].refine, c->inventory[i].attribute);

			for (j = 0; j < MAX_SLOTS; j++)
				str_p += sprintf(str_p, ", '%d'", c->inventory[i].card[j]);

			strcat(tmp_sql,")");

			if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
		}
	}

		//Save the cart
		sprintf(tmp_sql, "DELETE FROM `cart_inventory` WHERE `char_id` = '%d'", charid);
		if(mysql_query(&charsql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
		for(i = 0; i < MAX_CART; i++){
			if(c->cart[i].nameid > 0){
				str_p = tmp_sql;
				str_p += sprintf(str_p, "INSERT INTO `cart_inventory` (`char_id`, `nameid`, `amount`, `equip`, `identify`, `refine`, `attribute`");
				for (j = 0; j < MAX_SLOTS; j++)
					str_p += sprintf(str_p, ", `card%d`", j);

				str_p += sprintf(str_p, ") VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d'",
					charid, c->cart[i].nameid, c->cart[i].amount, c->cart[i].equip,
					c->cart[i].identify, c->cart[i].refine, c->cart[i].attribute);

				for (j = 0; j < MAX_SLOTS; j++)
					str_p += sprintf(str_p, ", '%d'", c->cart[i].card[j]);

				strcat(tmp_sql,")");

				if(mysql_query(&charsql_handle, tmp_sql)){
					ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				}
			}
		}


         //Save memo points
         sprintf(tmp_sql, "DELETE FROM `memo` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
         }
         for(i = 0; i < MAX_MEMOPOINTS; i++){
         	if(c->memo_point[i].map && c->memo_point[i].x > 0 && c->memo_point[i].y > 0){
                 	sprintf(tmp_sql, "INSERT INTO `memo` ( `char_id`, `map`, `x`, `y` ) VALUES ('%d', '%s', '%d', '%d')", charid, mapindex_id2name(c->memo_point[i].map), c->memo_point[i].x, c->memo_point[i].y);
	                if(mysql_query(&charsql_handle, tmp_sql)){
							ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	                }
                 }
         }


         //Save skills
         sprintf(tmp_sql, "DELETE FROM `skill` WHERE `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
         }
         for(i = 0; i < MAX_SKILL; i++){
         	if(c->skill[i].id > 0){
                 	sprintf(tmp_sql, "INSERT INTO `skill` (`char_id`, `id`, `lv`) VALUES ('%d', '%d', '%d')", charid, c->skill[i].id, c->skill[i].lv);
	                if(mysql_query(&charsql_handle, tmp_sql)){
							ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	                }
                 }
         }


/* Reg values are handled by the char server.
         //global_reg_value saving
         sprintf(tmp_sql, "DELETE FROM `global_reg_value` WHERE `type`=3 AND `char_id` = '%d'", charid);
         if(mysql_query(&charsql_handle, tmp_sql)){
				ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
         }
         for(i = 0; i < c->global_reg_num; i++){
           if(c->global_reg[i].str){
                 if(c->global_reg[i].value){
                 	//jstrescapecpy(tmp_str, c->global_reg[i].str);
                 	sprintf(tmp_sql, "INSERT INTO `global_reg_value` (`char_id`, `str`, `value`) VALUES ('%d', '%s', '%s')", charid, jstrescapecpy(tmp_str,c->global_reg[i].str), jstrescapecpy(tmp_str2,c->global_reg[i].value));
	                if(mysql_query(&charsql_handle, tmp_sql)){
							ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	                }
                 }
           }
         }
*/

         //friendlist saving
         sprintf(tmp_sql, "DELETE FROM `friends` WHERE `char_id` = '%d'", charid);
	if(mysql_query(&charsql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
         for(i = 0; i < MAX_FRIENDS; i++){
         	if(c->friends[i].char_id > 0){
			sprintf(tmp_sql, "INSERT INTO `friends` (`char_id`, `friend_account`, `friend_id`) VALUES ('%d','%d','%d')", charid, c->friends[i].account_id, c->friends[i].char_id);
	                if(mysql_query(&charsql_handle, tmp_sql)){
							ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
							ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	                }
                 }
    	}

    ShowInfo("charsql_savechar(): saving of '%d' (%s) complete.\n", charid, c->name);
    return 0;
}

int charsave_load_scdata(int account_id, int char_id)
{	//Loads character's sc_data
	struct map_session_data *sd;

	sd = map_id2sd(account_id);
	if (!sd)
	{
		ShowError("charsave_load_scdata: Player of AID %d not found!\n", account_id);
		return -1;
	}
	if (sd->status.char_id != char_id)
	{
		ShowError("charsave_load_scdata: Receiving data for account %d, char id does not matches (%d != %d)!\n", account_id, sd->status.char_id, char_id);
		return -1;
	}
	sprintf(tmp_sql, "SELECT `type`, `tick`, `val1`, `val2`, `val3`, `val4` FROM `sc_data`"
		"WHERE `account_id`='%d' AND `char_id`='%d'", account_id, char_id);

	if(mysql_query(&charsql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return -1;
	}

	sql_res = mysql_store_result(&charsql_handle);
	if(sql_res)
	{
		while ((sql_row = mysql_fetch_row(sql_res)))
		{
			if (atoi(sql_row[1]) < 1)
			{	//Protection against invalid tick values. [Skotlex]
				ShowWarning("charsave_load_scdata: Received invalid duration (%d ms) for status change %d (character %s)\n", atoi(sql_row[1]), sd->status.name);
				continue;
			}

			status_change_start(&sd->bl, atoi(sql_row[0]), 10000, atoi(sql_row[2]), atoi(sql_row[3]),
				atoi(sql_row[4]), atoi(sql_row[5]), atoi(sql_row[1]), 15);
		}
	}

	//Once loaded, sc_data must be disposed.
	sprintf(tmp_sql, "DELETE FROM `sc_data` WHERE `account_id`='%d' AND `char_id`='%d'", account_id, char_id);
	if(mysql_query(&charsql_handle, tmp_sql)){
		ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}
	return 0;
}

void charsave_save_scdata(int account_id, int char_id, struct status_change* sc_data, int max_sc)
{	//Saves character's sc_data.
	int i,count =0;
	struct TimerData *timer;
	unsigned int tick = gettick();
	char *p = tmp_sql;

	p += sprintf(p, "INSERT INTO `sc_data` (`account_id`, `char_id`, `type`, `tick`, `val1`, `val2`, `val3`, `val4`) VALUES ");

	for(i = 0; i < max_sc; i++)
	{
		if (sc_data->data[i].timer == -1)
			continue;
		timer = get_timer(sc_data->data[i].timer);
		if (timer == NULL || timer->func != status_change_timer || DIFF_TICK(timer->tick,tick) < 0)
			continue;

		p += sprintf(p, " ('%d','%d','%hu','%d','%d','%d','%d','%d'),", account_id, char_id,
			i, DIFF_TICK(timer->tick,tick), sc_data->data[i].val1, sc_data->data[i].val2, sc_data->data[i].val3, sc_data->data[i].val4);

		count++;
	}
	if (count > 0)
	{
		*--p = '\0'; //Remove the trailing comma.
		if(mysql_query(&charsql_handle, tmp_sql)){
			ShowSQL("DB error - %s\n",mysql_error(&charsql_handle));
			ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		}
	}
	ShowInfo("charsql_save_scdata(): saved %d status changes of '%d:%d'.\n", count, account_id, char_id);
	return;
}
#endif
