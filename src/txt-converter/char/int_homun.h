// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_HOMUN_H_
#define _INT_HOMUN_H_

int inter_homun_init(void);
void inter_homun_final(void);
int inter_homun_save(void);
int inter_homun_delete(int homun_id);
int inter_homun_parse_frommap(int fd);

extern char homun_txt[1024];

#endif /* _INT_HOMUN_H_ */
