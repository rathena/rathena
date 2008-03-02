// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "../common/timer.h"
#include "char.h"
#include "inter.h"
#include "int_mail.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static DBMap* auction_db_ = NULL; // int auction_id -> struct auction_data*

void auction_delete(struct auction_data *auction);
static int auction_end_timer(int tid, unsigned int tick, int id, int data);

// Copy Paste from map/mail.c
time_t calc_times(void)
{
	time_t temp = time(NULL);
	return mktime(localtime(&temp));
}

void inter_auction_save(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `seller_id` = '%d', `seller_name` = ?, `buyer_id` = '%d', `buyer_name` = ?, `price` = '%d', `buynow` = '%d', `hours` = '%d', `timestamp` = '%d', `nameid` = '%d', `refine` = '%d', `attribute` = '%d'",
		auction_db, auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, auction->timestamp, auction->item.nameid, auction->item.refine, auction->item.attribute);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ", `card%d` = '%d'", j, auction->item.card[j]);
	StringBuf_Printf(&buf, " WHERE `auction_id` = '%d'", auction->auction_id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
}

static bool auction_create(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return false;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,`price`,`buynow`,`hours`,`timestamp`,`nameid`,`refine`,`attribute`");
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, ") VALUES ('%d',?,'%d',?,'%d','%d','%d','%d','%d','%d','%d'",
		auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, auction->timestamp, auction->item.nameid, auction->item.refine, auction->item.attribute);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ",'%d'", auction->item.card[j]);
	StringBuf_AppendStr(&buf, ")");
	
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH)) )
	{
		SqlStmt_ShowDebug(stmt);
		auction->auction_id = 0;
	}
	else
	{
		auction->item.amount = 1;
		auction->item.identify = 1;

		auction->auction_id = (unsigned int)SqlStmt_LastInsertId(stmt);
		auction->auction_end_timer = add_timer( gettick() + ((auction->timestamp - (unsigned int)calc_times) * 1000) , auction_end_timer, auction->auction_id, 0);
		idb_put(auction_db_, auction->auction_id, auction);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return (auction->auction_id > 0);
}

static int auction_end_timer(int tid, unsigned int tick, int id, int data)
{
	struct auction_data *auction;
	if( (auction = (struct auction_data *)idb_get(auction_db_, id)) != NULL )
	{
		struct mail_message msg;
		memset(&msg, 0, sizeof(struct mail_message));

		msg.send_id = auction->seller_id;
		safestrncpy(msg.send_name, auction->seller_name, NAME_LENGTH);
		msg.timestamp = (unsigned int)calc_times();

		if( auction->buyer_id )
		{ // Send item to Buyer's Mail (custom messages)
			msg.dest_id = auction->buyer_id;
			safestrncpy(msg.dest_name, auction->buyer_name, NAME_LENGTH);
			safestrncpy(msg.title, "[Auction Winner] Your Item", MAIL_TITLE_LENGTH);
			safestrncpy(msg.body, "Thanks, you won the auction!.", MAIL_BODY_LENGTH);
		}
		else
		{ // Return item to Seller's Mail (custom messages)
			msg.dest_id = auction->seller_id;
			safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
			safestrncpy(msg.title, "[Auction Fail] Your Item", MAIL_TITLE_LENGTH);
			safestrncpy(msg.body, "Sorry, No one buy your item...", MAIL_BODY_LENGTH);
		}

		memcpy(&msg.item, &auction->item, sizeof(struct item));

		mail_savemessage(&msg);
		mapif_Mail_new(&msg);

		if( auction->buyer_id )
		{ // Send Money to Seller
			memset(&msg, 0, sizeof(struct mail_message));

			msg.send_id = auction->buyer_id;
			safestrncpy(msg.send_name, auction->buyer_name, NAME_LENGTH);
			msg.dest_id = auction->seller_id;
			safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
			msg.timestamp = (unsigned int)calc_times();
			msg.zeny = auction->price;

			// Custom Messages, need more info
			safestrncpy(msg.title, "[Auction] Your Zeny", MAIL_TITLE_LENGTH);
			safestrncpy(msg.body, "Thanks, you won the auction!.", MAIL_BODY_LENGTH);

			mail_savemessage(&msg);
			mapif_Mail_new(&msg);
		}

		auction->auction_end_timer = -1;
		auction_delete(auction);
	}

	return 0;
}

void auction_delete(struct auction_data *auction)
{
	unsigned int auction_id = auction->auction_id;

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `auction_id` = '%d'", auction_db, auction_id) )
		Sql_ShowDebug(sql_handle);

	if( auction->auction_end_timer != -1 )
		delete_timer(auction->auction_end_timer, auction_end_timer);

	aFree(auction);
	idb_remove(auction_db_, auction_id);
}

void inter_auctions_fromsql(void)
{
	int i;
	struct auction_data *auction;
	struct item *item;
	char *data;
	StringBuf buf;
	unsigned int tick = gettick(), endtick, now = (unsigned int)calc_times();

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `auction_id`,`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,"
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`refine`,`attribute`");
	for( i = 0; i < MAX_SLOTS; i++ )
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` ORDER BY `id` DESC", auction_db);

	if( SQL_ERROR == Sql_Query(sql_handle, StringBuf_Value(&buf)) )
		Sql_ShowDebug(sql_handle);

	StringBuf_Destroy(&buf);

	while( SQL_SUCCESS == Sql_NextRow(sql_handle) )
	{
		CREATE(auction, struct auction_data, 1);
		Sql_GetData(sql_handle, 0, &data, NULL); auction->auction_id = atoi(data);
		Sql_GetData(sql_handle, 1, &data, NULL); auction->seller_id = atoi(data);
		Sql_GetData(sql_handle, 2, &data, NULL); safestrncpy(auction->seller_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 3, &data, NULL); auction->buyer_id = atoi(data);
		Sql_GetData(sql_handle, 4, &data, NULL); safestrncpy(auction->buyer_name, data, NAME_LENGTH);
		Sql_GetData(sql_handle, 5, &data, NULL); auction->price	= atoi(data);
		Sql_GetData(sql_handle, 6, &data, NULL); auction->buynow = atoi(data);
		Sql_GetData(sql_handle, 7, &data, NULL); auction->hours = atoi(data);
		Sql_GetData(sql_handle, 8, &data, NULL); auction->timestamp = atoi(data);

		item = &auction->item;
		Sql_GetData(sql_handle, 9, &data, NULL); item->nameid = atoi(data);
		Sql_GetData(sql_handle,10, &data, NULL); item->refine = atoi(data);
		Sql_GetData(sql_handle,11, &data, NULL); item->attribute = atoi(data);

		item->identify = 1;
		item->amount = 1;

		for( i = 0; i < MAX_SLOTS; i++ )
		{
			Sql_GetData(sql_handle, 12 + i, &data, NULL);
			item->card[i] = atoi(data);
		}

		if( auction->timestamp > now )
			endtick = ((auction->timestamp - now) * 1000) + tick;
		else
			endtick = tick + 10000; // 10 Second's to process ended auctions

		auction->auction_end_timer = add_timer(endtick, auction_end_timer, auction->auction_id, 0);
		idb_put(auction_db_, auction->auction_id, auction);
	}

	Sql_FreeResult(sql_handle);
}

int inter_auction_sql_init(void)
{
	auction_db_ = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_auctions_fromsql();

	return 0;
}

void inter_auction_sql_final(void)
{
	auction_db_->destroy(auction_db_,NULL);

	return;
}
