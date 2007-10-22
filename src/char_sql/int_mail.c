// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct mail_data *mail_data_pt = NULL;

time_t calc_times(void)
{
	time_t temp = time(NULL);
	return mktime(localtime(&temp));
}

int mail_fromsql(int char_id, struct mail_data *md)
{
	int i, j;
	struct mail_message *msg;
	struct item *item;
	char *data;
	StringBuf buf;

	memset(md, 0, sizeof(struct mail_data));
	md->amount = 0;
	md->satured = 0;

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`read_flag`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`");
	for (i = 0; i < MAX_SLOTS; i++)
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `dest_id`='%d' ORDER BY `id` LIMIT %d", mail_db, char_id, MAX_MAIL_INBOX + 1);

	if( SQL_ERROR == Sql_Query(mail_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(mail_handle);

	StringBuf_Destroy(&buf);

	for (i = 0; i < MAX_MAIL_INBOX && SQL_SUCCESS == Sql_NextRow(mail_handle); ++i )
	{
		msg = &md->msg[i];
		Sql_GetData(mail_handle, 0, &data, NULL); msg->id = atoi(data);
		Sql_GetData(mail_handle, 1, &data, NULL); safestrncpy(msg->send_name, data, NAME_LENGTH);
		Sql_GetData(mail_handle, 2, &data, NULL); msg->send_id = atoi(data);
		Sql_GetData(mail_handle, 3, &data, NULL); safestrncpy(msg->dest_name, data, NAME_LENGTH);
		Sql_GetData(mail_handle, 4, &data, NULL); msg->dest_id = atoi(data);
		Sql_GetData(mail_handle, 5, &data, NULL); safestrncpy(msg->title, data, MAIL_TITLE_LENGTH);
		Sql_GetData(mail_handle, 6, &data, NULL); safestrncpy(msg->body, data, MAIL_BODY_LENGTH);
		Sql_GetData(mail_handle, 7, &data, NULL); msg->timestamp = atoi(data);
		Sql_GetData(mail_handle, 8, &data, NULL); msg->read = atoi(data);
		Sql_GetData(mail_handle, 9, &data, NULL); msg->zeny = atoi(data);
		item = &msg->item;
		Sql_GetData(mail_handle,10, &data, NULL); item->amount = (short)atoi(data);
		Sql_GetData(mail_handle,11, &data, NULL); item->nameid = atoi(data);
		Sql_GetData(mail_handle,12, &data, NULL); item->refine = atoi(data);
		Sql_GetData(mail_handle,13, &data, NULL); item->attribute = atoi(data);
		Sql_GetData(mail_handle,14, &data, NULL); item->identify = atoi(data);

		for (j = 0; j < MAX_SLOTS; j++)
		{
			Sql_GetData(mail_handle, 15 + j, &data, NULL);
			item->card[j] = atoi(data);
		}
	}

	if ( SQL_SUCCESS == Sql_NextRow(mail_handle) )
		md->satured = 1; // New Mails cannot arrive
	else
		md->satured = 0;

	md->amount = i;
	md->changed = 0;
	Sql_FreeResult(mail_handle);

	md->unchecked = 0;
	md->unreaded = 0;
	for (i = 0; i < md->amount; i++)
	{
		msg = &md->msg[i];
		if (!msg->read)
		{
			if ( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `read_flag` = '1' WHERE `id` = '%d'", mail_db, msg->id) )
				Sql_ShowDebug(mail_handle);

			md->unchecked++;
		}
		else if ( msg->read == 1 )
			md->unreaded++;

		msg->read = (msg->read < 2)?0:1;
	}

	ShowInfo("mail load complete from DB - id: %d (total: %d)\n", char_id, md->amount);
	return 1;
}

int mail_savemessage(struct mail_message *msg)
{
	StringBuf buf;
	int j;
	char esc_send_name[NAME_LENGTH*2+1], esc_dest_name[NAME_LENGTH*2+1];
	char esc_title[MAIL_TITLE_LENGTH*2+1], esc_body[MAIL_BODY_LENGTH*2+1];
	
	if (!msg)
		return 0;

	Sql_EscapeStringLen(mail_handle, esc_send_name, msg->send_name, strnlen(msg->send_name, NAME_LENGTH));
	Sql_EscapeStringLen(mail_handle, esc_dest_name, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH));
	Sql_EscapeStringLen(mail_handle, esc_title, msg->title, strnlen(msg->title, MAIL_TITLE_LENGTH));
	Sql_EscapeStringLen(mail_handle, esc_body, msg->body, strnlen(msg->body, MAIL_BODY_LENGTH));

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`send_name`, `send_id`, `dest_name`, `dest_id`, `title`, `message`, `time`, `read_flag`, `zeny`, `amount`, `nameid`, `refine`, `attribute`, `identify`", mail_db);
	for (j = 0; j < MAX_SLOTS; j++)
		StringBuf_Printf(&buf, ", `card%d`", j);
	StringBuf_Printf(&buf, ") VALUES ('%s', '%d', '%s', '%d', '%s', '%s', '%d', '0', '%d', '%d', '%d', '%d', '%d', '%d'",
		esc_send_name, msg->send_id, esc_dest_name, msg->dest_id, esc_title, esc_body, msg->timestamp, msg->zeny, msg->item.amount, msg->item.nameid, msg->item.refine, msg->item.attribute, msg->item.identify);
	for (j = 0; j < MAX_SLOTS; j++)
		StringBuf_Printf(&buf, ", '%d'", msg->item.card[j]);
	StringBuf_AppendStr(&buf, ")");

	if( SQL_ERROR == Sql_QueryStr(mail_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(mail_handle);
		j = 0;
	}
	else
		j = (int)Sql_LastInsertId(mail_handle);

	StringBuf_Destroy(&buf);

	return j;
}

int mail_loadmessage(int char_id, int mail_id, struct mail_message *message, short flag)
{
	char *data;
	struct item *item;
	int j = 0;
	StringBuf buf;

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`,`send_name`,`send_id`,`dest_name`,`dest_id`,`title`,`message`,`time`,`read_flag`,"
		"`zeny`,`amount`,`nameid`,`refine`,`attribute`,`identify`");
	for (j = 0; j < MAX_SLOTS; j++)
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `dest_id` = '%d' AND `id` = '%d'", mail_db, char_id, mail_id);

	if( SQL_ERROR == Sql_Query(mail_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(mail_handle);
	else if( Sql_NumRows(mail_handle) == 0 )
		ShowWarning("Char %d trying to read an invalid mail.\n", char_id);
	else
	{
		Sql_NextRow(mail_handle);

		Sql_GetData(mail_handle, 0, &data, NULL); message->id = atoi(data);
		Sql_GetData(mail_handle, 1, &data, NULL); safestrncpy(message->send_name, data, NAME_LENGTH);
		Sql_GetData(mail_handle, 2, &data, NULL); message->send_id = atoi(data);
		Sql_GetData(mail_handle, 3, &data, NULL); safestrncpy(message->dest_name, data, NAME_LENGTH);
		Sql_GetData(mail_handle, 4, &data, NULL); message->dest_id = atoi(data);
		Sql_GetData(mail_handle, 5, &data, NULL); safestrncpy(message->title, data, MAIL_TITLE_LENGTH);
		Sql_GetData(mail_handle, 6, &data, NULL); safestrncpy(message->body, data, MAIL_BODY_LENGTH);
		Sql_GetData(mail_handle, 7, &data, NULL); message->timestamp = atoi(data);
		Sql_GetData(mail_handle, 8, &data, NULL); message->read = atoi(data);
		Sql_GetData(mail_handle, 9, &data, NULL); message->zeny = atoi(data);
		item = &message->item;
		Sql_GetData(mail_handle,10, &data, NULL); item->amount = (short)atoi(data);
		Sql_GetData(mail_handle,11, &data, NULL); item->nameid = atoi(data);
		Sql_GetData(mail_handle,12, &data, NULL); item->refine = atoi(data);
		Sql_GetData(mail_handle,13, &data, NULL); item->attribute = atoi(data);
		Sql_GetData(mail_handle,14, &data, NULL); item->identify = atoi(data);
		for (j = 0; j < MAX_SLOTS; j++)
		{
			Sql_GetData(mail_handle,15 + j, &data, NULL);
			item->card[j] = atoi(data);
		}

		j = 1;
	}

	StringBuf_Destroy(&buf);
	Sql_FreeResult(mail_handle);

	if (message->read == 1)
	{
		message->read = 0;
		if (flag)
			if ( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `read_flag` = '2' WHERE `id` = '%d'", mail_db, message->id) )
				Sql_ShowDebug(mail_handle);
	}
	else
		message->read = 1;

	return j;
}

/*==========================================
 * Client Inbox Request
 *------------------------------------------*/
int mapif_Mail_sendinbox(int fd, int char_id, unsigned char flag)
{
	WFIFOHEAD(fd, sizeof(struct mail_data) + 9);
	mail_fromsql(char_id, mail_data_pt);
	WFIFOW(fd,0) = 0x3848;
	WFIFOW(fd,2) = sizeof(struct mail_data) + 9;
	WFIFOL(fd,4) = char_id;
	WFIFOB(fd,8) = flag;
	memcpy(WFIFOP(fd,9),mail_data_pt,sizeof(struct mail_data));
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int mapif_parse_Mail_requestinbox(int fd)
{
	RFIFOHEAD(fd);
	mapif_Mail_sendinbox(fd, RFIFOL(fd,2), RFIFOB(fd,6));

	return 0;
}

/*==========================================
 * Mail Readed Mark
 *------------------------------------------*/
int mapif_parse_Mail_read(int fd)
{
	int mail_id;
	RFIFOHEAD(fd);

	mail_id = RFIFOL(fd,2);
	if( SQL_ERROR == Sql_Query(mail_handle, "UPDATE `%s` SET `read_flag` = '2' WHERE `id` = '%d'", mail_db, mail_id) )
		Sql_ShowDebug(mail_handle);
	
	return 0;
}

/*==========================================
 * Client Attachment Request
 *------------------------------------------*/
int mail_DeleteAttach(int mail_id)
{
	StringBuf buf;
	int i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `zeny` = '0', `nameid` = '0', `amount` = '0', `refine` = '0', `attribute` = '0', `identify` = '0'", mail_db);
	for (i = 0; i < MAX_SLOTS; i++)
		StringBuf_Printf(&buf, ", `card%d` = '0'", i);
	StringBuf_Printf(&buf, " WHERE `id` = '%d'", mail_id);

	if( SQL_ERROR == Sql_Query(mail_handle, StringBuf_Value(&buf)) )
	{
		Sql_ShowDebug(mail_handle);
		StringBuf_Destroy(&buf);

		return 0;
	}

	StringBuf_Destroy(&buf);
	return 1;
}

int mapif_Mail_getattach(int fd, int char_id, int mail_id)
{
	struct mail_message *message = (struct mail_message*)aCalloc(sizeof(struct mail_message), 1);

	if( mail_loadmessage(char_id, mail_id, message, 0) )
	{
		if( (message->item.nameid < 1 || message->item.amount < 1) && message->zeny < 1 )
		{
			aFree(message);
			return 0; // No Attachment
		}

		if( mail_DeleteAttach(mail_id) )
		{
			WFIFOHEAD(fd, sizeof(struct item) + 12);
			WFIFOW(fd,0) = 0x384a;
			WFIFOW(fd,2) = sizeof(struct item) + 12;
			WFIFOL(fd,4) = char_id;
			WFIFOL(fd,8) = (message->zeny > 0)?message->zeny:0;
			memcpy(WFIFOP(fd,12), &message->item, sizeof(struct item));
			WFIFOSET(fd,WFIFOW(fd,2));
		}
	}

	aFree(message);
	return 0;
}

int mapif_parse_Mail_getattach(int fd)
{
	RFIFOHEAD(fd);
	mapif_Mail_getattach(fd, RFIFOL(fd,2), RFIFOL(fd,6));
	return 0;
}

/*==========================================
 * Delete Mail
 *------------------------------------------*/
int mapif_Mail_delete(int fd, int char_id, int mail_id)
{
	short flag = 0;

	if ( SQL_ERROR == Sql_Query(mail_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
	{
		Sql_ShowDebug(mail_handle);
		flag = 1;
	}

	WFIFOHEAD(fd,11);
	WFIFOW(fd,0) = 0x384b;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOW(fd,10) = flag;
	WFIFOSET(fd,12);
	
	return 0;
}

int mapif_parse_Mail_delete(int fd)
{
	RFIFOHEAD(fd);
	mapif_Mail_delete(fd, RFIFOL(fd,2), RFIFOL(fd,6));
	return 0;
}
/*==========================================
 * Return Mail
 *------------------------------------------*/
int mapif_Mail_return(int fd, int char_id, int mail_id)
{
	struct mail_message *msg = (struct mail_message*)aCalloc(sizeof(struct mail_message), 1);
	int new_mail = 0;

	if( mail_loadmessage(char_id, mail_id, msg, 0) )
	{
		if( SQL_ERROR == Sql_Query(mail_handle, "DELETE FROM `%s` WHERE `id` = '%d'", mail_db, mail_id) )
			Sql_ShowDebug(mail_handle);
		else
		{
			char temp_[MAIL_TITLE_LENGTH];

			swap(msg->send_id, msg->dest_id);
			safestrncpy(temp_, msg->send_name, NAME_LENGTH);
			safestrncpy(msg->send_name, msg->dest_name, NAME_LENGTH);
			safestrncpy(msg->dest_name, temp_, NAME_LENGTH);

			snprintf(temp_, MAIL_TITLE_LENGTH, "RE:%s", msg->title);
			safestrncpy(msg->title, temp_, MAIL_TITLE_LENGTH);
			msg->timestamp = (unsigned int)calc_times();

			new_mail = mail_savemessage(msg);
		}
	}

	aFree(msg);

	WFIFOHEAD(fd,14);
	WFIFOW(fd,0) = 0x384c;
	WFIFOL(fd,2) = char_id;
	WFIFOL(fd,6) = mail_id;
	WFIFOL(fd,10) = new_mail;
	WFIFOSET(fd,14);

	return 0;
}

int mapif_parse_Mail_return(int fd)
{
	RFIFOHEAD(fd);
	mapif_Mail_return(fd, RFIFOL(fd,2), RFIFOL(fd,6));
	return 0;
}

int mapif_Mail_send(int fd, struct mail_message *msg)
{
	int len = strlen(msg->title) + 16;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x384d;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = msg->send_id;
	WFIFOL(fd,8) = msg->id;
	WFIFOL(fd,12) = msg->dest_id;
	memcpy(WFIFOP(fd,16), msg->title, strlen(msg->title));
	WFIFOSET(fd,len);

	return 0;
}

int mapif_parse_Mail_send(int fd)
{
	struct mail_message *msg;
	int mail_id = 0, account_id = 0;

	if(RFIFOW(fd,2) != 8 + sizeof(struct mail_message))
		return 0;

	msg = (struct mail_message*)aCalloc(sizeof(struct mail_message), 1);
	memcpy(msg, RFIFOP(fd,8), sizeof(struct mail_message));

	account_id = RFIFOL(fd,4);

	if( !msg->dest_id )
	{
		// Try to find the Dest Char by Name
		char esc_name[NAME_LENGTH*2+1];
		
		Sql_EscapeStringLen(sql_handle, esc_name, msg->dest_name, strnlen(msg->dest_name, NAME_LENGTH));
		if ( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`, `char_id` FROM `%s` WHERE `name` = '%s'", char_db, esc_name) )
			Sql_ShowDebug(sql_handle);
		else if ( Sql_NumRows(sql_handle) > 0 )
		{
			char *data;
			Sql_NextRow(sql_handle);
			Sql_GetData(sql_handle, 0, &data, NULL);
			if (atoi(data) != account_id)
			{ // Cannot sends mail to char in the same account
				Sql_GetData(sql_handle, 1, &data, NULL);
				msg->dest_id = atoi(data);
			}
		}
		Sql_FreeResult(sql_handle);
	}

	if( msg->dest_id > 0 )
		mail_id = mail_savemessage(msg);

	msg->id = mail_id;
	mapif_Mail_send(fd, msg);

	aFree(msg);
	return 0;
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_mail_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3048: mapif_parse_Mail_requestinbox(fd); break;
		case 0x3049: mapif_parse_Mail_read(fd); break;
		case 0x304a: mapif_parse_Mail_getattach(fd); break;
		case 0x304b: mapif_parse_Mail_delete(fd); break;
		case 0x304c: mapif_parse_Mail_return(fd); break;
		case 0x304d: mapif_parse_Mail_send(fd); break;
		default:
			return 0;
	}
	return 1;
}

int inter_mail_sql_init(void)
{
	mail_data_pt = (struct mail_data*)aCalloc(sizeof(struct mail_data), 1);
	return 1;
}

void inter_mail_sql_final(void)
{
	if (mail_data_pt) aFree(mail_data_pt);
	return;
}
