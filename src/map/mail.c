// Mail System for eAthena SQL
// Created by Valaris

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socket.h"
#include "timer.h"
#include "nullpo.h"

#include "map.h"
#include "clif.h"
#include "chrif.h"
#include "intif.h"
#include "pc.h"
#include "mail.h"

char mail_db[32] = "mail";

int MAIL_CHECK_TIME = 120000;
int mail_timer;

#ifdef MEMWATCH
#include "memwatch.h"
#endif

int mail_check(struct map_session_data *sd,int type)
{
	int i=0,new=0,priority=0;
	char message[50];

	if(sd==NULL)
		return 0;
	
	sprintf(tmp_msql,"SELECT `message_id`,`to_account_id`,`from_char_name`,`read_flag`,`priority`,`check_flag` FROM `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id`", mail_db, sd->status.account_id);

	if (mysql_query(&mail_handle, tmp_msql)) {
		printf("Database server error (executing query for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
   	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
	        if (mysql_num_rows(mail_res) == 0) {
			clif_displaymessage(sd->fd,"You have no messages.");
		        mysql_free_result(mail_res);
		        return 0;
	        }

	        while ((mail_row = mysql_fetch_row(mail_res))) {
				i++;

				if(!atoi(mail_row[5])) {
				sprintf(tmp_msql,"UPDATE `%s` SET `check_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
				        if(mysql_query(&mail_handle, tmp_msql) ) {
						printf("DB server Error (update Read `%s`)- %s\n", mail_db, mysql_error(&mail_handle) );
				        }
				}

				if(!atoi(mail_row[3])) {
					new++;
					if(atoi(mail_row[4]))
						priority++;
					if(type==2 || type==3) {
						if(atoi(mail_row[4])) {
							sprintf(message, "%d - From : %s (New - Priority)", i, mail_row[2]);
							clif_displaymessage(sd->fd, message);
						}

						else {
							sprintf(message, "%d - From : %s (New)", i, mail_row[2]);
							clif_displaymessage(sd->fd, message);
						}
					}
				}

					else if(type==2){
						sprintf(message, "%d - From : %s", i, mail_row[2]);
						clif_displaymessage(sd->fd, message);
					}
				
	        }
    	
		mysql_free_result(mail_res);
	
	} else {
        	printf("MySQL error (storing query result for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
    	}

	if(i>0 && new>0 && type==1) {
		sprintf(message, "You have %d new messages.", new);
		clif_displaymessage(sd->fd, message);
	}
	if(i>0 && new>0 && priority>0 && type==1) {
		sprintf(message, "You have %d unread priority messages.", priority);
		clif_displaymessage(sd->fd, message);
	}
	if(!new) {
		clif_displaymessage(sd->fd, "You have no new messages.");
	}

	return 0;
}

int mail_read(struct map_session_data *sd, int message_id)
{

	char message[80];

	if(sd==NULL)
		return 0;
	
	sprintf(tmp_msql,"SELECT `message_id`,`to_account_id`,`from_char_name`,`message`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id` LIMIT %d, 1",mail_db,sd->status.account_id,message_id-1);

	if (mysql_query(&mail_handle, tmp_msql)) {
		printf("Database server error (executing query for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
   	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
	        if (mysql_num_rows(mail_res) == 0) {
		        mysql_free_result(mail_res);
			clif_displaymessage(sd->fd, "Message not found.");
		        return 0;
	        }

	        if ((mail_row = mysql_fetch_row(mail_res))) {

			if(!atoi(mail_row[6])) {
				sprintf(tmp_msql,"UPDATE `%s` SET `check_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
			        if(mysql_query(&mail_handle, tmp_msql) ) {
						printf("DB server Error (update Read `%s`)- %s\n", mail_db, mysql_error(&mail_handle) );
				        }
			}

			sprintf(message, "Reading message from %s", mail_row[2]);
			clif_displaymessage(sd->fd, message);

			sprintf(message, "%s", mail_row[3]);
			clif_displaymessage(sd->fd, message);

			sprintf(tmp_msql,"UPDATE `%s` SET `read_flag`='1' WHERE `message_id`= \"%d\"", mail_db, atoi(mail_row[0]));
		        if(mysql_query(&mail_handle, tmp_msql) ) {
				printf("DB server Error (update Read `%s`)- %s\n", mail_db, mysql_error(&mail_handle) );
		        }
		}
	
		mysql_free_result(mail_res);
	
	} else {
        	printf("MySQL error (storing query result for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
    	}

	return 0;
}

int mail_delete(struct map_session_data *sd, int message_id)
{
	if(sd==NULL)
		return 0;
	
	sprintf(tmp_msql,"SELECT `message_id`,`to_account_id`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = \"%d\" ORDER by `message_id` LIMIT %d, 1",mail_db,sd->status.account_id,message_id-1);

	if (mysql_query(&mail_handle, tmp_msql)) {
		printf("Database server error (executing query for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
   	}

   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
	        if (mysql_num_rows(mail_res) == 0) {
		        mysql_free_result(mail_res);
			clif_displaymessage(sd->fd, "Message not found.");
		        return 0;
	        }

	        if ((mail_row = mysql_fetch_row(mail_res))) {
			if(!atoi(mail_row[2]) && atoi(mail_row[3])) {
				mysql_free_result(mail_res);
				clif_displaymessage(sd->fd,"Cannot delete unread priority mail.");
				return 0;
			}
			if(!atoi(mail_row[4])) {
				mysql_free_result(mail_res);
				clif_displaymessage(sd->fd,"You have recieved new mail, use @listmail before deleting.");
				return 0;
			}
			sprintf(tmp_msql,"DELETE FROM `%s` WHERE `message_id` = \"%d\"", mail_db, atoi(mail_row[0]));
		        if(mysql_query(&mail_handle, tmp_msql) ) {
				mysql_free_result(mail_res);
				printf("DB server Error (update Read `%s`)- %s\n", mail_db, mysql_error(&mail_handle) );
				return 0;
		        }
			else clif_displaymessage(sd->fd,"Message deleted.");
		}
	
		mysql_free_result(mail_res);
	
	} else {
        	printf("MySQL error (delete query result for %s): %s\n", mail_db, mysql_error(&mail_handle));
		return 0;
    	}

	return 0;
}

int mail_send(struct map_session_data *sd, char *name, char *message, int flag)
{
	if(sd==NULL)
		return 0;
		
	if(pc_isGM(sd) < 80 && sd->mail_counter > 0) {
		clif_displaymessage(sd->fd,"You must wait 10 minutes before sending another message");
		return 0;
	}

	if(strcmp(name,"*")==0) {
		if(pc_isGM(sd) < 80) {
			clif_displaymessage(sd->fd, "Access Denied.");
			return 0;
		}
		else
			sprintf(tmp_msql,"SELECT DISTINCT `account_id` FROM `%s` WHERE `account_id` <> '%d' ORDER BY `account_id`", char_db, sd->status.account_id);
	}
	else
		sprintf(tmp_msql,"SELECT `account_id`,`name` FROM `%s` WHERE `name` = \"%s\"", char_db, name);

	if (mysql_query(&mail_handle, tmp_msql)) {
		printf("Database server error (executing query for %s): %s\n", char_db, mysql_error(&mail_handle));
		return 0;
   	}	
	
   	mail_res = mysql_store_result(&mail_handle);
	if(mail_res) {
	        if (mysql_num_rows(mail_res) == 0) {
			mysql_free_result(mail_res);
			clif_displaymessage(sd->fd,"Character does not exist.");
		        return 0;
	        }

	        while ((mail_row = mysql_fetch_row(mail_res))) {
			if(strcmp(name,"*")==0) {
				sprintf(tmp_msql, "INSERT INTO `%s` (`to_account_id`,`from_account_id`,`from_char_name`,`message`,`priority`)"
					" VALUES ('%d', '%d', '%s', '%s', '%d')",mail_db, atoi(mail_row[0]), sd->status.account_id, sd->status.name, message, flag);			
			}
			else {
				sprintf(tmp_msql, "INSERT INTO `%s` (`to_account_id`,`to_char_name`,`from_account_id`,`from_char_name`,`message`,`priority`)"
					" VALUES ('%d', '%s', '%d', '%s', '%s', '%d')",mail_db, atoi(mail_row[0]), mail_row[1], sd->status.account_id, sd->status.name, message, flag);
				if(pc_isGM(sd) < 80)
					sd->mail_counter=5;
			}
	
			if(mysql_query(&mail_handle, tmp_msql) ) {
				mysql_free_result(mail_res);
				printf("DB server Error (insert `mail_db`)- %s\n", mysql_error(&mail_handle) );
				return 0;
			}

		}
	}

	clif_displaymessage(sd->fd,"Mail has been sent.");

	return 0;
}

int mail_check_timer(int tid,unsigned int tick,int id,int data)
{
	struct map_session_data *sd = NULL;
	int i;		

	if(mail_timer != tid)
		return 0;

	sprintf(tmp_msql,"SELECT DISTINCT `to_account_id` FROM `%s` WHERE `read_flag` = '0' AND `check_flag` = '0'", mail_db);
	
	if (mysql_query(&mail_handle, tmp_msql)) {
		printf("Database server error (executing query for %s): %s\n", char_db, mysql_error(&mail_handle));
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

	       while ((mail_row = mysql_fetch_row(mail_res))) {
			for (i = 0; i < fd_max; i++) {
				if (session[i] && (sd = session[i]->session_data) && sd->state.auth){
					if(pc_isGM(sd) < 80 && sd->mail_counter > 0)
						sd->mail_counter--;
					if(sd->status.account_id==atoi(mail_row[0]))
						clif_displaymessage(sd->fd, "You have new mail.");						
				}
			}
		}
	}

	sprintf(tmp_msql,"UPDATE `%s` SET `check_flag`='1' WHERE `check_flag`= '0' ", mail_db);
        if(mysql_query(&mail_handle, tmp_msql) ) {
		printf("DB server Error (update Read `%s`)- %s\n", mail_db, mysql_error(&mail_handle) );
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

