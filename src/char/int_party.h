// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

struct party;

int inter_party_init(void);
void inter_party_final(void);
int inter_party_save(void);
int inter_party_parse_frommap(int fd);
int inter_party_leave(int party_id,int account_id, int char_id);

extern char party_txt[1024];

//For the TXT->SQL converter
int inter_party_fromstr(char *str, struct party *p);

#endif /* _INT_PARTY_H_ */
