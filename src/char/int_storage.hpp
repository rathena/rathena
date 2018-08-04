// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_STORAGE_HPP_
#define _INT_STORAGE_HPP_

#include "../common/cbasetypes.hpp"

struct s_storage;

void inter_storage_sql_init(void);
void inter_storage_sql_final(void);

bool inter_premiumStorage_exists(uint8 id);
int inter_premiumStorage_getMax(uint8 id);
const char *inter_premiumStorage_getTableName(uint8 id);
const char *inter_premiumStorage_getPrintableName(uint8 id);

bool inter_storage_parse_frommap(int fd);

bool guild_storage_tosql(int guild_id, struct s_storage *p);

#endif /* _INT_STORAGE_HPP_ */
