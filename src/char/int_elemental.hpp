// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_ELEMENTAL_HPP
#define INT_ELEMENTAL_HPP

#include <common/cbasetypes.hpp>

struct s_elemental;

void inter_elemental_sql_init(void);
void inter_elemental_sql_final(void);
int32 inter_elemental_parse_frommap(int32 fd);

bool mapif_elemental_delete(int32 ele_id);

#endif /* INT_ELEMENTAL_HPP */
