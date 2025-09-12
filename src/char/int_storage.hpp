// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_STORAGE_HPP
#define INT_STORAGE_HPP

#include <common/cbasetypes.hpp>

struct s_storage;

void inter_storage_sql_init(void);
void inter_storage_sql_final(void);

int32 inter_premiumStorage_getMax(uint8 id);
const char *inter_premiumStorage_getTableName(uint8 id);
const char *inter_premiumStorage_getPrintableName(uint8 id);

bool inter_storage_parse_frommap(int32 fd);

bool guild_storage_tosql(int32 guild_id, struct s_storage *p);

#endif /* INT_STORAGE_HPP */
