// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef TXT_ONLY

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int MAIL_CHECK_TIME = 120000;
int mail_timer;

/// type: 0 - mail check at login, silent if there aren't any new messages
///       1 - @checkmail, just print the number of messages
///       2 - @listmail, shows both read and unread messages
///       3 - @listnewmail, shows only unread messages
int mail_check(struct map_session_data* sd, int type)
{
	int i = 0, new_ = 0, priority = 0; // counters
	char message[80];

	nullpo_retr (0, sd);

	// retrieve all existing messages for this player
	if( SQL_ERROR == Sql_Query(mail_handle, "SELECT `message_id`,`to_account_id`,`from_char_name`,`read_flag`,`priority`,`check_flag` FROM `%s` WHERE `to_account_id` = %d ORDER by `message_id`", mail_db, sd->status.account_id) )
	{
		Sql_ShowDebug(mail_handle);
		return 0;
	}

	if( Sql_NumRows(mail_handle) == 0)
	{
		clif_displaymessage(sd->fd, msg_txt(516)); // "You have no messages."
		Sql_FreeResult(mail_handle);
		return 0;
	}

	while( SQL_SUCCESS == Sql_NextRow(mail_handle) )
	{
		char* data;
		int message_id;
		int to_account_id;
		char from_char_name[NAME_LENGTH];
		bool read_flag, priority_flag, check_flag;

		Sql_GetData(mail_handle, 0, &data, NULL); message_id = atoi(data);
		Sql_GetData(mail_handle, 1, &data, NULL); to_account_id = atoi(data);
		Sql_GetData(mail_handle, 2, &data, NULL); safestrncpy(from_char_name, data, sizeof(from_char_name));
		Sql_GetData(mail_handle, 3, &data, NULL); read_flag = (bool) atoi(data);
		Sql_GetData(mail_handle, 4, &data, NULL); priority_flag = (bool) atoi(data);
		Sql_GetData(mail_handle, 5, &data, NULL); check_flag = (bool) atoi(data);

		i++;

		if( !check_flag )
		{
			// mark this message as checked
			if( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `check_flag` = 1 WHERE `message_id` = %d", mail_db, message_id) )
				Sql_ShowDebug(mail_handle);
		}

		if( !read_flag )
		{
			new_++;
			if(priority_flag)
				priority++;

			if( type == 2 || type == 3 )
			{
				if( priority_flag )
				{
					snprintf(message, 80, msg_txt(511), i, from_char_name);
					clif_displaymessage(sd->fd, message); // "%d - From : %s (New - Priority)"
				}
				else
				{
					snprintf(message, 80, msg_txt(512), i, from_char_name);
					clif_displaymessage(sd->fd, message); // "%d - From : %s (New)"
				}
			}
		}
		else if( type == 2 )
		{
			snprintf(message, 80, msg_txt(513), i, from_char_name);
			clif_displaymessage(sd->fd, message); // "%d - From : %s"
		}
	}

	Sql_FreeResult(mail_handle);
	
	if( new_ > 0 && (type == 0 || type == 1) )
	{
		sprintf(message, msg_txt(514), new_);
		clif_displaymessage(sd->fd, message); // "You have %d unread messages."
		if (priority > 0)
		{
			sprintf(message, msg_txt(515), priority);
			clif_displaymessage(sd->fd, message); // "You have %d unread priority messages."
		}
	}
	if( new_ == 0 && type != 0 )
	{
		clif_displaymessage(sd->fd, msg_txt(516)); // "You have no unread messages."
	}

	return 0;
}

/// displays the selected message
int mail_read(struct map_session_data* sd, int index)
{
	char* data;
	int message_id;
	char from_char_name[NAME_LENGTH];
	char message[80];
	bool read_flag, priority_flag, check_flag;
	char output[100];

	nullpo_retr(0, sd);

	// retrieve the 'index'-th message 
	if( SQL_ERROR == Sql_Query(mail_handle, "SELECT `message_id`,`from_char_name`,`message`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = %d ORDER by `message_id` LIMIT %d, 1", mail_db, sd->status.account_id, index-1) )
	{
		Sql_ShowDebug(mail_handle);
		return 0;
	}

	if( 0 == Sql_NumRows(mail_handle) )
	{
		clif_displaymessage(sd->fd, msg_txt(517)); // "Message not found."
		Sql_FreeResult(mail_handle);
		return 0;
	}

	if( SQL_ERROR == Sql_NextRow(mail_handle) )
	{
		Sql_ShowDebug(mail_handle);
		Sql_FreeResult(mail_handle);
		return 0;
	}

	Sql_GetData(mail_handle, 0, &data, NULL); message_id = atoi(data);
	Sql_GetData(mail_handle, 1, &data, NULL); safestrncpy(from_char_name, data, sizeof(from_char_name));
	Sql_GetData(mail_handle, 2, &data, NULL); safestrncpy(message, data, sizeof(message));
	Sql_GetData(mail_handle, 3, &data, NULL); read_flag = (bool) atoi(data);
	Sql_GetData(mail_handle, 4, &data, NULL); priority_flag = (bool) atoi(data);
	Sql_GetData(mail_handle, 5, &data, NULL); check_flag = (bool) atoi(data);

	Sql_FreeResult(mail_handle);

	// mark mail as checked
	if( !check_flag )
	{
		if( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `check_flag` = 1 WHERE `message_id` = %d", mail_db, message_id) )
		Sql_ShowDebug(mail_handle);
	}

	sprintf(output, msg_txt(518), from_char_name);
	clif_displaymessage(sd->fd, output); // "Reading message from %s"
	clif_displaymessage(sd->fd, message);

	if( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `read_flag` = 1 WHERE `message_id` = %d", mail_db, message_id) )
	{
		Sql_ShowDebug(mail_handle);
	}

	return 0;
}

/// message deletion
int mail_delete(struct map_session_data* sd, int index)
{
	char* data;
	int message_id;
	bool read_flag, priority_flag, check_flag;

	nullpo_retr (0, sd);

	if( SQL_ERROR == Sql_Query(mail_handle, "SELECT `message_id`,`read_flag`,`priority`,`check_flag` from `%s` WHERE `to_account_id` = %d ORDER by `message_id` LIMIT %d, 1", mail_db, sd->status.account_id, index-1) )
	{
		Sql_ShowDebug(mail_handle);
		return 0;
	}

	if( 0 == Sql_NumRows(mail_handle) )
	{
		clif_displaymessage(sd->fd, msg_txt(517)); // "Message not found."
		Sql_FreeResult(mail_handle);
		return 0;
	}

	if( SQL_ERROR == Sql_NextRow(mail_handle) )
	{
		Sql_ShowDebug(mail_handle);
		Sql_FreeResult(mail_handle);
		return 0;
	}

	Sql_GetData(mail_handle, 0, &data, NULL); message_id = atoi(data);
	Sql_GetData(mail_handle, 1, &data, NULL); read_flag = (bool)atoi(data);
	Sql_GetData(mail_handle, 2, &data, NULL); priority_flag = (bool)atoi(data);
	Sql_GetData(mail_handle, 3, &data, NULL); check_flag = (bool)atoi(data);

	Sql_FreeResult(mail_handle);

	if( !read_flag && priority_flag )
	{
		clif_displaymessage(sd->fd,msg_txt(519)); // "Cannot delete unread priority mail."
		return 0;
	}

	if( !check_flag )
	{
		clif_displaymessage(sd->fd,msg_txt(520)); // "You have received new mail, use @listmail before deleting."
		return 0;
	}

	if( SQL_ERROR == Sql_Query(mail_handle, "DELETE FROM `%s` WHERE `message_id` = %d", mail_db, message_id) )
	{
		Sql_ShowDebug(mail_handle);
		return 0;
	}

	clif_displaymessage(sd->fd,msg_txt(521)); // "Message deleted."

	return 0;
}

/// for sending normal and priority messages
int mail_send(struct map_session_data* sd, char* name, char* message, int flag)
{
	SqlStmt* stmt;

	nullpo_retr (0, sd);

	if( pc_isGM(sd) < 80 && sd->mail_counter > 0 )
	{
		clif_displaymessage(sd->fd,msg_txt(522)); // "You must wait 10 minutes before sending another message"
		return 0;
	}

	if( strcmp(name,"*") == 0 && pc_isGM(sd) < 80 )
	{
		clif_displaymessage(sd->fd, msg_txt(523)); // "Access Denied."
		return 0;
	}

	if( strcmp(name,"*") == 0 )
	{
		if( SQL_ERROR == Sql_Query(mail_handle, "SELECT DISTINCT `account_id` FROM `%s` WHERE `account_id` != '%d' ORDER BY `account_id`", char_db, sd->status.account_id) )
		{
			Sql_ShowDebug(mail_handle);
			return 0;
		}
	}
	else
	{
		char name_[2*NAME_LENGTH];
		Sql_EscapeString(mail_handle, name_, name);
		if( SQL_ERROR == Sql_Query(mail_handle, "SELECT `account_id` FROM `%s` WHERE `name` = '%s'", char_db, name_) )
		{
			Sql_ShowDebug(mail_handle);
			return 0;
		}
	}

	if( 0 == Sql_NumRows(mail_handle) )
	{
		clif_displaymessage(sd->fd,msg_txt(524)); // "Character does not exist."
		Sql_FreeResult(mail_handle);
		return 0;
	}

	stmt = SqlStmt_Malloc(mail_handle);
	SqlStmt_Prepare(stmt, "INSERT DELAYED INTO `%s` (`to_account_id`,`to_char_name`,`from_account_id`,`from_char_name`,`message`,`priority`) VALUES (?, ?, '%d', ?, ?, '%d')", mail_db, sd->status.account_id, flag);
	SqlStmt_BindParam(stmt, 1, SQLDT_STRING, name, strnlen(name, NAME_LENGTH));
	SqlStmt_BindParam(stmt, 2, SQLDT_STRING, sd->status.name, strnlen(sd->status.name, NAME_LENGTH));
	SqlStmt_BindParam(stmt, 3, SQLDT_STRING, message, strnlen(message, 80));

	while( SQL_SUCCESS == Sql_NextRow(mail_handle) )
	{
		int id;
		char* data;
		Sql_GetData(mail_handle, 0, &data, NULL); id = atoi(data);
		SqlStmt_BindParam(stmt, 0, SQLDT_INT, &id, sizeof(id));
		if( SQL_ERROR == SqlStmt_Execute(stmt) )
			SqlStmt_ShowDebug(stmt);
	}
	Sql_FreeResult(mail_handle);
	SqlStmt_Free(stmt);

	if(pc_isGM(sd) < 80)
		sd->mail_counter = 5;

	clif_displaymessage(sd->fd,msg_txt(525)); // "Mail has been sent."

	return 0;
}

/// invoked every MAIL_CHECK_TIME ms, decreases the send blocking counter
static int mail_check_timer_sub(struct map_session_data* sd, va_list va)
{
	if(sd->mail_counter > 0)
		sd->mail_counter--;
	return 0;
}

/// periodically checks for new messages and notifies about them
int mail_check_timer(int tid, unsigned int tick, int id, int data)
{
	if(mail_timer != tid)
		return 0;

	mail_timer = add_timer(gettick() + MAIL_CHECK_TIME, mail_check_timer, 0, 0);

	// fetch account ids of people who received new mail since the last iteration
	if( SQL_ERROR == Sql_Query(mail_handle, "SELECT DISTINCT `to_account_id` FROM `%s` WHERE `read_flag` = '0' AND `check_flag` = '0'", mail_db) )
	{
		Sql_ShowDebug(mail_handle);
		return 0;
	}

	while( SQL_SUCCESS == Sql_NextRow(mail_handle) )
	{
		char* id;
		struct map_session_data* sd;
		Sql_GetData(mail_handle, 0, &id, NULL);
		if( (sd = map_id2sd(atoi(id))) != NULL )
			clif_displaymessage(sd->fd, msg_txt(526)); // "You have new mail."
	}

	Sql_FreeResult(mail_handle);

	// decrease the send-blocking counter
	clif_foreachclient(mail_check_timer_sub);

	// The 'check_flag' indicates whether the player was informed about the message.
	// All online players were notified by the above code, and offline players will get the notice at next login.
	// Therefore it is safe to simply set the flag to '1' for all existing mails.
	if( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `check_flag` = 1 WHERE `check_flag` = 0", mail_db) )
		Sql_ShowDebug(mail_handle);

	return 0;
}

int do_init_mail(void)
{
	add_timer_func_list(mail_check_timer, "mail_check_timer");
	mail_timer = add_timer(gettick() + MAIL_CHECK_TIME, mail_check_timer, 0, 0);

	return 0;
}

#endif
