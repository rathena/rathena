// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_ELEMENTAL_HPP_
#define _INT_ELEMENTAL_HPP_

struct s_elemental;

void inter_elemental_sql_init(void);
void inter_elemental_sql_final(void);
int inter_elemental_parse_frommap(int fd);

bool mapif_elemental_delete(int ele_id);

#endif /* _INT_ELEMENTAL_HPP_ */
