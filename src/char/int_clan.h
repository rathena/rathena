// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _INT_CLAN_H_
#define _INT_CLAN_H_

#ifdef __cplusplus
extern "C" {
#endif

int inter_clan_parse_frommap( int fd );
int inter_clan_init(void);
void inter_clan_final(void);

#ifdef __cplusplus
}
#endif

#endif /* _INT_CLAN_H_ */
