// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_HOMUN_SQL_H_
#define _INT_HOMUN_SQL_H_

int inter_homunculus_sql_init(void);
void inter_homunculus_sql_final(void);
int mapif_save_homunculus(struct s_homunculus *hd);
int mapif_load_homunculus(int fd);
int mapif_delete_homunculus(int fd);
int inter_delete_homunculus(int hom_id);
int inter_homunculus_parse_frommap(int fd);

#endif /* _INT_HOMUN_SQL_H_ */
