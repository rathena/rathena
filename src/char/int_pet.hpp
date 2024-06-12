// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_PET_HPP
#define INT_PET_HPP

#include <common/cbasetypes.hpp>

struct s_pet;

int inter_pet_init(void);
void inter_pet_sql_final(void);
int inter_pet_save(void);
int inter_pet_delete(int pet_id);

int inter_pet_parse_frommap(int fd);
int inter_pet_sql_init(void);
  //extern char pet_txt[256];

int inter_pet_tosql(int pet_id, struct s_pet *p);

#endif /* INT_PET_HPP */
