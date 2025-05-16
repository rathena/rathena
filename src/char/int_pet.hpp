// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_PET_HPP
#define INT_PET_HPP

#include <common/cbasetypes.hpp>

struct s_pet;

int32 inter_pet_init(void);
void inter_pet_sql_final(void);
int32 inter_pet_save(void);
int32 inter_pet_delete(int32 pet_id);

int32 inter_pet_parse_frommap(int32 fd);
int32 inter_pet_sql_init(void);
  //extern char pet_txt[256];

int32 inter_pet_tosql(int32 pet_id, struct s_pet *p);

#endif /* INT_PET_HPP */
