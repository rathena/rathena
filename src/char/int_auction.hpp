// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_AUCTION_HPP
#define INT_AUCTION_HPP

#include <common/cbasetypes.hpp>

int32 inter_auction_parse_frommap(int32 fd);

int32 inter_auction_sql_init(void);
void inter_auction_sql_final(void);

#endif /* INT_AUCTION_HPP */
