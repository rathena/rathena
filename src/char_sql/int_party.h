#ifndef _INT_PARTY_H_
#define _INT_PARTY_H_

int inter_party_parse_frommap(int fd);
int inter_party_sql_init();
void inter_party_sql_final();
int inter_party_leave(int party_id,int account_id);

#endif
