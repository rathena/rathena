// $Id: int_party.h,v 1.1.1.1 2004/09/10 17:26:51 MagicalTux Exp $
#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

int inter_party_init();
int inter_party_save();

int inter_party_parse_frommap(int fd);

int inter_party_leave(int party_id,int account_id);

extern char party_txt[1024];

#endif
