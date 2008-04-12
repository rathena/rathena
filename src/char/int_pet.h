// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PET_H_
#define _INT_PET_H_

struct s_pet;

int inter_pet_init(void);
void inter_pet_final(void);
int inter_pet_save(void);
int inter_pet_delete(int pet_id);

int inter_pet_parse_frommap(int fd);

extern char pet_txt[1024];

//Exported for use in the TXT-SQL converter.
int inter_pet_fromstr(char *str,struct s_pet *p);

#endif /* _INT_PET_H_ */
