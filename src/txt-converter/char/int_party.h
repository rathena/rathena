#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

int inter_party_init();
int inter_party_save();

int inter_party_parse_frommap(int fd);

extern char party_txt[256];

#endif
