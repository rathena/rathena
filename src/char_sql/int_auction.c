// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/mmo.h"
#include "../common/malloc.h"
#include "../common/db.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "../common/sql.h"
#include "char.h"
#include "inter.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static DBMap* auction_db = NULL; // int auction_id -> struct auction*

int inter_auction_sql_init(void)
{
	auction_db = idb_alloc(DB_OPT_RELEASE_DATA);
	
	return 0;
}

void inter_auction_sql_final(void)
{
	auction_db->destroy(auction_db,NULL);

	return;
}
