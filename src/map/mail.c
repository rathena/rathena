// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TXT_ONLY
// Mail System for eAthena SQL
// Created by Valaris
// moved all strings to msg_athena.conf [Lupus]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/strlib.h"
#include "../common/socket.h"
#include "../common/timer.h"
#include "../common/nullpo.h"
#include "../common/showmsg.h"

#include "map.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "atcommand.h"
#include "pc.h"
#include "mail.h"

#ifndef TXT_ONLY
	#ifndef SQL_DEBUG

		#define mysql_query(_x, _y) mysql_real_query(_x, _y, strlen(_y))

	#else 

		#define mysql_query(_x, _y) debug_mysql_query(__FILE__, __LINE__, _x, _y)

	#endif
#endif

int MAIL_CHECK_TIME = 120000;
int mail_timer;
//extern char *msg_table[1000]; // Server messages (0-499 reserved for GM commands, 500-999 reserved for others)

int mail_check(struct map_session_data *sd,int type)
{
	int i = 0, new_ = 0, priority = 0;
	char message[80];

	nullpo_retr (0, sd);

	sprintf(tmp_sql,"SELECT `message_id`,`to_account_id`,`from_char_name`,`read_flag`,`priority`,`check_flag` "
		"FROM `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id`", mail_db, sd->status.account_id);

	if (mysql_query(&mail_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
		if (mysql_num_rows(mail_res) == 0) {
			//clif_displaymessage(sd->fd,"You have no messages.");
			clif_displaymessage(sd->fd, msg_txt(516));

			mysql_free_result(mail_res);
			return 0;
		}

		while ((mail_row = mysql_fetch_row(mail_res))) {
			i++;
			if(!atoi(mail_row[5])) {
				sprintf(tmp_sql,"UPDATE `%s` SET `check_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
				if(mysql_query(&mail_handle, tmp_sql) ) {
					ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				}
			}

			if(!atoi(mail_row[3])) {
				new_++;
				if(atoi(mail_row[4]))
					priority++;
				if(type==2 || type==3) {
					if(atoi(mail_row[4])) {
						snprintf(message, 80, msg_txt(511), i, mail_row[2]);
						clif_displaymessage(sd->fd, message);
					} else {
						//sprintf(message, "%d - From : %s (New)", i, mail_row[2]);
						snprintf(message, 80, msg_txt(512), i, mail_row[2]);
						clif_displaymessage(sd->fd, message);
					}
				}
			} else if(type==2){
				snprintf(message, 80, msg_txt(513), i, mail_row[2]);
				clif_displaymessage(sd->fd, message);
			}
		}

		mysql_free_result(mail_res);
	} else {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
    }

	if(i>0 && new_>0 && type==1) {
		//sprintf(message, "You have %d new messages.", new_);
		sprintf(message, msg_txt(514), new_);

		clif_displaymessage(sd->fd, jstrescape(message));
	}
	if(i>0 && new_>0 && priority>0 && type==1) {
		//sprintf(message, "You have %d unread priority messages.", priority);
		sprintf(message, msg_txt(515), priority);
		clif_displaymessage(sd->fd, jstrescape(message));
	}
	if(!new_) {
		//clif_displaymessage(sd->fd, "You have no new messages.");
		clif_displaymessage(sd->fd, msg_txt(516));
	}

	return 0;
}

int mail_read(struct map_session_data *sd, int message_id)
{

	char message[80];

	nullpo_retr (0, sd);

	sprintf(tmp_sql,"SELECT `message_id`,`to_account_id`,`from_char_name`,`message`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id` LIMIT %d, 1",mail_db,sd->status.account_id,message_id-1);

	if (mysql_query(&mail_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
   	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
		if (mysql_num_rows(mail_res) == 0) {
			mysql_free_result(mail_res);
			//clif_displaymessage(sd->fd, "Message not found.");
			clif_displaymessage(sd->fd, msg_txt(517));
		       return 0;
		}

		if ((mail_row = mysql_fetch_row(mail_res))) {
			if(!atoi(mail_row[6])) {
				sprintf(tmp_sql,"UPDATE `%s` SET `check_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
				if(mysql_query(&mail_handle, tmp_sql) ) {
					ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
					ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				}
			}

			//sprintf(message, "Reading message from %s", mail_row[2]);
			sprintf(message, msg_txt(518), mail_row[2]);
			clif_displaymessage(sd->fd, jstrescape(message));

			sprintf(message, "%s", mail_row[3]);
			clif_displaymessage(sd->fd, jstrescape(message));

			sprintf(tmp_sql,"UPDATE `%s` SET `read_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
			if(mysql_query(&mail_handle, tmp_sql) ) {
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
			}
		}

		mysql_free_result(mail_res);

	} else {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
    }

	return 0;
}

int mail_delete(struct map_session_data *sd, int message_id)
{
	nullpo_retr (0, sd);

	sprintf(tmp_sql,"SELECT `message_id`,`to_account_id`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id` LIMIT %d, 1",mail_db,sd->status.account_id,message_id-1);

	if (mysql_query(&mail_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
   	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
		if (mysql_num_rows(mail_res) == 0) {
			mysql_free_result(mail_res);
			//clif_displaymessage(sd->fd, "Message not found.");
			clif_displaymessage(sd->fd, msg_txt(517));
			return 0;
		}

		if ((mail_row = mysql_fetch_row(mail_res))) {
			if(!atoi(mail_row[2]) && atoi(mail_row[3])) {
				mysql_free_result(mail_res);
				//clif_displaymessage(sd->fd,"Cannot delete unread priority mail.");
				clif_displaymessage(sd->fd,msg_txt(519));

				return 0;
			}
			if(!atoi(mail_row[4])) {
				mysql_free_result(mail_res);
				//clif_displaymessage(sd->fd,"You have recieved new mail, use @listmail before deleting.");
				clif_displaymessage(sd->fd,msg_txt(520));
				return 0;
			}
			sprintf(tmp_sql,"DELETE FROM `%s` WHERE `message_id` = \"%d\"", mail_db, atoi(mail_row[0]));
			if(mysql_query(&mail_handle, tmp_sql) ) {
				mysql_free_result(mail_res);
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				return 0;
			}
			//else clif_displaymessage(sd->fd,"Message deleted.");
			else clif_displaymessage(sd->fd,msg_txt(521));
		}

		mysql_free_result(mail_res);

	} else {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}

	return 0;
}

int mail_send(struct map_session_data *sd, char *name, char *message, int flag)
{
	nullpo_retr (0, sd);

	if(pc_isGM(sd) < 80 && sd->mail_counter > 0) {
		//clif_displaymessage(sd->fd,"You must wait 10 minutes before sending another message");
		clif_displaymessage(sd->fd,msg_txt(522));
		return 0;
	}

	if(strcmp(name,"*")==0) {
		if(pc_isGM(sd) < 80) {
			//clif_displaymessage(sd->fd, "Access Denied.");
			clif_displaymessage(sd->fd, msg_txt(523));
			return 0;
		}
		else
			sprintf(tmp_sql,"SELECT DISTINCT `account_id` FROM `%s` WHERE `account_id` <> '%d' ORDER BY `account_id`", char_db, sd->status.account_id);
	}
	else
		sprintf(tmp_sql,"SELECT `account_id`,`name` FROM `%s` WHERE `name` = \"%s\"", char_db, jstrescape(name));

	if (mysql_query(&mail_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		return 0;
  	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
		if (mysql_num_rows(mail_res) == 0) {
			mysql_free_result(mail_res);
			//clif_displaymessage(sd->fd,"Character does not exist.");
			clif_displaymessage(sd->fd,msg_txt(524));
			return 0;
		}

		while ((mail_row = mysql_fetch_row(mail_res))) {
			if(strcmp(name,"*")==0) {
				sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`to_account_id`,`from_account_id`,`from_char_name`,`message`,`priority`)"
					" VALUES ('%d', '%d', '%s', '%s', '%d')",mail_db, atoi(mail_row[0]), sd->status.account_id, sd->status.name, jstrescape(message), flag);
			}
			else {
				sprintf(tmp_sql, "INSERT DELAYED INTO `%s` (`to_account_id`,`to_char_name`,`from_account_id`,`from_char_name`,`message`,`priority`)"
					" VALUES ('%d', '%s', '%d', '%s', '%s', '%d')",mail_db, atoi(mail_row[0]), mail_row[1], sd->status.account_id, sd->status.name, jstrescape(message), flag);
				if(pc_isGM(sd) < 80)
					sd->mail_counter=5;
			}

			if(mysql_query(&mail_handle, tmp_sql) ) {
				mysql_free_result(mail_res);
				ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
				ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
				return 0;
			}
		}
	}

	//clif_displaymessage(sd->fd,"Mail has been sent.");
	clif_displaymessage(sd->fd,msg_txt(525));

	return 0;
}

static int mail_check_timer_sub(struct map_session_data *sd, va_list va)
{
	int id = va_arg(va, int);
	if(pc_isGM(sd) < 80 && sd->mail_counter > 0)
		sd->mail_counter--;
	if(sd->status.account_id==id)
		clif_displaymessage(sd->fd, msg_txt(526)); //you got new email.
	return 0;
}

int mail_check_timer(int tid,unsigned int tick,int id,int data)
{
	if(mail_timer != tid)
		return 0;

	sprintf(tmp_sql,"SELECT DISTINCT `to_account_id` FROM `%s` WHERE `read_flag` = '0' AND `check_flag` = '0'", mail_db);

	if (mysql_query(&mail_handle, tmp_sql)) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
		mail_timer=add_timer(gettick()+MAIL_CHECK_TIME,mail_check_timer,0,0);
		return 0;
   	}

	mail_res = mysql_store_result(&mail_handle);

	if (mail_res) {
		if (mysql_num_rows(mail_res) == 0) {
			mysql_free_result(mail_res);
			mail_timer=add_timer(gettick()+MAIL_CHECK_TIME,mail_check_timer,0,0);
			return 0;
		}

		while ((mail_row = mysql_fetch_row(mail_res)))
			clif_foreachclient(mail_check_timer_sub, atoi(mail_row[0]));
	}

	sprintf(tmp_sql,"UPDATE `%s` SET `check_flag`='1' WHERE `check_flag`= '0' ", mail_db);
	if(mysql_query(&mail_handle, tmp_sql) ) {
		ShowSQL("DB error - %s\n",mysql_error(&mail_handle));
		ShowDebug("at %s:%d - %s\n", __FILE__,__LINE__,tmp_sql);
	}

	mail_timer=add_timer(gettick()+MAIL_CHECK_TIME,mail_check_timer,0,0);
	return 0;
}

int do_init_mail(void)
{
	add_timer_func_list(mail_check_timer,"mail_check_timer");
	mail_timer=add_timer(gettick()+MAIL_CHECK_TIME,mail_check_timer,0,0);
	return 0;
}

#endif
