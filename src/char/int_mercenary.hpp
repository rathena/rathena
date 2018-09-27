// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_MERCENARY_HPP
#define INT_MERCENARY_HPP

#include "../common/cbasetypes.hpp"

int inter_mercenary_sql_init(void);
void inter_mercenary_sql_final(void);
int inter_mercenary_parse_frommap(int fd);

// Mercenary Owner Database
bool mercenary_owner_fromsql(uint32 char_id, struct mmo_charstatus *status);
bool mercenary_owner_tosql(uint32 char_id, struct mmo_charstatus *status);
bool mercenary_owner_delete(uint32 char_id);

bool mapif_mercenary_delete(int merc_id);


#endif /* INT_MERCENARY_HPP */
