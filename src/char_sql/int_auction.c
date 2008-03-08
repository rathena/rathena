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

// This is set to limit the search result
// On iRO, no one uses auctions, so there is no way to know
#define MAX_SEARCH_RESULTS 30

static DBMap* auction_db_ = NULL; // int auction_id -> struct auction_data*

void auction_delete(struct auction_data *auction);
static int auction_end_timer(int tid, unsigned int tick, int id, int data);

// Copy Paste from map/mail.c
time_t calc_times(void)
{
	time_t temp = time(NULL);
	return mktime(localtime(&temp));
}

static int auction_count(int char_id, bool buy)
{
	int i = 0;
	struct auction_data *auction;
	DBIterator* iter;
	DBKey key;

	iter = auction_db_->iterator(auction_db_);
	for( auction = iter->first(iter,&key); iter->exists(iter); auction = iter->next(iter,&key) )
	{
		if( (buy && auction->buyer_id == char_id) || (!buy && auction->seller_id == char_id) )
			i++;
	}
	iter->destroy(iter);

	return i;
}

void auction_save(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `seller_id` = '%d', `seller_name` = ?, `buyer_id` = '%d', `buyer_name` = ?, `price` = '%d', `buynow` = '%d', `hours` = '%d', `timestamp` = '%d', `nameid` = '%d', `item_name` = ?, `type` = '%d', `refine` = '%d', `attribute` = '%d'",
		auction_db, auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ", `card%d` = '%d'", j, auction->item.card[j]);
	StringBuf_Printf(&buf, " WHERE `auction_id` = '%d'", auction->auction_id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);
}

