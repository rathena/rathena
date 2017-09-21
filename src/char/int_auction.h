// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_AUCTION_SQL_H_
#define _INT_AUCTION_SQL_H_

#ifdef __cplusplus
extern "C" {
#endif

int inter_auction_parse_frommap(int fd);

int inter_auction_sql_init(void);
void inter_auction_sql_final(void);

#ifdef __cplusplus
}
#endif

#endif /* _INT_AUCTION_SQL_H_ */
