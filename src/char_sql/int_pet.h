#ifndef _INT_PET_H_
#define _INT_PET_H_

int inter_pet_init();
void inter_pet_sql_final();
int inter_pet_save();
int inter_pet_delete(int pet_id);

int inter_pet_parse_frommap(int fd);
int inter_pet_sql_init();
//extern char pet_txt[256];

#endif