unsigned int auction_create(struct auction_data *auction)
{
	int j;
	StringBuf buf;
	SqlStmt* stmt;

	if( !auction )
		return false;

	auction->timestamp = (int)calc_times() + (auction->hours * 3600);

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`seller_id`,`seller_name`,`buyer_id`,`buyer_name`,`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`", auction_db);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ",`card%d`", j);
	StringBuf_Printf(&buf, ") VALUES ('%d',?,'%d',?,'%d','%d','%d','%d','%d',?,'%d','%d','%d'",
		auction->seller_id, auction->buyer_id, auction->price, auction->buynow, auction->hours, auction->timestamp, auction->item.nameid, auction->type, auction->item.refine, auction->item.attribute);
	for( j = 0; j < MAX_SLOTS; j++ )
		StringBuf_Printf(&buf, ",'%d'", auction->item.card[j]);
	StringBuf_AppendStr(&buf, ")");
	
	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_SUCCESS != SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 0, SQLDT_STRING, auction->seller_name, strnlen(auction->seller_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 1, SQLDT_STRING, auction->buyer_name, strnlen(auction->buyer_name, NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_BindParam(stmt, 2, SQLDT_STRING, auction->item_name, strnlen(auction->item_name, ITEM_NAME_LENGTH))
	||  SQL_SUCCESS != SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		auction->auction_id = 0;
	}
	else
	{
		struct auction_data *auction_;
		unsigned int tick = auction->hours * 3600000;

		auction->item.amount = 1;
		auction->item.identify = 1;

		auction->auction_id = (unsigned int)SqlStmt_LastInsertId(stmt);
		auction->auction_end_timer = add_timer( gettick() + tick , auction_end_timer, auction->auction_id, 0);
		ShowInfo("New Auction %u | time left %u ms | By %s.\n", auction->auction_id, tick, auction->seller_name);

		CREATE(auction_, struct auction_data, 1);
		memcpy(auction_, auction, sizeof(struct auction_data));
		idb_put(auction_db_, auction_->auction_id, auction_);
	}

	SqlStmt_Free(stmt);
	StringBuf_Destroy(&buf);

	return auction->auction_id;
}

static int auction_end_timer(int tid, unsigned int tick, int id, int data)
{
	struct auction_data *auction;
	if( (auction = (struct auction_data *)idb_get(auction_db_, id)) != NULL )
	{
		struct mail_message msg;
		memset(&msg, 0, sizeof(struct mail_message));

		msg.send_id = 0;
		safestrncpy(msg.send_name, "Auction Manager", NAME_LENGTH);
		safestrncpy(msg.title, "Auction", MAIL_TITLE_LENGTH);
		msg.timestamp = (int)calc_times();

		if( auction->buyer_id )
		{ // Send item to Buyer's Mail (custom messages)
			msg.dest_id = auction->buyer_id;
			safestrncpy(msg.dest_name, auction->buyer_name, NAME_LENGTH);
			safestrncpy(msg.body, "Thanks, you won the auction!.", MAIL_BODY_LENGTH);
		}
		else
		{ // Return item to Seller's Mail (custom messages)
			msg.dest_id = auction->seller_id;
			safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
			safestrncpy(msg.body, "Sorry, No one buy your item...", MAIL_BODY_LENGTH);
		}

		memcpy(&msg.item, &auction->item, sizeof(struct item));

		mail_savemessage(&msg);
		mapif_Mail_new(&msg);

		if( auction->buyer_id )
		{ // Send Money to Seller
			memset(&msg, 0, sizeof(struct mail_message));

			msg.send_id = 0;
			safestrncpy(msg.send_name, "Auction Manager", NAME_LENGTH);
			msg.dest_id = auction->seller_id;
			safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
			msg.timestamp = (int)calc_times();
			msg.zeny = auction->price;

			// Custom Messages, need more info
			safestrncpy(msg.title, "Auction", MAIL_TITLE_LENGTH);
			safestrncpy(msg.body, "Here is the money from your Auction.", MAIL_BODY_LENGTH);

			mail_savemessage(&msg);
			mapif_Mail_new(&msg);
		}

		ShowInfo("Auction End: id %u.\n", auction->auction_id);

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
		"`price`,`buynow`,`hours`,`timestamp`,`nameid`,`item_name`,`type`,`refine`,`attribute`");
	for( i = 0; i < MAX_SLOTS; i++ )
		StringBuf_Printf(&buf, ",`card%d`", i);
	StringBuf_Printf(&buf, " FROM `%s` ORDER BY `auction_id` DESC", auction_db);

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
		Sql_GetData(sql_handle,10, &data, NULL); safestrncpy(auction->item_name, data, ITEM_NAME_LENGTH);
		Sql_GetData(sql_handle,11, &data, NULL); auction->type = atoi(data);

		Sql_GetData(sql_handle,12, &data, NULL); item->refine = atoi(data);
		Sql_GetData(sql_handle,13, &data, NULL); item->attribute = atoi(data);

		item->identify = 1;
		item->amount = 1;

		for( i = 0; i < MAX_SLOTS; i++ )
		{
			Sql_GetData(sql_handle, 14 + i, &data, NULL);
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

static void mapif_Auction_sendlist(int fd, int char_id, short count, unsigned char *buf)
{
	int len = (sizeof(struct auction_data) * count) + 8;

	WFIFOHEAD(fd, len);
	WFIFOW(fd,0) = 0x3850;
	WFIFOW(fd,2) = len;
	WFIFOL(fd,4) = char_id;
	memcpy(WFIFOP(fd,8), buf, len - 8);
	WFIFOSET(fd,len);
}

static void mapif_parse_Auction_requestlist(int fd)
{
	char searchtext[NAME_LENGTH];
	int char_id = RFIFOL(fd,4), len = sizeof(struct auction_data);
	unsigned int price = RFIFOL(fd,10);
	short type = RFIFOW(fd,8);
	unsigned char buf[MAX_SEARCH_RESULTS * sizeof(struct auction_data)];
	DBIterator* iter;
	DBKey key;
	struct auction_data *auction;
	short i = 0;

	memcpy(searchtext, RFIFOP(fd,14), NAME_LENGTH);

	iter = auction_db_->iterator(auction_db_);
	for( auction = iter->first(iter,&key); iter->exists(iter) && i < MAX_SEARCH_RESULTS; auction = iter->next(iter,&key) )
	{
		if( (type == 0 && auction->type != IT_ARMOR && auction->type != IT_PETARMOR) || 
			(type == 1 && auction->type != IT_WEAPON) ||
			(type == 2 && auction->type != IT_CARD) ||
			(type == 3 && auction->type != IT_ETC) ||
			(type == 4 && !strstr(auction->item_name, searchtext)) ||
			(type == 5 && auction->price > price) ||
			(type == 6 && auction->seller_id != char_id) ||
			(type == 7 && auction->buyer_id != char_id) )
			continue;

		memcpy(WBUFP(buf, i * len), auction, len);
		i++;
	}
	iter->destroy(iter);

	mapif_Auction_sendlist(fd, char_id, i, buf);
}

static void mapif_Auction_register(int fd, struct auction_data *auction)
{
	int len = sizeof(struct auction_data) + 4;

	WFIFOHEAD(fd,len);
	WFIFOW(fd,0) = 0x3851;
	WFIFOW(fd,2) = len;
	memcpy(WFIFOP(fd,4), auction, sizeof(struct auction_data));
	WFIFOSET(fd,len);
}

static void mapif_parse_Auction_register(int fd)
{
	struct auction_data auction;
	if( RFIFOW(fd,2) != sizeof(struct auction_data) + 4 )
		return;

	memcpy(&auction, RFIFOP(fd,4), sizeof(struct auction_data));
	if( auction_count(auction.seller_id, false) < 5 )
		auction.auction_id = auction_create(&auction);

	mapif_Auction_register(fd, &auction);
}

static void mapif_Auction_cancel(int fd, int char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3852;
	WFIFOL(fd,8) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

static void mapif_parse_Auction_cancel(int fd)
{
	int char_id = RFIFOL(fd,2), auction_id = RFIFOL(fd,6);
	struct auction_data *auction;
	struct mail_message msg;

	if( (auction = (struct auction_data *)idb_get(auction_db_, auction_id)) == NULL )
	{
		mapif_Auction_cancel(fd, char_id, 1); // Bid Number is Incorrect
		return;
	}

	if( auction->seller_id != char_id )
	{
		mapif_Auction_cancel(fd, char_id, 2); // You cannot end the auction
		return;
	}

	if( auction->buyer_id > 0 )
	{
		mapif_Auction_cancel(fd, char_id, 3); // An auction with at least one bidder cannot be canceled
		return;
	}

	memset(&msg, 0, sizeof(struct mail_message));
	safestrncpy(msg.send_name, "Auction Manager", NAME_LENGTH);
	msg.dest_id = auction->seller_id;
	safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
	msg.timestamp = (int)calc_times();
	safestrncpy(msg.title, "Auction", MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, "Auction canceled.", MAIL_BODY_LENGTH);

	memcpy(&msg.item, &auction->item, sizeof(struct item));

	mail_savemessage(&msg);
	mapif_Mail_new(&msg);
	
	auction_delete(auction);
	mapif_Auction_cancel(fd, char_id, 0); // The auction has been canceled
}

static void mapif_Auction_close(int fd, int char_id, unsigned char result)
{
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3853;
	WFIFOL(fd,8) = char_id;
	WFIFOB(fd,6) = result;
	WFIFOSET(fd,7);
}

static void mapif_parse_Auction_close(int fd)
{
	int char_id = RFIFOL(fd,2), auction_id = RFIFOL(fd,6);
	struct auction_data *auction;
	struct mail_message msg;

	if( (auction = (struct auction_data *)idb_get(auction_db_, auction_id)) == NULL )
	{
		mapif_Auction_cancel(fd, char_id, 2); // Bid Number is Incorrect
		return;
	}

	if( auction->buyer_id == 0 )
	{
		mapif_Auction_cancel(fd, char_id, 1); // You cannot end the auction
		return;
	}

	// Send Money to Seller
	memset(&msg, 0, sizeof(struct mail_message));
	safestrncpy(msg.send_name, "Auction Manager", NAME_LENGTH);
	msg.dest_id = auction->seller_id;
	safestrncpy(msg.dest_name, auction->seller_name, NAME_LENGTH);
	msg.timestamp = (int)calc_times();
	safestrncpy(msg.title, "Auction", MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, "Auction closed.", MAIL_BODY_LENGTH);
	msg.zeny = auction->price; // Current Bid
	mail_savemessage(&msg);
	mapif_Mail_new(&msg);

	// Send Item to Buyer
	memset(&msg, 0, sizeof(struct mail_message));
	safestrncpy(msg.send_name, "Auction Manager", NAME_LENGTH);
	msg.dest_id = auction->buyer_id;
	safestrncpy(msg.dest_name, auction->buyer_name, NAME_LENGTH);
	msg.timestamp = (int)calc_times();
	safestrncpy(msg.title, "Auction", MAIL_TITLE_LENGTH);
	safestrncpy(msg.body, "Auction winner.", MAIL_BODY_LENGTH);
	memcpy(&msg.item, &auction->item, sizeof(struct item));
	mail_savemessage(&msg);
	mapif_Mail_new(&msg);

	mapif_Auction_cancel(fd, char_id, 0); // You have ended the auction
}

/*==========================================
 * Packets From Map Server
 *------------------------------------------*/
int inter_auction_parse_frommap(int fd)
{
	switch(RFIFOW(fd,0))
	{
		case 0x3050: mapif_parse_Auction_requestlist(fd); break;
		case 0x3051: mapif_parse_Auction_register(fd); break;
		case 0x3052: mapif_parse_Auction_cancel(fd); break;
		case 0x3053: mapif_parse_Auction_close(fd); break;
		default:
			return 0;
	}
	return 1;
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
